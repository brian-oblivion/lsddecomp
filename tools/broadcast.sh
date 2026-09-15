#!/usr/bin/env bash
# The mid-round broadcast channel, as a FILE. SendMessage is not exposed in
# the desktop-app build this project runs under (rounds 31-33 and 43 found it
# missing), so a lever found by one runner could not reach the others and the
# head could not warn a runner mid-search. This replaces the tool with a
# shared, gitignored notes file that every worktree sees through a symlink.
#
#   tools/broadcast.sh post [--from <who>] "<message>"   append a dated entry
#   tools/broadcast.sh read                              print every entry
#   tools/broadcast.sh clear                             head, at round start
#
# Runners: `read` before starting each function, `post --from <name>` the
# moment you have a lever, a negative ("does not apply to my unit"), or a
# question for the head. Head: `post` anything the runners should act on,
# `read` at every triage. Entries are append-only and dated; nobody edits.
#
# The file is $MAIN/.round/BROADCAST.md. tools/setup-worktree.sh symlinks
# $MAIN/.round into each worktree, so the same path works everywhere.
set -euo pipefail
cd "$(dirname "$0")/.."
dir=.round
file=$dir/BROADCAST.md
if [ -L "$dir" ] || [ -d "$dir" ]; then :; else mkdir -p "$dir"; fi

case "${1:-}" in
    post)
        shift
        from=head
        if [ "${1:-}" = "--from" ]; then from=${2:?--from needs a name}; shift 2; fi
        msg=${1:?usage: broadcast.sh post [--from <who>] "<message>"}
        printf '\n## %s  from %s\n\n%s\n' "$(date '+%Y-%m-%d %H:%M')" "$from" "$msg" >> "$file"
        echo "posted to $file"
        ;;
    read)
        if [ -s "$file" ]; then cat "$file"; else echo "(no broadcasts yet)"; fi
        ;;
    clear)
        : > "$file"
        echo "cleared $file"
        ;;
    *)
        sed -n '2,17p' "$0"; exit 1
        ;;
esac
