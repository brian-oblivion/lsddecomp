# StyleEffect__StyleEffect -- MATCHED (41/41 words)

> Renamed from `Class876FC__Class876FC` on 2026-09-26 (tools/rename.py). Address 0x800563c0.

> Renamed from `func_800563C0` on 2026-09-23 (tools/rename.py). Address 0x800563c0.

Unit: `class_3bb8c_r` (round 17 continuation). `StyleEffectMethods::ctor`
(vtable offset `+0x008` of `gStyleEffectMethods`) -- chains to the shared base
class's own ctor (`GetActorMethods()->ctor`, the SAME shared-base getter
`class_3bb8c_o.c` already used for its own `Actor__Actor`/
`New_Actor` last pass), installs this class's own vtable, sets two
fields, dispatches its own `slot40`, and tail-calls `StyleEffect__InitByKind` for
its return value's side effect only.

## Final source

```c
void *StyleEffect__StyleEffect(StyleEffect *self, void *arg1, void *arg2, void *arg3, void *arg4) {
    if (GetActorMethods()->ctor(self) == NULL) {
        goto fail;
    }
    self->methods = GetStyleEffectMethods();
    self->unk44 = 0;
    self->unk54 = arg1;
    self->methods->slot40(self, arg2);
    StyleEffect__InitByKind(self, arg3, arg4);
    return self;
fail:
    return NULL;
}
```

## Derivation

- **`GetActorMethods()`, not `GetSceneNodeMethods()`.** Both are fixed-table
  getters `class_3bb8c_o.c` already resolved last pass for the SAME
  shared intermediate base class, but they are DIFFERENT symbols with
  DIFFERENT call sites in that unit (`GetSceneNodeMethods` for the ctor CHAIN
  inside `Actor__Actor`; `GetActorMethods` for the plain-allocator
  `New_Actor`'s own ctor dispatch). This function's own disassembly
  calls `GetActorMethods`, confirmed directly rather than assumed from
  surface similarity to last pass's ctor.
- **`goto fail; ... fail: return NULL;`, not `if (cond) return NULL;`.**
  Same lever as `class_3bb8c_o.c`'s own `Actor__Actor` (documented
  there): with a plain `if`/`return NULL`, the return-`self` path needs
  its own explicit `j` to reach the shared epilogue, costing one word.
  `goto` collapses both exits onto ONE epilogue.
- **The tail call to `StyleEffect__InitByKind`'s return value is DISCARDED, not
  forwarded.** The first attempt wrote `return StyleEffect__InitByKind(self, arg3,
  arg4);`, which is the "byte match tells you nothing about return type"
  trap's INVERSE mistake -- it assumed the tail call's own return
  propagates, but retail actually issues the call, then explicitly does
  `move v0,s0` (an extra instruction) to force the return value back to
  `self` regardless of what `StyleEffect__InitByKind` returned. Caught immediately
  by a real word-count mismatch (28/41 with `return StyleEffect__InitByKind(...)`,
  worse yet with the wrong exit-value idiom on top) -- the fix was two
  statements, `StyleEffect__InitByKind(self, arg3, arg4); return self;`, not one.
- `GetStyleEffectMethods` (installing this class's own vtable) and
  `StyleEffect__InitByKind` (this class's own post-init hook, still `INCLUDE_ASM`
  outside this unit's range) are both declared as local externs, per the
  established "calling into a function in another/uncarved unit is fine"
  convention.

### Proposed learning

- **A tail call whose value is visibly DISCARDED (an explicit `move
  v0,<other>` after the call, before the shared epilogue) is the mirror
  image of the "byte match tells you nothing about return type" trap.**
  Don't assume the LAST call in a function is a tail call whose return
  propagates just because nothing else follows it in program order --
  check whether retail's own bytes overwrite `$v0` again afterward. Here
  it cost one build/diff iteration to notice; a direct read of the
  disassembly's trailing instructions (past the `jal`) would have caught
  it without spending the attempt.

## Naming

**Tier A.** `StyleEffect` is the class name class_3bb8c_s.c already uses for `gStyleEffectMethods` (its own comment: "`StyleEffect` here; class_3bb8c_r.c's `Obj876FC`" -- confirmed via `tools/classtable.py 0x800876FC`, whose slot list mixes `SceneNode__`/`BaseObjO__`/`DreamSys__`-prefixed inherited slots with this unit's own `+0x008`/`+0x00C`/`+0x040`/`+0x0EC`). Ctor naming follows the `Class__Class` convention already used for `Actor__Actor`/`SceneNode__SceneNode`. The body is a constructor by construction (chains the shared base ctor, installs the vtable, dispatches init) -- purpose evident from the body.

## Track 4 (2026-09-26, round 88, charlie)

Retyped with the class's unification (include/StyleEffect.h): same parameters as New_StyleEffect. The two stores the old view called `unk44` and `kind` are Actor's `state` (+0x044) and `pendingExtra` (+0x054): the ctor keeps `kind` in pendingExtra, and the class overrides pendingExtra's setter slot (+0x0EC) with its update, so nothing else writes it. The old own slot `setParams` at +0x040 is SceneNode's `reset`; the call casts to StyleEffectSetParamsFn (no code). Image byte-identical.

## Track 6 (round 93, bravo)

The class `Class876FC` -> `StyleEffect` (`python3 tools/renametype.py
Class876FC StyleEffect`, with its table, getter, allocator, 16 methods, the
params block `StyleEffectParams` and the two cast types
`StyleEffectSetParamsFn`/`StyleEffectUpdateFn`). **Tier B.** Evidence from its
methods: every kind attaches under the parent at pos + params.offset
(InitByKind) and every frame moves back to pos + offset plus the viewpoint's
y change since it was built (UpdateByKind); what it carries is chosen by
`kind` (a model, the model with two copies in a row, or five sprites). Its
only creator is the style layer's effect-slot code (StyleFillEffectKind0..3
into gStyleEffectSlots), whose vocabulary the name reuses. What an effect is
in the game is not shown. The kinds became `enum StyleEffectKind` in
include/StyleEffect.h (MODEL_ROW, MODEL, SPRITES, JITTER_SPRITES), from what
each switch arm in class_3bb8c_s.c does; the switches still spell numbers.

The header banner was rewritten as documentation (what it is, who builds
it, lifecycle by slot); `params.rotation`/`params.scale` went from `void *`
to `Ratio16 *` (updateRotation/updateScale take Ratio16[3]), no accessor
changed. class_3bb8c_s.c's banner lost its history lines (unified round 88,
named round 70, last match round 75), all already in the per-function
reports.

renametype.py rewrote the old class name inside this report's earlier prose too (known, pending an operator decision); those lines are history and were not hand-restored, so read `StyleEffect` in them as `Class876FC`.
