# TaskCore__AdvanceSlotCursor — MATCHED (26/26)

> Renamed from `Obj86B60__AdvanceSlotCursor` on 2026-09-25 (tools/rename.py). Address 0x8003ddc8.

> Renamed from `func_8003DDC8` on 2026-09-24 (tools/rename.py). Address 0x8003ddc8.

**Unit:** task · **Size:** 26 words · **Result:** byte-exact

## What it does

`Obj86B60Methods::slot114` (already recorded in `task.h` before this
session, from an earlier round's `classtable.py` cross-reference). Advances
a ring-buffer index (`self->unk60[idx]`), wrapping to 0 at the per-slot
capacity (`self->unk5C[idx]`), and reports the new index through a call
that this function itself calls THROUGH a different slot, `+0x11C`.

```c
void TaskCore__AdvanceSlotCursor(Obj86B60 *self)
{
    s32 idx = self->unk58;
    s32 v = self->unk60[idx];

    v++;
    if (v >= self->unk5C[idx]) {
        v = 0;
    }
    self->methods->slot11C(self, v, 1);
}
```

## The one residue, and how it closed

First two attempts (direct `self->unk60[self->unk58] + 1`, then the same
with an explicit `idx` local, then with both `idx` and a separately-cached
`bound` local) all scored 24/26 or worse: the loaded value from
`self->unk60[idx]` landed in `$v1` where retail has it in `$a1` — the exact
register the value ends up in at the final call anyway (`slot11C`'s second
argument). Everything else, including the `+1` itself, already matched.

**Fix: split the load and the increment into two separate C statements**
(`s32 v = self->unk60[idx]; v++;` instead of
`s32 v = self->unk60[idx] + 1;`). The combined form let GCC fold the load
and the add into one RTL insn and pick its own register; splitting them
gave the load its own assignment, which is what let the compiler's
argument-register preferencing (the pseudo will live until the `jalr`
where it is `$a1`) claim `$a1` at the point of the load itself rather than
only at the final `move`. Caching the bound (`self->unk5C[idx]`) into its
own local, tried in isolation, made the score WORSE (21/26) — it competed
for the same register budget, so it went in the opposite direction from
the fix. Both changes looked similar (extra locals to "help" the compiler)
but had opposite effects; the load/increment split alone was the lever.

## Header additions

`include/task.h`: new slot `slot11C` on `Obj86B60Methods`
(`void (*)(Obj86B60 *, s32, s32)`), immediately after the existing
`slot118`. No existing slot's offset or type changed — `slot11C` was
previously undifferentiated space past the struct's last modelled slot.

### Proposed learning

**When a value's LAST use is as a call argument and a residue puts it in
the "wrong" general register (not `$s0`-`$s7`, an ordinary temp), try
splitting its computation from its own load into two statements** (load
into a bare local, then apply the arithmetic as a following statement)
before reaching for anything else. This let the compiler's own
argument-register preferencing claim the eventual call's hard register at
the load site instead of only at the final copy. Caching an UNRELATED
value used earlier in the same expression, tried as a parallel lever, made
the residue worse rather than better — the two moves are not
interchangeable even though both look like "help the compiler with an
explicit local". (`TaskCore__AdvanceSlotCursor`, `TaskCore__RetreatSlotCursor`, same fix both times)

## Naming (round 78, naming runner echo)

Renamed `func_` -> `Obj86B60__AdvanceSlotCursor`. **Tier B**: Advances `slotCounts[activeSlot]` by one, wrapping at `itemCounts[activeSlot]`, and forwards the new value through `slot11C` (== TaskCore__SetSlotCursor, confirmed by classtable.py). Kept tier B rather than A since it dispatches into a second function with its own further side effects, not a self-contained leaf.

## Track 4 (2026-09-25, round 84, alpha)

Renamed from Obj86B60__AdvanceSlotCursor (tools/rename.py): the class prefix. Occupant of its gTaskCoreMethods slot, named for it in TASKCORE_SLOTS (`classtable.py gTaskCoreMethods`). The class (id 0x130, table gTaskCoreMethods) is unified as `TaskCore` in `include/task_core.h`; `self` is `TaskCore *` (it was the `Obj86B60` or `StreamTaskObj` view). Any source block above is the pre-unification spelling; the live body takes the unified types and slot names, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
