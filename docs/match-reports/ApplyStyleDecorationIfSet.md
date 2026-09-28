# ApplyStyleDecorationIfSet -- MATCHED, round 44 (2026-09-15)

> Renamed from `func_80054660` on 2026-09-23 (tools/rename.py). Address 0x80054660.

Unit `ObjMStyleActor`. **45/45 words, byte-exact.** Reopened, never attempted
before this round.

## What it does

Takes no arguments; gated entirely on the global `sStyleDecorColor` (set by
`ApplyStyleConfig`, matched earlier this round). If it's non-NULL: builds an
object via `New_BoxFill(&sStyleDecorBoxSize, sStyleDecorColor, 0)` (already known
elsewhere as returning `ClassEAC0Obj *` from `include/Task.h`, a header
this unit doesn't own -- see below), stashes it in `sStyleDecorObj`, and
dispatches three method calls on it (`slot64(obj,1)`, `slot68(obj,0)`,
`slot4C(obj,tmp,&sStyleDecorBoxPos)`) plus one call on a completely different
object reached through `sStyleSceneRefs->unkC` (`slotAC(sub)`, whose return
feeds the `slot4C` call's middle argument).

```c
/* Local view only -- New_BoxFill already returns `ClassEAC0Obj *` per
 * include/Task.h, a header owned by a different unit. This function
 * only ever reaches slots 0x4C/0x64/0x68, so it gets its own minimal local
 * type instead of pulling that header in. */
typedef struct LocalM4D0Obj LocalM4D0Obj;
typedef struct LocalM4D0Methods LocalM4D0Methods;
struct LocalM4D0Methods {
    u8 pad00[0x4C];
    void (*slot4C)(LocalM4D0Obj *self, s32 arg1, void *arg2); /* +0x04C */
    u8 pad50[0x64 - 0x50];
    void (*slot64)(LocalM4D0Obj *self, s32 arg1);              /* +0x064 */
    void (*slot68)(LocalM4D0Obj *self, s32 arg1);              /* +0x068 */
};
struct LocalM4D0Obj {
    LocalM4D0Methods *methods;
};

typedef struct LocalSubObj LocalSubObj;
typedef struct LocalSubMethods LocalSubMethods;
struct LocalSubMethods {
    u8 pad00[0xAC];
    s32 (*slotAC)(LocalSubObj *self);                          /* +0x0AC */
};
struct LocalSubObj {
    LocalSubMethods *methods;
};
typedef struct FieldAC7CHolder {
    u8 pad0[0xC];
    LocalSubObj *unkC;
} FieldAC7CHolder;

extern s32 sStyleDecorObj;
extern s32 sStyleDecorBoxSize;
extern s32 sStyleDecorBoxPos;
extern LocalM4D0Obj *New_BoxFill(void *a0, void *a1, s32 a2);

void ApplyStyleDecorationIfSet(void) {
    s32 tmp;

    if (sStyleDecorColor != 0) {
        sStyleDecorObj = (s32) New_BoxFill(&sStyleDecorBoxSize, (void *) sStyleDecorColor, 0);
        ((LocalM4D0Obj *) sStyleDecorObj)->methods->slot64((LocalM4D0Obj *) sStyleDecorObj, 1);
        ((LocalM4D0Obj *) sStyleDecorObj)->methods->slot68((LocalM4D0Obj *) sStyleDecorObj, 0);

        tmp = ((FieldAC7CHolder *) sStyleSceneRefs)->unkC->methods->slotAC(
                ((FieldAC7CHolder *) sStyleSceneRefs)->unkC);

        ((LocalM4D0Obj *) sStyleDecorObj)->methods->slot4C((LocalM4D0Obj *) sStyleDecorObj, tmp, &sStyleDecorBoxPos);
    }
}
```

## The real lever: read through the GLOBAL at each use, not a local pseudo-variable

First attempt used the obvious local-variable idiom:

```c
LocalM4D0Obj *obj = New_BoxFill(...);
sStyleDecorObj = (s32) obj;
obj->methods->slot64(obj, 1);
```

This built ONE WORD LONGER than retail every time, regardless of statement
order (tried: store-before-call, store-after-call, hoisting the method
pointer into its own `LocalM4D0Methods *`/function-pointer variable first --
all four variants produced the identical extra instruction). The tell:
retail's first load off the freshly-returned object (`lw v1,0(v0)`, reading
`obj->methods` straight off the raw return register `$v0`) happens BEFORE
`move a0,v0` (the copy that puts `obj` where the call needs it as arg1) --
and that `move` lands in the FIRST load's delay slot, for free. Every
local-variable version instead emitted `move a0,v0` FIRST, then dereferenced
through `a0`, leaving the first load's delay slot with nothing to fill but
an explicit `nop`. Moving `sStyleDecorObj`'s assignment or the method lookup
earlier/later in the C never changed which register got promoted first --
GCC 2.6.3's allocator had already picked `a0` as `obj`'s home the moment a
named local variable existed for it, independent of source statement order.

**The fix was to never name it.** Storing the call's return value straight
into the global (`sStyleDecorObj = (s32) New_BoxFill(...);`) and then
re-deriving the pointer from `sStyleDecorObj` at every subsequent use point
(`((LocalM4D0Obj *) sStyleDecorObj)->methods->...`) let the compiler's local
value-numbering recognize that the gp-relative load it would otherwise need
for the FIRST use is redundant right after the store (the value is still in
`$v0`), so it read `$v0` directly there and only introduced the `a0` copy
where an argument register was actually needed -- exactly retail's schedule,
and byte-exact on the next build.

### Proposed learning

A near-miss that is exactly one word LONG, where the only visible
difference is an extra `move $an,$vN` positioned BEFORE a load instead of
filling its delay slot (with a corresponding stray `nop` appearing later),
is a register-promotion artifact of giving a value returned from a call ITS
OWN NAMED LOCAL VARIABLE, not a missing/extra statement. When the value
is about to be stored into a global anyway and reused only a few
instructions later, storing it and re-reading through the global at each
use point (instead of holding it in a local) can recover the exact
schedule GCC 2.6.3 -O2 produced, because it lets the compiler's own local
CSE decide when the raw call-return register is still cheaper to reuse than
copying it to the argument register early. This is the same family as
round 44's other two levers this session (guard-clause direction in
`RegisterStyleConfig`, join-point count in `ApplyStyleConfig`) -- all three are
cases where semantically-identical C phrasings hand GCC 2.6.3's allocator
and scheduler different amounts of freedom, and the fix was never new
logic, only a different way of naming the same values.

