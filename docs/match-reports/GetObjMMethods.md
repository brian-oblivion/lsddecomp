# GetObjMMethods

> Renamed from `func_800544D4` on 2026-09-23 (tools/rename.py). Address 0x800544d4.

**Unit:** class_3bb8c_m · **Size:** 4 instructions · **Status:** MATCHED (4/4 words)

## What this function does

Plain vtable getter: whole body is `lui`/`addiu` computing `&D_80087034`,
no `lw`/`sw` at all. `tools/classtable.py 0x80087034` confirms it is a
real class vtable (header word then `BasicClass__Release` at +4, the
class-framework fingerprint) -- a subclass of `BasicClass`. Nothing in
this unit dereferences the returned table, so it stays untyped beyond the
pointer itself.

```c
void *GetObjMMethods(void) {
    return &D_80087034;
}
```

## Residue

None -- matched on the first attempt.

## Provenance

round 15b (2026-09-04), runner echo, second pass on `class_3bb8c_m`
(the three functions left over the first round's 12-function budget).

## Naming

**GetObjMMethods** -- tier A. Plain vtable getter: whole body is `lui`/`addiu` computing `&D_80087034`, matching the `Get<Class>Methods` pattern already established elsewhere in this codebase (`GetClass869D8Methods` etc.). `tools/classtable.py 0x80087034` confirms it is a real class vtable (BasicClass framework fingerprint). A pure getter is tier A by definition; the class's own game-facing name stays unconfirmed, hence `GetObjMMethods` rather than a semantic name.
