# CdDriver__StopService — MATCHED (12/12 words)

> Renamed from `Class6D4E8__StopCdService` on 2026-09-26 (tools/rename.py). Address 0x80027d40.

> Renamed from `func_80027D40` on 2026-09-17 (tools/rename.py). Address 0x80027d40.

Round 45, runner echo (second sitting), `src/code_179d8_q.c`. This class's
own method-table slot **+0x070** (see the class-map comment above
`GetCdDriverMethods` in the `.c`).

## Result

Byte-exact on the first attempt.

```c
extern void LockCd(void);
extern void UnlockCd(void);
extern void StopCdServiceIfIdle(void);

void CdDriver__StopService(void)
{
    LockCd();
    StopCdServiceIfIdle();
    UnlockCd();
}
```

## Derivation

Pure three-call sequence, no branches, no locals: set the `gCdLock` latch,
call `StopCdServiceIfIdle` (also matched this round, ROM-later so needed a forward
declaration), clear the latch. `StopCdServiceIfIdle` in turn is defined later in
this file since it sits at a higher ROM address, so `LockCd`,
`UnlockCd` and `StopCdServiceIfIdle` all needed forward `extern` prototypes
ahead of this definition to satisfy strict ROM-address file ordering.

### Proposed learning

None new — same "set latch / do work / clear latch" bracket pattern already
seen around `gCdLock` in this unit (`CdDriver__RequestLoadFile`, `DisableCdQueue`, now
this one), just with a different body in the middle.

## Naming

Round 51 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_80027D40` | `CdDriver__StopService` | B |

**Evidence.** Slot `+0x070` of `gCdDriverMethods`. The whole body is
`LockCd(); StopCdServiceIfIdle(); UnlockCd();` -- the locked wrapper around
this unit's `StopCdServiceIfIdle`, which unhooks the `VSyncCallback` and
clears `gCdCallbackInstalled`/`gCdQueueEnabled` when the state machine has
nothing pending. The name says exactly that: the class's "stop the CD
service" slot.

Tier B for the class token only (see `CdDriver__RequestLoadFile.md`); the
method's own behaviour is not in doubt.


Track 4, 2026-09-26 (round 88). The class of gCdDriverMethods (was D_8006D4E8, id 0x13 = DATASOURCE_CD) is CdDriver, in include/CdDriver.h: its ctor calls InitCdDrive, its slots enqueue CD_OP_* requests and drive the CD read state machine, and it is VabDriver's sibling. The object views this function was typed against are replaced by CdDriver, whose fields are all Class6D430's (the driver runs on its clients' objects; Class6D430's +0x018/+0x01C were named pos/size for it). Byte-identical. `Class6D4E8__StopCdService` -> `CdDriver__StopService` by rename.py. Renamed from StopCdService for its slot, +0x070 stopService (VabDriver__StopService fills the same slot). Its one caller, CdDriver__RunRequestQueue, passes `self` (retail loads $a0 before the jalr) while Class6D430's slot is `void (*)(void)`; that call site casts through StopServiceSelfFn (no code).
