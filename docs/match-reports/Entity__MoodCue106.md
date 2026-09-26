# Entity__MoodCue106 — MATCH (73/73 words)

> Renamed from `func_80064B80` on 2026-09-24 (tools/rename.py). Address 0x80064b80.

**Unit:** Entity_g · **Size:** 73 instructions

## Blocker screen

No hits.

## What it does

`gEntityMoodHandlerTable` handler row; `out` unused. Sets `this->unk44 = 0xB` on
`(rand() & 1) == 0` when `this->unkFC == 0`. If `this->unk44 == 0xB`:
dispatches `slot130` (guarded by `unkFC==0`) then `slot48(this, 1,
D_80089E2C)`. Otherwise: dispatches `slot128` (guarded by `unkFC==0`) then
`slotC8(this, unkFC%20<10 ? 0x20 : -0x20, 0)`.

## The C

```c
void Entity__MoodCue106(Entity *this, EntityMoodHandlerArg *out) {
    if (this->unkFC == 0) {
        if ((rand() & 1) == 0) {
            this->unk44 = 0xB;
        }
    }
    if (this->unk44 == 0xB) {
        void *fn;

        if (this->unkFC == 0) {
            this->methods->slot130(this);
        }
        fn = this->methods->slot48;
        ((void (*)(Entity *, s32, void *))fn)(this, 1, D_80089E2C);
        return;
    }
    if (this->unkFC == 0) {
        this->methods->slot128(this, 1);
    }
    this->methods->slotC8(this, (this->unkFC % 20 < 10) ? 0x20 : -0x20, 0);
}
```

## Residue chased: retail shares ONE `jalr` across two different vtable slots

Retail's disassembly reaches a SINGLE `jalr $v0 / nop` instruction pair
(file `0x55488`/`vram 0x64c88`) from BOTH branches: the `unk44==0xB` path
sets up `v0`=`slot48`, `a1`=1, `a2`=`&D_80089E2C` and jumps there; the other
path sets up `v0`=`slotC8`, `a1`=`±0x20`, `a2`=0 and falls into the same
address. This is GCC 2.6.3's tail/cross-jump merge collapsing two
DIFFERENT call sites (different vtable offsets, different argument types)
into one shared "jalr" once their trailing bytes are identical -- not
something a plain two-statement C rewrite reproduces, since each `this->
methods->slotNN(...)` call expression compiles its own independent `jalr`.

Reached byte-exact by making the SHARED CALL explicit in the source: load
the target function pointer into a local (`fn = this->methods->slot48;`),
cast it to a common signature, and call through it as one expression. The
`slotC8` path's own two literal-argument calls (`0x20`/`-0x20`) were ALSO
originally written as two separate `if`/`else` call statements and needed
the same treatment in miniature -- collapsing them into one call with a
ternary for `a1` merged THEIR shared tail first (this alone went from
34/73 to 35/73 with the outside-range drift shrinking), which is what
exposed the remaining slot48-vs-slotC8 residue as the only one left.

## Attempts (4)

1. Plain nested `if`/`else`, two separate `slotC8` calls for the ±0x20
   arms -- 34/73, tail not shared anywhere.
2. Same shape with an explicit early `return` after the `slot48` branch --
   identical 34/73 (confirms `return` vs falling off the end is not the
   lever here).
3. A single shared `void (*fn)(Entity*,s32,void*)` used for BOTH the
   `slot48` and `slotC8` calls (one variable serving both branches) --
   regressed to 26/73: forcing the SAME variable across both branches
   changed unrelated scheduling (computed `a2` before `v0` instead of
   after).
4. `this->methods->slotC8(this, ternary, 0)` as ONE call (closing the
   `slotC8` pair's own tail first, 35/73), THEN a per-branch local `fn`
   pointer used only inside the `unk44==0xB` branch for the `slot48` call
   -- 73/73.

## Struct/table knowledge established

No new fields; corroborates `unkFC`, `unk44`, `slot130`, `slot48`,
`slot128`, `slotC8`.

### Proposed learning

When retail shares a single `jalr` across what read, at the C level, as
TWO STRUCTURALLY DIFFERENT calls (different vtable slot, different
argument types) rather than two copies of the SAME call, the "write the
literal goto graph" lever from `DECOMPILATION_LEARNINGS.md` needs a
companion: make the call target itself a variable. `fn = this->methods->
slotNN; ((cast)fn)(args);` lets GCC schedule and merge the indirect call
the same way a shared computed-jump would, where a plain `this->methods->
slotNN(args);` statement cannot merge with a sibling call to a DIFFERENT
slot no matter how the surrounding control flow is shaped. Try this
specifically when a residue is "one duplicated `jalr`/`nop` pair, otherwise
byte-identical" and the two call sites reached are for different slots.
Also worth restating: fold literal-differing-only-by-one-argument sibling
calls (here, `slotC8(...,0x20,...)` vs `slotC8(...,-0x20,...)`) into ONE
call with a ternary BEFORE tackling a cross-slot residue -- it can expose
(or even partially close) the real problem, as it did here.

## Head-broadcast levers: applicability

Neither lever (negation idiom; dual-based-type array walkers) applies.

## Provenance

round 13 (2026-09-03), runner alpha, unit Entity_g. 4 attempts.


## Naming

Why `MoodCue106`: the function's address sits in `gEntityMoodHandlerTable`
row 106 (base 0x80089EB0, stride 0x10, the row's own `handler` word),
confirmed by reading `disk/SLPS_015.56` directly rather than trusting
address proximity (Entity_d/Entity_e, rounds 76-77, measured that row
order does not track code address). Tier B: the row-to-function mapping is
a compiler fact, not a guess, but which dream state or object each row
represents is not established -- the row number is kept decimal, matching
the existing `MoodCueNN` siblings (Entity_b through Entity_f), so the
names sort in table order.

## Data constant left unnamed this round

`D_80089E2C` (`updateScale` arg, reached through a raw function-pointer
indirect call in the `moodState == 0xB` branch): s16-pair decoded
`(1,8, 2,1, 1,8, 6,1)` -- X=1/8, Y=2/1, Z=1/8, not uniform across X/Y/Z,
so it is not one of this project's single-ratio `SCALE_*` names.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a TodActor subclass whose table and object expand `TODACTOR_SLOTS`/`TODACTOR_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
