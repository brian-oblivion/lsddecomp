#!/usr/bin/env bash
#
# tools/setup.sh — one-command bootstrap for a fresh clone.
#
#   ./tools/setup.sh              # run every step that is not already done
#   ./tools/setup.sh --force      # redo every step from scratch
#   ./tools/setup.sh --no-verify  # skip the final build (faster; not advised)
#
# This is the ONLY setup path. Matching rounds run in worktrees that symlink an
# already-built toolchain, so this script is the only thing that re-runs often
# enough to catch toolchain drift. Every step is idempotent and fails loudly.
# Do not add `|| true` anywhere -- a setup step that "succeeds" without doing
# its job produces a checkout that builds different bytes than everyone else's.
#
set -euo pipefail

cd "$(dirname "$0")/.."
ROOT=$(pwd)

FORCE=0
VERIFY=1
for arg in "$@"; do
    case "$arg" in
        --force)     FORCE=1 ;;
        --no-verify) VERIFY=0 ;;
        -h|--help)   sed -n '2,12p' "$0"; exit 0 ;;
        *)           echo "unknown option: $arg" >&2; exit 2 ;;
    esac
done

STEP=0
TOTAL=7
step() { STEP=$((STEP+1)); printf '\n\033[1m[%d/%d] %s\033[0m\n' "$STEP" "$TOTAL" "$1"; }
skip() { printf '      already done (%s) — use --force to redo\n' "$1"; }
ok()   { printf '      \033[32mOK\033[0m %s\n' "$1"; }
die()  { printf '\n\033[31mFAILED:\033[0m %s\n' "$1" >&2; exit 1; }
need() { command -v "$1" >/dev/null 2>&1 || die "missing required tool: $1${2:+ ($2)}"; }

# ---------------------------------------------------------------------------
step "Checking host prerequisites"
need python3
need git
need curl "or wget; used to fetch binutils sources"
need make
need sha1sum
need cc "a host C compiler, to build binutils"
ok "python3, git, curl, make, sha1sum, cc"

# ---------------------------------------------------------------------------
step "Locating the game executable"
if [ ! -f disk/SLPS_015.56 ]; then
    # A disc image in the tree is the common case for a first run, so extract
    # from it rather than making the user find the incantation. Only when
    # exactly one image is present: guessing between several is worse than
    # asking.
    mapfile -t images < <(find . -maxdepth 3 \
        \( -iname '*.bin' -o -iname '*.iso' -o -iname '*.img' \) \
        -not -path './build/*' -not -path './.git/*' 2>/dev/null)
    if [ "${#images[@]}" -eq 1 ]; then
        printf '      extracting from %s\n' "${images[0]}"
        python3 tools/extract_exe.py "${images[0]}" \
            || die "could not extract SLPS_015.56 from ${images[0]}"
    elif [ "${#images[@]}" -gt 1 ]; then
        printf '      several disc images found; pick one:\n'
        printf '        %s\n' "${images[@]}"
        die "run: python3 tools/extract_exe.py <image>"
    fi
fi
[ -f disk/SLPS_015.56 ] || die "disk/SLPS_015.56 not found.
      Bring your own copy of SLPS-01556 — see disk/README.md."
sha1sum -c check.sha1 >/dev/null 2>&1 || die "disk/SLPS_015.56 is not the
      expected dump. Every address in config/ is wrong for another revision.
      expected: $(cut -d' ' -f1 check.sha1)
      actual:   $(sha1sum disk/SLPS_015.56 | cut -d' ' -f1)"
ok "disk/SLPS_015.56 verified"

# ---------------------------------------------------------------------------
step "Python venv (splat, m2c and asm-differ dependencies)"
if [ "$FORCE" = 1 ]; then rm -rf .venv; fi
if [ -d .venv ] && .venv/bin/python3 -c 'import splat' >/dev/null 2>&1; then
    skip ".venv imports splat"
else
    [ -d .venv ] || python3 -m venv .venv
    .venv/bin/pip install --quiet --upgrade pip
    .venv/bin/pip install --quiet -r tools/requirements.txt
    # Import for real. splat pulls in spimdisasm and the n64 image segtypes at
    # module load and declares neither as a dependency, so a missing one
    # otherwise surfaces as a confusing `make extract` traceback.
    .venv/bin/python3 -c 'import splat' \
        || die "venv installed but 'import splat' fails. Add the missing
      module to tools/requirements.txt (pinned) rather than installing it ad hoc."
    ok "splat importable"
fi

