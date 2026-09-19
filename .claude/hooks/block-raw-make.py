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
import subprocess
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
# `nonmatching` compiles the #ifdef NON_MATCHING bodies into build/nonmatching/
# and links nothing; funcdiff never reads that tree, so it cannot fake a score.
# Drive it through tools/check-nonmatching.sh.
ALLOWED_TARGETS = frozenset({"extract", "progress", "format", "clean", "nonmatching"})

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

# Redirection operators. These are NOT separators -- `make extract > log` is one
# command, not two -- but the operator and its target are not make TARGETS
# either, and leaving them in the span made the hook block `make extract > log`
# while allowing the identical `make extract | tail`. Round 18 spent a round
# believing that was a per-checkout difference; round 19 measured it here.
#
# `shlex(punctuation_chars=True)` splits a leading file descriptor into its own
# token BEFORE the operator, so `2>&1` arrives as ("2", ">&", "1") -- which is
# why adding these to SEPARATORS does not fix it and stripping does.
REDIRECTS = frozenset({">", ">>", ">|", ">&", "<", "<<", "<<-", "<<<", "<&",
                       "<>", "&>", "&>>"})

# `<<WORD` / `<<-'WORD'` -- the introducer, not the body.
HEREDOC_INTRO = re.compile(r"<<-?\s*(['\"]?)([A-Za-z_][A-Za-z0-9_]*)\1")


def project_dir() -> str:
    return os.path.realpath(
        os.environ.get("CLAUDE_PROJECT_DIR")
        or os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", ".."))


def repo_roots(root: str):
    """The project dir AND every git worktree of it.

    Round 58 measured that this hook never fired in a runner worktree: hooks
    run as $CLAUDE_PROJECT_DIR/.claude/hooks/..., so `root` is always the MAIN
    checkout, a worktree's cwd is "outside" it, and main() returned 0 before
    looking at the target. The old comment here claimed a worktree "is checked
    against ITS repo"; it was not. Every runner had been unguarded. A worktree
    shares this Makefile, so it is this repo for the purpose of the guard.
    """
    roots = [root]
    try:
        out = subprocess.run(["git", "-C", root, "worktree", "list", "--porcelain"],
                             capture_output=True, text=True, timeout=5).stdout
        for line in out.splitlines():
            if line.startswith("worktree "):
                roots.append(os.path.realpath(line[len("worktree "):].strip()))
    except (OSError, subprocess.SubprocessError):
        pass
    return roots


def inside(path: str, root) -> bool:
    """True if `path` is `root` (or any of a list of roots) or below it."""
    roots = root if isinstance(root, (list, tuple)) else [root]
    try:
        real = os.path.realpath(path)
    except OSError:
        return True  # Unresolvable: assume in-repo and stay strict.
    return any(real == r or real.startswith(r + os.sep) for r in roots)


def tokenize(command: str):
    """Shell-ish tokens, with punctuation split off as its own token.

    Raises ValueError on unbalanced quotes, which the caller treats as strict.
    """
    lexer = shlex.shlex(command, posix=True, punctuation_chars=True)
    lexer.whitespace_split = True
    return list(lexer)


def strip_heredoc_bodies(command: str):
    """Split a command into (text_without_heredoc_bodies, [(intro, body), ...]).

    A heredoc body is DATA fed to a command's stdin, not a command line -- but
    the tokeniser cannot tell, so `cat <<'EOF' ... make extract > log ... EOF`
    used to be judged as though the documentation inside it were a build. That
    made the hook block writing prose ABOUT itself, which is the papercut its
    own docstring says it fixed for `grep make Makefile`.

    The bodies are returned rather than discarded because for a SHELL the body
    really is a script (`bash <<'EOF' ... make ... EOF`), and dropping it would
    turn this fix into a bypass. The caller recurses into those.
    """
    lines = command.split("\n")
    kept, bodies, i = [], [], 0
    while i < len(lines):
        line = lines[i]
        kept.append(line)
        i += 1
        match = HEREDOC_INTRO.search(line)
        if not match:
            continue
        delimiter = match.group(2)
        body = []
        while i < len(lines) and lines[i].strip() != delimiter:
            body.append(lines[i])
            i += 1
        if i < len(lines):
            i += 1  # consume the terminator line
        bodies.append((line, "\n".join(body)))
    return "\n".join(kept), bodies


def strip_redirections(tokens):
    """Drop redirection operators, their targets, and any leading fd digit."""
    out, i = [], 0
    while i < len(tokens):
        token = tokens[i]
        if token in REDIRECTS:
            # `2>&1` tokenises as ("2", ">&", "1"): the fd landed in `out`.
            if out and out[-1].isdigit():
                out.pop()
            i += 2  # the operator and the word it redirects to
            continue
        out.append(token)
        i += 1
    return out


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

    command, heredocs = strip_heredoc_bodies(command)

    found = []
    # A heredoc fed to a SHELL is a script; anything else is data. Be
    # conservative about which is which -- if a shell name appears anywhere in
    # the introducing line, treat the body as code.
    for intro, body in heredocs:
        try:
            intro_words = {os.path.basename(t) for t in tokenize(intro)}
        except ValueError:
            intro_words = set()
        if intro_words & SHELLS and MENTIONS_MAKE.search(body):
            found.extend(make_invocations(body, depth + 1))

    for span in simple_commands(strip_redirections(tokenize(command))):
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

    root = repo_roots(project_dir())

    # The Bash tool reports the directory the command runs in. A session
    # already working outside the repo AND outside every worktree of it is
    # not touching our Makefile.
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
    base = cwd if cwd else root[0]
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
