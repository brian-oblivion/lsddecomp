# DreamSys__SelectCallback98 — MATCHED

> Renamed from `func_800596E8` on 2026-09-22 (tools/rename.py). Address 0x800596e8.

Round 2026-08-30, runner ALPHA, unit `DreamSys`. 54/54 words, full match.

## Source

```c
extern void InitSoundCueSet(s32 arg0, void *arg1, s32 arg2, DreamSys *arg3, void *arg4);

void DreamSys__SelectCallback98(DreamSys *this, s32 arg1)
{
	struct vtable_DreamSys *vt = this->vt;

	if (this->unk_0x9C == 2)
		vt->func_8005A134(this, 0);
	this->unk_0x9C = arg1;
	switch (arg1) {
	case 0:
		this->callback_0x98 = NULL;
		break;
	case 1:
		this->callback_0x98 = (void (*)(DreamSys *))vt->func_80059A58;
		break;
	case 2:
		this->callback_0x98 = vt->func_8005A0B0;
		this->unk_0xC4 = 1;
		this->unk_0xC8 = 1;
		InitSoundCueSet(this->unk_0x58, this->unk_0xCC, 1, this, this->vt->func_8005A1F4);
		break;
	}
}
```

`InitSoundCueSet` is still `INCLUDE_ASM` in the uncarved `code_179d8`; the local
`extern` follows CLAUDE.md's "calling into a function still INCLUDE_ASM in
another unit is fine" precedent. `this->vt->func_8005A1F4` (not the hoisted
`vt` local) is used deliberately for the last argument — see residue 3.

## Derivation

Same shape as `DreamSys__SelectCallback80`, matched earlier this round: hoist `this->vt`
into a local (retail loads `$s2 = this->vt` unconditionally at entry and
reuses it), and write the `arg1` dispatch as a `switch` with cases in
ascending textual order (`0, 1, 2`) — GCC 2.6.3 lays out each case's body in
that order regardless of the comparison tree it generates (here, a
binary-search pivot on 1 again, then `<2` vs `>=2`). `s3 = 2` is loaded once,
unconditionally, and reused for both the entry guard (`unk_0x9C == 2`) and
the case-2 dispatch (`arg1 == 2`) — the same constant appearing twice in one
function, genuinely shared, unlike the residue below.

`func_8005A134`, `func_80059A58` (cast — it actually returns `s32`, see
CLAUDE.md's "one-line wrapper" note; `callback_0x98`'s declared type is
`void(*)(DreamSys*)` so the assignment needs an explicit cast to silence the
warning without touching codegen), `func_8005A0B0`, and `func_8005A1F4` all
resolved via `tools/classtable.py DREAMSYS_METHODS` against the vtable slots
read in the disassembly (`0x17C`, `0x154`, `0x178`, `0x194`).

## Residues fixed, in order

1. First attempt (48/54): case 2's body written in the "obvious" order
   (`unk_0xC4 = 1; unk_0xC8 = 1; callback_0x98 = vt->func_8005A0B0;` — literal
   field-address order, ascending). Two symptoms: the switch's own
   dispatch constant (comparing `arg1` to 1) landed in `$v1` where retail
   has `$a0`, and the three stores in case 2's body came out in a different
   *relative* order than retail (retail's `sw v0,0x98` — the callback store —
   is scheduled AFTER the two literal-1 stores even though the LOAD feeding it
   is scheduled well before them).
2. Fix: reorder case 2's body so `this->callback_0x98 = vt->func_8005A0B0;`
   is the FIRST statement, followed by the two `unk_0xC4`/`unk_0xC8` literal
   stores — full match. Both symptoms (the stray register AND the store
   order) cleared from this single reorder; they were the same underlying
   cause (wrong source statement order), not two separate problems needing
   separate fixes.
   In retail's disassembly, the LOAD feeding `callback_0x98` (`lw
   v0,0x178(s2)`) appears textually before the two literal-1 stores, which
   made "assignment order = order the reads appear in the disassembly" look
   right at first glance for the ORIGINAL (wrong) ordering too — but the two
   independent stores that follow it (`sw a0,0xc4` / `sw a0,0xc8`) come
   before the callback's own store (`sw v0,0x98`). What settled it was
   noticing the STORE positions, not the load: `unk_0xC4`/`unk_0xC8` are
   written before `callback_0x98`'s value is written, even though its
   read happens first.

## Proposed learning

When a residue involves a shared/cached pointer (`this->vt`) feeding one
store among several independent ones, and the load/store instructions don't
obviously line up with a "top-to-bottom, statement-by-statement" reading of
the disassembly, check the STORE positions specifically to recover the true
C statement order — a read that happens to be schedulable early can appear
before unrelated stores in the disassembly regardless of source order, but
the stores themselves preserve it. Getting the order right fixed a stray
register too (`$v1` vs `$a0` for the switch's own dispatch constant); no
`__asm__("")` barrier or register trick was needed or attempted.

## Naming

- **Tier B.** Symmetric setter for callback_0x98's own small menu; additionally, when the mode was already 2 on entry, calls func_8005A134(this, 0) first, and its own mode-2 case wires up an InitSoundCueSet call.
