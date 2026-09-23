# Entity__MoodCue00

> Renamed from `func_8005E160` on 2026-09-23 (tools/rename.py). Address 0x8005e160.

**Unit:** Entity_b · **Size:** 153 words · **Status:** MATCHED (153/153
words, whole-image build verified byte-exact)

## What it does

`(Entity *this, EntityMoodHandlerArg *out) -> void`. The largest
mood-dispatch handler reached so far, with a genuinely redundant-looking
range test that turned out to be exactly what retail compiled:

1. If `out->unk4 == 0` and `this->unk94->methods->slot200(this->unk94) == 5`,
   sets `this->unk44 = 0x64`.
2. `out->unk10 = this->methods->slot148(this);` (the usual opener).
3. If `this->unk44 == 0`: conditionally sets `out->unk1C`/`out->unk20` when
   `out->unk4 % 10 == 0`, then a three-way `this->unkFC` dispatch
   (`== 0x960` sets `unkFC = -1`; `< 0x4B0` calls `slotC4(this, 0x32, 0)`;
   else `slotC4(this, -0x32, 0)`).
4. Else (`this->unk44 != 0`), a four-way `this->unkFC` dispatch:
   - `< 0xFA` (250): the SAME `out->unk4 % 10` check as step 3, THEN its own
     inner two-way split (`< 0x64` -> `slotC4(this, 0x32, 0)`; else
     `slotBC(this, D_80089DA8)`).
   - `== 0xFA`: `slot130(this)`, `out->unk1C = -2`.
   - `>= 0x105 && < 0x238`: `slotC4(this, -0x32, 0)` then
     `slot44(this, 1, D_80089CE8)`.
   - `>= 0x239`: `slot44(this, 1, D_80089CF4)`.

## The "redundant" range test that isn't reachable-code noise

Inside the `< 0xFA` branch of step 4, retail's own bytes re-test `this->unkFC
< 0xFA` a SECOND time (as the `else if` between the `< 0x64` and `slotBC`
cases) even though that fact is already established by the OUTER branch
guarding the whole block. This looked, on first read, like it might be
dead/unreachable code the compiler left in — but it's not: GCC 2.6.3 does
not eliminate a source-level redundant re-test across intervening code at
`-O2` in this pipeline, so the retest is a direct transcription of an
`if (unkFC < 0xFA) { ... if (unkFC < 0x64) {...} else if (unkFC < 0xFA)
{...} ... }` shape in the ORIGINAL source. Writing the C with the same
apparently-pointless second test reproduced retail's bytes on the first
attempt; collapsing it to a plain `else` (which is logically equivalent)
would very likely have cost a residue, though that alternative wasn't
needed since the literal transcription matched immediately.

## New symbols

- `Unk94Methods::slot200`, `s32 (*)(Unk94Obj *self)` — a new, far-out slot in
  the same `Unk94Obj` vtable `Entity__MoodCue01.md`/`Entity__IsTargetInRange.md`
  established; padded out to `+0x200` with no other slots resolved in
  between (nothing else in this unit reaches that far into the table yet).
- Three more `D_8008xxxx` opaque data-row externs: `D_80089DA8`,
  `D_80089CE8`, `D_80089CF4`.

## Final C

```c
void Entity__MoodCue00(Entity *this, EntityMoodHandlerArg *out) {
    if (out->unk4 == 0) {
        if (this->unk94->methods->slot200(this->unk94) == 5) {
            this->unk44 = 0x64;
        }
    }
    out->unk10 = this->methods->slot148(this);
    if (this->unk44 == 0) {
        if (out->unk4 % 10 == 0) {
            out->unk1C = 5;
            out->unk20 = -2;
        }
        if (this->unkFC == 0x960) {
            this->unkFC = -1;
        } else if (this->unkFC < 0x4B0) {
            this->methods->slotC4(this, 0x32, 0);
        } else {
            this->methods->slotC4(this, -0x32, 0);
        }
    } else if (this->unkFC < 0xFA) {
        if (out->unk4 % 10 == 0) {
            out->unk1C = 5;
            out->unk20 = -2;
        }
        if (this->unkFC < 0x64) {
            this->methods->slotC4(this, 0x32, 0);
        } else if (this->unkFC < 0xFA) {
            this->methods->slotBC(this, D_80089DA8);
        }
    } else if (this->unkFC == 0xFA) {
        this->methods->slot130(this);
        out->unk1C = -2;
    } else if (this->unkFC >= 0x105 && this->unkFC < 0x238) {
        this->methods->slotC4(this, -0x32, 0);
        this->methods->slot44(this, 1, D_80089CE8);
    } else if (this->unkFC >= 0x239) {
        this->methods->slot44(this, 1, D_80089CF4);
    }
}
```

The `>= 0x105 && < 0x238` test compiles directly to retail's unsigned
range-check idiom (`(unsigned)(unkFC - 0x105) < 0x133`) with no manual
reconstruction needed — GCC 2.6.3 performs that transform on its own for a
two-constant `&&` range test.

## Attempt log

Matched on the first attempt.

## Proposed learning

**A source-level re-test of a condition already established by an
enclosing `if` is not evidence of an authoring mistake or of unreachable
code — GCC 2.6.3 compiles it literally, byte for byte, rather than proving
it always-true and eliding it.** When a disassembly shows the same `slti`
constant tested twice with no intervening code that could invalidate the
first result, the straightforward reading (transcribe both tests as
written) is correct and should be tried BEFORE reaching for a `goto`,
`switch`, or collapsed `else` — collapsing it is tempting because it reads
as "cleaner" C, but it changes the source shape retail actually has.
