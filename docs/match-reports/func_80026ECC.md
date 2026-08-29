# func_80026ECC

**Unit:** code_171e0 · **Size:** 13 instructions · **Status:** STALLED, class TOOLCHAIN

## What it does

`s32 func_80026ECC(void) { if (D_8008A84C == 0x13) return func_80027EE0(); return 0; }`
— same shape as `func_80026E64`, forwarding to a different still-uncarved
function (`func_80027EE0`, in `asm/code_179d8.s`).

## Residue

Same root cause as `func_80026E0C`: `D_8008A84C` is a real `.sdata` global
this project's pinned `-G0` cannot address as `%gp_rel` from C. See
`docs/match-reports/func_80026E0C.md` for the isolated reproducer.

## Preserved body

```c
extern s32 D_8008A84C;
extern s32 func_80027EE0(void);

s32 func_80026ECC(void) {
    if (D_8008A84C == 0x13) {
        return func_80027EE0();
    }
    return 0;
}
```

## Proposed learning

See `func_80026E0C.md`.
