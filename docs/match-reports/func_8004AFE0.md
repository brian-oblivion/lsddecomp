# func_8004AFE0

**Unit:** class_3ac78 · **Size:** 20 instructions · **Status:** MATCHED (20/20 words)

## What it does

Reads two signed bytes out of a byte buffer (`arg1[2]`, `arg1[3]`), each
minus one, stored as two `s16` fields; sets two other fields to the same
`s32` value (`arg2`); then calls a helper with the object, discarding its
result.

## The C

```c
void func_8004AFE0(Class866E8 *self, u8 *arg1, s32 arg2)
{
	u8 b3;

	self->unk7C = (s8)arg1[2] - 1;
	b3 = arg1[3];
	self->unk80 = arg2;
	self->unk84 = arg2;
	self->unk7E = (s8)b3 - 1;
	func_8004C93C(self);
}
```

```c
extern void func_8004C93C(Class866E8 *self);
```

## The residue this took several tries to close, and how it closed

The obvious first draft --

```c
self->unk7C = (s8)arg1[2] - 1;
self->unk80 = arg2;
self->unk84 = arg2;
self->unk7E = (s8)arg1[3] - 1;
func_8004C93C(self);
```

-- matched 12/20 words at the right total SIZE (0x50, same as retail) but
in the WRONG ORDER: GCC 2.6.3's scheduler filled the first `lbu`'s load-delay
slot with `sw a2, 0x80(a0)` (since it's independent and available), leaving
a genuine `nop` later where retail instead has one right after the FIRST
`lbu` and fills the SECOND `lbu`'s delay slot with that same store. Same
instructions, same registers, purely a different delay-slot-filling choice
by the scheduler -- squarely the "instruction order only" residue class in
`docs/MATCHING-GUIDE.md`.

Two things that did NOT work, tried before the fix (both instruction-count
regressions, discarded immediately as size changes are a hard signal to
back off):

- A bare `__asm__("");` scheduling barrier right after the `unk7C` store --
  this forced an actual extra instruction into the output (21 words instead
  of 20), which is the wrong shape of "barrier changes order, not
  registers" -- here it changed the COUNT, so it was wrong to keep.
- Reordering the C statements outright (moving `unk84`/`unk80` before
  `unk7C`) -- same size, but a *different* wrong order (11/20).

What worked: pulling `arg1[3]` out into its own statement (`b3 = arg1[3];`)
positioned textually BETWEEN the `unk7C` store and the `unk80`/`unk84`
stores, with the sign-extend-and-subtract-1 done on the cached `b3` value
afterward. This didn't change what gets computed, but it gave the scheduler
a byte-sized load with no immediate consumer to place ahead of the two
independent stores, and it picked exactly retail's slot-filling choice.

## Provenance

round 2026-09-01, runner charlie, unit class_3ac78 (first pass, unit carved
this round).

### Proposed learning

When a same-size, same-register, same-branch-target residue is purely a
different delay-slot-filling CHOICE between two adjacent loads (not a
single load's slot vs. no slot), pulling the second load's value out into
its own named local, positioned between the two stores it competes with in
source order, can retarget GCC 2.6.3's scheduler onto retail's exact choice
without changing instruction count. Try this before reaching for
`__asm__("")` — the barrier in this instance forced a real extra
instruction (wrong direction entirely) where the temp-variable reshape cost
nothing.
