# PlaceDreamAuxEntityByPlayer

> Renamed from `DespawnDreamAuxEntity` on 2026-09-27 (tools/rename.py). Address 0x8005cf34.

> Renamed from `func_8005CF34` on 2026-09-21 (tools/rename.py). Address 0x8005cf34.

**Unit:** dream_aux · **Size:** 42 words · **Status:** MATCHED round 43
(42/42, byte-exact whole-image build).

## History

Filed BLOCKED in round 2026-08-30-a on five `%gp_rel` references (the first
to `gDreamAuxWorld`). Round 42 resolved the gp-relative blocker. Never actually
attempted -- the stub carried no derivation. Round 43 derived and matched it.

## What it does

Given a `DreamAuxSlot *`, if its `entity` field is live, runs it through a
short "despawn" sequence: tick its vtable slot 0x14, fill a local 3-word
position vector via `SceneNode__LocalOffsetToWorldPos`, hand that vector plus three unit
globals to vtable slot 0x13, then call `SceneNode__FaceTarget` on it.

```c
extern void SceneNode__LocalOffsetToWorldPos(void *self, s32 *dst, s32 *src, s32 arg4);
extern void SceneNode__FaceTarget(void *self, void *target, s32 arg2, s32 arg3, void *arg4);

void PlaceDreamAuxEntityByPlayer(DreamAuxSlot *a0)
{
    if (a0->entity != NULL) {
        s32 localPos[3];

        ((DreamAuxObjFn14)a0->entity->vtable[0x14])(a0->entity);
        SceneNode__LocalOffsetToWorldPos((void *)sDreamAuxWorld, localPos, a0->pos, 0);
        ((DreamAuxObjFn13)a0->entity->vtable[0x13])(a0->entity, sDreamAuxWorld, sDreamAuxFrameClock, (void *)sDreamAuxStageMap, localPos);
        SceneNode__FaceTarget(a0->entity, (void *)sDreamAuxWorld, 1, 0, 0);
    }
}
```

This closes the loop `SetDreamAuxWorld` (matched earlier this round) opened:
`DreamAuxSlot.entity` is the `New_Entity` result that function stashed at
offset 0x4, and `DreamAuxSlot`'s remaining 12 bytes (previously
`u8 unkC[0xC]`) are exactly a 3-word position vector -- confirmed by this
function passing `a0->pos` as `SceneNode__LocalOffsetToWorldPos`'s `src` parameter (that
function, `scene_node.c`, treats `src` as a 3-word vector unconditionally).
`DreamAuxSlot` is renamed accordingly in `include/dream_aux.h`
(`void *obj; DreamAuxObj *entity; s32 pos[3];`, still 0x14 bytes).

`SceneNode__LocalOffsetToWorldPos` and `SceneNode__FaceTarget` are both already-matched functions in a
different unit (`scene_node.c`), each with its OWN unit's typed view of
`self`/`target` (`SceneNodeObj *` / `Entity *`, per `scene_node.h` and
`entity.h`'s independent local views of the same shared-ancestor slot). This
unit adds a third, `void *`-typed, local view rather than pulling in either
header -- consistent with the project's per-unit-view convention
(CLAUDE.md's "keep next to your code anything that encodes *your* reading of
a class"). `SceneNode__LocalOffsetToWorldPos`'s 4th argument register is always 0 at every
known call site despite the function's own 3-parameter C signature not
reading it (see `src/world/dream_sys.c`'s identical local prototype); this unit's
local prototype reproduces that the same way.

Vtable slot 0x13 (byte offset 0x4C) is the SAME shared-ancestor slot
`include/entity.h` documents (`slot4C`), but that header's guessed parameter
types (`s32 arg1, s32 arg2, void *arg3, s32 arg4`) do not fit this call
site's actual last argument (`localPos`, a pointer, not a scalar) -- a
generic vtable slot passes through whatever the concrete override expects,
and different callers legitimately pass different real types through the
identical calling convention. This unit keeps its own `DreamAuxObjFn13`
alias (`s32, s32, void *, void *`) rather than importing entity.h's.

## Derivation notes

One attempt short of byte-exact, one fix, and it is the SAME shape as the
`PlaceDreamAuxEntityByPlayer`-adjacent learning already on file for `LookupDreamAuxTrigger`
(narrow value kept live too long) but the opposite direction:

- **First pass (5/42, one word too long):** cached `a0->entity` into a local
  `DreamAuxObj *entity` up front and used `entity` at all four call sites.
  This is the more natural-looking C, but retail does NOT cache the field --
  it keeps only `a0` itself in a saved register (`$s0`) across the whole
  function and RE-READS `a0->unk4` from memory at each of the three points
  that need it (once before each of the three calls that take it as an
  argument). Caching it costs a second saved register (`$s1` in the built
  object, `$s0`-only in retail) and reorders several loads. Rewriting every
  use as `a0->entity` directly (no local) matched immediately.

## Proposed learning

Add to the corpus: **do not cache a struct field into a local just because
it is used more than once -- check whether retail keeps the BASE pointer
alone live and re-reads the field at each use.** The tell here was a saved-
register count mismatch (built used two callee-saved registers where retail
used one) visible immediately from `objdump`, before even reading the
per-word diff. This is the same family as the "keep a pointer computation
that is one retail expression as ONE C statement" learning already on file
(round 10) but at one level up: that one is about splitting an expression
across statements, this one is about splitting a REPEATED field access into
a cached local at all.

## Naming

**PlaceDreamAuxEntityByPlayer** — tier B. Given a `DreamAuxSlot *`, if its `entity`
is live: ticks its vtable slot 0x14, computes a world-space position from
the slot's stored `pos` via `SceneNode__LocalOffsetToWorldPos`, dispatches
that position through vtable slot 0x13, then calls
`SceneNode__FaceTarget`. Confirming the name this function's own report
already carried since round 43 ("despawn sequence") -- consistent with
`TryDreamAuxTrigger` calling it as a small-probability ALTERNATIVE to firing
a trigger normally (culling an existing occupant instead of processing a
new one). Tier B: the sequence's mechanics are clear, but whether it
literally removes the entity (vs. repositions/reorients it) is inferred,
not read directly off any single instruction.

## Track 4 (2026-09-26, round 88, echo)

`a0->entity` is now `Entity *` and its raw `vtable[0x14]`/`vtable[0x13]` calls are detachFromParent and attachToParent (through TodActorAttachToParentFn), same bytes. DreamAuxObjFn13/14 deleted.

Byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Round 100 (alpha): track 7, moved from src/world/dream_aux.c and include/dream_aux.h

## Naming (round 100)

**PlaceDreamAuxEntityByPlayer** (was DespawnDreamAuxEntity) -- tier A. It
despawns nothing: it detaches the slot's entity, converts the slot's `pos`
(an offset from the player, {0, -200, 8000} in the image) to a world
position with SceneNode__LocalOffsetToWorldPos, re-attaches the entity there
and turns it to face the player (SceneNode__FaceTarget, zeroPitch 1).
TryDreamAuxTrigger calls it when a chunk trigger's day parity rules the day
out. a0 -> slot, localPos -> worldPos (it is the world position the
conversion writes). Byte-identical.
