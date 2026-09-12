# func_8003FB0C -- MATCH (4/4 words, first attempt)

> **ROUND 34 (2026-09-12), runner bravo -- UNIT MOVE, nothing else.** This
> function is still game code and still MATCHED; it simply lives in a
> different file. Seven of `code_2cc8c_e`'s functions turned out to be Sony's
> and are now linked from SDK objects, which left this one wedged between
> `o` segments -- so it has its own one-function unit, **`code_2cc8c_e0`**
> (`src/code_2cc8c_e0.c`). The body below is unchanged and still compiles
> byte-exact. `include/code_2cc8c.h` still declares it for its one caller,
> but that new file does NOT include the header, so the two are no longer
> cross-checked by the compiler and must be kept in step by hand.


Unit `code_2cc8c_e`, carved round 14.

Plain global-pointer setter: `void func_8003FB0C(void *a0) { D_800902E4 = a0;
}`. `D_800902E4` is otherwise unreferenced anywhere else decompiled so far;
declared `void *` since nothing dereferences it here.
