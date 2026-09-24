# Entity__MoodCue41 -- MATCHED (70/70 words)

> Renamed from `func_800602AC` on 2026-09-24 (tools/rename.py). Address 0x800602ac.

Unit: `Entity_d` (fresh carve, round 2026-09-03). Mood-dispatch handler, same
family as `Entity_c.c`'s `func_8005EF54`/`func_8005EFF4`/etc: `void
Entity__MoodCue41(Entity *this, EntityMoodHandlerArg *out)`.

## Final source

```c
void Entity__MoodCue41(Entity *this, EntityMoodHandlerArg *out) {
    s32 rv;
    s16 *tablePtr;

    if (this->unkFC == 0) {
        this->unk44 = rand() % 5 + 0xA;
    }
    if (this->unk44 < 0xE || this->unkFC < 0x140) {
        func_80064FBC(this, out, 0xBB8, 0x1F4, -0x100);
        return;
    }
    if (this->unk44 == 0xE) {
        if ((this->unkFC & 3) == 0) {
            rv = rand();
            tablePtr = &sScaleTemplateZDenom;
            *tablePtr = rv % 32 + 1;
            this->methods->slot48(this, 1, (u8 *)tablePtr - 0xA);
        }
    }
}
```

## Derivation notes

- `this->unkFC == 0`-guarded `this->unk44 = rand() % 5 + 0xA;` must be ONE
  expression, not `r = rand() % 5; this->unk44 = r + 0xA;` through a
  temporary -- the temp forces a different register in the magic-multiply
  divide-by-5 sign-fix chain (`sra`/`mfhi`/`sra`/`subu`), a register-identity
  residue closed purely by removing the intermediate.
- **Branch polarity trap, and the biggest time sink in this function.** My
  first reading of the `slti v0,v1,0x140 / beqz v0,L80060350` pair had the
  condition backwards: reaching the `func_80064FBC` call requires `unk44 <
  0xE || unkFC < 0x140`, not `unkFC >= 0x140` as an initial (wrong) read
  suggested. Always trace which branch is TAKEN vs FALLTHROUGH explicitly
  instead of pattern-matching the mnemonic.
- `func_80064FBC`'s argument order is NOT the order the literals appear in
  the instruction stream. The instruction sequence computes the stack
  argument (`-0x100`) FIRST, then `a2=0xBB8`, then `a3=0x1F4` in the `jal`'s
  delay slot -- but positionally that means the CALL's actual parameter list
  is `(this, out, 0xBB8, 0x1F4, -0x100)`: `a2`/`a3` take the two LATER
  literals, and the FIRST-computed one goes on the stack as the 5th
  argument. Reading argument order off computation order is wrong; read it
  off which register/slot each value lands in.
- The `sScaleTemplateZDenom` pointer must be computed AFTER the second `rand()` call in
  source order, not before -- computing it first forces the compiler to keep
  it alive across the call in a callee-saved register (`s1`), where retail
  keeps it in caller-saved `a2` because nothing calls between computing the
  address and its last use (the final `slot48` call). This is the same
  family as "cache a vtable pointer before an intervening call" but the
  observation runs the other way: compute the ADDRESS after the call it
  would otherwise have to survive.
- `sScaleTemplateZDenom - 0xA` (byte-pointer arithmetic) resolves to `SCALE_X3 + 0xC`
  numerically (`asm/data/79528.data.s`), but retail's relocation is against
  `sScaleTemplateZDenom` specifically (own `lui`/`addiu` pair), so the C must reference
  that symbol with a negative offset, not the neighbouring array with a
  positive one -- same final address, different symbol reference, and only
  one of them reproduces retail's bytes.
- `rand() % 32 + 1` on a signed value reproduces retail's bias-and-shift
  idiom (`bgez` test, `+0x1f` correction, `sra 5`, `sll 5`, `subu`) for a
  power-of-two signed modulus -- no manual reconstruction needed, `%` alone
  is correct (same family as the documented `x % N` idioms, extended to
  power-of-two divisors that still need the sign fix because the value can
  be negative).

### Proposed learning

- **`rand() % 5 + 0xA` (or any `x % C1 + C2`) must stay ONE expression.**
  Splitting the modulo into its own local before adding the offset changes
  which register the magic-multiply sign-fix chain lands in, even though
  the sequence of arithmetic operations is otherwise identical.
- **A callee's true argument order can put an EARLIER-computed literal in
  the LATER stack slot.** Do not assume the instruction stream's
  left-to-right literal order matches the parameter list; read each
  literal's actual destination register/slot.

## Naming

`Entity__MoodCue41` -- tier B (round 76, runner delta, FINISHING-PLAN track 3). Same
convention as `Entity__MoodCue00` (round 71): the function's address is the
handler word of `gEntityMoodHandlerTable` (`asm/data/79528.data.s`, base
0x80089EB0, 0x10-byte stride) at row 41, read directly from
`disk/SLPS_015.56` (not inferred from address proximity -- see
`src/Entity_d.c`'s unit header comment, which flags that row order does NOT
track code address once row 115 is reached). Mechanics established
(mood-tick sound-cue-set callback, per `Entity__StartSoundCue`/
`Entity_b.c`'s own header comment); which dream object owns the row is not.
