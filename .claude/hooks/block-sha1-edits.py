#!/usr/bin/env python3
"""Block modification of check.sha1 and build.sha1 — the ground-truth hashes.

A build that only passes against an edited hash is not a match, and the edit is
almost invisible in review: one hex string changes and everything goes green.
That makes this the single highest-value guard in the repo.

Write/Edit of either file is always blocked. A Bash command is blocked only
when it writes to, moves or deletes one, so read-only use (`sha1sum -c
check.sha1`, which build-and-verify.sh does on every run) keeps working.
"""
import json
import re
import sys

TARGETS = ("check.sha1", "build.sha1")

# Operations that would modify, replace or remove the file. mv/rm/ln count:
# they destroy or displace the ground truth just as surely as a write does.
WRITE_PATTERNS = [
    r">>?\s*(?:\./)?\S*{t}",                                  # > file, >> file
    r"\b(?:sed|perl|awk)\b[^|;&]*\s-i\b[^|;&]*{t}",
    r"\btee\b[^|;&]*{t}",
    r"\b(?:rm|mv|cp|install|truncate|chmod|chown|ln)\b[^|;&]*{t}",
    r"\bdd\b[^|;&]*\bof=\S*{t}",
    r"\bgit\b[^|;&]*\b(?:checkout|restore|rm)\b[^|;&]*{t}",
]

DENY = (
    "BLOCKED: {t} holds the ground-truth SHA1 of the retail executable and "
    "must never be modified. A build that only passes against an edited hash "
    "is not a match.\n"
    "(Read-only use, e.g. `sha1sum -c check.sha1`, is allowed.)"
)


def main() -> int:
    try:
        data = json.load(sys.stdin)
    except (json.JSONDecodeError, ValueError):
        return 0  # Never fail closed on malformed input.

    tool_input = data.get("tool_input", {}) or {}

    # Write/Edit: any touch of the file is a modification.
    path = tool_input.get("file_path", "") or ""
    for t in TARGETS:
        if t in path:
            print(DENY.format(t=t), file=sys.stderr)
            return 2

    # Bash: block only if the command actually writes to it.
    command = tool_input.get("command", "") or ""
    for t in TARGETS:
        if t not in command:
            continue
        for pattern in WRITE_PATTERNS:
            if re.search(pattern.format(t=re.escape(t)), command):
                print(DENY.format(t=t), file=sys.stderr)
                return 2

    return 0


if __name__ == "__main__":
    sys.exit(main())
