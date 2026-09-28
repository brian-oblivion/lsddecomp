# Entity__MoodCue76 -- MATCHED (52/52 words)

> Renamed from `func_80062970` on 2026-09-24 (tools/rename.py). Address 0x80062970.

Unit: `Entity` (round 12). The hardest function in this batch: two
branches each end in a vtable dispatch through a *different* slot
(`EntityMethods::slot48` in one, `slot44` in the other) with different
table arguments, and retail's compiled code shares a single `jalr`
instruction between both branches (classic GCC crossjump/tail-merge:
identical trailing machine code gets folded into one copy, reached by a
jump from one branch and a fallthrough from the other).
`void Entity__MoodCue76(Entity *this, EntityMoodHandlerArg *out)`.

## Final source

```c
void Entity__MoodCue76(Entity *this, EntityMoodHandlerArg *out) {
    void (*fn)(Entity *self, s32 arg1, void *arg2);
    void *table;

    if (this->unkF4 != 0) {
        this->methods->slot12C(this);
        if (this->unk84 == this->unk80 - 1) {
            this->methods->slot130(this);
            fn = (void (*)(Entity *, s32, void *))this->methods->slot48;
            table = sScaleMinusSixtyFourth;
            fn(this, 0, table);
        }
    } else {
        this->methods->slot130(this);
        fn = this->methods->slot44;
        table = sRotationYawPlus9;
        fn(this, 0, table);
    }
}
```

## Derivation notes

The control flow and the two vtable dispatches were easy to read off the
disassembly directly; the difficulty was entirely in getting GCC to
regenerate retail's tail-merge.

**First attempt** wrote the natural, idiomatic version -- two separate
statement calls, `this->methods->slot48(this, 0, sScaleMinusSixtyFourth);` in one
branch and `this->methods->slot44(this, 0, sRotationYawPlus9);` in the other,
both with discarded return values. This compiled to the *correct control
flow* but **did not tail-merge**: each branch got its own `jalr $2 / nop`
pair before jumping to a shared epilogue, 2 words (8 bytes) longer than
retail. This 8-byte size mismatch then shifted every symbol after it in
the whole image (visible as a cascading `nm` address drift through
`Entity__MoodCue77` and beyond, and as near-total funcdiff mismatches in the
*following* functions in this unit even though their own C was already
correct).

**Root cause, found by direct experiment against the pinned toolchain**
(a minimal 15-line probe run through `tools/gcc263/cc1`, per CLAUDE.md's
"Escalate, do not experiment" reproducer discipline -- see that file for
the exact pipeline invocation): GCC 2.6.3's crossjump pass **will** merge
two textually-different-but-machine-identical call tails when both calls
are through function pointers of the *same, void-returning* C type, but
**will not** merge them when one of the two vtable slots involved is typed
`s32 (*)(...)` (as `EntityMethods::slot48` already is, per positive
evidence from `Entity__MoodCue17`'s tail-call elsewhere in this class -- see
`include/Entity.h`'s comment on that slot). The `s32` return apparently
makes the compiler treat the two call sites' live-out register state as
different enough to block the merge, even though this call site itself
discards the result. Confirmed by a probe that reproduced the *exact* bug
(non-merged, extra 2 words) by simply changing one slot's declared return
type from `void` to `s32` with everything else held constant, and
reproduced the *fix* by reverting to `void` -- see the probe transcript in
the round's tool history for the two side-by-side `cc1` outputs.

**The fix, without retyping the shared vtable slot** (which CLAUDE.md
flags as required-checked-globally and which `slot48` already has
positive-evidence justification against, so retyping it back to `void`
here was not an option): read both call targets through a **locally-scoped
`void`-returning function pointer variable**, with an explicit cast on the
`s32`-typed slot's assignment. Both branches now compile to a call through
the *same* function-pointer type, discarding nothing (there's nothing to
discard -- the type itself is `void`), and the crossjump merge fires again,
landing at retail's exact instruction sequence and size (52/52,
byte-identical once downstream drift from the size fix cleared).

### Proposed learning

**A vtable slot's declared return type can change whether GCC merges an
otherwise-identical call tail with another branch's call, even when the
return value is discarded at both call sites.** If a function stalls with
a same-*logic*, wrong-*size* residue where two branches each end in a
discarded-result vtable call, and the branches don't tail-merge the way
retail's does, check whether the two slots involved have *different*
declared return types (one `void`, one value-returning). If so, and
retyping the shared slot isn't safe (per the existing "check every other
caller" rule), reading the value-returning slot through a **local,
`void`-returning function-pointer variable** (with an explicit cast on
assignment) reproduces the merge without touching the shared header
declaration -- the cast is purely a call-site device, doesn't change the
struct, and doesn't risk the other-caller hazard. Verify any such
crossjump-sensitivity lead against the actual pinned `cc1` on a minimal
probe before committing to it; it is a compiler internals question with a
fast, cheap, and definitive answer available (a probe function takes under
a second to compile and inspect), not something to reason about from first
principles alone.

## Naming

Why `MoodCueNN`: the function's address sits in `gEntityMoodHandlerTable` row 76 (`asm/data/79528.data.s`, base 0x80089EB0, stride 0x10; row = (slot address - 0x80089EB0) / 0x10), confirmed by reading `disk/SLPS_015.56` directly rather than trusting address proximity (Entity/round 76 measured that row order does not track code address). Tier B: the row-to-function mapping is a compiler fact, not a guess, but which dream object or mood state each row represents is not established -- the row number is kept decimal and zero-padded so the names sort in table order, same convention as Entity.

**`sScaleMinusSixtyFourth` left unnamed this round.** s16-pair-decoded it reads (-1,64,-1,64,-1,64,8,7) -- X=Y=Z=-1/64, none of the round-number ratios (1/2, 1/1, 6/1, ...) every named `SCALE_*` table uses so far. Passed to `updateScale` through a `void (*)(Entity*,s32,void*)` function pointer rather than a direct call, so it is genuinely a scale table by construction, just not one with an evident round value to name it after.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a TodActor subclass whose table and object expand `TODACTOR_SLOTS`/`TODACTOR_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 7 (round 93, echo)

Every literal in the live body is in its base: decimal for moodTimer and cue-set ticks, TOD frames, distances, VAB programs and `state` phases (the unit has no hex literal left). `D_80089DFC` -> `sScaleMinusSixtyFourth` (`python3 tools/rename.py`, tier A, named by value like the other motion templates): three Ratio16 {-1, 64} (words 0x0040FFFF), passed to updateScale with set 0, so it subtracts 1/64 from each axis's scale (SceneNode__UpdateScale adds when `set` is 0). Its extern is in include/Entity.h's motion-template block as Ratio16[]. The body now calls updateScale and updateRotation directly: the local void function pointer the derivation above needed existed because updateScale was then typed `s32 (*)`, which blocked the crossjump; the slot is void now (SCENENODE_SLOTS) and the direct calls merge the same way, byte-identical. Byte-identical (whole image green).
