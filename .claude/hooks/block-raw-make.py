#!/usr/bin/env python3
"""Force builds through ./build-and-verify.sh, the canonical build + verify.

`make build` produces an executable and says nothing about whether it matches.
Reading a funcdiff score after a bare `make` is how a session convinces itself
it matched something it did not. build-and-verify.sh checks the source dump
first and the result afterwards, and it is the project's only oracle.

Only THIS repo's Makefile is off limits. A `make` aimed elsewhere is allowed:
`make extract` (the documented way to regenerate the split), anything inside a
container, `-C`/`-f` pointing outside the repo, a `cd` out of the repo first,
or a cwd already outside it. Building third-party software -- binutils during
setup, for instance -- has to keep working.
"""
import json
import os
import re
import shlex
import sys

DENY = (
    "BLOCKED: run builds via ./build-and-verify.sh (the canonical build + "
    "verification). A bare `make` produces bytes without checking them, and "
    "every funcdiff score read afterwards is unanchored.\n"
    "`make extract` is allowed for regenerating the split. Builds of "
    "third-party software outside this repo, or inside docker, are allowed -- "
    "point make at another directory with -C/-f, or cd there first."
)

MAKE_RE = re.compile(r"(?:^|[;&|]|\s)make(?:\s|$)")


def project_dir() -> str:
    return os.path.realpath(
        os.environ.get("CLAUDE_PROJECT_DIR")
        or os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", ".."))


def inside(path: str, root: str) -> bool:
    """True if `path` is `root` or below it."""
    try:
        real = os.path.realpath(path)
    except OSError:
        return True  # Unresolvable: assume in-repo and stay strict.
    return real == root or real.startswith(root + os.sep)


def main() -> int:
    try:
        data = json.load(sys.stdin)
    except (json.JSONDecodeError, ValueError):
        return 0  # Never fail closed on malformed input.

    command = (data.get("tool_input", {}) or {}).get("command", "") or ""
    if not MAKE_RE.search(command):
        return 0

    root = project_dir()

    # The Bash tool reports the directory the command runs in. A session
    # already working outside the repo is not touching our Makefile.
    #
    # NOTE this deliberately does NOT special-case worktrees: a worktree is a
    # separate directory with its own root, so a runner's `make` there is
    # checked against ITS repo, which is what we want.
    cwd = data.get("cwd") or ""
    if cwd and not inside(cwd, root):
        return 0

    # Anything executed inside a container is building its own filesystem.
    if re.search(r"\bdocker\b|\bpodman\b", command):
        return 0

    # `cd <path> && ... make ...` -- if it leaves the repo, allow it.
    for match in re.finditer(r"\bcd\s+(\"[^\"]+\"|'[^']+'|[^\s;&|]+)", command):
        target = match.group(1).strip("\"'")
        base = cwd if cwd else root
        candidate = target if os.path.isabs(target) else os.path.join(base, target)
        if not inside(candidate, root):
            return 0

    # `make -C <dir>` or `make -f <makefile>` aimed outside the repo.
    try:
        tokens = shlex.split(command)
    except ValueError:
        tokens = command.split()
    for flag in ("-C", "-f"):
        for i, token in enumerate(tokens):
            if token == flag and i + 1 < len(tokens):
                target = tokens[i + 1]
                base = cwd if cwd else root
                candidate = (target if os.path.isabs(target)
                             else os.path.join(base, target))
                if not inside(candidate, root):
                    return 0

    # In-repo make: allow only the documented non-build targets.
    if re.search(r"\bmake\s+(extract|progress|format|clean)\b", command):
        return 0

    print(DENY, file=sys.stderr)
    return 2


if __name__ == "__main__":
    sys.exit(main())
