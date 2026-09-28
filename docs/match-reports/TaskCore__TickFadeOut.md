# TaskCore__TickFadeOut — MATCHED (38/38)

> Renamed from `TaskCore__TickFadeColor` on 2026-09-28 (tools/rename.py). Address 0x8003cd48.

> Renamed from `Obj86B60__TickFadeColor` on 2026-09-25 (tools/rename.py). Address 0x8003cd48.

> Renamed from `func_8003CD48` on 2026-09-24 (tools/rename.py). Address 0x8003cd48.

**Unit:** Task · **Size:** 38 words · **Result:** byte-exact

## What it does

`Obj86B60Methods::slotC4` (confirmed by `Task.h`'s own header comment,
which already recorded this slot as "external (TaskCore__TickFadeOut)" from the
`Task` unit's own `classtable.py` work before this unit ever attempted
the function). Computes a greyscale-ish colour byte from two fields, fills a
3-byte buffer with it, forwards the buffer to two other objects, and returns
whether the byte exceeds a threshold.

```c
s32 TaskCore__TickFadeOut(Obj86B60 *self)
{
    s32 c = 0x80 - (self->unk1C * self->unk84);
    u8 buf[3];

    buf[0] = c;
    buf[1] = c;
    buf[2] = c;
    self->methods->slotE4(self, buf);
    self->unk78->methods->slotB8(self->unk78, 1, buf);
    return (u8)c >= 0x81;
}
```

## The one residue, and how it closed

First attempt used `u8 c` for the intermediate (matching the fact that the
value is ultimately stored into three `u8` array slots and compared as a
byte). That produced 37/38: every instruction matched except the constant
load feeding the subtraction, which compiled to `addiu $s0, $zero, -0x80`
(`li s0, -128`) instead of retail's `ori $s0, $zero, 0x80` (`li s0, 0x80`).

The two forms are numerically equivalent modulo 256 (`-128 ≡ 128 mod 256`),
so the STORED byte and the final masked comparison are unaffected either
way — which is exactly why it reads as "off by one instruction" rather than
a wrong answer. Root cause, confirmed with an isolated reproducer through
the pinned toolchain: when the subtraction's result is assigned directly
into a `u8`-typed local, GCC 2.6.3 -O2 folds the literal `0x80` into its
"efficient" 8-bit signed representation (`-128`) ahead of time, on the
theory that only the low byte will ever be read. When the same expression is
assigned into an `s32` local instead — narrowing only at the later `u8`
array stores and the explicit `(u8)c >= 0x81` comparison — the literal stays
a plain positive `0x80` and gets `ori`, matching retail exactly.

Reproducer (`s32 c = 0x80 - (a*b); return c;` vs `u8 c = 0x80 - (a*b); return
c;`) isolates it in ~15 lines; see the two objdumps below.

```
u8 c:  li v0,-128 / subu v0,v0,v1   (WRONG constant sign)
s32 c: li v0,0x80 / subu v0,v0,v1   (matches retail)
```

## Header note

No changes to `include/Task.h` were needed — `self->unk1C`,
`self->unk84`, `self->methods->slotE4`, `self->unk78`,
`self->unk78->methods->slotB8` were all already modelled from the sibling
unit's (`Task`) prior work on this same class, and every field/slot
type matched this function's actual usage with no adjustment.

### Proposed learning

**A literal constant folded into a `C - X` subtraction can flip sign
(`0x80` vs `-0x80`) depending purely on whether the RESULT is assigned into
a narrow (`u8`) local versus a wide (`s32`) one, even though the final
stored/compared byte value is identical either way.** When a one-instruction
residue is a `li`/`ori` vs `addiu`-negative-immediate mismatch on an
otherwise-matching subtraction, widen the intermediate to `s32` and narrow
only at the point(s) of use (array store, explicit `(u8)` cast in a
comparison) rather than typing the local at its "natural" narrow width.
(`TaskCore__TickFadeOut`, 37/38 -> 38/38)

## Naming (round 78, naming runner echo)

Renamed `func_` -> `Obj86B60__TickFadeColor`. **Tier B**: Computes a decreasing grey/colour byte from `frameCounter*unk84` each call, forwards the 3-byte buffer to two colour-consuming slots (`slotE4`, `unk78->slotB8` -- both established elsewhere as colour consumers, see `Unk4CObj::unk10`'s own "3-byte colour buffer" comment), and returns whether the byte has wrapped past a threshold -- the shape of a per-frame fade/countdown with a completion flag. Mechanics are solid; the game-visible purpose (what fades) is not.

## Track 4 (2026-09-25, round 84, alpha)

Renamed from Obj86B60__TickFadeColor (tools/rename.py): the class prefix. Occupant of its gTaskCoreMethods slot, named for it in TASKCORE_SLOTS (`classtable.py gTaskCoreMethods`). The class (id 0x130, table gTaskCoreMethods) is unified as `TaskCore` in `include/TaskCore.h`; `self` is `TaskCore *` (it was the `Obj86B60` or `StreamTaskObj` view). Any source block above is the pre-unification spelling; the live body takes the unified types and slot names, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 4 (2026-09-26, round 88, alpha)

bgLayer is a `BgLayer *` (include/BgLayer.h): the `Unk78Obj` view is gone and +0x0B8 is called as `setColor(bgLayer, 1, (BgLayerRgb *)buf)`. Byte-identical.

## Track 7 (2026-09-27, round 98, bravo)

`0x80 - frameCounter * fadeRate` and `(u8)c >= 0x81` are now `TASKCORE_FADE_FULL - ...` and `(u8)level > TASKCORE_FADE_FULL` (TaskCore.h, alpha's constant: 128, the colour that draws a texture at its own brightness). `>= 129` and `> 128` compile to the same sltiu, byte-exact. c -> level, buf -> color.

## Track 10 (2026-09-28, round 104, echo)

The six per-class aliases of `ColorRgb` (include/DrawSystem.h) -- BgLayerRgb, BoxFillRgb, FlatLightColor, LightRigRgb, ViewportRgb, TimBlockSrcColor -- are deleted and every use is spelled `ColorRgb`. Byte-identical. Measured for the MATCHING line in TaskCore__SetColors: the whole-struct copy is three `lb` then three `sb`, and rewriting one of the copies byte by byte loads each byte with `lbu` and interleaves the stores (asm-differ on the experiment), so the struct copy stays; the old line's "signed bytes" was wrong (ColorRgb's channels are u8; the lb comes from the block copy, not the type), and the same claim in GraphRoom__BuildGraphPoints' colour comment is corrected.
