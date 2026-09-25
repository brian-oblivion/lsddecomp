# DecDCTReset -- SDK identification (round 81, bravo, track 2)

> Renamed from `func_80045E54` on 2026-09-25 (tools/rename.py). Address 0x80045e54.

Sony's `DecDCTReset` (`libpress/libpress.o`). Uncarved `asm/psyq_36654.s`
segment, the start of a `libpress` DecDCT-family run that continues through
`func_80045F84` (`DecDCTin`), `func_80046000` (`DecDCTout`),
`func_80046084` (`DecDCToutCallback`) and `func_80046568` (`DecDCTvlc`),
all identified this round -- see their own reports.

## Evidence

- **Fingerprint**: EXACT masked 1.00, shape 1.00, unique, against
  `libpress/libpress` on all three discs. Next candidate (`GetDrawEnv`)
  scores masked 0.57 -- well below.
- **Position**: after placed `libgs/gs_107` (3.3, ends `0x80043008`), before
  placed `libc2/exit` (3.3, starts `0x80046538`) -- a wide gap that this
  round's cluster of five identifications fills entirely with `libpress`.
- **Header**: `include/psyq/LIBPRESS.H`: `extern void DecDCTReset(int mode);`

No C call site; no extern needed. Renamed with `tools/rename.py`.
