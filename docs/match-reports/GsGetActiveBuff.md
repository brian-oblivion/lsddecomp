# GsGetActiveBuff -- SDK identification (round 81, bravo, track 2)

> Renamed from `func_80024820` on 2026-09-25 (tools/rename.py). Address 0x80024820.

Sony's `GsGetActiveBuff` (`libgs/gs_002.o`). Uncarved `asm/psyq_15020.s`
segment.

## Evidence

- **Fingerprint**: TINY (4 words) -- `tools/sdkname.py` flags a bare EXACT
  tie on a body this small as not identification alone. The tie is between
  `GsGetActiveBuff` (libgs/gs_002) and `SsUtGetReverbType` (libsnd/ut_rev),
  both masked 1.00.
- **Position**: after placed `libgs/gs_121` (3.3, ends `0x80024820`), before
  placed `libgs/gs_010` (3.3, starts `0x8002494c`) -- both neighbours are
  `libgs` objects, which favours `GsGetActiveBuff` over the `libsnd`
  candidate.
- **Body semantics**: the function reads global `D_80090B78` (a halfword)
  and returns it, touching nothing else -- a bare state-variable getter. The
  very next function in the same segment (`func_80024830`, not part of this
  round's assignment) also reads `D_80090B78` alongside a second global
  `D_80090B80`, i.e. `D_80090B78` is shared internal `libgs` buffer-tracking
  state, not a sound-driver reverb-type setting. This corroborates
  `GsGetActiveBuff` (which reports the currently active draw/display buffer
  index) over `SsUtGetReverbType`.

Two independent evidence kinds (fingerprint tie broken by position, plus
body/sibling semantics) settle it as `GsGetActiveBuff`.

- **Header**: `include/psyq/LIBGS.H`: `int GsGetActiveBuff(void);`

No C call site; no extern needed. Renamed with `tools/rename.py`.
