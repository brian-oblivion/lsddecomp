#!/usr/bin/env python3
"""The finishing plan, measured: which tracks are open, what is ready to run,
and on which model.

    python3 tools/plan.py                      # status + ready jobs
    python3 tools/plan.py --json
    python3 tools/plan.py jobs [--n 12]        # the ready-jobs list only
    python3 tools/plan.py units                # per-unit readability table

    python3 tools/plan.py record-round --track 1 --round N --model sonnet|opus \\
                          --runners R --attempts A --matches M [--note "..."] [--not-calibration]
    python3 tools/plan.py amend-round --track T --round N [--note ...] [--attempts A] --why "..."
    python3 tools/plan.py mark-unit --unit <unit> [--undo]        # track 3 pass done
    python3 tools/plan.py mark-class --table <sym> --class <Name> [--park "reason"] [--undo]  # track 4 class unified
    python3 tools/plan.py classes              # track 4: every class, its state and its views
    python3 tools/plan.py set-track --track N --status open|parked|done --reason "..."
    python3 tools/plan.py check --item <id> [--undo]              # checklists: track 5, 6/7 setup, track 9
    python3 tools/plan.py set-model --role naming_runner|match_runner|polish_runner --model sonnet|opus
    python3 tools/plan.py mark-unit --unit <unit> --track 7 [--undo]   # track 7 polish pass done
    python3 tools/plan.py flag-type --name <Type> --reason "..." [--after f1,f2] [--units u1,u2] [--undo]   # track 6: a name the patterns miss
    python3 tools/plan.py park-type --name <Type> --reason "..." [--undo]   # track 6: kept, with the reason
    python3 tools/plan.py regions              # track 8: every region of game units, evidence and state

This is the companion to docs/FINISHING-PLAN.md the way progress.py is the
companion to CLAUDE.md: the DOC holds the rules and quotes no counts; this
TOOL derives every count from the tree. The one thing it stores is
config/plan-state.json, a ledger of DECISIONS and EVENTS that the tree cannot
express: calibration-round results, "this unit's naming pass is done", a
track parked with a reason, a checklist item ticked. Those are not counts
that rot; they are facts about what the head decided, and this tool applies
the plan's rules to them.

Everything else is measured:
  - queue and matched counts from progress.py (--json);
  - the ranked stall list from nearmiss.py (its ASSIGN FROM HERE block);
  - readability debt per unit from the LIVE code in src/ (dead code stripped
    exactly as progress.py strips it): definitions still named func_, raw
    unk fields, slotNN vtable calls, raw D_ globals, NON_MATCHING bodies;
  - the SDK call surface: functions game code calls whose address lies in a
    Psy-Q segment, split into named and still func_;
  - doc word budgets, so the sprawl this plan replaced does not grow back.
"""
import argparse
import datetime
import json
import os
import re
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import progress  # noqa: E402
import srcpath   # noqa: E402

ROOT = Path(__file__).resolve().parent.parent
STATE = ROOT / "config/plan-state.json"
REPORTS = ROOT / "docs/match-reports"

# --- the model table (mirrors FINISHING-PLAN.md; the doc is the authority) ---
# "premium" is a ROLE, not a model: the strongest model available, for a head
# that writes procedure and for the between-round plan session. Revision 14
# (2026-09-24): Opus 5.5, replacing Fable 5.1. A later swap edits this line.
PREMIUM = "opus-5.5"
MODELS = {
    "head": "opus",
    "head_when_new_procedure": f"premium ({PREMIUM})",
    "match_runner_default": "sonnet",
    "naming_runner_default": "opus",
    "mechanical_runner": "sonnet",
    "types_runner": "opus",
    "polish_runner_default": "opus",
    "files_runner": "opus",
}

# WORD budgets. A doc over budget is a warning here and a job in track 5.
# Words, not lines, since revision 10: round 64 met the LEARNINGS line budget
# by reflowing to a wider column with the text word-for-word identical, so a
# line count no longer measured how much a newcomer has to read. Each figure
# is the old line budget converted at that doc's own words-per-line on
# 2026-09-22 (LEARNINGS at its pre-reflow ratio), so the slack each doc had is
# unchanged and only the reflow loophole closed.
DOC_BUDGETS = {
    "CLAUDE.md": 7300,
    "docs/FINISHING-PLAN.md": 5600,
    "docs/PARALLEL-RUNS.md": 4300,
    "docs/DECOMPILATION_LEARNINGS.md": 9800,
    "docs/MATCHING-GUIDE.md": 6100,
    "docs/SDK-OBJECTS-GUIDE.md": 3500,
    "docs/SDK-OBJECTS-RUNS.md": 3100,
}

TRACK5_ITEMS = {
    "readme": "reader-facing README.md: what the game's code is, how it is organised, how to build",
    "credits": "CREDITS.md current for every inherited name and tool",
    "asm-sites": "every live __asm__ site justified or retired (the bare barriers, the GTE blocks)",
    "docs-budget": "every doc within its word budget; archive holds the history",
    "nonmatching-clean": "tools/check-nonmatching.sh green and every stall has a NON_MATCHING body or a written reason",
}

# Phase 2 (plan revision 27, 2026-09-26): the code reads like a game's source.
# One-time setup items a PREMIUM head does before its track staffs runners
# (operator decisions of 2026-09-26), then the close-out.
TRACK6_SETUP = {
    "sdk-headers": "include/psyq/*.h normalized to LF and include/types.h reconciled, so game code can "
                   "#include <libgte.h>/<libgs.h>/<libgpu.h>; proven byte-identical (docs/research/psyq-header-crlf-blocker.md)",
}
TRACK7_SETUP = {
    "format": ".clang-format checked in matching the house style, every src/include file formatted once "
              "between rounds (byte-identical), `make format` then `git diff --exit-code` clean",
}
TRACK9_ITEMS = {
    "layout": "src/ grouped into subsystem directories; README's code map says which directory holds what",
    "readme": "README.md describes the code by its final names and layout, no counts",
    "comments": "no project history in src/include comments (readability.py history 0, headers too)",
    "globals": "no game global named UPPER_SNAKE (readability.py --globals empty): `rename.py` each to "
               "gName, or sName when one file references it",
    "style": "`make format` leaves no diff",
    "docs-budget": "every doc within its word budget",
    "nonmatching-clean": "tools/check-nonmatching.sh green",
}
# Phases 3 and 4 (plan revisions 41 and 42, 2026-09-28): the tree is ready to
# publish, then its C is ready for a Linux port (the port is its own repo).
# Each track is an ordered checklist; an item is (done-when, model, after).
# model: "opus"/"sonnet" is a runner job, "premium" a head setup item, None
# an operator decision the head records. An item is ready once every item in
# `after` is ticked; a track opens when the one before it is done.
_DEBT = ("second pass on readability.py debt in {}: unk/slot/rawoff/m2c/func_/D_ named where the "
         "accessors show what they are, literals named only when they mean more than their value; the "
         "area's duplicate types merged into one and its leftovers done (release review, debt passes); "
         "tools/declcheck.py still clean")
# Track 12's items are tools/apidoc.py's areas: each owns a set of .c files
# AND the headers they define, so a banner moved into its class header or a
# header's matching note moved into its .c never leaves the item (revision 45).
import apidoc  # noqa: E402
_API = ("`python3 tools/apidoc.py --item {}` clean: its headers documented per track 12 (@file; class, "
        "struct and field docs; @brief, @param, @return per prototype), its .c comments explaining the code "
        "with long banners split into class and function docs, and no process text left in either")
PHASE3 = {
    "10": ("declarations and conventions", {
        "prototypes": ("every function and global declared once, in the defining file's header or Sony's "
                       "(kernel.h for the BIOS and card calls: no disc ships a libapi.h), with the "
                       "definition's types; no unit re-declares a header's name (tools/declcheck.py clean)",
                       "opus", ()),
        "conventions": ("Get<Class>Methods not Get_vtable_, self not this, guards <NAME>_H, no s-prefixed "
                        "extern in a header, the review's misleading names and typos renamed", "opus", ()),
        "sony-code": ("src/psyq/ on Sony's names (libcd's CD_* globals, _SpuInit, _ss_MarkCallback) and "
                      "Sony's headers; its #if 0/#if 1 blocks in the NON_MATCHING form or gone", "opus", ()),
        "tabs": ("no tab in src/ or include/ outside .inc files, and `make format` fails on one",
                 "sonnet", ()),
        "debt-app-cd": (_DEBT.format("src/app and src/cd"), "opus", ("prototypes", "conventions")),
        "debt-graphics": (_DEBT.format("src/graphics"), "opus", ("prototypes", "conventions")),
        "debt-world": (_DEBT.format("src/world"), "opus", ("prototypes", "conventions")),
        "debt-ui-sound": (_DEBT.format("src/ui, src/sound and src/psyq"), "opus", ("prototypes", "conventions")),
    }),
    "11": ("file names", {
        "files-setup": ("unitfile.py renames a type-named file's stem (paths only, no token rewrite) and a "
                        "header with no unit; proven byte-identical on one of each", "premium", ()),
        "file-names": ("every game file in src/ and include/ snake_case and named for what it holds, no "
                       "concatenated names; README's code map follows", "opus", ("files-setup",)),
    }),
    "12": ("documented API", {
        "apidoc-setup": ("tools/apidoc.py (per header: undocumented prototypes, missing @param, process-text "
                         "hits in include/ and src/ comments) and a Doxyfile", "premium", ()),
        **{i: (_API.format(i), "opus", ("apidoc-setup",)) for i in apidoc.AREAS},
    }),
    "13": ("publish", {
        "readme": ("README: what the code is, the layout, how to build, verify and change it while "
                   "keeping it matching", "opus", ()),
        "lint": ("one disc-free lint (make format check, apidoc.py, readability.py) runnable as CI",
                 "opus", ()),
        "licence": ("the operator's licence decision: LICENSE and README's Licence section", None, ()),
        "process-docs": ("the operator's decision on docs/, CLAUDE.md and one-off tools, applied (docs/ to the archive/process branch)", None,
                         ("readme",)),
    }),
}
# Phase 4 (docs/CLEANUP.md): the code reads like the game's source, not like
# a decompilation. The six tracks are independent and open together once
# track 13 is done; each is an ordered checklist like phase 3's.
_AREAS = (("world", "src/world"), ("graphics", "src/graphics"), ("rest", "src/app, src/cd, src/sound and src/ui"))
PHASE4 = {
    "14": ("data in C", {
        "data-setup": ("one .data table defined in C instead of extern'd from the disassembly, byte-identical, "
                       "and the procedure written into docs/CLEANUP.md (splat re-cut, section order, labels "
                       "that split a table into columns); sEntityMoodTable is the first", "premium", ()),
        **{f"data-{a}": (f"{d}'s tables defined in C where they are the file's own data; an extern stays only "
                         "for data another file defines", "opus", ("data-setup",)) for a, d in _AREAS},
    }),
    "15": ("named literals, a pass", {
        **{f"literals-{a}": (f"{d}: literals named where the name says more than the value (per-script states "
                             "as local enums, flags, ids, sizes that recur); an enum used where one exists; a "
                             "motion amount or timer value stays a number", "opus", ()) for a, d in _AREAS},
    }),
    "16": (".c comments", {
        **{f"comments-{a}": (f"{d}: .c comments say what the code does in the game's terms, as the headers do; "
                             "a MATCHING: line only where the spelling is not obviously the natural one, and "
                             "phrased without registers or tools", "opus", ()) for a, d in _AREAS},
    }),
    "17": ("code bent to match", {
        "ub-calls": ("the calls that rely on leftover argument registers and the functions that fall off "
                     "their end: natural C if it matches, else the readable body in #ifdef NON_MATCHING",
                     "opus", ()),
        **{f"shape-{a}": (f"{d}: each goto, scheduling barrier, box struct and odd loop counter rewritten as "
                          "natural C where that is byte-identical; the rest keep one MATCHING: line", "opus",
                          ("ub-calls",)) for a, d in _AREAS},
    }),
    "18": ("behaviour names", {
        "mood-cues": ("the Entity__MoodCueNN handlers named for what they do (a MoodCue row and its handler "
                      "are the entity's script)", "opus", ()),
        "noop-slots": ("the *__NoOpSlotNN occupants and remaining slotNN fields named for the slot's role, "
                       "read from the callers and the classes that override it", "opus", ()),
    }),
    "19": ("one class, one file", {
        "split-setup": ("a multi-class unit split into one file per class in ROM order, byte-identical, and "
                        "the procedure written into docs/CLEANUP.md", "premium", ()),
        "split-dream-scene": ("src/world/dream_scene.c split per class (ItemList's half, ObjM, the style "
                              "layer, StyleEffect, Actor, VariantSprite, GraphRoom); file-private #defines at "
                              "the top of each new file", "opus", ("split-setup",)),
        "split-rest": ("every other unit holding more than one class split the same way where the rodata "
                       "boundaries allow (tools/tuboundary.py)", "opus", ("split-setup",)),
    }),
}
PHASE3.update(PHASE4)
PHASE3_ITEMS = {k: {i: v[0] for i, v in items.items()} for k, (_, items) in PHASE3.items()}
CHECK_ITEMS = {"5": TRACK5_ITEMS, "6": TRACK6_SETUP, "7": TRACK7_SETUP, "9": TRACK9_ITEMS, **PHASE3_ITEMS}
TABLE_NAME = re.compile(r"^g[A-Z]\w*Methods$")

