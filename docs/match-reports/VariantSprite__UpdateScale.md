# VariantSprite__UpdateScale -- MATCHED (81/81)

> Renamed from `Class879C4__UpdateScale` on 2026-09-26 (tools/rename.py). Address 0x80057df4.

> Renamed from `D800879C4__UpdateScale` on 2026-09-26 (tools/rename.py). Address 0x80057df4.

> Renamed from `func_80057DF4` on 2026-09-25 (tools/rename.py). Address 0x80057df4.

Round 47 (runner charlie). Unit: `src/world/dream_scene.c`, a BRAND NEW carve
(this unit did not exist before round 47). Class: table `gVariantSpriteMethods` (49
slots, resolved with `tools/classtable.py 0x800879C4`) -- this is slot48,
immediately after `VariantSprite__SetVariantClut` (slot40, same unit, see its own
report).

Frameless leaf, zero `addiu $sp, $sp, -N`. The unit's carve-note history
had this filed as `nop_mflo_mfhi`-blocked; that construct was RESOLVED in
round 42 (`--no-nop-mflo-mfhi`), and `tools/uncarved.py` measures this
function blocker-clean.

## Signature

```c
void VariantSprite__UpdateScale(D_800879C4Obj_q *self, s32 arg1, s16 *pair);
```

`arg1` ($a1) is read by NOTHING in the whole function body -- retail
never touches it after function entry -- kept as an unused parameter
only to match the real arity (three integer-class argument registers are
consumed by the caller regardless). `pair` ($a2) is a caller-supplied
`{s16 whole; s16 frac;}` pair, twice over (axis 0 at `+0x0`/`+0x2`, axis
1 at `+0x4`/`+0x6`), read as a raw `s16 *` rather than a named struct --
same shape and same precedent as `task.c`'s `Viewport__SetTwist` and
`scene_node.c`'s `RatioToFixed12` (`task.h`'s own note on
`Ratio16`: "a different unit's own local view of the same shape,
not a shared type").

## Body

```c
void VariantSprite__UpdateScale(D_800879C4Obj_q *self, s32 arg1, s16 *pair) {
    s32 q1, r1, q2, ratio1;
    s32 q3, r3, q4, ratio2;
    s16 short1, short2;

    q1 = pair[0] / pair[1];
    r1 = pair[0] % pair[1];
    q2 = (r1 << 12) / pair[1];
    ratio1 = (q1 << 12) + q2;
    short1 = (s16)ratio1;

    q3 = pair[2] / pair[3];
    r3 = pair[2] % pair[3];
    q4 = (r3 << 12) / pair[3];
    ratio2 = (q3 << 12) + q4;
    short2 = (s16)ratio2;

    if (self->unk58 != 0) {
        self->unk5C = ((s16)ratio1 * self->unk5C) >> 12;
        self->unk60 = ((s16)ratio2 * self->unk60) >> 12;
    } else {
        self->unk80 = short1;
        self->unk82 = short2;
    }
}
```

Struct `D_800879C4Obj_q` is defined once, shared by both functions in
this unit -- see `VariantSprite__SetVariantClut`'s report for its full text and the
multiple-independent-local-views note.

## Derivation

The `q = a/b; r = a%b; q2 = (r<<12)/b; ratio = (q<<12)+q2;` block is the
project's already-established 20.12 fixed-point split-division idiom
(one `div` reused for quotient+remainder via `mflo`/`mfhi`, then a SECOND
`div` for the shifted remainder) -- confirmed live and matched at
`task.c:Viewport__SetTwist` and referenced from
`scene_node.c:RatioToFixed12`. Read straight off the two GTE-style
overflow-check idioms (`bnez`/`break 7` for divide-by-zero, the
`-1`/`0x80000000` pair check/`break 6` for `INT_MIN / -1`) that GCC 2.6.3
emits for a plain C `/` and `%` on `s32`.

