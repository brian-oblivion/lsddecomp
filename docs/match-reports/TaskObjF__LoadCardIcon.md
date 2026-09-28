# TaskObjF__LoadCardIcon -- MATCH (71/71 words, ~3 attempts)

> Renamed from `Class86E00_3bb8c_g__LoadCardIcon` on 2026-09-23 (tools/rename.py). Address 0x8004fe24.

> Renamed from `func_8004FE24` on 2026-09-23 (tools/rename.py). Address 0x8004fe24.

Unit `class_3bb8c_g`, class `Class86E00_3bb8c_g`. "Load a card-slot resource
by index, if not already loaded" -- builds a `CARD\<NAME>.TIM` path (same
naming shape as `TextEntry__LoadCardResources`'s `CARD\COMINPUT.TIM`, a different unit),
loads it through the `ChildObj86ED0` short-lived-handle idiom, and stashes
the derived object into `self->unk70`.

```c
extern ChildObj86ED0 *New_TimImage(char *path);
extern ChildObj86ED0 *New_ScreenSprite(ChildObj86ED0 *arg0, void *arg1, s32 arg2);

extern char *gCardIconNames[];
extern const char gCardPathPrefix[]; /* "CARD\\" */
extern const char gCardPathSuffix[]; /* ".TIM" */
extern s32 gCardIconRect; /* 3-word opaque block, New_ScreenSprite's arg1, address-only here */
extern s32 gCardIconPos; /* opaque block, the fresh unk70's own slot4C arg2, address-only here */

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

    handle = New_TimImage(buf);
    handle->methods->slot78(handle);
    newVal = New_ScreenSprite(handle, (void *)&gCardIconRect, 0);
    self->unk70 = newVal;
    handle->methods->release(handle);
    newVal->methods->slot4C(newVal, self->unk68, (void *)&gCardIconPos);
}
```

## Deriving the shape

Three early-return guards (`arg1 >= 0x11`, `self->unk68 == 0`,
`self->unk70 != NULL`) all branch to the same epilogue -- textbook
straight-line `if (...) return;` chain, confirmed since the `.s` shows
three independent `beqz`/`bnez` to the identical label.

The path-building block does NOT call the already-matched
`BuildFileName` (`dest[0]=0; if(arg2) strcat(dest,arg2); strcat(dest,arg1);
strcat(dest,arg3);`, `src/GameApplicationFileResource.c`) -- it inlines the same three-strcat
shape directly with no null-guard on the first strcat, which is what rules
out an actual call to that helper (a call would need the conditional
branch). Reading the `strcat` argument order off the delay slots
(`strcat(buf,"CARD\\")`, `strcat(buf,name)`, `strcat(buf,".TIM")`) gives
`buf = "CARD\" + name + ".TIM"`, `name = gCardIconNames[arg1]` -- the same
`CARD\<NAME>.TIM` shape as `TextEntry__LoadCardResources`'s `CARD\COMINPUT.TIM` (that
unit builds it via a real `BuildFileName` call instead; this one just
happens to inline the identical three-piece concatenation).

`gCardIconNames` (`asm/data/76DC8.data.s`) is a flat 17-word (`0x11`, matching
the `arg1 >= 0x11` guard) array; most entries are `D_8008AAxx` rodata
string pointers, a handful are raw non-pointer literal words (`0x100`,
`0x2000`, and three bare `0x800115xx` addresses) that this call site never
reaches for the `arg1` values this function is actually invoked with --
declared as a plain `extern char *gCardIconNames[];`, which is enough to index
without needing the full contents.

The `New_TimImage` / `slot78` / `New_ScreenSprite` / `release` sequence on
the temp `handle` is the exact idiom already established by
`TextEntry__LoadCardResources` (`docs/match-reports/TextEntry__LoadCardResources.md`, a DIFFERENT unit,
`class_3bb8c_i`) using the SAME shared `ChildObj86ED0`/`ChildMethods86ED0`
type from `class_3bb8c.h` (`slot78`/`release` already declared there).

