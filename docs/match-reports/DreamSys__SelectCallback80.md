# DreamSys__SelectCallback80 — MATCHED

> Renamed from `func_8005966C` on 2026-09-22 (tools/rename.py). Address 0x8005966c.

Round 2026-08-30, runner ALPHA, unit `DreamSys`. 31/31 words, full match.

## Source

```c
void DreamSys__SelectCallback80(DreamSys *this, s32 arg1)
{
	struct vtable_DreamSys *vt = this->vt;

	this->unk_0x84 = arg1;
	switch (arg1) {
	case 0:
		this->callback_0x80 = NULL;
		break;
	case 1:
		this->callback_0x80 = vt->func_800597C0;
		break;
	case 2:
		this->callback_0x80 = vt->DreamSys__NoOpSlot14C;
		break;
	case 3:
		this->callback_0x80 = vt->DreamSys__NoOpSlot150;
		break;
	}
}
```

## Derivation

`this->unk_0x84 = arg1` is unconditional — retail folds it into the delay slot
of the very first comparison. The vtable slots (`0x140`/`0x14C`/`0x150`)
resolved with `tools/classtable.py` to `func_800597C0` / `DreamSys__NoOpSlot14C` /
`DreamSys__NoOpSlot150`.

## Residues fixed, in order

1. **First attempt: `switch` with cases written `case 1: case 0: case 2:
   case 3:`** (matching the *comparison* order I read off the disassembly —
   retail checks `arg1==1` first, then `arg1<2`, then `==2`, `==3`). This
   miscompiled badly (2/31, huge outside-range drift): GCC placed the case-1
   body FIRST in the switch's linear layout, matching my source's textual
   case order — not retail's. **The lesson: for a `switch`, GCC 2.6.3 lays
   out each case's body in *textual* order, but is free to choose whatever
   *comparison* structure it likes (here, a binary-search-style pivot on
   `1`) independent of that layout.** Do not read comparison order off the
   disassembly and transcribe it as `case` order — they are unrelated axes.
2. **Second attempt: nested `if`/`else if` mirroring the exact comparison
   tree** (`if (arg1==1) ... else if (arg1<2) { if (arg1==0) ... } else if
   (arg1==2) ... else if (arg1==3) ...`) with `vt` still read inline via
   `this->vt` at each use. Fixed the comparison-order problem but still
   wrong (2/31): retail loads `$v1 = this->vt` exactly ONCE, unconditionally,
   as the function's very first instruction, then reuses it in all three
   branches that read it. My inline `this->vt` per-branch caused a reload at
   each site instead.
3. **Fix: hoist `struct vtable_DreamSys *vt = this->vt;` as a local declared
   at the top of the function** (matches CLAUDE.md's "real struct fields,
   declarations at block top" C89 guidance anyway) — this reproduced the
   unconditional load, but the case-1-body-placed-first problem from
   attempt 1 was still present since I'd kept the `if`-chain body order from
   attempt 2's transcription. Went back to `switch` with cases written in
   ascending source order (`0,1,2,3`), keeping the hoisted `vt` — full match.

## Proposed learning

**For a `switch(arg)` over a small dense range, GCC 2.6.3 -O2 may lower the
dispatch as a binary-search comparison tree (pivoting on a middle case value,
not necessarily case 0) while still laying out each case's generated code in
the SAME order the cases appear in the source.** Two independent axes:
comparison order (compiler's choice, e.g. "check 1 first") and body layout
order (source's case order). Reading a disassembly's comparison order and
transcribing it 1:1 as `if`/`else if` order, or as `switch` case-list order,
conflates the two and produces a body-order mismatch that reads as "mostly
right, huge diff" rather than an obviously-wrong shape. When a residue looks
like "the right blocks, wrong place," try writing the cases/branches in plain
ascending value order first, independent of the order you observed the
comparisons happen in.

Also: **cache a repeatedly-used `this->member` pointer into a local at the
top of the function when the disassembly shows it loaded unconditionally
before any branch**, even if the source only *uses* it inside conditional
paths — this is a real, recurring shape (`struct vtable_DreamSys *vt =
this->vt;`), not over-fitting.
