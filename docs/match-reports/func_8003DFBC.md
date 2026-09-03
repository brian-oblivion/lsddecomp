# func_8003DFBC — MATCH (4/4 words)

**Unit:** code_2cc8c_c · **Size:** 4 instructions

## What it does

A tiny getter: returns the address of the static table `D_8006E730`
(`TaskCoreMethods`, per `include/code_2c054.h`'s own richer local view of
the same table -- that unit's `func_8003BAB4`/`func_8003BB5C`/etc. dispatch
through several of its slots). This unit never dereferences the table, only
returns its address, so it is declared here as an opaque `u8[]`.

## The C

```c
extern u8 D_8006E730[];

void *func_8003DFBC(void)
{
    return D_8006E730;
}
```

## Provenance

round 12 (2026-09-03), runner alpha, unit code_2cc8c_c. Matched on the
first build.
