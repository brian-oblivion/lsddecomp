#!/usr/bin/env python3
"""The finishing plan, measured: which tracks are open, what is ready to run,
and on which model.

    python3 tools/plan.py                      # status + ready jobs
    python3 tools/plan.py --json
    python3 tools/plan.py jobs [--n 12]        # the ready-jobs list only
    python3 tools/plan.py units                # per-unit readability table

    python3 tools/plan.py record-round --track 1 --round N --model sonnet|opus \\
                          --runners R --attempts A --matches M [--note "..."] [--not-calibration]
    python3 tools/plan.py mark-unit --unit <unit> [--undo]        # track 3 pass done
    python3 tools/plan.py set-track --track N --status open|parked|done --reason "..."
    python3 tools/plan.py check --item <id> [--undo]              # track 5 checklist
    python3 tools/plan.py set-model --role naming_runner|match_runner --model sonnet|opus

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
    "asm-sites": "every live __asm__ site justified or retired (func_800195EC question, the bare barriers)",
    "docs-budget": "every doc within its word budget; archive holds the history",
    "nonmatching-clean": "tools/check-nonmatching.sh green and every stall has a NON_MATCHING body or a written reason",
}

DEFAULT_STATE = {
    "stop_rule": {"attempts_per_model": 6, "min_matches": 3},
    "models": {"match_runner": None, "naming_runner": None},
    "tracks": {
        "1": {"status": "open", "reason": "", "rounds": []},
        "1b": {"status": "auto", "reason": ""},
        "2": {"status": "open", "reason": ""},
        "3": {"status": "open", "reason": "", "units_done": {}},
        "4": {"status": "auto", "reason": ""},
        "5": {"status": "auto", "reason": "", "checklist": {}},
    },
}

# A splat placeholder, or the tier-C method form `Class__func_xxxxx` that keeps
# the address in the name: both count as NOT YET NAMED (FINISHING-PLAN track 3).
FUNC_PH = re.compile(r"^(?:\w+__)?func_(?:800)?[0-9A-Fa-f]{5}$")
DEF_RE = progress.DEF_RE
NOT_DEF = {"if", "while", "for", "switch", "do", "return", "sizeof"}


def load_state():
    if STATE.exists():
        st = json.loads(STATE.read_text())
    else:
        st = json.loads(json.dumps(DEFAULT_STATE))
    # fill in any key a newer version of this tool added
    for k, v in DEFAULT_STATE.items():
        st.setdefault(k, v)
    for t, v in DEFAULT_STATE["tracks"].items():
        st["tracks"].setdefault(t, v)
    return st


def save_state(st):
    STATE.write_text(json.dumps(st, indent=2, sort_keys=True) + "\n")


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
    for c in srcpath.src_files():
        live = progress.strip_dead_code(c.read_text(errors="replace"))
        referenced.update(re.findall(r"\b([A-Za-z_]\w*)\s*\(", live))
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
    todo3 = sorted((u for u in units if u not in done and (units[u]["defs"] or units[u]["include_asm"])),
                   key=lambda u: (-units[u]["centrality"], -units[u]["func_named"]))
    pct3 = len(done) / max(1, len(units))

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
    if t4["status"] == "auto":
        t4_status = "open" if pct3 >= 0.8 else "waiting (opens at 80% of units through track 3)"
    else:
        t4_status = t4["status"]

    t5 = st["tracks"]["5"]
    ticked = t5["checklist"]
    if t5["status"] == "auto":
        t5_status = "open" if (not todo3 and t4_status in ("done",)) else "waiting (opens when tracks 3 and 4 are done)"
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
            "2": {"status": "open" if unnamed_sdk else "done",
                  "named": len(named_sdk), "unnamed": len(unnamed_sdk), "parked": len(parked_sdk),
                  "unnamed_list": unnamed_sdk},
            "3": {"status": "done" if not todo3 else st["tracks"]["3"]["status"],
                  "units_total": len(units), "units_done": len(done),
                  "func_named_defs": sum(u["func_named"] for u in units.values()),
                  "defs": sum(u["defs"] for u in units.values()),
                  "unk_refs": sum(u["unk_refs"] for u in units.values()),
                  "slot_refs": sum(u["slot_refs"] for u in units.values()),
                  "d_refs": sum(u["d_refs"] for u in units.values()),
                  "naming_model": st["models"].get("naming_runner") or MODELS["naming_runner_default"]},
            "4": {"status": t4_status,
                  "local_struct_views": sum(len(re.findall(r"^typedef struct", (ROOT / "src" / f"{u}.c").read_text(errors='replace'), re.M)) for u in units)},
            "5": {"status": t5_status, "checklist": {k: bool(ticked.get(k)) for k in TRACK5_ITEMS}},
        },
        "docs": docs,
        "units": units,
        "_fresh": fresh_funcs, "_stalls": stall_rows, "_promotable": promotable,
        "_todo3": todo3, "_revisit": revisit, "_revisit_label": revisit_label,
        "_t4_rounds": st["tracks"]["4"].get("rounds", []),
    }


_nm_cache = {}


def nm_defined(unit):
    if unit not in _nm_cache:
        p = srcpath.unit_src(unit)
        _nm_cache[unit] = nm_bodies(p.read_text(errors="replace")) if p else set()
    return _nm_cache[unit]


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
    q_fresh = [("1", f"match {func} ({unit}, {words}w, fresh)", t["1"]["next_match_model"])
               for words, unit, func, title in sorted(d["_fresh"])]
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
    first_class = not d.get("_t4_rounds")
    q_types = [("4", (f"HEAD (premium, FINISHING-PLAN track 4): unify the FIRST class yourself and write the "
                      f"recipe into track 4; {t['4']['local_struct_views']} unit-local struct views remain")
                if first_class else
                f"unify {t['4']['local_struct_views']} unit-local struct views into shared "
                "headers, one class at a time",
                MODELS["head_when_new_procedure"] if first_class else "opus")] \
        if t["4"]["status"] == "open" else []
    q_close = [("5", f"{k}: {TRACK5_ITEMS[k]}", "opus")
               for k, done in t["5"]["checklist"].items() if not done] if t["5"]["status"] == "open" else []
    queues = [q_fresh, q_naming, q_stall, q_sdk, q_revisit, q_promote, q_types, q_close]
    order = []
    while any(queues):
        for q in queues:
            if q:
                order.append(q.pop(0))
    return select_jobs(d, order, n)


RENAMES = ("naming pass on", "identify and name", "extern review")
JOB_UNIT_RES = (r"naming pass on (\w+)", r"runner on (\w+):", r"in (\w+) to #ifdef", r"match \w+ \((\w+),")


def job_units(d, desc):
    """The units a job's runner edits: its one unit, or for the track-2 batch
    every unit holding one of the listed functions."""
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
    Returns the first n kept; d["_deferred"] gets (job, reason) for the rest
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
        for i, (tj, tu) in enumerate(taken, 1):
            if units & tu:
                reason = f"same unit as #{i}"
            elif units and tu and ((job[1].startswith(RENAMES) and rewrites(units, tu))
                                   or (tj[1].startswith(RENAMES) and rewrites(tu, units))):
                reason = f"call-graph contention with #{i} (a rename in one rewrites the other)"
            if reason:
                break
        if reason:
            deferred.append((job, reason))
        else:
            taken.append((job, units))
    d["_deferred"] = deferred
    return [j for j, _ in taken]


def print_status(d, n, st):
    p = d["progress"]
    t = d["tracks"]
    print("LSD: Dream Emulator finishing plan (docs/FINISHING-PLAN.md), measured now")
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
          f"{t['2']['unnamed']} still func_, {t['2']['parked']} parked (unidentified, done for this track)")
    t3 = t["3"]
    print(f"  3      {t3['status']:<10} readability: {t3['units_done']}/{t3['units_total']} units passed; "
          f"{t3['func_named_defs']}/{t3['defs']} defs still func_; {t3['unk_refs']} unk refs, "
          f"{t3['slot_refs']} slotNN calls, {t3['d_refs']} D_ globals; naming runner {t3['naming_model']}")
    print(f"  4      {t['4']['status']:<10} types: {t['4']['local_struct_views']} unit-local struct typedefs in src/")
    c5 = t["5"]["checklist"]
    print(f"  5      {t['5']['status']:<10} close-out: {sum(c5.values())}/{len(c5)} items ticked")
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
        for (track, desc, model), why in d["_deferred"]:
            print(f"        [{track:<2}] {model:<7} {desc[:90]}{'...' if len(desc) > 90 else ''}  <- {why}")
    review = [u for u in d["_todo3"] if not any(d["units"][u][k] for k in
              ("func_named", "unk_refs", "slot_refs", "d_refs"))]
    if review:
        print(f"  REVIEW-ONLY (unmarked, zero measured naming debt; the head reviews and mark-units, no runner):")
        print(f"    {' '.join(review)}")
    print()
    print("  Models: head runs on", MODELS["head"], "unless the round writes a new procedure, tool or doc,",
          "or adjudicates a HARD RULE or toolchain lead: then", MODELS["head_when_new_procedure"] + ".")
    print("  Ledger: config/plan-state.json (record-round / mark-unit / set-track / check / set-model).")


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
    m = sub.add_parser("mark-unit")
    m.add_argument("--unit", required=True)
    m.add_argument("--undo", action="store_true")
    s = sub.add_parser("set-track")
    s.add_argument("--track", required=True)
    s.add_argument("--status", required=True, choices=["open", "parked", "done", "auto"])
    s.add_argument("--reason", default="")
    c = sub.add_parser("check")
    c.add_argument("--item", required=True, choices=sorted(TRACK5_ITEMS))
    c.add_argument("--undo", action="store_true")
    sm = sub.add_parser("set-model")
    sm.add_argument("--role", required=True, choices=["match_runner", "naming_runner"])
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
    if a.cmd == "mark-unit":
        if not srcpath.unit_src(a.unit):
            sys.exit(f"FATAL: no such unit {a.unit}")
        ud = st["tracks"]["3"]["units_done"]
        if a.undo:
            ud.pop(a.unit, None)
        else:
            ud[a.unit] = today
        save_state(st)
        print(f"track 3: {a.unit} {'un' if a.undo else ''}marked")
        return
    if a.cmd == "set-track":
        st["tracks"].setdefault(a.track, {})
        st["tracks"][a.track]["status"] = a.status
        st["tracks"][a.track]["reason"] = a.reason + (f" ({today})" if a.reason else "")
        save_state(st)
        print(f"track {a.track}: {a.status}")
        return
    if a.cmd == "check":
        st["tracks"]["5"]["checklist"][a.item] = not a.undo
        save_state(st)
        print(f"track 5: {a.item} {'un' if a.undo else ''}ticked")
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
    else:
        print_status(d, a.n, st)


if __name__ == "__main__":
    main()
