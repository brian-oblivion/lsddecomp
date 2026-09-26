# CdRead2 -- SDK identification (round 81, bravo, track 2)

> Renamed from `func_800479C8` on 2026-09-25 (tools/rename.py). Address 0x800479c8.

Sony's `CdRead2` (`libcd/cdread2.o`). Uncarved `asm/psyq_381c8.s` segment.

## Evidence

- **Fingerprint**: EXACT masked 1.00, shape 1.00, unique, against
  `libcd/cdread2` on discs 3.5 and 3.6 only -- the 3.3 disc's build does not
  score as a candidate at all (no 3.3 `CdRead2` entry appears; the module
  was evidently reworked between 3.3 and 3.5). Next unrelated candidate
  (`GsMulCoord0`) scores masked 0.16.
- **Position**: after placed `libcd/c_003` (3.3, ends `0x800479c8`), before
  placed `libcd/c_005` (3.3, starts `0x80047a5c`) -- both bracketing objects
  are `libcd`, consistent with a `libcd` module here, though the DISC
  differs from its 3.3 neighbours. This is an instance of the "game mixed
  library builds" pattern CLAUDE.md documents (`libetc` is disc 3.5,
  `libgpu`/`libcd` elsewhere carry RCS ids matching neither 3.5 nor 3.6) --
  which disc owns which object is measured per-function, not inferred from
  neighbours.
- **Header**: not in `include/psyq/libcd.h` -- `CdRead2` is an
  undocumented/internal `libcd` export (no public prototype shipped).

Fingerprint (unique EXACT, unambiguous) plus position (bracketed by
`libcd` objects) settle it as `CdRead2` despite the disc mismatch with its
neighbours, per the documented mixed-build pattern.

No C call site; no extern needed. Renamed with `tools/rename.py`.
