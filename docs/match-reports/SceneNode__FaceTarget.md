# SceneNode__FaceTarget -- MATCHED (110/110 words)

> Renamed from `Class6B5CC__FaceTarget` on 2026-09-26 (tools/rename.py). Address 0x8001eacc.

> Renamed from `func_8001EACC` on 2026-09-17 (tools/rename.py). Address 0x8001eacc.

Unit: `code_d294_c` (round 14). **The function two runners independently
flagged for its argument-swap oddity** (see `docs/match-reports/
Entity__MoodCue115.md` and `Entity__MoodCue81.md` from earlier rounds, and
`docs/DECOMPILATION_LEARNINGS.md`). A "face target" orientation setter:
computes yaw/pitch from `self` toward `target` via two `ratan2` calls,
converts both to degrees, builds a 3-entry `Ratio16` table, and
dispatches it to `slot44` -- with an optional second `slot44` call
forwarding a caller-supplied table verbatim.
`void SceneNode__FaceTarget(SceneNodeObj *self, SceneNodeObj *target, s32 arg2,
s32 arg3, void *arg4)`.

## Final source

```c
void SceneNode__FaceTarget(SceneNodeObj *self, SceneNodeObj *target, s32 arg2, s32 arg3, void *arg4) {
    s32 *pos;
    s32 *table;
    s32 dx;
    s32 dz;
    Ratio16 out[3];

    pos = &self->unk14->unk18;
    table = target->unkC != 0 ? target->unk14->unk38 : 0;

    if (table[0] != pos[0]) {
        dx = table[0] - pos[0];
        dz = table[2] - pos[2];
        out[1].whole = ratan2(dx, dz);
    } else {
        dz = table[2] - pos[2];
        out[1].whole = ratan2(1, dz);
    }

    if (table[2] != pos[2]) {
        dz = table[2] - pos[2];
        dx = table[1] - pos[1];
        out[0].whole = ratan2(dz, dx);
    } else {
        dx = table[1] - pos[1];
        out[0].whole = ratan2(1, dx);
    }

    out[0].whole = (out[0].whole + 0x400) * 360 / 4096;
    out[1].whole = out[1].whole * 360 / 4096;

    out[2].whole = 0;
    out[2].frac = 1;
    out[1].frac = 1;
    out[0].frac = 1;
    if (arg2 != 0) {
        out[0].whole = 0;
    }
    if (arg3 == 0) {
        out[1].whole = out[1].whole + 0xB4;
    }

    self->methods->slot44(self, 1, out);
    if (arg4 != 0) {
        self->methods->slot44(self, 0, arg4);
    }
}
```

## THE ARGUMENT-SWAP FINDING -- resolved, not just corroborated

Two runners in earlier rounds independently hit the same oddity from the
CALLER side: `SceneNode__FaceTarget`'s first two arguments appear reversed at
some call sites (`(this, this->unk94)` at `a3==0` sites,
`(this->unk94, this)` at `a3==1` sites, confirmed across all 9 `asm` call
sites and all matched C call sites at the time). The open question,
explicitly left unresolved pending this function's own decompilation:
**why is the swap safe, and are the two parameter types actually the
same base class?**

**Now resolved from this function's own body, not asserted:**

