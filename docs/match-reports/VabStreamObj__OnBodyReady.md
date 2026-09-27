# VabStreamObj__OnBodyReady -- MATCHED (27/27 words)

> Renamed from `func_8002C824` on 2026-09-18 (tools/rename.py). Address 0x8002c824.

Unit: `code_179d8_d`. Runner: echo, round 17.

## Result

```c
s32 VabStreamObj__OnBodyReady(VabStreamObj *self, s32 arg1) {
    s32 result;

    result = 0;
    if (self->bodyTransferPending != 0) {
        if (arg1 != 0) {
            SsVabTransCompleted(1);
            self->bodyTransferPending = 0;
            self->attrsReady = 1;
            self->methods->slot7C(self);
            result = 1;
        }
    }
    return result;
}
```

(Updated to current names: `func_8003370C` was this call's placeholder
name before the SDK-linking track identified it as Sony's
`SsVabTransCompleted` -- that rename predates this report's own update and
was never back-filled here until round 52; `ObjDA34::unk5A/unk58` are now
`VabStreamObj::bodyTransferPending/attrsReady`. Bytes unchanged either
way.)

`VabStreamObj::bodyTransferPending` typed `u16` (see below).
`gVabStreamObjMethods`'s vtable slot +0x078.

Byte-exact, 27/27 words.

## Notes

`self->methods->slot7C(self)`'s return value is discarded -- retail
unconditionally overwrites `$v1` (the `result` accumulator) with `1` right
after the `jalr`, regardless of what `slot7C` (`VabStreamObj__LoadVagAttrs`)
returned.

Two wrong turns before the match, both worth recording:

1. **First attempt mis-typed `bodyTransferPending` as `s16`.** Retail loads it
   with `lhu` (unsigned); an `s16` field compiles to `lh` (signed). Field
   changed to `u16`.
2. **Second attempt collapsed the two nested conditions into
   `if (a && b) { ...; return 1; } return 0;` with two `return` statements.**
   Retail has a SINGLE shared exit: a `result` local initialized to `0`
   before either test, set to `1` only on the success path, with both
   guard branches jumping to the SAME trailing `move $v0,$v1` epilogue
   rather than each condition having its own early return. Two-early-return
   C compiles to a different (larger, differently-scheduled) CFG even when
   it's logically equivalent to the accumulator form -- rewriting as nested
   `if`s around a single `result` variable that both guards can fall through
   to matched immediately.

### Proposed learning

When a boolean-guarded "do work, return 1, else return 0" residue has extra
instructions and a `j`/extra `move` near the end, try the single-shared-exit
accumulator form (`result = 0; if (...) { if (...) { ...; result = 1; } }
return result;`) before assuming a deeper structural mismatch -- GCC 2.6.3
at `-O2` does not always fold a logical-AND-guarded early return into the
same code as the accumulator form, even though they're semantically
identical.

## Naming

Renamed `func_8002C824` -> `VabStreamObj__OnBodyReady`, tier B. Confirmed
as `gVabStreamObjMethods`'s own +0x078 slot, and its only caller in this
unit is `VabStreamObj__AdvanceLoadState`'s own case-6 branch, dispatched right after
`SsVabTransBody` succeeds (`self->methods->slot78(self, 1)`) -- so this is
the notification step between "body transfer just finished" and "go load
the VAG attribute tables" (`slot7C`/`VabStreamObj__LoadVagAttrs`). Tier B,
not A: this reads as a virtual hook a subclass could override, and nothing
in this unit shows whether anything OTHER than `VabStreamObj__AdvanceLoadState` ever
dispatches through it.
