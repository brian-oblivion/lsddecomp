# func_8003FD4C -- MATCH (25/25 words)

Unit `code_2cc8c_e`, carved round 14.

```c
void func_8003FD4C(s32 a0, s32 a1) {
    func_80024B9C((-(a0 * 5 * 64)) / a1);
    func_80024BA8(0x1400000);
}
```

`func_80024B9C`/`func_80024BA8` are not decompiled in this unit (declared
`extern void f(s32)` from this call site's own shape only).

## Residue and fix

First attempt stored the division result in a named local (`quotient`) before
passing it to `func_80024B9C`. That forces GCC to `move` the `mflo` result
into `$a0` for the call. Retail's `mflo $a0` targets `$a0` directly (the
call's own argument register) with NO intervening `move` -- inlining the
whole division expression directly as the call argument let GCC allocate the
`mflo` destination straight into `$a0`, matching retail exactly. Same family
as the project's existing "value already in the register the call needs"
idiom, just for a computed expression rather than a stored field.

### Proposed learning

When a computed value (not just a loaded field) is used ONCE, immediately, as
a call argument, inlining the expression directly into the call (rather than
naming it in a local first) can be what lets GCC 2.6.3 land `mflo`/`mfhi`
straight into the argument register with no extra `move`. Generalizes the
existing "value already in the register the call needs" family past plain
field reads to computed arithmetic results.
