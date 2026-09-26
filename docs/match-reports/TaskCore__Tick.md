# TaskCore__Tick — MATCH (30/30 words)

> Renamed from `Obj86B60__Tick` on 2026-09-25 (tools/rename.py). Address 0x8003ca1c.

> Renamed from `func_8003CA1C` on 2026-09-24 (tools/rename.py). Address 0x8003ca1c.

**Unit:** code_2cc8c · **Size:** 30 instructions

## What it does

```c
void TaskCore__Tick(Obj86B60 *self)
{
    Unk4CObj *target;
    s32 idx;

    target = self->unk4C;
    idx = self->unk58;
    if (target->unk24[idx] != NULL) {
        self->methods->slot108(self);
    } else if (idx == target->unkC) {
        self->methods->slot94(self);
    }
}
```

The first function in the unit to DEREFERENCE `self->unk4C` rather than
just null-check it (see `TaskCore__func_8003C7F4` and siblings) -- establishes real
fields on its pointee, `Unk4CObj`: `+0x00C` (s32, compared directly against
`self->unk58`) and `+0x024` (a `void **`, indexed by `self->unk58` and
null-checked: `target->unk24[idx]`).

## Residue and fix (1 wasted attempt, then matched)

**First attempt scored 29/30, one word off** (`lw $v0,0x24($a1)` in retail
vs. `lw $v0,0x30($a1)` in the build). The cause was a header bug, not a
source-shape issue: `Unk4CObj`'s padding field between `unkC` (+0x00C) and
`unk24` (+0x024) was declared `u8 pad10[0x010];` -- a **16-byte array**
(size literal `0x010`, not a `target - current` computation), which pushed
every field after it forward by 12 bytes. Fixed by giving the +0x010 field
its own name (`unk10[3]`, the INFERRED colour-buffer byte range read by
`TaskCore__SetState`, STALL) and correctly sizing the remaining pad as
`[0x024 - 0x013]`. **This is a copy-paste trap worth flagging generally**:
`pad<N>[<literal>]` reads as innocuous but silently means "N more bytes of
padding from here", not "pad up to offset N" -- only `pad<N>[<target> -
<N>]` means the latter, and the two are trivially confusable when writing a
header by hand from a table of offsets rather than by subtracting each pair.

### Proposed learning

When writing an offset-derived struct by hand, ALWAYS spell a padding
array's size as an arithmetic `<next_offset> - <this_offset>` expression,
never as a bare literal equal to the current offset -- `u8 pad10[0x010]` and
`u8 pad10[0x024 - 0x010]` look almost identical at a glance but differ by
12 bytes, and the bug only surfaces as a one-word-off residue several
fields later, in a DIFFERENT function than the one whose header edit caused
it. Grep a new header for `pad[0-9A-F]*\[0x[0-9A-F]*\]` (a bare hex literal,
not a subtraction) before trusting it.

## Struct knowledge established

- `Obj86B60::unk58` (s32, +0x058) -- OBSERVED here, an index into
  `unk4C->unk24[]`.
- `Unk4CObj::unkC` (s32, +0x00C), `::unk24` (`void **`, +0x024) -- both
  OBSERVED here.
- `Obj86B60Methods::slot108` (+0x108, external `TaskCore__BeginElementScroll`) and
  `::slot94` (+0x094, external `TitleMenu__RefreshViewValue`, shared with `TaskCore__SetState`
  STALL's case `a1==6`).

## Provenance

round 2026-09-02, runner echo, unit code_2cc8c. 2 attempts.

## Naming (round 78, delta)

**Tier A.** `func_8003CA1C` -> `Obj86B60__Tick`. Occupies slot90 in
`gTaskCoreMethods`; `gTitleMenuMethods` overrides the same slot with the
independently-named `TitleMenu__Tick`, settling the name the same way as
`SetState` above. `GraphRoomObj` inherits this exact function unmodified
(unoverridden occupant of its own table too).

## Track 4 (2026-09-25, round 84, alpha)

Renamed from Obj86B60__Tick (tools/rename.py): the class prefix. Occupant of its gTaskCoreMethods slot, named for it in TASKCORE_SLOTS (`classtable.py gTaskCoreMethods`). The class (id 0x130, table gTaskCoreMethods) is unified as `TaskCore` in `include/TaskCore.h`; `self` is `TaskCore *` (it was the `Obj86B60` or `StreamTaskObj` view). Any source block above is the pre-unification spelling; the live body takes the unified types and slot names, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
