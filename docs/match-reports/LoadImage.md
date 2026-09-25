# LoadImage -- SDK identification (round 81, bravo, track 2)

> Renamed from `func_8002133C` on 2026-09-25 (tools/rename.py). Address 0x8002133c.

Sony's `LoadImage` (`libgpu/sys.o`). Uncarved `asm/psyq_11474.s` segment,
same GPU-trampoline region as `ClearImage`/`DrawSync`/`GsSortClear` above.

## Evidence

- **Fingerprint**: EXACT masked 1.00, shape 1.00, against `libgpu/sys` on
  all three discs. The only close competitor is `StoreImage` (SHAPE masked
  0.96) -- its sibling function immediately after this one at
  `func_800213A0`, which is the mirror-image swap (see that report); the
  EXACT tie for THIS address is unique to `LoadImage`.
- **Position**: same bracket as `ClearImage` (after `libgte/msc01` 3.3,
  before `libapi/c73` 3.3).
- **Header**: `include/psyq/LIBGPU.H`: `extern int LoadImage(RECT *rect, u_long *p);`

No C call site (`grep -rn LoadImage src/` -- no hits); no extern needed.
Renamed with `tools/rename.py`.
