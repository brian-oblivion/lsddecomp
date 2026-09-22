# SetInstantTeleportersEnabled — MATCHED 3/3

> Renamed from `func_8005BF68` on 2026-09-22 (tools/rename.py). Address 0x8005bf68.

**Unit:** DreamSys · **Size:** 3 words · **Status:** MATCHED, round 43.

## History

Filed round 2026-08-30-a as BLOCKED on `gp_rel` (the sole reference is
`D_8008ABE4` via `%gp_rel`). Round 42 (2026-09-15) RESOLVED that blocker with
`--gp-symbols`/`--no-nop-mflo-mfhi` (see `docs/research/gp-relative-blocker.md`,
"RESOLVED"). This round rebuilt the function fresh under the fixed toolchain
and it matched on the first attempt -- no re-derivation needed, the mechanism
this function hits was already fully understood, only the toolchain flag was
missing.

## What it does

A one-instruction setter: stores its argument into the global flag
`D_8008ABE4`. `code_4cd08.c` calls it as `SetInstantTeleportersEnabled(bool value)` with
either a computed boolean expression or the literal `1` -- consistent with a
plain `bool` parameter here.

## Final body

```c
/* Flag set here, tested by Test4InstantTeleporters right below; local to
   this unit -- code_4cd08.c calls the setter through its own extern
   (`extern void SetInstantTeleportersEnabled(bool value);`), never touches the flag
   directly. */
extern s32 D_8008ABE4;

void SetInstantTeleportersEnabled(bool value)
{
	D_8008ABE4 = value;
}
```

`D_8008ABE4` is declared `extern s32` (not `bool`) because it is also read as
a plain nonzero/zero flag by `Test4InstantTeleporters` (next in ROM order),
and its underlying data is a full 32-bit word (`asm/data/7B3C0.sdata.s`,
`D_8008ABE4`, one `.word`).

## Verification

`./build-and-verify.sh` -> `build exit=0`, whole-image SHA1 matches retail.
`tools/funcdiff.py SetInstantTeleportersEnabled` -> `3/3 words match`.

### Proposed learning

None beyond what round 42's blanket note already says: every `gp_rel`-only
stub in this unit's queue is now a plain rebuild-and-confirm, not a
re-derivation.

## Provenance

round 43, runner ALPHA, unit DreamSys.
