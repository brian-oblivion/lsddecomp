# Obj86B60__ReleaseTarget — MATCH (81/81 words)

> Renamed from `func_8003D050` on 2026-09-24 (tools/rename.py). Address 0x8003d050.

**Unit:** code_2cc8c_b · round 12 straggler.

## What it does

The teardown counterpart to `Obj86B60__SetTarget`: releases `self->unk4C`'s
handle (if any), dispatches through `self->unk68`, walks the parallel
arrays calling a per-element release slot (and, for entries the target
still owns, its own `slotFC`), then frees the four arrays themselves.

```c
void Obj86B60__ReleaseTarget(Obj86B60 *self)
{
    Unk64Elem **arr;
    s32 i;

    if (self->unk4C == NULL) {
        return;
    }
    if (self->unk4C->unk0 != NULL) {
        Unk74Obj *o = self->unk4C->unk4;
        o->methods->slot4(o);
    }
    self->unk68->methods->slot4(self->unk68);
    arr = self->unk54;
    for (i = 0; i < self->unk50; arr++) {
        Unk64Elem *elem;

        if (self->unk4C->unk24[i] != NULL) {
            self->unk58 = i;
            self->methods->slotFC(self);
        }
        elem = *arr;
        elem->methods->slot4(elem);
        i++;
    }
    BMemPMgrFree(self->unk64);
    BMemPMgrFree(self->unk60);
    BMemPMgrFree(self->unk5C);
    BMemPMgrFree(self->unk54);
}
```

## New struct knowledge

- Confirms `Unk4CObj.unk0`/`unk4` (see `Obj86B60__SetTarget`'s report) from the
  OTHER side: `unk0` gates a call through `unk4->methods->slot4(unk4)`.
- `self->unk68->methods->slot4(self->unk68)` — new `Unk68ObjMethods` slot
  `+0x004`, no args beyond self.
- New `Obj86B60Methods` slot `+0x0FC slotFC(self)`.

## Getting to the match

Two residues, both the same "don't cache a loop bound in a local variable"
shape already documented project-wide, but worth restating precisely
since this function hit it twice:

1. **`s32 count = self->unk50;` used as the loop bound.** Retail reloads
   `self->unk50` fresh every iteration (a single `lw`, cheap — this is
   NOT the array-index case where caching wins, see `Obj86B60__RefreshSlotView`'s
   stall report for the contrast). Removing the cached `count` and writing
   `for (i = 0; i < self->unk50; ...)` directly dropped the function from
   8 live callee-saved registers to the correct 7.
2. **`i++` vs `arr++` ORDER inside the loop.** Retail does the call, THEN
   `i++` (inside the loop body, right after the call), and only
   increments the walking pointer `arr` in the FOR-loop's own increment
   clause. My first pass had it the other way around (arr++ in the body,
   i++ as the for-clause) — same total instructions, but the delay-slot
   filler differed by which increment ended up adjacent to the branch.
   Swapping which increment lives in the `for(...)` header vs. the body
   fixed the last remaining word.

Both fixes came from asm-differ, not guesswork — in both cases the diff
showed a REGISTER-COUNT or SINGLE-WORD swap with the exact same
instruction sequence otherwise, which is the signature of "right logic,
wrong caching/ordering choice" rather than a wrong understanding of what
the function does.

## Naming (round 78, naming runner echo)

Renamed `func_` -> `Obj86B60__ReleaseTarget`. **Tier B**: The exact teardown counterpart of Obj86B60__SetTarget: releases the same handle, walks the same four arrays, frees them. Named to pair with SetTarget.
