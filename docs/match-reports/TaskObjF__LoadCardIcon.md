# TaskObjF__LoadCardIcon -- MATCH (71/71 words, ~3 attempts)

> Renamed from `Class86E00_3bb8c_g__LoadCardIcon` on 2026-09-23 (tools/rename.py). Address 0x8004fe24.

> Renamed from `func_8004FE24` on 2026-09-23 (tools/rename.py). Address 0x8004fe24.

Unit `class_3bb8c_g`, class `Class86E00_3bb8c_g`. "Load a card-slot resource
by index, if not already loaded" -- builds a `CARD\<NAME>.TIM` path (same
naming shape as `Obj86ED0__LoadCardResources`'s `CARD\COMINPUT.TIM`, a different unit),
loads it through the `ChildObj86ED0` short-lived-handle idiom, and stashes
the derived object into `self->unk70`.

```c
extern ChildObj86ED0 *func_8003B39C(char *path);
extern ChildObj86ED0 *New_D8006ED4C(ChildObj86ED0 *arg0, void *arg1, s32 arg2);

extern char *gCardIconNames[];
extern const char gCardPathPrefix[]; /* "CARD\\" */
extern const char gCardPathSuffix[]; /* ".TIM" */
extern s32 D_80086EC4; /* 3-word opaque block, New_D8006ED4C's arg1, address-only here */
extern s32 D_8008AA94; /* opaque block, the fresh unk70's own slot4C arg2, address-only here */

void TaskObjF__LoadCardIcon(Class86E00_3bb8c_g *self, s32 arg1)
{
    char path[0x20];
    char *buf;
    char *name;
    ChildObj86ED0 *handle;
    Class86E00Unk70Obj_3bb8c_g *newVal;

    if (arg1 >= 0x11) {
        return;
    }
    if (self->unk68 == 0) {
        return;
    }
    if (self->unk70 != NULL) {
        return;
    }

    buf = path;
    name = gCardIconNames[arg1];
    buf[0] = '\0';
    strcat(buf, gCardPathPrefix);
    strcat(buf, name);
    strcat(buf, gCardPathSuffix);

    handle = func_8003B39C(buf);
    handle->methods->slot78(handle);
    newVal = New_D8006ED4C(handle, (void *)&D_80086EC4, 0);
    self->unk70 = newVal;
    handle->methods->release(handle);
    newVal->methods->slot4C(newVal, self->unk68, (void *)&D_8008AA94);
}
```

## Deriving the shape

Three early-return guards (`arg1 >= 0x11`, `self->unk68 == 0`,
`self->unk70 != NULL`) all branch to the same epilogue -- textbook
straight-line `if (...) return;` chain, confirmed since the `.s` shows
three independent `beqz`/`bnez` to the identical label.

The path-building block does NOT call the already-matched
`BuildFileName` (`dest[0]=0; if(arg2) strcat(dest,arg2); strcat(dest,arg1);
strcat(dest,arg3);`, `src/code_171e0.c`) -- it inlines the same three-strcat
shape directly with no null-guard on the first strcat, which is what rules
out an actual call to that helper (a call would need the conditional
branch). Reading the `strcat` argument order off the delay slots
(`strcat(buf,"CARD\\")`, `strcat(buf,name)`, `strcat(buf,".TIM")`) gives
`buf = "CARD\" + name + ".TIM"`, `name = gCardIconNames[arg1]` -- the same
`CARD\<NAME>.TIM` shape as `Obj86ED0__LoadCardResources`'s `CARD\COMINPUT.TIM` (that
unit builds it via a real `BuildFileName` call instead; this one just
happens to inline the identical three-piece concatenation).

`gCardIconNames` (`asm/data/76DC8.data.s`) is a flat 17-word (`0x11`, matching
the `arg1 >= 0x11` guard) array; most entries are `D_8008AAxx` rodata
string pointers, a handful are raw non-pointer literal words (`0x100`,
`0x2000`, and three bare `0x800115xx` addresses) that this call site never
reaches for the `arg1` values this function is actually invoked with --
declared as a plain `extern char *gCardIconNames[];`, which is enough to index
without needing the full contents.

