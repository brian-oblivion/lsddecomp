# Actor__func_56f5c -- MATCHED, round 44 (2026-09-15)

> Renamed from `BaseObjO__func_56f5c` on 2026-09-25 (tools/rename.py). Address 0x80056f5c.

> Renamed from `func_80056F5C` on 2026-09-18 (tools/rename.py). Address 0x80056f5c.

Unit `class_3bb8c_o`. **34/34 words, byte-exact, first build.** Reopened,
never attempted before this round.

## What it does

Stashes `self` (arg1) and two more scalar args (arg2, arg3) into `.sbss`
globals `D_8008ACA4`/`D_8008ACA8`/`D_8008ACAC`, then runs a fixed 2-iteration
loop calling `self->methods->slot80(self, D_8008AB98[i])` and feeding the
result plus `&D_8008AB94` (a 1-word `.sdata` constant, address-only, never
loaded) to library function `SetTargetOffset` (still `psyq_fa50.s`, unrenamed
Psy-Q object; called with an unused return value). `arg0` (the function's
first parameter) is never read anywhere in the body -- it is discarded, the
same as its caller (`StyleBuildEffectSlots`, unaddressed `class_3bb8c_n.s`) passes
its own unrelated `self` there without any indication of shared meaning.

```c
extern s32 D_8008ACA4;
extern s32 D_8008ACA8;
extern s32 D_8008ACAC;
extern s32 D_8008AB98[3];
extern s32 D_8008AB94;

extern void SetTargetOffset(void *arg0, void *arg1);

void Actor__func_56f5c(s32 arg0, BaseObjO *self, s32 arg2, s32 arg3) {
    s32 i;
    void *ret;

    D_8008ACA4 = (s32) self;
    D_8008ACA8 = arg2;
    D_8008ACAC = arg3;
    i = 0;
    do {
        ret = self->methods->slot80(self, D_8008AB98[i]);
        SetTargetOffset(ret, &D_8008AB94);
        i++;
    } while (i < 2);
}
```

## New vtable slot

`tools/classtable.py gClass876FCMethods` shows `+0x080` occupied by
`SceneNode__GetSetUnk10Flag8` (a `BasicClass`-range function outside this unit, same
inherited-base pattern as the sibling occupied slots already documented on
`BaseObjOMethods`). That offset previously sat inside this unit's own
`pad44[0x8C - 0x44]` gap. Split it additively into
`pad44[0x80 - 0x44]` + `void *(*slot80)(BaseObjO *self, s32 arg1);` +
`pad84[0x8C - 0x84]` -- same total byte span, so no other already-matched
function's field offsets moved. `BaseObjOMethods`/`BaseObjO` are local to
this `.c` file (not `class_3bb8c.h`), so this edit cannot collide with a
sibling unit.

### Proposed learning

When a struct pad region covers an offset a new function actually calls
through, split the pad rather than guessing a name from nothing --
`tools/classtable.py <table>` names the real occupant function at that
offset even when this unit never defines it, which is enough to type the
slot (return/argument shape) without derivation risk.

## Naming

**`Actor__func_56f5c` -- tier C.** Class is known (`self` is `BaseObjO`,
confirmed by the `self->methods->slot80` dispatch), and the mechanics are
fully described in this report (stash `self`/two scalars into three
globals, then loop twice through the still-unresolved `slot80` occupant
and an unrenamed Psy-Q object, `SetTargetOffset`), but nothing establishes
WHAT this accomplishes -- `arg0` is discarded by every known caller, and
`class_3bb8c_s.c`'s own comment calls it merely "ctor-shaped" as a guess,
not a finding. Kept the tier-C `Class__func_xxxxx` form.

## Track 4 (2026-09-25, round 82, delta)

Renamed from `BaseObjO__func_56f5c`. Prefix only: the second parameter is an Actor (it calls getSetUnk10Flag8, +0x080, through its table; the s32 result is cast to the void * SetTargetOffset takes). Still tier C. The class (id 0x34, table `gActorMethods`, formerly `D_800878D4`) is unified as `Actor` in `include/Actor.h`: a SceneNode subclass and the base of Class65650/Entity, DreamSys and Class876FC. Any source block above is the pre-unification spelling; the live body in `src/class_3bb8c_o.c` takes the unified types and field/slot names, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 4b (2026-09-25, round 85)

`D_8008ACA4`/`D_8008ACA8`/`D_8008ACAC` were `s32` in class_3bb8c_o.c and
`D_8008ACA4Obj *`/`void *`/`void *` in class_3bb8c_s.c. Both units now
declare `Actor *`/`void *`/`void *`: this function stores its own `Actor *self` into the first, the two scalars go in through a `(void *)` cast. Byte-identical; no new `-Wall`
warning.
