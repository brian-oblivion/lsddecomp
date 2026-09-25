# TaskCore__UpdateSlotElements — MATCH (78/78 words)

> Renamed from `Obj86B60__UpdateSlotElements` on 2026-09-25 (tools/rename.py). Address 0x8003d194.

> Renamed from `func_8003D194` on 2026-09-24 (tools/rename.py). Address 0x8003d194.

**Unit:** code_2cc8c_b · round 12 straggler.

## What it does

Walks the parallel `unk54`/`unk18`/`unk24` arrays: for each slot already
"claimed" (`unk4C->unk18[i] != NULL`), just pings the element
(`slot50`); for each unclaimed slot, forwards the caller's payload
(`slot4C`) and, if the target descriptor also has an entry at
`unk4C->unk24[i]`, calls back into `self` (`slot100`) after marking that
index current.

```c
void TaskCore__UpdateSlotElements(Obj86B60 *self, void *a1)
{
    Unk64Elem **arr;
    u8 *ptr;
    s32 i;

    if (self->unk4C == NULL) {
        return;
    }
    arr = self->unk54;
    ptr = self->unk4C->unk20;
    for (i = 0; i < self->unk50; i++, arr++, ptr += 8) {
        if (self->unk4C->unk18[i] == NULL) {
            Unk64Elem *elem = *arr;

            elem->methods->slot4C(elem, a1, ptr);
            if (self->unk4C->unk24[i] != NULL) {
                self->unk58 = i;
                self->methods->slot100(self, a1, 0);
            }
        } else {
            Unk64Elem *elem = *arr;

            elem->methods->slot50(elem);
        }
    }
}
```

## New struct knowledge

- `Unk4CObj.unk20` (`+0x020`, `u8 *`) — a pointer loaded once, then walked
  forward 8 bytes per loop iteration and forwarded as `slot4C`'s 3rd
  ("buf") argument. An external 8-byte-stride record array this unit never
  reads through directly.
- New `Unk64ElemMethods` slots: `+0x04C slot4C(self, void *a1, void *buf)`
  and `+0x050 slot50(self)`.

## Getting to the match

The trickiest of the three matched functions this round, purely on
control-flow SHAPE (not field/type guessing — those were right first
try).

**Residue 1 — condition polarity decides which arm becomes the fallthrough
block.** First draft wrote `if (unk18[i] != NULL) { slot50 } else {
slot4C...}`. GCC lowers `if(cond){A}else{B}` as "test cond, branch-if-false
to B, [A], jump end, B:, end:" — so the THEN-arm becomes the physically
first (fallthrough) block. Retail's actual layout has the BIGGER arm
(`slot4C` + nested `unk24` check) as the fallthrough and the smaller
`slot50` arm as the jump target — i.e. retail's source tests the OPPOSITE
condition (`== NULL`) from the first guess. Flipping the polarity got the
branch direction and block order right.

**Residue 2 — `arr[i]`/`elem = *arr` needs to happen SEPARATELY inside
each arm, not once before the `if`.** Original code read `elem = *arr;
arr++;` once before branching. Retail re-reads `*arr` independently in
EACH arm (two separate `lw` instructions at two different addresses, both
reading the not-yet-incremented pointer) and increments `arr` exactly
once, in a tail shared by both arms. Duplicating the `Unk64Elem *elem =
*arr;` declaration into each arm (still the same source text, just
written twice) matched this.

**Residue 3 — `i++` is NOT the for-loop's automatic increment when one arm
does EXTRA work.** The `slot100`-calling sub-path does its OWN `i++`
immediately after the call, then jumps STRAIGHT to the `arr++`/`ptr+=8`
tail, skipping a second, separately-compiled `i++` that the OTHER two
paths (unk24==NULL, and the whole slot50 arm) fall through into instead.
Writing `i++` as an explicit statement inside EACH arm (rather than in the
`for(...)` header) let the compiler naturally emit this shared-tail
pattern, since the two arms' trailing `i++;` statements happened to be
identical and GCC merged their code (branch/tail merging) — this is what
finally produced the exact retail layout. The `arr++`/`ptr+=8` stayed in
the `for(...)` header throughout since ALL paths need them exactly once,
unconditionally, every iteration.

### Proposed learning

When asm-differ shows a small nearby function (or a sub-branch of one)
whose TAIL matches a DIFFERENT branch's tail byte-for-byte at a
DIFFERENT address than where your structured C put it, suspect GCC branch
tail-merging: two source-level statements that are textually identical
(e.g. `i++;` appearing at the end of two different if/else arms) get
compiled to ONE shared block that both arms jump/fall into, rather than
being duplicated per-arm. Write the repeated statement explicitly in each
arm rather than hoisting it to a common point after the `if` — the
compiler does the hoisting itself when it's actually safe to, and won't
if doing so would require skipping over an unrelated arm's code.

## Naming (round 78, naming runner echo)

Renamed `func_` -> `Obj86B60__UpdateSlotElements`. **Tier B**: Walks every slot's element (`self->slotElements[]`), forwarding a payload to unclaimed (`unk4C->unk18[i]==NULL`) slots and pinging already-claimed ones -- an update/refresh pass over all slots. What 'claimed' means in the game is not established, so the name stays at the mechanics level.

## Track 4 (2026-09-25, round 84, alpha)

Renamed from Obj86B60__UpdateSlotElements (tools/rename.py): the class prefix. Occupant of its gTaskCoreMethods slot, named for it in TASKCORE_SLOTS (`classtable.py gTaskCoreMethods`). The class (id 0x130, table gTaskCoreMethods) is unified as `TaskCore` in `include/TaskCore.h`; `self` is `TaskCore *` (it was the `Obj86B60` or `StreamTaskObj` view). Any source block above is the pre-unification spelling; the live body takes the unified types and slot names, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
