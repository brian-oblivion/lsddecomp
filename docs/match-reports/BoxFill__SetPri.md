# BoxFill__SetPri — MATCHED (2/2 words)

> Renamed from `func_800408A0` on 2026-09-25 (tools/rename.py). Address 0x800408a0.

Unit: `src/code_2cc8c_f.c`. Trivial setter, first attempt.

```c
void BoxFill__SetPri(Obj6EAC0 *self, s32 a1) {
    self->unk44 = a1;
}
```

Base-table occupant of `Obj6EAC0Methods::slot0xC8` (see
`include/Task.h`); the derived table's slot0xC8 is
`TextRow__NoOpGetCell`, a splat-generated trivial `jr $ra; nop` already present
in `src/code_2cc8c_f.c` before this unit was carved.

### Proposed learning

None beyond what's already recorded — a plain one-field setter.

## Naming

Round 54 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_800408A0` | (kept `func_800408A0` at track 3; `BoxFill__SetPri` since round 85, see Track 4) | C |

**What is known.** Base-table occupant of `slotC8`: a plain one-field
setter, `self->unk44 = a1`. The derived table's own `slotC8` occupant
(`TextRow__NoOpGetCell`) does nothing at all for this same slot, which
argues `unk44` is a container-level attribute meaningless for leaf/child
instances -- but nothing in this unit ever READS `unk44` back, so its
real purpose (beyond "a stored word") is not established. Kept `func_`
per the naming rule ("when unsure, keep func_ and write down what you
know") rather than inventing a `Set<Guess>` name for an unread field.

## Track 4 (2026-09-25, round 85, charlie)

Class 0x64 (was D_8006EAC0) is unified as BoxFill in include/BoxFill.h: Viewport__DrawNode draws a node whose class-id low byte is 0x64 with GsSortBoxFill over the GsBOXF at +0x058 (pri +0x044, `relative` +0x048, x/y +0x050/+0x054). The body now takes `BoxFill *`; zero bytes changed. Renamed from `func_800408A0`: the +0x0C8 occupant, BoxFill's own `setPri` (tier A): it stores +0x044, the word whose low halfword Viewport__DrawNode passes to GsSortBoxFill as the ordering-table priority; Reset stores the ctor's third argument there (StyleBuildDecorSet passes 0x1FFF).
