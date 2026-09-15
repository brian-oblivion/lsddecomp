# func_8001CEB4 -- MATCHED, round 45 (2026-09-15)

**Unit:** `code_d294` · **Size:** 85 words · **Status:** MATCHED, 85/85 exact,
whole-image SHA1 green.

Filed since round 13/14 as blocked by `nop_mflo_mfhi` (a sibling of the
`addiu_at` blocker). That flag was RESOLVED in round 42 by
`maspsx --no-nop-mflo-mfhi`, pinned in the Makefile. Round 42's own reopen
note on this file recorded that the PRESERVED body, rebuilt with the flag on,
was NOT a match (one word longer than retail, plus a register-identity
difference) -- that finding held: the body needed one more source-level fix
beyond the flag, described below.

## Body

```c
void func_8001CEB4(Class6B5CCObj *self, s32 flag, void *data) {
    s32 vals[3];
    Class6B5CCSub44 *dst;
    s16 *field;

    vals[0] = func_8001EC84(data);
    vals[1] = func_8001EC84((u8 *)data + 4);
    vals[2] = func_8001EC84((u8 *)data + 8);
    vals[0] /= 360;
    vals[1] /= 360;
    vals[2] /= 360;
    dst = self->unk14->unk44;
    field = &dst->vec.x;
    if (flag) {
        dst->vec.x = vals[0];
        dst->vec.y = vals[1];
        dst->vec.z = vals[2];
    } else {
        s32 i;
        s16 *cur;

        for (i = 0; i < 3; i++) {
            cur = field;
            field++;
            *cur = (*cur + vals[i]) % 4096;
        }
    }
    self->unk14->unk0 = 0;
}
```

## What changed from the round-13/round-42 preserved body

The round-13 derivation (three `func_8001EC84` reads divided by 360, an
`if(flag)` overwrite vs. a wrap-accumulate `else`, unconditional
`field = &dst->vec.x` before the branch) was already correct in full --
confirmed by rebuilding it as-is first this round: **71/85 words match, zero
drift outside the function**, once the toolchain flag was on. The remaining
14-word mismatch was a single loop-shape choice, not a new derivation:

- The original preserved body wrote the accumulate loop as
  `field[i] = (field[i] + vals[i]) % 4096;` (array indexing off a fixed
  base). That produces the right VALUES but retail's own loop is a
  **pointer-walk with a one-iteration-ahead advance**: each pass copies the
  carried pointer into a scratch register, uses the scratch register for
  the load AND the store, and advances the carried pointer immediately
  after the load (before the arithmetic) -- i.e. `cur = field; field++;
  *cur = ...;`, not `field[i] = ...; field++`. Array indexing keeps `field`
  itself as the base and recomputes an offset each time, which allocates
  differently and needed an extra `move` to get the initial pointer into
  the loop's working register -- the "one word longer" residue round 42's
  rebuild note flagged.
- Rewriting the loop with an explicit `cur` scratch pointer (copy-then-
  advance-then-dereference, matching retail's instruction order exactly)
  reproduced retail's register allocation (`field` in `$a3` as the carried
  pointer, `cur` in `$a0` as the per-iteration scratch) with zero remaining
  diff. No other change was needed -- struct layout, division idiom, and
  overall control flow were already right.

## Derivation notes (carried over, still accurate)

- Three `func_8001EC84` reads (`data+0`/`+4`/`+8`), each divided by 360
  (magic `0xB60B60B7`, shift 8) -- the three divisions are scheduled
  back-to-back by the compiler (each `mult` issued before the previous
  one's `mfhi` is consumed), which is why the raw disassembly interleaves
  them even though the C is three simple, independent statements in
  source order.
- `flag != 0`: overwrite `dst->vec.x/y/z` directly with the divided values
  (truncating store to `s16`, implicit via the destination's field type).
- `flag == 0`: accumulate each divided delta into the existing field value
  and wrap modulo 4096 (a full-turn wrap for a PSX 4096-per-circle angle
  unit) -- ordinary C `%`, verified byte-for-byte against the pinned `cc1`
  in the original round-13 derivation.
- `field = &dst->vec.x` is computed unconditionally before the `if`, not
  inside the `else` alone -- retail materializes it in the `beqz` branch's
  own delay slot on both paths, even though only the `else` path reads it
  back.

### Proposed learning

**A loop that reads-modifies-writes through a pointer and also advances
that pointer can need the advance modeled as an EXPLICIT extra step
(`cur = p; p++; *cur = ...`), not array indexing (`p[i] = ...`) and not a
combined post-increment dereference (`*p++ = ...`) -- even when all three
are value-equivalent C.** This function is the second confirmed instance of
the "resolved toolchain blocker, but the preserved body still needed a
loop-shape fix, not just a rebuild" pattern this round (see also the
round-42 reopen notes generally) -- worth checking on any other queued
function whose STALL predates round 42 and involves a pointer-walking loop
over a small fixed count.
