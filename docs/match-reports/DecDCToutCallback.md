# DecDCToutCallback -- SDK identification (round 81, bravo, track 2)

> Renamed from `func_80046084` on 2026-09-25 (tools/rename.py). Address 0x80046084.

Sony's `DecDCToutCallback` (`libpress/libpress.o`). Uncarved
`asm/psyq_36654.s` segment, inside the `libpress` DecDCT-family run.

## Evidence

- **Fingerprint**: AMBIGUOUS -- two EXACT ties at masked 1.00 on this
  9-word body: `CdGetToc` (libcd/toc) and `DecDCToutCallback`
  (libpress/libpress).
- **Position**: inside the same `libpress`-confirmed run as
  `DecDCTReset`/`DecDCTin`/`DecDCTout` (all identified this
  round), which favours `DecDCToutCallback` over the unrelated `libcd`
  candidate.
- **Call-target semantics decide it outright**: the body forwards its
  incoming `$a0` (a function pointer) into `$a1` and calls the already-named
  `DMACallback(int dmaChannel, void (*func)())` (`libetc/intr.o`, pinned in
  `config/psyq-objects.ld`) with `$a0 = 1`. PS1 DMA channel 1 is MDEC-out.
  `CdGetToc(CdlLOC *tocbuf)` takes a struct-pointer output argument and does
  not call `DMACallback` at all, so it is ruled out; the sibling wrapper at
  `func_80046060` (channel 0 = MDEC-in, not part of this round's
  assignment) is the matching `DecDCTinCallback`.
- **Header**: `include/psyq/libpress.h`:
  `extern int DecDCToutCallback(void (*func)());`

Two independent evidence kinds (position + call-target/DMA-channel
semantics) settle it as `DecDCToutCallback`.

No C call site; no extern needed. Renamed with `tools/rename.py`.
