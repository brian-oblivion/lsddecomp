# Entity__MoodCue25

> Renamed from `func_8005F454` on 2026-09-24 (tools/rename.py). Address 0x8005f454.

**Unit:** Entity_c · **Size:** 60 words · **Status:** MATCHED (60/60 words,
whole-image build verified byte-exact)

## What it does

`(Entity *this, EntityMoodHandlerArg *out) -> void`. A three-way
`unkFC`-banded mood handler that ends every arm with its own
`this->methods->slotCC(this, K, 0)` call rather than falling into one shared
call site:

```c
void Entity__MoodCue25(Entity *this, EntityMoodHandlerArg *out) {
    out->unk10 = 0;
    if (this->unkFC < 0x64) {
        if (out->unk4 % 3 == 0) {
            out->unk1C = 0x16;
            out->unk20 = 1;
        }
        this->methods->slotCC(this, -0x40, 0);
    } else if (this->unkFC < 0x12C) {
        out->unk1C = 0xC;
        out->unk20 = -1;
        out->unk30 = 0xC;
        out->unk34 = -1;
        out->unk44 = 0xC;
        out->unk48 = -1;
        this->methods->slotCC(this, -0x100, 0);
    } else {
        this->methods->slot44(this, 0, D_80089C58);
        this->methods->slotCC(this, -0x200, 0);
    }
}
```

`D_80089C58` is a new rodata pointer, extern-declared alongside this unit's
other `D_80089*` constants — not yet dereferenced by any carved code, so its
contents are unknown.

## Attempt log (2 attempts)

1. **First attempt (23/60, address drift outside range) — wrong shape, not a
   wrong value.** Wrote the natural-looking refactor: hoist the `-0x40`/
   `-0x100`/`-0x200` literal into one `s32 arg1;` set per branch, then a
   single `this->methods->slotCC(this, arg1, 0);` after the `if`/`else`
   chain. This is semantically identical but codegens differently: GCC put
   `arg1` in `$a2` (loaded once, then `move $a1,$a2` before the shared call)
   and hoisted the `this->methods` load to a single point right before that
   one call, producing a 3-instruction-shorter body and shifting everything
   downstream. Retail instead reloads `this->methods` separately at the
   *end* of each arm and jumps into a short shared 3-instruction tail
   (`lw v0,0xcc(v0); jalr v0; move a2,zero`) — the call is written
   textually inside each branch and GCC 2.6.3's cross-jump/tail-merge
   collapses the identical suffix, per the "write the literal jump graph"
   entry in docs/DECOMPILATION_LEARNINGS.md (that entry was about a vanished
   branch; this is the same mechanism from the opposite direction — merging
   identical tails, not dropping a guard).
2. **Second attempt (60/60) — call `slotCC` directly inside each arm**,
   exactly as shown above, no shared local. Matched immediately.

## Proposed learning

**A shared call site with a per-branch literal argument does not always
codegen from a single post-`if` call with a hoisted local.** When retail's
disassembly shows each arm ending with its OWN copy of the pointer-chase
(`lw this->methods` + call setup) followed by a short shared tail reached by
`j`, write the call inside each branch, textually identical except for the
literal. Let GCC's cross-jump/tail-merge collapse the shared suffix rather
than hoisting the varying argument into a local yourself — the local
changes which register holds it (`$a2` instead of `$a1`) and removes the
per-branch reload, shrinking the function and shifting everything after it.
This is the same `-O2` tail-merge behind the existing "write the literal
jump graph with `goto`" entry, just triggering the merge instead of avoiding
it.

## Naming

Why `MoodCueNN`: the function's address sits in `gEntityMoodHandlerTable` row 25 (`asm/data/79528.data.s`, base 0x80089EB0, stride 0x10; row = (slot address - 0x80089EB0) / 0x10), read directly off the table (this unit's own row assignment, round 78). Tier B: the row-to-function mapping is a compiler fact, not a guess, but which dream object or mood state each row represents is not established -- the row number is kept decimal and zero-padded so the names sort in table order, same convention as Entity_b/d/e/g.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a Class65650 subclass whose table and object expand `CLASS65650_SLOTS`/`CLASS65650_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
