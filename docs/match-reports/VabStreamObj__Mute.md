# VabStreamObj__Mute -- MATCHED (17/17 words)

> Renamed from `func_8002CB58` on 2026-09-18 (tools/rename.py). Address 0x8002cb58.

Unit: `PlacementGridVabSound`. Runner: echo, round 17.

## Result

```c
s32 VabStreamObj__Mute(VabStreamObj *self) {
    s32 flag;

    flag = self->muted;
    if (flag == 0) {
        SsSetMute(1);
        flag = 1;
        self->muted = flag;
    }
    return flag;
}
```

(Updated to round 52's names: `func_800336CC` was `SsSetMute`'s
placeholder name before the SDK-linking track identified it -- never
back-filled into this report until now; `ObjDA34::unk56` is now
`VabStreamObj::muted`. Bytes unchanged.)

Byte-exact, 17/17 words.

## Notes

`gVabStreamObjMethods`'s vtable slot +0x088 (confirmed against `tools/classtable.py
gVabStreamObjMethods`'s output).

**First attempt was a near-miss (6/17, and it warned of an out-of-range
size drift) from a single wrong detail: I initially wrote
`result = SsSetMute(1);` -- using the CALL's return value as the flag's
new value.** Retail actually discards `SsSetMute`'s return and
overwrites `$v0` with the LITERAL constant `1` (`li v0,1`) right after the
call, before storing it to `self->muted` and returning it. Using the call's
return value forced GCC to preserve the pre-call flag value across the call
in a second register (`$v1`), producing two extra `move` instructions that
don't exist in retail. Once the assignment became a literal `1` instead of
the call's result, the extra register pressure disappeared and the function
matched immediately.

The general lesson: **when a stored/returned value coincides with a value
computed just before a call, check whether retail's asm ACTUALLY threads the
call's return value through, or whether it discards it and uses a constant
instead** -- an `li` immediately after a `jal` with no other use of the call's
`$v0` result is the tell.

### Proposed learning

A `jal` followed immediately by `li $v0, N` (or similar) with the call's own
`$v0` never otherwise read is retail discarding that call's return value
and using a constant instead. Reusing the call's return value where retail
used a constant is a cheap, easy-to-miss source of spurious register
shuffling (extra `move`s) that shows up as a large early residue rather than
a one-word near-miss -- worth checking before assuming a deeper structural
mismatch. Confirmed twice in the same commit: `VabStreamObj__Mute` (constant `1`)
and its mirror `VabStreamObj__Unmute` (which, by contrast, DOES thread the call's
return value through -- no `li` between the `jal` and the following store,
so the two functions are NOT structurally identical despite looking like a
symmetric flag get/set pair at a glance).

## Naming

Renamed `func_8002CB58` -> `VabStreamObj__Mute`, tier A. Confirmed
`gVabStreamObjMethods`'s own +0x088 slot; body is an unambiguous
"mute if not already muted" (`SsSetMute(1)`, a real Sony call) -- mechanics
ARE the purpose for a leaf like this.
