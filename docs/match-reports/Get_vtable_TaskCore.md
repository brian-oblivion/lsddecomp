# Get_vtable_TaskCore — MATCH (4/4 words)

> Renamed from `func_8003DFBC` on 2026-09-19 (tools/rename.py). Address 0x8003dfbc.

**Unit:** Task · **Size:** 4 instructions

## What it does

A tiny getter: returns the address of the static table `gTaskCoreMethods`
(`TaskCoreMethods`, per `include/Task.h`'s own richer local view of
the same table -- that unit's `StreamTask__OnInit`/`StreamTask__Update`/etc. dispatch
through several of its slots). This unit never dereferences the table, only
returns its address, so it is declared here as an opaque `u8[]`.

## The C

```c
extern u8 gTaskCoreMethods[];

void *Get_vtable_TaskCore(void)
{
    return gTaskCoreMethods;
}
```

## Provenance

round 12 (2026-09-03), runner alpha, unit Task. Matched on the
first build.

## Naming

**Get_vtable_TaskCore** (renamed from `func_8003DFBC`, round 55, runner alpha).
Tier A: pure leaf getter, returns `&gTaskCoreMethods` (formerly `D_8006E730`).
Already independently declared and named `TaskCoreMethods *func_8003DFBC(void)`
in `include/Task.h` (that unit's own richer local view of the same
table) -- this rename aligns the function's own name with the established
type name (`GetBasicClassMethods` naming precedent already used in this
project for exactly this shape: a no-argument getter returning a shared
static vtable).

## Track 4 (2026-09-25, round 84, alpha)

Returns `TaskCoreMethods *` (`&gTaskCoreMethods`); it returned `void *` over an `extern u8 gTaskCoreMethods[]`. Byte-identical.
