# SetStyleEffectSources -- MATCHED, round 44 (2026-09-15)

> Renamed from `Actor__func_56f5c` on 2026-09-26 (tools/rename.py). Address 0x80056f5c.

> Renamed from `BaseObjO__func_56f5c` on 2026-09-25 (tools/rename.py). Address 0x80056f5c.

> Renamed from `func_80056F5C` on 2026-09-18 (tools/rename.py). Address 0x80056f5c.

Unit `ObjMStyleActor`. **34/34 words, byte-exact, first build.** Reopened,
never attempted before this round.

## What it does

Stashes `self` (arg1) and two more scalar args (arg2, arg3) into `.sbss`
globals `gStyleEffectTmd`/`sStyleEffectTim`/`gStyleEffectViewport`, then runs a fixed 2-iteration
loop calling `self->methods->slot80(self, sStyleEffectModelIds[i])` and feeding the
result plus `&sStyleEffectClutPos` (a 1-word `.sdata` constant, address-only, never
loaded) to library function `TmdModel__SetFirstPrimClut` (still `psyq_fa50.s`, unrenamed
Psy-Q object; called with an unused return value). `arg0` (the function's
first parameter) is never read anywhere in the body -- it is discarded, the
same as its caller (`StyleBuildEffectSlots`, unaddressed `ObjMStyleActor.s`) passes
its own unrelated `self` there without any indication of shared meaning.

```c
extern s32 gStyleEffectTmd;
extern s32 sStyleEffectTim;
extern s32 gStyleEffectViewport;
extern s32 sStyleEffectModelIds[3];
extern s32 sStyleEffectClutPos;

extern void TmdModel__SetFirstPrimClut(void *arg0, void *arg1);

void SetStyleEffectSources(s32 arg0, BaseObjO *self, s32 arg2, s32 arg3) {
    s32 i;
    void *ret;

    gStyleEffectTmd = (s32) self;
    sStyleEffectTim = arg2;
    gStyleEffectViewport = arg3;
    i = 0;
    do {
        ret = self->methods->slot80(self, sStyleEffectModelIds[i]);
        TmdModel__SetFirstPrimClut(ret, &sStyleEffectClutPos);
        i++;
    } while (i < 2);
}
```

## New vtable slot

`tools/classtable.py gStyleEffectMethods` shows `+0x080` occupied by
`SceneNode__SetBackClip` (a `BasicClass`-range function outside this unit, same
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

**`SetStyleEffectSources` -- tier C.** Class is known (`self` is `BaseObjO`,
confirmed by the `self->methods->slot80` dispatch), and the mechanics are
fully described in this report (stash `self`/two scalars into three
globals, then loop twice through the still-unresolved `slot80` occupant
and an unrenamed Psy-Q object, `TmdModel__SetFirstPrimClut`), but nothing establishes
WHAT this accomplishes -- `arg0` is discarded by every known caller, and
`ObjMStyleActor.c`'s own comment calls it merely "ctor-shaped" as a guess,
not a finding. Kept the tier-C `Class__func_xxxxx` form.

## Track 4 (2026-09-25, round 82, delta)

Renamed from `BaseObjO__func_56f5c`. Prefix only: the second parameter is an Actor (it calls getSetUnk10Flag8, +0x080, through its table; the s32 result is cast to the void * TmdModel__SetFirstPrimClut takes). Still tier C. The class (id 0x34, table `gActorMethods`, formerly `D_800878D4`) is unified as `Actor` in `include/Actor.h`: a SceneNode subclass and the base of TodActor/Entity, DreamSys and StyleEffect. Any source block above is the pre-unification spelling; the live body in `src/world/ObjMStyleActor.c` takes the unified types and field/slot names, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 4b (2026-09-25, round 85)

`gStyleEffectTmd`/`sStyleEffectTim`/`gStyleEffectViewport` were `s32` in ObjMStyleActor.c and
`D_8008ACA4Obj *`/`void *`/`void *` in ObjMStyleActor.c. Both units now
declare `Actor *`/`void *`/`void *`: this function stores its own `Actor *self` into the first, the two scalars go in through a `(void *)` cast. Byte-identical; no new `-Wall`
warning.

## Track 6 (round 93, bravo)

`Actor__func_56f5c` -> `SetStyleEffectSources` (`python3 tools/rename.py
Actor__func_56f5c SetStyleEffectSources`). **Tier B.** It is not an Actor
method: its one caller, StyleBuildEffectSlots, passes the variant (unused),
ObjM's DREAMER.TMD resource (a LinkResource, cast to Actor), ETC.TIM and the
viewport, and it only stores them for StyleEffect's methods and prepares
the two models the TMD's +0x080 slot (LinkResource's getModel) returns for
sStyleEffectModelIds[0..1] (TmdModel__SetFirstPrimClut with sStyleEffectClutPos). The stored
globals were renamed with it (one rename.py run each): D_8008ACA4 ->
gStyleEffectTmd, D_8008ACA8 -> sStyleEffectTim, D_8008ACAC ->
gStyleEffectViewport, D_8008AB98 -> sStyleEffectModelIds. PROPOSED (not
applied, a prototype and body outside the header): its `Actor *self`
parameter and the `Actor *` view of gStyleEffectTmd in ObjMStyleActor.c/_s.c
are really `LinkResource *`, and the `setBackClip` calls through it are
`getModel`; its prototype belongs in include/StyleEffect.h, not Actor.h.

## Track 7 (round 99, alpha)

Parameters named (unused, tmd, tim, viewport), in the prototype in include/Actor.h too; the local `ret` is `TmdModel *model`, and the local prototype of TmdModel__SetFirstPrimClut takes its definition's (TmdModel *, s16 *xy). `python3 tools/rename.py D_8008AB94 sStyleEffectClutPos`, **tier A**: an s16[2] VRAM position, 0x01FF03F0 = (x 1008, y 511), which TmdModel__SetFirstPrimClut turns into a CLUT id (x / 16 + y * 64) for the first primitive of each model; this function is its only accessor. The comment on the three source globals, which said "track 4b, round 85", is covered by the Track 4b section above; it now says what they are. Still PROPOSED from track 6 (bravo): `tmd` and gStyleEffectTmd are a LinkResource (the +0x080 slot is LinkResource's getModel, returning a TmdModel), which would drop the Actor view and the cast; that changes ObjMStyleActor.c's declaration too.
