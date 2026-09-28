# Entity__MoodCue23

> Renamed from `func_8005F1D4` on 2026-09-24 (tools/rename.py). Address 0x8005f1d4.

**Unit:** Entity · **Size:** 101 words · **Status:** MATCHED (101/101 words,
whole-image build verified byte-exact)

## What it does

`(Entity *this) -> void`. Not a mood handler. A five-way dispatch on
`unkF4`/`unkFC`, each arm calling `slotBC` (and sometimes `slotC4`) with a
different rodata pointer / literal:

```c
void Entity__MoodCue23(Entity *this) {
    s32 arg1;

    SceneNode__FaceTarget(this, this->unk94, 1, 0, 0);

    if (this->unkF4 != 0) {
        if (this->unkFC >= 0x41) {
            this->unkFC = 0;
        }
        if (this->unkFC >= 7) {
            this->methods->slotBC(this, TRANSLATE_Y_MINUS64);
            this->methods->slotC4(this, 0xA, 0);
        } else {
            this->methods->slotBC(this, TRANSLATE_X_MINUS64);
        }
    } else if (this->unkFC == 0) {
        this->methods->slotBC(this, TRANSLATE_Y_MINUS4096);
    } else if (this->unkFC < 0x41) {
        this->methods->slotBC(this, TRANSLATE_Y_PLUS64);
    } else if (this->unkFC < 0x47) {
        this->methods->slotBC(this, TRANSLATE_Y_MINUS64);
        this->methods->slotC4(this, -0x1E, 0);
    } else {
        EntityMethods *methods = this->methods;

        if (this->unkFC < 0x100) {
            arg1 = -this->unkFC - 0x41;
        } else {
            arg1 = 0xFF;
        }
        methods->slotC4(this, arg1, 0);
    }
}
```

`TRANSLATE_Y_MINUS64`/`TRANSLATE_X_MINUS64`/`TRANSLATE_Y_MINUS4096`/`TRANSLATE_Y_PLUS64` are new rodata pointers
(`TRANSLATE_Y_MINUS64` used twice, by two different arms), extern-declared alongside
this unit's other `D_80089*` constants; none yet dereferenced by any carved
code.

## Attempt log (3 attempts)

1. **First attempt (29/101, address drift) -- branch sense inverted on the
   `unkFC >= 7` vs `< 7` split.** Wrote `if (this->unkFC < 7) { small } else
   { big }` (small arm first, matching how it read naturally off the
   disassembly labels). GCC compiled the small arm as the fallthrough and
   the big arm as the jump target -- the opposite of retail, which computes
   the identical `slti $v0,$v0,7` but uses `bnez` to jump FORWARD into the
   small arm, leaving the big arm as fallthrough. Every word from that
   branch onward drifted.
2. **Second attempt (31/101, still drifted) -- swapped the branch to `if
   (this->unkFC >= 7) { big } else { small }`,** matching retail's
   fallthrough/jump-target assignment (confirmed: the `bnez`/`beqz` sense
   now agreed at every branch in the diff). Branch TARGETS now lined up
   correctly, but the function was still 4 words short: two of the three
   `this->methods->slotC4(this, K, 0)` call sites (the `unkFC >= 7` arm and
   the `0x41 <= unkFC < 0x47` arm) had their `this->methods` pointer chase
   (`lw v0,0(a0)` then `lw v0,0xc4(v0)`) fully absorbed into the shared
   jump-target tail, rather than each arm keeping its own `lw v0,0xc4(v0)`
   fetch and jumping only into a 3-word shared suffix (`nop; jalr; a2=0`)
   the way retail's disassembly shows. Writing the same call expression
   verbatim in all three arms let GCC's cross-jump pass merge more of the
   suffix than retail's compile did.
3. **Third attempt (101/101) -- cached `this->methods` into a local
   `EntityMethods *methods` at the top of the THIRD (`else`, unbounded)
   arm only, and called through that local instead of re-deriving
   `this->methods` right before the call.** This reproduces retail's own
   asymmetry: that arm's disassembly loads `this->methods` into `$a2` at
   the very top of the block (before the `arg1` range check), not adjacent
   to the call the way the other two arms do. Caching it broke the
   byte-for-byte identity between this arm's call-site code and the other
   two arms', which stopped the over-merge and restored the 3-word shared
   tail retail has.

## Proposed learning

**Three call sites reaching the same virtual call with a literal argument
are not always merged to the same degree, even when written identically.**
Retail's disassembly here shares only the trailing 3 words (`nop; jalr;
a2=0`) across three `this->methods->slotC4(this, K, 0)` sites, while an
identically-worded three-arm source let GCC's cross-jump pass merge a
5-word suffix (the `this->methods` pointer chase included) instead. The
lever that recovers the narrower merge: if ONE of the arms' disassembly
shows the base pointer (here, `this->methods`) loaded earlier/differently
than the others (a register that isn't freshly re-derived right next to the
call), cache it into a local in that ONE arm only. This breaks the
byte-identity between call sites that the cross-jump pass needs to fold
them together, without touching the other arms' source at all. This is the
same GCC 2.6.3 cross-jump/tail-merge mechanism behind the existing "write
the literal jump graph with `goto`" and `Entity__MoodCue25` learnings, but from
a new direction: those entries are about triggering or avoiding merges by
choosing WHERE a call is written; this one is about breaking an
over-eager merge by choosing WHICH arm gets an early-cached pointer.

## Naming

Why `MoodCueNN`: the function's address sits in `gEntityMoodHandlerTable` row 23 (`asm/data/79528.data.s`, base 0x80089EB0, stride 0x10; row = (slot address - 0x80089EB0) / 0x10), read directly off the table (this unit's own row assignment, round 78). Tier B: the row-to-function mapping is a compiler fact, not a guess, but which dream object or mood state each row represents is not established -- the row number is kept decimal and zero-padded so the names sort in table order, same convention as Entity/d/e/g.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a TodActor subclass whose table and object expand `TODACTOR_SLOTS`/`TODACTOR_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
