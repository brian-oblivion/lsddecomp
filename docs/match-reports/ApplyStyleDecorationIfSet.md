# ApplyStyleDecorationIfSet -- MATCHED, round 44 (2026-09-15)

> Renamed from `func_80054660` on 2026-09-23 (tools/rename.py). Address 0x80054660.

Unit `class_3bb8c_m`. **45/45 words, byte-exact.** Reopened, never attempted
before this round.

## What it does

Takes no arguments; gated entirely on the global `D_8008AB54` (set by
`ApplyStyleConfig`, matched earlier this round). If it's non-NULL: builds an
object via `New_ClassEAC0(&D_8008AB60, D_8008AB54, 0)` (already known
elsewhere as returning `ClassEAC0Obj *` from `include/code_2cc8c.h`, a header
this unit doesn't own -- see below), stashes it in `D_8008AC94`, and
dispatches three method calls on it (`slot64(obj,1)`, `slot68(obj,0)`,
`slot4C(obj,tmp,&D_8008AB58)`) plus one call on a completely different
object reached through `D_8008AC7C->unkC` (`slotAC(sub)`, whose return
feeds the `slot4C` call's middle argument).

```c
/* Local view only -- New_ClassEAC0 already returns `ClassEAC0Obj *` per
 * include/code_2cc8c.h, a header owned by a different unit. This function
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

extern s32 D_8008AC94;
extern s32 D_8008AB60;
extern s32 D_8008AB58;
extern LocalM4D0Obj *New_ClassEAC0(void *a0, void *a1, s32 a2);

void ApplyStyleDecorationIfSet(void) {
    s32 tmp;

    if (D_8008AB54 != 0) {
        D_8008AC94 = (s32) New_ClassEAC0(&D_8008AB60, (void *) D_8008AB54, 0);
        ((LocalM4D0Obj *) D_8008AC94)->methods->slot64((LocalM4D0Obj *) D_8008AC94, 1);
        ((LocalM4D0Obj *) D_8008AC94)->methods->slot68((LocalM4D0Obj *) D_8008AC94, 0);

        tmp = ((FieldAC7CHolder *) D_8008AC7C)->unkC->methods->slotAC(
                ((FieldAC7CHolder *) D_8008AC7C)->unkC);

        ((LocalM4D0Obj *) D_8008AC94)->methods->slot4C((LocalM4D0Obj *) D_8008AC94, tmp, &D_8008AB58);
    }
}
```

## The real lever: read through the GLOBAL at each use, not a local pseudo-variable

First attempt used the obvious local-variable idiom:

```c
LocalM4D0Obj *obj = New_ClassEAC0(...);
D_8008AC94 = (s32) obj;
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
an explicit `nop`. Moving `D_8008AC94`'s assignment or the method lookup
earlier/later in the C never changed which register got promoted first --
GCC 2.6.3's allocator had already picked `a0` as `obj`'s home the moment a
named local variable existed for it, independent of source statement order.

**The fix was to never name it.** Storing the call's return value straight
into the global (`D_8008AC94 = (s32) New_ClassEAC0(...);`) and then
re-deriving the pointer from `D_8008AC94` at every subsequent use point
(`((LocalM4D0Obj *) D_8008AC94)->methods->...`) let the compiler's local
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
