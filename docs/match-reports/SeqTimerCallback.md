# SeqTimerCallback

> Renamed from `func_80032A9C` on 2026-09-23 (tools/rename.py). Address 0x80032a9c.

**Unit:** code_179d8_c · **Size:** 13 instructions · **Status:** MATCHED (13/13 words)

## What it does

Calls through a global callback pointer if one is registered, then always
calls `func_80033738` (external, lives in the still-uncarved
`asm/code_179d8_tail.s` monolith -- declared `extern` locally per this
unit's own-declarations convention, not shared). `D_8006DC9C` is set
elsewhere (by `SeqTimerControl`, not attempted this round) to either `0` or
the address of a callback such as this very function's sibling
`SeqTimerDividerCallback`.

## The C

```c
extern void func_80033738(void);
extern void (*D_8006DC9C)(void);

void SeqTimerCallback(void)
{
    if (D_8006DC9C != NULL) {
        D_8006DC9C();
    }
    func_80033738();
}
```

## Provenance

round 16 (2026-09-04), runner delta, unit code_179d8_c (fresh carve).
Matched first attempt.
