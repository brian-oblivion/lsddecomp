# DreamSys__GetSaveBlock

> Renamed from `func_8005A350` on 2026-09-22 (tools/rename.py). Address 0x8005a350.

**Unit:** DreamSys · **Size:** 6 instructions · **Status:** MATCHED (6/6 words)

## What it does

Vtable `+0x1B0`. If `arg1` is non-NULL, writes the literal `0x700` through
it. Always returns `&this->unknown_sdata_0x178` (the address computation is
in the `jr` delay slot, so it executes unconditionally on both paths).

## The C

```c
s32 *DreamSys__GetSaveBlock(DreamSys *this, s32 *arg1)
{
	if (arg1 != NULL)
		*arg1 = 0x700;
	return &this->unknown_sdata_0x178;
}
```

Retyped the vtable field from `void *DreamSys__GetSaveBlock;` to
`s32 *(*DreamSys__GetSaveBlock)(DreamSys *this, s32 *arg1);`.

## Provenance

round 2026-08-30-c, runner ALPHA, unit DreamSys (whole-unit second pass).

## Naming

`DreamSys__GetSaveBlock` -- tier B (round 66, runner alpha, FINISHING-PLAN track 3).

Renamed from `func_8005A350`.

Writes 0x700 through `arg1` when non-NULL and returns
`&this->saveMagic` -- a (pointer, length) accessor for one region of the object.
Three independent facts say which region:
  1. 0x178 + 0x700 == 0x878, and the fields in [0x178, 0x878) are exactly the
     playthrough state: year, day, the three unlock scores, `moodPreviousDays[365]`,
     the stored flashbacks, the nav-challenge array, the dynamic-link penalty and
     the screen-shake flag.
  2. `DreamSys__InitNewGame` initializes precisely that span -- it starts by writing
     offset 0x178 and ends with `memset(&this->unknown_values_0x684, 0, 0x1F4)`,
     and 0x684 + 0x1F4 == 0x878.
  3. The first word of the span is `SAVE_MAGIC`, whose bytes are 4A 30 31 00 --
     "J01".
Tier B rather than A: the function has no carved caller, so nothing observed
actually persists what it hands out.
