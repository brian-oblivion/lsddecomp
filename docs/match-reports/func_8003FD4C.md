# func_8003FD4C -- CONVERTED to a linked SDK object (round 34). NOT game code.

> **ROUND 34 (2026-09-12), runner bravo. THIS FUNCTION IS NOW LINKED FROM
> SONY'S OWN OBJECT `libgte/fog_01.o` (Psy-Q 3.3) AND IS NAMED `SetFogNear`.**
> The object's text covers exactly it. It was part of the five-object run
> 0x303F4..0x305B0 inside `code_2cc8c_e`; that unit is now split three ways
> and this function is no longer in it.
>
> **This RECLASSIFIES a matched function out of the game-code count, which is
> the correction CLAUDE.md asks for, not a regression.** Nothing here is
> assignable.
>
> Its declaration left `include/Task.h` (six units) in the same step
> rather than being renamed in place -- under Sony's name in a shared header
> it is the `conflicting types` failure against LIBGS.H that round 33 flagged
> for GsSetRefView2. The caller, `src/Task.c`, declares it locally
> under the Sony name with its own call site's shape.
>
> **Everything below is kept as the derivation it was, not as live guidance.**

_Previously: func_8003FD4C -- MATCH (25/25 words)_


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
