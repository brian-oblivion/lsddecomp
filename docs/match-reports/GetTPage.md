# GetTPage -- SDK identification (round 81, bravo, track 2)

> Renamed from `func_80023F08` on 2026-09-25 (tools/rename.py). Address 0x80023f08.

Sony's `GetTPage` (`libgpu/prim.o`). Uncarved `asm/psyq_140dc.s` segment.

## Evidence

- **Fingerprint**: EXACT masked 1.00, shape 1.00, unique, against
  `libgpu/prim` on all three discs. Next candidate (`_SsContResetAll`)
  scores masked 0.10 -- far below.
- **Position**: same bracket as `GsInitGraph` (after `libc2/memcpy` 3.3,
  before `libgs/gs_103` 3.3) -- the GPU-primitive helper module sits inside
  the same gap as `GsInitGraph`.
- **Header**: `include/psyq/LIBGPU.H`:
  `extern u_short GetTPage(int tp, int abr, int x, int y);`

No C call site; no extern needed. Renamed with `tools/rename.py`.
