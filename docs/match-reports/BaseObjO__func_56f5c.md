# BaseObjO__func_56f5c -- MATCHED, round 44 (2026-09-15)

> Renamed from `func_80056F5C` on 2026-09-18 (tools/rename.py). Address 0x80056f5c.

Unit `class_3bb8c_o`. **34/34 words, byte-exact, first build.** Reopened,
never attempted before this round.

## What it does

Stashes `self` (arg1) and two more scalar args (arg2, arg3) into `.sbss`
globals `D_8008ACA4`/`D_8008ACA8`/`D_8008ACAC`, then runs a fixed 2-iteration
loop calling `self->methods->slot80(self, D_8008AB98[i])` and feeding the
result plus `&D_8008AB94` (a 1-word `.sdata` constant, address-only, never
loaded) to library function `func_80020510` (still `psyq_fa50.s`, unrenamed
Psy-Q object; called with an unused return value). `arg0` (the function's
first parameter) is never read anywhere in the body -- it is discarded, the
same as its caller (`func_80054B84`, unaddressed `class_3bb8c_n.s`) passes
its own unrelated `self` there without any indication of shared meaning.

```c
extern s32 D_8008ACA4;
extern s32 D_8008ACA8;
extern s32 D_8008ACAC;
extern s32 D_8008AB98[3];
extern s32 D_8008AB94;

extern void func_80020510(void *arg0, void *arg1);

void BaseObjO__func_56f5c(s32 arg0, BaseObjO *self, s32 arg2, s32 arg3) {
    s32 i;
    void *ret;

    D_8008ACA4 = (s32) self;
    D_8008ACA8 = arg2;
    D_8008ACAC = arg3;
    i = 0;
    do {
        ret = self->methods->slot80(self, D_8008AB98[i]);
        func_80020510(ret, &D_8008AB94);
        i++;
    } while (i < 2);
}
```

## New vtable slot

`tools/classtable.py D_800876FC` shows `+0x080` occupied by
`Class6B5CC__GetSetUnk10Flag8` (a `BasicClass`-range function outside this unit, same
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

**`BaseObjO__func_56f5c` -- tier C.** Class is known (`self` is `BaseObjO`,
confirmed by the `self->methods->slot80` dispatch), and the mechanics are
fully described in this report (stash `self`/two scalars into three
globals, then loop twice through the still-unresolved `slot80` occupant
and an unrenamed Psy-Q object, `func_80020510`), but nothing establishes
WHAT this accomplishes -- `arg0` is discarded by every known caller, and
`class_3bb8c_s.c`'s own comment calls it merely "ctor-shaped" as a guess,
not a finding. Kept the tier-C `Class__func_xxxxx` form.
