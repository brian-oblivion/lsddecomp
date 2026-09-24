# Entity__MoodCue55 -- MATCHED (58/58 words)

> Renamed from `func_80061070` on 2026-09-24 (tools/rename.py). Address 0x80061070.

Unit: `Entity_d` (fresh carve, round 2026-09-03). Mood-dispatch handler:
`void Entity__MoodCue55(Entity *this, EntityMoodHandlerArg *out)`.

## Final source

```c
void Entity__MoodCue55(Entity *this, EntityMoodHandlerArg *out) {
    s32 mood = this->unk84;

    if (this->unkFC == 0) {
        if (rand() % 3 == 0) {
            this->methods->slot48(this, 1, SCALE_Y2);
        }
    }
    out->unk10 = this->methods->slot148(this);
    if (mood >= 0x20) {
        mood -= 0x20;
    }
    if (mood == 9 || mood == 0x11 || mood == 0x17) {
        out->unk1C = 0x13;
    }
    if (mood == 0x17) {
        out->unk30 = 0x13;
    }
}
```

## Derivation notes

- **`this->unk84` must be read into a local at the very TOP of the
  function, as the declaration's own initializer, not read where it is
  first logically needed (right before the range check near the end).**
  Retail keeps it in a callee-saved register (`s0` in retail's own
  allocation) from function entry, because it has to survive the `rand()`
  call and the `slot48`/`slot148` vtable calls that come before its first
  use. Reading it late left it as a throwaway load in a caller-saved
  register (`a0`) computed right before the compare, one word short of
  retail and with the WRONG base register throughout the whole rest of the
  function (a full `$s0`/`$s1`/`$s2` register-identity cascade, not just a
  single word). `s32 mood = this->unk84;` as the first statement fixed
  the whole cascade in one change.
- The three-way `mood == 9 || mood == 0x11 || mood == 0x17` check followed
  by a SEPARATE `if (mood == 0x17)` (not nested inside the first) is the
  "source-level re-test of an already-established condition"
  idiom from `docs/DECOMPILATION_LEARNINGS.md` -- retail's bytes literally
  re-test `mood == 0x17` after already having taken one of three paths into
  the shared block, rather than branching once and remembering which case
  it was.
- This function was the SECOND instance (after `Entity__func_80060710`) of a single
  function's defect cascading into every function after it in ROM order --
  before the `mood` fix, this function itself scored 6/58 with a
  `differs outside range` count that had otherwise dropped to ~30 bytes
  project-wide (confirming `Entity__func_80060710`'s fix had already landed and
  isolated the remaining drift to just this one function).

### Proposed learning

- **A struct field read late in a function, used only after several
  intervening calls, can still need to be the function's very FIRST
  statement.** The tell is not "is this field used across a call" (true of
  many fields that don't need this treatment) but whether retail's own
  register allocation keeps it in a callee-saved register from entry --
  visible as the field's first `lw` sitting immediately after the prologue
  in the disassembly, well before its first logical use. When the
  disassembly shows this pattern, declare the local eagerly with an
  initializer at the top of the function, not at its first point of
  logical need.