1. **`self` and `target` are used completely symmetrically.** Both need
   only a `SceneNodeObj`-shaped object: `target->unkC` is null-checked
   (matching `SceneNodeObj::unkC`'s own existing type, `UnkOwner_d294
   *`), and `target->unk14->unk38` is read as a 3-word table (matching
   `SceneNodeObj::unk14`'s own existing type, `SceneNodeSub14 *`, and
   its `unk38` field -- the SAME field `SceneNode__LocalOffsetToWorldPos` established this
   same round). `self`'s own position comes from the identical
   `unk14->unk18` path. Nothing in this function's body prefers one
   parameter over the other structurally.
2. **`arg3` is not an arbitrary "direction selector" -- it gates a
   180-degree correction on the computed yaw** (`if (arg3 == 0) { yaw +=
   180; }`). This is the MECHANISM, not just a correlated flag: swapping
   which object is `self` vs `target` negates the computed direction
   vector (`target - self` becomes `self - target`, exactly 180 degrees
   apart for an angle). A caller passing the "natural" order (`self`,
   `target`) needs the function's own internal correction (`arg3 == 0`);
   a caller that ALREADY swapped the two objects at the call site
   (`arg3 == 1`) does not, because the swap itself already produced the
   flipped direction. The correlation two runners observed (`a3==0` <->
   natural order, `a3==1` <-> swapped order) is exactly what this
   mechanism predicts.
3. **This does NOT resolve the cross-unit question of whether `Entity`
   and `Unk94Obj` (the types `Entity_e.c`/`Entity_d.c` etc. pass at THEIR
   OWN call sites, via `Entity.h`'s own separate declaration) share a
   named common base with `SceneNodeObj`.** That question stays open --
   this function's own two parameters are typed `SceneNodeObj *` here
   because that's what THIS unit's body actually needs and what's
   locally available (matching the `+0xC`/`+0x14` layout used), per the
   project's established "per-call-site signature, not a callee
   property" precedent (same as `GetSceneNodeMethods`/`SceneNode__NoOpSlot5C`). Full
   writeup left in `include/code_d294.h`'s own comment on this function,
   so the next reader doesn't have to re-derive it.

## New extern knowledge (`include/code_d294.h`, additive)

- **`ratan2`** (Psy-Q library, `s32 ratan2(s32 dy, s32 dx)`): arctangent
  in PSX-native 4096-per-circle BAM units -- confirmed by this function's
  own subsequent `x * 360 / 4096` degree conversion, the same unit
  convention `SceneNode__GetRotationDegrees` already established for the SAME kind of
  angle field.
- `SceneNode__FaceTarget` itself gets its own local prototype in this unit's
  header (first declaration here; `Entity.h`'s separate, differently-typed
  declaration for the same external symbol is untouched, out of scope,
  and deliberately not unified -- see the header's own comment).

No existing field was retyped; `SceneNodeObj::unkC`/`unk14` and
`SceneNodeSub14::unk38` (added this round by `SceneNode__LocalOffsetToWorldPos`) are used
here exactly as already declared.

## Derivation notes

- **The `x*360/4096` degree conversion needed `/`, not `>>12`.** Writing
  it as a right-shift compiles to the SAME final `sra`, but the compiler's
  overload-by-constant-power-of-2 lowering for genuine DIVISION includes
  a `bgez`/`+4095`-if-negative rounding correction that a bare `>>`
  doesn't get for free (a plain shift rounds toward -infinity;
  division must round toward zero) -- confirmed with an isolated `cc1`
  probe (`int f(int x){return (x+1024)*360/4096;}` reproduces retail's
  exact `sll`/`addu`/`sll`/`subu`/`sll`/`bgez`/`addiu 0xFFF`/`sra`
  sequence byte-for-byte; `>>12` skips the correction entirely).
- **A SECOND, much harder residue: even with `/`, the shift opcode came
  out `srl` (unsigned) instead of retail's `sra` (signed) when the
  divided value was held in a LOCAL VARIABLE before being stored into the
  output struct field.** Isolated `cc1` probes confirmed the mechanism:
  GCC recognizes that a value ONLY EVER narrowed into a 16-bit
  destination (a local `s16`/`s32` variable used once, then assigned to
  an `s16` struct field) has dead upper bits after the shift, and freely
  substitutes `srl` for `sra` since both give the same low 16 bits in
  that case -- UNLESS the value is read again afterward in a WIDER
  context (confirmed: adding a later `int`-width use of the same local
  flips the opcode back to `sra`) OR the shift is performed DIRECTLY on
  a MEMORY location (a struct field) rather than a promoted-to-register
  local (also confirmed: `out[0].whole = (out[0].whole + 0x400) * 360 /
  4096;`, operating on the struct field in place with no intermediate
  named local, reproduces `sra` unconditionally). The final source uses
  the SECOND form -- direct in-place struct-field arithmetic -- which
  also happens to be the more literal reading of retail's own stack-slot
  reuse (`sh`/`lh` round-tripping through the SAME `sp+0x10`/`sp+0x14`
  offsets that become `out[0].whole`/`out[1].whole`, not a separate named
  local at all).
- **A THIRD residue, register/branch-polarity, on the "avoid atan2(0,0)"
  guards.** `dx = (table[0]==pos[0]) ? 1 : table[0]-pos[0];` (a ternary)
  and the equivalent `if (table[0]==pos[0]) {special} else {normal}`
  BOTH compiled to a MERGED, shorter form (sharing the trailing
  `table[2]`/`pos[2]` load between both arms) -- 9 words short of
  retail, which duplicates that load independently in EACH arm and
  shares only the final `ratan2` call (the SAME "GCC tail-merges an
  identical trailing call, but does not otherwise fuse the branches"
  shape already documented in `SceneNode__OnNotify`'s report). Restructuring
  as `if (table[0] != pos[0]) { ...normal, with its OWN duplicate
  table[2]/pos[2] load... } else { ...special... }` (note the INVERTED
  condition, `!=` not `==`) matched exactly: both the word count and the
  branch polarity (`beq`-to-the-special-case vs `bne`-to-skip-it)
  depended on writing the "normal" (subtraction) arm as the `if`'s taken
  branch and the "special" (guard) arm as the `else`, not the reverse --
  the LOGICALLY equivalent `if (== ) {special} else {normal}` reliably
  produced the opposite (wrong) branch sense in every attempt.

### Proposed learnings

1. **`x / N` (genuine division) and `x >> log2(N)` (a bare shift) are NOT
   interchangeable for this compiler when the dividend can be negative,
   even though both compute the same VALUE for a value that happens to
   be exactly representable either way -- only `/` gets the
   negative-rounding correction GCC 2.6.3 emits for signed
   division-by-power-of-2.** Prefer `/` whenever retail's disassembly
   shows a `bgez`/`+  (N-1)`-if-negative guard before the shift; a bare
   `>>` silently drops that guard with no diagnostic.
2. **A value narrowed only into a 16-bit destination can flip GCC's shift
   opcode from `sra` to `srl` (both are equally valid for the LOW 16
   bits, but retail's own choice is a fact about the SOURCE shape, not
   just the math) -- write the arithmetic directly on the destination
   memory location (a struct field, in place) rather than through an
   intermediate named local, if retail's disassembly shows a value that
   round-trips through the SAME memory offset it started at.** This is a
   new, general lever: prefer "compute in place on the field" over
   "compute in a local, then assign" whenever the local would otherwise
   be used exactly once.
3. Confirms and extends `SceneNode__OnNotify`'s "GCC tail-merges an identical
   trailing call, but does not fuse the whole branch" finding: the
   merge-vs-duplicate boundary is sensitive to which arm of an `if`/`else`
   is written as the `if` (taken-first) vs the `else` -- swapping the
   condition's polarity (and which arm holds the "normal" vs "special"
   case) can flip a compiler-optimized MERGE into retail's own
   duplicated-but-simpler shape, even when the two source forms are
   logically identical.

## Naming (round 50, charlie -- FINISHING-PLAN track 3)

- **`func_8001EACC` -> `SceneNode__FaceTarget`. Tier A.** The purpose is
  pinned end to end by the body, not inferred: `ratan2` over `target`'s
  world position minus `self`'s own coord translation gives two angles;
  both are converted to degrees by the same `* 360 / 4096` this unit's
  `SceneNode__GetRotationDegrees` uses; they are packed as a
  `Ratio16[3]` and dispatched to `slot44`, whose occupant is
  `SceneNode__UpdateRotation` (code_d294.c, matched) -- the setter that writes
  `GsCOORD2PARAM.rotate`, i.e. the object's own rotation. Compute an
  orientation from self toward a target and install it as the object's
  rotation is the whole function.
- **The argument-swap section above survives the rename and explains the
  name's one soft spot:** `self` and `target` are used symmetrically and
  `arg3 == 0` adds the half-turn that compensates a caller which already
  swapped them. "FaceTarget" therefore names the `arg3 == 1` reading of the
  parameters; every one of the ~30 `Entity_*.c` call sites passes
  `(this, this->unk94, 1, 0, 0)`, so the common case is "this faces its
  unk94", which the name states correctly.
- `arg2`/`arg3`/`arg4` are NOT renamed: `arg2` forces the pitch entry to 0
  and `arg3` selects the half-turn, but whether those are "yaw only" and
  "already swapped" as modes, or something narrower, is a reading of two
  branches, not evidence.


## Round 95 (bravo): moved from include/code_d294.h

The header's banner was rewritten as documentation in round 95; the comment it carried about this function, verbatim:

```c
/* SceneNode__FaceTarget (prototype in include/SceneNode.h; round 14, this unit -- the SAME external symbol
 * Entity_b/c/d/e.c call via their own separate `Entity.h` declaration,
 * `SceneNode__FaceTarget(Entity *this, void *arg1, s32 arg2, s32 arg3, s32
 * arg4)`; same "per-call-site signature, not a callee property"
 * precedent as `GetSceneNodeMethods`/`SceneNode__NoOpSlot5C` above -- this unit's own
 * view differs in the first two parameters' types only, see below).
 *
 * A "face target" orientation setter: computes yaw/pitch from `self` to
 * `target` via two `ratan2` calls, converts both to degrees (see
 * `SceneNode__GetRotationDegrees`'s same `x*360>>12` idiom -- pitch gets an EXTRA `+
 * 0x400` [90 degrees] added before conversion, yaw does not), builds a
 * `Ratio16[3]` {pitch, yaw, 0} table (each `.den = 1`), and
 * dispatches it to `updateRotation`. `arg2 != 0` forces the pitch entry to 0
 * (a "yaw only" mode); `arg3 == 0` adds 180 degrees to yaw (see below);
 * a non-NULL `arg4` fires a SECOND `updateRotation(self, 0, arg4)` call with the
 * caller's own table forwarded as-is.
 *
 * THE ARGUMENT-SWAP FINDING, CONFIRMED FROM THIS FUNCTION'S OWN BODY:
 * `self` and `target` (this unit's own params 1/2) are used completely
 * SYMMETRICALLY -- both need only a `SceneNode`-SHAPED object
 * (`->unkC` null-checked, `->unk14->unk38` read as a 3-word table), and
 * the position subtraction is always `target - self`. `arg3` is what
 * makes this safe to call with the roles swapped: `Entity.h`'s own
 * documented finding (all `a3==0` call sites pass `(this, this->unk94)`,
 * all `a3==1` sites pass `(this->unk94, this)`) now has a mechanism, not
 * just a correlation -- swapping which object is `self` vs `target`
 * negates the computed direction, and the function's own `+180 degrees
 * on arg3==0` step is EXACTLY the correction needed to compensate. A
 * caller that already swapped the two objects at the call site (`a3==1`)
 * skips the correction because it does not need it; a caller passing
 * them in the "natural" order (`a3==0`) gets the correction applied
 * internally. This resolves the mechanism (not just the correlation)
 * without asserting a name for whatever base type `Entity`/`Unk94Obj`/
 * `SceneNode` share -- that question stays open, per the caller-side
 * finding in Entity.h and DECOMPILATION_LEARNINGS.
 *
 * This unit's own two parameters are typed `SceneNode *` rather than
 * a shared/generic type: `self->methods` is dispatched directly (needs
 * the real vtable type), and `target`'s `->unkC`/`->unk14->unk38` shape
 * matches `SceneNode` exactly, with no evidence in this call site
 * alone for anything narrower or wider. */
```

## Round 97 (alpha): Sony's GsCOORDINATE2

SceneNodeSub14 is deleted: SceneNode.coord2 is Sony's GsCOORDINATE2 (flg; MATRIX coord, whose t is the offset from the parent; MATRIX workm, whose t is the world position; param, super, sub -- 0x50 bytes, offset for offset). Accessors here follow the compiler's list: tx/ty/tz -> coord.t[0]/[1]/[2], unk38 -> workm.t; a local that holds coord.t or workm.t is `long *` (MATRIX.t is long[3]; s32 is int); any cast to GsCOORDINATE2 * is gone. Byte-identical.

## Round 98 (echo): track 7, moved from src/code_d294_c.c

Parameters named for what their branches do, which is all the body establishes (tier B; the purpose of the modes stays open, as round 50 said): `arg2` -> `zeroPitch` (non-zero clears the pitch entry), `arg3` -> `noHalfTurn` (0 adds 180 degrees to the yaw), `arg4` -> `extraRotation` (a Ratio16[3] handed to updateRotation with set = 0, i.e. added). `table` -> `targetPos`, and the pitch half's reuse of `dx` for the y difference is now its own `dy` local: byte-identical. `4096` is `ONE`, the quarter turn `0x400` is `ONE / 4`, and the half turn `0xB4` is written `180` (degrees).

The source comment was rewritten as documentation; the one it replaced, verbatim (field names as they were then):

```c
/* Points the object at `target`: two ratan2 calls over `target`'s world
 * position minus `self`'s own coord translation give yaw and pitch, both
 * converted to degrees, packed into a {pitch, yaw, 0} ratio triple and
 * dispatched to updateRotation (SceneNode__UpdateRotation, the rotation setter that writes
 * GsCOORD2PARAM.rotate). `arg2 != 0` zeroes the pitch entry; `arg3 == 0`
 * adds 180 degrees to yaw; a non-NULL `arg4` fires a second updateRotation with the
 * caller's own table forwarded verbatim.
 *
 * `self` and `target` are used SYMMETRICALLY -- the subtraction is always
 * target minus self -- and the `arg3 == 0` half-turn is exactly the
 * correction for a caller that already swapped the two at the call site.
 * That is the mechanism behind the argument-swap correlation Entity.h
 * records; see docs/match-reports/SceneNode__FaceTarget.md. */
```
