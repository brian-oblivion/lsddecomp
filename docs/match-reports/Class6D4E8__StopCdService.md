> Renamed from `func_80027D40` on 2026-09-17 (tools/rename.py). Address 0x80027d40.

# Class6D4E8__StopCdService — MATCHED (12/12 words)

Round 45, runner echo (second sitting), `src/code_179d8_q.c`. This class's
own method-table slot **+0x070** (see the class-map comment above
`GetClass6D4E8Methods` in the `.c`).

## Result

Byte-exact on the first attempt.

```c
extern void LockCd(void);
extern void UnlockCd(void);
extern void StopCdServiceIfIdle(void);

void Class6D4E8__StopCdService(void)
{
    LockCd();
    StopCdServiceIfIdle();
    UnlockCd();
}
```

## Derivation

Pure three-call sequence, no branches, no locals: set the `D_8008A88C` latch,
call `StopCdServiceIfIdle` (also matched this round, ROM-later so needed a forward
declaration), clear the latch. `StopCdServiceIfIdle` in turn is defined later in
this file since it sits at a higher ROM address, so `LockCd`,
`UnlockCd` and `StopCdServiceIfIdle` all needed forward `extern` prototypes
ahead of this definition to satisfy strict ROM-address file ordering.

### Proposed learning

None new — same "set latch / do work / clear latch" bracket pattern already
seen around `D_8008A88C` in this unit (`Class6D4E8__RequestLoadFile`, `DisableCdQueue`, now
this one), just with a different body in the middle.
