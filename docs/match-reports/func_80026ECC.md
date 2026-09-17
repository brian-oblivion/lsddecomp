# func_80026ECC

**Unit:** code_171e0 · **Size:** 13 words · **Status:** MATCHED, round 43
(2026-09-15, runner bravo). 13/13 words, byte-exact whole-image build.

## History

Stalled round 2026-08-29-a as class TOOLCHAIN (`gp_rel`), same mechanism as
`func_80026E0C`. Round 42 resolved `gp_rel` project-wide. Round 43 rebuilt
the preserved body verbatim; matched on the first build.

## What it does

`s32 func_80026ECC(void) { if (D_8008A84C == 0x13) return GetCdOperation(); return 0; }`
— same shape as `func_80026E64`, forwarding to a different still-uncarved
function (`GetCdOperation`, in `asm/code_179d8.s`).

## Final body

```c
extern s32 GetCdOperation(void);

s32 func_80026ECC(void) {
    if (D_8008A84C == 0x13) {
        return GetCdOperation();
    }
    return 0;
}
```

## Proposed learning

See `func_80026E0C.md` — same family.
