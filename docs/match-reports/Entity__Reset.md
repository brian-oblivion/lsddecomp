# Entity__Reset -- MATCHED (39/39 words)

> Renamed from `Entity__InitState` on 2026-09-26 (tools/rename.py). Address 0x8005d278.

> Renamed from `func_8005D278` on 2026-09-19 (tools/rename.py). Address 0x8005d278.

**Unit:** Entity · Runner: charlie, round 23.

## What it does

Reads `gEntityUnlockKindTable[this->moodIndex*0x10]` (the same "GetUnlockEffect" table
`Entity__GetUnlockEffect` reads, unsigned byte load here vs. that stalled
function's signed one), and if `kind - 1` is unsigned-less-than 9 (i.e.
`kind` in `[1..9]`) calls a new self-only vtable slot with `arg1==1`. Then
unconditionally calls two more vtable slots.

## Final C

```c
void Entity__Reset(Entity *this) {
    s32 kind;

    kind = ((u8 *)gEntityUnlockKindTable)[this->moodIndex * 0x10];
    if ((u32)(kind - 1) < 9) {
        this->methods->slot70(this, 1);
    }
    this->methods->slot10C(this, 0x42);
    this->methods->slot160(this);
}
```

## Attempt log

First attempt wrote the range check as `(u32)((kind + 9) & 0xFF) < 9`,
copying the `+9`/`&0xFF`/byte-sign-extend pattern from `Entity__IsNearTarget`'s
similar-looking table read by mistake -- that function's residue really
does need that shape, but this one's own disassembly is simpler:
`addiu $v0,$v0,-0x1` / `sltiu $v0,$v0,0x9`, no `andi` mask anywhere. The
wrong condition compiled to extra/different instructions and shifted the
whole image (127668 bytes outside-range, funcdiff's drift warning fired).
Fixed to plain `(u32)(kind - 1) < 9` and it matched immediately.

Two new `EntityMethods` vtable slots: `slot70` (`self, s32 arg1`, called
here with `arg1==1`) and `slot10C` (`self, s32 arg1`, called here with
`arg1==0x42`, unconditionally). `slot160` was already documented.

`kind` is read via a cast to `u8 *` (`((u8 *)gEntityUnlockKindTable)[...]`) rather than
through the header's own `extern s8 gEntityUnlockKindTable[]` declaration, because this
site's load is `lbu` (unsigned) while `Entity__GetUnlockEffect`'s stalled
body reads the same table with `lb` (signed) -- same table, two different
element interpretations at two different call sites. Casting locally avoids
redeclaring `gEntityUnlockKindTable` with a conflicting type in this same translation
unit (which would be a silent fatal `conflicting types` error, per
CLAUDE.md's build-log guidance).

### Proposed learning

**A table read that "looks like" another function's near-identical-looking
table read is not evidence of the same expression shape -- check the actual
instructions before pattern-matching from memory.** Here `Entity__IsNearTarget`
(read moments earlier while scoping the whole batch) uses `gEntityUnlockKindTable`-style
byte reads with a `+9`/`&0xFF`/sign-extend-by-shift idiom for an unrelated
purpose (widening a byte to a signed multiplier); this function reads the
*same table symbol* for a plain unsigned range check and has neither the
`+9` nor the mask. Carrying the first function's idiom into the second
produced a 127668-byte whole-image drift on the first attempt -- caught
immediately by the drift warning, not by a plausible-looking wrong score.

Also: none of the round-23 head broadcast's three levers apply here -- no
`s16` locals, no loop counter, no `&arr[i+j]` pointer arithmetic.

## Naming

**Tier B.** Renamed from `func_8005D278` this round (tools/rename.py).
Occupies `EntityMethods` +0x040 (`tools/classtable.py`) -- the exact slot
`Entity__Entity` calls immediately after (re)assigning `this->methods`, so
this is definitely Entity's own post-construction setup step (hence
`initState`, not a bare `func_`). WHAT state it initializes (an unlock-kind-
gated conditional call plus two unconditional ones) is described in this
report but not asserted as a specific game concept -- kept at the mechanic
level.

## Track 4 (2026-09-26, round 88, echo)

Renamed from `Entity__InitState`: the occupant of +0x040, SceneNode's `reset` (Entity__Entity calls it through the slot after installing the table). Body: setLightMode(1) for unlock kinds 1..9, selectTickCallback(TICK_CALLBACK_B) -- the 'B' callback slot Entity fills with Entity__TickSoundCue -- then deactivate. Tier A: an override named for its slot (FINISHING-PLAN track 4 step 6).

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a TodActor subclass whose table and object expand `TODACTOR_SLOTS`/`TODACTOR_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
