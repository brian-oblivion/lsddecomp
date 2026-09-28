# VariantSprite__NoOpSlotBC -- MATCHED (2/2)

> Renamed from `VariantSprite__func_57f40` on 2026-09-26 (tools/rename.py). Address 0x80057f40.

> Renamed from `Class879C4__func_57f40` on 2026-09-26 (tools/rename.py). Address 0x80057f40.

Unit: `src/world/dream_scene.c`. Address 0x80057f40. `jr $ra; nop`: splat
matched it itself, so there is no derivation.

## Naming

Renamed from `func_80057F40` on 2026-09-26 (round 87, track 4, tools/rename.py).
Tier C. `tools/classtable.py gVariantSpriteMethods` puts it at +0x0BC, this class's own slot `slotBC`.
The body is empty. Nothing in `src/` calls through the slot, so the placeholder `Class__func_xxxxx` form stays.

## Verify

`./build-and-verify.sh`: OK: build matches retail.

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

`VariantSprite__func_57f40` -> `VariantSprite__NoOpSlotBC` (tools/rename.py),
tier A: an empty leaf whose mechanics are its purpose, named after its slot
(+0x0BC) as the `NoOpSlotNN` precedent does (Actor, VabStreamObj,
CdStream). The "Tier C" line above is superseded. The slot stays `slotBC`:
nothing calls it.
