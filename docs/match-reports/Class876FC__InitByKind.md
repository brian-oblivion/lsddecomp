# Class876FC__InitByKind -- MATCHED (72/72 words)

> Renamed from `func_80056520` on 2026-09-23 (tools/rename.py). Address 0x80056520.

**Unit:** class_3bb8c_s · **Round:** 44 (2026-09-15)

## Provenance

Round-42's "REOPENED -- ASSIGNABLE" banner applies: stub-stalled as
`gp_rel`-blocked on `D_8008ACAC`/`gTrackedYSnapshot` (and, in an even older revision
of this report, `addiu_at`, itself resolved round 21). Both globals were
already present in `config/gp-symbols.txt`. Matched byte-exact this round.

## What it is

One of this unit's three `self->unk54`-dispatch handlers (see
`Class876FC__ReleaseByKind`'s comment). Folds `*arg2 + self->unk58` into a stack-local
Vec3, forwards it through `AttachWithRotScale`, snapshots a lookup table's current
value into `gTrackedYSnapshot`, and — only when `unk54` is 0 or 1 — calls through a
NEW cross-class vtable slot (`+0x080` on the object pointed to by the global
`D_8008ACA4`) before dispatching on `unk54` a second time.

## C

```c
void Class876FC__InitByKind(LinkNode *self, void *arg1, Vec3S *arg2) {
    Vec3S local;
    s32 state;

    gTrackedYSnapshot = *(s32 *)((u8 *)D_8008ACAC + 0x18);
    AddVec3(&local, arg2, &self->unk58);
    AttachWithRotScale(self, arg1, &local, self->unk64, self->unk68);

    state = self->unk54;
    if (state < 2) {
        s32 ret = D_8008ACA4->methods->slot80(D_8008ACA4, D_8008AB98[state]);
        Class6B5CC__LinkModel(self, ret);
        state = self->unk54;
    }

    switch (state) {
    case 0:
        Class876FC__PlaceModelChildren(self, 0);
        break;
    case 2:
        Class876FC__BuildRandomSprites(self, 0);
        break;
    case 3:
        Class876FC__SpawnPlainSprites(self, 0);
        break;
    default:
        break;
    }
}
```

