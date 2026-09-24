# Entity__UpdateTargetProximity

> Renamed from `func_8005DE18` on 2026-09-23 (tools/rename.py). Address 0x8005de18.

**Unit:** Entity_b · **Size:** 50 words · **Status:** MATCHED (50/50 words,
whole-image build verified byte-exact)

## What it does

Occupies `EntityMethods` slot `+0x178` (dispatched by `Entity__Update` in
`Entity.c` as `this->methods->slot178(this)`). Looks up this entity's mood row
(`gEntityMoodTable[this->moodIndex]`), and, gated on `this->unkF0 != 0`:

- if `this->unkF4 == 0`: computes a distance argument from `row->unk6`
  (absolute value), calls `Entity__IsNearTarget(this, &this->unk14->x, dist,
  row->unk9)`, and if that returns non-zero, dispatches
  `this->methods->slot164(this, 1)`.
- independently (not `else`), if `row->unk6 < 0`: calls
  `Class6B5CC__FaceTarget(this, this->unk94, 1, 0, 0)` (still uncarved, `code_d294.s`).

Returns `this->unkF4` regardless of whether the `if (this->unkF0 != 0)` body
ran — the one known caller (`Entity__Update`) discards the result, but that is
never evidence of `void` (see CLAUDE.md); the disassembly's own final
`lw $v0, 0xf4($s0)` right before the epilogue settles it.

## Final C

```c
s32 Entity__UpdateTargetProximity(Entity *this) {
    EntityMoodRow *row;
    s32 *xptr;
    s32 dist;

    row = &gEntityMoodTable[this->moodIndex];
    if (this->unkF0 != 0) {
        if (this->unkF4 == 0) {
            xptr = &this->unk14->x;
            dist = row->unk6;
            if (dist < 0) {
                dist = ~dist + 1;
            }
            if (Entity__IsNearTarget(this, xptr, dist, row->unk9) != 0) {
                this->methods->slot164(this, 1);
            }
        }
        if (row->unk6 < 0) {
            Class6B5CC__FaceTarget(this, this->unk94, 1, 0, 0);
        }
    }
    return this->unkF4;
}
```

## Attempt log

This was the first function tackled this round, and it (plus its immediate
sibling `Entity__UpdateSoundCueStart`) surfaced three reusable residues:

1. **`row` must be computed OUTSIDE the `if (this->unkF0 != 0)` guard.**
   Retail computes `&gEntityMoodTable[this->moodIndex]` unconditionally, right at
   the top of the function, and lets the guarding `beqz` skip over its
   (harmless, dead) use in the delay slot. Nesting the computation inside the
   `if` instead made GCC schedule it AFTER the branch, shifting every
   following instruction by one word and cascading a huge (~125 KB)
   whole-image address drift onto everything after this function.
2. **`Entity__IsNearTarget`'s `s8` parameter types were wrong** (an inherited
   assumption from Entity.c's own call sites, which only ever pass literal
   byte-range values). Passing an already-computed `s32 dist` (post-negation)
   into a parameter declared `s8` forced a spurious `sll $24/sra $24`
   byte-truncate that retail does not have — because the callee's own body
   (dividing `0x800` by the argument, shifting by 11) treats it as a full
   word with no narrowing on entry. Retyping `Entity__IsNearTarget`'s `arg2`/`arg3`
   to `s32` in `include/Entity.h` fixed this with no regression to Entity.c's
   already-matched caller (a `lb`-loaded `s8` value promotes to `s32`
   identically either way).
3. **Negation idiom: `~x + 1`, not `-x`.** `dist = -dist;` compiles to a
   single `negu` (`subu $rd,$zero,$rs`); retail's bytes are the two-instruction
   `nor $v0,$zero,$a2` / `addiu $v0,$v0,1` sequence. Already known from
   `Entity__DistanceToRegion` in Entity.c, confirmed again here.
4. **The position pointer (`&this->unk14->x`) needs a dedicated local,
   computed BEFORE the sign-check on `dist`, evaluated in that order.**
   Retail loads `this->unk14` and adds the `+0x18` field offset back-to-back,
   right when entering the `this->unkF4 == 0` block, before even reading
   `row->unk6`. Writing `&this->unk14->x` inline as the call argument let GCC
   defer computing it until the `jal` itself (splitting the load and the
   `+0x18` add across the whole negation sequence); assigning it to an
   explicit `s32 *xptr` local in statement order right after entering the
   block reproduced retail's early, back-to-back scheduling exactly.

## Proposed learning

**A parameter type inferred only from a callee's KNOWN call sites can still be
wrong, and the callee's own body is the tiebreaker.** `Entity__IsNearTarget`'s `s8`
typing looked reasonable from Entity.c's literal-byte call sites, but its own
disassembly (dividing/shifting the argument as a full word, no narrowing
instructions on entry) shows the real parameter is `s32`; the previous typing
only survived because no caller so far had passed a WIDER computed value that
would expose the forced truncation.

**"Compute the pointer eagerly into its own local, in source order" is a
distinct, repeatable lever from "compute the value eagerly."** This is the
same family as `Pad__DispatchEvents`'s hoisted-invariant lesson in
`docs/DECOMPILATION_LEARNINGS.md`, but for an ADDRESS-of expression rather
than a loop-hoisted value: writing `&this->unk14->x` inline at the call site
let the compiler treat the add-immediate as free to schedule anywhere before
the call, while assigning it to a named local pinned it to the point of
assignment.

## Naming

`Entity__UpdateTargetProximity` -- tier B (round 71, runner echo, FINISHING-PLAN track 3). Renamed from `func_8005DE18`.

ENTITY_METHODS +0x178 (`tools/classtable.py ENTITY_METHODS`); its one caller is `Entity__Update`, every tick. While `active`, and only while `unkF4` is still 0, it calls `Entity__IsNearTarget` with `|row->proximityRange|` and `row->unk9`; on a hit it calls slot +0x164 (occupant `Entity__SetTargetReached`) with 1, which notifies parents with 9 and latches `unkF4`. A negative `proximityRange` also makes the entity face its target (`Class6B5CC__FaceTarget`) every active tick. It returns `unkF4`. Tier B: what the code does is clear, what the `unkF4` latch means in the game is not.

Also renamed here: `EntityMoodRow::unk6` -> `proximityRange` (compiler-checked: this function is its only accessor in the default and NON_MATCHING builds).

## Proposed field names

Cross-unit (the compiler lists accessors outside Entity_b), so these are proposals only:

| member | proposed | tier | evidence |
| --- | --- | --- | --- |
| `EntityMethods::slot164` (+0x164) | `setUnkF4` | B | occupant `Entity__SetTargetReached` (Entity.c); accessors are Entity.c's Entity__Deactivate and this function |
| `EntityMethods::slot178` (+0x178) | `updateTargetProximity` | B | occupant is this function; only accessor is Entity__Update (Entity.c) |
| `Entity::unkF4` (+0xF4) | `targetReached` | B | latched to 1 by this function (via setUnkF4, which also notifies parents with 9) once the target is within proximityRange; cleared only by Entity__Deactivate (setUnkF4(0)); returned by this slot |
