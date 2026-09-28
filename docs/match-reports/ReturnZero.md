# ReturnZero

> Renamed from `func_8002C3B8` on 2026-09-27 (tools/rename.py). Address 0x8002c3b8.

**Unit:** PlacementGridVabSound · **Size:** 2 instructions (0x8 bytes) ·
**Status: MATCHED 2/2**, whole-image SHA1 green.

## Role

Plain accessor, no parameters, always returns `0`. No other evidence in
this unit of what the value represents (no caller in this window), so
typed as a plain `s32` return.

```c
s32 ReturnZero(void)
{
    return 0;
}
```

## Naming (round 77, charlie -- track 3, no rename)

Checked against both class tables this unit's slice of code_179d8 touches:
not a slot of `gPlacementGridMethods` (30 slots, ends at `+0x078`, `tools/classtable.py
0x8006D940`) and not a slot of `gNullDriverMethods` (29 slots,
`tools/classtable.py 0x8006D9BC`) or `gCdDriverMethods` (`tools/classtable.py
0x8006D4E8`). Not in `gDataSourceClientGetters`'s client-getter array either
(`asm/data/5DB70.data.s`: 14 entries, `GetPlacementGridMethods` through
`GetLbdFileMethods`, none at this address). No caller found anywhere in
`src/`. Genuinely free-standing as far as this unit's own evidence goes;
kept `func_`.

## Naming (round 100, delta -- track 7)

`func_8002C3B8` -> `ReturnZero`, tier A by the naming rules' leaf clause:
the body is `return 0` and nothing else, so its mechanics are its purpose.
Re-checked the "no caller" finding against the whole image rather than
`src/`: no `jal`, and no data word anywhere in `asm/data/` holds
0x8002C3B8 (so it is in no method table and no getter array). The free
empty function `NoOp` (0x80026C80) is the project's precedent for naming
an unreferenced leaf by what it does.