DEFAULT_STATE = {
    "stop_rule": {"attempts_per_model": 6, "min_matches": 3},
    "models": {"match_runner": None, "naming_runner": None},
    "tracks": {
        "1": {"status": "open", "reason": "", "rounds": []},
        "1b": {"status": "auto", "reason": ""},
        "2": {"status": "open", "reason": ""},
        "3": {"status": "open", "reason": "", "units_done": {}},
        "4": {"status": "auto", "reason": "", "classes": {}},
        "5": {"status": "auto", "reason": "", "checklist": {}},
        "6": {"status": "auto", "reason": "", "checklist": {}, "flagged": {}, "parked": {}, "after": {}},
        "7": {"status": "auto", "reason": "", "checklist": {}, "units_done": {}},
        "8": {"status": "auto", "reason": "", "parked": {}},
        "9": {"status": "auto", "reason": "", "checklist": {}},
        **{k: {"status": "auto", "reason": "", "checklist": {}} for k in PHASE3},
    },
}

# A splat placeholder, or the tier-C method form `Class__func_xxxxx` that keeps
# the address in the name: both count as NOT YET NAMED (FINISHING-PLAN track 3).
FUNC_PH = re.compile(r"^(?:\w+__)?func_(?:800)?[0-9A-Fa-f]{5}$")
DEF_RE = progress.DEF_RE
NOT_DEF = {"if", "while", "for", "switch", "do", "return", "sizeof"}


def defaultdict_count(it):
    out = {}
    for x in it:
        out[x] = out.get(x, 0) + 1
    return out


def phase3_status(tr, prev):
    """Each phase-3 track opens when the one before it is done and, once an
    item is ticked, stays open beside a reopened earlier track (as track 9)."""
    out = {}
    for k, (title, items) in PHASE3.items():
        tk = tr[k]["checklist"]
        if k in PHASE4:
            prev = out["13"]["status"]  # phase 4's tracks open together
        if tr[k]["status"] != "auto":
            s = tr[k]["status"]
        elif prev != "done" and not any(tk.values()):
            s = f"waiting (opens when track {int(k) - 1} is done)"
        else:
            s = "done" if all(tk.get(i) for i in items) else "open"
        out[k] = {"status": s, "title": title, "checklist": {i: bool(tk.get(i)) for i in items}}
        prev = s
    return out


def load_state():
    if STATE.exists():
        st = json.loads(STATE.read_text())
    else:
        st = json.loads(json.dumps(DEFAULT_STATE))
    # fill in any key a newer version of this tool added
    for k, v in DEFAULT_STATE.items():
        st.setdefault(k, v)
    for t, v in json.loads(json.dumps(DEFAULT_STATE["tracks"])).items():
        st["tracks"].setdefault(t, v)
        for key, val in v.items():
            st["tracks"][t].setdefault(key, val)
    return st


def save_state(st):
    STATE.write_text(json.dumps(st, indent=2, sort_keys=True) + "\n")


# The ledger names things: track 3/7 units, track 4 tables, classes and
# headers, track 6 flagged/parked types. A rename that leaves it behind turns
# a unified class into a class with STRAY VIEWS and a passed unit into an
# unpassed one. rename.py, renametype.py and unitfile.py call this; narrative
# fields (a round's note, a reason) are history and are not rewritten.
LEDGER_NARRATIVE = {"note", "reason", "rounds", "amended", "why", "was"}


def ledger_rename(mapping, drop_duplicates=False):
    """Rewrite whole tokens OLD -> NEW in the ledger's keys and name values.
    With drop_duplicates (a unit merge), a key that now collides keeps the
    surviving unit's entry."""
    if not STATE.exists() or not mapping:
        return
    rxs = [(re.compile(rf"(?<![A-Za-z0-9_]){re.escape(o)}(?![A-Za-z0-9_])"), n)
           for o, n in sorted(mapping.items(), key=lambda kv: -len(kv[0]))]

    def fix(x):
        for rx, n in rxs:
            x = rx.sub(n, x)
        return x

    def walk(x):
        if isinstance(x, dict):
            out = {}
            for k, v in x.items():
                nk = k if k in LEDGER_NARRATIVE else fix(k)
                nv = v if k in LEDGER_NARRATIVE else walk(v)
                if nk in out and drop_duplicates:
                    continue
                out[nk] = nv
            return out
        if isinstance(x, list):
            return [walk(v) for v in x]
        return fix(x) if isinstance(x, str) else x
    st = json.loads(STATE.read_text())
    new = walk(st)
    if new != st:
        save_state(new)


def ledger_rename_unit(old, new):
    """A paths-only unit rename (unitfile.py, track 11): the unit's own keys
    move (`units_done`, track 8's `parked`) while a class of the same name,
    and every other value, stays as it is."""
    if not STATE.exists() or old == new:
        return
    st = json.loads(STATE.read_text())
    changed = False
    for tr in st["tracks"].values():
        for key in ("units_done", "parked"):
            d = tr.get(key)
            if isinstance(d, dict) and old in d:
                d[new] = d.pop(old)
                changed = True
    if changed:
        save_state(st)


def class_header(name, led=None):
    """A class's header: the ledger's, else include/<Class>.h, else the game
    header that defines the type (track 11 names files in snake_case, so the
    stem no longer spells the class)."""
    if led and led.get("header"):
        return led["header"]
    if (ROOT / f"include/{name}.h").exists():
        return f"include/{name}.h"
    rx = re.compile(rf"(?:\}}\s*{re.escape(name)}\s*;|\bstruct\s+{re.escape(name)}\s*\{{)")
    for h in sorted((ROOT / "include").glob("*.h")):
        if rx.search(h.read_text(errors="replace")):
            return f"include/{h.name}"
    return f"include/{name}.h"


def run_json(cmd):
    out = subprocess.run(cmd, capture_output=True, text=True, cwd=ROOT).stdout
    # progress.py may print warnings above the JSON
    return json.loads(out[out.index("{"):])


def nearmiss_rows():
    """[(words, unit, func, title)] from nearmiss.py's ASSIGN FROM HERE block."""
    out = subprocess.run([sys.executable, "tools/nearmiss.py"],
                         capture_output=True, text=True, cwd=ROOT).stdout
    rows, on = [], False
    for line in out.splitlines():
        if "ASSIGN FROM HERE" in line:
            on = True
            continue
        if not on:
            continue
        m = re.match(r"^\s+(\d+)w\s+(\S+)\s+(\S+)\s+(.*)$", line)
        if m:
            rows.append((int(m.group(1)), m.group(2), m.group(3), m.group(4)))
    return rows


def nm_bodies(code_text):
    """Function names defined inside #ifdef NON_MATCHING blocks."""
    names = set()
    for m in re.finditer(r"^#ifdef NON_MATCHING\b(.*?)^#(?:else|endif)\b",
                         code_text, re.M | re.S):
        names.update(d for d in DEF_RE.findall(m.group(1)) if d not in NOT_DEF)
    return names


def unit_metrics(info):
    """Per-unit readability debt, from LIVE code only."""
    units = {}
    all_defs = {}
    for c in srcpath.src_files():
        raw = c.read_text(errors="replace")
        live = progress.strip_dead_code(raw)
        defs = [d for d in DEF_RE.findall(live) if d not in NOT_DEF]
        if info:
            # Sony code in a game unit is track 2's to name, never track 3's
            # (revision 13: code_179d8_c_b's pass named libsnd in game words).
            defs = [d for d in defs if d in info and not progress.is_library(info[d][0])]
        inc = progress.INCLUDE_RE.findall(live)
        if info:
            inc = [f for f in inc if not (f in info and progress.is_library(info[f][0]))]
        nm = nm_bodies(raw)
        # Debt is measured in GAME bodies only (revision 16): code_179d8_g had
        # one game function and 56 D_ globals, all of them libcd's, which is
        # an invitation to name Sony data in game words (round 75).
        game_text = live
        if info:
            ms = list(DEF_RE.finditer(live))
            game_text = "\n".join(live[m.start():(ms[k + 1].start() if k + 1 < len(ms) else len(live))]
                                  for k, m in enumerate(ms) if m.group(1) in defs)
        units[c.stem] = {
            "defs": len(defs),
            "def_names": defs,
            "func_named": sum(1 for d in defs if FUNC_PH.match(d)),
            "include_asm": len(inc),
            "nm_bodies": len(nm & set(inc)),
            "unk_refs": len(re.findall(r"(?:->|\.)unk_?0?x?[0-9A-Fa-f]+", game_text)),
            "slot_refs": len(re.findall(r"->slot0?x?[0-9A-Fa-f]+\s*\(", game_text)),
            "d_refs": len(set(re.findall(r"\bD_800[0-9A-F]{5}\b", game_text))),
            "refs_out": 0,
            "centrality": 0,
            "live": live,
            "inc": inc,
        }
        all_defs[c.stem] = set(defs)
    # centrality: how many OTHER units reference this unit's definitions
    for u, m in units.items():
        n = 0
        for v, mv in units.items():
            if v == u:
                continue
            n += sum(1 for d in all_defs[u] if re.search(rf"\b{re.escape(d)}\b", mv["live"]))
        m["centrality"] = n
    for m in units.values():
        del m["live"]
    return units


def all_symbols():
    """name -> vram for EVERY symbol in the linked ELF, any type. Sony's object
    symbols are pinned by config/psyq-objects.ld and show up as ABSOLUTE (`A`),
    which progress.text_symbols() rightly ignores for counting functions but
    which is exactly what the SDK call surface needs."""
    nm = ROOT / "tools/binutils/bin/mipsel-linux-gnu-nm"
    elf = ROOT / "build/lsdde.elf"
    if not (nm.exists() and elf.exists()):
        return {}
    out = subprocess.run([str(nm), str(elf)], capture_output=True, text=True).stdout
    syms = {}
    for line in out.splitlines():
        parts = line.split()
        if len(parts) == 3:
            syms[parts[2]] = int(parts[0], 16)
    return syms


def parked_sdk_names():
    """func_ names the track-2 park rule has closed: their symbols-file line, or
    the comment line right above it, says `unidentified` (round 53 parked 44
    this way and plan.py kept re-offering them as the top track-2 job)."""
    parked, block = set(), []      # block: the contiguous comment lines above
    for line in (ROOT / "config/symbols.slps01556.lsdde.txt").read_text().splitlines():
        m = re.match(r"^\s*(func_800[0-9A-Fa-f]{5})\s*=", line)
        if m:
            if "unidentified" in line or any("unidentified" in b for b in block):
                parked.add(m.group(1))
            block = []
        elif line.strip().startswith("//"):
            block.append(line)
        else:
            block = []
    return parked


def sdk_surface(info, units):
    """Functions game code calls that live in Psy-Q segments: named vs func_,
    minus the ones the park rule has closed."""
    info = {n: (a, 0) for n, a in all_symbols().items()} or info
    referenced = set()
    queued = set()
    for c in srcpath.src_files():
        live = progress.strip_dead_code(c.read_text(errors="replace"))
        referenced.update(re.findall(r"\b([A-Za-z_]\w*)\s*\(", live))
        queued.update(progress.INCLUDE_RE.findall(live))
    # Game code still INCLUDE_ASM calls the SDK too, and its call sites become
    # C as track 1 matches it. Revision 18 carved 299 such functions calling
    # 14 still-func_ Sony functions that no C call site showed yet: read their
    # `jal`s from the image so track 2 sees them now, not one match at a time.
    import gameinsdk
    img = gameinsdk.Image()
    by_name = {n: a for a, n, _ in img.funcs}
    for fn in queued:
        a = by_name.get(fn)
        if a is not None and not progress.is_library(a):
            referenced.update(img.name[t] for t in img.calls[a])
    named, unnamed, parked = [], [], []
    closed = parked_sdk_names()
    sony = sdk_in_game_names()
    for name in sorted(referenced):
        a = info.get(name, (None, 0))[0]
        if a is None or not progress.is_library(a):
            continue
        if a in sony and name not in sony[a]:
            continue            # counted below, whether or not game code calls it
        if not FUNC_PH.match(name):
            named.append(name)
        elif name in closed:
            parked.append(name)
        else:
            unnamed.append(name)
    # A function config/sdk-in-game.txt fingerprints EXACTLY is unnamed until it
    # carries one of its candidates' Sony names, however readable the game-style
    # name it has now (SetSeqTimerMode is libsnd's SsSetTickMode).
    by_addr = {a: n for n, (a, _) in progress.text_symbols().items()}
    cmts = progress.symbol_comments()
    for a, cands in sorted(sony.items()):
        name = by_addr.get(a)
        if name is None or a not in progress.SONY_IN_GAME:
            continue            # a lead rejected with `// not SDK:`
        # A lead is closed by its `identified` comment, whatever Sony name the
        # evidence settled on (StartNote is SpuVmKeyOn, not the shape's pick).
        if name in cands or (not FUNC_PH.match(name) and progress.IDENTIFIED_RE.search(cmts.get(a, ""))):
            if name not in named:
                named.append(name)
        elif name not in closed and name not in unnamed:
            unnamed.append(name)
    # An `identified` entry whose comment still says the rename is PENDING
    # (StartNote is SpuVmKeyOn, recorded by the plan session in revision 15):
    # the name the evidence settled on is in the comment, not in a table.
    for a, c in sorted(cmts.items()):
        if progress.IDENTIFIED_RE.search(c) and re.search(r"(?i)\b(rename|name) pending\b", c):
            name = by_addr.get(a)
            if name and name not in unnamed:
                unnamed.append(name)
                if name in named:
                    named.remove(name)
    return named, unnamed, parked


