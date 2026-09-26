# Entity__MoodCue34 -- MATCHED (59/59 words)

> Renamed from `func_8005FB6C` on 2026-09-24 (tools/rename.py). Address 0x8005fb6c.

Unit: `Entity_c`. Runner: bravo.

## Shape

```c
void Entity__MoodCue34(Entity *this) {
    EntityMethods *methods;
    s32 arg1;

    if ((u32)(this->unkFC - 0x190) < 0xA) {
        this->methods->slot44(this, 0, ROTATION_YAW_PLUS9);
    } else if ((u32)(this->unkFC - 0x2BC) < 0xA) {
        this->methods->slot44(this, 0, ROTATION_YAW_MINUS9);
    } else if ((u32)(this->unkFC - 0x33E) < 0x4) {
        this->methods->slot44(this, 0, ROTATION_YAW_MINUS9);
    } else if (this->unkFC >= 0x353) {
        this->methods->slot160(this);
    }
    methods = this->methods;
    arg1 = -0x200;
    if (this->unkFC < 0x320) {
        arg1 = -0x3C;
    }
    methods->slotC4(this, arg1, 1);
}
```

## Notes -- three separate residues, each closed independently

1. **`this->unkFC` must NOT be cached in a local across the whole
   function.** First attempt cached it as `s32 fc = this->unkFC;` at the
   top. Retail never does this -- it re-`lw`s `0xFC($s0)` fresh at every use
   (entry, the merge point, and the final range test) and keeps NO value
   live across the vtable calls in between. Caching it forced GCC to
   promote it to a callee-saved register, growing the frame from
   retail's `-0x18`/`{$s0,$ra}` to an oversized one with an extra `$s1` --
   scored 2/59. Removing the local and reading `this->unkFC` fresh at each
   site fixed the frame immediately (43/59).

2. **The final `slotC4` call needs the "default value, then conditionally
   overwritten" idiom for its argument, AND an explicit re-fetch of
   `this->methods` into its own local right before the call.** Retail's
   merge-point tail reloads BOTH `this->unkFC` (`0xFC($s0)`) and
   `this->methods` (`0x0($s0)`) into two different registers immediately
   after the three-way dispatch, then uses the SECOND one (`$v1`, the
   methods pointer) for the `slotC4` call while computing the argument via
   default-then-override on `$a1`. Writing the call as a bare
   `this->methods->slotC4(this, arg1, 1);` right there makes GCC fetch
   `this->methods` fresh AT the call site instead of at the merge point
   proper -- same net effect semantically, but ONE INSTRUCTION short
   relative to retail (which does the methods fetch earlier, separately
   from the call's own address computation). Introducing
   `EntityMethods *methods = this->methods;` right at the merge point,
   used once for the call, reproduced retail's explicit reload and closed
   this residue (43 -> 59, full match).

3. **The two disjoint ranges that share a handler
   (`[0x2BC,0x2C6)` and `[0x33E,0x342)`, both calling
   `slot44(this, 0, ROTATION_YAW_MINUS9)`) must be written as two SEPARATE
   `else if` arms with duplicated bodies, not combined with `||`.** A
   combined `(u32)(fc-0x2BC)<0xA || (u32)(fc-0x33E)<0x4` compiled to a
   confusingly-scrambled result where the two `slot44` call sites' data
   arguments (`ROTATION_YAW_PLUS9` vs `ROTATION_YAW_MINUS9`) appeared to land at the wrong
   physical addresses in the funcdiff/asm-differ byte dump. **This turned
   out to be a complete red herring, not a real bug**: at that point the
   function was still one instruction short overall (residue #2, above),
   which shifted the WHOLE REST OF THE LINKED IMAGE (funcdiff's "differs
   outside this range" warning was firing, ~72-130KB). The apparently-wrong
   data-symbol immediates were a downstream symptom of that unrelated size
   drift, not evidence that the `||` form was wrong at the machine-code
   level (`objdump -dr` on the plain `.o` showed the correct
   `R_MIPS_HI16`/`R_MIPS_LO16` relocations against the right symbols even
   in the `||` version). Once residue #2 was fixed and the function's
   overall size matched retail exactly, the duplicated-arm form (kept from
   this step) matched byte-for-byte; the `||` form was never re-tested
   afterward and may well also have worked. Recorded here mainly as a
   warning: **do not diagnose a data-symbol/address anomaly at the
   instruction level while `differs OUTSIDE this range` is still firing at
   the tens-of-KB scale -- fix the local size mismatch first, then
   re-examine.**

## Notes on control flow

- Four-way dispatch on `this->unkFC` (three ranges plus a `>=` tail), with
  the two middle ranges sharing one handler -- the assembly's shared merge
  label (`.L8005FBD4`) confirms retail's source reaches the same call site
  from two different tests, matching the two-arm duplication above.
- `(u32)(x - LOW) < COUNT` is the established unsigned-range-check idiom
  already used in `Entity_b.c` (`Entity__MoodCue11`'s
  `(u32)(this->unkFC - 0xD5D) < 0x78`).
- Externs added: `ROTATION_YAW_PLUS9`, `ROTATION_YAW_MINUS9`.
- Clean of both open toolchain blockers.

Matched on the 4th distinct attempt (~4/30), after the local-caching miss,
the missing-methods-reload miss, and the red-herring data-symbol
investigation (which cost no additional code changes, only diagnosis time).

### Proposed learning

**A data-symbol immediate that looks wrong in a byte diff is not evidence
of a source bug when `differs OUTSIDE this range` is already firing.**
`objdump -dr` on the plain (unlinked) `.o` file shows the true relocation
target and is the fast way to rule this out: if the relocation there is
against the right symbol, the wrong-looking linked value is downstream of
an unrelated size mismatch in the SAME function (or an earlier one), not a
new bug at the site of the anomaly. Fix the size mismatch first; do not
spend attempts theorizing about symbol/table layout while it stands.

## Naming

Why `MoodCueNN`: the function's address sits in `gEntityMoodHandlerTable` row 34 (`asm/data/79528.data.s`, base 0x80089EB0, stride 0x10; row = (slot address - 0x80089EB0) / 0x10), read directly off the table (this unit's own row assignment, round 78). Tier B: the row-to-function mapping is a compiler fact, not a guess, but which dream object or mood state each row represents is not established -- the row number is kept decimal and zero-padded so the names sort in table order, same convention as Entity_b/d/e/g.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a TodActor subclass whose table and object expand `TODACTOR_SLOTS`/`TODACTOR_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
