# BoxFill__SetSemiTrans — MATCHED (11/11 words)

> Renamed from `func_80040714` on 2026-09-25 (tools/rename.py). Address 0x80040714.

Unit: `src/ui/ScreenWidgets.c`. First attempt.

```c
s32 BoxFill__SetSemiTrans(Obj6EAC0 *self, s32 a1) {
    return GetSetBitField(&self->unk58, 0x1E, 1, a1 != 0);
}
```

Sibling of `BoxFill__SetDisplay`/`BoxFill__SetSemiTransRate`; base-table occupant of
`Obj6EAC0Methods::slot0x64`. Shift 0x1E, width 1, no negation on the
result -- matches `SceneNode.c`'s `SceneNode__SetSemiTrans` shape exactly modulo
field name.

### Proposed learning

None beyond `BoxFill__SetDisplay`'s entry.

## Naming

Round 54 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_80040714` | (kept `func_80040714` at track 3; `BoxFill__SetSemiTrans` since round 85, see Track 4) | C |

**What is known.** A thin wrapper around `GetSetBitField(&self->flags,
shift, width, value)` (see `include/Task.h`'s own comment on
`flags`, renamed from `unk58` this round), the SAME generic
packed-bitfield-word accessor `SceneNode.c`'s own sibling functions
(`SceneNode__SetDisplay`/`D374`/`D3A0`) wrap -- and those, the FIRST instances
of this exact idiom in the project, are still unnamed too, for the same
reason: `GetSetBitField` returns the bit's OLD value while setting a
NEW one, and without knowing what the individual bit MEANS, a name can
only restate the mechanics (shift/width/negation), which the existing
header/report prose already documents precisely. Kept `func_` rather
than invent a `Get`/`SetFlagNN`-shaped name for an unidentified bit,
consistent with this project's own precedent on the sibling group.

## Track 4 (2026-09-25, round 85, charlie)

Class 0x64 (was D_8006EAC0) is unified as BoxFill in include/BoxFill.h: Viewport__DrawNode draws a node whose class-id low byte is 0x64 with GsSortBoxFill over the GsBOXF at +0x058 (pri +0x044, `relative` +0x048, x/y +0x050/+0x054). The body now takes `BoxFill *`; zero bytes changed. Renamed from `func_80040714`: the +0x064 occupant (SceneNode's setSemiTrans), the same GetSetBitField call as SceneNode__SetSemiTrans (shift 30, width 1, `on != 0`) over the GsBOXF attribute (tier A). ApplyStyleDecorationIfSet calls it with 1.
