# SsUtGetVVol -- MATCHED, 35/35 words byte-exact (round 38)

> Renamed from `func_80031D6C` on 2026-09-23 (tools/rename.py). Address 0x80031d6c.

Unit: `src/code_179d8_j_c.c` · vram `0x80031D6C` · file `0x2256C-0x225F8` · 35 words.

Closed in round 38. Whole-image SHA1 green; `funcdiff` 35/35 with zero
out-of-range bytes.

## Provenance

This was filed as a STALL since round 21 (2026-09-06), corrected for length in
round 31, and reassigned this round with a stale cross-reference in its own
line 5/9 ("`SsUtGetDetVVol`/`SsUtKeyOff` (blocked, elsewhere)") -- both are
now MATCHED (`SsUtGetDetVVol` in this same unit, `SsUtKeyOff` in
`libsnd_vm_vol_ut_key_ut_keyv`), which is corrected below.

## The C

```c
s32 SsUtGetVVol(s16 idx, s16 *out1, s16 *out2)
{
    EntryDAD4 *e;
    s16 f0, f2;

    if ((u16) idx < 0x18) {
        e = &D_8006DAD4[idx];
        f0 = e->unk0;
        f2 = e->unk2;
        *out1 = f0 / 129;
        *out2 = f2 / 129;
        return 0;
    }
    return -1;
}
```

The "divide by 129" sibling of `SsUtGetDetVVol`'s raw getter: same 0x18-entry
bounds check on `idx`, same cached `D_8006DAD4[idx]` entry pointer (confirmed
via `asm-differ` back in round 21 to be load-bearing over double-indexing),
but each field is divided by 129 before being written to `*out1`/`*out2`.
The divisor was confirmed arithmetically in round 21 (see that round's probe
in git history/prior report revision) against retail's magic-multiply
sequence -- not re-derived here, nothing about the divisor changed.

## The residue that mattered: load BOTH fields before dividing EITHER

Every round-21 attempt wrote the natural sequential form:

```c
*out1 = e->unk0 / 129;
*out2 = e->unk2 / 129;
```

This compiles the first statement to completion (load, magic-multiply divide,
store) before touching the second field at all -- confirmed by objdump: the
built `lhu` for `unk2` appeared *after* the `sh` that stores `*out1`. Retail's
own disassembly does the opposite: it loads **both** `lhu`s back-to-back
(interleaved only with the magic-multiply constant materialization), runs
**both** `mult`s, and only then does the two `sra`/`subu`/`sh` finishing
sequences together at the tail. Round 21's four attempts all varied
*declaration/assignment order of the pointer arguments* and *field-read
order*, but never varied *when the two loads happen relative to the two
divisions* -- that axis was never exercised.

Introducing two explicit `s16` temporaries (`f0`, `f2`) for the loaded field
values, assigned immediately after the entry pointer is computed and *before*
either division, reproduces retail's scheduling exactly: both `lhu`s emit
back-to-back, both `mult`s follow, and the register roles fall out identically
to retail --

- `$a1` (incoming `out1`) rescued into `$a3` **unconditionally, as the very
  first instruction**, not in the branch's delay slot.
- `$a2` (incoming `out2`) rescued into `$t0` in the bounds-check branch's
  delay slot.
- `$a0` (dead `idx`) reused as scratch for the first loaded field.
- `$a1` (freed by `out1`'s rescue) reused for the magic-multiply constant.
- `$a2` (freed by `out2`'s rescue) available and used as expected.

Every register assignment retail makes -- both rescues, both scratch reuses,
the constant placement -- came out correct with zero further changes needed.
This confirms round 21's own hypothesis was half right: the register-rescue
*choice* genuinely does not respond to declaration/assignment-order shuffles
of the pointer arguments (as round 21 exhaustively showed), but the residue
was never actually about the rescue targets at all -- it was about
**source-level evaluation order of the two loads relative to the two
divisions**, an axis round 21 never tried because both prior candidate
reshapes always computed field 1 completely before touching field 2.

## What this means for the classification

Round 21/31 filed this as a pure register-identity residue, citing CLAUDE.md
rule 6's "different registers, same instructions" test and stating "there is
no ORDER lever available here to try". **That was incomplete, not wrong about
the mechanism it tested**: the four reshapes tried really did all produce
identical output, and reordering the *statements already present* really
doesn't move the residue. What was missing was a reshape that changes which
values are *live simultaneously* -- forcing both loads to happen while both
are still needed, before either is consumed -- which is an ORDER change (of
evaluation, not of declaration) and is exactly the class of lever CLAUDE.md
permits (only *register-identity*-changing edits like `register T v
asm("$N")` are banned; this is ordinary C reordering that changes GCC's own
scheduling decision, and GCC's decision is what then determines register
identity as an effect, not a cause we forced).

## Verdict on the `SsUtSetDetVVol`/`SsUtSetVVol` "one class" question

Not applicable to this function's own closure (this one closed on a
load-scheduling lever, not the unused-frame lever that closed `SsUtSetDetVVol`)
-- see `SsUtSetVVol.md` for the verdict on that specific pairing, since this
function has no unused frame in its own prologue at all (no `addiu $sp` in its
`.s`) and was never claimed to share that residue class.

### Proposed learning

**A register-identity residue that resists declaration/statement-reordering
should next be checked against evaluation-order: does the built code compute
field A's entire result (load, transform, store) before loading field B at
all, while retail loads both up front?** If so, hoisting the loads into
explicit temporaries assigned before any of the per-field arithmetic is a
distinct lever from reordering the existing statements, and it is *not*
covered by "tried three declaration/assignment orderings, all identical" --
all three of round 21's reshapes kept the same per-field load-then-divide
grouping and only moved surrounding pointer-rescue statements around it.

## Round 97 (bravo, track 6)

The definition now takes <libsnd.h>'s prototype: return type `s32` became `s16` (Sony's `short`). Zero bytes.
The local `EntryDAD4 *e` became `SpuVoiceRegs *e = &D_8006DAD4->voice[idx]`
(fields `volL`/`volR`); see SsUtGetDetVVol.md for the SpuRegs evidence.