def sdk_in_game_names():
    """{vram: {candidate Sony names}} from config/sdk-in-game.txt and the
    externals config/psyq-objects.ld pins in game segments."""
    out = {a: {n} for a, n in progress.ld_pinned_externals().items()}
    if progress.SDK_IN_GAME.exists():
        for line in progress.SDK_IN_GAME.read_text().splitlines():
            m = re.match(r"^(?:LEAD )?(0x[0-9A-Fa-f]{8})\s+\d+\s+(?:[01]\.\d\d\s+)?(.*)$", line)
            if m:
                out.setdefault(int(m.group(1), 16), set()).update(
                    c.split(":", 1)[1].split("@")[0] for c in m.group(2).split())
    return out


REVISITED_RE = re.compile(r"REVISITED[,:]?\s*\(?round (\d+)")


def revisits_done(func):
    """How many revisits a stall has had: distinct rounds on its REVISITED
    lines (the line format varies; a round mentioned twice is one revisit)."""
    p = REPORTS / f"{func}.md"
    if not p.exists():
        return 0
    t = p.read_text(errors="replace")
    n = len(set(REVISITED_RE.findall(t)))
    return n if n else (1 if "REVISITED" in t else 0)


def report_state(func):
    """('fresh'|'stalled'|'none', has_preserved_body)"""
    p = REPORTS / f"{func}.md"
    if not p.exists():
        return "fresh", False
    t = p.read_text(errors="replace")
    # Line-anchored, like stalesyms.has_if0: prose that mentions "`#if 0`" is not a body (round 66).
    body = bool(re.search(r"^[ \t]*#if\s+0\b.*?^[ \t]*#endif", t, re.S | re.M))
    if progress.REOPENED_RE.search(t):
        return "fresh", body
    return "stalled", body


PERM_RUN_RE = re.compile(r"[0-9]{3,}[, ]*(iteration|iters)|rc=(124|137)|permuter-exhausted|--stop-on-zero", re.I)
SPENT_RE = re.compile(r"levers? (are |is )?spent|exhausted|do not (re-)?attempt|not worth (another|further)|no further attempt", re.I)
EXACT_RE = re.compile(r"length[: ]*EXACT|exact length|length (match|matches)|\b(\d+)/\2\b", re.I)


def stall_cost(func, title):
    """(spent, searched, not_exact, words_key): lower is cheaper to attempt."""
    p = REPORTS / f"{func}.md"
    txt = p.read_text(errors="replace") if p.exists() else ""
    spent = 1 if SPENT_RE.search(txt) else 0
    searched = 1 if PERM_RUN_RE.search(txt) else 0
    exact = 1 if EXACT_RE.search(title) else 0
    return (spent, searched, 1 - exact)


def cost_tag(func, title):
    spent, searched, notexact = stall_cost(func, title)
    tags = []
    tags.append("spent" if spent else "unspent")
    tags.append("searched" if searched else "never-searched")
    if not notexact:
        tags.append("len-exact")
    elif re.search(r"\d+\s*/\s*\d+|\bshort\b|\blong\b|0x[0-9A-Fa-f]+", title, re.I):
        tags.append("len-off")
    else:
        tags.append("len-?")     # the title states no length figure (round 77)
    return ",".join(tags)


def git_date(path):
    out = subprocess.run(["git", "log", "-1", "--format=%cs", "--", str(path)],
                         capture_output=True, text=True, cwd=ROOT).stdout.strip()
    return out or None


def calibration_rounds(st):
    """The rounds that COUNT toward calibration and the stop rule: recorded
    without --not-calibration, i.e. their attempts came from the ranked stall
    band. Calibration is measured in ATTEMPTS PER MODEL, not in rounds, so a
    single three-function stall runner per round accumulates toward it (plan
    revision 3: rounds 51 and 52 never reached a stall job at a 2-3 slot cap,
    and a per-round threshold could never have been met)."""
    return [r for r in st["tracks"]["1"]["rounds"] if r.get("calibration", True)]


def calibration_tally(st):
    """{model: [matches, attempts]} over calibration rounds."""
    per = {"sonnet": [0, 0], "opus": [0, 0]}
    for r in calibration_rounds(st):
        m = r.get("model", "?")
        per.setdefault(m, [0, 0])
        per[m][0] += r.get("matches", 0)
        per[m][1] += r.get("attempts", 0)
    return per


def match_model(st):
    """Which model matching runners use next: Sonnet until it has its
    calibration attempts, then Opus until it has its own, then whichever
    produced more matches per attempt."""
    if st["models"].get("match_runner"):
        return st["models"]["match_runner"], "set by head"
    need = st["stop_rule"].get("attempts_per_model", 6)
    per = calibration_tally(st)
    for m in ("sonnet", "opus"):
        if per[m][1] < need:
            return m, f"calibration: {m} has {per[m][1]}/{need} stall attempts"
    best = max(("sonnet", "opus"), key=lambda m: per[m][0] / max(1, per[m][1]))
    return best, "won calibration (matches per attempt)"


def track1_status(st, fresh, stalled):
    t = st["tracks"]["1"]
    if t["status"] in ("parked", "done"):
        return t["status"], t.get("reason", "")
    if fresh == 0 and stalled == 0:
        return "done", "queue empty"
    # The stop rule below bounds STALL matching. Fresh ground is never parked
    # by it: revision 18 carved 299 fresh game functions out of psyq_*
    # segments after calibration had parked the (then empty-of-fresh) track.
    if fresh:
        return "open", f"{fresh} fresh function(s): fresh ground is not subject to the stall stop rule"
    per = calibration_tally(st)
    need_att = st["stop_rule"].get("attempts_per_model", 6)
    need = st["stop_rule"]["min_matches"]
    if all(per[m][1] >= need_att for m in ("sonnet", "opus")):
        got = sum(v[0] for v in per.values())
        att = sum(v[1] for v in per.values())
        if got < need:
            return "parked", (f"stop rule: calibration complete, {got} match(es) in {att} stall "
                              f"attempts, fewer than {need}. Revisits per FINISHING-PLAN track 1 still run.")
    return "open", ""


def collect(st):
    pj = run_json([sys.executable, "tools/progress.py", "--json"])
    info = progress.text_symbols()
    units = unit_metrics(info)
    named_sdk, unnamed_sdk, parked_sdk = sdk_surface(info, units)
    rows = nearmiss_rows()

    fresh_funcs, stall_rows, promotable = [], [], []
    for words, unit, func, title in rows:
        state, has_body = report_state(func)
        if state == "fresh":
            fresh_funcs.append((words, unit, func, title))
        else:
            stall_rows.append((words, unit, func, title))
            um = units.get(unit)
            if has_body and um is not None and func not in nm_defined(unit):
                promotable.append((words, unit, func))

    t1_status, t1_reason = track1_status(st, pj["fresh"], pj["stalled"])
    mm, mm_why = match_model(st)

    # track 3
    done = st["tracks"]["3"]["units_done"]
    # A unit with FRESH ground is track 1's until it is matched: naming it
    # now would name INCLUDE_ASM, and counting it would move the 80% gate
    # that opens track 4 (revision 18 carved eleven all-fresh units at once).
    fresh_units = {u for u, m in pj["units"].items() if m.get("fresh")}
    todo3 = sorted((u for u in units if u not in done and u not in fresh_units
                    and (units[u]["defs"] or units[u]["include_asm"])),
                   key=lambda u: (-units[u]["centrality"], -units[u]["func_named"]))
    pct3 = len(done) / max(1, len([u for u in units if u not in fresh_units or u in done]))

    # Stall ordering is by ATTEMPT COST, read from the report (revision 5).
    # nearmiss.py ranks by size and revision 4 ranked by report date; both put
    # deeply-searched, levers-spent functions first, and heads skipped the
    # top job four rounds running. PARALLEL-RUNS 3.3's screens 5 and 6 name
    # the signals; this reads them: a spent-levers verdict, evidence of an
    # actual permuter RUN (iteration counts, rc=, --stop-on-zero), then
    # length-exactness, then size. Never-searched, unspent, length-exact and
    # small comes first.
    stall_rows = sorted(stall_rows, key=lambda r: stall_cost(r[2], r[3]) + (r[0],))

    # track 1 revisit (revision 8): EVERY stall gets exactly one revisit, a
    # fresh cost-ranked Opus re-read with the body rebuilt and funcdiff's
    # ins/del line recorded. Revisions 4 and 6 gated it on "unit passed
    # track 3" and then "title stale"; three consecutive revisits recorded
    # names/types NOT relevant while paying 3 matches in 7 attempts against
    # the calibration band's 1 in 13. The trigger was never what paid; the
    # re-read was. The runner's REVISITED line retires a function.
    revisit = []
    for words, unit, func, title in stall_rows:
        if revisits_done(func) == 0:
            revisit.append((words, unit, func, title))
    # Revision 14: revisits paid 40/60 against the band's 1/13, so a stall
    # whose one revisit did not close it gets ONE more, once every stall has
    # had its first. Same cost order; a second REVISITED line retires it.
    revisit_label = "REVISIT"
    if not revisit:
        revisit = [r for r in stall_rows if revisits_done(r[2]) == 1]
        revisit_label = "REVISIT-2"
    # Revision 15: REVISIT-2 is the LAST pass. With neither pass left and no
    # fresh ground, track 1 is done; what remains is track 1b's.
    if not revisit and not fresh_funcs and t1_status == "parked":
        t1_status, t1_reason = "done", ("every stall has had its two revisits; the rest get "
                                        "NON_MATCHING bodies or a written reason (track 1b)")
    # A function eligible for a revisit is offered ONLY there, so the two
    # track-1 queues never hand the same function to two runners.
    rv = {f for _, _, f, _ in revisit}
    stall_rows = [r for r in stall_rows if r[2] not in rv]

    t4 = st["tracks"]["4"]
    classes, shared = track4_classes(st)
    if t4["status"] == "auto":
        t4_status = "open" if pct3 >= 0.8 else "waiting (opens at 80% of units through track 3)"
        if t4_status == "open" and classes and all(c["state"] in ("unified", "parked") for c in classes) \
                and not shared:
            t4_status = "done"
    else:
        t4_status = t4["status"]

    t5 = st["tracks"]["5"]
    ticked = t5["checklist"]
    if t5["status"] == "auto":
        t5_status = "open" if (not todo3 and t4_status in ("done",)) else "waiting (opens when tracks 3 and 4 are done)"
        if t5_status == "open" and all(ticked.get(k) for k in TRACK5_ITEMS):
            t5_status = "done"
    else:
        t5_status = t5["status"]

    t1b = st["tracks"]["1b"]
    if t1b["status"] == "auto":
        t1b_status = "open" if t1_status in ("parked", "done") else "waiting (opens when track 1 parks)"
        if t1_status == "done" and not promotable and not stall_rows and not revisit:
            t1b_status = "done"
    else:
        t1b_status = t1b["status"]

    docs = {}
    for rel, budget in DOC_BUDGETS.items():
        p = ROOT / rel
        n = len(p.read_text(errors="replace").split()) if p.exists() else None
        docs[rel] = {"words": n, "budget": budget,
                     "over": n is not None and n > budget}

    # Game names on Sony DATA (round 86): objects that never placed have no
    # psyq-objects.ld pins, so rename.py's pin guard could not see their bss.
    try:
        sys.path.insert(0, str(ROOT / "tools"))
        import sonydata
        sony_named = [n for n, _, _ in sonydata.flagged()]
    except Exception:
        sony_named = []
    p2 = collect_phase2(st, info, t5_status, classes, units)
    ec = subprocess.run([sys.executable, "tools/externcheck.py"], capture_output=True, text=True, cwd=ROOT).stdout
    m_ec = re.search(r"^(\d+) function\(s\) with conflicting arity", ec, re.M)
    return {
        "extern_conflicts": int(m_ec.group(1)) if m_ec else 0,
        "progress": {k: pj[k] for k in ("matched", "queued", "stalled", "fresh", "uncarved", "game", "library")},
        "elf_present": bool(info),
        "tracks": {
            "1": {"status": t1_status, "reason": t1_reason, "fresh": len(fresh_funcs),
                  "stalled": len(stall_rows), "rounds": st["tracks"]["1"]["rounds"],
                  "next_match_model": mm, "why": mm_why, "revisit": len(revisit)},
            "1b": {"status": t1b_status, "promotable": len(promotable),
                   "nm_bodies": sum(u["nm_bodies"] for u in units.values())},
            "2": {"status": "open" if unnamed_sdk or sony_named else "done",
                  "named": len(named_sdk), "unnamed": len(unnamed_sdk), "parked": len(parked_sdk),
                  "unnamed_list": unnamed_sdk, "sony_data_game_named": sony_named},
            "3": {"status": "done" if not todo3 and not fresh_units else st["tracks"]["3"]["status"],
                  "units_total": len(units), "units_done": len(done),
                  "units_fresh": len(fresh_units - set(done)),
                  "func_named_defs": sum(u["func_named"] for u in units.values()),
                  "defs": sum(u["defs"] for u in units.values()),
                  "unk_refs": sum(u["unk_refs"] for u in units.values()),
                  "slot_refs": sum(u["slot_refs"] for u in units.values()),
                  "d_refs": sum(u["d_refs"] for u in units.values()),
                  "naming_model": st["models"].get("naming_runner") or MODELS["naming_runner_default"]},
            "4": {"status": t4_status,
                  "local_struct_views": sum(len(re.findall(r"^typedef struct", srcpath.unit_src(u).read_text(errors='replace'), re.M)) for u in units),
                  "classes_total": len(classes),
                  "classes_unified": sum(c["state"] == "unified" for c in classes),
                  "classes_parked": sum(c["state"] == "parked" for c in classes),
                  "classes_ready": [c["table"] for c in classes if c["state"] == "ready"],
                  "classes_incomplete": [c["table"] for c in classes if c["state"] == "unified" and c["stray"]],
                  "shared_globals": len(shared)},
            "5": {"status": t5_status, "checklist": {k: bool(ticked.get(k)) for k in TRACK5_ITEMS}},
            **{k: v for k, v in p2.items() if not k.startswith("_")},
        },
        "_p2": p2,
        "docs": docs,
        "units": units,
        "_fresh": fresh_funcs, "_stalls": stall_rows, "_promotable": promotable,
        "_todo3": todo3, "_revisit": revisit, "_revisit_label": revisit_label,
        "_t4_rounds": st["tracks"]["4"].get("rounds", []),
        "_classes": classes, "_shared_globals": shared,
        "_globals_recipe": st["tracks"].get("4b", {}).get("status") in ("open", "done"),
    }


