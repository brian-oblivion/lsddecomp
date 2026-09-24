# Class86F88__RefreshRows

> Renamed from `func_8005281C` on 2026-09-24 (tools/rename.py). Address 0x8005281c.

**Unit:** class_3bb8c_k · **Size:** 68 instructions (0x110 bytes) ·
**Status: MATCHED 68/68**, whole-image SHA1 green.

## Role

Refreshes the active window's display text: for each of up to 4 active
`self->unk40[]` elements, formats a fixed-width label via `func_8005292C`
(this unit, matched this round) into a local stack buffer and dispatches
`elem->methods->slotCC(elem, buf)`; then forwards `(arg1, arg2, arg3, 0)`
to `func_800529FC` (already matched) and optionally notifies `self` via
`slot60` (new this round, shared with `func_80052A58`).

```c
void Class86F88__RefreshRows(Class86F88 *self, s32 arg1, s32 arg2, s32 arg3, s32 arg4)
{
    s32 count;
    s32 i;
    char buf[0x20];
    Class86F88Elem **p;

    if (!self->unk50) {
        return;
    }
    count = self->unk10;
    p = &self->unk40[0];
    if (count >= 5) {
        count = 4;
    }
    for (i = 0; i < count; i++) {
        func_8005292C(self, buf, i, arg1, (char *)arg2);
        (*p)->methods->slotCC(*p, buf);
        p++;
    }
    func_800529FC(self, arg1, arg2, arg3, 0);
    if (arg4) {
        self->methods->slot60(self, 0);
    }
}
```

## Three separate reshaping steps, each fixing a distinct mismatch

This one needed three rounds of adjustment, each isolated by comparing
frame size / instruction count against retail before touching anything
else -- worth recording as a sequence, since fixing them in the wrong
order or bundled together would have been much harder to diagnose.

1. **Stack-buffer size, not padding.** The local formatting buffer only
   ever needs indices `0..0x1A` (27 bytes, `func_8005292C`'s own fixed
   width), but declaring `char buf[0x28]` (matching a first guess at "the
   gap between the outgoing-args area and the saved registers") produced
   an 8-byte-OVERSIZED frame (`addiu $sp,$sp,-0x70` vs retail's `-0x68`) --
   funcdiff's whole-image shift warning caught it immediately (170KB
   outside-range diff). `char buf[0x20]` (32 bytes) is what actually
   reproduces retail's frame size exactly. The lesson: back into a local
   array's size from the TOTAL FRAME SIZE budget empirically (try a size,
   check `addiu $sp,$sp,-N` in the compiled `.o`), not from "how many bytes
   are unaccounted for in the `.s`'s offset arithmetic" -- the two are not
   the same number here.
2. **`p = &self->unk40[0]` needs to be computed BEFORE the count-clamp
   `if`, not inside the `if (count > 0)` guard.** Retail computes this
   address in the DELAY SLOT of the `count < 5` clamp's `bnez` -- i.e.
   unconditionally, before the clamp even resolves, simply because nothing
   stops the compiler from scheduling an independent computation into an
   available delay slot early. Moving the `p = ...` statement earlier in
   the C (right after `count = self->unk10;`, before the clamp) let GCC
   make the same scheduling choice.
3. **No explicit `if (count > 0) { for (...) }` guard -- the bare `for`
   loop's own entry check already reproduces retail's SINGLE `blez`.**
   Wrapping the loop in an extra `if` produced a REDUNDANT DUPLICATE
   `blez` (two back-to-back, functionally-identical zero/negative checks)
   -- GCC 2.6.3 does not recognize that the `if` guard and the `for`
   loop's own bounds check are the same condition and dead-code-eliminate
   one of them. Retail's single check is what a bare `for (i = 0; i <
   count; i++)` compiles to on its own (a standard single-entry-check
   do-while lowering) -- no separate guard needed or wanted.

## Notes

`func_8005292C`'s call site here passes this function's own `arg2`
(established as a plain `s32`, since it is ALSO forwarded unmodified to
`func_800529FC`'s already-typed `s32 a2` parameter) through an explicit
`(char *)` cast to match `func_8005292C`'s own `base` parameter type
(`char *`, fixed by that function's internal pointer arithmetic -- see its
own report). Both typings are correct for their own function; the cast is
the bridge, not a contradiction.

`Class86F88ElemMethods::slotCC` (new slot, `+0x0CC`,
`void (*)(Class86F88Elem *, char *)`) added additively after the existing
`slotB8`.
