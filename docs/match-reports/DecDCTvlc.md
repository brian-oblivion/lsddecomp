# DecDCTvlc -- SDK identification (round 81, bravo, track 2)

> Renamed from `func_80046568` on 2026-09-25 (tools/rename.py). Address 0x80046568.

Sony's `DecDCTvlc` (`libpress/libpress.o`). Uncarved `asm/psyq_36d48.s`
segment, the tail end of the `libpress` DecDCT-family run identified this
round.

## Evidence

- **Fingerprint**: EXACT masked 1.00, shape 1.00, unique, against
  `libpress/libpress` on disc 3.3. Next candidate (`GsSetFogParam`) scores
  masked 0.30 -- well below.
- **Position**: after placed `libc2/exit` (3.3, ends `0x80046548`), before
  placed `libpress/vlc` (3.3, starts `0x800465b8`) -- directly bracketed by
  a `libpress` object on one side.
- **Header**: `include/psyq/LIBPRESS.H`:
  `extern int DecDCTvlc(u_long *bs, u_long *buf);`

No C call site; no extern needed. Renamed with `tools/rename.py`.
