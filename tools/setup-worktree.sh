#!/usr/bin/env bash
# Create a git worktree ready for an independent matching session.
#
#   tools/setup-worktree.sh <name> [branch]
#
# Creates ../<checkout>-wt-<name> on branch runner/<name> -- <checkout> is the
# basename of the MAIN checkout, derived at run time, not a fixed string (the
# script's own output prints the real path; trust that, not this comment).
# Symlinks the
# gitignored essentials from the main checkout (the executable, the venv, the
# toolchain), extracts asm/, and PROVES the worktree byte-verifies before
# handing it over.
#
# WHY A WORKTREE AND NOT JUST ANOTHER CLONE OR ANOTHER SHELL. Two sessions in
# one checkout share `build/`. Concurrent `build-and-verify.sh` runs interleave
# objects and can report a FALSE MISMATCH -- which poisons the only oracle this
# project has, and does it silently. A worktree gives each session its own
# working tree, its own build directory and its own branch, while sharing
# history so the head can merge normally.
set -euo pipefail
cd "$(dirname "$0")/.."
MAIN=$(pwd)

name=${1:?usage: setup-worktree.sh <name> [branch]}
branch=${2:-runner/$name}
dest="$MAIN/../$(basename "$MAIN")-wt-$name"

if [ -e "$dest" ]; then
    echo "FATAL: $dest already exists" >&2
    exit 1
fi

# REQUIRED is checked BEFORE creating anything. A worktree only symlinks the
# toolchain -- it cannot build one -- so an incomplete main checkout has to
# fail here, loudly, rather than as a confusing build error inside a runner
# session an hour later.
REQUIRED="binutils gcc263 maspsx asm-differ m2c"
OPTIONAL="decomp-permuter"

missing=""
[ -e "$MAIN/disk/SLPS_015.56" ] || missing="$missing disk/SLPS_015.56"
[ -e "$MAIN/.venv" ]            || missing="$missing .venv"
for t in $REQUIRED; do
    [ -e "$MAIN/tools/$t" ] || missing="$missing tools/$t"
done
if [ -n "$missing" ]; then
    echo "FATAL: the main checkout at $MAIN is missing:$missing" >&2
    echo "       A worktree only SYMLINKS the toolchain; it cannot build it." >&2
    echo "       Run ./tools/setup.sh in the main checkout first." >&2
    exit 1
fi

git worktree add -b "$branch" "$dest" main

mkdir -p "$dest/disk"
ln -s "$MAIN/disk/SLPS_015.56" "$dest/disk/SLPS_015.56"
ln -s "$MAIN/.venv" "$dest/.venv"
for t in $REQUIRED $OPTIONAL; do
    src="$MAIN/tools/$t"
    [ -e "$src" ] || continue          # only reachable for OPTIONAL entries
    ln -s "$src" "$dest/tools/$t"
done

cd "$dest"
sha1sum -c check.sha1
make extract > /dev/null
if ./build-and-verify.sh > /dev/null; then
    echo "OK: worktree $dest verified"
else
    echo "FATAL: the worktree build does not verify. Do NOT start a runner" >&2
    echo "       here -- every score it produces would be meaningless." >&2
    exit 1
fi

cat <<DONE

Worktree ready: $dest  (branch: $branch)
Start a runner session there with the prompt in docs/PARALLEL-RUNS.md.

Tear down only after the four preconditions in PARALLEL-RUNS.md 4b.
--force is REQUIRED, not optional: asm/, build/ and lsdde.ld are generated and
untracked, and plain \`git worktree remove\` refuses while they exist. Check the
preconditions yourself; the flag suppresses the only automatic guard.
  git worktree remove --force $dest && git branch -d $branch
DONE
