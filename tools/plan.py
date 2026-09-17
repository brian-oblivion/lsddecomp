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
  - doc line budgets, so the sprawl this plan replaced does not grow back.
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
MODELS = {
    "head": "opus",
    "head_when_new_procedure": "fable",
    "match_runner_default": "sonnet",
    "naming_runner_default": "opus",
    "mechanical_runner": "sonnet",
}

# Line budgets. A doc over budget is a warning here and a job in track 5.
DOC_BUDGETS = {
    "CLAUDE.md": 800,
    "docs/FINISHING-PLAN.md": 600,
    "docs/PARALLEL-RUNS.md": 500,
    "docs/DECOMPILATION_LEARNINGS.md": 800,
    "docs/MATCHING-GUIDE.md": 700,
    "docs/SDK-OBJECTS-GUIDE.md": 400,
    "docs/SDK-OBJECTS-RUNS.md": 300,
}

TRACK5_ITEMS = {
    "readme": "reader-facing README.md: what the game's code is, how it is organised, how to build",
    "credits": "CREDITS.md current for every inherited name and tool",
    "asm-sites": "every live __asm__ site justified or retired (func_800195EC question, the bare barriers)",
    "docs-budget": "every doc within its line budget; archive holds the history",
    "nonmatching-clean": "tools/check-nonmatching.sh green and every stall has a NON_MATCHING body or a written reason",
}