def collect_phase2(st, info, t5_status, classes, units_meta):
    """Tracks 6 to 9 (plan revision 27): names for types, a polish pass per
    unit, files that are real translation units, and the close-out. Every
    count comes from tools/readability.py and tools/tuboundary.py; the ledger
    holds only setup ticks, passed units, and flagged/parked names."""
    import readability
    import tuboundary
    rd = readability.collect(info)
    tr = st["tracks"]
    t6, t7, t8, t9 = tr["6"], tr["7"], tr["8"], tr["9"]
    # Phase 2 latches on track 5's ticked checklist, not its live status: a
    # phase 1 track that reopens on a regression (round 93: an Entity polish
    # retyped seven motion-template externs, reopening 4b) is worked BESIDE
    # phase 2, since tracks are not gates on each other (FINISHING-PLAN §3).
    phase2 = t5_status == "done" or all(tr["5"]["checklist"].get(k) for k in TRACK5_ITEMS)

    # --- track 6: placeholder class and type names ---------------------------
    parked6 = t6.get("parked", {})
    types = {n: (why, list(fs)) for n, (why, fs) in rd["types"].items() if n not in parked6}
    flagged = {n: w for n, w in t6.get("flagged", {}).items() if n not in parked6}
    if flagged:
        # A flag is the head's decision and wins over a pattern's reason for
        # the same name: an unparked placeholder flagged `--after` must reach
        # the flagged-job path (round 96: SceneNodeSub14 and its siblings).
        for n in set(flagged) & set(types):
            types[n] = (f"flagged: {flagged[n]}", types[n][1])
        for p in readability.game_headers() + list(srcpath.src_files()):
            rel = p.relative_to(ROOT).as_posix()
            for n in readability.type_names(p.read_text(errors="replace")) & set(flagged):
                fs = types.setdefault(n, (f"flagged: {flagged[n]}", []))[1]
                if rel not in fs:
                    fs.append(rel)
    depth = {}

    def dep(c):
        if c["table"] not in depth:
            par = next((x for x in classes if x["table"] == c["parent"]), None)
            depth[c["table"]] = 0 if par is None else dep(par) + 1
        return depth[c["table"]]
    class_jobs, covered = [], set()
    byt = {c["table"]: c for c in classes}
    for c in sorted(classes, key=lambda c: (dep(c), c["id"])):
        name = c.get("name") or c["table"]
        hdr = class_header(name)
        ph_name = name in types
        ph_table = not TABLE_NAME.match(c["table"])
        if c["table"] in parked6 or not (ph_name or ph_table):
            continue
        own = sorted(n for n, (_, fs) in types.items() if hdr in fs)
        covered |= set(own)
        par = byt.get(c["parent"])
        # The runner also edits the units holding the class's own methods
        # (their banners, the renamed fields' accessors), so they are in the
        # job's edit set: round 93 held a class_3bb8c polish by hand while
        # 19 of its 21 functions were Class866E8's, under its class job.
        owned = set(c.get("owned", []))
        own_units = sorted(u for u, m in units_meta.items() if owned & set(m.get("def_names", ())))
        class_jobs.append({"class": name, "table": c["table"], "header": hdr, "types": own,
                           "units": own_units,
                           "parent": (par.get("name") or par["table"]) if par else None,
                           "methods": len(c.get("owned", [])), "ph_name": ph_name, "ph_table": ph_table})
    homes = {}
    for n, (why, fs) in types.items():
        if n in covered:
            continue
        homes.setdefault(sorted(fs)[0] if fs else "?", []).append(n)
    t6_setup = all(t6["checklist"].get(k) for k in TRACK6_SETUP)
    # Files that re-declare a Sony name cannot include Sony's headers
    # (tools/sonyheaders.py): a header's is track 6's, a unit's its polish pass's.
    sony = {}
    if phase2 and t6_setup:
        import sonyheaders
        sony = sonyheaders.collect()
    jobbed = {j["header"] for j in class_jobs} | set(homes)
    sony_left = {f: n for f, n in sony.items() if f.startswith("include/") and f not in jobbed}
    left6 = len(class_jobs) + sum(len(v) for v in homes.values()) + len(sony_left)
    if t6["status"] != "auto":
        s6 = t6["status"]
    elif not phase2:
        s6 = "waiting (phase 2 opens when track 5 is done)"
    else:
        s6 = "done" if (t6_setup and not left6) else "open"

    # --- track 7: one polish pass per unit -----------------------------------
    done7 = t7.get("units_done", {})
    keys = readability.DEBT_KEYS
    todo7 = sorted((u for u, m in rd["units"].items() if m["defs"] and u not in done7),
                   key=lambda u: (-sum(rd["units"][u][k] for k in keys), u))
    t7_setup = all(t7["checklist"].get(k) for k in TRACK7_SETUP)
    if t7["status"] != "auto":
        s7 = t7["status"]
    elif not phase2:
        s7 = "waiting (phase 2 opens when track 5 is done)"
    else:
        s7 = "done" if (t7_setup and not todo7) else "open"

    # --- track 8: files, region by region ------------------------------------
    text, _, _ = tuboundary.map_sections()
    regions, cur = [], []
    for _, _, path in text:
        u = tuboundary.unit_of(path) if path.startswith("build/src/") else None
        if u:
            cur.append(u)
        elif cur:
            regions.append(cur)
            cur = []
    if cur:
        regions.append(cur)
    try:
        rows, gaps, forced, contra, val = tuboundary.analyse()
        rep = {r["unit"]: r for r in tuboundary.unit_report(rows, gaps, forced)}
        tu_ok = not val["bad_edges"] and not val["bad_forced"] and not contra
    except SystemExit:
        rep, tu_ok = {}, False
    ph_type_files = {f for _, fs in types.values() for f in fs}
    parked8 = t8.get("parked", {})
    region_rows = []
    for reg in regions:
        ph_units = [u for u in reg if readability.PLACEHOLDER_FILE.match(u)]
        ph_hdrs = [f"include/{u}.h" for u in reg if f"{u}.h" in rd["placeholder_headers"]]
        blockers = []
        for u in reg:
            m = rd["units"].get(u, {})
            if m.get("defs") and u not in done7:
                blockers.append(f"{u}: track 7")
            src_rel = srcpath.unit_src(u).relative_to(ROOT).as_posix() if srcpath.unit_src(u) else f"src/{u}.c"
            if m.get("ph_prefix") or src_rel in ph_type_files or f"include/{u}.h" in ph_type_files:
                blockers.append(f"{u}: track 6 names")
        joins = [u for u in reg[1:] if rep.get(u, {}).get("start_possible") is False]
        splits = [s_ for u in reg for s_ in rep.get(u, {}).get("forced_inside", [])]
        state = ("parked" if reg[0] in parked8 else "done" if not (ph_units or ph_hdrs)
                 else "ready" if not blockers else "waiting")
        region_rows.append({"units": reg, "ph_units": ph_units, "ph_headers": ph_hdrs, "state": state,
                            "blockers": blockers, "joins": joins, "splits": splits})
    open8 = [r for r in region_rows if r["state"] in ("ready", "waiting")]
    # A placeholder-named header whose unit a merge absorbed belongs to no
    # region, so the region test above cannot see it (round 102:
    # class_3bb8c.h, orphaned by round 101's merge, left track 8 "done").
    in_regions = {u for reg in regions for u in reg}
    orphans8 = [h for h in rd["placeholder_headers"]
                if Path(h).stem not in in_regions and f"include/{h}" not in parked8]
    if t8["status"] != "auto":
        s8 = t8["status"]
    elif not phase2:
        s8 = "waiting (phase 2 opens when track 5 is done)"
    else:
        s8 = "done" if not (open8 or orphans8) else "open"

    # --- track 9: close-out ---------------------------------------------------
    ticked9 = t9["checklist"]
    if t9["status"] != "auto":
        s9 = t9["status"]
    elif not (s6 == "done" and s7 == "done" and s8 == "done") and not any(ticked9.values()):
        # Once opened (an item ticked), it stays open beside a reopened
        # earlier track, as phase 2 stays open beside phase 1.
        s9 = "waiting (opens when tracks 6, 7 and 8 are done)"
    else:
        s9 = "done" if all(ticked9.get(k) for k in TRACK9_ITEMS) else "open"

    return {
        "6": {"status": s6, "setup": {k: bool(t6["checklist"].get(k)) for k in TRACK6_SETUP},
              "placeholder_types": len(types), "class_jobs": len(class_jobs),
              "type_homes": len(homes), "parked": len(parked6), "flagged": len(flagged),
              "ph_prefix_defs": rd["totals"]["ph_prefix"],
              "sony_headers": sum(f.startswith("include/") for f in sony),
              "sony_units": sum(f.startswith("src/") for f in sony)},
        "7": {"status": s7, "setup": {k: bool(t7["checklist"].get(k)) for k in TRACK7_SETUP},
              "units_done": len(done7), "units_total": len(done7) + len(todo7),
              "totals": {k: rd["totals"][k] for k in keys},
              "model": st["models"].get("polish_runner") or MODELS["polish_runner_default"]},
        "8": {"status": s8, "regions": len(region_rows),
              "regions_done": sum(r["state"] == "done" for r in region_rows),
              "regions_ready": sum(r["state"] == "ready" for r in region_rows),
              "placeholder_units": rd["totals"]["placeholder_units"],
              "placeholder_headers": rd["totals"]["placeholder_headers"], "tu_model_valid": tu_ok,
              "orphan_headers": orphans8},
        "9": {"status": s9, "checklist": {k: bool(ticked9.get(k)) for k in TRACK9_ITEMS},
              "history": rd["totals"]["history"] + rd["totals"]["header_history"]},
        **phase3_status(tr, s9),
        "_class_jobs": class_jobs, "_homes": homes, "_types": types, "_todo7": todo7,
        "_after": {n: fs for n, fs in t6.get("after", {}).items() if n in flagged},
        "_units": {n: us for n, us in t6.get("units", {}).items() if n in flagged},
        "_rd": rd, "_regions": region_rows, "_sony": sony, "_sony_left": sony_left,
    }


