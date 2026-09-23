# CD_initvol

> Renamed from `func_8002A5F8` on 2026-09-23 (tools/rename.py). Address 0x8002a5f8.

**Unit:** code_179d8_g · **Size:** 61 words · **Status:** MATCHED (61/61 words)

## What it does

Programs a block of hardware registers through the `D_8006D8D4` pointer
(halfword offsets 0x180/0x182/0x1AA/0x1B0/0x1B2/0x1B8/0x1BA), conditionally
resetting two of them to `0x3FFF` if both status halfwords are already zero,
then stages a fixed 4-byte sequence (`0x80, 0, 0x80, 0`) through the same
byte pointer dance as `CD_vol`.

## The C

```c
s32 CD_initvol(void)
{
    u8 buf[4];

    if (D_8006D8D4[0xDC] == 0 && D_8006D8D4[0xDD] == 0) {
        D_8006D8D4[0xC0] = 0x3FFF;
        D_8006D8D4[0xC1] = 0x3FFF;
    }
    D_8006D8D4[0xD8] = 0x3FFF;
    D_8006D8D4[0xD9] = 0x3FFF;
    D_8006D8D4[0xD5] = 0xC001;

    buf[2] = 0x80;
    buf[0] = 0x80;
    buf[3] = 0;
    buf[1] = 0;
    *D_8006D8C0 = 2;
    *D_8006D8C8 = buf[0];
    *D_8006D8CC = buf[1];
    *D_8006D8C0 = 3;
    *D_8006D8C4 = buf[2];
    *D_8006D8C8 = buf[3];
    *D_8006D8CC = 0x20;
    return 0;
}
```

## Notes

Matched on the first attempt -- straightforward translation once
`D_8006D8D4` was already established as `volatile u16 *` in this unit's
header (declared while matching `CD_vol`/`func_8002B304`'s neighbours;
see `code_179d8_g.c`'s extern block). Indices are the halfword offsets
divided by 2 (`0x180/2 = 0xC0`, etc), matching retail's byte-offset
immediates exactly through ordinary `u16 *` pointer arithmetic -- no cast
juggling needed.

The `D_8006D8D4[0xDC] == 0 && D_8006D8D4[0xDD] == 0` short-circuit `&&`
compiles directly to retail's two-test nested-skip shape (test A, skip inner
if nonzero; test B, skip inner if nonzero; else run inner) with no
restructuring needed.