The `func_8003B39C` / `slot78` / `New_D8006ED4C` / `release` sequence on
the temp `handle` is the exact idiom already established by
`Obj86ED0__LoadCardResources` (`docs/match-reports/Obj86ED0__LoadCardResources.md`, a DIFFERENT unit,
`class_3bb8c_i`) using the SAME shared `ChildObj86ED0`/`ChildMethods86ED0`
type from `class_3bb8c.h` (`slot78`/`release` already declared there).

`self->unk70` is ALREADY typed `Class86E00Unk70Obj_3bb8c_g *` in this unit
(established by the already-matched `TaskObjF__TickCardIcon`, same file). Assigning
`New_D8006ED4C`'s `ChildObj86ED0 *` return into it is the same
implicit-pointer-type-mismatch-is-harmless pattern already documented in
`Obj86ED0__LoadCardResources`'s own report (`self->unk44 = New_Obj6EAC0(...)` there) --
a warning, not an error, zero byte cost. Extended
`Class86E00Unk70ObjMethods_3bb8c_g` (in `include/class_3bb8c.h`)
ADDITIVELY with `slot4C` at `+0x04C`
(`void (*slot4C)(Class86E00Unk70Obj_3bb8c_g *self, s32 arg1, void *arg2)`),
padding `+0x008` through `+0x04C` -- the call's own evidence types `arg1`
as `self->unk68` forwarded verbatim (that field is already the owning
struct's established bare `s32`, per its own header comment: "never
dereferenced in this unit") and `arg2` as an address-only opaque block
(`D_8008AA94`).

## The one real residue: buffer/handle register reuse and a discarded call result

First working version (semantically identical, `path` used directly with
no `buf`/`name` locals, and `self->unk70 = New_D8006ED4C(...)` assigned
straight into the struct field with no intermediate local) scored 62/71:
retail keeps the temp-buffer address AND the `func_8003B39C` handle in the
SAME callee-saved register across all four calls that need it (computed
`addiu $s0,$sp,0x10` once, reused via `move a0,s0` for every `strcat` and
the `func_8003B39C` call, then reassigned `s0=v0` to hold the handle for
the rest of the function) -- passing `path` as a bare array argument at
each call site instead let the compiler recompute `addiu $a0,$sp,0x10`
fresh every time rather than keeping one persistent register. Introducing
`buf = path;` and always calling through `buf` fixed this half.

The remaining 9-word gap: retail keeps `New_D8006ED4C`'s return value in a
register (`s1`) both for the `self->unk70 = ...` store AND for every
subsequent use (`handle->methods->release(handle)`'s own... no, actually
for the following `newVal->methods->slot4C(newVal, ...)` call's `a0`) --
assigning the call's result directly into `self->unk70` and then reading
`self->unk70` back out of memory for the final `slot4C` call cost an extra
`lw` where retail has a register move. Introducing the `newVal` local
(holding the call result, assigned into `self->unk70`, and reused for the
final call) closed this to byte-exact.

### Proposed learning

Same shape as `Obj86ED0__LoadCardResources`'s already-documented lesson, but for a
SINGLE-attach path rather than a two-block repeat: when a call's return
value is BOTH stored into a struct field AND consumed again later in the
same function, don't write `self->field = call(...); ...
self->field->methods->foo(self->field, ...);` even though it reads
naturally -- introduce a named local for the call result, assign it into
the field, and drive every later use through the local. The struct field
gets re-read from memory each time it's referenced source-level; retail
consistently keeps the value in a register instead once it has been
computed.

## Naming

`TaskObjF__LoadCardIcon` (was `func_8004FE24`), tier B (head review, round 73:
the body proves "load `CARD\<name>.TIM` once"; that the image is an ICON is
inferred from the memcard context, not shown by a consumer). Original filing, tier A: the
whole body is the load itself (three early-return guards, then build
`CARD\<name>.TIM`, load it through the generic resource-handle idiom, and
stash the result in `self->unk70` if not already loaded) -- a load-if-
absent action whose mechanics ARE its purpose, same standard as a getter
or cache-fill.
