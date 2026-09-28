#!/usr/bin/env bash
# Disc-free lint (FINISHING-PLAN track 13): the source checks CI can run with
# the repo and the toolchain from tools/setup.sh, and nothing else. It needs no
# disk/SLPS_015.56, sdk/, lib/, asm/ or build/. It never rewrites a file.
#
#   tools/lint.sh            run every check; exit 0 when all pass, 1 otherwise
#
# The byte-exact build is not here: it needs the disc, and it is
# ./build-and-verify.sh's job.
set -u -o pipefail
cd "$(dirname "$0")/.." || exit 2

PY=python3
VENV_PY=.venv/bin/python3
failed=()

# check NAME CMD...: run CMD, show its output only when it fails
check() {
    local name=$1 out rc
    shift
    out=$("$@" 2>&1)
    rc=$?
    if [ "$rc" -eq 0 ]; then
        echo "ok    $name"
    else
        echo "FAIL  $name"
        printf '%s\n' "$out" | sed 's/^/      /'
        failed+=("$name")
    fi
}

# The file set `make format` formats, read from the Makefile's own recipe so
# the two can never drift (the `$$(...)` on its `clang-format -i` line).
format_files() {
    local cmd
    cmd=$(sed -n 's/^\tclang-format -i \$\$(\(.*\))$/\1/p' Makefile | sed 's/\$\$/$/g')
    if [ -z "$cmd" ]; then
        echo "lint: no \`clang-format -i \$\$(...)\` line in the Makefile's format target" >&2
        return 1
    fi
    eval "$cmd"
}

lint_format() {
    local want have files out
    command -v clang-format >/dev/null || { echo "clang-format not installed"; return 1; }
    # .clang-format names the major it was written for; another major formats
    # differently, so its verdict would not be the house style's
    want=$(sed -n 's/.*Written for clang-format \([0-9][0-9]*\).*/\1/p' .clang-format | head -1)
    have=$(clang-format --version | sed -n 's/.*clang-format version \([0-9][0-9]*\).*/\1/p')
    if [ -n "$want" ] && [ "$want" != "$have" ]; then
        echo "clang-format major $have; .clang-format is written for $want"
        return 1
    fi
    files=$(format_files) || return 1
    # shellcheck disable=SC2086
    if ! out=$(clang-format --dry-run --Werror $files 2>&1); then
        # one line per violation (file:line:col), not the quoted source under it
        printf '%s\n' "$out" | grep -E '^[^ ].*: (error|warning):' || printf '%s\n' "$out"
        echo "(fix with: make format)"
        return 1
    fi
    # clang-format cannot fix a tab inside a comment, so `make format` greps for
    # any tab after formatting; the lint does the same over the same files
    # shellcheck disable=SC2086
    grep -nP '\t' $files && { echo "tab characters above (clang-format cannot fix tabs inside comments)"; return 1; }
    return 0
}

# readability.py always exits 0; gate the counters that are both finished (a
# done track holds them at 0) and disc-free (the same value without
# build/lsdde.elf). D_, m2c, magic and rawoff change without the ELF, which
# decides which bodies are game code (without it, library bodies count too),
# so they are not gated here; func_, unk, slot, magic and rawoff are a done
# track's accepted residue, not a failure.
lint_readability() {
    "$PY" tools/readability.py --json | "$PY" -c '
import json, sys
t = json.load(sys.stdin)["totals"]
gated = {
    "history": "project history in .c comments (track 9)",
    "header_history": "project history in header comments (track 9)",
    "placeholder_units": "units named code_/class_<hex> (track 8)",
    "placeholder_headers": "headers named code_/class_<hex> (track 8)",
    "placeholder_types": "placeholder type names (track 6)",
    "ph_prefix": "definitions under a placeholder class prefix (track 6)",
    "upper_globals": "game globals named UPPER_SNAKE (readability.py --globals)",
}
bad = [f"{k} = {t[k]}: {why}" for k, why in gated.items() if t.get(k) != 0]
print("\n".join(bad))
sys.exit(1 if bad else 0)
'
}

# declcheck.py's exit status ignores DELIBERATE (mismatches kept on purpose);
# on a failure, list the hits but not those
lint_decl() {
    local out rc
    out=$("$VENV_PY" tools/declcheck.py --verbose 2>&1)
    rc=$?
    [ "$rc" -eq 0 ] && return 0
    printf '%s\n' "$out" | awk '/^== DELIBERATE/ { skip = 1; next } /^== / { skip = 0 } !skip'
    return "$rc"
}

# unitfile.py check always exits 0; its last line carries the count
lint_snake() {
    local out
    out=$("$PY" tools/unitfile.py check) || { printf '%s\n' "$out"; return 1; }
    printf '%s\n' "$out" | grep -q '^0 game file(s) not snake_case' && return 0
    printf '%s\n' "$out"
    return 1
}

check "format (clang-format --dry-run over make format's files, no tabs)" lint_format
check "apidoc (python3 tools/apidoc.py -v)" "$PY" tools/apidoc.py -v
check "readability (python3 tools/readability.py --json, finished counters)" lint_readability
check "declcheck (.venv/bin/python3 tools/declcheck.py --verbose)" lint_decl
check "snake_case (python3 tools/unitfile.py check)" lint_snake

if [ ${#failed[@]} -eq 0 ]; then
    echo "lint: all checks passed"
    exit 0
fi
echo "lint: ${#failed[@]} check(s) failed:"
printf '  %s\n' "${failed[@]}"
exit 1
