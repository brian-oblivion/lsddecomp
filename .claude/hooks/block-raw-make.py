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

`make` only builds when it is in COMMAND POSITION. Matching the bare word
anywhere in the line blocked `command -v make`, `which make`, `grep make
Makefile` and `echo "run make"` -- none of which build anything, all of which
turned a guardrail into a papercut and taught the reader to route around it. So
the line is tokenised and split into simple commands, leading `VAR=x`
assignments and wrappers (`time`, `env`, `sudo`, ...) are stepped over, and only
the resulting head word counts. `sh -c '...'` is recursed into, so quoting is
not a way through.

Each invocation is then judged on ITS OWN targets: `make extract && make` blocks
on the second one, where the old whole-string check saw `make extract` and
allowed the pair.

Unparseable input still fails STRICT -- if the line contains the word at all and
cannot be tokenised, it is blocked rather than waved through.
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

# Cheap pre-filter. Nothing without the bare word can possibly be a make call,
# and this keeps the tokeniser off the overwhelming majority of commands.
MENTIONS_MAKE = re.compile(r"\bmake\b")

# Non-build targets that are the documented way to drive this repo's Makefile.
ALLOWED_TARGETS = frozenset({"extract", "progress", "format", "clean"})

# Commands that run another command, so `make` behind them is still a build.
WRAPPERS = frozenset({
    "time", "env", "nice", "ionice", "nohup", "stdbuf", "timeout",
    "sudo", "doas", "exec", "command", "builtin",
})

# Shells whose -c argument is another command line to analyse.
SHELLS = frozenset({"sh", "bash", "zsh", "dash", "ksh"})

# Tokens that end a simple command, so whatever follows is a fresh head word.
SEPARATORS = frozenset({";", "&&", "||", "|", "&", "(", ")", "{", "}", "|&", "\n"})
KEYWORDS = frozenset({"then", "do", "else", "elif", "if", "while", "until", "!"})

# Flags whose VALUE is the next token, which must not be mistaken for a target.
FLAGS_WITH_VALUE = frozenset({"-C", "-f", "-o", "-W", "-I", "-j", "--directory",
                              "--file", "--makefile", "--jobs"})


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


def tokenize(command: str):
    """Shell-ish tokens, with punctuation split off as its own token.

    Raises ValueError on unbalanced quotes, which the caller treats as strict.
    """
    lexer = shlex.shlex(command, posix=True, punctuation_chars=True)
    lexer.whitespace_split = True
    return list(lexer)


def simple_commands(tokens):
    """Split a token stream into simple commands at shell separators."""
    spans, current = [], []
    for token in tokens:
        if token in SEPARATORS or token in KEYWORDS:
            if current:
                spans.append(current)
            current = []
        else:
            current.append(token)
    if current:
        spans.append(current)
    return spans


def make_invocations(command: str, depth: int = 0):
    """Every `make` in command position, as a list of its argument tokens.

    Returns [] when the line mentions make but never actually runs it.
    """
    if depth > 4:  # A shell nested this deep is not an honest build.
        return [[]]

    found = []
    for span in simple_commands(tokenize(command)):
        # Leading `VAR=value` assignments keep us in command position.
        while span and re.match(r"^[A-Za-z_][A-Za-z0-9_]*=", span[0]):
            span = span[1:]
        if not span:
            continue

        head = os.path.basename(span[0])

        # `command -v make` / `command -V make` describe, they do not run.
        if head in ("command", "builtin") and span[1:2] and span[1] in ("-v", "-V"):
            continue

        if head in WRAPPERS:
            # A wrapper takes its own flags and operands (`nice -n 5`,
            # `timeout 60`, `sudo -u x`), so where it ends is not worth
            # guessing. Scan the rest of the command for the real head
            # instead: over-blocking a contrived `env grep make Makefile` is
            # much cheaper than letting `sudo make` through, and a wrapper is
            # not where the everyday false positives live.
            span = next((span[k:] for k, t in enumerate(span[1:], 1)
                         if os.path.basename(t) == "make"
                         or os.path.basename(t) in SHELLS), [])
            if not span:
                continue
            head = os.path.basename(span[0])

        if head == "make":
            found.append(span[1:])
        elif head in SHELLS and "-c" in span:
            script = span[span.index("-c") + 1:]
            if script:
                found.extend(make_invocations(script[0], depth + 1))

    return found


def targets_of(args):
    """The target words of a make invocation, minus flags and their values."""
    targets = []
    skip = False
    for arg in args:
        if skip:
            skip = False
            continue
        if arg in FLAGS_WITH_VALUE:
            skip = True
            continue
        if arg.startswith("-") or "=" in arg:
            continue
        targets.append(arg)
    return targets


def aimed_outside(args, base: str, root: str) -> bool:
    """`make -C <dir>` or `make -f <makefile>` pointing out of the repo."""
    for flag in ("-C", "-f", "--directory", "--file", "--makefile"):
        for i, token in enumerate(args):
            if token == flag and i + 1 < len(args):
                target = args[i + 1]
                candidate = (target if os.path.isabs(target)
                             else os.path.join(base, target))
                if not inside(candidate, root):
                    return True
    return False


def main() -> int:
    try:
        data = json.load(sys.stdin)
    except (json.JSONDecodeError, ValueError):
        return 0  # Never fail closed on malformed input.

    command = (data.get("tool_input", {}) or {}).get("command", "") or ""
    if not MENTIONS_MAKE.search(command):
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

    try:
        invocations = make_invocations(command)
    except ValueError:
        # Unbalanced quotes. The word is in there somewhere; stay strict.
        print(DENY, file=sys.stderr)
        return 2

    if not invocations:
        return 0  # Mentioned, never run: `command -v make`, `grep make ...`.

    # `cd <path> && ... make ...` -- if it leaves the repo, allow it.
    base = cwd if cwd else root
    for match in re.finditer(r"\bcd\s+(\"[^\"]+\"|'[^']+'|[^\s;&|]+)", command):
        target = match.group(1).strip("\"'")
        candidate = (target if os.path.isabs(target)
                     else os.path.join(base, target))
        if not inside(candidate, root):
            return 0

    # Every invocation must clear the bar; one bare build spoils the line.
    for args in invocations:
        if aimed_outside(args, base, root):
            continue
        targets = targets_of(args)
        if targets and all(t in ALLOWED_TARGETS for t in targets):
            continue
        print(DENY, file=sys.stderr)
        return 2

    return 0


if __name__ == "__main__":
    sys.exit(main())
