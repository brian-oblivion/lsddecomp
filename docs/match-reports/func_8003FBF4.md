# func_8003FBF4 -- MATCH (9/9 words, first attempt)

Unit `code_2cc8c_e`, carved round 14.

```c
void func_8003FBF4(Class6E99CObj *self) {
    func_80021678(self->unk10);
}
```

`func_80021678` is Psy-Q library code (`asm/psyq_GsLinkObject4.s`), not
decompiled here; declared with the opaque `void *` shape its own body
forwards without dereferencing.

`self->unk10` (`Class6E99CObj`, `include/code_2cc8c.h`) is loaded as a plain
word and forwarded unmodified -- same base offset as `Class6B5CCObj`'s own
inherited `unk10` field in `code_d294.h` (a `u32` packed bit-flags word),
plausibly the same underlying field reused opaquely here, but kept as an
independent local view per this project's convention.

## Existing-declaration retype

`code_2cc8c_d.c`'s `func_8003F04C` already forward-declared this function
(`extern void func_8003FBF4(s32 a0);`) before this unit was carved. Retyped
the header declaration to `extern void func_8003FBF4(Class6E99CObj *self);`
to match the real signature -- ABI-identical (both a plain word register),
so this does not change that call site's own compiled bytes.
