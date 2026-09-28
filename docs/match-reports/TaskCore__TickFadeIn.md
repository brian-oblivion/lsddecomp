# TaskCore__TickFadeIn — MATCH (44/44 words)

> Renamed from `TaskCore__TickColorFade` on 2026-09-28 (tools/rename.py). Address 0x8003cc2c.

> Renamed from `Obj86B60__TickColorFade` on 2026-09-25 (tools/rename.py). Address 0x8003cc2c.

> Renamed from `func_8003CC2C` on 2026-09-24 (tools/rename.py). Address 0x8003cc2c.

**Unit:** task · **Size:** 44 instructions

## What it does

```c
s32 TaskCore__TickFadeIn(Obj86B60 *self)
{
    s32 prod;
    u8 buffer[3];

    prod = self->unk1C * self->unk84;
    buffer[0] = prod + self->unk90[0];
    buffer[1] = prod + self->unk90[1];
    buffer[2] = prod + self->unk90[2];
    self->methods->slotE4(self, buffer);
    self->unk78->methods->slotB8(self->unk78, 1, buffer);
    return (u8)prod >= 0x81;
}
```

Builds a 3-byte "tinted colour" buffer (`unk90` base colour + a
`unk1C * unk84` product, truncated to a byte per component -- `sb`'s own
truncation reproduces the C `u8 buffer[3]` element type with no extra
cast needed), forwards it to two different colour-consuming slots on two
different objects (`self->methods->slotE4` and `self->unk78->methods->
slotB8`), then returns whether the LOW BYTE of the product is `>= 0x81`
(computed as `!((u8)prod < 0x81)`, GCC 2.6.3's ordinary
`sltiu`+`xori` idiom for `>=`).

## Residue and fix (1 wasted attempt, then matched)

**First attempt (`buffer[i] = self->unk90[i] + prod;`, field first) scored
41/44, three `addu` operands swapped** (`addu v0,v0,s0` vs retail's `addu
v0,s0,v0`). Addition is commutative for the VALUE but the two operand
orders assemble to different bytes; retail's source evidently writes the
product FIRST (`prod + self->unk90[i]`), not the field first. Swapping the
operand order in the C source (no other change) matched immediately.

### Proposed learning

For a byte-truncated `a + b` where operand order is otherwise
inconsequential, GCC 2.6.3 preserves the SOURCE's left-to-right order in
the emitted `addu`'s operand fields -- if a residue is a single `addu`
with both operands swapped (same registers, same result), try reversing
the C expression's operand order before looking for anything more exotic.

## Struct knowledge established

- `Obj86B60::unk1C` (s32, +0x01C) -- confirmed as a genuine running value
  (multiplied here), not just a flag.
- `Obj86B60Methods::slotE4` (+0x0E4, external `func_8003D9D4`) --
  `void (*)(Obj86B60*, u8*)`.
- `Unk78Obj::methods->slotB8` (+0x0B8, external) --
  `void (*)(Unk78Obj*, s32, u8*)`.

## Provenance

round 2026-09-02, runner echo, unit task. 2 attempts.

## Naming (round 78, delta)

**Tier B.** `func_8003CC2C` -> `Obj86B60__TickColorFade`. Body: `prod =
frameCounter * unk84` (the fade rate, see `SetFadeRate`); builds a 3-byte
buffer `{unk90[i] + prod : i in 0..2}` (the base colour, see `SetColors`);
forwards that buffer to `self->methods->slotE4` and to
`self->unk78->methods->slotB8(self->unk78, 1, buffer)`; returns whether the
low byte of `prod` has reached `0x81`. That return value is exactly what
`TickFadeCallback` treats as "fade done" (nonzero -> `SetState(5)`).
Mechanically this is a per-tick RGB fade computation forwarded to a display
target; occupies slotB0 (also read as DATA -- a function pointer value, not
called -- by `SetFadeCallbackEnabled`). What the fade represents in-game
(and what `unk78` is) is not established, so tier B rather than A.

## Track 4 (2026-09-25, round 84, alpha)

Renamed from Obj86B60__TickColorFade (tools/rename.py): the class prefix. Occupant of its gTaskCoreMethods slot, named for it in TASKCORE_SLOTS (`classtable.py gTaskCoreMethods`). The class (id 0x130, table gTaskCoreMethods) is unified as `TaskCore` in `include/TaskCore.h`; `self` is `TaskCore *` (it was the `Obj86B60` or `StreamTaskObj` view). Any source block above is the pre-unification spelling; the live body takes the unified types and slot names, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 4 (2026-09-26, round 88, alpha)

bgLayer is a `BgLayer *` (include/BgLayer.h): the `Unk78Obj` view is gone and +0x0B8 is called as `setColor(bgLayer, 1, (BgLayerRgb *)buffer)`. Byte-identical.

## Track 7 (round 98, alpha)

Locals `prod`/`buffer` -> `level`/`color`; the `level + base` operand order
(the residue above) carries a MATCHING line. `(u8)prod >= 0x81` is now
`(u8)level > TASKCORE_FADE_FULL` (128, the neutral GsBG/sprite colour),
byte-identical.

## Track 10 (2026-09-28, round 104, echo)

The six per-class aliases of `ColorRgb` (include/draw_system.h) -- BgLayerRgb, BoxFillRgb, FlatLightColor, LightRigRgb, ViewportRgb, TimBlockSrcColor -- are deleted and every use is spelled `ColorRgb`. Byte-identical. Measured for the MATCHING line in TaskCore__SetColors: the whole-struct copy is three `lb` then three `sb`, and rewriting one of the copies byte by byte loads each byte with `lbu` and interleaves the stores (asm-differ on the experiment), so the struct copy stays; the old line's "signed bytes" was wrong (ColorRgb's channels are u8; the lb comes from the block copy, not the type), and the same claim in GraphRoom__BuildGraphPoints' colour comment is corrected.
