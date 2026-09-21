> Renamed from `func_8005C7D4` on 2026-09-21 (tools/rename.py). Address 0x8005c7d4.

# TryDreamAuxTrigger

**Unit:** code_4cd08 · **Size:** 54 words · **Status:** MATCHED round 43
(54/54, byte-exact whole-image build).

## History

Filed BLOCKED in round 2026-08-30-a on one `%gp_rel` reference (to
`gDreamAuxStage`). Round 42 resolved the gp-relative blocker. Never actually
attempted -- the stub carried no derivation. Round 43 derived and matched it.

## What it does

Look up a table entry via `LookupDreamAuxTrigger`; if found, check a parity
condition via `CheckTriggerParity` and either dispatch into `FireDreamAuxTriggerEntries`
(returning its result) or -- on a small random/parity chance -- spawn via
`DespawnDreamAuxEntity`:

```c
extern s32 rand(void);

s32 LookupDreamAuxTrigger(s16 *a0);
bool CheckTriggerParity(s32 coordParity, s8 *entry);
s32 FireDreamAuxTriggerEntries(s32 a0, s32 a1, s32 a2);
void DespawnDreamAuxEntity(DreamAuxSlot *a0);

s32 TryDreamAuxTrigger(s32 a0, s16 *a1, s32 a2)
{
    s32 record = LookupDreamAuxTrigger(a1);

    if (record != 0) {
        if (CheckTriggerParity(a2, (s8 *)record)) {
            return FireDreamAuxTriggerEntries(a2, record, a0);
        }
        if (gDreamAuxStage != 0 && rand() % 12 == 0 && (a2 & 1) == 0) {
            DespawnDreamAuxEntity(gDreamAuxSlots);
        }
    }
    return 0;
}
```

`LookupDreamAuxTrigger`'s return value (a small-record pointer or 0, per that
function's own report) is kept as a plain `s32` and cast to `s8 *` only at
the point `CheckTriggerParity` needs it (that function's own signature takes a
raw `s8 *`, per its existing match report) -- there is no evidence either
way that this is a distinct pointer type worth naming, so it stays untyped
like `gDreamAuxWorld` elsewhere in this unit.

`FireDreamAuxTriggerEntries` is forward-declared here with a placeholder `(s32, s32,
s32)` signature to be filled in when that function (also queued this round)
is itself derived; its true parameter types must end up compatible with this
call site (`a2, record, a0` in that order).

The caller (`class_3bb8c_m.c`, a different unit) has its OWN typed view of
this function, `extern s32 TryDreamAuxTrigger(s32 arg0, s32 *arg1, void *arg2);`
(`include/class_3bb8c.h`) -- `s32 *` where this unit reads `s16 *`, and
`void *` where this unit treats the value as a plain `s32` bitmask (the `& 1`
parity test). Both views are internally consistent with their own unit's
usage and the mismatch is invisible at the ABI level (everything is a
32-bit register value); per CLAUDE.md's "multiple independent local views"
convention this unit keeps its own reading rather than importing
`class_3bb8c.h`.

## Derivation notes

One attempt short of byte-exact, one arithmetic-idiom fix:

- **First pass (23/54, 82195 bytes of drift):** wrote the guard as
  `rand() % 3 == 0`. This reproduces the general SHAPE of retail's
  reciprocal-multiply division sequence (same `mult`/`mfhi` skeleton already
  seen in `CheckDreamAuxTriggerCondition`'s `value % 3` checks) but not its CONTENT: retail's
  magic constant is `0x2AAAAAAB` (not `0x55555556`), and retail has an extra
  `sra $a0, $a0, 1` between the `mfhi` and the sign correction, plus a final
  `sll` by 2 (not 1) before the compare. All three are the signature of a
  **division by 12, not by 3** -- verified with the pinned toolchain
  reproducer (`tools/gcc263/cpp`/`cc1`/`maspsx`/`as`, per CLAUDE.md's
  "Escalate, do not experiment" recipe, used here purely to compare compiled
  C idioms, no toolchain flags touched): `rand() % 3`, `rand() % 6` and
  `rand() % 12` were each compiled in isolation, and only `% 12` reproduced
  retail's exact instruction sequence (constant, extra shift, and final
  shift amount all matched). The isolated reproducer even initially differed
  in its LAST two instructions (`subu`+`sltiu` vs retail's direct `bne`) --
  that difference was just the reproducer's own `return (cond);` idiom
  materializing a boolean where the real call site's `if (...)` guard
  branches directly; it went away once substituted into the real
  `if (gDreamAuxStage != 0 && rand() % 12 == 0 && (a2 & 1) == 0)` guard. Fixing
  the divisor alone (`% 3` -> `% 12`) reached byte-exact.

## Proposed learning

Add to the corpus: **a `rand() % N == 0`-shaped guard's reciprocal-multiply
sequence encodes N in three places at once (the magic constant, an optional
extra `sra` for factors of 2 beyond the first, and the final `sll` shift
amount), and getting any one of the three from "looks about right" is not
enough -- isolate the candidate divisors in the pinned-toolchain reproducer
and diff the instruction sequence directly rather than guessing from the
constant alone.** `0x55555556` is `/3`; `0x2AAAAAAB` is the same reciprocal
halved, and its correct divisor (`6`? `12`? more factors of 2?) is read off
the extra `sra` count and final `sll` shift, not off the constant.

## Naming

**TryDreamAuxTrigger** — tier B. Called externally from `class_3bb8c_m.c`
(`func_8005C7D4(child->unk4->unk34, &out, thing)` at the time of writing).
Looks up a trigger record by key (`LookupDreamAuxTrigger`); on a hit, either
dispatches it (`FireDreamAuxTriggerEntries`, whose result it returns) or, on
a small random/parity chance, silently despawns instead
(`DespawnDreamAuxEntity`); returns 0 on a miss or on the despawn branch.
"Try" reflects the function's own fallible, silently-returning-0 shape; the
broader game meaning of the (value, key, parity) triple it is handed is not
established from this unit alone, hence B not A.
