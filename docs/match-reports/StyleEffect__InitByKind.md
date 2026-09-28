# StyleEffect__InitByKind -- MATCHED (72/72 words)

> Renamed from `Class876FC__InitByKind` on 2026-09-26 (tools/rename.py). Address 0x80056520.

> Renamed from `func_80056520` on 2026-09-23 (tools/rename.py). Address 0x80056520.

**Unit:** dream_scene · **Round:** 44 (2026-09-15)

## Provenance

Round-42's "REOPENED -- ASSIGNABLE" banner applies: stub-stalled as
`gp_rel`-blocked on `sStyleEffectViewport`/`sStyleEffectBaseViewY` (and, in an even older revision
of this report, `addiu_at`, itself resolved round 21). Both globals were
already present in `config/gp-symbols.txt`. Matched byte-exact this round.

## What it is

One of this unit's three `self->unk54`-dispatch handlers (see
`StyleEffect__ReleaseByKind`'s comment). Folds `*arg2 + self->unk58` into a stack-local
Vec3, forwards it through `AttachWithRotScale`, snapshots a lookup table's current
value into `sStyleEffectBaseViewY`, and — only when `unk54` is 0 or 1 — calls through a
NEW cross-class vtable slot (`+0x080` on the object pointed to by the global
`sStyleEffectTmd`) before dispatching on `unk54` a second time.

## C

```c
void StyleEffect__InitByKind(LinkNode *self, void *arg1, Vec3S *arg2) {
    Vec3S local;
    s32 state;

    sStyleEffectBaseViewY = *(s32 *)((u8 *)sStyleEffectViewport + 0x18);
    AddVec3(&local, arg2, &self->unk58);
    AttachWithRotScale(self, arg1, &local, self->unk64, self->unk68);

    state = self->unk54;
    if (state < 2) {
        s32 ret = sStyleEffectTmd->methods->slot80(sStyleEffectTmd, sStyleEffectModelIds[state]);
        SceneNode__LinkModel(self, ret);
        state = self->unk54;
    }

    switch (state) {
    case 0:
        StyleEffect__PlaceModelChildren(self, 0);
        break;
    case 2:
        StyleEffect__BuildRandomSprites(self, 0);
        break;
    case 3:
        StyleEffect__SpawnPlainSprites(self, 0);
        break;
    default:
        break;
    }
}
```

Declarations added to `src/world/dream_scene.c` (kept regardless of any other
function's match state):

```c
typedef struct {
    u8 pad0[0x80];
    s32 (*slot80)(void *self, s32 arg);
} D_8008ACA4Methods;
typedef struct {
    D_8008ACA4Methods *methods;
} D_8008ACA4Obj;
extern D_8008ACA4Obj *sStyleEffectTmd;
extern void *sStyleEffectTim;
extern void *sStyleEffectViewport;
extern s32 sStyleEffectBaseViewY;
extern s32 sStyleEffectModelIds[];
```

`LinkNode`'s `pad58[0xC]` was renamed `Vec3S unk58` (same size, offset and
every other field unchanged — confirmed via the whole-image SHA1 staying
green).

## Notes

- **`state` is read once, reused, and only refreshed inside the `if`
  branch** — mirrors retail exactly: the two calls inside the `if` clobber
  the register holding `self->unk54`, so retail reloads it there and only
  there; skipping the `if` entirely falls straight into the switch on the
  original cached value. Writing this as "cache, conditionally refresh,
  then switch on the one variable" was what made the shape land without any
  register-identity residue.
- **`sStyleEffectTmd` is declared as a pointer to a tiny local struct** whose only
  named field is the `+0x080` function-pointer slot this call needs; the
  object's true type belongs to a different, uncarved unit and there is no
  shared header to extend, so this stays a local, minimal view (project's
  multiple-independent-local-views convention).
- **Two functions called here (`StyleEffect__BuildRandomSprites`, `StyleEffect__SpawnPlainSprites`) are defined
  with a NARROWER real prototype than this call site uses** (`StyleEffect__BuildRandomSprites`
  takes only `self`; `StyleEffect__SpawnPlainSprites` — defined in `dream_scene.c` — takes
  only `this`). Retail's own call sites still set up a dead second argument
  register for both. Reproduced with old-style (unprototyped) `extern void
  func_X();` declarations local to this file, which suppress the
  argument-count check without touching either function's real, narrower
  definition. See the "Proposed learning" below — this combined with the
  address-drift bug is the substantive finding this round.

