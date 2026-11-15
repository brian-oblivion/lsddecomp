#!/usr/bin/env python3
"""Write map.html, a treemap of every function in the exe. — a function treemap of the whole exe.

Boxes are sized by bytes and colored by status:
  green  = decompiled to C (in the matching build)
  gray   = PsyQ library code (stays asm)
  dark red = game code still in asm (the to-do pile)

Data comes from build/lsdde.elf (function symbols) plus config/splat.yaml
(c subsegments = matched ranges, psyq files = lib). Every box links to its
source file on GitHub and shows name/bytes/status on hover.

Run after scripts/build.sh. Output: map.html (open in a browser, or serve
via the gh-pages branch).
"""
import json
import re
import subprocess
from pathlib import Path

import yaml

ROOT = Path(__file__).resolve().parent.parent
ELF = ROOT / "build" / "lsdde.elf"
YAML = ROOT / "config" / "splat.slps01556.lsdde.yaml"
OUT = ROOT / "map.html"
REPO_URL = "https://github.com/brian-oblivion/lsddecomp/blob/main"

TEXT_START, TEXT_END = 0x800118DC, 0x80066828  # vram bounds of .text
LIB_FILES = {"psyq_2258", "psyq_rcpolyf3", "psyq_rcpolyf4", "psyq_rcpolyg3",
             "psyq_rcpolyft3", "psyq_rcpolygt3", "psyq_GsLinkObject4",
             "psyq_15d04", "psyq_PadInit", "psyq_SpuSetMute", "psyq_memset",
             "psyq_rand"}


