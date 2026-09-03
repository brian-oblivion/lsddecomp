# func_8001EA8C -- MATCHED (16/16 words)

Unit: `code_d294_c` (round 14). A standalone leaf, not yet reached by any
caller in this round's queue -- three-element vector subtraction between
two `s16` arrays, widening the result into an `s32` array.
`void func_8001EA8C(s32 *dest, s16 *b, s16 *a)`.

## Final source

```c
void func_8001EA8C(s32 *dest, s16 *b, s16 *a) {
    dest[0] = a[0] - b[0];
    dest[1] = a[1] - b[1];
    dest[2] = a[2] - b[2];
}
```

## Derivation notes

No struct type fits either input array -- both are read as bare 3-element
`s16` vectors (`lh` at offsets 0/2/4), and the result is written as a bare
3-element `s32` vector (`sw` at offsets 0/4/8). Declared with raw pointer
parameters rather than inventing a named type, since nothing here
constrains it further; a future caller reaching this function may narrow
the parameter types if it passes a named struct's address.

First-try match, no residue. Parameter naming follows the register order
(`a0`=`dest`, `a1`=`b`, subtracted; `a2`=`a`, subtracted from) rather than
guessing semantic names, since nothing here indicates what `a`/`b`
represent (possibly a target-minus-current delta for the same kind of
16-bit angle triple `Class6B5CCSub44::unk10/12/14` holds, given the
matching element count and halfword source width, but that is a guess,
not evidence -- left unstated).

No new struct or vtable-slot knowledge.
