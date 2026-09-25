# DecDCTin -- SDK identification (round 81, bravo, track 2)

> Renamed from `func_80045F84` on 2026-09-25 (tools/rename.py). Address 0x80045f84.

Sony's `DecDCTin` (`libpress/libpress.o`). Uncarved `asm/psyq_36654.s`
segment, immediately after `DecDCTReset` (`func_80045E54`, this round) in
the `libpress` DecDCT-family run.

## Evidence

- **Fingerprint**: EXACT masked 1.00, shape 1.00, unique, against
  `libpress/libpress` on discs 3.3/3.5/3.6 (the 3.0 disc's build of the same
  function scores lower, masked 0.81 -- a different library build, not a
  competing name). Next unrelated candidate (`SpuQuit`) scores masked 0.23.
- **Position**: same `libpress` bracket as `DecDCTReset` (after
  `libgs/gs_107` 3.3, before `libc2/exit` 3.3).
- **Header**: `include/psyq/LIBPRESS.H`:
  `extern void DecDCTin(u_long *buf, int mode);`

No C call site; no extern needed. Renamed with `tools/rename.py`.