def track4_classes(st):
    """Every class (method table) in tree order with its track-4 state.

    unified / parked: the ledger says so (mark-class). A unified class is
    re-measured every run: any object or table view of it defined outside
    its ledger header is listed as `stray` (a regression, or a view the
    unification missed). ready: every ancestor is unified or parked, so the
    class header can expand its parent's macros. waiting: an ancestor is not.
    Ready classes rank by how many classes sit below them (unifying a base
    unblocks its subtree), then by how few views they have."""
    import typeviews
    views, funcs = typeviews.collect()
    cen = typeviews.census(views, funcs)
    shared = typeviews.shared_globals()
    ledger = st["tracks"]["4"].get("classes", {})
    byname = {c["table"]: c for c in cen}
    info = progress.text_symbols()
    queued = set()
    for f in srcpath.src_files():
        for fn in progress.INCLUDE_RE.findall(progress.strip_dead_code(f.read_text(errors="replace"))):
            if fn in info and not progress.is_library(info[fn][0]):
                queued.add(fn)
    kids = {}
    for c in cen:
        kids.setdefault(c["parent"], []).append(c["table"])

    def below(t):
        return sum(1 + below(k) for k in kids.get(t, []))
    # an ancestor's unified type seen as a "view" (a getter or method declared
    # with the parent's type) is not this class's: every includer of its
    # header would count (round 88: BgLayer's Class6B5CCMethods, 45 units)
    unified_types = {n + sfx for led in ledger.values() if not led.get("parked") and led.get("class")
                     for n in [led["class"]] for sfx in ("", "Methods")}
    out = []
    for c in cen:
        led = ledger.get(c["table"])
        hdr = led.get("header") if led else None
        stray = []
        if led and not led.get("parked"):
            for kind in ("objects", "tables"):
                for v, d in c[kind].items():
                    if any(f != hdr for f in d["defined"]):
                        stray.append(f"{v} ({', '.join(f for f in d['defined'] if f != hdr)})")
        def settled(t):
            # an ancestor with no C methods and no views cannot be unified from
            # C; it does not block its subclasses (track 4 recipe, "no C")
            b = byname.get(t)
            return t in ledger or (b is not None and not b["owned_c"] and not b["objects"] and not b["tables"])
        anc, ok = c["parent"], True
        while anc:
            if not settled(anc):
                ok = False
                break
            anc = byname[anc]["parent"] if anc in byname else None
        if led:
            state = "parked" if led.get("parked") else "unified"
        elif not c["owned_c"] and not c["objects"] and not c["tables"]:
            # Revision 18: a class whose own methods are carved INCLUDE_ASM in
            # a game unit is not "no C" for good -- track 1 will match them,
            # and then it is an ordinary ready/waiting class. Until then it
            # settles like "no C" so it does not stall its subclasses.
            state = "no C yet" if any(f in queued for f in c["owned"]) else "no C"
        else:
            state = "ready" if ok else "waiting"
        files = sorted({f for kind in ("objects", "tables") for d in c[kind].values() for f in d["defined"]
                        if f != "?"})
        units_ = sorted({u for d in c["objects"].values() for u in d["units"]} |
                        {Path(f).stem for f in files if f.startswith("src/")})
        if state == "ready":
            units_ = sorted(set(units_) | class_footprint(c, unified_types))
        out.append(dict(c, state=state, stray=stray, below=below(c["table"]),
                        nviews=len(c["objects"]) + len(c["tables"]), files=files, units=units_,
                        name=(led or {}).get("class")))
    # A class's own method table declared with another type somewhere is a
    # VIEW of that class (track 4 step 1), so its class job removes it; 4b
    # lists it only once the class is unified, as the regression it then is
    # (round 85: D_8006D3C8 in code_171e0.h).
    pending = {c["table"] for c in out if c["state"] not in ("unified", "parked")}
    shared = {k: v for k, v in shared.items() if k not in pending}
    return out, shared


_fp_texts = None
SCALAR_TYPES = frozenset({"void", "char", "int", "short", "long", "unsigned",
                          "s8", "u8", "s16", "u16", "s32", "u32", "s64", "u64"})


def class_footprint(c, unified_types=frozenset()):
    """Every unit a class job may edit (revision 23): the units whose code
    (comments stripped) names the class's table, a getter of it, an own
    method or a view type, plus every unit that includes, directly or through
    another header, a header that does. Track 4 step 7 deletes those views
    and externs and fixes the accessors the compiler then lists, and those
    accessors can only be in code that sees a view. Two ready classes whose
    footprints are disjoint (and pass select_jobs' call-graph test) run in
    parallel; the old one-class-per-round rule is this test with nothing
    measured."""
    global _fp_texts
    if _fp_texts is None:
        _fp_texts = {}
        for f in list(srcpath.src_files()) + list((ROOT / "include").glob("*.h")):
            t = f.read_text(errors="replace")
            _fp_texts[f] = (re.sub(r"/\*.*?\*/", " ", t, flags=re.S),
                            set(re.findall(r'#include\s+"([^"]+)"', t)))
    # A scalar is a view to delete (`extern s32 D_80087034;`), never a symbol
    # to search for: `\bs32\b` hits every file (round 89: 86 units). The
    # table symbol already finds that extern line.
    views = [v for v in (*c["objects"], *c["tables"])
             if v not in unified_types and v not in SCALAR_TYPES]
    syms = {c["table"], *c["owned"], *views}
    tre = re.escape(c["table"])
    ore = "|".join(map(re.escape, [o for o in c["objects"] if o not in unified_types])) or r"(?!)"
    for t, _ in _fp_texts.values():
        syms |= set(re.findall(rf"\b(\w+)\s*\(\s*(?:void)?\s*\)\s*\{{\s*return\s+&?\s*{tre}\s*;", t))
        # a function DEFINED returning the object (a singleton getter, an
        # allocator): round 87's GetDrawSystem, extern'd under other views
        syms |= set(re.findall(rf"^\s*(?:{ore})\s*\*\s*(\w+)\s*\([^;{{]*\)\s*\{{", t, re.M))
    # a function whose BODY calls a getter of the table: the allocator, whatever
    # type it is declared returning (round 88: New_Obj6EAC0 returned Unk64Elem *
    # and was extern'd as FieldM7C *, so its four calling units were missed).
    # Only the .c files naming it count: an extern of it in a widely included
    # header does not make that header's includers its accessors.
    getters = {g for t, _ in _fp_texts.values()
               for g in re.findall(rf"\b(\w+)\s*\(\s*(?:void)?\s*\)\s*\{{\s*return\s+&?\s*{tre}\s*;", t)}
    alloc = set()
    if getters:
        gre = re.compile(r"\b(?:" + "|".join(map(re.escape, sorted(getters))) + r")\s*\(")
        for t, _ in _fp_texts.values():
            for m in re.finditer(r"^\w[\w \t\*]*?\b(\w+)\s*\([^;{]*\)\s*\{(.*?)^\}", t, re.M | re.S):
                if gre.search(m.group(2)):
                    alloc.add(m.group(1))
    alloc -= syms
    rx = re.compile(r"\b(?:" + "|".join(sorted(map(re.escape, syms))) + r")\b")
    hit = {f for f, (t, _) in _fp_texts.items() if rx.search(t)}
    if alloc:
        arx = re.compile(r"\b(?:" + "|".join(sorted(map(re.escape, alloc))) + r")\b")
        direct = {f for f, (t, _) in _fp_texts.items() if f.suffix == ".c" and arx.search(t)}
    else:
        direct = set()
    hdrs = {f.name for f in hit if f.suffix == ".h"}
    grew = True
    while grew:                                  # headers that include a hit header, transitively
        grew = False
        for f, (_, inc) in _fp_texts.items():
            if f.suffix == ".h" and f.name not in hdrs and inc & hdrs:
                hdrs.add(f.name)
                grew = True
    if hdrs & {"common.h", "types.h"}:           # headercontention.UBIQUITOUS: every unit sees it
        return {f.stem for f in _fp_texts if f.suffix == ".c"}
    return {f.stem for f, (_, inc) in _fp_texts.items()
            if f.suffix == ".c" and (f in hit or f in direct or inc & hdrs)}


_nm_cache = {}


def nm_defined(unit):
    if unit not in _nm_cache:
        p = srcpath.unit_src(unit)
        _nm_cache[unit] = nm_bodies(p.read_text(errors="replace")) if p else set()
    return _nm_cache[unit]


# a fresh job carries this many functions of one unit (revision 18)
FRESH_PER_RUNNER = 10
REGION_UNITS = 6        # track 8: units per files job
STALLS_PER_RUNNER = 3


def stall_runner_jobs(rows, model, label):
    """Group ranked stalls into one-unit runner jobs of up to STALLS_PER_RUNNER,
    in ranking order: a runner owns one unit, so the top stall's unit-mates
    (next in rank) ride along with it."""
    by_unit, order = {}, []
    for words, unit, func, title in rows:
        if unit not in by_unit:
            by_unit[unit] = []
            order.append(unit)
        by_unit[unit].append((words, func, title))
    jobs_ = []
    for unit in order:
        fs = by_unit[unit]
        for i in range(0, len(fs), STALLS_PER_RUNNER):
            chunk = fs[i:i + STALLS_PER_RUNNER]
            names = ", ".join(f"{f} ({w}w; {cost_tag(f, t)})" for w, f, t in chunk)
            jobs_.append(("1", f"{label} runner on {unit}: {names}", model))
    return jobs_


