# StyleEffect__UpdateByKind -- MATCHED (54/54 words)

> Renamed from `Class876FC__UpdateByKind` on 2026-09-26 (tools/rename.py). Address 0x80056640.

> Renamed from `func_80056640` on 2026-09-23 (tools/rename.py). Address 0x80056640.

**Unit:** class_3bb8c_s · **Round:** 44 (2026-09-15)

## Provenance

Round-42's "REOPENED -- ASSIGNABLE" banner applies: stub-stalled as
`gp_rel`-blocked on `D_8008ACAC`/`gTrackedYSnapshot`, both already present in
`config/gp-symbols.txt`. Matched byte-exact this round (after fixing an
unrelated whole-image size regression caused by a sibling function in the
same session -- see `StyleEffect__SpawnSprites`'s report).

## What it is

The second of this unit's three `self->unk54`-dispatch handlers (see
`StyleEffect__ReleaseByKind`'s comment and `StyleEffect__InitByKind`, the first). Folds
`*(Vec3S*)arg1 + self->unk58` into a stack-local, adds the delta between
`D_8008ACAC`'s pointee's `+0x18` field and the snapshot `StyleEffect__InitByKind` left
in `gTrackedYSnapshot`, forwards the result through `slotB8`, then dispatches on
`unk54` to one of three different callees than `StyleEffect__InitByKind`'s own switch.

## C

```c
void StyleEffect__UpdateByKind(LinkNode *self, void *arg1) {
    Vec3S local;

    AddVec3(&local, (Vec3S *)arg1, &self->unk58);
    local.y += *(s32 *)((u8 *)D_8008ACAC + 0x18) - gTrackedYSnapshot;
    self->methods->slotB8(self, &local);

    switch (self->unk54) {
    case 0:
        StyleEffect__DriftModelChildren(self, arg1);
        break;
    case 2:
        NoOpIgnoreArgs(self, arg1);
        break;
    case 3:
        StyleEffect__RandomizeSprites(self, arg1);
        break;
    default:
        break;
    }
}
```

## Notes

- **`NoOpIgnoreArgs` and `StyleEffect__RandomizeSprites` are `class_3bb8c_o.c` functions with
  narrower real signatures** (`NoOpIgnoreArgs(void)`, `StyleEffect__RandomizeSprites
  (LinkOwnerObj *this)`) than this call site's two-argument shape. Retail's
  own caller here still sets up the dead second register regardless.
  Declared old-style (`extern void NoOpIgnoreArgs(); extern void
  StyleEffect__RandomizeSprites();`) local to this file so the extra argument doesn't
  trigger a parameter-count error against their real definitions elsewhere.
- **`StyleEffect__DriftModelChildren` is defined later in this SAME file** (still
  `INCLUDE_ASM` as of this report) and is likewise called here with a dead
  second argument; same old-style-declaration treatment, needed so the
  eventual real (one-argument) definition doesn't conflict.
- This function and `StyleEffect__InitByKind` share the `D_8008ACAC`/`gTrackedYSnapshot`
  snapshot-and-diff pattern; `StyleEffect__InitByKind` always runs first for a given
  node (it's the one that WRITES `gTrackedYSnapshot`), so the diff computed here is
  "how much did the tracked field move since the last time `StyleEffect__InitByKind`
  ran" -- descriptive only, doesn't affect the C shape.

### Proposed learning

Same session, same unit: an old-style (`extern void f();`) declaration is
the right tool whenever a call site sets up more argument registers than a
callee's real, already-established prototype takes, whether the callee
lives in this file (forward reference, `StyleEffect__DriftModelChildren`) or in a sibling unit
already matched elsewhere (`NoOpIgnoreArgs`, `StyleEffect__RandomizeSprites`). It reproduces
retail's caller-side register setup without touching the callee's real
signature, and avoids the C89 "too many arguments to function" error a full
prototype would otherwise produce -- one of CLAUDE.md's own listed
silent-on-grep semantic errors.

## Extern arity (round 59)

**Verdict: arity-ok idiom**, in the opposite direction from most of the round:
here the declaration has FEWER parameters than the definition, not more.

**Callee evidence** (`0x80056640`, and the matched definition in
`src/class_3bb8c_s.c`): the body reads both argument registers before writing
them, and forwards `$a1` straight on:

```
80056648:  move  s0,a0
80056650:  move  s1,a1            <- $a1 read
80056654:  addiu a0,sp,16
8005665c:  jal   80056794 <AddVec3>   ; AddVec3(&local, arg1, &self->unk58)
80056660:  addiu a2,s0,88
```

`s1` is later re-forwarded to every arm of the unit's switch
(`move a1,s1` at `0x800566D8`, `0x800566EC`, `0x800566FC`). So the definition's
`void StyleEffect__UpdateByKind(LinkNode *self, void *arg1)` is right: two real arguments.

**Why `src/class_3bb8c_r.c`'s one-parameter declaration stays.** Its caller
`StyleEffect__Update` passes only `self`, and retail sets up nothing else:

```
800564fc:  lw    v0,36(a0)
80056504:  addiu v0,v0,1
80056508:  jal   80056640 <StyleEffect__UpdateByKind>
8005650c:  sw    v0,36(a0)         <- the delay slot is the `self->unk24 + 1`
                                      store, not argument setup
```

`$a1` at the `jal` is whatever `StyleEffect__Update`'s own caller left there, and
`StyleEffect__UpdateByKind` consumes it as `arg1`. This is the same register-forwarding
trap that `externcheck.py` was written for (round 57, `SceneNode__RaycastVertical`) —
with the difference that here it reproduces retail, so the narrow declaration
is correct for this unit and must not be widened.

**Declaration sites changed:** none (arity unchanged). `/* arity-ok: ... */`
added to `src/class_3bb8c_r.c:438`. Oracle green.

## Naming

Round 70 (alpha). `func_80056640` -> `StyleEffect__UpdateByKind`, **tier B**.

Only caller is `StyleEffect__Update`, which sits in gStyleEffectMethods's slot +0x0EC
(asm/data/76DC8.data.s, the table's last word) and increments `tick` (+0x024)
before the call. Body: owner's slotB8 (Actor__SetTranslation in gStyleEffectMethods,
i.e. set translation) with pos + offset + (D_8008ACAC's +0x018 word now -
gTrackedYSnapshot); then kind 0 -> StyleEffect__DriftModelChildren, 2 ->
NoOpIgnoreArgs, 3 -> StyleEffect__RandomizeSprites. "Update" rests on the
per-frame counter in its one caller, so B.

## Track 4 (2026-09-26, round 88, charlie)

class_3bb8c_s.c's `LinkNode` view (owner and children under one type) is gone: the owner is `StyleEffect` (include/StyleEffect.h), `modelChildren` are `Actor *`, `sprites` are `VariantSprite *`, and the local `Vec3S` is `LongVec3`. Accessor renames: `kind` is Actor's `pendingExtra` (+0x054, where the ctor stores it); `offset`/`rotation`/`scale`/`modelChildLayout`/`tableIndex`/`color`/`altColor` are `params.*`; slot `slotB8` is Actor's `setTranslation` on the owner and model children and Sprite's `setColor` on sprites; `*coord2 = 0` is `coord2->flg = 0`. Image byte-identical.
