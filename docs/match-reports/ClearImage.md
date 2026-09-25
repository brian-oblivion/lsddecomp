# ClearImage -- SDK identification (round 81, bravo, track 2)

> Renamed from `func_800212A8` on 2026-09-25 (tools/rename.py). Address 0x800212a8.

Sony's `ClearImage` (`libgpu/sys.o`). Not game code; lives in the uncarved
`asm/psyq_11474.s` segment, the same region round 53 identified `DrawSync`
(`func_80021114`) and `GsSortClear` (`func_80023DA0`) in.

## Evidence

- **Fingerprint**: `tools/sdkname.py ClearImage` -- EXACT masked 1.00,
  shape 1.00, unique, against `libgpu/sys` on all three discs (3.3, 3.5,
  3.6). Next candidate (`_make_packet`, libgs/2d_com1) scores masked 0.24 --
  far below.
- **Position**: after placed `libgte/msc01` (3.3, ends `0x800206e0`), before
  placed `libapi/c73` (3.3, starts `0x80023898`) -- both bracketing objects
  are disc 3.3.
- **Header**: `include/psyq/LIBGPU.H`:
  `extern int ClearImage(RECT *rect, u_char r, u_char g, u_char b);`

No C call site exists yet (`grep -rn ClearImage src/` -- no hits); the
function is only reached from `INCLUDE_ASM` game assembly not yet carved, so
no local `extern` is needed. Renamed with `tools/rename.py`.
