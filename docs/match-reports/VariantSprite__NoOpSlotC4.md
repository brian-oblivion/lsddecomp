# VariantSprite__NoOpSlotC4 -- MATCHED (2/2)

> Renamed from `VariantSprite__func_57f50` on 2026-09-26 (tools/rename.py). Address 0x80057f50.

> Renamed from `Class879C4__func_57f50` on 2026-09-26 (tools/rename.py). Address 0x80057f50.

Unit: `src/class_3bb8c_t.c`. Address 0x80057f50. `jr $ra; nop`: splat
matched it itself, so there is no derivation.

## Naming

Renamed from `func_80057F50` on 2026-09-26 (round 87, track 4, tools/rename.py).
Tier C. `tools/classtable.py gVariantSpriteMethods` puts it at +0x0C4, this class's own slot `slotC4`.
The body is empty. Nothing in `src/` calls through the slot, so the placeholder `Class__func_xxxxx` form stays.

## Verify

`./build-and-verify.sh`: OK: build matches retail.

## Track 6 (2026-09-26, round 93, bravo)

The class `Class879C4` is now `VariantSprite` (`include/VariantSprite.h`,
`python3 tools/renametype.py Class879C4 VariantSprite`), tier B: the
mechanics are certain and are the whole of what the class adds to Sprite --
`variant` (0 or 1) picks the texture cell the Sprite ctor binds
(`gVariantSpriteCells`) and the CLUT row the reset slot sets
(`gVariantSpriteClutX/Y`). What the sprites are in the game is not
established (their only builder is StyleEffect, kinds 2 and 3, and every
path passes variant 0), which is why it is not tier A. The table, getter,
allocator, methods and the three data tables followed the class name.
The same tool run rewrote `Class879C4` tokens inside this report's older
history prose (the known renametype behaviour pending an operator
decision); those lines were left as the tool wrote them.

`VariantSprite__func_57f50` -> `VariantSprite__NoOpSlotC4` (tools/rename.py),
tier A: an empty leaf, named after its slot (+0x0C4) as the `NoOpSlotNN`
precedent does. The "Tier C" line above is superseded. The slot stays
`slotC4`: nothing calls it.
