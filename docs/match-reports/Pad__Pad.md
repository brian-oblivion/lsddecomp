# Pad__Pad -- MATCHED (36/36 words)

> Renamed from `func_80025BA0` on 2026-09-24 (tools/rename.py). Address 0x80025ba0.

**Unit:** class_16334 · **Round:** 44 (2026-09-15)

## Provenance

Round-42's "REOPENED -- ASSIGNABLE" banner applies: this had been stub-stalled
as blocked by the `gp_rel` construct (`sPadRefCount` accessed via
`%gp_rel($gp)`). That blocker is RESOLVED as of round 42
(`--gp-symbols=config/gp-symbols.txt`, `docs/research/gp-relative-blocker.md`)
and `sPadRefCount` is already present in `config/gp-symbols.txt`, so no new
toolchain work was needed here. Matched byte-exact on the FIRST build.

## What it is

The Pad constructor. Calls the base-class ctor through the inherited table,
installs this class's own method table, and — on the very first live
instance only — initializes the Psy-Q Pad library.

## C

```c
void Pad__Pad(Pad *self, void *arg1, s32 port) {
    Get_vtable_BasicClass()->ctor(self);
    self->methods = Get_vtable_Pad();
    if (sPadRefCount++ == 0) {
        PadInit(arg1);
    }
    self->methods->init(self, port);
}
```

## Notes

The postfix increment (`sPadRefCount++ == 0`) is the exact retail shape: retail
loads the OLD counter value first, uses it for the branch, stores `old + 1`
back, and stores `self->methods` in between the load and the branch — all of
which C's ordinary statement-order and postfix-increment semantics reproduce
without any reshaping. No residue.

### Proposed learning

Nothing new — this confirms round 42/43's finding that the `gp_rel`-stalled
functions in this corpus were mechanism-correct all along and just needed the
flag; a preserved derivation (where one existed) was directly usable.
