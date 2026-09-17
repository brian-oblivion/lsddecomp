> Renamed from `func_800281B0` on 2026-09-17 (tools/rename.py). Address 0x800281b0.

# StartCdService — MATCHED (26/26 words)

Round 45, runner echo (second sitting), `src/code_179d8_q.c`.

## Result

Byte-exact on the first attempt.

```c
extern s32 gCdCallbackInstalled;

void StartCdService(void)
{
    LockCd();

    if (gCdCallbackInstalled == 0) {
        if (gCdUseVSyncCallback != 0) {
            VSyncCallback((void (*)(void))ServiceCdDriver);
        }
        gCdCallbackInstalled = 1;
    }

    gCdQueueEnabled = 1;
    UnlockCd();
}
```

## Derivation

Set the `gCdLock` latch, then a one-shot guard on `gCdCallbackInstalled`: if it's
still 0, optionally register `ServiceCdDriver` as a `VSyncCallback` (guarded
by `gCdUseVSyncCallback`) and set the guard to 1. Either way, set `gCdQueueEnabled = 1`
and clear the latch. All three loads/stores collapse to constant `1`s in
the disassembly (every `sw` in this function stores a literal `ori
$v0,$zero,0x1` value, never a loaded one) — reading the raw instruction
stream as "track v0 across branches" is a trap here (see
`ServiceCdDriver`'s report for the general form of that trap); the direct
translation to nested `if`s with literal assignments is what matches.

`ServiceCdDriver`'s own report already covers the `VSyncCallback` cast and
the `D_8006D4E8` own-slot dispatch it performs; this function is simply one
of its two registration call sites (`StopCdServiceIfIdle` is the other, guarding
the same fields in a near-identical shape but written as its own
independent nested-if — not factored, since the two bodies are close but not
identical).

### Proposed learning

None new beyond what `ServiceCdDriver`'s report already states.