# ---------------------------------------------------------------------------
step "Cloning asm-differ, m2c, maspsx and decomp-permuter"
clone() {
    local dir=$1 url=$2 probe=$3
    if [ "$FORCE" = 1 ]; then rm -rf "$dir"; fi
    if [ -e "$dir/$probe" ]; then
        skip "$dir"
    else
        rm -rf "$dir"
        git clone --quiet --depth 1 "$url" "$dir" || die "could not clone $url"
        ok "$dir"
    fi
}
# All four are load-bearing. maspsx is IN THE BUILD PIPELINE -- without it the
# executable does not assemble, let alone match.
clone tools/maspsx          https://github.com/mkst/maspsx.git                maspsx.py
clone tools/asm-differ      https://github.com/simonlindholm/asm-differ.git   diff.py
clone tools/m2c             https://github.com/matt-kempster/m2c.git          m2c.py
clone tools/decomp-permuter https://github.com/simonlindholm/decomp-permuter.git permuter.py

# maspsx does not expose addiu_at independently of --aspsx-version: it is only
# switched on below 2.30, where it drags three nop-insertion rules with it
# (nop_at_expansion, nop_mflo_mfhi, nop_lw_lw) that the 2.34 defaults get right
# across every matched function. This patch adds a --addiu-at flag that sets
# ONLY addiu_at. Retail uses the unfolded indexed form at 502 of 502 sites and
# the folded form at none, so the flag is required, not optional -- the
# Makefile passes it and the build does not match without it.
# See docs/research/addiu-at-blocker.md.
if [ -f tools/maspsx/maspsx.py ]; then
    if grep -q -- '--addiu-at' tools/maspsx/maspsx.py; then
        skip "maspsx addiu_at patch (already applied)"
    elif git -C tools/maspsx apply --check ../../tools/patches/maspsx-addiu-at.patch 2>/dev/null; then
        git -C tools/maspsx apply ../../tools/patches/maspsx-addiu-at.patch \
            && ok "maspsx addiu_at patch"
    else
        die "tools/patches/maspsx-addiu-at.patch does not apply to this maspsx checkout -- upstream has moved. The build WILL NOT match until this is resolved; see docs/research/addiu-at-blocker.md."
    fi
fi

# ---------------------------------------------------------------------------
step "Psy-Q compiler (GCC 2.6.3) and mipsel binutils"

# -- GCC 2.6.3 --------------------------------------------------------------
# The compiler Sony shipped in the Psy-Q SDK; cc1's own banner says "Sony
# Playstation". These are prebuilt i386 binaries from decompals/old-gcc, which
# is where the whole PSX decomp scene gets them.
if [ "$FORCE" = 1 ]; then rm -rf tools/gcc263; fi
if [ -x tools/gcc263/cc1 ]; then
    skip "tools/gcc263/cc1"
