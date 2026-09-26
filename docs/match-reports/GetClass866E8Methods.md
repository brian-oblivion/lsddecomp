# GetClass866E8Methods — MATCHED (4/4 words)

> Renamed from `func_8004D244` on 2026-09-24 (tools/rename.py). Address 0x8004d244.

Get-vtable helper for the class whose method table is `gClass866E8Methods`
(`Obj866E8` in this unit's header, `Class866E8` in `class_3ac78.h`'s
independent view of the SAME table). This is the SAME real function
`class_3ac78.h` already documents an `extern` prototype for (`"Get-vtable
helper for Class866E8. Still raw asm... lives in class_3bb8c"`) — it now
has a real body, contributed by this unit.

## Disassembly

```
lui   $v0, %hi(gClass866E8Methods)
addiu $v0, $v0, %lo(gClass866E8Methods)
jr    $ra
 nop
```

No dereference — this just materializes `&gClass866E8Methods`. Confirmed as a
zero-argument call from its only external caller, `class_3ac78.c`'s
`Class866E8__Class866E8` (the `New_Class866E8` constructor): the `jal` there has a
`nop` in its own delay slot (no argument setup) and the very next
instruction stores `$v0` straight into `self->methods` (offset 0), i.e.
`self->methods = GetClass866E8Methods();`.

## Final C

```c
Obj866E8Methods *GetClass866E8Methods(void) {
    return &gClass866E8Methods;
}
```

## New struct/global knowledge

- `extern Obj866E8Methods gClass866E8Methods;` added to `include/class_3bb8c.h`
  (this unit's own independent view of the table; `class_3ac78.h` keeps
  its own separate `Class866E8Methods` view of the identical memory, per
  the project's established multiple-independent-views convention).

## Attempts

1 (matched on first attempt).

### Proposed learning

None new — confirms the already-established "extern Methods D_xxx; return
&D_xxx;" getter idiom used throughout the project (`class_16334.c`,
`Entity_b.c`, `class_39e08.c`, `class_3ac78.c`, `code_2c054.c`,
`code_55dd4.c`).

## Naming

**Tier A.** `lui`/`addiu` of `&gClass866E8Methods`, no dereference -- the class's
get-vtable helper. Matches this project's established
`GetClass<addr>Methods` convention for these helpers exactly (e.g.
`GetClass86668Methods`, `GetClass86F88Methods`, `GetNodeGuardedViewportMethods`),
which class_3ac78.h's own extern for this SAME real function already
anticipated under this exact name pattern (previously documented there as
an unnamed extern for "the get-vtable helper... lives in class_3bb8c").
