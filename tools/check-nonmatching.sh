#!/usr/bin/env bash
# Prove the readable bodies kept under `#ifdef NON_MATCHING` still BUILD.
#
#   tools/check-nonmatching.sh            # all units
#   tools/check-nonmatching.sh <unit>...  # just these
#
# What it checks, and what it deliberately does not:
#
#   1. Every src/*.c compiles with -DNON_MATCHING, through the pinned pipeline,
#      into build/nonmatching/ (Makefile target `nonmatching`).
#   2. Every symbol a NON_MATCHING object leaves undefined is defined somewhere
#      in the REAL link (build/lsdde.elf). A body that calls a function under a
#      name that no longer exists compiles fine and could never link; this is
#      the round-33/35 never-linked-body class, caught here mechanically.
#
# It does NOT link, verify or score anything. build/nonmatching/ is not the
# oracle. A NON_MATCHING body is readable documentation of a function the
# verified build still carries as INCLUDE_ASM, and this script only proves the
# documentation is real C. See docs/FINISHING-PLAN.md, track 1b.
#
# Run the real oracle FIRST: the undefined-symbol check reads build/lsdde.elf,
# so a stale or missing verified build makes step 2 lie in both directions.
set -euo pipefail
cd "$(dirname "$0")/.."

NM=tools/binutils/bin/mipsel-linux-gnu-nm
ELF=build/lsdde.elf

if [ ! -f "$ELF" ]; then
    echo "FATAL: $ELF missing. Run ./build-and-verify.sh first." >&2
    exit 1
fi

units=("$@")
if [ ${#units[@]} -eq 0 ]; then
    mapfile -t units < <(find src -name '*.c' | sed 's#^src/##; s#\.c$##' | sort)
else
    # A unit NAME is flat (tools/srcpath.py); its file may sit in a subdirectory.
    for i in "${!units[@]}"; do
        p=$(find src -name "${units[$i]##*/}.c" | head -1)
        [ -n "$p" ] || { echo "FATAL: no unit ${units[$i]} under src/" >&2; exit 1; }
        p=${p#src/}; units[$i]=${p%.c}
    done
fi

targets=()
for u in "${units[@]}"; do
    targets+=("build/nonmatching/src/$u.c.o")
done

echo "== compiling ${#units[@]} unit(s) with -DNON_MATCHING"
if ! make nonmatching -s "${targets[@]}" > build/nonmatching.log 2>&1; then
    echo "COMPILE FAILED under -DNON_MATCHING:" >&2
    grep -nE 'error|parse error|undeclared|conflicting|redefinition|incompatible|\*\*\*' \
        build/nonmatching.log | head -20 >&2
    echo "(full log: build/nonmatching.log)" >&2
    exit 1
fi

# Symbols the verified image defines, any section.
defined=$("$NM" "$ELF" | awk '$2 ~ /^[TtDdBbRrAa]$/ {print $3}' | sort -u)

bad=0
for u in "${units[@]}"; do
    obj="build/nonmatching/src/$u.c.o"
    # Undefined symbols in the NM object that the real link does not define.
    missing=$(comm -23 <("$NM" -u "$obj" | awk '{print $2}' | sort -u) <(printf '%s\n' "$defined"))
    if [ -n "$missing" ]; then
        echo "UNRESOLVED in $u (NON_MATCHING body references a symbol the real link does not define):"
        printf '    %s\n' $missing
        bad=1
    fi
done

# grep exits 1 on zero matches, which under pipefail would end the script
# silently right here; `|| true` keeps "no bodies yet" an ordinary answer.
count=$( { grep -rl --include='*.c' '^#ifdef NON_MATCHING' src || true; } | wc -l)
bodies=$( { grep -rc --include='*.c' '^#ifdef NON_MATCHING' src || true; } | awk -F: '{s+=$2} END{print s+0}')
if [ "$bad" -ne 0 ]; then
    echo "FAIL: $bodies NON_MATCHING body(ies) in $count unit(s) compile, but some cannot link."
    exit 1
fi
echo "OK: $bodies NON_MATCHING body(ies) in $count unit(s) compile and reference only linked symbols."
