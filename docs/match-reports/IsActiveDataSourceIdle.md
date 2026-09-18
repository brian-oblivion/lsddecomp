> Renamed from `func_80026E98` on 2026-09-18 (tools/rename.py). Address 0x80026e98.

# IsActiveDataSourceIdle

**Unit:** code_171e0 · **Size:** 13 words · **Status:** MATCHED, round 43
(2026-09-15, runner bravo). 13/13 words, byte-exact whole-image build.

## History

Stalled round 2026-08-29-a as class TOOLCHAIN (`gp_rel`), same mechanism as
`LockActiveDataSource`. Round 42 resolved `gp_rel` project-wide. Round 43 rebuilt
the preserved body verbatim; matched on the first build.

## What it does

`s32 IsActiveDataSourceIdle(void) { if (gActiveDataSource == 0x13) return IsCdIdle(); return 1; }`
— same shape as `IsActiveDataSourceBusy`, but the "not in mode 0x13" path returns `1`
instead of `0`. `IsCdIdle` is a still-uncarved function in
`asm/code_179d8.s`.

## Final body

```c
extern s32 IsCdIdle(void);

s32 IsActiveDataSourceIdle(void) {
    if (gActiveDataSource == 0x13) {
        return IsCdIdle();
    }
    return 1;
}
```

## Proposed learning

See `LockActiveDataSource.md` — same family.

## Naming

Round 52 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_80026E98` | `IsActiveDataSourceIdle` | B |

**Evidence.** Forwards to `IsCdIdle()` when the CD driver is active, else
always reports idle (`1`). Same family.
