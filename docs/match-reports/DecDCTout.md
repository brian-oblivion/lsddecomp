# DecDCTout -- SDK identification (round 81, bravo, track 2)

> Renamed from `func_80046000` on 2026-09-25 (tools/rename.py). Address 0x80046000.

Sony's `DecDCTout` (`libpress/libpress.o`). Uncarved `asm/psyq_36654.s`
segment, inside the `libpress` DecDCT-family run bracketed by `DecDCTin`
(`DecDCTin`) below and `DecDCToutCallback` (`func_80046084`) above,
both identified this round.

## Evidence

- **Fingerprint**: AMBIGUOUS -- 18 names tie at masked 1.00 on this 8-word
  body, four of them `libpress/libpress` functions (`DecDCTvlcSize`,
  `DecDCTout`, `DecDCTinSync`, `DecDCToutSync`). Position narrows the field
  from 18 to those four: this function sits inside a run whose immediate
  neighbours (`func_80045E54` = `DecDCTReset`, `func_80045F84` = `DecDCTin`,
  `func_80046568` = `DecDCTvlc`) are all confirmed unique `libpress` EXACT
  matches, so the address is `libpress`, not any of the other 14 tied names
  from unrelated libraries.
- **Call-target semantics decide among the four `libpress` candidates**: the
  body (`asm/psyq_36654.s`) is a bare stack-frame wrapper that forwards `$a0`
  and `$a1` UNCHANGED into `func_80046244` (an internal, not-yet-named
  helper in the same object) -- i.e. it is a two-argument passthrough, not a
  bare `int mode` passthrough. `func_80046244` itself uses both incoming
  registers substantively: it stores the first (`$s1` = `$a0`) into a
  work-buffer-pointer slot, and shifts the second (`$s0` = `$a1`) right by 5
  (`>> 5`, i.e. `/32`) before combining it into a DMA block-count word. That
  is exactly the `(buf, size)` shape of `DecDCTout(u_long *buf, int size)`
  from `include/psyq/libpress.h` -- a buffer pointer plus a byte count
  converted to 32-byte DMA blocks -- and rules out `DecDCTinSync(int mode)`
  and `DecDCToutSync(int mode)`, which take a single `int`, and
  `DecDCTvlcSize(u_long *bs)`, which takes a single pointer.
- **Header**: `include/psyq/libpress.h`:
  `extern void DecDCTout(u_long *buf, int size);`

Two independent evidence kinds (position narrowing the fingerprint tie to
`libpress`, plus call-target argument-count/shape semantics narrowing
within `libpress`) settle it as `DecDCTout`.

No C call site; no extern needed. Renamed with `tools/rename.py`.
