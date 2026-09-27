# New_Entity

**Unit:** Entity · **Size:** 35 words · **Status:** MATCHED (35/35 words, whole-image build verified byte-exact)

## What it does

The allocator for `Entity`: `BMemPMgrAlloc(0x108)` allocates 0x108 bytes,
then `Get_vtable_Entity()->ctor(obj, arg0, arg1, arg2)` (the vtable's own
`ctor` slot, `Entity__Entity` — see that report) constructs it in place. On
allocation failure, returns NULL immediately. On construction failure
(`ctor` returns NULL), frees the allocation via `BMemPMgrFree` and returns
NULL. On success, returns the constructed object.

## Derivation

Straightforward from the disassembly; the only real question was C shape,
not semantics — see the attempt log.

## Final C

```c
Entity *New_Entity(void *arg0, void *arg1, void *arg2) {
    Entity *obj;
    Entity *result;

    obj = BMemPMgrAlloc(0x108);
    result = NULL;
    if (obj != NULL) {
        result = obj;
        if (Get_vtable_Entity()->ctor(obj, arg0, arg1, arg2) == NULL) {
            BMemPMgrFree(obj);
            result = NULL;
        }
    }
    return result;
}
```

## Attempt log

Two size-correct-but-not-byte-exact attempts before this one:

1. `obj = alloc(); result = NULL; if (obj) { result = obj; if (ctor(...)==NULL) {...; result=NULL;} } return result;`
   — matched retail's overall size (0x8C) but diverged on ONE instruction:
   retail's first `beqz $s0,END` fills its delay slot with `move $v0,zero`
   (the failure-return value, computed *before* the branch is even decided),
   while this shape put `move $v0,$s0` there instead — GCC hadn't spilled the
   allocation result into `$s0` yet at that point, it kept it live in `$v0`
   across the whole first check.
2. `obj = alloc(); if (obj != NULL) { if (ctor(...)==NULL) {free; obj=NULL;} } return obj;`
   (single variable, no separate `result`) — this got the `move $s0,$v0`
   *timing* right (spilled immediately after the alloc call, matching
   retail), but now the delay slot held `move $v0,$s0` instead of retail's
   `move $v0,zero`, and the function grew 4 bytes (0x90) from an extra
   `move` needed to route the final NULL back through `$s0`.

**What matched:** going back to the early-`return NULL;` idiom
(`if (obj == NULL) { return NULL; }`) rather than a single shared-`result`
flag threaded through an `if`/`else`. With *nothing else* in that branch,
GCC folds the constant return value into the branch's own delay slot AND
still eagerly spills `obj` into `$s0` right after the allocation call —
exactly retail's shape. The naive assumption that an early return duplicates
epilogue code (and would therefore *grow* the function) was wrong here; GCC
2.6.3 merges both `return NULL;` sites (the early one and the
post-`BMemPMgrFree` one) onto the same physical epilogue via branches, using
the branch's delay slot for the free case.

## Proposed learning

For a `New_X`-shaped allocator (`alloc → null check → ctor → null check →
optional free → return`), prefer a **simple early `if (x == NULL) return
NULL;`** for the allocation-failure check over threading a separate
`result` variable through the whole function. The early-return form let GCC
both (a) eagerly spill the allocated pointer into a callee-saved register
right after the call, and (b) fold the failure return value into the
branch's own delay slot — a single-variable, single-purpose local matches
retail's register allocation more often than a "value defaults to failure,
gets overwritten on success" flag pattern does, at least for this compiler
at `-O2`. This generalizes the project's existing note (`New_GameApplication`,
`docs/MATCHING-GUIDE.md`) that the `New_X` shape is common across ~60
classes — worth trying the early-return form first on any future one.

## Naming

**Tier A.** The class's own dedicated allocator: allocate raw bytes, dispatch
the vtable's own `ctor` slot, free and return NULL on either failure. Its
mechanics (a `New_Class` allocator paired with a `Class__Class` constructor)
ARE its purpose, and the shape matches `New_DreamSys`/`New_TodActor`
elsewhere in the codebase. Not renamed (already correct).

## Track 4 (2026-09-26, round 88, echo)

First parameter retyped `void *` -> `s32 moodIndex` (Entity__Entity stores it in `moodIndex`; code_4cd08 passes `i + 0x62` and `kind` with `(void *)` casts, now dropped). code_4cd08's local `extern void *New_Entity` is deleted; it includes Entity.h.

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a TodActor subclass whose table and object expand `TODACTOR_SLOTS`/`TODACTOR_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Polish (round 96, bravo, track 7)

- Step 3: parameter arg2 -> sound: it is TodActor's ctor's second argument, the sound bank TodActor keeps in `sound` (TodActor.h). Also in Entity.h's prototype and ctor slot.

- Step 4: BMemPMgrAlloc(0x108) -> sizeof(Entity) (Entity is 0x108 bytes; byte-identical).

- Step 5: the unit banner of src/Entity.c was rewritten as documentation. The history it carried, kept here:
  - "The Entity class -- fully matched, no INCLUDE_ASM left (round 56 was a track 3 naming pass, not matching work)."
  - "This is the first 25 of a 142-function block split at Entity__UpdateTargetProximity; the rest (Entity_b through Entity_g, all sharing include/Entity.h) hold the mood-dispatch handler tables and the per-frame behaviour those handlers run."
  - "Entity's own vtable is gEntityMethods (asm/data/79528.data.s), reached via Get_vtable_Entity (Entity_b.c); `tools/classtable.py gEntityMethods` is the ground truth for which function occupies which slot, including the several self-referential slots this unit's own functions dispatch back into (activate/deactivate/getProximityRatio/startSoundCue/stopSoundCue)."
  - "The overrides of TodActor's slots are named for their slots (Entity__Finalize, Reset, AttachToParent, DetachFromParent, OnGridCellLinkCommand; track 4, round 88)."
