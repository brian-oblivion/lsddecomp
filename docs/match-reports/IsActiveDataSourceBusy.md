# IsActiveDataSourceBusy

> Renamed from `func_80026E64` on 2026-09-18 (tools/rename.py). Address 0x80026e64.

**Unit:** game_shell · **Size:** 13 words · **Status:** MATCHED, round 43
(2026-09-15, runner bravo). 13/13 words, byte-exact whole-image build.

## History

Stalled round 2026-08-29-a as class TOOLCHAIN (`gp_rel`), same mechanism as
`LockActiveDataSource`. Round 42 resolved `gp_rel` project-wide. Round 43 rebuilt
the preserved body verbatim; matched on the first build.

## What it does

`s32 IsActiveDataSourceBusy(void) { if (sActiveDataSource == 0x13) return IsCdBusy(); return 0; }`
— same region/mode gate as `LockActiveDataSource`, but forwarding the callee's
return value (or a fixed `0` on the other path) instead of returning `void`.
`IsCdBusy` is a still-uncarved function in `asm/code_179d8.s`.

## Final body

```c
extern s32 IsCdBusy(void);

s32 IsActiveDataSourceBusy(void) {
    if (sActiveDataSource == 0x13) {
        return IsCdBusy();
    }
    return 0;
}
```

## Proposed learning

See `LockActiveDataSource.md` — same family. Confirms the CLAUDE.md caution about
tail-call return types: this function's shape (fixed constant on the
false-path, callee's return on the true-path) is direct positive evidence the
function is non-void, not an assumption.

## Naming

Round 52 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_80026E64` | `IsActiveDataSourceBusy` | B |

**Evidence.** Forwards to `IsCdBusy()` when the CD driver is active, else
always reports not-busy (`0`). Same family as `LockActiveDataSource`.
