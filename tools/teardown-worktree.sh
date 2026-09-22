#!/usr/bin/env bash
# Tear down a runner worktree, REFUSING unless the preconditions hold.
#
#   tools/teardown-worktree.sh <name>
#
# `git worktree remove --force` is the only form that works here (asm/,
# build/ and lsdde.ld are untracked, so plain remove always refuses), and
# --force disables git's own "are you sure". PARALLEL-RUNS 3.9 lists four
# preconditions to check by hand before it; round 57's head chained the
# command onto a push and removed a worktree while main..runner/bravo still
# held an unmerged commit. Nothing was lost that time. This script is the
# guard that fails instead of relying on the head to look:
#
#   1. the branch has NO commits that main lacks (everything merged);
#   2. the worktree has no uncommitted changes;
#   3. no permuter process, or multiprocessing worker of one, is cwd-ed into the worktree.
#
# The fourth precondition (every touched function has a report) is checked by
# the merge, not here. Pass --i-know for a deliberate discard of a branch you
# have decided to drop; it prints what is being dropped first.
set -euo pipefail
cd "$(dirname "$0")/.."
MAIN=$(pwd)

name=${1:?usage: teardown-worktree.sh <name> [--i-know]}
force=${2:-}
dest="$MAIN/../$(basename "$MAIN")-wt-$name"
branch="runner/$name"

if [ ! -d "$dest" ]; then
    echo "no worktree at $dest"
    git branch -d "$branch" 2>/dev/null && echo "deleted stale branch $branch" || true
    exit 0
fi

fail=0
ahead=$(git log --oneline "main..$branch" 2>/dev/null | wc -l)
if [ "$ahead" -ne 0 ]; then
    echo "REFUSING: $branch has $ahead commit(s) main does not:"
    git log --oneline "main..$branch" | sed 's/^/    /'
    fail=1
fi
dirty=$(git -C "$dest" status --porcelain | wc -l)
if [ "$dirty" -ne 0 ]; then
    echo "REFUSING: $dest has $dirty uncommitted path(s):"
    git -C "$dest" status --porcelain | head -10 | sed 's/^/    /'
    fail=1
fi
live=0
for d in /proc/[0-9]*; do
    cwd=$(readlink "$d/cwd" 2>/dev/null) || continue
    case "$cwd" in "$dest"*)
        # Python 3.14 starts multiprocessing workers through a FORKSERVER, so
        # the permuter's workers run as `python -c "from multiprocessing.
        # forkserver import main; ..."` -- no "permuter" in their cmdline, and
        # they outlive a parent killed by `timeout` (round 65: 11 of them
        # survived a teardown this guard let through).
        if tr '\0' ' ' < "$d/cmdline" 2>/dev/null | grep -qE 'permuter|decomp-permuter|multiprocessing'; then
            echo "REFUSING: live search process ${d#/proc/} in $cwd"; live=1
        fi;;
    esac
done
[ "$live" -ne 0 ] && fail=1

if [ "$fail" -ne 0 ]; then
    if [ "$force" = "--i-know" ]; then
        echo "--i-know given: discarding the above."
    else
        echo "Merge or commit first (PARALLEL-RUNS 3.9), or pass --i-know to discard deliberately."
        exit 1
    fi
fi

git worktree remove --force "$dest"
git branch -D "$branch" >/dev/null 2>&1 || true
echo "removed $dest and $branch"
