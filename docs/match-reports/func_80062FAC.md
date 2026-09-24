# func_80062FAC -- MATCHED (58/58 words)

Unit: `Entity_e` (round 13). Dispatches `slot148`, checks `out->unk4 % 10`,
unconditionally fires `slot44` with the same `ROTATION_YAW_PLUS2` table other units
already reference, then a one-shot `slot30`/`unk44` latch gated on
`unk44 == 0 && unkF4 != 0`.
`void func_80062FAC(Entity *this, EntityMoodHandlerArg *out)`.

## Final source

```c
void func_80062FAC(Entity *this, EntityMoodHandlerArg *out) {
    out->unk10 = this->methods->slot148(this);
    if (out->unk4 % 10 == 0) {
        out->unk1C = 0x19;
        out->unk20 = 2;
    }
    this->methods->slot44(this, 0, ROTATION_YAW_PLUS2);
    if (this->unk44 == 0 && this->unkF4 != 0) {
        this->methods->slot30(this, 0xB);
        this->unk44 = 0xB;
    }
}
```

## Derivation notes

- `out->unk4 % 10 == 0` is the magic-constant idiom for divisor 10
  (`0x66666667`, shift 2 after `mfhi`) -- verified against the pinned `cc1`
  with `int f(int x){return x/10;}`, byte-for-byte, continuing the
  shift-fingerprints-the-divisor technique from `Entity__MoodCue69`'s report.
- The `slot44(this, 0, ROTATION_YAW_PLUS2)` call is UNCONDITIONAL: it sits between
  the `% 10` check and the `unk44`/`unkF4` gate, not inside either `if`. The
  `a0 = this` set up in the `bne`'s delay slot at the end of the `% 10`
  branch survives untouched across the branch-not-taken path because
  nothing else writes `$a0` before the call, which is why the call reads as
  reachable from both arms of the first `if` in the disassembly -- it is
  simply after it, not conditioned by it.
- `this->unk44 == 0 && this->unkF4 != 0` is a short-circuit `&&`: the first
  `bnez` skips the whole block when `unk44 != 0`, the second `beqz` skips it
  when `unkF4 == 0`, both landing on the same `.L8006307C` exit label --
  standard back-to-back guard clauses, not two independent `if`s (there is
  no code between them for a first `if` to fall through into).
- `ROTATION_YAW_PLUS2` already has externs and a callsite (`slot44(this, 0,
  ROTATION_YAW_PLUS2)`) in `Entity_b.c`/`Entity_c.c`; this unit gets its own
  per-unit `extern u8 ROTATION_YAW_PLUS2[];` per the file's existing convention
  (documented at the top of the data-table externs already in this file),
  not a shared header declaration.

No new struct or vtable-slot knowledge; `slot148`, `slot44`, `slot30`,
`unk44`, `unkF4`, and all touched `EntityMoodHandlerArg` fields were already
known.