The one non-obvious residue, found via a size-drift diagnostic (see
below): retail computes `short1`/`short2` -- 16-bit truncated copies of
each ratio -- **unconditionally, immediately after each ratio is
computed**, before the `if` even though only the `else` arm uses them.
The `if` arm re-derives its own 16-bit view from the full `s32` ratio at
the point of use (`(s16)ratio1 * self->unk5C`) rather than reusing
`short1`. Writing the truncation only inside the `else` arm (the more
"natural" C) drops exactly one instruction (`move $a0, $a3` /
equivalently `move $v1, $a1`) relative to retail, which is a whole-word
SIZE difference -- not a register swap -- so it does not merely produce
a near-miss on this function, it shifts every following byte in the
executable by 4 (found because `build-and-verify.sh` still went green
with `INCLUDE_ASM` at this point in the debugging session, but a manual
per-function `funcdiff.py` run on the wrong intermediate C showed a
145-KB "outside range" drift and a `build/lsdde.map` symbol,
`sVariantSpriteClutX`, landing 4 bytes off its documented address). Hoisting the
truncation to right after each ratio's computation reproduces the extra
`move` and closes the drift to zero.

## Verification

`./build-and-verify.sh` -- whole-image SHA1 matches retail (0 bytes
differ outside or inside the function's range). Committed alongside
`VariantSprite__SetVariantClut` (same unit, same commit, ROM-address order preserved).

### Proposed learning

A twelfth-lever candidate, in the same family as round 46's "a value
assigned on both branch outcomes belongs above the `if`": here the value
is used on only ONE branch, but retail still computes (and truncates) it
UNCONDITIONALLY before the branch, while the OTHER branch recomputes its
own narrower view from the wide value at the point of use rather than
sharing the hoisted one. The tell is a bare register `move` in retail's
disassembly with no corresponding arithmetic -- that is a copy of an
already-computed value into a second register, and the question to ask
is not "is this value used on both paths" but "is it computed once,
early, regardless of path, even though only one path consumes the
COPY".

## Naming

Round 79 naming pass (runner echo). The body above is the round-47 match;
the live source now reads `VariantSprite__UpdateScale(D800879C4Obj *self, s32
set, s16 *ratios)` with `spriteScaleX`/`spriteScaleY`, byte-identical.

| name | tier | evidence |
| --- | --- | --- |
| `VariantSprite__UpdateScale` (was `func_80057DF4`) | A | Slot `+0x048` of `gVariantSpriteMethods`; `tools/classtable.py gVariantSpriteMethods --vs gSceneNodeMethods` shows it overriding `SceneNode__UpdateScale`, and the name follows the slot. Every caller agrees it is a scale: dream_scene.c calls it through `updateScale` on the sprites with `sSpriteScaleLarge` ({6,5},{6,5},{1,1}) / `sSpriteScaleSmall` ({4,6},{4,6},{1,1}) or a caller-supplied table (`StyleEffect__SpawnSprites`), always `set = 1`. The body turns two num/den ratios into 20.12 and (unk58 == 0) stores them at `+0x80`/`+0x82`, which are GsSPRITE.scalex/scaley at `+0x64` (layout evidence in `VariantSprite__SetVariantClut`'s report). |
| param `set` | A | The base method's name for `$a1` (1 = assign, 0 = accumulate); this override never reads it. |
| param `ratios` | A | Two s16 {num, den} pairs, x then y. The round-47 description "`{s16 whole; s16 frac;}`" was wrong: the body computes `num / den` in 20.12, and the caller tables hold {6,5} and {4,6}. |
| fields `spriteScaleX` / `spriteScaleY` (`+0x80`/`+0x82`) | A | GsSPRITE.scalex/scaley. Unit-local struct: renamed in place. |
| fields `unk58`, `unk5C`, `unk60` | C (kept) | Known: the base init `Sprite__Reset` zeroes `+0x58`; when `+0x58` is non-zero this method multiplies the s32s at `+0x5C`/`+0x60` by the x/y ratios (`>> 12`) instead of setting the sprite scale. The draw path `Viewport__DrawNode` does not read them. Nothing found that sets `+0x58` non-zero, so no name. |

Could not name: `unk58`/`unk5C`/`unk60`, above.

## Track 4 (2026-09-26, round 87, alpha)

Renamed from `D800879C4__UpdateScale` (tools/rename.py); the class is
`VariantSprite` (`include/VariantSprite.h`). The override is named for its slot
(+0x048 `updateScale`), which it already was. The slot keeps SceneNode's
`void *table`; the occupant reads it as `s16 *ratios` (two num/den pairs).
The local view's `spriteScaleX/Y` are now `sprite.scalex` /
`sprite.scaley` (SpriteGs, s16 at +0x080 / +0x082); `unk58/5C/60` are
Sprite's fields (SPRITE_FIELDS). Byte-identical.

## Track 6 (2026-09-26, round 93, bravo)

The class `Class879C4` is now `VariantSprite` (`include/VariantSprite.h`,
`python3 tools/renametype.py Class879C4 VariantSprite`), tier B: the
mechanics are certain and are the whole of what the class adds to Sprite --
`variant` (0 or 1) picks the texture cell the Sprite ctor binds
(`sVariantSpriteCells`) and the CLUT row the reset slot sets
(`sVariantSpriteClutX/Y`). What the sprites are in the game is not
established (their only builder is StyleEffect, kinds 2 and 3, and every
path passes variant 0), which is why it is not tier A. The table, getter,
allocator, methods and the three data tables followed the class name.
The same tool run rewrote `Class879C4` tokens inside this report's older
history prose (the known renametype behaviour pending an operator
decision); those lines were left as the tool wrote them.

Its `s16 *ratios` is two `Ratio16` pairs; retyping it is proposed for the
class's track 7 pass (see `VariantSprite__SetVariantClut`'s report).

## Track 7 (2026-09-28, round 101, echo)

### Naming

| name | tier | evidence |
| --- | --- | --- |
| Sprite fields `accumScaleX` / `accumScaleY` (were `unk5C` / `unk60`, `+0x05C` / `+0x060`) | B | Mechanics only: while `unk58` is non-zero this method multiplies each by its axis's 20.12 ratio (`(ratio * v) >> 12`) and stores it back, in place of writing `sprite.scalex/scaley`, so the ratios accumulate into them. Renaming them in `SPRITE_FIELDS` (include/sprite.h) broke only this unit in the default build and under `-DNON_MATCHING`, so this is their whole accessor set: nothing else reads or writes them, `Viewport__DrawNode` included. What the accumulated value is for is not established (nothing found sets `unk58` non-zero), hence B. |
| param `ratios`: `s16 *` -> `Ratio16 *` | A | The body reads `[0]/[1]` and `[2]/[3]` as num/den, two scene_node.h `Ratio16`s (x, y), the type `Sprite__UpdateRotation` and `BgLayer__UpdateScale` already take; round 93's proposal. A type change, so the oracle decided: byte-identical. The prototype in include/VariantSprite.h changed with it; nothing calls it directly (only `gVariantSpriteMethods`' slot). |
| locals `xWhole`/`xRem`/`xFrac`/`xRatio`, `y...` (were `q1`/`r1`/`q2`/`ratio1`, `q3`/`r3`/`q4`/`ratio2`) | A | The split division: whole part `num / den`, remainder, the remainder's 12 fractional bits, and their 20.12 sum. |
| locals `xScale` / `yScale` (were `short1` / `short2`) | A | The 16-bit truncations stored into `sprite.scalex` / `.scaley`. |