Declarations added to `src/class_3bb8c_s.c` (kept regardless of any other
function's match state):

```c
typedef struct {
    u8 pad0[0x80];
    s32 (*slot80)(void *self, s32 arg);
} D_8008ACA4Methods;
typedef struct {
    D_8008ACA4Methods *methods;
} D_8008ACA4Obj;
extern D_8008ACA4Obj *D_8008ACA4;
extern void *D_8008ACA8;
extern void *D_8008ACAC;
extern s32 gTrackedYSnapshot;
extern s32 D_8008AB98[];
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
- **`D_8008ACA4` is declared as a pointer to a tiny local struct** whose only
  named field is the `+0x080` function-pointer slot this call needs; the
  object's true type belongs to a different, uncarved unit and there is no
  shared header to extend, so this stays a local, minimal view (project's
  multiple-independent-local-views convention).
- **Two functions called here (`Class876FC__BuildRandomSprites`, `Class876FC__SpawnPlainSprites`) are defined
  with a NARROWER real prototype than this call site uses** (`Class876FC__BuildRandomSprites`
  takes only `self`; `Class876FC__SpawnPlainSprites` — defined in `class_3bb8c_o.c` — takes
  only `this`). Retail's own call sites still set up a dead second argument
  register for both. Reproduced with old-style (unprototyped) `extern void
  func_X();` declarations local to this file, which suppress the
  argument-count check without touching either function's real, narrower
  definition. See the "Proposed learning" below — this combined with the
  address-drift bug is the substantive finding this round.

### Struct edit surfaced a broader hazard: SIZE DRIFT via an unnecessary local variable

Building this function alongside `Class876FC__SpawnSprites` (same session) surfaced a
whole-image SHA1 failure that was NOT a compile error and NOT a diff in
either function's own instruction stream — a first from-scratch instance of
CLAUDE.md's "shared-struct hazard is broader than retyping" class, but the
trigger here was a LOCAL VARIABLE choice, not a struct edit: an
unnecessary `LinkNode *sn = self;` alias in `Class876FC__SpawnSprites` (written to
avoid repeating a cast) added ONE extra callee-saved register to that
function's own prologue, growing it by 12 bytes (3 words) and shifting
EVERY function and rodata blob after it in the whole link — including,
misleadingly, this function's own funcdiff window, which then reported 5
scattered "residue" words that were pure ripple and vanished the moment the
alias was removed. See `Class876FC__SpawnSprites`'s own report for the mechanism.

### Proposed learning

Confirms MATCHING-GUIDE.md's own recorded lesson (`Class876FC__DriftModelChildren`'s report,
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

Round 70 (alpha). `func_80056520` -> `Class876FC__InitByKind`, **tier B**.

Only caller is the class's ctor `Class876FC__Class876FC` (class_3bb8c_r.c), with the
ctor's own arg3/arg4 as (parent, pos). Body: snapshot `*(D_8008ACAC + 0x18)`
into gTrackedYSnapshot; `AttachWithRotScale(self, parent, pos + offset,
rotation, scale)`; for kind < 2, `Class6B5CC__LinkModel(self,
D_8008ACA4->slot80(D_8008AB98[kind]))`; then kind 0 ->
Class876FC__PlaceModelChildren(self, 0), 2 -> Class876FC__BuildRandomSprites,
3 -> Class876FC__SpawnPlainSprites (= Class876FC__SpawnSprites(self, 0, 0, NULL)).
"Init" rests on the one ctor caller, so B.

The class: every function here runs on a gClass876FCMethods instance.
`New_Class876FC` allocates 0x98 bytes (where `sprites[5]` ends) and passes a
kind 0..3 as its first argument (class_3bb8c_n.c passes 0, 1, 2, 3 at its
four call sites); `Class876FC__Class876FC` (table +0x008, the ctor) stores it at
+0x054. `Class876FC` is the table-address class name, the
`Class6B5CC`/`Class65650` convention.

Globals named in this pass: `gTrackedYSnapshot` (was D_8008ACB0, tier B:
written here from D_8008ACAC's +0x018 word, subtracted from it again by
Class876FC__UpdateByKind; only this unit references it).

### Field and slot names in this unit's local view (applied, round 70)

`LinkNode` and `LinkNodeMethods` are defined only in `src/class_3bb8c_s.c`,
so the compiler's accessor list after renaming the definition was entirely in
this unit (every `has no member` error was in src/class_3bb8c_s.c, all fixed; build and `tools/check-nonmatching.sh`
green). `typedef struct LinkNode Class876FC;` was added for the owner's
method signatures; zero bytes changed.

| offset | old | new | tier | evidence |
| --- | --- | --- | --- | --- |
| +0x014 | unk14 | coord2 | B | code_d294.h maps Class6B5CCObj +0x14 to GsDOBJ2.coord2; `*coord2 = 0` is its flg |
| +0x020 | unk20 | model | A | Class6B5CC__LinkModel stores its 2nd argument here; PlaceModelChildren hands it to each child |
| +0x024 | unk24 | tick | B | Class876FC__SetParams zeroes it, Class876FC__Update (slot +0x0EC) increments it before every update |
| +0x054 | unk54 | kind | B | ctor stores New's first argument, 0..3 at the four class_3bb8c_n.c call sites; three switches on it |
| +0x058 | unk58 | offset | B | added to the caller's position in Init and Update |
| +0x064 | unk64 | rotation | B | passed as updateRotation's data |
| +0x068 | unk68 | scale | B | passed as updateScale's data |
| +0x06C | unk6C | modelChildLayout | B | 0 = no model children; index into gModelChildSpacing |
| +0x070 | unk70 | tableIndex | C-ish | only ever an index (gModelChildDriftZ, gSpriteShiftX) |
| +0x074 | unk74 | color | B | every sprite's slotB8 (Sprite__SetColor copies 3 bytes to GsSPRITE r,g,b) |
| +0x078 | unk78 | altColor | B | sprites[1]'s slotB8 argument instead of color when non-NULL |
| +0x07C | arr7C | modelChildren | B | New_Actor objects, linked to the owner's model |
| +0x084 | arr84 | sprites | B | New_Class879C4 objects (GsSPRITE at +0x64, see Class876FC__SpawnSprites) |
| slot +0x044 | slot44 | updateRotation | B | Class6B5CC__UpdateRotation (set/add GsCOORD2PARAM.rotate, degrees) |
| slot +0x048 | slot48 | updateScale | B | Class6B5CC__UpdateScale (set/add .scale); sprite override Class879C4__UpdateScale also a scale |
| slot +0x04C | slot4C | attachToParent | B | Class6B5CC__AttachToParent (parent link, coord2 super, coord.t) |
| slot +0x060 | slot60 | setDisplay | B | Class6B5CC__SetDisplay / Sprite__SetDisplay: attribute bit 31 = !on (GsDOFF) |
| slot +0x064 | slot64 | setSemiTrans | B | Class6B5CC__SetSemiTrans / Sprite__SetSemiTrans: bit 30 (GsALON) |
| slot +0x068 | slot68 | setSemiTransRate | B | Class6B5CC__SetSemiTransRate / Sprite__SetSemiTransRate: bits 28-29 (GsAZERO..GsATHREE) |
| slot +0x0B8 | slotB8 | kept | C | class-dependent: Actor__SetTranslation (translation) on the owner and model children, RGB on sprites |
| slot +0x0BC | slotBC | addTranslation | B | Actor__AddTranslation; only called on model children (sprite override is a no-op) |

The unused `D_8008ACA4Methods::slot80` is left alone: D_8008ACA4 is a
different object (captured by Actor__func_56f5c) whose class is unknown.

## Proposed field names

For the HEAD, by type scope; none applied here (other units' views).

- `class_3bb8c_r.c` `Obj876FC` (same object): `unk24` -> `tick` (B, same
  evidence as above: zeroed by Class876FC__SetParams, incremented by Class876FC__Update);
  `unk54` -> `kind` (B); `block58` -> `params` (B: the 0x24-byte block this
  unit reads as offset/rotation/scale/modelChildLayout/tableIndex/color/
  altColor).
- `include/code_d294.h` `Class6B5CCMethods`: `slot44` -> `updateRotation`,
  `slot48` -> `updateScale` (B; Class6B5CC__UpdateRotation / Class6B5CC__UpdateScale bodies, both
  matched since those comments said "still queued").

## Track 4b (2026-09-25, round 85)

`D_8008ACA4`/`D_8008ACA8`/`D_8008ACAC` were `s32` in class_3bb8c_o.c and
`D_8008ACA4Obj *`/`void *`/`void *` in class_3bb8c_s.c. Both units now
declare `Actor *`/`void *`/`void *`: the local `D_8008ACA4Methods` view is gone and the +0x080 call reads `getSetUnk10Flag8`, the name `Actor__func_56f5c` calls the same slot by. Byte-identical; no new `-Wall`
warning.
