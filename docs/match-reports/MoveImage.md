# MoveImage -- SDK identification (round 81, bravo, track 2)

> Renamed from `func_80021404` on 2026-09-25 (tools/rename.py). Address 0x80021404.

Sony's `MoveImage` (`libgpu/sys.o`). Uncarved `asm/psyq_11474.s` segment,
same GPU-trampoline region.

## Evidence

- **Fingerprint**: EXACT masked 1.00, shape 1.00, unique, against
  `libgpu/sys` on all three discs. Next candidate (`PutDrawEnv`) scores
  masked 0.18 -- far below.
- **Position**: same bracket (after `libgte/msc01` 3.3, before `libapi/c73`
  3.3).
- **Header**: `include/psyq/LIBGPU.H`:
  `extern int MoveImage(RECT *rect, int x, int y);`

No C call site; no extern needed. Renamed with `tools/rename.py`.
