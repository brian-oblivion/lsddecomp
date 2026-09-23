# GetDefaultStreamTaskInitData — MATCH (4/4 words)

> Renamed from `func_8003DFCC` on 2026-09-19 (tools/rename.py). Address 0x8003dfcc.

**Unit:** code_2cc8c_c · **Size:** 4 instructions

## What it does

A tiny getter: returns the address of the static table `gDefaultStreamTaskInitData`, a
3-word struct per `include/code_2c054.h`'s own `StreamTaskInitData` local
view (that unit's `StreamTaskObj__StreamTaskObj` uses it as the 5th/stack argument to a
constructor call, and `GetDefaultStreamTaskInitData()`'s return value feeds the same
3-word copy). This unit never dereferences it, only returns its address,
so it is declared here as an opaque `u8[]`.

## The C

```c
extern u8 gDefaultStreamTaskInitData[];

void *GetDefaultStreamTaskInitData(void)
{
    return gDefaultStreamTaskInitData;
}
```

## Provenance

round 12 (2026-09-03), runner alpha, unit code_2cc8c_c. Matched on the
first build.

## Naming

**GetDefaultStreamTaskInitData** (renamed from `func_8003DFCC`, round 55,
runner alpha). Tier A: pure leaf getter, returns `&gDefaultStreamTaskInitData`
(formerly `D_8006E854`), already declared `StreamTaskInitData *func_8003DFCC(void)`
in `include/code_2c054.h` and used there (`code_2c054.c`) as the fallback
default when a caller supplies no init data -- "Default" is the confirmed
mechanic (a fixed fallback constant), not a guess.
