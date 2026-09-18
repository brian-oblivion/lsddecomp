> Renamed from `func_8002CB9C` on 2026-09-18 (tools/rename.py). Address 0x8002cb9c.

# VabStreamObj__Unmute -- MATCHED (16/16 words)

Unit: `code_179d8_e`. Runner: echo, round 17.

## Result

```c
s32 VabStreamObj__Unmute(VabStreamObj *self) {
    s32 flag;

    flag = self->muted;
    if (flag != 0) {
        flag = SsSetMute(0);
        self->muted = 0;
    }
    return flag;
}
```

Byte-exact, 16/16 words.

## Notes

`gVabStreamObjMethods`'s vtable slot +0x08C. The mirror of `VabStreamObj__Mute` (slot
+0x088) in shape (a `self->muted` boolean-ish flag guarded by a
`SsSetMute` call), but NOT byte-symmetric with it: unlike
`VabStreamObj__Mute`, which discards `SsSetMute`'s return value and stores a
literal constant instead, `VabStreamObj__Unmute` genuinely threads
`SsSetMute(0)`'s return value through as both the new flag value AND the
function's own return value -- there is no `li $v0,...` between the `jal`
and the following `sh`. See `VabStreamObj__Mute`'s report for the near-miss this
distinction caused on the first attempt at the sibling function.

(`func_800336CC` was `SsSetMute`'s placeholder name before the SDK-linking
track identified it -- never back-filled into this report until round 52.)

## Naming

Renamed `func_8002CB9C` -> `VabStreamObj__Unmute`, tier A. Confirmed
`gVabStreamObjMethods`'s own +0x08C slot; mirror of `VabStreamObj__Mute`
(unmute-if-muted via `SsSetMute(0)`) -- same tier-A leaf reasoning.
