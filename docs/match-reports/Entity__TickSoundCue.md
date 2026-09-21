# Entity__TickSoundCue

> Renamed from `func_8005D6D4` on 2026-09-19 (tools/rename.py). Address 0x8005d6d4.

**Unit:** Entity · **Size:** 16 instructions · **Status:** MATCHED (16/16 words, whole-image build verified byte-exact)

## What it does

Calls a still-uncarved function, `func_8002CD08(this->unk58, &this->unk9C)`,
then increments a counter, `this->unkFC++`. `this->unk9C` is address-taken
only (an output buffer, never read within this unit), `this->unk58` is
passed by value.

**Updated in round 2026-09-01 (runner bravo, Entity 11-function pass):**
`this->unk9C` is no longer a `u8[0xF0-0x9C]` array — `Entity__Entity`
(matched in that pass) only ever zeroes its first word
(`this->unk9C = 0;`), so `Entity.h` now types it as a plain `s32 unk9C` with
the remainder folded into a following pad field. This call site changed from
the old array-decay form to an explicit `&this->unk9C`, which compiles to
the identical `addiu $a1,$s0,0x9C` — re-verified byte-exact after the
change.

## Derivation

```
lw   $a0, 0x58($s0)          ; a0 = this->unk58
jal  func_8002CD08
 addiu $a1, $s0, 0x9C          ; a1 = &this->unk9C
lw   $v0, 0xFC($s0)
addiu $v0, $v0, 0x1
sw   $v0, 0xFC($s0)          ; this->unkFC++
```

## Final C

```c
void Entity__TickSoundCue(Entity *this) {
    func_8002CD08(this->unk58, &this->unk9C);
    this->unkFC++;
}
```

`this->unk9C` is now `s32 unk9C` (see the update note above), so the call
site takes its address explicitly with `&this->unk9C` — same
`addiu $a1,$s0,0x9C` as the array-decay form it replaced.

`func_8002CD08` is declared `extern void func_8002CD08(s32 arg0, void
*arg1);` in `Entity.h` — still uncarved (no `asm/nonmatchings` file), called
directly by name (`jal`), so it needs a real prototype per CLAUDE.md's
"calling into a function that is still `INCLUDE_ASM`" guidance (this one
isn't even `INCLUDE_ASM` yet, it's inside an unsplit segment entirely, but
the same rule applies: a real extern is required for a direct-by-name call,
unlike a vtable-dispatched call). The return value is unused at this call
site, so `void` is a safe read regardless of the real signature.

## Attempt log

Matched on the first attempt.

## Proposed learning

`func_8002CD08` takes `(s32, void *)` judging by this call site alone;
`Entity__StopSoundCue` (also in this unit, see its own report) calls a different
uncarved function, `FlushSoundCueSet`, with the exact same two-argument shape
(`this->unk58`, `this->unk9C`) — worth checking whether these two are a
matched setter/getter pair or share a helper signature when either is
eventually carved.

## Naming

**Tier B.** Renamed from `func_8005D6D4` this round (tools/rename.py).
Calls the still-uncarved `func_8002CD08(this->soundCueChannel,
&this->soundCueSet)` -- the exact same two-argument shape as
`Entity__StartSoundCue`'s `InitSoundCueSet` and `Entity__StopSoundCue`'s
`FlushSoundCueSet` calls on the identical field pair (renamed this round,
see `docs/match-reports/... ` field-rename commit) -- then increments
`this->unkFC`. Strongly suggests `func_8002CD08` is the third
("tick"/"update") member of an Init/Tick/Flush trio for the same resource;
NOT renamed here since it is defined in no unit yet (still asm-only, no
`src/*.c` owns it) and renaming a function outside this assignment's unit
boundary risks colliding with whoever eventually carves it -- flagged on the
broadcast instead.

## Proposed field names

- `Entity::unkFC` -> `moodTimer` -- **tier B.** Reset to 0 by
  `Entity__StartSoundCue`, incremented here, and read/compared by every one
  of Entity_b/c/d/e/f/g (grep -rn -- '->unkFC\b') for purposes well beyond
  sound (mood-duration and threshold comparisons in those units' own
  reports) -- so a sound-specific name would undersell it. CROSS-UNIT,
  proposed rather than applied.
