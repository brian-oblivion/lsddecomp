# Actor__OnActorLinkCommand -- MATCHED (33/33)

> Renamed from `DreamSys__DispatchLinkCommandAndTryAttach` on 2026-09-25 (tools/rename.py). Address 0x80057b90.

> Renamed from `func_80057B90` on 2026-09-19 (tools/rename.py). Address 0x80057b90.

Unit: `src/class_3bb8c_p.c`. Class: `DreamSys`, own vtable slot `+0x0DC`
(base-class-inherited at this offset; `include/DreamSys.h`'s
`DreamSysBaseMethods::slot0xDC` already carried a comment naming this
exact function as its `+0xDC` resolution, from before this unit converted
it).

## Signature

```c
void Actor__OnActorLinkCommand(DreamSys *self, void *arg1, s32 count);
```

## Body

```c
void Actor__OnActorLinkCommand(DreamSys *self, void *arg1, s32 count) {
    GetSceneNodeMethods()->dispatchLinkCommand(self, arg1, count);
    if (count < 9) {
        if (count >= 5) {
            self->vt->slotA0(self, arg1, count);
        }
    }
}
```

Two calls with the SAME (self, arg1, count) shape but through DIFFERENT
tables:
- The first, unconditional, dispatches through
  `GetSceneNodeMethods()`'s return -- the shared base-class table at
  `gSceneNodeMethods` (already established with the project's "per-call-site
  signature" precedent in `include/SceneNode.h`) -- at its `+0x09C` slot.
  This unit's own local view (`SceneNodeBaseTable`, declared in this file)
  types only that one slot.
- The second, conditional on `5 <= count < 9`, dispatches through
  `self`'s OWN vtable (`self->vt->slotA0`, `include/DreamSys.h`) at
  `+0x0A0` -- ordinary polymorphic dispatch, resolves to `SceneNode__TryAttachNearby`
  currently (not overridden at the `DreamSys` level, per
  `tools/classtable.py gDreamSysMethods`), but written as a real vtable
  call rather than a fixed symbol.

## Shape note: the range check must be nested `if`s, not `&&`

`if (count >= 5 && count < 9)` compiles to a strength-reduced unsigned
range check (`addiu v0,count,-5; sltiu v0,v0,4`) -- one instruction
shorter than retail's two separate `slti`/`slti` comparisons, and it also
calls `GetSceneNodeMethods()` a SECOND time for the polymorphic dispatch
instead of reading `self->vt` directly (an artifact of how the code was
written when this was mis-attributed to the shared table, not a language
issue -- see below). Nesting as `if (count < 9) { if (count >= 5) ... }`
reproduces retail's two-`slti` shape exactly.

## What was mis-diagnosed first

The first attempt routed BOTH calls through `GetSceneNodeMethods()`'s table,
assuming the second call was just another `+0xA0` slot on the same shared
base object. That scored 15/33 with the tail completely displaced by one
word. Reading the disassembly closely: the second call's `lw v0,0(a0)`
loads from `a0` == `s1` == `self` (not from `GetSceneNodeMethods()`'s return,
which was never re-fetched) -- i.e. it is `self->vt->slotA0`, a genuinely
different dispatch mechanism from the first call, not a repeat of it.

## Naming

**`Actor__OnActorLinkCommand` -- tier B.** Mechanics fully
confirmed (33/33): unconditionally forwards through the shared
`SceneNodeBaseTable::dispatchLinkCommand` slot, then -- only for
`5 <= count < 9` -- ALSO dispatches through the object's own inherited
`vt->slotA0` (resolves to `SceneNode__TryAttachNearby` via
`tools/classtable.py gSceneNodeMethods`, confirmed this round). The name states
both calls and their conditional relationship; why `[5,9)` specifically
gates the attach attempt is not established.

## Proposed field names

`vtable_DreamSys::slotA0` is accessed from SEVEN other units too
(`grep -rln -- '->slotA0\b' src/` lists `class_3bb8c_g.c`,
`class_3bb8c_l.c`, `class_3bb8c_c.c`, `SceneNode.c`, `Task.c`,
`class_3bb8c_i.c`, `Task.c`, besides this unit), so per
FINISHING-PLAN.md track 3 step 3 it is proposed here, not renamed, and
posted to the broadcast for the head to apply at merge.

- **`vtable_DreamSys::slotA0` -> `tryAttachNearby`** (tier B). Evidence:
  `tools/classtable.py gSceneNodeMethods` resolves the SAME offset (`+0x0A0`) in
  the fixed base table `GetSceneNodeMethods()` returns to
  `SceneNode__TryAttachNearby`, and this function's own report already
  established the slot is unoverridden at the `DreamSys` level (still
  resolves to that same function) -- matching this project's convention
  of naming a resolved base-class slot after the method it dispatches to
  (e.g. `Actor__AddTranslation`, `LinkWall` in the same struct). Not tier A:
  the other seven call sites' own purpose for calling it is not reviewed
  here, only this unit's own.

## Verify

```
./build-and-verify.sh   # build exit=0, OK: build matches retail
tools/funcdiff.py Actor__OnActorLinkCommand   # 33/33
```

### Proposed learning

**Two calls with the identical argument shape are not necessarily the
same dispatch mechanism.** Check the register the vtable pointer is
LOADED FROM at each call site, not just the slot offset and argument
list -- one may go through a shared/base table fetched via a getter
function, and a superficially identical sibling call may instead go
through the object's OWN `self->vt`. They can resolve to the same function
today (inherited, unoverridden) while being byte-different call shapes.

## Track 4 (2026-09-25, round 82, delta)

Renamed from `DreamSys__DispatchLinkCommandAndTryAttach`. Occupant of +0x0DC, which Actor__DispatchLinkCommand (+0x09C) calls when the SENDER's class byte is 0x34 (an Actor); DreamSys overrides it as DreamSys__DispatchInstanceEffect (testing for an Entity, 0x1F234) and Entity as Entity__NotifyLinkStage. Body: chain SceneNode's dispatchLinkCommand, then tryAttachNearby for events 5..8 -- called through a function-pointer cast with (self, sender, event), since SceneNode's slot declares self alone and both arguments are reloaded after the base call. The class (id 0x34, table `gActorMethods`, formerly `D_800878D4`) is unified as `Actor` in `include/Actor.h`: a SceneNode subclass and the base of TodActor/Entity, DreamSys and StyleEffect. Any source block above is the pre-unification spelling; the live body in `src/class_3bb8c_p.c` takes the unified types and field/slot names, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 7 (2026-09-27, round 96, bravo)

Comments quoted below are verbatim as the file stood before this round's
comment pass, i.e. with this round's renames already applied (the
`LinkQueryBuf` one as it stood before step 2).

- Re-measured this round: `if (event >= 5 && event < 9)` and `if (event <
  9 && event >= 5)` both break the image; the nested pair stays, with one
  `/* MATCHING: */` line.

The function comment, verbatim:

```c
/* tryAttachNearby is called with (self, sender, event): SceneNode's slot
 * declares self alone (its occupant's second parameter arrives in the
 * caller's untouched $a1), and here both are reloaded after the base call,
 * so the call spells them out through a cast. */
```
