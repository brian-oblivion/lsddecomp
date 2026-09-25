# StoreImage -- SDK identification (round 81, bravo, track 2)

> Renamed from `func_800213A0` on 2026-09-25 (tools/rename.py). Address 0x800213a0.

Sony's `StoreImage` (`libgpu/sys.o`). Uncarved `asm/psyq_11474.s` segment,
immediately after `LoadImage` (`func_8002133C`, this round) -- the natural
Load/Store pair.

## Evidence

- **Fingerprint**: EXACT masked 1.00, shape 1.00, against `libgpu/sys` on
  all three discs. Closest competitor `LoadImage` (SHAPE masked 0.96) is
  exactly its sibling above -- the EXACT tie for this address is unique to
  `StoreImage`.
- **Position**: same bracket (after `libgte/msc01` 3.3, before `libapi/c73`
  3.3).
- **Header**: `include/psyq/LIBGPU.H`: `extern int StoreImage(RECT *rect, u_long *p);`

No C call site; no extern needed. Renamed with `tools/rename.py`.