### Struct edit surfaced a broader hazard: SIZE DRIFT via an unnecessary local variable

Building this function alongside `StyleEffect__SpawnSprites` (same session) surfaced a
whole-image SHA1 failure that was NOT a compile error and NOT a diff in
either function's own instruction stream — a first from-scratch instance of
CLAUDE.md's "shared-struct hazard is broader than retyping" class, but the
trigger here was a LOCAL VARIABLE choice, not a struct edit: an
unnecessary `LinkNode *sn = self;` alias in `StyleEffect__SpawnSprites` (written to
avoid repeating a cast) added ONE extra callee-saved register to that
function's own prologue, growing it by 12 bytes (3 words) and shifting
EVERY function and rodata blob after it in the whole link — including,
misleadingly, this function's own funcdiff window, which then reported 5
scattered "residue" words that were pure ripple and vanished the moment the
alias was removed. See `StyleEffect__SpawnSprites`'s own report for the mechanism.

### Proposed learning

Confirms MATCHING-GUIDE.md's own recorded lesson (`StyleEffect__DriftModelChildren`'s report,
same unit) a THIRD time in a different function: a spurious local variable
can change a function's own register allocation footprint enough to grow
its instruction count, and because `.rodata` is linked before `.text`, that
growth's symptom shows up as a *whole-image* SHA1 failure with scattered,
plausible-looking single-word diffs in OTHER, unrelated, already-correct
functions — not as an obvious diff in the function that actually grew.
`cmp -l` plus the map (CLAUDE.md's own recipe) found the true culprit in
under a minute; funcdiff's per-function window alone would have pointed at
the wrong function.

## Naming

Round 70 (alpha). `func_80056520` -> `StyleEffect__InitByKind`, **tier B**.

Only caller is the class's ctor `StyleEffect__StyleEffect` (dream_scene.c), with the
ctor's own arg3/arg4 as (parent, pos). Body: snapshot `*(sStyleEffectViewport + 0x18)`
into sStyleEffectBaseViewY; `AttachWithRotScale(self, parent, pos + offset,
rotation, scale)`; for kind < 2, `SceneNode__LinkModel(self,
sStyleEffectTmd->slot80(sStyleEffectModelIds[kind]))`; then kind 0 ->
StyleEffect__PlaceModelChildren(self, 0), 2 -> StyleEffect__BuildRandomSprites,
3 -> StyleEffect__SpawnPlainSprites (= StyleEffect__SpawnSprites(self, 0, 0, NULL)).
"Init" rests on the one ctor caller, so B.

The class: every function here runs on a gStyleEffectMethods instance.
`New_StyleEffect` allocates 0x98 bytes (where `sprites[5]` ends) and passes a
kind 0..3 as its first argument (dream_scene.c passes 0, 1, 2, 3 at its
four call sites); `StyleEffect__StyleEffect` (table +0x008, the ctor) stores it at
+0x054. `StyleEffect` is the table-address class name, the
`SceneNode`/`TodActor` convention.

Globals named in this pass: `sStyleEffectBaseViewY` (was D_8008ACB0, tier B:
written here from sStyleEffectViewport's +0x018 word, subtracted from it again by
StyleEffect__UpdateByKind; only this unit references it).

### Field and slot names in this unit's local view (applied, round 70)

`LinkNode` and `LinkNodeMethods` are defined only in `src/world/dream_scene.c`,
so the compiler's accessor list after renaming the definition was entirely in
this unit (every `has no member` error was in src/world/dream_scene.c, all fixed; build and `tools/check-nonmatching.sh`
green). `typedef struct LinkNode StyleEffect;` was added for the owner's
method signatures; zero bytes changed.

| offset | old | new | tier | evidence |
| --- | --- | --- | --- | --- |
| +0x014 | unk14 | coord2 | B | scene_node.h maps SceneNodeObj +0x14 to GsDOBJ2.coord2; `*coord2 = 0` is its flg |
| +0x020 | unk20 | model | A | SceneNode__LinkModel stores its 2nd argument here; PlaceModelChildren hands it to each child |
| +0x024 | unk24 | tick | B | StyleEffect__SetParams zeroes it, StyleEffect__Update (slot +0x0EC) increments it before every update |
| +0x054 | unk54 | kind | B | ctor stores New's first argument, 0..3 at the four dream_scene.c call sites; three switches on it |
| +0x058 | unk58 | offset | B | added to the caller's position in Init and Update |
| +0x064 | unk64 | rotation | B | passed as updateRotation's data |
| +0x068 | unk68 | scale | B | passed as updateScale's data |
| +0x06C | unk6C | modelChildLayout | B | 0 = no model children; index into sModelChildSpacing |
| +0x070 | unk70 | tableIndex | C-ish | only ever an index (sModelChildDriftZ, sSpriteShiftX) |
| +0x074 | unk74 | color | B | every sprite's slotB8 (Sprite__SetColor copies 3 bytes to GsSPRITE r,g,b) |
| +0x078 | unk78 | altColor | B | sprites[1]'s slotB8 argument instead of color when non-NULL |
| +0x07C | arr7C | modelChildren | B | New_Actor objects, linked to the owner's model |
| +0x084 | arr84 | sprites | B | New_VariantSprite objects (GsSPRITE at +0x64, see StyleEffect__SpawnSprites) |
| slot +0x044 | slot44 | updateRotation | B | SceneNode__UpdateRotation (set/add GsCOORD2PARAM.rotate, degrees) |
| slot +0x048 | slot48 | updateScale | B | SceneNode__UpdateScale (set/add .scale); sprite override VariantSprite__UpdateScale also a scale |
| slot +0x04C | slot4C | attachToParent | B | SceneNode__AttachToParent (parent link, coord2 super, coord.t) |
| slot +0x060 | slot60 | setDisplay | B | SceneNode__SetDisplay / Sprite__SetDisplay: attribute bit 31 = !on (GsDOFF) |
| slot +0x064 | slot64 | setSemiTrans | B | SceneNode__SetSemiTrans / Sprite__SetSemiTrans: bit 30 (GsALON) |
| slot +0x068 | slot68 | setSemiTransRate | B | SceneNode__SetSemiTransRate / Sprite__SetSemiTransRate: bits 28-29 (GsAZERO..GsATHREE) |
| slot +0x0B8 | slotB8 | kept | C | class-dependent: Actor__SetTranslation (translation) on the owner and model children, RGB on sprites |
| slot +0x0BC | slotBC | addTranslation | B | Actor__AddTranslation; only called on model children (sprite override is a no-op) |

The unused `D_8008ACA4Methods::slot80` is left alone: sStyleEffectTmd is a
different object (captured by SetStyleEffectSources) whose class is unknown.

## Proposed field names

For the HEAD, by type scope; none applied here (other units' views).

- `dream_scene.c` `Obj876FC` (same object): `unk24` -> `tick` (B, same
  evidence as above: zeroed by StyleEffect__SetParams, incremented by StyleEffect__Update);
  `unk54` -> `kind` (B); `block58` -> `params` (B: the 0x24-byte block this
  unit reads as offset/rotation/scale/modelChildLayout/tableIndex/color/
  altColor).
- `include/scene_node.h` `SceneNodeMethods`: `slot44` -> `updateRotation`,
  `slot48` -> `updateScale` (B; SceneNode__UpdateRotation / SceneNode__UpdateScale bodies, both
  matched since those comments said "still queued").

## Track 4b (2026-09-25, round 85)

`sStyleEffectTmd`/`sStyleEffectTim`/`sStyleEffectViewport` were `s32` in dream_scene.c and
`D_8008ACA4Obj *`/`void *`/`void *` in dream_scene.c. Both units now
declare `Actor *`/`void *`/`void *`: the local `D_8008ACA4Methods` view is gone and the +0x080 call reads `getSetUnk10Flag8`, the name `SetStyleEffectSources` calls the same slot by. Byte-identical; no new `-Wall`
warning.

## Track 4 (2026-09-26, round 88, charlie)

dream_scene.c's `LinkNode` view (owner and children under one type) is gone: the owner is `StyleEffect` (include/style_effect.h), `modelChildren` are `Actor *`, `sprites` are `VariantSprite *`, and the local `Vec3S` is `LongVec3`. Accessor renames: `kind` is Actor's `pendingExtra` (+0x054, where the ctor stores it); `offset`/`rotation`/`scale`/`modelChildLayout`/`tableIndex`/`color`/`altColor` are `params.*`; slot `slotB8` is Actor's `setTranslation` on the owner and model children and Sprite's `setColor` on sprites; `*coord2 = 0` is `coord2->flg = 0`. Image byte-identical.

## Track 6 (round 93, bravo)

Globals renamed with rename.py: D_8008ACA4 -> sStyleEffectTmd (the
DREAMER.TMD LinkResource SetStyleEffectSources stores; kinds 0 and 1 take
their model from it), D_8008AB98 -> sStyleEffectModelIds (the model index per
kind), D_8008ACAC -> sStyleEffectViewport (ObjM's cached Viewport; the +0x018
word read here is refView.vp.y, the viewpoint y), gTrackedYSnapshot ->
sStyleEffectBaseViewY (that y, snapshotted at build). PROPOSED for track 7:
read the viewpoint y as `sStyleEffectViewport->refView.vp.y` once the global
is typed `Viewport *`, and `sStyleEffectTmd` as `LinkResource *` with
`getModel`. renametype.py rewrote the old class name inside this report's earlier prose too (known, pending an operator decision); those lines are history and were not hand-restored, so read `StyleEffect` in them as `Class876FC`.

## History moved from the unit's comments (track 7, round 101)

The unit's comment on the four unprototyped helpers (the source now keeps a
one-paragraph `MATCHING:` note), verbatim:

```
/* Four of the class's one-parameter helpers are called here with a dead
 * second argument that is byte-load-bearing, so include/StyleEffect.h
 * declares them WITHOUT a prototype (old-style), which is what lets these
 * calls pass it:
 *  - StyleEffect__SpawnPlainSprites (class_3bb8c_o.c): its body WRITES
 *    $a1/$a2/$a3 to zero before any read, but InitByKind's retail emits
 *    `move a1,zero` at 0x80056624;
 *  - StyleEffect__RandomizeSprites (class_3bb8c_o.c): its body reads only
 *    $a0 (`addiu s0,a0,136`), but UpdateByKind's retail emits `move a1,s1`
 *    at 0x800566FC;
 *  - StyleEffect__BuildRandomSprites (below): its body reads only $a0
 *    (`move s1,a0`); InitByKind's retail emits `move a1,zero` at 0x80056614;
 *  - StyleEffect__DriftModelChildren (below): its body writes $a1 (`move
 *    a1,zero`) before any read; UpdateByKind's retail emits `move a1,s1` in
 *    the jal delay slot at 0x800566D8 (round 75).
 * NoOpIgnoreArgs (class_3bb8c_o.c, empty) is the same idiom. */
```

And its comment on the SetStyleEffectSources globals, verbatim:

```
/* Three globals class_3bb8c_o.c's SetStyleEffectSources captures once from its
 * parameters (declared there with the same types; track 4b, round 85):
 * gStyleEffectTmd is the Actor it ran on, called here through SceneNode's
 * +0x080 getSetUnk10Flag8 as that function calls it; gStyleEffectTim is
 * forwarded opaquely to New_VariantSprite as its third argument; gStyleEffectViewport's
 * pointee has a field at +0x018 that StyleEffect__InitByKind and
 * StyleEffect__UpdateByKind snapshot/diff via gStyleEffectBaseViewY. */
```

(The slot named there is now `setBackClip`; the +0x018 field is the
Viewport's `refView.vp.y`, see below.)

## Naming (track 7, round 101)

- `*(s32 *)((u8 *)sStyleEffectViewport + 0x18)` is
  `((Viewport *)sStyleEffectViewport)->refView.vp.y`: the pointer is
  StyleSceneRefs::viewport, a `Viewport *` (dream_scene.c passes it to
  SetStyleEffectSources), and +0x018 is `refView` (+0x014) `.vp.y` (+0x004)
  in include/viewport.h. The extern stays `void *` because dream_scene.c
  declares it so (proposed to the head: retype both).
- Locals: `local` -> `placed` (pos + offset), `state` -> `kind` (it is
  `pendingExtra`, the StyleEffectKind), `ret` -> `model` (setBackClip's
  model, handed to SceneNode__LinkModel).
- Cases are StyleEffectKind's members; `kind < 2` is
  `kind <= STYLE_EFFECT_MODEL` (the two model kinds link a model). Same
  code: `slti 2` either way.
