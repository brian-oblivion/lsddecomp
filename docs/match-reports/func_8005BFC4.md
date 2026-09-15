# func_8005BFC4 — MATCHED 6/6

**Unit:** DreamSys · **Size:** 6 words · **Status:** MATCHED, round 43.

## History

Filed round 2026-08-30-a as BLOCKED on `gp_rel` (sole reference `D_8008ACBC`
via `%gp_rel`). Round 42 RESOLVED that blocker with
`--gp-symbols`/`--no-nop-mflo-mfhi`. Rebuilt fresh this round and matched on
the first attempt.

## What it does

`return (D_8008ACBC == 0) ? 0xA : 0;` -- the asm computes this via
`sltiu`/`negu`/`andi` rather than a branch (an unsigned "is zero" test turned
into an all-ones mask, then masked to `0xA`), which is exactly what GCC 2.6.3
emits for a ternary on a simple equality-to-zero test; no special shape was
needed to reproduce it.

Called by `func_8005A82C` (still `INCLUDE_ASM`, see that function's own
report) as `saved = func_8005BFC4();` with no arguments -- consistent with
the existing header prototype `extern s32 func_8005BFC4(void);`.

## Final body

```c
s32 func_8005BFC4(void)
{
	return (D_8008ACBC == 0) ? 0xA : 0;
}
```

`D_8008ACBC` was already declared `extern s32 D_8008ACBC;` in `include/DreamSys.h`.

## Verification

`./build-and-verify.sh` -> `build exit=0`, whole-image SHA1 matches retail.
`tools/funcdiff.py func_8005BFC4` -> `6/6 words match`.

## Provenance

round 43, runner ALPHA, unit DreamSys.