def jobs(d, n):
    """Ready jobs across open tracks, ROUND-ROBIN by track priority, a model each.

    Revision 3. A priority-sorted list starved every track but the first: at
    a two or three slot cap rounds 51 and 52 never reached a stall job, so
    calibration could not record and track 2 sat unstaffed. Now each open
    track contributes one job per turn, in this order of turns: fresh matches,
    naming, stall runner (calibration), SDK batch, revisits, promotions,
    types, close-out. A head with K slots takes the top K, one unit each."""
    t = d["tracks"]
    queues = []
    # Fresh ground, one job per unit (chunked), cheapest first inside it:
    # revision 18 reopened 299 fresh functions in eleven units, and one line
    # per function put nine two-word stubs at the top and hid the unit sizes.
    by_unit = {}
    for words, unit, func, title in sorted(d["_fresh"]):
        by_unit.setdefault(unit, []).append((words, func))
    q_fresh = []
    for unit in sorted(by_unit, key=lambda u: (sum(w for w, _ in by_unit[u]), u)):
        fs = by_unit[unit]
        for i in range(0, len(fs), FRESH_PER_RUNNER):
            chunk = fs[i:i + FRESH_PER_RUNNER]
            names = ", ".join(f"{f} ({w}w)" for w, f in chunk)
            q_fresh.append(("1", f"match fresh ground in {unit} ({len(fs)} fresh, "
                                 f"{sum(w for w, _ in fs)}w): {names}", t["1"]["next_match_model"]))
    q_naming = []
    for unit in d["_todo3"]:
        u = d["units"][unit]
        if not (u["func_named"] or u["unk_refs"] or u["slot_refs"] or u["d_refs"]):
            continue        # nothing a naming pass can measure: review-only, listed below
        q_naming.append(("3", f"naming pass on {unit} (centrality {u['centrality']}, "
                              f"{u['func_named']}/{u['defs']} defs unnamed, {u['unk_refs']} unk, "
                              f"{u['slot_refs']} slot calls, {u['d_refs']} D_)", t["3"]["naming_model"]))
    q_stall = stall_runner_jobs(d["_stalls"], t["1"]["next_match_model"], "stall") \
        if t["1"]["status"] == "open" else []
    if d.get("extern_conflicts"):
        q_naming.insert(0, ("3", f"extern review: {d['extern_conflicts']} function(s) whose extern "
                                 "arity disagrees with the definition (python3 tools/externcheck.py; "
                                 "prompt FINISHING-PLAN 4.5; touches many units, so it runs alone or merges last)",
                            "opus"))
    q_sdk = []
    if t["2"]["unnamed"]:
        if not (ROOT / "tools/sdkname.py").exists():
            q_sdk.append(("2", "HEAD: build tools/sdkname.py before staffing track 2",
                          MODELS["head_when_new_procedure"]))
        else:
            q_sdk.append(("2", f"identify and name {t['2']['unnamed']} SDK functions game code calls "
                               f"(one runner, batch; list: plan.py --json .tracks.2.unnamed_list)",
                          MODELS["mechanical_runner"]))
    if t["2"]["sony_data_game_named"]:
        q_sdk.append(("2", f"give Sony's names back to {len(t['2']['sony_data_game_named'])} game-named "
                           "Sony data symbol(s) (python3 tools/sonydata.py -v; anchor each lead, "
                           "rename.py to Sony's name or to the placeholder; a field inside a Sony "
                           "table is a C access, not a symbol)", MODELS["mechanical_runner"]))
    q_revisit = stall_runner_jobs(d["_revisit"], t["1"]["next_match_model"], d["_revisit_label"])
    # One job per UNIT (a runner owns one unit; round 62's slot drew one 32w
    # function while its unit had nine promotable bodies).
    q_promote = []
    if t["1b"]["status"] == "open":
        by_unit = {}
        for words, unit, func in sorted(d["_promotable"]):
            by_unit.setdefault(unit, []).append((words, func))
        for unit, fs in sorted(by_unit.items(), key=lambda kv: -len(kv[1])):
            names = ", ".join(f"{f} ({w}w)" for w, f in fs)
            q_promote.append(("1b", f"promote {len(fs)} preserved body(ies) in {unit} to #ifdef NON_MATCHING: {names}",
                              MODELS["mechanical_runner"]))
    first_class = not any(c["state"] == "unified" for c in d["_classes"])
    q_types = []
    if t["4"]["status"] == "open":
        if first_class:
            q_types.append(("4", f"HEAD (premium, FINISHING-PLAN track 4): unify the FIRST class yourself and write "
                                 f"the recipe into track 4; {t['4']['local_struct_views']} unit-local struct views remain",
                            MODELS["head_when_new_procedure"]))
        else:
            # Classes whose footprints (class_footprint) are disjoint run in
            # parallel; select_jobs DEFERS any that share a unit or call-graph
            # contention with a class above it (revision 23; revision 20 ran
            # one at a time without measuring the overlap).
            ready = sorted((c for c in d["_classes"] if c["state"] == "ready"),
                           key=lambda c: (-c["below"], c["nviews"], c["id"]))
            for c in ready:
                q_types.append(("4", f"unify class {c['table']} (id 0x{c['id']:X}, parent {c['parent']}, "
                                     f"{c['below']} class(es) below it; {c['nviews']} view(s) in "
                                     f"{', '.join(c['files']) or 'none found'}) (units: {','.join(c['units'])})",
                                "opus"))
            if not d.get("_globals_recipe"):
                q_types.append(("4", f"HEAD (premium, FINISHING-PLAN track 4b): unify the FIRST shared global yourself "
                                     f"and write its recipe; {t['4']['shared_globals']} global(s) declared with more "
                                     "than one type (typeviews.py --globals)",
                                MODELS["head_when_new_procedure"]))
            elif d["_shared_globals"]:
                # 4b jobs, once its recipe exists: the global declared in the most files first
                g = max(d["_shared_globals"].items(),
                        key=lambda kv: (len({f for fs in kv[1].values() for f in fs}), kv[0]))
                files = sorted({f for fs in g[1].values() for f in fs})
                q_types.append(("4", f"unify global {g[0]} ({len(g[1])} types in {len(files)} files, FINISHING-PLAN "
                                     f"track 4b) (units: {','.join(Path(f).stem for f in files if f.startswith('src/'))})",
                                "opus"))
    q_close = [("5", f"{k}: {TRACK5_ITEMS[k]}", "opus")
               for k, done in t["5"]["checklist"].items() if not done] if t["5"]["status"] == "open" else []
    p2 = d["_p2"]
    P = MODELS["head_when_new_procedure"]
    q6, q7, q8, q9 = [], [], [], []
    def sony_includers(header):
        """Units that include `header`, directly or through another project
        header: the ones that must take Sony's headers when it stops
        re-declaring Sony's names."""
        import headercontention
        base = Path(header).name
        inc = {h.name: set(re.findall(r'#include\s+"([^"]+)"', h.read_text(errors="replace")))
               for h in Path("include").glob("*.h")}
        closure, grew = {base}, True
        while grew:
            more = {h for h, hs in inc.items() if hs & closure} - closure
            closure |= more
            grew = bool(more)
        return sorted(u for u, hs in headercontention.build_map().items() if hs & closure)

    def sony_note(f):
        names = p2["_sony"].get(f) or next((n for k, n in p2["_sony"].items()
                                             if k.startswith("src/") and Path(k).stem == f), None)
        return f"; re-declares Sony's {', '.join(names)} (tools/sonyheaders.py)" if names else ""
    if t["6"]["status"] == "open":
        for k, done in t["6"]["setup"].items():
            if not done:
                q6.append(("6", f"HEAD (premium) setup {k}: {TRACK6_SETUP[k]} (FINISHING-PLAN track 6)", P))
        if all(t["6"]["setup"].values()):
            # A file defining a type the head FLAGGED goes first: a flag is a
            # decision the patterns cannot rank, usually because other jobs'
            # debt waits on it (round 92: EntityMoodHandlerArg is SoundCueSet,
            # and six Entity_* polish passes read its unk fields).
            def is_flagged(n):
                return p2["_types"].get(n, ("",))[0].startswith("flagged:")
            # A header that re-declares a Sony name goes before everything
            # else: no includer of it can take <libgs.h> and friends until it
            # does (round 94: ViewportOt -> GsOT broke five of Viewport.h's
            # eleven includers on TimImage.h's GsIMAGE, TileMap.h's GsMAP,
            # TileAtlas.h's GsCELL and class_3bb8c.h's RotMatrix, while those
            # headers' own jobs ranked last).
            # Its edit set is the header AND every unit that includes it,
            # because each of those takes Sony's headers in the same fix; two
            # headers sharing an includer are one job, since that unit cannot
            # take <libgs.h> until both are fixed (round 95: TileAtlas.h,
            # TileMap.h and TimImage.h all reach code_33808, and alpha could not
            # land the first two green without the third).
            # A header that ALSO defines placeholder types (code_d294.h) is
            # the same blocker, so it ranks here too and carries its types
            # (round 95: it had ranked eighth, behind polish passes).
            class_hdrs = {j["header"] for j in p2["_class_jobs"]}
            sony_hdrs = {f: n for f, n in p2["_sony"].items()
                         if f.startswith("include/") and f not in class_hdrs}
            # A flagged job the head marked `--after` waits until every file it
            # names has left tools/sonyheaders.py's list: its fix needs Sony's
            # headers in files those collisions would break.
            d["_waiting"] = []
            def waits_on(names):
                return sorted({w for n in names for w in p2["_after"].get(n, []) if w in p2["_sony"]})
            first = {f for f, names in p2["_homes"].items() if any(is_flagged(n) for n in names)}
            # Once it is ready, a flagged `--after` HEADER is a collision
            # header's job in all but name: its fix puts Sony's headers in it,
            # so every includer takes them too, and it joins the group of any
            # collision header it shares an includer with (round 96:
            # ViewportOt's <libgs.h> in Viewport.h reaches code_2864 and
            # code_2cc8c_d, both Sprite.h's, and two runners would each have
            # added the same Sony #include block to them).
            sony_views = {f for f in first if f.startswith("include/") and f not in sony_hdrs
                          and f not in class_hdrs and not waits_on(p2["_homes"][f])
                          and any(n in p2["_after"] for n in p2["_homes"][f] if is_flagged(n))}
            groups = []   # [(set of includer units, [(header, names or None for a sony view)])]
            for f in sorted(set(sony_hdrs) | sony_views):
                us, fs = set(sony_includers(f)), [(f, sony_hdrs.get(f))]
                for g in [g for g in groups if g[0] & us]:
                    groups.remove(g)
                    us |= g[0]
                    fs = g[1] + fs
                groups.append((us, sorted(fs, key=lambda x: x[0])))

            def part(f, names):
                if names is None:
                    ts = sorted(p2["_homes"][f])
                    why = "; ".join(p2["_types"][n][0] for n in ts if is_flagged(n))
                    return f"{f}: name its flagged type(s) {', '.join(ts)} ({why})"
                return (f"{f}: it re-declares {', '.join(names)}"
                        + (f", and name its placeholder type(s) {', '.join(sorted(p2['_homes'][f]))}"
                           if f in p2["_homes"] else ""))
            for us, fs in sorted(((sorted(u), f) for u, f in groups), key=lambda g: g[1][0][0]):
                what = "; ".join(part(f, names) for f, names in fs)
                q6.append(("6", f"use Sony's own declarations in {what} "
                                f"(FINISHING-PLAN track 6 step 4; tools/sonyheaders.py) "
                                f"(units: {','.join([f for f, _ in fs] + list(us))})",
                           MODELS["types_runner"]))
            for f in sorted(first - sony_views):
                names = p2["_homes"][f]
                u = Path(f).stem if f.startswith("src/") else f
                why = "; ".join(p2["_types"][n][0] for n in sorted(names) if is_flagged(n))
                if waits_on(names):
                    d["_waiting"].append((f"name {', '.join(sorted(names))} in {f}", waits_on(names)))
                    continue
                # A flag's fix can reach units other than the file defining the
                # type (`flag-type --units`); they join the edit set so a job
                # holding them defers it (round 97: ResourceRequest's views in
                # GraphicsResources.c and code_55dd4.c, both SceneNode.h's job).
                extra = sorted({x for n in names for x in p2["_units"].get(n, [])} - {u})
                q6.append(("6", f"name {len(names)} placeholder type(s) defined in {f}: {', '.join(sorted(names))}"
                                f" ({why}){sony_note(f)} (units: {','.join([u] + extra)})", MODELS["types_runner"]))
            # A class whose own name is fine but whose TABLE is D_/ALLCAPS is one
            # rename.py each: batched into one mechanical job, not a runner per table.
            tables = [j for j in p2["_class_jobs"] if not j["ph_name"]]
            if tables:
                q6.append(("6", f"rename {len(tables)} method table(s) of named classes to g<Class>Methods with "
                                "rename.py, one commit each (FINISHING-PLAN track 6 step 3): "
                                + ", ".join(f"{j['table']} ({j['class']})" for j in tables)
                                + f" (units: {','.join(j['header'] for j in tables)})", MODELS["mechanical_runner"]))
            for j in p2["_class_jobs"]:
                if not j["ph_name"]:
                    continue
                extra = f"; placeholder types in its header: {', '.join(j['types'])}" if j["types"] else ""
                extra += sony_note(j["header"])
                tab = f" and table {j['table']}" if j["ph_table"] else ""
                q6.append(("6", f"name class {j['class']}{tab} (parent {j['parent'] or 'none'}, {j['methods']} own "
                                f"methods, {j['header']}{extra}) (units: {','.join([j['header']] + j['units'])})", MODELS["types_runner"]))
            for f, names in sorted(p2["_homes"].items(), key=lambda kv: (-len(kv[1]), kv[0])):
                if f in first or f in sony_hdrs:
                    continue
                u = Path(f).stem if f.startswith("src/") else f
                q6.append(("6", f"name {len(names)} placeholder type(s) defined in {f}: {', '.join(sorted(names))}"
                                f"{sony_note(f)} (units: {u})", MODELS["types_runner"]))
    if t["7"]["status"] == "open":
        for k, done in t["7"]["setup"].items():
            if not done:
                q7.append(("7", f"HEAD (premium) setup {k}: {TRACK7_SETUP[k]} (FINISHING-PLAN track 7)", P))
        if all(t["7"]["setup"].values()):
            rdu = p2["_rd"]["units"]
            # A unit a WAITING job waits on goes first: its Sony collisions
            # are its polish pass's to fix (round 95: ViewportOt waited on
            # code_2cc8c_d, whose pass ranked 22nd).
            blockers = {Path(w).stem for _, ws in d.get("_waiting", []) for w in ws if w.startswith("src/")}
            for u in sorted(p2["_todo7"], key=lambda u: u not in blockers):
                m = rdu[u]
                debt = ", ".join(f"{m[k]} {k}" for k in ("func_", "D_", "unk", "magic", "rawoff", "m2c", "history")
                                 if m[k])
                q7.append(("7", f"polish pass on {u} ({m['defs']} defs; {debt or 'no measured debt'}"
                                f"{sony_note(u)}) (units: {u})",
                           t["7"]["model"]))
    if t["8"]["status"] == "open":
        # Ready regions in text order, batched to about REGION_UNITS units a
        # job: most regions are one small unit between two Sony objects.
        batch = []

        def flush():
            if not batch:
                return
            us = [u for r in batch for u in r["units"]]
            ev = [f"rodata proves one file across {', '.join(r['joins'])} and its predecessor"
                  for r in batch if r["joins"]]
            ev += [f"{len(r['splits'])} forced split(s) in {r['units'][0]}.." for r in batch if r["splits"]]
            regs = "; ".join(f"{r['units'][0]}..{r['units'][-1]}" if len(r["units"]) > 1 else r["units"][0]
                             for r in batch)
            q8.append(("8", f"files for {len(batch)} region(s) [{regs}] ({len(us)} unit(s)"
                            f"{'; ' + '; '.join(ev) if ev else ''}) (units: {','.join(us)})",
                       MODELS["files_runner"]))
            batch.clear()
        for h in t["8"]["orphan_headers"]:
            us = sony_includers(h)
            q8.append(("8", f"re-home include/{h}: a placeholder-named header no unit owns; move each "
                            f"section into the header that owns its subject (archived track 4b), or rename "
                            f"it for what it holds (units: include/{h},{','.join(us)})",
                       MODELS["files_runner"]))
        for r in p2["_regions"]:
            if r["state"] != "ready":
                continue
            # A region that would take the batch past the limit starts its
            # own job (round 101: one unit was glued to a 22-unit region).
            if batch and sum(len(x["units"]) for x in batch) + len(r["units"]) > REGION_UNITS:
                flush()
            batch.append(r)
            if sum(len(x["units"]) for x in batch) >= REGION_UNITS:
                flush()
        flush()
    if t["9"]["status"] == "open":
        # globals is a mechanical rename list: Sonnet (revision 39)
        q9 = [("9", f"{k}: {TRACK9_ITEMS[k]}", "sonnet" if k == "globals" else "opus")
              for k, done in t["9"]["checklist"].items() if not done]
    q3 = []
    for k, (_, items) in PHASE3.items():
        if t[k]["status"] != "open":
            continue
        ck = t[k]["checklist"]
        for i, (desc, model, after) in items.items():
            if ck[i] or not all(ck[x] for x in after) or model is None:
                continue
            if model == "premium":
                q3.append((k, f"HEAD (premium) setup {i}: {desc} ({"docs/CLEANUP.md" if k in PHASE4 else "FINISHING-PLAN"} track {k})",
                           MODELS["head_when_new_procedure"]))
            elif k == "12":
                us = [Path(f).stem for f in apidoc.areas().get(i, []) if f.endswith(".c")]
                q3.append((k, f"{i}: {desc} (units: {','.join(us)})", model))
            else:
                q3.append((k, f"{i}: {desc}", model))
    queues = [q_fresh, q_naming, q_stall, q_sdk, q_revisit, q_promote, q_types, q_close, q6, q7, q8, q9, q3]
    order = []
    while any(queues):
        for q in queues:
            if q:
                order.append(q.pop(0))
    return select_jobs(d, order, n)


