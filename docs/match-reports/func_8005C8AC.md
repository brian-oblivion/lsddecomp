# func_8005C8AC

**Unit:** code_4cd08 · **Size:** 33 words · **Status:** MATCHED round 43
(33/33, byte-exact whole-image build).

## History

Filed BLOCKED in round 2026-08-30-a on one `%gp_rel` reference (to
`D_8008ABF8`) plus two `addiu_at` indexed loads. Round 42 resolved both
mechanisms. Never actually attempted -- the stub carried no derivation.
Round 43 derived and matched it fresh.

## What it does

A linear search over one of 14 parallel groups (selected by `D_8008ABF8`,
the same index this unit also uses in `func_8005C930`/`func_8005CD58`) for
an entry whose 2-byte `key` matches `*a0`, dispatching the match (or its
absence) into `func_8005C930`:

```c
typedef struct DreamAuxTriggerEntry {
    s16 key;
    u8 unk2[4];
} DreamAuxTriggerEntry;

extern s8 D_80089AC4[];
extern DreamAuxTriggerEntry *D_80089A8C[];

s32 func_8005C8AC(s16 *a0)
{
    s32 idx = D_8008ABF8;
    s32 count = D_80089AC4[idx];
    DreamAuxTriggerEntry *entry = D_80089A8C[idx];
    s32 i;

    for (i = 0; i < count; i++) {
        if (*a0 == entry->key) {
            return func_8005C930((s32)entry, i);
        }
        entry++;
    }
    return 0;
}
```

`D_80089AC4`/`D_80089A8C` is a second "count + pointer-to-array" parallel
family in this unit, structurally identical to the already-documented
`D_80089A7C`/`D_80089A44` (`func_8005C508`) but a different stride (6 bytes,
not 8) and a different index space (`D_8008ABF8`, not a loop counter). The
record type is named `DreamAuxTriggerEntry` since `func_8005C8AC`'s only
consumer of the match, `func_8005C930`, is itself part of this unit's
trigger-dispatch cluster (`func_8005CAB4`/`func_8005CBC8`/etc.).

## Derivation notes

One attempt short of byte-exact, one fix:

- **First pass (10/33, 129492 bytes of drift, one word too long):**
  `s8 count = D_80089AC4[idx];` produced a `lb` into `$v0` followed by a
  separate `move $a2, $v0` to get the value into the register the loop bound
  needs to live in across the whole function. Retail's `lb $a2, 0x0($at)`
  loads the byte DIRECTLY into `$a2` with no intermediate register at all.
  Widening the local from `s8` to `s32` (`s32 count = D_80089AC4[idx];`)
  removed the extra `move` and matched retail's direct-into-`$a2` load.
  Byte-exact immediately after.

This is a different mechanism from the `lbu`-vs-`lb` sign-extend idiom
documented for `func_8005CD58` earlier this round (that one was about load
INSTRUCTION CHOICE; here the load instruction was already `lb` in both
versions -- the difference was an extra copy into the register a
longer-lived variable needed to occupy). Both point the same direction
though: **for a byte-sized value that stays live across more than its
immediate use (here: both the initial bound check and the whole loop), widen
its C-level local to `s32` rather than leaving it `s8`/`s16`** -- a narrow
local invites the compiler to route the value through a scratch register
first.

## Proposed learning

Add to the corpus: **a byte/half-word value kept alive as a LOOP BOUND (not
just re-read once) should be declared as a wide (`s32`) local, even when the
source array element is narrower.** Symptom when this is missed: one extra
`move` from the load's natural destination register into the loop-bound
register, costing exactly one word and (because it's inside a small
function) enough to look like an unrelated residue rather than a type-width
issue.
