# GetActiveDataSourceState

> Renamed from `func_80026F00` on 2026-09-18 (tools/rename.py). Address 0x80026f00.

**Unit:** GameApplicationFileResource · **Size:** 13 words · **Status:** MATCHED, round 43
(2026-09-15, runner bravo). 13/13 words, byte-exact whole-image build.

## History

Stalled round 2026-08-29-a as class TOOLCHAIN (`gp_rel`), same mechanism as
`LockActiveDataSource`. Round 42 resolved `gp_rel` project-wide. Round 43 rebuilt
the preserved body verbatim; matched on the first build.

## What it does

`s32 GetActiveDataSourceState(void) { if (gActiveDataSource == 0x13) return GetCdState(); return 0; }`
— same shape as `IsActiveDataSourceBusy`, forwarding to a different still-uncarved
function (`GetCdState`, in `asm/code_179d8.s`).

## Final body

```c
extern s32 GetCdState(void);

s32 GetActiveDataSourceState(void) {
    if (gActiveDataSource == 0x13) {
        return GetCdState();
    }
    return 0;
}
```

## Proposed learning

See `LockActiveDataSource.md` — last of the six-function family
(`LockActiveDataSource`/`E38`/`E64`/`E98`/`ECC`/`F00`). All six matched on the first
build once `gp_rel` was resolved, with zero source-shape changes from the
round-2026-08-29-a preserved bodies — strong confirmation this really was a
pure toolchain block, not a source-shape problem.

## Naming

Round 52 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_80026F00` | `GetActiveDataSourceState` | B |

**Evidence.** Forwards to `GetCdState()` when the CD driver is active, else
`0`. Same family.
