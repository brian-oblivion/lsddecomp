> Renamed from `func_80026E38` on 2026-09-18 (tools/rename.py). Address 0x80026e38.

# UnlockActiveDataSource

**Unit:** code_171e0 · **Size:** 11 words · **Status:** MATCHED, round 43
(2026-09-15, runner bravo). 11/11 words, byte-exact whole-image build.

## History

Stalled round 2026-08-29-a as class TOOLCHAIN (`gp_rel`), same mechanism as
`LockActiveDataSource` (see that report for the full reproducer). Round 42 resolved
`gp_rel` project-wide. Round 43 rebuilt the preserved body verbatim; matched
on the first build.

## What it does

`if (gActiveDataSource == 0x13) { UnlockCd(); }` — same shape as
`LockActiveDataSource`, forwarding to a different still-uncarved function
(`UnlockCd`, in `asm/code_179d8.s`).

## Final body

```c
extern s32 UnlockCd(void);

void UnlockActiveDataSource(void) {
    if (gActiveDataSource == 0x13) {
        UnlockCd();
    }
}
```

(`extern s32 gActiveDataSource;` is declared once, above `LockActiveDataSource` earlier in
this file.)

## Proposed learning

See `LockActiveDataSource.md` — same six-function family, same finding.