## Naming

**ApplyStyleDecorationIfSet** -- tier B. Gated entirely on `sStyleDecorColor` (set by `ApplyStyleConfig`'s colour-table branch): if non-NULL, builds a `ClassEAC0Obj` via the already-known `New_BoxFill`, configures it (`slot64`/`slot68`), pulls a value from an unrelated holder object (`sStyleSceneRefs`'s `unkC`), and feeds both into `slot4C`. Mechanically described; what the conditional decoration represents is not established, hence tier B rather than a guessed "spawn X" name.

## Track 4 (2026-09-25, round 85, charlie)

sStyleDecorObj is a BoxFill (include/BoxFill.h); the deleted `LocalM4D0Obj` view's slots are setSemiTrans (+0x064, 1), setSemiTransRate (+0x068, 0) and attachToParent (+0x04C, cast to BoxFillAttachToParentFn). Zero bytes.

## Track 6 (2026-09-27, round 96, charlie)

`FieldAC7CHolder`, `LocalSubObj` and `LocalSubMethods` are retired; the
function now reads `((StyleSceneRefs *)sStyleSceneRefs)->viewport->methods->getFadeBox(...)`
into a `SceneNode *fadeBox` and passes that to attachToParent without a cast.
Zero bytes (whole-image SHA1 green, 0 new typeview warnings).

Evidence: `sStyleSceneRefs` (0x8008AC7C, an `s32` in every unit) is
RegisterStyleConfig's third argument, which ObjM__InitStyleAndWorld
(ObjMStyleActor) passes as `&self->ctorSound`: it points at ObjM's
+0x06C..+0x07B block (include/ObjM.h's banner). The holder's +0x00C is
therefore ObjM::cachedViewport, a NodeGuardedViewport, and +0x0AC of its
table is Viewport's `getFadeBox` (include/Viewport.h, `SceneNode *(*)(Self *)`)
-- ObjM.h's banner already said so. ObjMStyleActor.c had the same block as
`StyleSceneRefs {sound, dreamerTmd, etcTim, Viewport *viewport}`; this unit
now carries the identical view (`typeviews.py --merge StyleSceneRefs`: 2
views, 0x10, 0 conflicts). Tier A for the type (it names what the pointer
is, established from the one writer); the name is ObjMStyleActor's, not new.

Not applied (outside the edit set): hoisting `StyleSceneRefs` into one
shared header (ObjM.h, beside the block it views) and dropping both unit
copies; retyping the `sStyleSceneRefs` global from `s32` to
`StyleSceneRefs *` (track 4b, it would remove every cast in _m and _n).

## Track 7 (2026-09-27, round 98, delta)

| old | new | tier | evidence |
| --- | --- | --- | --- |
| `D_8008AB58` | `sStyleDecorBoxPos` | A | (-100, -100), the box's attachToParent position; now declared `BoxFillPos`, no cast (Viewport places its own fade box there) |
| `D_8008AB60` | `sStyleDecorBoxSize` | A | (320, 240), New_BoxFill's size pair; now `s32[2]` |

The colour goes in as `(BoxFillRgb *)sStyleDecorColor` (BoxFill.h's
record, this round) instead of `(void *)`. Zero bytes. The comment "(track
4b's to retype)" on sStyleDecorObj is gone; the declaration says it holds
a BoxFill *.

## Track 10 (2026-09-28, round 104, echo)

The six per-class aliases of `ColorRgb` (include/draw_system.h) -- BgLayerRgb, BoxFillRgb, FlatLightColor, LightRigRgb, ViewportRgb, TimBlockSrcColor -- are deleted and every use is spelled `ColorRgb`. Byte-identical. Measured for the MATCHING line in TaskCore__SetColors: the whole-struct copy is three `lb` then three `sb`, and rewriting one of the copies byte by byte loads each byte with `lbu` and interleaves the stores (asm-differ on the experiment), so the struct copy stays; the old line's "signed bytes" was wrong (ColorRgb's channels are u8; the lb comes from the block copy, not the type), and the same claim in GraphRoom__BuildGraphPoints' colour comment is corrected.