def main():
    nm = subprocess.run(
        ["mipsel-none-elf-nm", "-n", "--defined-only", str(ELF)],
        capture_output=True, text=True, check=True).stdout
    funcs = []
    for line in nm.splitlines():
        m = re.match(r"([0-9a-f]{8,16}) \w ([^\s]+)", line)
        if m and not m.group(2).endswith(".NON_MATCHING"):
            addr = int(m.group(1), 16) & 0xFFFFFFFF
            if TEXT_START <= addr < TEXT_END and not m.group(2).startswith(("$", ".")):
                funcs.append((addr, m.group(2)))
    funcs.sort()
    # size = delta to next function (last one capped)
    sizes = {}
    for i, (addr, name) in enumerate(funcs):
        nxt = funcs[i + 1][0] if i + 1 < len(funcs) else addr + 16
        sizes[addr] = max(4, nxt - addr)

    cfg = yaml.safe_load(YAML.read_text())
    subs = next(s for s in cfg["segments"]
                if isinstance(s, dict) and s.get("name") == "main")["subsegments"]

    def to_vram(off):
        return 0x80010000 + off - 0x800

    c_ranges, groups = [], {}
    for i, s in enumerate(subs):
        if not isinstance(s, list) or len(s) < 2:
            continue
        start = s[0]
        end = subs[i + 1][0] if i + 1 < len(subs) and isinstance(subs[i + 1], list) else 0x7B800
        if start < 0x20DC or start >= 0x57028:
            continue
        kind = s[1] if isinstance(s[1], str) else "asm"
        name = str(s[2]) if len(s) > 2 else ("unk_%X" % start)
        if kind == "c":
            c_ranges.append((to_vram(start), to_vram(end)))
            groups[name] = (to_vram(start), to_vram(end), name)
        elif kind in ("o", "asm", "rodata", ".rodata", "hasm") or kind.endswith("rodata"):
            if TEXT_START <= to_vram(start) < TEXT_END:
                groups["sdk:" + name] = (to_vram(start), to_vram(end), name)

    entries = []
    sdk_groups = {k for k in groups if k.startswith("sdk:")}
    for addr, name in funcs:
        st = "done" if any(a <= addr < b for a, b in c_ranges) else None
        if st is None:
            gname = str(next((g for g, (a, b, _) in groups.items() if a <= addr < b), "other"))
            st = "lib" if gname in sdk_groups else "todo"
        gname = str(next((g for g, (a, b, _) in groups.items() if a <= addr < b), "other"))
        url = f"{REPO_URL}/src/{gname}.c" if st == "done" else f"{REPO_URL}/asm/{gname}.s"
        entries.append({"n": name, "a": addr, "b": sizes[addr], "s": st,
                        "g": gname, "u": url})

    groups_json = {}
    for e in entries:
        groups_json.setdefault(e["g"], []).append(e)

    html = """<!DOCTYPE html><html><head><meta charset="utf-8">
<title>lsddecomp function map</title>
<style>
body{margin:0;background:#111;color:#ddd;font:12px/1.4 system-ui,sans-serif}
header{padding:10px 16px;background:#000}
h1{margin:0;font-size:16px}
.legend{font-size:11px;color:#999}
.legend i{display:inline-block;width:10px;height:10px;margin:0 4px 0 12px;border-radius:2px}
.done{background:#2ea043}.todo{background:#8a2f2f}.lib{background:#3a3f4a}
#map{position:relative;height:calc(100vh - 64px);margin:8px}
#map div{position:absolute;overflow:hidden;border:1px solid #111;color:#ccc;
text-indent:2px;cursor:pointer}
#map div:hover{outline:2px solid #fff;z-index:2}
.done{background:#2ea043}.todo{background:#8a2f2f}.lib{background:#3a3f4a}
.grouplabel{position:absolute;background:#000;color:#8b949e;font-size:11px;
padding:1px 6px;border-radius:2px;z-index:3;pointer-events:none}
</style></head><body>
<header><h1>lsddecomp function map</h1>
<div class="legend">box area = bytes
<i class="done"></i>decompiled (C, matching)
<i class="todo"></i>game asm remaining
<i class="lib"></i>PsyQ lib (stays asm)</div></header>
<div id="map"></div>
<script>
const groups = __GROUPS__;
const H = 40;  // header space per group
const mapEl = document.getElementById('map');
const W = mapEl.clientWidth, HH = mapEl.clientHeight;
// group areas proportional to total bytes
const totals = Object.entries(groups).map(([g, f]) =>
    ({g, b: f.reduce((s, x) => s + x.b, 0)}));
const grand = totals.reduce((s, t) => s + t.b, 0);
function squarify(sizes, x, y, w, h, out) {
    function worst(row, thick) {
        return Math.max(...row.map(r => Math.max(thick*thick/r, r/(thick*thick))));
    }
    function layoutRow(row, x, y, w, h) {
        if (w >= h) { let cy = y; for (const r of row) { const hh = r / w; out.push([x, cy, w, hh]); cy += hh; } }
        else { let cx = x; for (const r of row) { const ww = r / h; out.push([cx, y, ww, h]); cx += ww; } }
    }
    function rec(sz, x, y, w, h) {
        if (!sz.length || w < 1 || h < 1) return;
        const vertical = w >= h;
        const len = vertical ? h : w;
        let row = [], thick = 0;
        while (sz.length) {
            const cand = row.concat(sz[0]);
            const t = cand.reduce((a, b) => a + b, 0) / len;
            if (row.length === 0 || worst(cand, t) <= worst(row, thick)) { row = cand; thick = t; sz.shift(); }
            else break;
        }
        if (vertical) { layoutRow(row, x, y, thick, h); rec(sz, x + thick, y, w - thick, h); }
        else { layoutRow(row, x, y, w, thick); rec(sz, x, y + thick, w, h - thick); }
    }
    rec(sizes.slice(), x, y, w, h);
    return out;
}
const gRects = squarify(totals.map(t => t.b / grand * W * HH), 0, 0, W, HH, []);
totals.forEach((t, i) => {
    const [gx, gy, gw, gh] = gRects[i];
    const lab = document.createElement('div');
    lab.className = 'grouplabel';
    lab.textContent = t.g + ' (' + t.b + ' B)';
    lab.style.cssText = `left:${gx}px;top:${gy}px`;
    mapEl.appendChild(lab);
    const innerH = Math.max(gh - 12, 4), innerY = gy + 12;
    const fns = groups[t.g];
    if (gw < 8 || gh < 16) {   // too small to split: one merged block
        const d0 = document.createElement('div');
        d0.className = fns[0].s;
        d0.style.cssText = `left:${gx}px;top:${gy}px;width:${Math.max(gw,1)}px;height:${Math.max(gh,1)}px`;
        d0.title = t.g + ' (' + t.b + ' B)';
        mapEl.appendChild(d0);
        return;
    }
    const fRects = squarify(fns.map(f => f.b / t.b * gw * innerH), gx, innerY, gw, innerH, []);
    fns.forEach((f, j) => {
        const [x, y, w, h] = fRects[j] || [gx, innerY, 0, 0];
        const d = document.createElement('div');
        d.className = f.s;
        d.style.cssText = `left:${x}px;top:${y}px;width:${w}px;height:${h}px`;
        d.title = `${f.n}\\n${f.b} bytes @ 0x${f.a.toString(16)}\\n${f.s}`;
        if (w > 44 && h > 13) d.textContent = f.n.replace(/^func_/, 'f_');
        d.onclick = () => window.open(f.u);
        mapEl.appendChild(d);
    });
});
</script></body></html>"""
    html = html.replace("__GROUPS__", json.dumps(groups_json))
    OUT.write_text(html)
    n_done = sum(1 for e in entries if e["s"] == "done")
    n_lib = sum(1 for e in entries if e["s"] == "lib")
    n_todo = sum(1 for e in entries if e["s"] == "todo")
    print(f"wrote {OUT}: {len(entries)} functions "
          f"({n_done} done, {n_todo} game-asm todo, {n_lib} lib)")


if __name__ == "__main__":
    main()
