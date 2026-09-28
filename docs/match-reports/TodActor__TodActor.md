# TodActor__TodActor

> Renamed from `Class65650__Class65650` on 2026-09-26 (tools/rename.py). Address 0x80065650.

> Renamed from `class_65650__Constructor` on 2026-09-24 (tools/rename.py). Address 0x80065650.

**Unit:** TodActor · **Size:** 59 words (0xEC bytes) · **Status:** MATCHED
(59/59 words, whole-image `./build-and-verify.sh` green)

## What it does

The constructor for the class at method table `gTodActorMethods` (see
`src/world/tod_actor.c` for the resolved inheritance:
`BasicClass -> gActorMethods (intermediate, header 0x34) -> this class`).
Signature `(self, arg1, arg2)`, matching `New_TodActor`'s call. Sequence:

1. Call the base class's constructor through its own vtable, slot `+0x008`
   (`GetActorMethods()->ctor(self)`, per `docs/research/class-framework.md`'s
   "base constructor called through the base table's slot +0x008" shape). If
   it returns `NULL`, bail out immediately (no partial teardown needed —
   nothing of this class's own has been touched yet).
2. Install this class's own vtable (`GetTodActorMethods()`, a `Get_vtable`-style
   accessor — **called, not inlined as `&gTodActorMethods` directly**, because
   retail's own bytes are a `jal` to that function, not a `lui`/`addiu`).
3. Field init: `self->arg2 = arg2` (the constructor's own third parameter,
   stashed verbatim at `+0x58`); zero `+0x5C`, `+0x68`, `+0x70`, `+0x94`.
4. Call `self->methods->slot_setup5C(self, arg1)` (`+0x0F4`, still
   `INCLUDE_ASM` this round as `TodActor__SetupModelData`) — forwarding the
   constructor's own `arg1` through. If it returns nonzero (failure), roll
   back: fetch the base table *again* (a second, independent
   `GetActorMethods()` call — retail really does call it twice, not cache the
   first result) and call its dtor slot, then return `NULL`.
5. On success, call `self->methods->slot10(self, self->unk5C)` (`+0x010`,
   inherited from the base, not overridden here) and
   `self->methods->slot40(self)` (`+0x040`, this class's own override,
   `TodActor__Reset`, still `INCLUDE_ASM`), then return `self`.

```c
TodActor *TodActor__TodActor(TodActor *self, void *arg1, void *arg2)
{
    D800878D4Methods *base;

    base = GetActorMethods();
    if (base->ctor(self) == NULL) {
        return NULL;
    }
    self->methods = GetTodActorMethods();
    self->arg2 = arg2;
    self->unk5C = NULL;
    self->unk68 = NULL;
    self->unk70 = NULL;
    self->unk94 = 0;
    if (self->methods->slot_setup5C(self, arg1) != 0) {
        base = GetActorMethods();
        base->dtor(self);
        return NULL;
    }
    self->methods->slot10(self, self->unk5C);
    self->methods->slot40(self);
    return self;
}
```

Matched with no reshaping at all beyond the direct translation above — no
`goto`, no scheduling barrier. Both early-exit branches (`base->ctor` failure,
`slot_setup5C` failure) are plain `if (...) return NULL;`, and both matched
byte-exact on the first successful build.

## Notes on the header

The struct/vtable derivation is in `src/world/tod_actor.c`. Key point for
future work in this unit: `self->methods->ctor`/`slot_setup5C`/etc. are
**vtable slots**, resolved indirectly at runtime; calling into
`TodActor__SetupModelData` (`slot_setup5C`) and `TodActor__Reset` (`slot40`), both still
`INCLUDE_ASM`, required **no forward `extern` prototype for those functions
by name** — the call goes through a typed function-pointer field in
`TodActorMethods`, so only the struct's field type needs to be right, not
a direct declaration of the not-yet-matched function. This is cheaper than
the "calling into a function that is still `INCLUDE_ASM`" pattern in
CLAUDE.md, which applies to *direct* `jal`-by-name calls (like
`GetActorMethods` here), not vtable dispatch.

### Proposed learning

When a constructor/method calls another slot of its OWN class's vtable (not
a base-class slot), you do not need an extern prototype for the not-yet-
matched target function — type the vtable struct's field correctly and let
the indirect call go through `self->methods->slotN(...)`. This sidesteps the
whole "forward declaration for a same-unit INCLUDE_ASM callee" question for
every method-table dispatch, which is most calls in this class framework.

## Naming

Round 75 (charlie), track 3.

- `TodActor__TodActor` (was `class_65650__Constructor`), tier A. Occupies gTodActorMethods +0x008 (`tools/classtable.py gTodActorMethods --vs gActorMethods`: overrides Actor__Actor); chains the base ctor, installs gTodActorMethods, zeroes modelData/mainPart/parts/peer, runs setupModelData, links the model data as a companion, calls initDefaults. `Class__Class` convention; replaces FirecatFG's `class_65650__Constructor`.

### Field and slot names (round 75, tod_actor.c)

Every rename below was made in the struct definition first; the compiler
listed 162 accessors, all in `src/world/tod_actor.c`, and `check-nonmatching.sh`
stayed green, so none needed proposing.

