# Entity__MoodCue57

> Renamed from `func_80061198` on 2026-09-24 (tools/rename.py). Address 0x80061198.

**Unit:** Entity_d · **Size:** 154 words · **Status:** MATCHED (154/154 words)

## What it does

A mood-dispatch handler (`Entity *this, EntityMoodHandlerArg *out`), same
family as this unit's other `func_800*` mood handlers. On `this->unkFC==0`
rolls a `% 3` and, on a hit, fires `slot130`/`slotCC(0x1800, 0)`. Always
fetches `out->unk10` via `slot148`. If `this->unk44` is still zero, writes a
fixed `out->unk1C/unk20` pair, calls `slotC4(-0x200, 0)`, and dispatches on an
`unkFC` range/exact-value check into `slotCC`/`slot12C`. If `this->unk44` is
nonzero, dispatches on `this->unk84` ("mood") against five constants
(`<0x1E`, `0x1E`, `0x23`, `0x30`, `0x3B`), the `0x30` arm calling
`Entity__IsNearTarget` (a vector-taking helper) and, on success, lazily creating/
reusing `this->unk100` via `Entity__GetOrCreateUnk100` to call its `slotD4`, then
possibly `slot30(this, 0xB)`.

## Derivation

Read the disassembly directly (no m2c seed needed — the shape is a close
sibling of the surrounding functions in this unit, all with the same
`(Entity *this, EntityMoodHandlerArg *out)` signature and `EntityMethods`
slot-call idiom). Two real mistakes were made and fixed along the way, both
worth recording:

### 1. Misread store offset: `out->unk48`, not `out->unk1C`

The `mood == 0x23` arm stores `-2` a second time, and a first pass
mistranscribed its offset as `0x1C` (the same field the `mood == 0x1E` arm
writes), producing a spurious "cross-jump tail-merge" symptom: two sibling
`if`-arms both ending `out->unk1C = -2; return;` got merged by GCC into a
single shared block, and the branch reaching it lost a word relative to
retail. The instinct at that point was to reach for `goto` per this
project's cross-jump lore (`EnableTeleportsForKind`'s report) — confirmed in isolation
that `goto` does NOT change GCC 2.6.3's cross-jump decision here (front-end
lowers an equivalent `if`/`else-if` chain and an equivalent `goto` chain to
the same RTL), so that lead was a dead end. Re-reading the raw disassembly
bytes directly settled it:

```
/* 51AA4 800612A4 480022AE */   sw        $v0, 0x48($s1)
```

`0x48`, not `0x1C`. The two arms write **different** fields
(`out->unk1C` for `mood==0x1E`, `out->unk48` for `mood==0x23`, alongside
`out->unk44=0x16`) — there was never a real merge opportunity, and no
`goto` was needed once the field was read correctly.

**Lesson: when a residue looks like a compiler over-eagerness (cross-jump,
tail-merge, dropped branch), re-verify every store offset in the raw `.s`
bytes before reaching for a source-shape workaround.** A misread offset that
happens to coincide with another arm's target manufactures a fake instance of
a real, documented compiler quirk.

### 2. `%3` register identity: drop the intermediate local, read the field back

`this->unkFC == 0` block's `rand() % 3` computation initially used a local:

```c
r = rand() % 3;
this->unk44 = r;
if (r == 0) { ... }
```

This compiles the standard `%3` mult/mfhi/sub sequence into `$a0` for the
final remainder (and swaps which of `$v0`/`$a0` holds the sign-fix vs. the
`mfhi` result partway through) — a pure register-identity residue, same
instructions, different registers, banned from an `asm` fix per CLAUDE.md.
Isolated in a standalone reproducer (`tools/gcc263/cpp` through `as`, see
CLAUDE.md's "Escalate, do not experiment" pipeline) confirms it's the source
shape, not this function's surroundings: dropping the local and using the
field directly for both the store and the guard —

```c
this->unk44 = rand() % 3;
if (this->unk44 == 0) { ... }
```

— reproduces retail's register assignment exactly (`$v1`=sign, `$a0`=`mfhi`
result, final remainder lands back in `$v0`, matching retail's reuse of the
`mult` dividend register for the store). The one-instruction-longer version
with the intermediate local is one instruction *count*-identical but
register-wrong throughout the whole 8-instruction `%3` sequence.

### Control-flow shape

