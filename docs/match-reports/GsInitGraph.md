# GsInitGraph -- SDK identification (round 81, bravo, track 2)

> Renamed from `func_800238DC` on 2026-09-25 (tools/rename.py). Address 0x800238dc.

Sony's `GsInitGraph` (`libgs/gs_001.o`). Uncarved `asm/psyq_140dc.s`
segment, the same region round 53 identified `GsSortClear` in.

## Evidence

- **Fingerprint**: EXACT masked 1.00, shape 1.00, unique, against
  `libgs/gs_001` on discs 3.3/3.5 (and again on 3.6). Next candidate
  (`__fixdfdi`, libsn) scores masked 0.17 -- far below.
- **Position**: after placed `libc2/memcpy` (3.3, ends `0x800238dc`), before
  placed `libgs/gs_103` (3.3, starts `0x80024734`) -- both disc 3.3.
- **Header**: `include/psyq/LIBGS.H`:
  `void GsInitGraph(unsigned short x, unsigned short y, unsigned short intmode, unsigned short dith, unsigned short vrammode);`

No C call site; no extern needed. Renamed with `tools/rename.py`.