RENAMES = ("naming pass on", "identify and name", "extern review")
JOB_UNIT_RES = (r"naming pass on (\w+)", r"runner on (\w+):", r"in (\w+) to #ifdef", r"match \w+ \((\w+),", r"match fresh ground in (\w+) \(")
RENAMES = RENAMES + ("unify class", "unify global")   # rewrites accessors in every unit that sees the type


def job_units(d, desc):
    """The units a job's runner edits: its one unit, or for the track-2 batch
    every unit holding one of the listed functions."""
    m = re.search(r"\(units: ([\w,./-]*)\)$", desc)
    if m:
        return set(filter(None, m.group(1).split(",")))
    for rx in JOB_UNIT_RES:
        m = re.search(rx, desc)
        if m:
            return {m.group(1)}
    if desc.startswith("identify and name"):
        want = set(d["tracks"]["2"]["unnamed_list"])
        us = set()
        for c in srcpath.src_files():
            t = c.read_text(errors="replace")
            if any(re.search(rf"\b{re.escape(f)}\b", t) for f in want):
                us.add(c.stem)
        return us
    return set()


def select_jobs(d, order, n):
    """Take jobs in round-robin order, DEFERRING one whose units a renaming job
    already taken would rewrite (or that would rewrite a taken job's units):
    call-graph contention, headercontention.py's test. Rounds 73 to 75 each
    skipped two or three top jobs by hand for exactly this (plan revision 15).
    Returns the first n kept; d["_deferred"] gets (job, reason, taken job) for the rest
    that collided before n were kept."""
    import headercontention
    taken, deferred = [], []
    pair_cache = {}

    def rewrites(ua, ub):
        """Does a RENAME in units ua rewrite a file in units ub?"""
        key = (frozenset(ua), frozenset(ub))
        if key not in pair_cache:
            us = sorted(ua | ub)
            hits = headercontention.call_contention(us) if len(us) > 1 else []
            pair_cache[key] = any(a in ua and b in ub for a, b, _ in hits)
        return pair_cache[key]

    for job in order:
        if len(taken) >= n:
            break
        units = job_units(d, job[1])
        reason = None
        for tj, tu in taken:
            if units & tu:
                reason = "same unit as {#}"
            elif job[1].startswith("unify class") and tj[1].startswith("unify class"):
                # a class job renames only its own table and methods, and
                # class_footprint already holds every unit naming them, so
                # disjoint footprints ARE the rename test (round 87: eleven
                # class merges, no conflict beyond adjacent lines)
                pass
            elif units and tu and ((job[1].startswith(RENAMES) and rewrites(units, tu))
                                   or (tj[1].startswith(RENAMES) and rewrites(tu, units))):
                reason = "call-graph contention with {#} (a rename in one rewrites the other)"
            if reason:
                break
        if reason:
            # `{#}` is resolved at print time against the SHOWN numbering: a
            # premium job is taken here but printed apart, so a taken-list index
            # was off by one for every job after it (revision 18).
            deferred.append((job, reason, tj))
        else:
            taken.append((job, units))
    d["_deferred"] = deferred
    return [j for j, _ in taken]


def print_status(d, n, st):
    p = d["progress"]
    t = d["tracks"]
    print("LSD: Dream Emulator plan (docs/CLEANUP.md; tracks 1-13 archived on archive/process), measured now")
    if not d["elf_present"]:
        print("  WARNING: build/lsdde.elf missing; run ./build-and-verify.sh first. Counts below are partial.")
    print(f"  game functions matched {p['matched']}/{p['game']}; queued {p['queued']} "
          f"(stalled {p['stalled']}, fresh {p['fresh']}); uncarved {p['uncarved']}")
    print()
    print("  track  status     measured")
    t1 = t["1"]
    per = calibration_tally(st)
    rv = [r for r in st["tracks"]["1"]["rounds"] if not r.get("calibration", True) and "REVISIT" in r.get("note", "").upper()]
    rv_m, rv_a = sum(r.get("matches", 0) for r in rv), sum(r.get("attempts", 0) for r in rv)
    print(f"  1      {t1['status']:<10} stall matching: {t1['fresh']} fresh, {t1['stalled']} stalled; "
          f"{len(t1['rounds'])} round(s) recorded; calibration sonnet {per['sonnet'][0]}/{per['sonnet'][1]} "
          f"opus {per['opus'][0]}/{per['opus'][1]} (matches/attempts); "
          f"next runner model {t1['next_match_model']} ({t1['why']})")
    if t1["reason"]:
        print(f"                    {t1['reason']}")
    if t1["revisit"]:
        print(f"                    {t1['revisit']} stall(s) awaiting a {d['_revisit_label']}; revisit yield so far "
              f"{rv_m}/{rv_a} (matches/attempts, from rounds recorded with REVISIT in the note)")
    print(f"  1b     {t['1b']['status']:<10} NON_MATCHING bodies: {t['1b']['nm_bodies']} in src, "
          f"{t['1b']['promotable']} stall(s) with a preserved body and none yet")
    print(f"  2      {t['2']['status']:<10} SDK call surface: {t['2']['named']} named, "
          f"{t['2']['unnamed']} still func_, {t['2']['parked']} parked (unidentified, done for this track); "
          f"{len(t['2']['sony_data_game_named'])} Sony data symbol(s) with a game name (tools/sonydata.py)")
    t3 = t["3"]
    print(f"  3      {t3['status']:<10} readability: {t3['units_done']}/{t3['units_total']} units passed"
          f"{' (' + str(t3['units_fresh']) + ' wait for track 1: fresh ground)' if t3.get('units_fresh') else ''}; "
          f"{t3['func_named_defs']}/{t3['defs']} defs still func_; {t3['unk_refs']} unk refs, "
          f"{t3['slot_refs']} slotNN calls, {t3['d_refs']} D_ globals; naming runner {t3['naming_model']}")
    t4 = t["4"]
    print(f"  4      {t4['status']:<10} types: {t4['classes_unified']}/{t4['classes_total']} classes unified "
          f"({t4['classes_parked']} parked, {len(t4['classes_ready'])} ready); {t4['shared_globals']} global(s) "
          f"declared with more than one type; {t4['local_struct_views']} unit-local struct typedefs in src/")
    if t4["classes_incomplete"]:
        print(f"                    UNIFIED BUT A VIEW REMAINS OUTSIDE ITS HEADER: {', '.join(t4['classes_incomplete'])}"
              " (plan.py classes)")
    c5 = t["5"]["checklist"]
    print(f"  5      {t['5']['status']:<10} close-out: {sum(c5.values())}/{len(c5)} items ticked")
    t6, t7, t8, t9 = t["6"], t["7"], t["8"], t["9"]
    setup = lambda tt: "; setup " + ", ".join(f"{k} {'ticked' if v else 'NOT DONE'}" for k, v in tt["setup"].items())
    print("  -- phase 2: the code reads like a game's source (revision 27; python3 tools/readability.py)")
    print(f"  6      {t6['status'].split(' (')[0]:<10} type names: {t6['placeholder_types']} placeholder type name(s), "
          f"{t6['class_jobs']} class(es) with a placeholder name or table, {t6['type_homes']} other file(s) defining them; "
          f"{t6['ph_prefix_defs']} defs under a placeholder class prefix; {t6['parked']} parked; "
          f"{t6['sony_headers']} header(s) and {t6['sony_units']} unit(s) re-declare a Sony name"
          f"{setup(t6)}")
    tt7 = t7["totals"]
    print(f"  7      {t7['status'].split(' (')[0]:<10} polish: {t7['units_done']}/{t7['units_total']} units passed; "
          + ", ".join(f"{tt7[k]} {k}" for k in tt7) + f"; polish runner {t7['model']}{setup(t7)}")
    print(f"  8      {t8['status'].split(' (')[0]:<10} files: {t8['regions_done']}/{t8['regions']} regions done, "
          f"{t8['regions_ready']} ready; {t8['placeholder_units']} unit(s) and {t8['placeholder_headers']} "
          f"header(s) still code_/class_<hex>; TU evidence model "
          f"{'valid' if t8['tu_model_valid'] else 'INVALID (tools/tuboundary.py validation failed)'}")
    c9 = t9["checklist"]
    print(f"  9      {t9['status'].split(' (')[0]:<10} close-out: {sum(c9.values())}/{len(c9)} items ticked; "
          f"{t9['history']} history mention(s) left in comments")
    for k in PHASE3:
        if k == "10":
            print("  -- phase 3: the tree is ready to publish (revisions 41, 43)")
        if k == "14":
            print("  -- phase 4: the code reads like the game's source (docs/CLEANUP.md)")
        ck = t[k]["checklist"]
        left = [i for i, v in ck.items() if not v]
        ops = [i for i in left if PHASE3[k][1][i][1] is None]
        extra = ""
        if k == "11":
            import unitfile
            extra = f"; {len(unitfile.not_snake())} game file(s) not snake_case (tools/unitfile.py check)"
        if k == "12" and not t[k]["status"].startswith("waiting"):
            left = defaultdict_count(x[1] for v in apidoc.census().values() for x in v)
            extra = (f"; apidoc.py: {sum(left.values())} left (" + ", ".join(f"{left[w]} {w}" for w in apidoc.KINDS if w in left)
                     + ")" if left else "; apidoc.py clean")
        print(f"  {k:<6} {t[k]['status'].split(' (')[0]:<10} {t[k]['title']}: {sum(ck.values())}/{len(ck)} "
              f"items ticked" + (f"; operator decision: {', '.join(ops)}" if ops else "") + extra)
    for k in ("6", "7", "8", "9", *PHASE3):
        if t[k]["status"].startswith("waiting"):
            print(f"         track {k}: {t[k]['status']}")
    print()
    over = [(k, v) for k, v in d["docs"].items() if v["over"]]
    if over:
        print("  DOC BUDGET EXCEEDED (a rule states its reason in a sentence; the story goes to PROGRESS.md):")
        for k, v in over:
            print(f"    {k}: {v['words']} words, budget {v['budget']} (distil to docs/archive/; a reflow changes nothing here)")
        print()
    print(f"  READY JOBS (top {n}; head fills the operator's runner cap from the top, 5 if none is stated,"
          f" never more than 5; one unit per runner):")
    import headercontention
    allj = jobs(d, n + 3)
    premium = [j for j in allj if j[2] == MODELS["head_when_new_procedure"]]
    shown, heads = [j for j in allj if j not in premium][:n], []
    for i, (track, desc, model) in enumerate(shown, 1):
        hs = set().union(*(headercontention.unit_headers(u) for u in job_units(d, desc))) \
            if job_units(d, desc) else set()
        share = [f"{h} with #{j}" for j, prev in enumerate(heads, 1) for h in sorted(hs & prev)]
        heads.append(hs)
        note = f"   (shares {', '.join(share[:2])}: header edits additive, PARALLEL-RUNS 2.1; not a reason to reorder)" \
            if share else ""
        print(f"    {i:>2}. [{track:<2}] {model:<7} {desc}{note}")
    for track, desc, model in premium:
        print(f"  FOR A PREMIUM SESSION (not a runner slot; an ordinary head reports it and staffs the rest):")
        print(f"        [{track:<2}] {desc}")
    if d.get("_deferred"):
        print("  DEFERRED (would collide with a job above; staff after that one merges, or in a later slot):")
        for (track, desc, model), why, tj in d["_deferred"]:
            ref = f"#{shown.index(tj) + 1}" if tj in shown else "the premium job"
            why = why.replace("{#}", ref)
            print(f"        [{track:<2}] {model:<7} {desc[:90]}{'...' if len(desc) > 90 else ''}  <- {why}")
    if d.get("_waiting"):
        print("  WAITING (flagged --after: staff once every file named has left tools/sonyheaders.py's list):")
        for desc, on in d["_waiting"]:
            print(f"        [6 ] {desc}  <- waits on {', '.join(on)}")
    review = [u for u in d["_todo3"] if t["3"]["status"] != "done" and not any(d["units"][u][k] for k in
              ("func_named", "unk_refs", "slot_refs", "d_refs"))]
    if review:
        print(f"  REVIEW-ONLY (unmarked, zero measured naming debt; the head reviews and mark-units, no runner):")
        print(f"    {' '.join(review)}")
    print()
    print("  Models: head runs on", MODELS["head"], "unless the round writes a new procedure, tool or doc,",
          "or adjudicates a HARD RULE or toolchain lead: then", MODELS["head_when_new_procedure"] + ".")
    print("  Ledger: config/plan-state.json (record-round / mark-unit [--track 7] / mark-class / flag-type / park-type"
          " / set-track / check / set-model).")


