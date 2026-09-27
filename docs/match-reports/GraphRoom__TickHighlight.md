# GraphRoom__TickHighlight -- MATCHED (52/52)

> Renamed from `GraphRoomObj__TickHighlight` on 2026-09-26 (tools/rename.py). Address 0x80058694.

> Renamed from `func_80058694` on 2026-09-24 (tools/rename.py). Address 0x80058694.

Unit: `src/class_3bb8c_t.c`. Class: `gGraphRoomMethods`, own vtable slot `+0x124`
(resolved via `tools/classtable.py gGraphRoomMethods`).

## Signature

```c
void GraphRoom__TickHighlight(D_80087AACObj *self);
```

## Body

```c
extern s32 gGraphPointHighlightColor;

void GraphRoom__TickHighlight(D_80087AACObj *self) {
    if (self->unk_0x238 != 0) {
        if (self->unk_0x1C >= 0x1F) {
            if (self->unk_0x23C < 4) {
                if ((self->unk_0x1C % 24) == 0) {
                    s8 idx = self->unk_0x240[self->unk_0x23C];
                    self->unk_0xA8[idx]->methods->slotB8(self->unk_0xA8[idx], 1, &gGraphPointHighlightColor);
                    self->unk_0x23C += 1;
                }
            }
        }
    }
}
```

Four nested guards, all gating a single call through
`self->unk_0xA8[idx]->methods->slotB8` (a new opaque entry type,
`D_80087AACEntry`, for the 100-entry `unk_0xA8` array this unit's own
`GraphRoom__BuildGraphPoints`/`GraphRoom__ReleaseGraphPoints` build/destroy). `self->unk_0x1C % 24 ==
0` reproduces retail's `multu`/`mfhi`/reconstruct-and-compare magic-number
sequence with a plain `%`, per the existing `x % N for a compile-time
constant N` learning.

## Two things that were not obvious from a first read

1. **`self->unk_0x1C`/`unk_0x23C` need unsigned comparisons.** Both
   guards compile to `sltiu`, not `slti`; typing the fields `u32` (not
   `s32`) reproduces that without an explicit cast at the comparison
   site.
2. **The third guard (`unk_0x23C`) is inverted from what its `sltiu`
   might suggest at a glance.** Its branch is `beqz`, not `bnez` like
   the second guard's -- meaning "continue" requires `unk_0x23C < 4`,
   not `>= 4`. Misreading this the first time (matching the SECOND
   guard's branch polarity instead of reading THIS guard's own
   instruction) produced a 51/52 near-miss, one word off, at exactly this
   branch. Read every guard's own branch opcode (`beqz` vs `bnez`)
   independently -- do not assume sibling guards share polarity just
   because they look structurally similar.

## Verify

```
./build-and-verify.sh   # build exit=0, OK: build matches retail
tools/funcdiff.py GraphRoom__TickHighlight   # 52/52
```

## Naming (round 75, track 3)

**`GraphRoom__TickHighlight`** -- tier B. Own vtable slot +0x124
(`tools/classtable.py gGraphRoomMethods` -- the class's own highest slot,
past the inherited range). Four nested guards gate a single call through
`points[idx]->methods->highlight`, advancing `highlightCount` (0..3) once
per `elapsedHours % 24 == 0` tick once `elapsedHours >= 0x1F` -- reads as
"once a day, once the room has been open long enough, highlight the next
ScoreDayLog match" (in-game trigger cadence not independently confirmed).

## Track 4 (2026-09-25, round 85, charlie)

points[] are BoxFills (include/BoxFill.h); the deleted `GraphRoomPoint` view's `highlight` (+0x0B8) is setColor(1, &gGraphPointHighlightColor): the highlight is a colour overwrite. Zero bytes.

## Track 4 (2026-09-26, round 87, alpha): renamed `GraphRoomObj__TickHighlight` -> `GraphRoom__TickHighlight`

The class is unified in `include/GraphRoom.h` (class id 0x2F130, table `gGraphRoomMethods`, parent TaskCore; `tools/classtable.py gGraphRoomMethods --vs gTaskCoreMethods`). The class name drops round 75's `Obj` suffix (FINISHING-PLAN track 4 step 2); every function prefix moved with it. The unit's GraphRoomObj/GraphRoomMethods views are deleted and `self` is `GraphRoom *`; inherited fields and slots carry TaskCore's and IntermediateBase's names. Zero bytes changed (build-and-verify OK, typeviews --warnings 0 new). Prefix only. The class's own slot +0x124, `tickHighlight`. The field the round-19 view called elapsedHours is IntermediateBase's s32 frameCounter; the body's `sltiu`/`divu` need it unsigned, so it is read as `(u32)self->frameCounter`.

## Track 7 (2026-09-27, round 97, delta)

- **Naming: `D_8008ABBC` -> `gGraphPointHighlightColor`** (tier A): sdata `00 FF 00`, green; this function `setColor(1, ...)`s (overwrite) each matched dot to it. Its only user. Declared `GraphPointColor` instead of `s32`: zero bytes changed.
- Local `idx` -> `dot`; `highlightCount < GRAPH_SCORE_MOOD_COUNT`; `0x1F` is decimal 31. The 31 and 24 stay literals (one highlight every 24 frames once frameCounter passes 30): a name would only restate them.

## Track 6 (2026-09-27, round 98, delta): `GraphPointColor` -> `BoxFillRgb`

The local `GraphPointColor` (round 97) is retired onto include/BoxFill.h's `BoxFillRgb`: it is New_BoxFill's and setColor's colour argument, the same record BoxFill__ApplyColor copies whole into the GsBOXF r, g, b (code_2cc8c_f's `RGB80040790`, retired onto it the same round). Same layout (signed, three bytes); `gGraphPointNewestColor`, `gGraphPointBaseColor`, `gGraphPointHighlightColor` and the local `rgb` are now declared `BoxFillRgb`. Zero bytes changed: whole image green, 0 new typeviews warnings, nonmatching green.