`self->unk70` is ALREADY typed `Class86E00Unk70Obj_3bb8c_g *` in this unit
(established by the already-matched `TaskObjF__ReleaseCardIcon`, same file). Assigning
`New_ScreenSprite`'s `ChildObj86ED0 *` return into it is the same
implicit-pointer-type-mismatch-is-harmless pattern already documented in
`TextEntry__LoadCardResources`'s own report (`self->unk44 = New_TextRow(...)` there) --
a warning, not an error, zero byte cost. Extended
`Class86E00Unk70ObjMethods_3bb8c_g` (in `include/class_3bb8c.h`)
ADDITIVELY with `slot4C` at `+0x04C`
(`void (*slot4C)(Class86E00Unk70Obj_3bb8c_g *self, s32 arg1, void *arg2)`),
padding `+0x008` through `+0x04C` -- the call's own evidence types `arg1`
as `self->unk68` forwarded verbatim (that field is already the owning
struct's established bare `s32`, per its own header comment: "never
dereferenced in this unit") and `arg2` as an address-only opaque block
(`gCardIconPos`).

## The one real residue: buffer/handle register reuse and a discarded call result

First working version (semantically identical, `path` used directly with
no `buf`/`name` locals, and `self->unk70 = New_ScreenSprite(...)` assigned
straight into the struct field with no intermediate local) scored 62/71:
retail keeps the temp-buffer address AND the `New_TimImage` handle in the
SAME callee-saved register across all four calls that need it (computed
`addiu $s0,$sp,0x10` once, reused via `move a0,s0` for every `strcat` and
the `New_TimImage` call, then reassigned `s0=v0` to hold the handle for
the rest of the function) -- passing `path` as a bare array argument at
each call site instead let the compiler recompute `addiu $a0,$sp,0x10`
fresh every time rather than keeping one persistent register. Introducing
`buf = path;` and always calling through `buf` fixed this half.

The remaining 9-word gap: retail keeps `New_ScreenSprite`'s return value in a
register (`s1`) both for the `self->unk70 = ...` store AND for every
subsequent use (`handle->methods->release(handle)`'s own... no, actually
for the following `newVal->methods->slot4C(newVal, ...)` call's `a0`) --
assigning the call's result directly into `self->unk70` and then reading
`self->unk70` back out of memory for the final `slot4C` call cost an extra
`lw` where retail has a register move. Introducing the `newVal` local
(holding the call result, assigned into `self->unk70`, and reused for the
final call) closed this to byte-exact.

### Proposed learning

Same shape as `TextEntry__LoadCardResources`'s already-documented lesson, but for a
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

## Track 4

2026-09-25, round 84 (charlie): The class `New_D8006ED4C` constructs is unified as ScreenSprite in `include/ScreenSprite.h`; the unit includes it and its local extern (which typed the return `ChildObj86ED0 *`) is gone. The call reads `newVal = (Class86E00Unk70Obj_3bb8c_g *)New_ScreenSprite(handle, (SpriteRect *)&gCardIconRect, 0)`: gCardIconRect is the rect (words 0, 160, 120). The unit's baseline "assignment from incompatible pointer type" was this line and is gone. `cardIcon`'s type, Class86E00Unk70Obj_3bb8c_g, is Class86E00's view of a ScreenSprite (release at +0x004, attachToParent at +0x04C with the screen position gCardIconPos = (-70, -60)); it is Class86E00's field and is left as it is. Image byte-identical.

## Track 4 (2026-09-26, round 88)

TimImage is unified (`include/TimImage.h`); this unit's local `extern
ChildObj86ED0 *New_TimImage(char *)` is deleted. `handle` is `TimImage *`:
+0x078 (FileResource's `void *slot78`, occupant TimImage__Upload) is called
through `TimImageUploadFn`, +0x004 is TimImage's inherited `release`.
Image byte-identical.

## Track 7 (2026-09-27, round 95)

`D_80086EC4` -> `gCardIconRect` (declared `SpriteRect`, its three words
{0, 0, 160, 120}; the `(SpriteRect *)` cast went) and `D_8008AA94` ->
`gCardIconPos` (declared `ScreenSpritePos`, (-70, -60)), both tier A, by what
they hold (tools/rename.py). Locals: `arg1` -> `index`, the array `path` ->
`pathBuf` (size written 32), `buf` -> `path`, `handle` -> `tim`, `newVal`
-> `icon`. The guard `index >= 0x11` is `ARRAY_COUNT(gCardIconNames)`, the
extern now sized `[TASKOBJF_STATE_EDIT_TITLE]` (one name per message
state). The residue section above keeps one line: `MATCHING: path and icon
keep the buffer and the sprite in saved registers`.

Comments moved out of the source (verbatim):

- on `gCardIconNames`: "0x11 (17) entries, indexed by `arg1`
  (range-checked `< 0x11` below); mostly `char *` string pointers into
  rodata, a few raw literal words at indices never reached from this call
  site. `asm/data/76DC8.data.s`." Entries 0 and 1 are the words 0x100 and
  0x2000; 2, 9 and 13 point into rodata (0x80011524, 0x80011518,
  0x8001150C), the rest into sdata.
- on `gCardIconRect`: "3 words, `New_ScreenSprite`'s rect: a SpriteRect
  {0, 0, 160, 120}."
- on `gCardIconPos`: "opaque block, the fresh `cardIcon`'s own `slot4C`
  arg2, address-only here." (slot +0x04C is attachToParent; ScreenSprite's
  override hands it to setPosition.)
