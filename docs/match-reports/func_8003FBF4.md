# func_8003FBF4 -- CONVERTED to a linked SDK object (round 34). NOT game code.

> **ROUND 34 (2026-09-12), runner bravo. THIS FUNCTION IS NOW LINKED FROM
> SONY'S OWN OBJECT `libgs/gs_111.o` (Psy-Q 3.3) AND IS NAMED `GsDrawOt`.**
> The object's text covers exactly it. It was part of the five-object run
> 0x303F4..0x305B0 inside `ScreenWidgets`; that unit is now split three ways
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

_Previously: func_8003FBF4 -- MATCH (9/9 words, first attempt)_


Unit `ScreenWidgets`, carved round 14.

```c
void func_8003FBF4(FadeBoxObj *self) {
    func_80021678(self->unk10);
}
```

`func_80021678` is Psy-Q library code (`asm/psyq_GsLinkObject4.s`), not
decompiled here; declared with the opaque `void *` shape its own body
forwards without dereferencing.

`self->unk10` (`FadeBoxObj`, `include/Task.h`) is loaded as a plain
word and forwarded unmodified -- same base offset as `SceneNodeObj`'s own
inherited `unk10` field in `SceneNode.h` (a `u32` packed bit-flags word),
plausibly the same underlying field reused opaquely here, but kept as an
independent local view per this project's convention.

## Existing-declaration retype

`Task.c`'s `Viewport__Flip` already forward-declared this function
(`extern void func_8003FBF4(s32 a0);`) before this unit was carved. Retyped
the header declaration to `extern void func_8003FBF4(FadeBoxObj *self);`
to match the real signature -- ABI-identical (both a plain word register),
so this does not change that call site's own compiled bytes.