`unk58` itself is also written by `Sprite__Reset` (src/graphics/sprite.c, outside
this job), so its name is proposed, not applied: see "Proposed field
names" below.

### Constants

The six `12`s (`<< 12` three times per axis, `>> 12` on the product) are
`FIX12_SHIFT` (include/common.h, 20.12 fixed point), as `sprite.c`'s
`Sprite__UpdateRotation` spells the same split division.

### Comment history (moved from src/class_3bb8c_q.c, round 101)

The function comment cited `tools/classtable.py gVariantSpriteMethods --vs
gSceneNodeMethods` for the override (in "Naming" above) and read `ratios`
"as a raw `s16 *` per the RatioToFixed12 precedent", which the retype to
`Ratio16 *` retired. The hoisted truncation's derivation (the size drift
it caused, found through `build/lsdde.map`) stays in "Derivation" above;
the source keeps one `MATCHING:` line on `xScale`.

## Proposed field names

- Sprite (include/sprite.h, `SPRITE_FIELDS`) `unk58` (`+0x058`) ->
  `accumulateScale`, tier B. Accessors: `Sprite__Reset` (src/graphics/sprite.c)
  zeroes it; this method reads it and, while it is non-zero, multiplies
  `accumScaleX/Y` by the ratios instead of writing `sprite.scalex/scaley`.
  No writer of a non-zero value found. Not applied: `Sprite__Reset` is
  outside this job. The rename touches `self->unk58` in src/graphics/sprite.c and
  src/world/dream_scene.c, plus the comments on the two fields after it and
  VariantSprite.h's banner line that names it.

Applied by the round 101 head at merge: `unk58` is `accumulateScale` (tier B), in `SPRITE_FIELDS`, `Sprite__Reset` and this method; zero bytes.
