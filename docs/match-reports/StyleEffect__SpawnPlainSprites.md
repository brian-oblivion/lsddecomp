# StyleEffect__SpawnPlainSprites -- MATCHED (10/10 words)

> Renamed from `Class876FC__SpawnPlainSprites` on 2026-09-26 (tools/rename.py). Address 0x80056e1c.

> Renamed from `LinkOwnerObj__func_56e1c` on 2026-09-26 (tools/rename.py). Address 0x80056e1c.

> Renamed from `func_80056E1C` on 2026-09-18 (tools/rename.py). Address 0x80056e1c.

Unit: `class_3bb8c_o` (round 17). A 4-argument forward to `StyleEffect__SpawnSprites`.

## Final source

```c
extern void StyleEffect__SpawnSprites(void *arg0, s32 arg1, s32 arg2, s32 arg3);

void StyleEffect__SpawnPlainSprites(void *this) {
    StyleEffect__SpawnSprites(this, 0, 0, 0);
}
```

## Derivation

Whole body:

```
addiu $a1, $zero, 0
addiu $a2, $zero, 0
jal   StyleEffect__SpawnSprites
 addiu $a3, $zero, 0
```

Nothing touches `$a0` before the call, so the caller's own first argument
is forwarded unchanged -- `StyleEffect__SpawnSprites(this, 0, 0, 0)`.
`StyleEffect__SpawnSprites` is OUTSIDE this unit's carved range (part of the still-
uncarved `class_3bb8c_k` monolithic segment immediately in front of this
slice), so no prototype for it exists anywhere yet; declared locally here
per the established "calling into a function in another/uncarved unit is
fine" convention (`DECOMPILATION_LEARNINGS.md`). `$v0` is never read at
this call site, so `void` is the conservative return type -- subject to the
usual "a discarded return is never evidence of `void`" caveat if a second
caller of `StyleEffect__SpawnSprites` turns up with a different answer. `this` is left
generic (`void *`) since nothing in this function's own body constrains it
further.

### Proposed learning

None beyond what's already documented -- a plain forwarding wrapper.

## Naming

**`StyleEffect__SpawnPlainSprites` -- tier C.** Class is known (`LinkOwnerObj`,
confirmed by its caller's dispatch context in `class_3bb8c_s.c`), but the
function is a pure forward to `StyleEffect__SpawnSprites(this, 0, 0, 0)`, a function
outside this unit's carved range with no prototype or report anywhere yet.
Three literal zero arguments carry no evidence of what they mean, so
naming this wrapper would just be naming a guess about `StyleEffect__SpawnSprites`.
Kept the tier-C `Class__func_xxxxx` form per FINISHING-PLAN track 3.

## Extern arity (round 59)

**Verdict: arity-ok idiom.** `src/class_3bb8c_s.c`'s unprototyped declaration
stays.

**Callee evidence** (`0x80056E1C`, and the definition in
`src/class_3bb8c_o.c`): the whole body is a forwarding tail call, and it
*writes* `$a1`/`$a2`/`$a3` to zero before reading anything, passing only its
incoming `$a0` through:

```
80056e24:  move  a1,zero
80056e28:  move  a2,zero
80056e2c:  jal   80056d18 <StyleEffect__SpawnSprites>
80056e30:  move  a3,zero
```

So one real argument, exactly as `void StyleEffect__SpawnPlainSprites(void *this)`
says — and unusually clear, since the second argument register is not merely
ignored but overwritten.

**Why the extern must stay unprototyped.** `StyleEffect__InitByKind`'s dispatch passes a
second argument anyway, and retail emits it:

```
80056620:  jal   80056e1c <StyleEffect__SpawnPlainSprites>
80056624:  move  a1,zero          <- the dead 2nd argument, in retail
```

Its sibling arms do the same (`jal StyleEffect__PlaceModelChildren` / `move a1,zero` at
`0x80056600`, `jal StyleEffect__BuildRandomSprites` / `move a1,zero` at `0x80056614`). The
dispatch forwards `(self, 0)` uniformly; a one-parameter prototype would break
every arm.

**Declaration sites changed:** none (arity unchanged). `/* arity-ok: ... */`
added to `src/class_3bb8c_s.c:144`. Oracle green.

## Track 4 (2026-09-26, round 88, charlie)

Renamed from the `LinkOwnerObj__` family to `StyleEffect__` with the class's
unification (`include/StyleEffect.h`). Evidence: the only caller is
StyleEffect's own per-kind dispatch in `class_3bb8c_s.c`
(`StyleEffect__InitByKind` kind 3, `StyleEffect__UpdateByKind` kind 3,
`StyleEffect__ReleaseByKind` kinds 2/3), each passing its own `self`; the
five-element array at +0x084 ("links") is `StyleEffect::sprites`, filled by
`StyleEffect__SpawnSprites` with `New_VariantSprite` objects. The old
`LinkOwnerObj`/`LinkElemObj` views were StyleEffect and VariantSprite under
another name; RandomizeSprites' `slot48` is VariantSprite's inherited
`updateScale` and its `angle` (+0x084) is `sprite.rotate` (Sprite, +0x064 +
0x020, 4096 per degree -- the `(rand() % 360) << 12` it stores).

View replaced the same day: the `LinkOwnerObj`/`LinkElemObj` views in class_3bb8c_o.c are deleted and the unit includes include/StyleEffect.h (`this` is `StyleEffect *self`; `links` is `sprites`, `slot48` is `updateScale`, `angle` is `sprite.rotate`). Image byte-identical.

## Track 7 (round 99, alpha)

A one-line comment only.
