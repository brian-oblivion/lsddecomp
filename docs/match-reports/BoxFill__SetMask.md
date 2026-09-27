# BoxFill__SetMask — MATCHED (5/5 words)

> Renamed from `Obj6EAC0__SetMask` on 2026-09-25 (tools/rename.py). Address 0x800408a8.

> Renamed from `func_800408A8` on 2026-09-18 (tools/rename.py). Address 0x800408a8.

Unit: `src/ScreenWidgets.c`. First attempt.

```c
s32 BoxFill__SetMask(Obj6EAC0 *self, s32 a1) {
    return self->unk68 = (1 << a1) - 1;
}
```

`$v0` already holds the computed value at the point of `jr $ra` in
retail, so `return self->unk68 = expr;` (reusing the "set a value,
return the same value" idiom already documented for other setters)
reproduces it with no extra `move`.

Base-table occupant of `Obj6EAC0Methods::slot0xCC`; the derived table's
slot0xCC is `TextRow__SetText` (this unit, also queued).

### Proposed learning

None beyond what's already recorded.

## Naming

Round 54 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_800408A8` | `BoxFill__SetMask` | A |

**Evidence.** A pure leaf whose mechanics ARE its purpose (tier A):
`self->mask = (1 << a1) - 1`, a bitmask-of-the-low-`a1`-bits computation
and store, with no other logic. Renamed the field too (`unk68` -> `mask`,
this unit's own exclusive field -- see the struct-field rename commit).

## Track 4 (2026-09-25, round 85, charlie)

Class 0x64 (was D_8006EAC0) is unified as BoxFill in include/BoxFill.h: Viewport__DrawNode draws a node whose class-id low byte is 0x64 with GsSortBoxFill over the GsBOXF at +0x058 (pri +0x044, `relative` +0x048, x/y +0x050/+0x054). The body now takes `BoxFill *`; zero bytes changed. Renamed from `Obj6EAC0__SetMask`: the +0x0CC occupant, BoxFill's own `setMask`; mask = (1 << bits) - 1, and Reset passes 13. Nothing in BoxFill reads +0x068; FadeBox__Configure divides it (as `unk68`). Tier B: the mechanics only. The slot returns s32, the occupant's; Reset ignores it.
