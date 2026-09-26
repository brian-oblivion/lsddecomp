# VariantSprite__Update -- MATCHED (2/2)

> Renamed from `Class879C4__Update` on 2026-09-26 (tools/rename.py). Address 0x80057f38.

Unit: `src/class_3bb8c_t.c`. Address 0x80057f38. `jr $ra; nop`: splat
matched it itself, so there is no derivation.

## Naming

Renamed from `func_80057F38` on 2026-09-26 (round 87, track 4, tools/rename.py).
Tier B. `tools/classtable.py gVariantSpriteMethods` puts it at +0x098 `update`, overriding Sprite__Update: named for the slot and typed as it (`(VariantSprite *self, void *sender, s32 event)`).
The body is empty. The name says which slot it overrides, not what an update of this sprite means in the game.

## Verify

`./build-and-verify.sh`: OK: build matches retail.
