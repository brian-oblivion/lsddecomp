# Class6B5CC__FaceTarget -- MATCHED (110/110 words)

> Renamed from `func_8001EACC` on 2026-09-17 (tools/rename.py). Address 0x8001eacc.

Unit: `code_d294_c` (round 14). **The function two runners independently
flagged for its argument-swap oddity** (see `docs/match-reports/
func_80061778.md` and `func_80063144.md` from earlier rounds, and
`docs/DECOMPILATION_LEARNINGS.md`). A "face target" orientation setter:
computes yaw/pitch from `self` toward `target` via two `ratan2` calls,
converts both to degrees, builds a 3-entry `WholeFrac_d294` table, and
dispatches it to `slot44` -- with an optional second `slot44` call
forwarding a caller-supplied table verbatim.
`void Class6B5CC__FaceTarget(Class6B5CCObj *self, Class6B5CCObj *target, s32 arg2,
s32 arg3, void *arg4)`.

## Final source

```c
void Class6B5CC__FaceTarget(Class6B5CCObj *self, Class6B5CCObj *target, s32 arg2, s32 arg3, void *arg4) {
    s32 *pos;
    s32 *table;
    s32 dx;
    s32 dz;
    WholeFrac_d294 out[3];

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
CALLER side: `Class6B5CC__FaceTarget`'s first two arguments appear reversed at
some call sites (`(this, this->unk94)` at `a3==0` sites,
`(this->unk94, this)` at `a3==1` sites, confirmed across all 9 `asm` call
sites and all matched C call sites at the time). The open question,
explicitly left unresolved pending this function's own decompilation:
**why is the swap safe, and are the two parameter types actually the
same base class?**

**Now resolved from this function's own body, not asserted:**

1. **`self` and `target` are used completely symmetrically.** Both need
   only a `Class6B5CCObj`-shaped object: `target->unkC` is null-checked
   (matching `Class6B5CCObj::unkC`'s own existing type, `UnkOwner_d294
   *`), and `target->unk14->unk38` is read as a 3-word table (matching
   `Class6B5CCObj::unk14`'s own existing type, `Class6B5CCSub14 *`, and
   its `unk38` field -- the SAME field `Class6B5CC__LocalOffsetToWorldPos` established this
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
   named common base with `Class6B5CCObj`.** That question stays open --
   this function's own two parameters are typed `Class6B5CCObj *` here
   because that's what THIS unit's body actually needs and what's
   locally available (matching the `+0xC`/`+0x14` layout used), per the
   project's established "per-call-site signature, not a callee
   property" precedent (same as `GetClass6B5CCMethods`/`func_8001D33C`). Full
   writeup left in `include/code_d294.h`'s own comment on this function,
   so the next reader doesn't have to re-derive it.

## New extern knowledge (`include/code_d294.h`, additive)

- **`ratan2`** (Psy-Q library, `s32 ratan2(s32 dy, s32 dx)`): arctangent
  in PSX-native 4096-per-circle BAM units -- confirmed by this function's
  own subsequent `x * 360 / 4096` degree conversion, the same unit
  convention `Class6B5CC__GetRotationDegrees` already established for the SAME kind of
  angle field.
- `Class6B5CC__FaceTarget` itself gets its own local prototype in this unit's
  header (first declaration here; `Entity.h`'s separate, differently-typed
  declaration for the same external symbol is untouched, out of scope,
  and deliberately not unified -- see the header's own comment).

No existing field was retyped; `Class6B5CCObj::unkC`/`unk14` and
`Class6B5CCSub14::unk38` (added this round by `Class6B5CC__LocalOffsetToWorldPos`) are used
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
  shape already documented in `Class6B5CC__OnNotify`'s report). Restructuring
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
3. Confirms and extends `Class6B5CC__OnNotify`'s "GCC tail-merges an identical
   trailing call, but does not fuse the whole branch" finding: the
   merge-vs-duplicate boundary is sensitive to which arm of an `if`/`else`
   is written as the `if` (taken-first) vs the `else` -- swapping the
   condition's polarity (and which arm holds the "normal" vs "special"
   case) can flip a compiler-optimized MERGE into retail's own
   duplicated-but-simpler shape, even when the two source forms are
   logically identical.

## Naming (round 50, charlie -- FINISHING-PLAN track 3)

- **`func_8001EACC` -> `Class6B5CC__FaceTarget`. Tier A.** The purpose is
  pinned end to end by the body, not inferred: `ratan2` over `target`'s
  world position minus `self`'s own coord translation gives two angles;
  both are converted to degrees by the same `* 360 / 4096` this unit's
  `Class6B5CC__GetRotationDegrees` uses; they are packed as a
  `WholeFrac_d294[3]` and dispatched to `slot44`, whose occupant is
  `func_8001CEB4` (code_d294.c, matched) -- the setter that writes
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
