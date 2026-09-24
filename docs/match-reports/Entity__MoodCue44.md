# Entity__MoodCue44 -- MATCHED (80/80 words)

> Renamed from `func_800605D0` on 2026-09-24 (tools/rename.py). Address 0x800605d0.

Unit: `Entity_d` (second pass, round 2026-09-03). Mood-dispatch handler,
calls `func_80060710` (already matched, earlier ROM address) as a helper:
`void Entity__MoodCue44(Entity *this, EntityMoodHandlerArg *out)`.

## Final source

```c
void Entity__MoodCue44(Entity *this, EntityMoodHandlerArg *out) {
    s32 a1val;
    s32 r1;
    s32 r2;
    u8 *table;

    func_80060710(this);
    out->unk10 = this->methods->slot148(this);
    if (this->unk84 == 7 || this->unk84 == 0x16) {
        out->unk1C = 3;
    }
    if ((u32)(this->unkFC - 0x12C) < 0x14) {
        this->methods->slotC4(this, -0x3C, 0);
    } else if ((u32)(this->unkFC - 0x141) < 0x13) {
        this->methods->slot44(this, 0, D_80089C70);
    } else if (this->unkFC >= 0x141) {
        r1 = rand();
        a1val = -0x80;
        if ((r1 & 1) != 0) {
            a1val = 0x80;
        }
        this->methods->slotC8(this, a1val, 1);
        r2 = rand();
        table = D_80089C64;
        if ((r2 & 3) != 0) {
            table = D_80089C70;
        }
        this->methods->slot44(this, 0, table);
    }
}
```

## Derivation notes

- The two half-open range tests (`unkFC` in `[0x12C, 0x140)` and `[0x141,
  0x154)`) are retail's own `addiu`+`sltiu` unsigned-range-check idiom --
  `(u32)(x - LO) < (HI - LO)` reproduces the exact `addiu $v0,$v1,-LO /
  sltiu $v0,$v0,(HI-LO)` pair, same idiom as any bounds check against a
  compile-time range in this codebase.
- **Same "default value must be assigned AFTER the call its guard depends
  on" trap as `Entity__MoodCue43`'s table selection, but for a scalar this
  time, not a pointer.** `a1val = -0x80; if ((rand()&1) != 0) a1val =
  0x80;` written with the default BEFORE the `rand()` call forced the
  compiler to keep `a1val` alive across that call in an extra callee-saved
  register -- and specifically, it picked `s1`, silently colliding with
  `out`'s own register even though `out` is dead by that point in this
  branch. Retail computes `rand()` first, saves nothing, and only assigns
  the default (`-0x80`) in the branch's OWN delay slot, after the call has
  already returned. Introducing `r1 = rand();` before the default
  assignment fixed it in one change, mirroring the `Entity__MoodCue43` fix
  exactly (see that report) -- two independent instances now, worth
  treating as a general rule for this codebase.
- `slotC4`/`slot44`/`slotC8`/`slot148` were all already correctly typed;
  no header change needed. The only new extern was `D_80089C7C`-style
  data-table symbols already declared for sibling functions in this unit
  (`D_80089C64`/`D_80089C70`, both reused here, no new declarations).
- All three branches funnel into a SHARED `jalr v0` at one physical
  address in retail's bytes (`L800606F0`), with each branch loading its
  own target function pointer, `a0`, `a1`, `a2` before falling into (or
  jumping to) the common call instruction. Writing three ordinary,
  separate `this->methods->slotNN(...)` calls in C reproduced this without
  any `goto` -- the compiler's own cross-jump pass merged the trailing
  `jalr`/delay-slot pair on its own, unlike `func_80060800`'s case where an
  explicit `goto` was required. The difference: here each branch is a
  self-contained, complete call (own function pointer AND own arguments),
  so nothing needs to survive from one branch into another; `func_80060800`
  needed `goto` because multiple DIFFERENT control-flow predecessors had to
  reach the exact same single call with the exact same arguments.

### Proposed learning

- **"Default value, then conditionally overwritten," second confirmed
  instance for a plain scalar (not just a pointer): the default assignment
  must be sequenced after any call the guarding condition depends on.**
  Two independent functions in this unit (`Entity__MoodCue43`'s table pointer,
  this function's `a1val`) both needed `result_of_call = f(); local =
  default; if (cond_on_result) local = override;` rather than `local =
  default; if (cond_on_call()) local = override;` -- the second form drags
  the default value across the call into an unnecessary callee-saved
  register. Worth promoting to a general rule rather than filing as two
  one-off residues.
- **A dead-by-this-point struct-pointer parameter's register can get
  silently reused for an unrelated local, and that reuse is a clean tell
  that the local shouldn't have been promoted to a callee-saved register at
  all.** If a residue's extra callee-saved register happens to match
  another live variable's register from earlier in the function, check
  whether that variable is actually dead at this point before assuming a
  genuine sharing conflict -- it's usually a sign the promotion itself is
  the bug (fixable via the "default after call" lever above), not a naming
  collision to route around.
