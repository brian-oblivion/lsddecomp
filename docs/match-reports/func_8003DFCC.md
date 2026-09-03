# func_8003DFCC — MATCH (4/4 words)

**Unit:** code_2cc8c_c · **Size:** 4 instructions

## What it does

A tiny getter: returns the address of the static table `D_8006E854`, a
3-word struct per `include/code_2c054.h`'s own `StreamTaskInitData` local
view (that unit's `func_8003B8E4` uses it as the 5th/stack argument to a
constructor call, and `func_8003DFCC()`'s return value feeds the same
3-word copy). This unit never dereferences it, only returns its address,
so it is declared here as an opaque `u8[]`.

## The C

```c
extern u8 D_8006E854[];

void *func_8003DFCC(void)
{
    return D_8006E854;
}
```

## Provenance

round 12 (2026-09-03), runner alpha, unit code_2cc8c_c. Matched on the
first build.
