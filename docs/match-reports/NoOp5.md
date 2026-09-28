# NoOp5

> Renamed from `func_80017EA8` on 2026-09-27 (tools/rename.py). Address 0x80017ea8.

**Unit:** BMemPMgr · **Size:** 1 instruction (`jr $ra`; splat pads the
delay slot) · **Status:** MATCHED (trivial; splat-generated, not decomp work)

## What it does

```c
void NoOp5(void)
{
}
```

An empty, zero-argument function. `jr $ra; nop` — the empty-body case
CLAUDE.md's "not every matched function was work" note describes.

## Naming (round 97)

`func_80017EA8` -> `NoOp5`, tier A (`tools/rename.py`, runner delta). An
empty body is its own complete mechanics, and FINISHING-PLAN's naming rules
make a pure leaf whose mechanics are its purpose tier A by definition; the
round-74 verdict below kept the placeholder only because no caller gave it
a purpose, which a no-op does not need. Follows the project's existing
free-function no-op names (`NoOp` in `GameApplicationFileResource.c`, `NoOp2`-`NoOp4` in
`cd_driver.c`); the suffix is the next free number, for disambiguation
only, and implies no link to those functions.

## Naming (round 74)

**Kept `func_80017EA8`, tier C: unnameable with current evidence.**
Checked all three places a caller could hide before giving up on it:

- No `jal func_80017EA8` anywhere in the remaining uncarved `asm/*.s`
  (recursive grep across `asm/`).
- No textual call in any carved `src/*.c`.
- No reference to its address `0x80017EA8` as a raw little-endian data
  word anywhere in the retail image (`disk/SLPS_015.56`), ruling out an
  as-yet-uncarved function-pointer table entry too.

So this function is, as far as anything currently decoded or scanned can
tell, **never called** — dead code shipped in retail. With an empty body
and no caller, there is nothing to name it FROM: no behavior beyond "does
nothing" and no call site to infer intent from. `func_80017AC8`'s
sibling comment in the splat yaml already flags it alongside
`func_80017A9C`/`func_80017AC8` as one of the unit's three frameless
leaf functions; it earns no name here.

## Provenance

Present in `src/app/bmem_pmgr.c` since the unit's initial carve (no report
previously filed — the FINISHING-PLAN note that a trivial `jr $ra; nop`
body is often not real decomp work applies here). round 74 (2026-09-24),
runner alpha: wrote this report as part of the unit's track-3 naming
pass, and confirmed via a full-image scan that it has no caller anywhere
in retail.
