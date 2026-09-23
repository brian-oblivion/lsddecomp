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
        LinkOwnerObj__func_56e1c(self, 0);
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
- **Two functions called here (`Class876FC__BuildRandomSprites`, `LinkOwnerObj__func_56e1c`) are defined
  with a NARROWER real prototype than this call site uses** (`Class876FC__BuildRandomSprites`
  takes only `self`; `LinkOwnerObj__func_56e1c` — defined in `class_3bb8c_o.c` — takes
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