def print_classes(d):
    """Track 4, per class, in tree order: state, subtree size, C methods, views."""
    kids = {}
    for c in d["_classes"]:
        kids.setdefault(c["parent"], []).append(c)

    def show(c, depth):
        tag = {"unified": "UNIFIED", "parked": "PARKED", "ready": "ready", "waiting": "waiting",
               "no C": "no C", "no C yet": "no C yet"}[c["state"]]
        name = f" = {c['name']}" if c.get("name") else ""
        if c.get("parent_by") == "ctor":
            name += "  (parent by CTOR CHAIN, not by id: the id tree is wrong here)"
        print(f"  {'  ' * depth}0x{c['id']:X} {c['table']}{name}  [{tag}]  below={c['below']} "
              f"C={len(c['owned_c'])}/{len(c['owned'])} views={c['nviews']}"
              + (f"  in {', '.join(c['files'])}" if c["files"] and c["state"] != "unified" else ""))
        for sv in c["stray"]:
            print(f"  {'  ' * depth}    STRAY VIEW: {sv}")
        for k in sorted(kids.get(c["table"], []), key=lambda k: k["id"]):
            show(k, depth + 1)
    for r in sorted(kids.get(None, []), key=lambda k: k["id"]):
        show(r, 0)
    print("\n  C=a/b: a of the class's b own methods are C. 'no C': no own method is C and no view exists;")
    print("  'no C yet': the same, but its methods are carved INCLUDE_ASM game code, track 1's ground")
    print("  (revision 18). Neither blocks its subclasses; a 'no C yet' class turns ready once one matches.")


def print_regions(d):
    """Track 8: every run of game units between two non-game objects, text order."""
    for r in d["_p2"]["_regions"]:
        head = f"{r['units'][0]}..{r['units'][-1]}" if len(r["units"]) > 1 else r["units"][0]
        print(f"  [{r['state']:<7}] {head}  ({len(r['units'])} unit(s), {len(r['ph_units'])} placeholder)")
        if len(r["units"]) > 1:
            print(f"            {' '.join(r['units'])}")
        for j in r["joins"]:
            print(f"            rodata: {j} is one file with the unit before it")
        for s_ in r["splits"]:
            print(f"            rodata: a file boundary lies between {s_[0]} and {s_[1]}")
        if r["state"] == "waiting":
            print(f"            waits for: {'; '.join(r['blockers'][:4])}{' ...' if len(r['blockers']) > 4 else ''}")
    print("\n  Evidence: python3 tools/tuboundary.py --unit <unit>. 'waiting' = a unit still needs track 7's pass or")
    print("  track 6's names (FINISHING-PLAN track 8: a file is named once its contents are).")


def print_units(d):
    print(f"  {'unit':<16} {'done':>4} {'defs':>5} {'func_':>5} {'unk':>5} {'slot':>5} {'D_':>4} {'asm':>4} {'NM':>3} {'centr':>5}")
    for u, m in sorted(d["units"].items(), key=lambda kv: -kv[1]["centrality"]):
        done = "no" if u in d["_todo3"] else "yes"
        print(f"  {u:<16} {done:>4} {m['defs']:>5} {m['func_named']:>5} {m['unk_refs']:>5} "
              f"{m['slot_refs']:>5} {m['d_refs']:>4} {m['include_asm']:>4} {m['nm_bodies']:>3} {m['centrality']:>5}")


def main():
    ap = argparse.ArgumentParser(description="finishing-plan status and ledger")
    ap.add_argument("--json", action="store_true")
    ap.add_argument("--n", type=int, default=12)
    sub = ap.add_subparsers(dest="cmd")
    j = sub.add_parser("jobs")
    j.add_argument("--n", type=int, default=12, dest="n_jobs")
    sub.add_parser("units")
    sub.add_parser("classes")
    mc = sub.add_parser("mark-class")
    mc.add_argument("--table", required=True, help="the method-table symbol (typeviews.py --census)")
    mc.add_argument("--class", dest="klass", help="the class's type name; its header is found by plan.class_header")
    mc.add_argument("--header", help="the class's header, when the lookup cannot tell (include/<file>.h)")
    mc.add_argument("--park", default="", help="park instead of unify: the reason (track 4 park rule)")
    mc.add_argument("--undo", action="store_true")
    r = sub.add_parser("record-round")
    r.add_argument("--track", required=True)
    r.add_argument("--round", type=int, required=True)
    r.add_argument("--model", required=True, choices=["sonnet", "opus", "premium", "fable"])
    r.add_argument("--runners", type=int, required=True)
    r.add_argument("--attempts", type=int, required=True)
    r.add_argument("--matches", type=int, required=True)
    r.add_argument("--note", default="")
    r.add_argument("--not-calibration", action="store_true",
                   help="this round's attempts were NOT from the ranked stall band (fresh "
                        "giants, revisits); recorded, but not counted toward calibration "
                        "or the stop rule. Any number of band attempts counts, even one.")
    am = sub.add_parser("amend-round", help="correct a recorded round (round 86: a wrong figure in a note)")
    am.add_argument("--track", required=True)
    am.add_argument("--round", type=int, required=True)
    am.add_argument("--note", help="replacement note")
    am.add_argument("--attempts", type=int)
    am.add_argument("--matches", type=int)
    am.add_argument("--runners", type=int)
    am.add_argument("--model", help="which entry, when the round recorded one per model (round 102)")
    am.add_argument("--why", required=True, help="what was wrong; kept in the entry's `amended` list")
    m = sub.add_parser("mark-unit")
    m.add_argument("--unit", required=True)
    m.add_argument("--track", default="3", choices=["3", "7"])
    m.add_argument("--undo", action="store_true")
    sub.add_parser("regions")
    for cmd in ("flag-type", "park-type"):
        ft = sub.add_parser(cmd)
        ft.add_argument("--name", required=True)
        ft.add_argument("--reason", default="")
        ft.add_argument("--undo", action="store_true")
        if cmd == "flag-type":
            ft.add_argument("--after", default="", help="comma-separated files whose Sony-name collisions "
                            "(tools/sonyheaders.py) must clear before this job is staffed")
            ft.add_argument("--units", default="", help="comma-separated units (src/ stems or include/ "
                            "paths) the fix also edits beyond the defining file; they join the job's edit set")
    s = sub.add_parser("set-track")
    s.add_argument("--track", required=True)
    s.add_argument("--status", required=True, choices=["open", "parked", "done", "auto"])
    s.add_argument("--reason", default="")
    c = sub.add_parser("check")
    c.add_argument("--item", required=True, choices=sorted({k for v in CHECK_ITEMS.values() for k in v}))
    c.add_argument("--track", choices=sorted(CHECK_ITEMS), help="only needed when two tracks share an item name")
    c.add_argument("--undo", action="store_true")
    sm = sub.add_parser("set-model")
    sm.add_argument("--role", required=True, choices=["match_runner", "naming_runner", "polish_runner"])
    sm.add_argument("--model", required=True, choices=["sonnet", "opus", "auto"])
    a = ap.parse_args()
    if getattr(a, "n_jobs", None):
        a.n = a.n_jobs
    os.chdir(ROOT)
    st = load_state()
    today = datetime.date.today().isoformat()

    if a.cmd == "record-round":
        st["tracks"].setdefault(a.track, {"status": "open", "reason": "", "rounds": []})
        st["tracks"][a.track].setdefault("rounds", []).append({
            "round": a.round, "date": today, "model": a.model, "runners": a.runners,
            "attempts": a.attempts, "matches": a.matches, "note": a.note,
            "calibration": not a.not_calibration})
        save_state(st)
        if a.track == "1" and not a.not_calibration:
            print("counted toward calibration (attempts from the ranked stall band; "
                  "use --not-calibration for fresh giants or revisits).")
        print(f"recorded round {a.round} on track {a.track}: {a.matches} match(es) from "
              f"{a.attempts} attempt(s) by {a.runners} {a.model} runner(s)")
        return
    if a.cmd == "amend-round":
        rs = [r for r in st["tracks"].get(a.track, {}).get("rounds", []) if r.get("round") == a.round
              and (a.model is None or r.get("model") == a.model)]
        if len(rs) != 1:
            sys.exit(f"FATAL: {len(rs)} recorded round(s) {a.round} on track {a.track}; need exactly one")
        r, old_ = rs[0], {}
        for k in ("note", "attempts", "matches", "runners"):
            v = getattr(a, k)
            if v is not None and v != r.get(k):
                old_[k], r[k] = r.get(k), v
        if not old_:
            sys.exit("nothing to change")
        r.setdefault("amended", []).append({"date": today, "why": a.why, "was": old_})
        save_state(st)
        print(f"amended round {a.round} on track {a.track}: {', '.join(old_)} ({a.why})")
        return
    if a.cmd == "mark-unit":
        if not srcpath.unit_src(a.unit):
            sys.exit(f"FATAL: no such unit {a.unit}")
        ud = st["tracks"][a.track].setdefault("units_done", {})
        if a.undo:
            ud.pop(a.unit, None)
        else:
            ud[a.unit] = today
        save_state(st)
        print(f"track {a.track}: {a.unit} {'un' if a.undo else ''}marked")
        return
    if a.cmd in ("flag-type", "park-type"):
        key = "flagged" if a.cmd == "flag-type" else "parked"
        led = st["tracks"]["6"].setdefault(key, {})
        if a.undo:
            led.pop(a.name, None)
        else:
            if not a.reason:
                sys.exit("FATAL: --reason is required (what the name hides, or why it stays)")
            led[a.name] = f"{a.reason} ({today})"
        after = st["tracks"]["6"].setdefault("after", {})
        if a.cmd == "flag-type" and (a.undo or a.after):
            after.pop(a.name, None)
            if not a.undo:
                after[a.name] = [f.strip() for f in a.after.split(",") if f.strip()]
        units = st["tracks"]["6"].setdefault("units", {})
        if a.cmd == "flag-type" and (a.undo or a.units):
            units.pop(a.name, None)
            if not a.undo:
                units[a.name] = [f.strip() for f in a.units.split(",") if f.strip()]
        save_state(st)
        print(f"track 6: {a.name} {'un' if a.undo else ''}{key}")
        return
    if a.cmd == "mark-class":
        led = st["tracks"]["4"].setdefault("classes", {})
        if a.undo:
            led.pop(a.table, None)
            save_state(st)
            print(f"track 4: {a.table} unmarked")
            return
        import typeviews
        tabs, _, _, _ = typeviews.class_tree()
        if a.table not in {t["name"] for t in tabs.values()}:
            sys.exit(f"FATAL: {a.table} is not a method table (typeviews.py --tree)")
        if not a.park and not a.klass:
            sys.exit("FATAL: --class is required to mark a class unified")
        hdr = (a.header or class_header(a.klass)) if a.klass else None
        if hdr and not (ROOT / hdr).exists():
            sys.exit(f"FATAL: {hdr} does not exist; a unified class has its own header")
        led[a.table] = {"class": a.klass, "header": hdr, "date": today,
                        **({"parked": a.park} if a.park else {})}
        save_state(st)
        print(f"track 4: {a.table} {'parked' if a.park else 'unified as ' + a.klass + ' (' + hdr + ')'}")
        return
    if a.cmd == "set-track":
        st["tracks"].setdefault(a.track, {})
        st["tracks"][a.track]["status"] = a.status
        st["tracks"][a.track]["reason"] = a.reason + (f" ({today})" if a.reason else "")
        save_state(st)
        print(f"track {a.track}: {a.status}")
        return
    if a.cmd == "check":
        owners = [t for t, items in CHECK_ITEMS.items() if a.item in items and (not a.track or t == a.track)]
        if len(owners) != 1:
            sys.exit(f"FATAL: item {a.item} belongs to track(s) {owners}; pass --track")
        st["tracks"][owners[0]].setdefault("checklist", {})[a.item] = not a.undo
        save_state(st)
        print(f"track {owners[0]}: {a.item} {'un' if a.undo else ''}ticked")
        return
    if a.cmd == "set-model":
        st["models"][a.role] = None if a.model == "auto" else a.model
        save_state(st)
        print(f"{a.role}: {a.model}")
        return

    if not STATE.exists():
        save_state(st)
    d = collect(st)
    if a.json:
        out = {k: v for k, v in d.items() if not k.startswith("_")}
        out["jobs"] = jobs(d, a.n)
        print(json.dumps(out, indent=2, default=str))
    elif a.cmd == "jobs":
        for track, desc, model in jobs(d, a.n):
            print(f"[{track:<2}] {model:<7} {desc}")
    elif a.cmd == "units":
        print_units(d)
    elif a.cmd == "classes":
        print_classes(d)
    elif a.cmd == "regions":
        print_regions(d)
    else:
        print_status(d, a.n, st)


if __name__ == "__main__":
    main()
