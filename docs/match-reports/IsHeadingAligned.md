# IsHeadingAligned

> Renamed from `func_8005BE28` on 2026-09-22 (tools/rename.py). Address 0x8005be28.

**Unit:** DreamSys · **Size:** 26 words · **Status:** MATCHED, 26/26.

## What it does

An angle-proximity test: takes a heading and an index into a 4-entry cardinal
direction table, computes the signed difference, normalizes it into
`[-180, 180)`, and returns whether it lies within ±44 of the direction table's
angle.

```c
typedef struct DirectionCheckArg {
	s8 unk0[4];
	u16 heading;
} DirectionCheckArg;

typedef struct DirectionTableEntry {
	u16 angle;
	u16 unk2;
	u16 unk4;
	u16 unk6;
	u16 unk8;
	u16 unkA;
} DirectionTableEntry;

extern DirectionTableEntry CARDINAL_ANGLES[];

s32 IsHeadingAligned(DirectionCheckArg *a0, u8 a1)
{
	s16 diff;

	diff = a0->heading - CARDINAL_ANGLES[a1].angle;
	if (diff >= 181) {
		diff -= 360;
	} else if (diff < -180) {
		diff += 360;
	}
	return (u16)(diff + 44) < 89;
}
```

`CARDINAL_ANGLES` (`asm/data/783DC.data.s:1192`) holds 4 entries at a 12-byte
(6-halfword) stride; the entries' first halfword is `0, 0x5A, 0xB4, 0x10E`
(0°, 90°, 180°, 270°) — the four cardinal directions. Only that first field is
read anywhere in this unit's current queue, so the other five halfwords per
entry are left as `unkN` padding.

## Provenance of the two local types

Both callers of this function (`DreamSys__CheckTunnelHeading`, `DreamSys__CheckStaircaseHeading`, both
themselves stalled on the `gp_rel` blocker per their own reports) pass down,
two levels removed, the `s32 local[4]` buffer that `DreamSys.c:727` fills via
`Class6B5CC__GetRotationDegrees(this, local)`. That function's own report
(`docs/match-reports/Class6B5CC__GetRotationDegrees.md`, unit `code_d294_c`) establishes it
writes a 3-entry `WholeFrac_d294 {s16 whole; s16 frac;}` table there, so byte
offset +4 of `local` is `out[1].whole` — a degrees value.

This function reads that same offset with `lhu` (unsigned), not the `lh` an
`s16 whole` field would emit — a second, disjoint reading of the same bytes.
Per the project's multiple-independent-local-views convention this is kept as
a small unit-local type (`DirectionCheckArg`) rather than folded into
`WholeFrac_d294` in the shared header; the two views simply disagree on
signedness of the same halfword; because `diff`'s magnitude here never exceeds
one full rotation either way, the unsigned load and immediate signed
truncation to `s16 diff` produce the same numeric value as a signed load
would, so nothing about the *type mismatch* was ever visible on the wire.

## Attempts

First attempt matched byte-exact; no iteration needed. The one thing worth
noting for a future reader is the two-stage `if (diff >= 181) diff -= 360;
else if (diff < -180) diff += 360;` shape — a single conditional un-wrap, not
a `while` loop, because retail's own `slti`/`beqz` pair only ever tests once
per branch and the second `j` unconditionally skips the third instruction.
Reading it as an iterative wrap-into-range loop would have been a plausible
but wrong generalization from the value's *meaning* (an angle) rather than
from the control flow actually present.

### Proposed learning

Confirms the existing "multiple independent local views" convention with a
concrete cross-unit instance: a `WholeFrac_d294.whole` (`s16`) established by
one unit's report is read as `u16` by an unrelated function in a different
unit two calls downstream, and both are correct for the byte-match goal
because the value's magnitude never triggers a difference between the two
readings. Worth a general note that "another report already typed this
offset" is not by itself grounds to reuse that type when the current
function's own load instruction disagrees (`lhu` vs `lh`) — trust the
instruction over the precedent.

## Naming

- **Tier A.** Pure leaf: normalizes a heading delta to [-180,180) and tests it against a fixed window. Mechanics are the whole story; free function shared by DreamSys__CheckTunnelHeading and DreamSys__CheckStaircaseHeading.