DEFAULT_STATE = {
    "stop_rule": {"calibration_rounds": 2, "min_matches": 3},
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
DEF_RE = re.compile(r"^\w[^;=]*?\b(\w+)\s*\([^;{]*\)\s*\{", re.M)
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
            defs = [d for d in defs if d in info]
        inc = progress.INCLUDE_RE.findall(live)
        nm = nm_bodies(raw)
        units[c.stem] = {
            "defs": len(defs),
            "func_named": sum(1 for d in defs if FUNC_PH.match(d)),
            "include_asm": len(inc),
            "nm_bodies": len(nm & set(inc)),
            "unk_refs": len(re.findall(r"(?:->|\.)unk_?0?x?[0-9A-Fa-f]+", live)),
            "slot_refs": len(re.findall(r"->slot0?x?[0-9A-Fa-f]+\s*\(", live)),
            "d_refs": len(set(re.findall(r"\bD_800[0-9A-F]{5}\b", live))),
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


def sdk_surface(info, units):
    """Functions game code calls that live in Psy-Q segments: named vs func_."""
    info = {n: (a, 0) for n, a in all_symbols().items()} or info
    referenced = set()
    for c in srcpath.src_files():
        live = progress.strip_dead_code(c.read_text(errors="replace"))
        referenced.update(re.findall(r"\b([A-Za-z_]\w*)\s*\(", live))
    named, unnamed = [], []
    for name in sorted(referenced):
        a = info.get(name, (None, 0))[0]
        if a is None or not progress.is_library(a):
            continue
        (unnamed if FUNC_PH.match(name) else named).append(name)
    return named, unnamed


def report_state(func):
    """('fresh'|'stalled'|'none', has_preserved_body)"""
    p = REPORTS / f"{func}.md"
    if not p.exists():
        return "fresh", False
    t = p.read_text(errors="replace")
    body = "#if 0" in t
    if progress.REOPENED_RE.search(t):
        return "fresh", body
    return "stalled", body


def git_date(path):
    out = subprocess.run(["git", "log", "-1", "--format=%cs", "--", str(path)],
                         capture_output=True, text=True, cwd=ROOT).stdout.strip()
    return out or None


MIN_CALIBRATION_ATTEMPTS = 4


def calibration_rounds(st):
    """The rounds that COUNT toward calibration and the stop rule: recorded
    without --not-calibration, on the ranked stall band, with at least
    MIN_CALIBRATION_ATTEMPTS assignments. Round 50 measured two runners on a
    324w and a 954w FRESH body and was recorded as calibration round A; the
    stop rule would then have parked the track on evidence that never touched
    the small stalls (FINISHING-PLAN track 1, revision 2)."""
    return [r for r in st["tracks"]["1"]["rounds"]
            if r.get("calibration", True) and r.get("attempts", 0) >= MIN_CALIBRATION_ATTEMPTS]


def match_model(st):
    """Which model matching runners use next: alternate through calibration,
    then whichever won on matches per runner-session."""
    if st["models"].get("match_runner"):
        return st["models"]["match_runner"], "set by head"
    rounds = calibration_rounds(st)
    k = st["stop_rule"]["calibration_rounds"]
    if len(rounds) < k:
        order = ["sonnet", "opus"]
        return order[len(rounds) % 2], f"calibration round {len(rounds) + 1} of {k}"
    per = {}
    for r in rounds:
        m = r.get("model", "?")
        per.setdefault(m, [0, 0])
        per[m][0] += r.get("matches", 0)
        per[m][1] += max(1, r.get("runners", 1))
    best = max(per.items(), key=lambda kv: kv[1][0] / kv[1][1])
    return best[0], "won calibration (matches per runner-session)"


def track1_status(st, fresh, stalled):
    t = st["tracks"]["1"]
    if t["status"] in ("parked", "done"):
        return t["status"], t.get("reason", "")
    if fresh == 0 and stalled == 0:
        return "done", "queue empty"
    rounds = calibration_rounds(st)
    k = st["stop_rule"]["calibration_rounds"]
    need = st["stop_rule"]["min_matches"]
    if len(rounds) >= k:
        recent = rounds[-k:]
        got = sum(r.get("matches", 0) for r in recent)
        if got < need:
            return "parked", (f"stop rule: last {k} rounds produced {got} match(es), "
                              f"fewer than {need}. Revisit only per FINISHING-PLAN track 1 revisit rule.")
    return "open", ""


def collect(st):
    pj = run_json([sys.executable, "tools/progress.py", "--json"])
    info = progress.text_symbols()
    units = unit_metrics(info)
    named_sdk, unnamed_sdk = sdk_surface(info, units)
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
    todo3 = sorted((u for u in units if u not in done),
                   key=lambda u: (-units[u]["centrality"], -units[u]["func_named"]))
    pct3 = len(done) / max(1, len(units))

    # track 1 revisit: stalls in units whose naming pass finished AFTER the
    # stall's report was last touched
    revisit = []
    for words, unit, func, title in stall_rows:
        if unit in done:
            rd = git_date(REPORTS / f"{func}.md")
            if rd and rd < done[unit]:
                revisit.append((words, unit, func, title))

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
    else:
        t1b_status = t1b["status"]

    docs = {}
    for rel, budget in DOC_BUDGETS.items():
        p = ROOT / rel
        n = sum(1 for _ in p.open(errors="replace")) if p.exists() else None
        docs[rel] = {"lines": n, "budget": budget,
                     "over": n is not None and n > budget}

    return {
        "progress": {k: pj[k] for k in ("matched", "queued", "stalled", "fresh", "uncarved", "game", "library")},
        "elf_present": bool(info),
        "tracks": {
            "1": {"status": t1_status, "reason": t1_reason, "fresh": len(fresh_funcs),
                  "stalled": len(stall_rows), "rounds": st["tracks"]["1"]["rounds"],
                  "next_match_model": mm, "why": mm_why, "revisit": len(revisit)},
            "1b": {"status": t1b_status, "promotable": len(promotable),
                   "nm_bodies": sum(u["nm_bodies"] for u in units.values())},
            "2": {"status": st["tracks"]["2"]["status"] if unnamed_sdk else "done",
                  "named": len(named_sdk), "unnamed": len(unnamed_sdk), "unnamed_list": unnamed_sdk},
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
        "_todo3": todo3, "_revisit": revisit,
    }


_nm_cache = {}


def nm_defined(unit):
    if unit not in _nm_cache:
        p = srcpath.unit_src(unit)
        _nm_cache[unit] = nm_bodies(p.read_text(errors="replace")) if p else set()
    return _nm_cache[unit]


def jobs(d, n):
    """Ready jobs across open tracks, highest value first, with a model each."""
    out = []
    t = d["tracks"]
    per = max(3, n // 3)   # no single track may crowd the others out of the list
    for words, unit, func, title in sorted(d["_fresh"]):
        out.append(("1", f"match {func} ({unit}, {words}w, fresh)", t["1"]["next_match_model"]))
    for unit in d["_todo3"][:per]:
        u = d["units"][unit]
        out.append(("3", f"naming pass on {unit} (centrality {u['centrality']}, "
                         f"{u['func_named']}/{u['defs']} defs unnamed, {u['unk_refs']} unk, "
                         f"{u['slot_refs']} slot calls, {u['d_refs']} D_)", t["3"]["naming_model"]))
    if t["2"]["unnamed"]:
        if not (ROOT / "tools/sdkname.py").exists():
            out.append(("2", "HEAD: build tools/sdkname.py (fingerprint unnamed SDK functions against every "
                             "object on every disc; self-check on ten placed functions) before staffing track 2",
                        MODELS["head_when_new_procedure"]))
        else:
            out.append(("2", f"identify and name {t['2']['unnamed']} SDK functions game code calls "
                             f"(one runner, batch; list: plan.py --json .tracks.2.unnamed_list)",
                        MODELS["mechanical_runner"]))
    if t["1"]["status"] == "open":
        for words, unit, func, title in d["_stalls"][:per]:
            out.append(("1", f"stall attempt {func} ({unit}, {words}w) :: {title[:60]}",
                        t["1"]["next_match_model"]))
    for words, unit, func, title in d["_revisit"][:per]:
        out.append(("1", f"REVISIT stall {func} ({unit}, {words}w) after its unit's naming pass",
                    t["1"]["next_match_model"]))
    if t["1b"]["status"] == "open":
        for words, unit, func in sorted(d["_promotable"])[:per]:
            out.append(("1b", f"promote {func} ({unit}, {words}w) preserved body to #ifdef NON_MATCHING",
                        MODELS["mechanical_runner"]))
    if t["4"]["status"] == "open":
        out.append(("4", f"unify {t['4']['local_struct_views']} unit-local struct views into shared headers, one class at a time", "opus"))
    if t["5"]["status"] == "open":
        for k, done in t["5"]["checklist"].items():
            if not done:
                out.append(("5", f"{k}: {TRACK5_ITEMS[k]}", "opus"))
    return out[:n]


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
    ncal = len(calibration_rounds(st))
    print(f"  1      {t1['status']:<10} stall matching: {t1['fresh']} fresh, {t1['stalled']} stalled; "
          f"{len(t1['rounds'])} round(s) recorded, {ncal} count for calibration; "
          f"next runner model {t1['next_match_model']} ({t1['why']})")
    if t1["reason"]:
        print(f"                    {t1['reason']}")
    if t1["revisit"]:
        print(f"                    {t1['revisit']} stall(s) eligible for a post-naming REVISIT")
    print(f"  1b     {t['1b']['status']:<10} NON_MATCHING bodies: {t['1b']['nm_bodies']} in src, "
          f"{t['1b']['promotable']} stall(s) with a preserved body and none yet")
    print(f"  2      {t['2']['status']:<10} SDK call surface: {t['2']['named']} named, {t['2']['unnamed']} still func_")
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
            print(f"    {k}: {v['lines']} lines, budget {v['budget']}")
        print()
    print(f"  READY JOBS (top {n}; head fills at most 5 runner slots from the top, one unit per runner):")
    for track, desc, model in jobs(d, n):
        print(f"    [{track:<2}] {model:<7} {desc}")
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
    r.add_argument("--model", required=True, choices=["sonnet", "opus", "fable"])
    r.add_argument("--runners", type=int, required=True)
    r.add_argument("--attempts", type=int, required=True)
    r.add_argument("--matches", type=int, required=True)
    r.add_argument("--note", default="")
    r.add_argument("--not-calibration", action="store_true",
                   help="this round did not take >=4 assignments from the ranked stall band "
                        "(e.g. it worked fresh giants); it is recorded but does not count "
                        "toward calibration or the stop rule")
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
        if not a.not_calibration and a.attempts < MIN_CALIBRATION_ATTEMPTS:
            print(f"NOTE: {a.attempts} attempt(s) is below {MIN_CALIBRATION_ATTEMPTS}; this round "
                  f"is recorded but does not count toward calibration or the stop rule.")
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
