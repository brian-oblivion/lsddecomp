# CD_initintr

> Renamed from `func_8002A6EC` on 2026-09-23 (tools/rename.py). Address 0x8002a6ec.

**Unit:** code_179d8_g · **Size:** 28 words · **Status:** MATCHED (28/28 words)

## What it does

Minimal driver-thread starter: clears four scalar state words
(`D_8006D600`, `D_8006D5FC`, `D_8006D610`, `D_8006D60C`), zeroes a run of
ten consecutive words starting at `D_8006D8DC`, then calls
`func_80024D10` (Psy-Q, `asm/psyq_GsLinkObject4.s`) and registers
`func_8002B3F4` as a thread entry via `func_80024D40(2, func_8002B3F4)`.

## The C

```c
void CD_initintr(void)
{
    s32 *p;
    s32 i;

    D_8006D600 = 0;
    D_8006D5FC = 0;
    D_8006D610 = 0;
    D_8006D60C = 0;
    p = &D_8006D8DC;
    for (i = 9; i != -1; i--) {
        *p = 0;
        p++;
    }
    func_80024D10();
    func_80024D40(2, func_8002B3F4);
}
```

## The zeroing-loop shape -- important for the sibling CD_init

The ten words zeroed here (`D_8006D8DC` through `D_8006D900`) are each
ALREADY individually named symbols (confirmed against the true, undrifted
link -- see below), not a real C array; splat named each one separately
because each is also referenced individually elsewhere in this unit
(`D_8006D8E0`/`D_8006D8E4` as plain scalars in `CD_readm`, etc). Writing
this as `for (i = 0; i < 9; i++) D_8006D8DC[i] = 0;` over a declared
`s32 D_8006D8DC[9]` compiles to a SHORTER, single-register pointer-compare
loop -- 4 bytes short of retail, which uses a separate down-counting index
register (`$v0`, `li v0,9`/`li a0,-1`/`bne v0,a0`) alongside the address
pointer (`$v1`). Retail's source is a pointer walk with an explicit counter:

```c
s32 *p = &D_8006D8DC;
for (i = 9; i != -1; i--) { *p = 0; p++; }
```

This produces the exact `li v0,0x9` / `li a0,-1` / post-decrement-and-branch
shape.

### Proposed learning

**A 4-byte length mismatch in ANY function in a unit shifts the addresses
embedded in EVERY OTHER function's `%lo(sym)` references for globals that
live in a later-linked `.data`/`.rodata` object -- even earlier-defined,
already-matched functions in the SAME file appear to "break" with every word
after the first `lui` looking wrong.** This is exactly
`docs/MATCHING-GUIDE.md`'s documented address-drift class, but the
presentation was confusing enough to write down concretely: `CD_vol`
(the FIRST function in this file, matched and unchanged) started reporting
27/34 with every `%lo` immediate off by 4, purely because a LATER function in
the same file (`CD_initintr`) was compiling 4 bytes short. `funcdiff.py`'s
"build differs OUTSIDE this range" warning is the tell -- trust it over the
per-function word count when several unrelated-looking functions all show
partial mismatches at once.

**A run of consecutive globals that are each individually named (not a real
array) but zeroed together in a loop is walked with a raw pointer plus a
SEPARATE counter variable, not a `for (i = 0; i < N; i++) arr[i] = 0;` over a
declared array** -- even when nothing else in the unit ever indexes them as
an array. Check the TRUE (undrifted) addresses of the neighbouring symbols
before declaring one as `T name[N]`; if a later symbol's true address is
only 4 bytes past the "array"'s start, it isn't an array.
