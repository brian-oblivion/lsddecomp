# CancelSeqTimer

> Renamed from `func_800329D8` on 2026-09-23 (tools/rename.py). Address 0x800329d8.

**Unit:** code_179d8_c · **Size:** 41 instructions · **Status:** MATCHED (41/41 words)

## What it does

A callback-registration teardown/re-arm routine, guarded by
`D_8006DCA8` (returns immediately, doing nothing, if it's nonzero).
Clears `D_8006DC94`, calls `func_80024CE0` (unconditional per-call
setup), then:

- if `D_8006DC8C` is set, calls `func_80024DA0(0)` and clears it;
- otherwise, if `D_8006DC90` isn't the sentinel `-1`: when it holds a
  real id (`!= 0`) deregisters via `func_80024D40(id, NULL)`, else
  registers the callback `D_8006DC9C` via `func_80024D40(0,
  D_8006DC9C)`; either way resets `D_8006DC90` to `-1` afterward.

Both paths fall through to a final `func_80024CF0()` call before
returning.

## The C

```c
extern void func_80024CE0(void);
extern void func_80024DA0(s32 arg0);
extern void func_80024D40(s32 arg0, void (*callback)(void));
extern void func_80024CF0(void);
extern s32 D_8006DCA8;
extern s32 D_8006DC94;
extern s32 D_8006DC8C;
extern s32 D_8006DC90;
extern void (*D_8006DC9C)(void);

void CancelSeqTimer(void)
{
    s32 v;

    if (D_8006DCA8 != 0) {
        return;
    }

    D_8006DC94 = 0;
    func_80024CE0();

    if (D_8006DC8C != 0) {
        func_80024DA0(0);
        D_8006DC8C = 0;
    } else {
        v = D_8006DC90;
        if (v != -1) {
            if (v != 0) {
                func_80024D40(v, NULL);
            } else {
                func_80024D40(0, D_8006DC9C);
            }
            D_8006DC90 = -1;
        }
    }

    func_80024CF0();
}
```

## Residue note: another instance of the `SeqTimerDividerCallback` block-order lever

First attempt wrote the if/else in "natural" order (`if (D_8006DC8C ==
0) { the D_8006DC90 logic } else { the func_80024DA0 teardown }`), which
compiled with the WRONG block placed inline: GCC 2.6.3 put the `==0`
branch inline/fallthrough and the `!=0` branch out-of-line, where retail
does the opposite (the `!=0`/teardown branch is the fallthrough, the
`==0`/re-arm branch is reached by a taken `beqz`). Swapping the C to test
`D_8006DC8C != 0` first (matching which block retail places first)
reproduced the exact instruction sequence with no other change --
consistent with the `SeqTimerDividerCallback` finding, though note this is a
*different* shape than that one: this is a full `if/else` where BOTH
arms do real work and rejoin at a shared tail (not a guard-clause-with-
early-return), so the "boundary" written up in `GetRCnt.md` /
`ResetRCnt.md` (block-order flip doesn't help guard-clauses) does
not contradict this -- this residue confirms the lever DOES apply to
plain two-sided `if/else` compiled from a raw loaded value, just not to
"if (range check fails) return fail;" guards.

### Proposed learning

`SeqTimerDividerCallback`'s block-order lever generalises to any two-sided `if
(rawValue) {A} else {B}` where GCC needs to decide which arm is
fallthrough vs. out-of-line -- not just single-sided toggles. The
distinguishing factor found so far across four instances this round: it
applies to raw-value/full-if-else shapes (`SeqTimerDividerCallback`,
`CancelSeqTimer`), NOT to guard-clause range-check-then-bail shapes
(`GetRCnt`, `ResetRCnt`, `SetRCnt`), where negative-first is
already what retail compiles to and flipping regresses.

## Provenance

round 16 (2026-09-04), runner delta, unit code_179d8_c (fresh carve,
second pass, head-directed follow-up). Matched second attempt (one
if/else block-order flip after the head's `SeqTimerDividerCallback` lever
generalisation request surfaced the same axis here).
