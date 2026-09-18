> Renamed from `func_8002CB9C` on 2026-09-18 (tools/rename.py). Address 0x8002cb9c.

# VabStreamObj__Unmute -- MATCHED (16/16 words)

Unit: `code_179d8_e`. Runner: echo, round 17.

## Result

```c
s32 VabStreamObj__Unmute(ObjDA34 *self) {
    s32 flag;

    flag = self->unk56;
    if (flag != 0) {
        flag = func_800336CC(0);
        self->unk56 = 0;
    }
    return flag;
}
```

Byte-exact, 16/16 words.

## Notes

`D_8006DA34`'s vtable slot +0x08C. The mirror of `VabStreamObj__Mute` (slot
+0x088) in shape (a `self->unk56` boolean-ish flag guarded by a
`func_800336CC` call), but NOT byte-symmetric with it: unlike
`VabStreamObj__Mute`, which discards `func_800336CC`'s return value and stores a
literal constant instead, `VabStreamObj__Unmute` genuinely threads
`func_800336CC(0)`'s return value through as both the new flag value AND the
function's own return value -- there is no `li $v0,...` between the `jal`
and the following `sh`. See `VabStreamObj__Mute`'s report for the near-miss this
distinction caused on the first attempt at the sibling function.
