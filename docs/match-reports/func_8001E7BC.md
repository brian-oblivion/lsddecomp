# func_8001E7BC -- STALL (structural analysis only, no C attempted)

Unit: `code_d294_c` (round 14). By far the largest function in this
round's queue (200 asm lines, 0x2D0 bytes / 180 words -- more than 60%
bigger than the next-largest, `func_8001EACC` at 110 words). Blocker
screen clean: no `gp_rel`, no `addiu $at,$at,%lo`, no `mfhi`/`mflo`-
adjacent-`mult`/`div` hit.

**No C was written or built for this function.** Given the size, the two
still-uncarved callees it depends on (`func_8001F8B8`, `slotA4`'s
occupant `func_8001D950`), and the amount of genuinely new field/struct
derivation needed, a full structural read was the higher-value use of
this session's remaining time over a first, likely-incomplete C attempt.
This does NOT count against the 30-attempt budget -- the analysis below
is meant to let the next runner (or this runner in a future round) start
writing C directly instead of re-deriving the shape from scratch.

## Signature and top-level shape

`s32 func_8001E7BC(Class6B5CCObj *self, void *arg1, void *arg2)`
(return value is 0 or 1; `arg1`/`arg2` types not yet pinned down --
`arg2` is read as a 3-entry `u16`-ish table, `arg1` is forwarded whole
into `func_8001EA8C`'s own `dest` parameter, already `s32*` per that
function's own signature).

```
if (self->unk20 == 0) return 0;
if (self->unk10 < 0) {
    <accumulate self's own "attached node" list into self->unk14->unk38>
}
<compute a 3-entry s16 delta between arg2 and self->unk14->unk38 (or {0,0,0} if self->unkC==0)>
self->methods->slotA4(self, 0, /* &sp+0x18-ish scratch */, /* &delta */, 1);  /* 5 args, occupant func_8001D950, code_d294_b, out of scope */
if (!func_8001F8B8(self->unk20, &scratch1, &scratch2, 0, &scratch3, delta_minus_1024)) {
    if (!func_8001F8B8(self->unk20, &scratch1, &scratch2, 0, &scratch3, delta_plus_1024)) {
        return 0;
    }
}
func_8001EA8C(arg1, &scratch3, &scratch2);   /* already matched, this unit */
return 1;
```

## New field/struct knowledge NOT yet committed (needs verification against
## real C before adding to `include/code_d294.h`)

- **`Class6B5CCObj` gains (at least) THREE new fields**, all read directly
  off `self` (not through `unk14`):
  - `+0x20`: ALREADY typed `void *unk20` (round 12, `func_8001D600`) --
    this function is a SECOND confirming use, null-checked at entry (a
    `return 0` guard) and later passed as `func_8001F8B8`'s own first
    argument. No retype needed, just a second confirmed non-NULL-checked
    use.
  - `+0x10`: a NEW `s32` field, compared with `bgez` (signed, so `s32`
    not `u32`) -- gates the entire "accumulate list into `unk38`" block
    (only runs when `< 0`).
  - `+0xC`: **CONFLICTS with the ALREADY-TYPED `UnkOwner_d294 *unkC`**
    (an owner back-reference, established by `func_8001D0EC`/
    `func_8001D1A4` in `code_d294.c`). This function's OWN use --
    null-checked, then (when non-null) `self->unk14` is read and treated
    as the base for a `+0x38` sub-table, exactly the SAME
    `Class6B5CCSub14::unk38` field `func_8001E600`/`func_8001EACC`
    already established this round -- is CONSISTENT with `unkC` staying
    a simple null/non-null gate (its own POINTER value is never
    dereferenced here, only tested against 0), so this is likely NOT a
    real conflict, just a THIRD confirmed null-check use of the same
    field. Flagging explicitly per the shared-header rule since it
    touches an offset two other functions in TWO different rounds
    already named, even though no type change is implied.
- **A linked-list walk, `self->unkC`'s OWN target chases a `+0xC` "next"
  pointer** (inside the accumulation loop, `node = node->unkC` advances,
  loop continues `while (node != 0)`) -- so whatever `self->unkC` points
  to is a DIFFERENT type from `UnkOwner_d294` (which has no `+0xC` field
  of its own) OR `UnkOwner_d294` itself needs a `+0xC` "next" field added.
  Each list node also has its own `+0x14` pointing to a Vec3-shaped
  structure (`+0x18`/`+0x1C`/`+0x20`, read via `node->unk14->unk18` etc.)
  -- the SAME `EntityPos`/`Class6B5CCSub14`-family "position vector
  pointer at `+0x14`" convention seen throughout this project. This
  needs its own type, not yet named.
- **`Class6B5CCSub14::unk38`'s element type needs reconciling across TWO
  different access widths.** `func_8001E600` (matched, this round) reads
  and writes it as `s32[3]` (full-word arithmetic, `dst[i] += table[i]`).
  THIS function reads the SAME field via `lhu` (unsigned HALFWORD) at the
  same stride-4 offsets (0, 4, 8) -- i.e., only the LOW 16 bits of each
  `s32` slot, consistent with values that never exceed the `s16` range in
  practice (a plausible reading for bounded in-game deltas), but not
  proven from this call site alone. Whichever runner writes this
  function's C should decide between an explicit `(u16)table[i]` cast (no
  struct change) and a genuine struct retype (e.g. `struct { s16 lo, hi;
  }` per element) based on what the pinned `cc1` actually selects for
  each candidate -- do not guess from the disassembly alone, this project
  has repeatedly found the "obvious" reading wrong for this exact
  toolchain (see this unit's other reports from this round).
- **An apparent dead/defensive check**: `if ((u8 *)self->unk14 + 0x38 ==
  0)` (adding `0x38` to a pointer, then comparing the SUM against zero,
  rather than checking `self->unk14` itself for null) gates entry to the
  accumulation loop. Algebraically this is almost never true for a real
  heap pointer; reproduce it literally rather than "fixing" it to a
  direct `self->unk14 == 0` check, which would NOT match retail's bytes
  (confirmed by the disassembly computing the sum FIRST, then testing
  it, not testing `self->unk14` before the addition).
- **New extern needed: `func_8001F8B8`** (still uncarved, presumably the
  NEXT slice past this one or a sibling segment) -- called twice with
  identical arguments except the LAST (a candidate angle table, one
  computed with `-0x400`/`+0x400` applied to one axis, i.e. a +/-90-degree
  BAM offset), tried in sequence until one returns nonzero; reads as a
  "does this candidate angle lead to something valid" test, but that's
  inference, not confirmed.
- **New `Class6B5CCMethods` slot needed: `+0xA4`** (occupant
  `func_8001D950`, `code_d294_b`, confirmed via `tools/classtable.py
  D_8006B5CC`, out of this carve's scope) -- called with 5 arguments
  (`self`, `0`, two scratch pointers, `1`), signature not yet pinned down
  precisely enough to commit.

## Derivation notes / open questions for whoever attempts the C

- The TWO `func_8001F8B8` calls are near-identical (`self->unk20`,
  `&scratch1`, `&scratch2`, literal `0`, `&scratch3` as the 5th
  stack-passed argument, a locally-built 3-entry `s16` angle table as the
  6th) -- almost certainly a shared subexpression/pattern worth writing
  as a single repeated statement with only the 6th argument's
  construction differing (`-0x400` vs `+0x400` on one axis), similar in
  spirit to this unit's other "manually unrolled 3x/2x repetition"
  functions this round (`func_8001E6F8`, `func_8001E600`).
- The delta-table construction before `slotA4` (`out[i] = arg2[i] -
  table[i]`, `u16` reads, `s16` stores) is structurally identical to
  `func_8001EA8C`'s OWN body (already matched, this unit) -- worth
  checking whether the SOURCE literally calls `func_8001EA8C` here too
  (with `s16`-truncated inputs) rather than reproducing its logic inline;
  the disassembly doesn't show a `jal` for this specific 3-word diff
  (matching an INLINED/duplicated computation, not a call), but this is
  worth double-checking against a real `cc1` build before committing to
  either reading.
- Given the size and the two blocking uncarved callees, this is a strong
  candidate to SPLIT into sub-attempts: get the two guard checks + list
  accumulation matching first (word count 0xEFE0..0xF154, roughly the
  first 96 bytes / 24 words), confirm against retail before tackling the
  `func_8001F8B8`-dependent tail, rather than attempting the whole 180
  words in one pass.

### Proposed learning

**When a function is disproportionately larger than the rest of its
queue (here, 60%+ bigger than the next-largest) and depends on
still-uncarved callees, a structural-analysis-only stall report --
field offsets, control flow, candidate types, open questions -- is a
legitimate and valuable contribution distinct from a "reached but didn't
match" stall.** It costs zero attempts against the 30-limit and lets the
next session start from working C immediately instead of re-reading 200
lines of MIPS. Distinguish this class explicitly from the "best-reached
body, restored to INCLUDE_ASM" class in `docs/PARALLEL-RUNS.md`'s own
staffing guidance, since the two need different next steps (write C vs.
read disassembly).
