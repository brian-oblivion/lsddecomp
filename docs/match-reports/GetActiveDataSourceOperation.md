> Renamed from `func_80026ECC` on 2026-09-18 (tools/rename.py). Address 0x80026ecc.

# GetActiveDataSourceOperation

**Unit:** code_171e0 · **Size:** 13 words · **Status:** MATCHED, round 43
(2026-09-15, runner bravo). 13/13 words, byte-exact whole-image build.

## History

Stalled round 2026-08-29-a as class TOOLCHAIN (`gp_rel`), same mechanism as
`LockActiveDataSource`. Round 42 resolved `gp_rel` project-wide. Round 43 rebuilt
the preserved body verbatim; matched on the first build.

## What it does

`s32 GetActiveDataSourceOperation(void) { if (gActiveDataSource == 0x13) return GetCdOperation(); return 0; }`
— same shape as `IsActiveDataSourceBusy`, forwarding to a different still-uncarved
function (`GetCdOperation`, in `asm/code_179d8.s`).

## Final body

```c
extern s32 GetCdOperation(void);

s32 GetActiveDataSourceOperation(void) {
    if (gActiveDataSource == 0x13) {
        return GetCdOperation();
    }
    return 0;
}
```

## Proposed learning

See `LockActiveDataSource.md` — same family.

## Naming

Round 52 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_80026ECC` | `GetActiveDataSourceOperation` | B |

**Evidence.** Forwards to `GetCdOperation()` when the CD driver is active,
else `0`. Same family; `GetCdOperation`'s own name is Sony's / already
established, carried straight through to the generalised wrapper's name.
