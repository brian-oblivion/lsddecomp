# StartCdService — MATCHED (26/26 words)

> Renamed from `func_800281B0` on 2026-09-17 (tools/rename.py). Address 0x800281b0.

Round 45, runner echo (second sitting), `src/code_179d8_q.c`.

## Result

Byte-exact on the first attempt.

```c
extern s32 sCdCallbackInstalled;

void StartCdService(void)
{
    LockCd();

    if (sCdCallbackInstalled == 0) {
        if (gCdUseVSyncCallback != 0) {
            VSyncCallback((void (*)(void))ServiceCdDriver);
        }
        sCdCallbackInstalled = 1;
    }

    sCdQueueEnabled = 1;
    UnlockCd();
}
```

## Derivation

Set the `sCdLock` latch, then a one-shot guard on `sCdCallbackInstalled`: if it's
still 0, optionally register `ServiceCdDriver` as a `VSyncCallback` (guarded
by `gCdUseVSyncCallback`) and set the guard to 1. Either way, set `sCdQueueEnabled = 1`
and clear the latch. All three loads/stores collapse to constant `1`s in
the disassembly (every `sw` in this function stores a literal `ori
$v0,$zero,0x1` value, never a loaded one) — reading the raw instruction
stream as "track v0 across branches" is a trap here (see
`ServiceCdDriver`'s report for the general form of that trap); the direct
translation to nested `if`s with literal assignments is what matches.

`ServiceCdDriver`'s own report already covers the `VSyncCallback` cast and
the `gCdDriverMethods` own-slot dispatch it performs; this function is simply one
of its two registration call sites (`StopCdServiceIfIdle` is the other, guarding
the same fields in a near-identical shape but written as its own
independent nested-if — not factored, since the two bodies are close but not
identical).

### Proposed learning

None new beyond what `ServiceCdDriver`'s report already states.

## Naming

Round 51 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_800281B0` | `StartCdService` | A |
| `D_8008A89C` | `sCdCallbackInstalled` | A |

**Evidence.** Under the lock: if `sCdCallbackInstalled` is 0, register
`ServiceCdDriver` with `VSyncCallback` (when the VSync path is selected) and
set the flag; then enable queue processing. Its one caller is
`EnqueueCdRequest`, immediately after appending a node -- "a request now
exists, make sure the service is running". Start is what it does; the
one-shot flag is what stops it doing it twice.

`sCdCallbackInstalled` is written 1 exactly where the callback is registered
and 0 exactly where `StopCdServiceIfIdle` clears it, and is read nowhere
else. Tier A.

## Track 7 (round 101, echo): comments moved here, and names

`gCdCallbackInstalled` -> `sCdCallbackInstalled` and `gCdQueueEnabled` ->
`sCdQueueEnabled` (`tools/rename.py`): only code_179d8_q accesses either
(StartCdService, StopCdServiceIfIdle, DisableCdQueue, ServiceCdDriver), so
they are unit-static data, `sName`. The meanings stand as named: the
first is set once the tick is installed and cleared when
StopCdServiceIfIdle removes it; the second gates ServiceCdDriver's
runRequestQueue call.