else
    # `-psx` is the load-bearing half of the name: it is the Psy-Q-patched
    # build, not stock 2.6.3. This exact tarball was confirmed to rebuild
    # SLPS_015.56 byte-for-byte, so it is what the pin below refers to. The
    # binaries inside are not reproducible (build IDs differ between
    # re-releases of the same source), which is why the TARBALL is hashed
    # rather than its contents.
    GCCURL=https://github.com/decompals/old-gcc/releases/download/0.17/gcc-2.6.3-psx.tar.gz
    GCCSHA=2051de9da5cbba81d4c14ef0bce85e35981ccb51
    GCCTMP=$(mktemp -d)
    curl -sfL -o "$GCCTMP/gcc.tar.gz" "$GCCURL" \
        || { rm -rf "$GCCTMP"; die "could not fetch $GCCURL
      If the release layout has changed, drop cc1/cpp/gcc into tools/gcc263/
      by hand -- any Psy-Q-patched GCC 2.6.3 build will do, but PROVE it with
      ./build-and-verify.sh before matching against it."; }
    got=$(sha1sum "$GCCTMP/gcc.tar.gz" | cut -d' ' -f1)
    if [ "$got" != "$GCCSHA" ]; then
        printf '      \033[33mWARNING\033[0m gcc-2.6.3-psx.tar.gz sha1 %s\n' "$got"
        printf '              expected %s -- upstream re-released it.\n' "$GCCSHA"
        printf '              Continuing; the build verification below is the real check.\n'
    fi
    mkdir -p tools/gcc263
    tar xzf "$GCCTMP/gcc.tar.gz" -C tools/gcc263
    rm -rf "$GCCTMP"
    chmod +x tools/gcc263/*
    tools/gcc263/gcc --version 2>/dev/null | grep -q '^2\.6\.3' \
        || die "tools/gcc263/gcc is not GCC 2.6.3"
    ok "tools/gcc263 (cc1, cpp, gcc)"
fi

# -- binutils ---------------------------------------------------------------
# No distro ships a mipsel-linux-gnu binutils, so build one. Only ld, as,
# objcopy, objdump and nm are used, and the build takes a few minutes.
if [ "$FORCE" = 1 ]; then rm -rf tools/binutils; fi
if [ -x tools/binutils/bin/mipsel-linux-gnu-ld ]; then
    skip "tools/binutils/bin"
else
    BUVER=2.43.1
    BUDIR=$(mktemp -d)
    trap 'rm -rf "$BUDIR"' EXIT
    echo "      fetching and building binutils $BUVER (a few minutes)"
    curl -sL -o "$BUDIR/bu.tar.xz" \
        "https://ftp.gnu.org/gnu/binutils/binutils-$BUVER.tar.xz" \
        || die "could not download binutils $BUVER"
    tar xf "$BUDIR/bu.tar.xz" -C "$BUDIR"
    mkdir -p "$BUDIR/build"
    (
        cd "$BUDIR/build"
        # -std=gnu17 is required on GCC 15 hosts.
        "$BUDIR/binutils-$BUVER/configure" \
            --target=mipsel-linux-gnu --prefix="$ROOT/tools/binutils" \
            --disable-gdb --disable-sim --disable-gprofng --disable-nls \
            CFLAGS="-O2 -std=gnu17" >configure.log 2>&1 \
            || { tail -20 configure.log; exit 1; }
        make -j"$(nproc)" >build.log 2>&1 || { tail -30 build.log; exit 1; }
        make install >install.log 2>&1 || { tail -20 install.log; exit 1; }
    ) || die "binutils $BUVER build failed (log tail above)"
    ok "tools/binutils"
fi

# ---------------------------------------------------------------------------
step "Psy-Q library objects (sdk/ -> lib/)"
# The build LINKS Sony's own library objects (splat `o` segments) instead of
# carrying that code as disassembly. They come from the user's SDK disc in
# sdk/ -- bring-your-own, exactly like disk/ -- converted from Sony's LNK
# object format to ELF by pcsx-redux's psyq-obj-parser. The static Linux
# binary is the one decompme/compilers publishes (also what lom-decomp uses).
POP=tools/psyq-obj-parser/psyq-obj-parser
POPURL='https://github.com/decompme/compilers/releases/download/compilers/psyq-obj-parser.tar.gz?2025-03-18'
if [ "$FORCE" = 1 ]; then rm -rf tools/psyq-obj-parser lib; fi
if [ -x "$POP" ]; then
    skip "$POP"
else
    POPTMP=$(mktemp -d)
    curl -sfL -o "$POPTMP/pop.tar.gz" "$POPURL" \
        || { rm -rf "$POPTMP"; die "could not fetch psyq-obj-parser from $POPURL
      Build it from https://github.com/grumpycoders/pcsx-redux (tools/psyq-obj-parser)
      and place the binary at $POP."; }
    mkdir -p tools/psyq-obj-parser
    tar xzf "$POPTMP/pop.tar.gz" -C tools/psyq-obj-parser
    rm -rf "$POPTMP"
    chmod +x "$POP"
    "$POP" -h >/dev/null 2>&1 || die "$POP does not run on this host"
    ok "$POP"
fi
# Idempotent: re-extracts nothing that is already under sdk/work/, and dies
# with the exact archive.org file name if a disc the manifest needs is absent.
# The build cannot link without lib/, so this is not optional.
.venv/bin/python3 tools/psyq_sdk.py install \
    || die "could not produce lib/ from sdk/. See sdk/README.md."
ok "lib/ ($(find lib -name '*.o' | wc -l) objects, verified against retail)"

# ---------------------------------------------------------------------------
step "Extracting asm/ and verifying the build"
make extract >/dev/null || die "make extract failed"
ok "asm/ extracted"

if [ "$VERIFY" = 1 ]; then
    printf '\n      running ./build-and-verify.sh\n'
    ./build-and-verify.sh >/dev/null || die "the executable did not rebuild
      byte-for-byte. Do NOT start matching until this passes: every funcdiff
      score is meaningless against a toolchain that cannot reproduce retail."
    printf '\n\033[32mSetup complete and verified.\033[0m Start with docs/MATCHING-GUIDE.md.\n'
else
    printf '\n\033[33mSetup complete (verification skipped).\033[0m Run ./build-and-verify.sh before matching.\n'
fi
