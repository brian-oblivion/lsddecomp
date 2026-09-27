# BoxFill__SetDisplay — MATCHED (12/12 words)

> Renamed from `func_800406E4` on 2026-09-25 (tools/rename.py). Address 0x800406e4.

Unit: `src/code_2cc8c_f.c`. First attempt.

```c
s32 BoxFill__SetDisplay(Obj6EAC0 *self, s32 a1) {
    return GetSetBitField(&self->unk58, 0x1F, 1, a1 == 0) == 0;
}
```

Same generic packed-bitfield-word accessor already documented in
`include/code_d294.h` (`GetSetBitField(word, shift, width, value)`),
here over `self->unk58` (a fresh field, not `code_d294`'s `unk10`).
Shift/width/negate shape matches `code_d294.c`'s own
`SceneNode__SetDisplay` verbatim (`GetSetBitField(&self->unk10, 0x1F, 1,
a1 == 0) == 0`), just against a different field.

### Proposed learning

The `GetSetBitField`-wrapper family generalises past `code_d294`: any
unit with a packed-bitfield-word field can reuse the exact same
shift/width/`==0` idioms already catalogued there. Worth checking
`code_d294.h`'s comment block first whenever a new unit's disassembly
shows a `jal GetSetBitField` (or the sibling helper it's built from).

## Naming

Round 54 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_800406E4` | (kept `func_800406E4` at track 3; `BoxFill__SetDisplay` since round 85, see Track 4) | C |

**What is known.** A thin wrapper around `GetSetBitField(&self->flags,
shift, width, value)` (see `include/TaskViewport.h`'s own comment on
`flags`, renamed from `unk58` this round), the SAME generic
packed-bitfield-word accessor `code_d294.c`'s own sibling functions
(`SceneNode__SetDisplay`/`D374`/`D3A0`) wrap -- and those, the FIRST instances
of this exact idiom in the project, are still unnamed too, for the same
reason: `GetSetBitField` returns the bit's OLD value while setting a
NEW one, and without knowing what the individual bit MEANS, a name can
only restate the mechanics (shift/width/negation), which the existing
header/report prose already documents precisely. Kept `func_` rather
than invent a `Get`/`SetFlagNN`-shaped name for an unidentified bit,
consistent with this project's own precedent on the sibling group.

## Track 4 (2026-09-25, round 85, charlie)

Class 0x64 (was D_8006EAC0) is unified as BoxFill in include/BoxFill.h: Viewport__DrawNode draws a node whose class-id low byte is 0x64 with GsSortBoxFill over the GsBOXF at +0x058 (pri +0x044, `relative` +0x048, x/y +0x050/+0x054). The body now takes `BoxFill *`; zero bytes changed. Renamed from `func_800406E4`: the +0x060 occupant (SceneNode's setDisplay), and the body is SceneNode__SetDisplay's own (GetSetBitField shift 31, width 1, `on == 0`, result `== 0`) over the GsBOXF attribute at +0x058 instead of the GsDOBJ2 one (tier A). GraphRoom__Update calls it on points[0] with the hour's low bit (a blink).
