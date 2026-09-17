#!/usr/bin/env python3
"""Table test for block-raw-make.py.

    python3 .claude/hooks/test-block-raw-make.py

The hook is a guardrail, and both of its failure modes are expensive in ways
that are easy to miss. Letting a build through means a funcdiff score gets read
against unverified bytes. Blocking a command that merely says "make" -- which
the original `(?:^|[;&|]|\\s)make(?:\\s|$)` did to `command -v make` and `grep
make Makefile` -- trains the reader to route around the hook, which costs more
than the papercut. So both directions are pinned here.
"""
import json
import os
import subprocess
import sys

HOOKS = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(HOOKS))
HOOK = os.path.join(HOOKS, "block-raw-make.py")

BLOCK = [
    "make",
    "make build",
    "make all",
    "make -j8",
    "make -j 16 build",
    "make unknown-target",
    "make extract build",           # one good target does not excuse the other
    "make \"build\"",
    "/usr/bin/make",
    # Wrappers, which take their own flags and operands.
    "time make",
    "env FOO=1 make",
    "FOO=1 make",
    "FOO=1 BAR=2 make build",
    "sudo make install",
    "nice -n 5 make",
    "timeout 60 make",
    "sudo -u builder make",
    # Every invocation on the line has to clear the bar.
    "make extract && make",
    "make extract; make build",
    "./build-and-verify.sh && make",
    # Quoting is not a way through.
    "bash -c 'make'",
    "bash -c 'make build'",
    "sh -c \"make\"",
    "sudo bash -c 'make build'",
    "make 'unbalanced",             # unparseable -> strict
    # Still this repo's Makefile.
    "cd src && make",
    "make -C .",
    "make -C src",
    "make -f Makefile",
    # Shell structure does not hide the head word.
    "echo hi | make",
    "if true; then make; fi",
    "for f in a b; do make; done",
]

ALLOW = [
    # Mentions, not invocations. These are the regression this file exists for.
    "command -v make",
    "command -V make",
    "which make",
    "type make",
    "grep make Makefile",
    "grep -n 'make' CLAUDE.md",
    "echo \"run make\"",
    "echo 'make build'",
    "man make",
    "ls | grep make",
    "for t in python3 git make; do command -v $t; done",
    "git log --oneline | grep make",
    "rg 'make extract' docs/",
    # The documented non-build targets.
    "make extract",
    "make progress",
    "make nonmatching",
    "make clean",
    "make format",
    "make clean extract",
    "make extract && make progress",
    # Out-of-repo and container builds have to keep working.
    "make -C /tmp/binutils-build",
    "make -f /tmp/other/Makefile",
    "cd /tmp/build && make",
    "cd /tmp/build && make -j16",
    "docker run img make",
    "podman run img make all",
    # Nothing to do with make.
    "ls -la",
    "./build-and-verify.sh",
    "python3 tools/progress.py",
]


def run(command, cwd=ROOT):
    payload = json.dumps({"tool_input": {"command": command}, "cwd": cwd})
    proc = subprocess.run([sys.executable, HOOK], input=payload,
                          capture_output=True, text=True)
    return proc.returncode


def main() -> int:
    fails = []
    for command in BLOCK:
        if run(command) != 2:
            fails.append(("SHOULD BLOCK, allowed", command))
    for command in ALLOW:
        if run(command) != 0:
            fails.append(("SHOULD ALLOW, blocked", command))

    # A session already working outside the repo is not touching our Makefile.
    if run("make", cwd="/tmp") != 0:
        fails.append(("SHOULD ALLOW, blocked", "make (cwd=/tmp)"))

    total = len(BLOCK) + len(ALLOW) + 1
    for why, command in fails:
        print(f"  FAIL  {why}: {command!r}")
    print(f"{total - len(fails)}/{total} passed")
    return 1 if fails else 0


if __name__ == "__main__":
    sys.exit(main())
