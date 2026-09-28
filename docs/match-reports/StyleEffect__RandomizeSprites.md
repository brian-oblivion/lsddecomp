# StyleEffect__RandomizeSprites -- MATCHED (57/57 words)

> Renamed from `Class876FC__RandomizeSprites` on 2026-09-26 (tools/rename.py). Address 0x80056e44.

> Renamed from `LinkOwnerObj__RandomizeLinks` on 2026-09-26 (tools/rename.py). Address 0x80056e44.

> Renamed from `func_80056E44` on 2026-09-18 (tools/rename.py). Address 0x80056e44.

Unit: `class_3bb8c_k` (round 17). Walks 4 elements of the 5-element
`arr84` array (indices 1..4), and for each one calls its own vtable
`slot48` with a random entry from a 6-element global table, then sets a
random "angle" field.

## Final source

```c
extern s32 rand(void);
extern Vec3O gStyleEffectJitterScales[];

void StyleEffect__RandomizeSprites(LinkOwnerObj *this) {
    LinkElemObj **p = &this->arr84[1];
    s32 i;

    for (i = 0; i < 4; i++, p++) {
        u32 r = rand();

        (*p)->methods->slot48(*p, 1, &gStyleEffectJitterScales[r % 6]);
        (*p)->unk84 = (rand() % 360) << 12;
    }
}
```

## Derivation

- **Incrementing pointer, not array indexing.** `this->arr84[i]` inside the
  loop (with `i` walking 1..4) compiles to a base+index decomposition
  (`addiu s1,a0,4` then `lw s0,0x84(s1)`) that costs 3 extra words versus
  retail's single `addiu $s0,$a0,0x88` at the top of the loop plus a plain
  `addiu $s0,$s0,4` at the bottom. Matches the documented "walk an array
  with an incrementing pointer" idiom -- `LinkElemObj **p = &this->arr84[1]`
  with `p++` in the loop's increment clause.
- **No named `elem` local.** Caching `*p` into a named `LinkElemObj *elem`
  local costs an extra `move` (retail re-derefs `*p` fresh into `$a0`
  wherever the element pointer is needed -- as the vtable-dispatch `self`,
  as the `slot48` call's first argument, and again via a fresh `lw` for the
  final `unk84` store). Writing `(*p)->...` and `*p` directly, instead of
  through a local, reproduces this.
- **`rand() % 6` (unsigned semantics) and `rand() % 360` (signed
  semantics), not hand-transcribed magic-multiply sequences** -- per
  CLAUDE.md's "`x % N` for compile-time-constant `N`: just write `%`."
  Confirmed both directions with the reproducer script (see Notes below):
  the first `%6` compiles UNSIGNED (`multu`/`mfhi`/`srl`, no sign-fix) only
  because the intermediate result of the modulo is stored into a variable
  the compiler can prove non-negative here (an implicit consequence of
  `rand()`'s int return combined with the `u32` index expression
  `gStyleEffectJitterScales[r % 6]` requiring an unsigned index) -- written as
  `u32 r = rand(); ... r % 6 ...`, not `(unsigned)rand() % 6` inline, to
  land the intermediate in the right register lifetime. The second `%360`
  is a genuinely signed division (retail's sign-fix `sra`/`subu` chain is
  present), so it stays `rand() % 360` on the plain `s32`-returning
  `rand()` with no cast.
- **`gStyleEffectJitterScales` is a 6-entry, 12-byte-stride rodata table**
  (`asm/data/76DC8.data.s`, `0x8008788C`..`0x800878D0`, 18 words = 6 * 3),
  confirmed from the disassembly directly rather than guessed; declared
  `extern Vec3O gStyleEffectJitterScales[];`. `LinkElemMethods::slot48`'s third argument
  is a pointer into this table.
- **`(rand() % 360) << 12`** is a plain degrees -> Q19.12-ish fixed-point
  conversion; no idiom needed beyond writing the arithmetic directly.

### Proposed learning

- **A base+index decomposition costing extra words on an
  interior-array-slice loop is the incrementing-pointer idiom's fingerprint
  again**, this time triggered by starting the walk at index 1 of a
  5-element array rather than index 0 -- the existing "walk with an
  incrementing pointer" entry in `DECOMPILATION_LEARNINGS.md` doesn't call
  out that a non-zero starting index makes this MORE likely to bite (array
  indexing with a variable base offset gives the compiler more freedom to
  split the address computation), not less.

