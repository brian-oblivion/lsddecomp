# DreamSys__func_5ba20

> Renamed from `func_8005BA20` on 2026-09-22 (tools/rename.py). Address 0x8005ba20.

**Unit:** DreamSys · **Size:** 8 instructions · **Status:** MATCHED (8/8 words)

## What it does

Vtable `+0x228` -- the field long named `func_228` in the header (kept, see
below). Bounds-checked setter on `unk_0x924`, returning the OLD value:
`if (value >= 0) { old = field; field = value; } else { old = field; }
return old;`.

## The C

```c
s32 DreamSys__func_5ba20(DreamSys *this, s32 value)
{
	s32 old;

	if (value >= 0) {
		old = this->unk_0x924;
		this->unk_0x924 = value;
	} else {
		old = this->unk_0x924;
	}
	return old;
}
```

## Residue: retail reloads `unk_0x924` independently in EACH branch, not once before the `if`

First attempt (`old = this->unk_0x924; if (value >= 0) this->unk_0x924 =
value; return old;` -- the get/set/return-old shape that matched
`DreamSys__GetSetMoveMode` perfectly in the first pass) compiled to only 7 words, one
short of retail's 8: GCC hoisted the single load before the branch and
shared it across both paths, which retail's own compile evidently didn't
do. Splitting the load into both branches (shown above) matches retail's
redundant-load shape exactly and closed it on the second try. Contrast with
`DreamSys__GetSetMoveMode`, which genuinely DOES want the hoisted single load -- same
surface shape, different actual source, distinguishable only by counting
retail's own instruction words (7 vs 8) before choosing which form to
write.

## Field name kept as `func_228`, not renamed to `DreamSys__func_5ba20`

This slot was previously documented as a standalone tail field named
`func_228` (with an incorrect note that it "lives past the previously-
documented end of this struct at 0x21c"). `tools/classtable.py
gDreamSysMethods` resolves it this round to be DreamSys__func_5ba20's own slot,
directly following `ResetFlashbackList`/`DreamSys__SaveLinkSnapshot`/`DreamSys__RestoreLinkSnapshot` --
no gap, and no "past the end" special case. The field is NOT renamed,
though: `src/app/GameApplicationFileResource.c` (a different unit, out of this runner's scope)
already calls it as `self->dreamSys->vt->func_228(self->dreamSys,
arg->unk14)`, and renaming the C field would require an out-of-scope edit
there. Retyped from `void (*)(DreamSys*, s32)` to `s32 (*)(DreamSys*, s32)`
instead (a discarded non-void return in a bare statement is legal C either
way, so this doesn't need `GameApplicationFileResource.c` touched).

### Proposed learning

Two structurally-identical-looking "get old value, conditionally set new
value" residues can want DIFFERENT C shapes -- one wants the load hoisted
before the branch (shared across both paths, 7 words), the other wants it
duplicated inside each branch (8 words). The tell is retail's own
instruction count for the function, not the shape of the C you'd naturally
reach for first. Count words before picking a form.

## Provenance

round 2026-08-30-c, runner ALPHA, unit DreamSys (whole-unit second pass).

## Naming

`DreamSys__func_5ba20` -- tier C (round 66, runner alpha, FINISHING-PLAN track 3).

Renamed from `func_8005BA20`.

Tier-C placeholder. Known: the unit's get/set shape over
`unk_0x924` (negative argument means query only), vtable slot +0x228, which
src/app/GameApplicationFileResource.c reaches by the field name `func_228` -- GameApplication's constructor
calls it once with `arg->unk14` and discards the result. `unk_0x924` is cleared by
`DreamSys__ResetSessionState` and read by nothing in carved code, and the one caller
names its argument no better, so there is nothing to name it after.

Re-checked round 92 (runner delta, track 7): still no reader of `unk_0x924`
anywhere in src/, and the slot has no caller that names its argument, so the
tier-C placeholder stays.
