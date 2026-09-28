# GetDreamSysMethods

> Renamed from `Get_vtable_DreamSys` on 2026-09-28 (tools/rename.py). Address 0x8005ba40.

**Unit:** DreamSys · **Size:** 4 instructions · **Status:** MATCHED (4/4 words)

## What it does

The canonical class-table accessor, documented in
`docs/research/class-framework.md` as the shape that appears throughout the
codebase: `lui`/`addiu` the table's address, return it. No dereference, no
gate, no computation.

## The C

```c
struct vtable_DreamSys *GetDreamSysMethods(void)
{
	return &gDreamSysMethods;
}
```

Exactly as free as the coordinator flagged it -- one line, matched on the
first try.

## Provenance

round 2026-08-30-c, runner ALPHA, unit DreamSys (whole-unit second pass).
