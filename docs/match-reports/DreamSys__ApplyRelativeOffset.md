# DreamSys__ApplyRelativeOffset

> Renamed from `func_8005AF64` on 2026-09-22 (tools/rename.py). Address 0x8005af64.

**Unit:** DreamSys · **Size:** 27 instructions · **Status:** MATCHED (27/27 words)

## What it does

Builds a full-word `(x,y,z)` difference vector from two `RelativePos`
(s16-triplet) points, forces the y component to 0, and calls
`this->vt->BaseObjO__AddVec14(this, &diff)`.

## The C

```c
void DreamSys__ApplyRelativeOffset(DreamSys *this, struct RelativePos *a, struct RelativePos *b)
{
	DreamSysVec3 diff;

	diff.x = a->x - b->x;
	diff.y = a->y - b->y;
	diff.z = a->z - b->z;
	diff.y = 0;
	this->vt->BaseObjO__AddVec14(this, &diff);
}
```

## Residue: a real, KEPT dead store, and it matters where in source order it sits

`diff.y` is computed from `a->y - b->y` and then immediately overwritten
with `0` -- a textbook dead store. Retail's compiled output keeps BOTH the
computation and the overwrite (doesn't eliminate the dead one), which only
reproduces when the `diff.y = 0;` statement is written LAST, after `diff.z`
-- not immediately after computing `diff.y` (which let GCC 2.6.3 eliminate
it as adjacent-assignment dead code, one instruction short). This reads as
intentional/harmless leftover code in the original source, not a
transcription error on this project's part: retail truly executes the
subtraction and then discards it.

### Proposed learning

A locally-declared struct whose address escapes to a call does not
protect ALL its field writes from dead-store elimination in GCC 2.6.3 --
only adjacency does. `x = compute(); x = literal;` written back-to-back
gets the first store eliminated; separating them with an unrelated
statement in between (even one touching a different field of the same
struct) keeps both. Useful wherever a residue looks like "my C is one
instruction short and the missing one is a value that gets immediately
overwritten anyway."

## Provenance

round 2026-08-30-d, runner ALPHA, unit DreamSys (whole-unit, third pass).
