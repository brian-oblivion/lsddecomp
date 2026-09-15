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
OPTIONAL="decomp-permuter psyq-obj-parser"

missing=""
[ -e "$MAIN/disk/SLPS_015.56" ] || missing="$missing disk/SLPS_015.56"
[ -e "$MAIN/.venv" ]            || missing="$missing .venv"
# lib/ holds the Sony objects the link needs (splat `o` segments); a worktree
# cannot regenerate them any more than it can build binutils.
[ -e "$MAIN/lib" ]              || missing="$missing lib (run tools/psyq_sdk.py install)"
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
# The mid-round broadcast channel (tools/broadcast.sh) is a file in the MAIN
# checkout; link it so every worktree reads and posts to the same one.
mkdir -p "$MAIN/.round"
ln -s "$MAIN/.round" "$dest/.round"
ln -s "$MAIN/disk/SLPS_015.56" "$dest/disk/SLPS_015.56"
ln -s "$MAIN/.venv" "$dest/.venv"
ln -s "$MAIN/lib" "$dest/lib"
# The SDK discs and everything unpacked from them. sdk/README.md is tracked, so
# sdk/ itself is a real directory in the worktree; link its CONTENTS (the zips
# and the work/ tree with match.txt and the ELF objects). An SDK-object
# conversion runner needs these for `psyq_sdk.py install`, `place`, `runs`
# and `symbols`; without them every one of those silently sees an empty
# corpus (round 29's runner found this and linked them by hand).
for f in "$MAIN"/sdk/*; do
    case "$(basename "$f")" in README.md) continue;; esac
    ln -s "$f" "$dest/sdk/$(basename "$f")"
done
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