| offset | name | tier | evidence |
| --- | --- | --- | --- |
| +0x0C | parent | A | SceneNode's +0x0C (code_d294.h `parent`); AttachToParent runs while 0, DetachFromParent while set |
| +0x14 | coord2 | A | SceneNode's GsCOORDINATE2; Tick zeroes its first word (flg) |
| +0x24 | tick | B | SceneNode `tick` (tier B there); Tick increments it |
| +0x50 | companion2 | B | BaseObjO `companion2` (tag-5 companion, dream_scene.c) |
| +0x5C | modelData | B | gModelDataMethods instance (header 0x5F03) from New_ModelData or borrowed; supplies part ids, TOD packets, models |
| +0x60 | ownsModelData | A | 1 when New_ModelData made it, 0 when borrowed; ReleaseModelData releases only when set |
| +0x64 | unk64 (kept) | C | set by SetUnk64 (1 in InitDefaults), gates TickCallbackA; meaning unknown |
| +0x68 | mainPart | B | parts[buf[0]] from getObjectIds; its model is linked to self in InitDefaults |
| +0x6C/+0x70/+0x74 | partCount / parts / partIds | A | CreateParts/DestroyParts/FindPartIndex |
| +0x78 / +0x8C | tickCallback / tickCallbackEnabled | A | SelectTickCallback, Enable/DisableTickCallback, Tick |
| +0x7C..+0x88 | todIndex / todFrameCount / todFrame / todFramePtr | A | SetTod, Tick |
| +0x90 | todPlaying | A | PlayTod/StopTod; gates frame advance in Tick |
| +0x94 | peer | A | LinkPeer/UnlinkPeer |

Method-table slots (TodActorMethods, D800878D4Methods, the part and
model-data tables) are named for the function `tools/classtable.py` shows
in each slot. Part views: `attribute` (SceneNode +0x10), `coord2`, and the
GsCOORDINATE2/GsCOORD2PARAM members `flg`, `tx/ty/tz`, `param`, `scale`,
`rotate`, `trans` (identified in scene_node.h).

### Proposed type names (for track 4, not applied)

Types were left as they are (renaming a type is not an additive header
edit): TodActor could be named for what it is (a TOD-animated multi-part
model); D800878D4Methods is BaseObjO's table; TimeTargetObj is this unit's
GsCOORD2PARAM view and Elem14Obj its GsCOORDINATE2 view; Unk5CObj is the
gModelDataMethods model-data class.

## Track 4 (2026-09-25, round 85, alpha)

The class (id 0x234, table `gTodActorMethods`) is unified as `TodActor` in `include/tod_actor.h`: an Actor subclass (its ctor chains to Actor's first) and Entity's base. Any source block above is the pre-unification spelling (the local `TodActorMethods` of `include/code_55dd4.h`, `linkCompanion`/`unlinkCompanion`, `companion2`, `Unk5CObj`/`Unk70ElemObj`); the live body in `src/code_55dd4.c` takes the unified types and the inherited slot and field names (`addChild`/`removeChild`, Actor's `ticker`, `Actor *` parts, `ModelData *` modelData), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 6 (round 93, echo)

Class `Class65650` renamed `TodActor` (`tools/renametype.py Class65650
TodActor`: the family, `gTodActorMethods`, `New_TodActor`, every
`TodActor__*` method, the macros, and the header, now `include/tod_actor.h`
with guard `TOD_ACTOR_H`). **Tier A for what it claims**: an Actor whose own
methods make one Actor part per TOD object (`CreateParts`, from ModelData's
`scanPackets`), select a TOD of the ModelData's TodSet (`SetTod`), advance it
per tick (`Tick`, `PlayTod`/`StopTod`) and write each frame's packets into the
parts (`ApplyTodFrame`/`ApplyTodPacket`). The name claims "an Actor animated by
a TOD" and nothing about what it is for in the game; the other methods (peer
link, tick callbacks, playTone) are described in the banner, not named in the
class name.

Types with it: `Class65650AttachToParentFn` -> `TodActorAttachToParentFn`
(same run); the getter `Get_vtable_TodActor` -> `GetTodActorMethods`
(`tools/rename.py`, the getter was spelled that way in code, not only in
prose); the ctor's descriptor `UnkArg1Obj` -> `TodActorDesc`
(`tools/renametype.py UnkArg1Obj TodActorDesc --any-stem`, tier B: only its
+0x00C is read here). Sony and member-type substitutions, one header commit:
`TimeTargetObj` -> Sony's `GsCOORD2PARAM`; `UnkArg2Obj`/`UnkArg2Methods` ->
`VabStreamObj` (field `arg2`'s type); `GroupObj` -> `Tod` via
`TODSET_TOD(set, i)`; `EntryObj2` -> `TodHeader`. See PlayTone, SetTod,
ApplyTodPacket, SetMainPartNotifies.

The header banner was rewritten as documentation (lifecycle, playback, peer
and companion, sound). The field `arg2` keeps its name: its other accessors
are in src/world/entity.c, outside this job; `sound` is PROPOSED.

History note: `tools/renametype.py` rewrote the old class name inside this report's earlier sections too (e.g. round 85's "unified as `TodActor`" was written as `Class65650`); those lines are left as the tool wrote them, pending the operator's decision on history prose.
