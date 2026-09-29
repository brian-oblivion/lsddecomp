#!/usr/bin/env bash
# THE canonical build entry point. Every build goes through this script.
#
#   1. Verify disk/SLPS_015.56 is the expected dump. check.sha1 is ground
#      truth and must NEVER be edited -- a build that only passes against an
#      edited hash is not a match.
#   2. Build.
#   3. Verify the built executable is byte-for-byte identical to retail.
#
# Step 3 is the ONLY oracle this project has. Nothing else -- not a clean
# compile, not a plausible-looking diff, not a funcdiff score -- is evidence
# that a function matches.
set -euo pipefail
cd "$(dirname "$0")"

if ! sha1sum -c check.sha1 >/dev/null 2>&1; then
    echo "FATAL: disk/SLPS_015.56 missing or does not match check.sha1." >&2
    echo "       See README.md ("Building it") -- bring your own copy of SLPS-01556." >&2
    exit 1
fi

make all

# Name the tree: a runner whose shell resets its cwd between tool calls runs
# ./build-and-verify.sh in the MAIN checkout and reads main's green as its own
# (round 97: four unbuilt commits in a worktree, every one reported byte-exact).
echo "OK: build matches retail SLPS_015.56 (tree: $(pwd))"
