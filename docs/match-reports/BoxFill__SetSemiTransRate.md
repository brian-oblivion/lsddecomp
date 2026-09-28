# BoxFill__SetSemiTransRate — MATCHED (11/11 words)

> Renamed from `func_80040740` on 2026-09-25 (tools/rename.py). Address 0x80040740.

Unit: `src/ui/ScreenWidgets.c`. First attempt.

```c
s32 BoxFill__SetSemiTransRate(Obj6EAC0 *self, s32 a1) {
    return GetSetBitField(&self->unk58, 0x1C, 2, a1);
}
```

Sibling of `BoxFill__SetDisplay`/`BoxFill__SetSemiTrans`; base-table occupant of
`Obj6EAC0Methods::slot0x68`. Shift 0x1C, width 2, raw `a1` passed
through unchanged -- matches `scene_node.c`'s `SceneNode__SetSemiTransRate` shape.

### Proposed learning

None beyond `BoxFill__SetDisplay`'s entry.

## Naming

Round 54 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_80040740` | (kept `func_80040740` at track 3; `BoxFill__SetSemiTransRate` since round 85, see Track 4) | C |

**What is known.** A thin wrapper around `GetSetBitField(&self->flags,
shift, width, value)` (see `include/Task.h`'s own comment on
`flags`, renamed from `unk58` this round), the SAME generic
packed-bitfield-word accessor `scene_node.c`'s own sibling functions
(`SceneNode__SetDisplay`/`D374`/`D3A0`) wrap -- and those, the FIRST instances
of this exact idiom in the project, are still unnamed too, for the same
reason: `GetSetBitField` returns the bit's OLD value while setting a
NEW one, and without knowing what the individual bit MEANS, a name can
only restate the mechanics (shift/width/negation), which the existing
header/report prose already documents precisely. Kept `func_` rather
than invent a `Get`/`SetFlagNN`-shaped name for an unidentified bit,
consistent with this project's own precedent on the sibling group.

## Track 4 (2026-09-25, round 85, charlie)

Class 0x64 (was D_8006EAC0) is unified as BoxFill in include/BoxFill.h: Viewport__DrawNode draws a node whose class-id low byte is 0x64 with GsSortBoxFill over the GsBOXF at +0x058 (pri +0x044, `relative` +0x048, x/y +0x050/+0x054). The body now takes `BoxFill *`; zero bytes changed. Renamed from `func_80040740`: the +0x068 occupant (SceneNode's setSemiTransRate), the same call as SceneNode__SetSemiTransRate (shift 28, width 2) over the GsBOXF attribute (tier A). ApplyStyleDecorationIfSet calls it with 0.