The `this->unk44 == 0` vs `!= 0` split is a genuine "big body on one arm, big
body on the other" case — not the earlier "small early exit" idiom. Retail's
own branch (`beqz $v0, .L8006135C`) sends the **zero** case to a
higher-address out-of-line block and keeps the **nonzero** case (the
mood-dispatch chain) as the fallthrough, i.e. structurally:

```c
if (this->unk44 != 0) {
    /* mood dispatch chain, all return */
    return;
}
/* unk44==0 tail */
```

writing it the other way around (`if (this->unk44 == 0) { ...; return; }`
followed by the mood chain) compiles with the branch sense and block order
inverted relative to retail — confirmed by objdump, not just theorized.

### `Entity__IsNearTarget` argument

`this->unk14->x` — `Entity__IsNearTarget`'s `pos` argument is
`&this->unk14->x` (offset `0x18` into `EntityPos`, i.e. the start of the
`{x,y,z}` vector), derived from `lw $a1, 0x14($s0)` immediately followed by
`addiu $a1, $a1, 0x18` in the branch delay slot (pointer arithmetic on the
just-loaded `this->unk14`, not a fresh load).

## Header additions (`include/Entity.h`, additive only)

- `Unk100Methods::slotD4` — new slot at `+0xD4`
  (`void (*)(Unk100Obj *self, s32 arg1, s32 arg2, s32 arg3)`), immediately
  after the existing `slotD0`.
- `extern Unk100Obj *Entity__GetOrCreateUnk100(...)` — `Entity__GetOrCreateUnk100` was already
  matched in `src/Entity.c` (defined there, not `INCLUDE_ASM`) but had no
  cross-unit prototype; this is its first caller outside that file.
- `Entity::unk50` split out of the existing `pad50[0x58-0x50]` padding as a
  named `s32` field (padding narrowed to `pad54[0x58-0x54]`) — a pad split,
  not a retype of any named field.

None of these retype or rename an existing named declaration.

## Proposed learning

- **A "cross-jump tail-merge" diagnosis is falsifiable in under a second and
  should be checked before it's trusted.** Two sibling `if`-arms whose last
  visible C statement stores the same constant to the same field name are
  only real merge bait if they actually target the same struct offset —
  re-read the raw hex bytes (not just the mnemonic) for the store's
  immediate/offset field before writing up a cross-jump stall or reaching for
  `goto`; a misread offset produces the exact same *symptom* (a branch
  target absorbed into a shared block) as a real compiler quirk.
- **For a `x = e % 3` (or similar small-constant-modulus) assignment stored
  straight into a field and guarded by `== 0`/`!= 0` immediately after: skip
  the intermediate local.** `field = e % 3; if (field == 0)` and
  `t = e % 3; field = t; if (t == 0)` are NOT register-identical even though
  they're instruction-count-identical — the local forces GCC 2.6.3 to pick a
  different final-remainder register (`$a0` instead of reusing the `mult`
  dividend register `$v0`) through the whole mult/mfhi/sub expansion.
  Confirmed in an isolated reproducer, independent of this function's
  surroundings.

## Naming

`Entity__MoodCue57` -- tier B (round 76, runner delta, FINISHING-PLAN track 3). Same
convention as `Entity__MoodCue00` (round 71): the function's address is the
handler word of `gEntityMoodHandlerTable` (`asm/data/79528.data.s`, base
0x80089EB0, 0x10-byte stride) at row 57, read directly from
`disk/SLPS_015.56`. Mechanics established (mood-tick sound-cue-set
callback); which dream object owns the row is not.

## Track 4 (2026-09-26, round 87, echo)

`this->unk100` is a `FadeBox *` (include/FadeBox.h); the slot
call through its +0x0D4 is now `startFadeDown` (FadeBox__StartFadeDown), with `companion2`, an
`s32` in Entity.h, cast `(BasicClass *)` as the fade's source (FadeBox's
configure adds it as a child; no code). Image byte-identical.

## Track 4 (2026-09-26, round 88, echo)

Measured: through Class65650's `s32 playTod` slot this function grows 3 words (the final playTod call no longer cross-jumps with void siblings). Every Entity playTod call casts the slot to `EntityPlayTodFn` (void), which emits no code.

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a Class65650 subclass whose table and object expand `CLASS65650_SLOTS`/`CLASS65650_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
