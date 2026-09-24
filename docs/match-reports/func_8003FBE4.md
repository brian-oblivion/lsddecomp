# func_8003FBE4 -- MATCH (4/4 words, first attempt)

> **ROUND 34 (2026-09-12), runner bravo -- UNIT MOVE, nothing else.** This
> function is still game code and still MATCHED; it simply lives in a
> different file. Seven of `code_2cc8c_e`'s functions turned out to be Sony's
> and are now linked from SDK objects, which left this one wedged between
> `o` segments -- so it has its own one-function unit, **`code_2cc8c_e1`**
> (`src/code_2cc8c_e1.c`). The body below is unchanged and still compiles
> byte-exact. `include/code_2cc8c.h` still declares it for its one caller,
> but that new file does NOT include the header, so the two are no longer
> cross-checked by the compiler and must be kept in step by hand.


Unit `code_2cc8c_e`, carved round 14.

Plain global-pointer setter, same shape as `SetClipNear`:
`void func_8003FBE4(void *a0) { D_8008E794 = a0; }`.