## Naming

**`StyleEffect__RandomizeSprites` -- tier A.** Mechanics ARE the purpose:
for each of 4 link elements, calls the element's own `slot48` with a
random entry from a 6-entry `Vec3O` table and sets the element's own
`angle` field to a random degrees value -- "randomize" describes exactly
this and nothing more speculative.

## Global naming

**`gStyleEffectJitterScales` (was `D_8008788C`) -- tier B.** A rand()-indexed
table of 6 `Vec3O`-shaped rodata entries, used only by this unit, passed to
each link element's own `slot48`. Named for its structure and access
pattern (a global table of `Vec3` entries feeding `LinkElemObj`), not for
a guessed game purpose (what `slot48`'s occupant does with the vector is
not established).

## Extern arity (round 59)

**Verdict: arity-ok idiom.** `src/class_3bb8c_k.c`'s unprototyped declaration
stays.

**Callee evidence** (`0x80056E44`, and the definition in
`src/class_3bb8c_k.c`): entry is `addiu s0,a0,136` and `$a1` is never read —
one real argument, exactly as `void StyleEffect__RandomizeSprites(LinkOwnerObj *this)`
says.

**Why the extern must stay unprototyped.** `StyleEffect__UpdateByKind`'s dispatch passes a
second argument, and retail emits it:

```
800566f8:  jal   80056e44 <StyleEffect__RandomizeSprites>
800566fc:  move  a1,s1            <- the dead 2nd argument, in retail
```

`s1` is `StyleEffect__UpdateByKind`'s own `arg1`. Its two sibling arms in the same switch
do the same thing (`jal StyleEffect__DriftModelChildren` / `move a1,s1` at `0x800566D8`,
`jal NoOpIgnoreArgs` / `move a1,s1` at `0x800566EC`), so the whole dispatch
forwards `(self, arg1)` uniformly regardless of what each target reads. A
one-parameter prototype here would make every arm a `too many arguments`
error, and dropping the argument would delete `move a1,s1`.

**Declaration sites changed:** none (arity unchanged). `/* arity-ok: ... */`
added to `src/class_3bb8c_k.c:145`. Oracle green.

## Track 4 (2026-09-26, round 88, charlie)

Renamed from the `LinkOwnerObj__` family to `StyleEffect__` with the class's
unification (`include/StyleEffect.h`). Evidence: the only caller is
StyleEffect's own per-kind dispatch in `class_3bb8c_k.c`
(`StyleEffect__InitByKind` kind 3, `StyleEffect__UpdateByKind` kind 3,
`StyleEffect__ReleaseByKind` kinds 2/3), each passing its own `self`; the
five-element array at +0x084 ("links") is `StyleEffect::sprites`, filled by
`StyleEffect__SpawnSprites` with `New_VariantSprite` objects. The old
`LinkOwnerObj`/`LinkElemObj` views were StyleEffect and VariantSprite under
another name; RandomizeSprites' `slot48` is VariantSprite's inherited
`updateScale` and its `angle` (+0x084) is `sprite.rotate` (Sprite, +0x064 +
0x020, 4096 per degree -- the `(rand() % 360) << 12` it stores).

View replaced the same day: the `LinkOwnerObj`/`LinkElemObj` views in class_3bb8c_k.c are deleted and the unit includes include/StyleEffect.h (`this` is `StyleEffect *self`; `links` is `sprites`, `slot48` is `updateScale`, `angle` is `sprite.rotate`). Image byte-identical.

## Track 7 (round 99, alpha)

`python3 tools/rename.py gLinkElemVec3Table gStyleEffectJitterScales`, **tier A**: six entries of three Ratio16 (the old local view was an s32 Vec3O, same 12-byte stride): {1/16, 7/1, 1/1}, {7/1, 1/16, 1/1}, then the same with 3 and with 2, so each one shapes a sprite into a thin streak along y or along x; this function, its only accessor, hands a rand()-picked one to each jittering sprite's updateScale (VariantSprite__UpdateScale reads x and y). It is declared `Ratio16 [6][3]` now and the index bound is ARRAY_COUNT of it; the loop bound is ARRAY_COUNT(self->sprites) - 1 (sprites[1..4]); the rotation is `(rand() % 360) * ONE` (GsSPRITE.rotate is 4096ths of a degree; the same sll 12). rand() comes from Sony's <rand.h>. Locals p/r -> sprite/pick.
