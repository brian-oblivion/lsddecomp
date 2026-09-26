# VariantSprite__func_57f48 -- MATCHED (2/2)

> Renamed from `Class879C4__func_57f48` on 2026-09-26 (tools/rename.py). Address 0x80057f48.

Unit: `src/class_3bb8c_t.c`. Address 0x80057f48. `jr $ra; nop`: splat
matched it itself, so there is no derivation.

## Naming

Renamed from `func_80057F48` on 2026-09-26 (round 87, track 4, tools/rename.py).
Tier C. `tools/classtable.py gVariantSpriteMethods` puts it at +0x0C0, this class's own slot `slotC0`.
The body is empty. Nothing in `src/` calls through the slot, so the placeholder `Class__func_xxxxx` form stays.

## Verify

`./build-and-verify.sh`: OK: build matches retail.
