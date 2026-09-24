# GraphRoomObj__TickHighlight -- MATCHED (52/52)

> Renamed from `func_80058694` on 2026-09-24 (tools/rename.py). Address 0x80058694.

Unit: `src/class_3bb8c_t.c`. Class: `gGraphRoomMethods`, own vtable slot `+0x124`
(resolved via `tools/classtable.py gGraphRoomMethods`).

## Signature

```c
void GraphRoomObj__TickHighlight(D_80087AACObj *self);
```

## Body

```c
extern s32 D_8008ABBC;

void GraphRoomObj__TickHighlight(D_80087AACObj *self) {
    if (self->unk_0x238 != 0) {
        if (self->unk_0x1C >= 0x1F) {
            if (self->unk_0x23C < 4) {
                if ((self->unk_0x1C % 24) == 0) {
                    s8 idx = self->unk_0x240[self->unk_0x23C];
                    self->unk_0xA8[idx]->methods->slotB8(self->unk_0xA8[idx], 1, &D_8008ABBC);
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
`GraphRoomObj__BuildGraphPoints`/`GraphRoomObj__Destroy` build/destroy). `self->unk_0x1C % 24 ==
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
tools/funcdiff.py GraphRoomObj__TickHighlight   # 52/52
```
