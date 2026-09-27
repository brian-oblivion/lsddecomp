# TaskCore__FindNextFreeSlot — MATCHED (37/37)

> Renamed from `Obj86B60__FindNextFreeSlot` on 2026-09-25 (tools/rename.py). Address 0x8003d3b0.

> Renamed from `func_8003D3B0` on 2026-09-24 (tools/rename.py). Address 0x8003d3b0.

**Unit:** code_2cc8c · **Size:** 37 words · **Result:** byte-exact

## What it does

`Obj86B60Methods::slotE8` (already recorded in `code_2cc8c.h` as
`TaskCore__FindNextFreeSlot`). Finds the next free (null) slot in
`self->unk4C->unk18[]`, starting just after the current index
`self->unk58` and wrapping at the capacity `self->unk50`, stopping either
when an empty slot is found or when the search has wrapped all the way
back to the start index with none found. If `self->unk4C` is null, the
whole function is a no-op. The found (or fallback) index is reported
through `self->methods->slotF0`.

```c
void TaskCore__FindNextFreeSlot(Obj86B60 *self)
{
    s32 i;

    if (self->unk4C == NULL) {
        return;
    }
    i = self->unk58;
    i++;
    for (;;) {
        if (i >= self->unk50) {
            i = 0;
        }
        if (i == self->unk58) {
            break;
        }
        if (self->unk4C->unk18[i++] != NULL) {
            continue;
        }
        i--;
        break;
    }
    self->methods->slotF0(self, i, 1);
}
```

## Header additions

`include/code_2cc8c.h`:

- New field `unk50` on `Obj86B60` (`s32`, the wrap capacity for the
  `unk58`-indexed search), carved out of existing padding
  (`0x050`-`0x058`).
- New field `unk18` on `Unk4CObj` (`void **`, a registration-slot pointer
  table, null-checked but never dereferenced by this unit), carved out of
  existing padding (`0x013`-`0x024`). No existing field on either struct
  changed type or offset.

## Residues, and how each closed

Four attempts before the match, in order:

1. **Naive translation** (cached `start = self->unk58` local, `for(;;)`
   with `if (i>=cap) i=0; if(i==start) break; if(arr[i]==NULL) break; i++;`):
   30/37 words match, but 181261 bytes differ OUTSIDE the function's own
   range — a size mismatch (one word short), confirming a structural
   defect, not a simple value residue.
2. **Dropped the cached `start`, re-read `self->unk58` directly** for the
   equality test (matching this unit's established pattern of not trusting
   CSE across a branch — see `TaskCore__BroadcastToSlotElements`'s report for the same class
   of fix applied to a different pair of fields): fixed a register-identity
   residue on the initial load, but the function was still one word short.
3. **`while`-loop rewrite** (wrap-check duplicated once before the loop and
   once at the loop's tail): worse (14/37) — this restructuring lost the
   `self->unk50` value's live range across the loop entirely, forcing a
   fresh reload every iteration where retail (and the `for(;;)` shape)
   keeps it in one register for the function's whole body.
4. **Back to `for(;;)`, condition inverted to `if (elem != NULL) { i++;
   continue; } break;`** instead of `if (elem == NULL) break;` (with the
   implicit loop-back `i++` at the bottom): 22/37, all register residues
   gone, but still one word short. Reading the disassembly closely: retail
   places the loop's own `i++` **inside the taken branch's delay slot** as
   a genuinely separate physical instruction from the priming `i++` before
   the loop, whereas GCC's cross-jump pass recognised my source's two
   `i++;` statements (priming, and inside the `continue` branch) as
   identical RTL and MERGED them into one, jumping back to the shared
   instruction instead of emitting retail's second copy. A bare
   `__asm__("")` placed between them did not stop the merge (it only
   reorders scheduling within a block, not cross-jump unification across
   blocks).

**Fix:** move the increment INTO the array index expression as a
post-increment (`self->unk4C->unk18[i++]`), with an explicit `i--;`
compensating on the null-found (break) path. This is the established
`while (*p++) {} p--;` idiom (see DECOMPILATION_LEARNINGS, closed
previously on `strcat`) applied to an index variable instead of a raw
pointer — it ties the increment to the SAME instruction that performs the
array load/test (so it lands in that branch's own delay slot,
unconditionally, exactly once), rather than being a separate statement GCC
is free to unify with an unrelated identical-looking statement elsewhere
in the function. Matched immediately once applied.

### Proposed learning

**GCC 2.6.3's cross-jump pass will merge two SOURCE-DISTINCT but
RTL-IDENTICAL `i++;` statements into one shared instruction, even when
retail's own binary has two separate physical copies** — and a bare
`__asm__("")` between them does not stop it, because cross-jump is a
whole-block unification pass, not a local scheduling choice (the
`__asm__("")` barrier only ever affects the latter). When a loop's
continuation needs its own dedicated (non-shared) increment instruction to
match retail's word count, fold the increment into the same expression
that decides the branch (a post-increment inside an array index or a
pointer dereference) instead of writing it as a following statement --
this ties the increment to that specific branch's delay slot and makes it
ineligible for cross-jump merging with an unrelated priming increment
elsewhere in the function. (`TaskCore__FindNextFreeSlot`, 30/37 -> 37/37 across four
attempts)

## Naming (round 78, naming runner echo)

Renamed `func_` -> `Obj86B60__FindNextFreeSlot`. **Tier A**: A pure search: scans `unk4C->unk18[]` forward from `activeSlot`, wrapping at `slotCount`, for the next NULL (free) entry, reporting the index found. A find/search leaf, tier A by the 'mechanics are the purpose' rule for a clamp/search leaf.

## Track 4 (2026-09-25, round 84, alpha)

Renamed from Obj86B60__FindNextFreeSlot (tools/rename.py): the class prefix. Occupant of its gTaskCoreMethods slot, named for it in TASKCORE_SLOTS (`classtable.py gTaskCoreMethods`). The class (id 0x130, table gTaskCoreMethods) is unified as `TaskCore` in `include/TaskCore.h`; `self` is `TaskCore *` (it was the `Obj86B60` or `StreamTaskObj` view). Any source block above is the pre-unification spelling; the live body takes the unified types and slot names, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
