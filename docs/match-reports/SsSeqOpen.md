# SsSeqOpen -- SDK identification (round 81, bravo, track 2)

> Renamed from `func_8003A99C` on 2026-09-25 (tools/rename.py). Address 0x8003a99c.

Sony's `SsSeqOpen` (`libsnd/seqinit.o`). Uncarved `asm/psyq_2ae70.s`
segment.

## Evidence

- **Fingerprint**: EXACT masked 1.00, shape 1.00, unique, against
  `libsnd/seqinit` on discs 3.3/3.5. Next candidate (`CdReadSync`) scores
  masked 0.10 -- far below.
- **Position**: after placed `libsnd/vol` (3.3, ends `0x8003a670`), before
  placed `libsnd/ssplay` (3.3, starts `0x8003aa68`) -- both `libsnd`
  objects, consistent with a `libsnd` module in between.
- **Header**: `include/psyq/LIBSND.H`:
  `extern short SsSeqOpen (unsigned long*, short);`

No C call site; no extern needed. Renamed with `tools/rename.py`.
