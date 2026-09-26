# Class866E8__OnNotify — MATCH

> Renamed from `func_8004A984` on 2026-09-22 (tools/rename.py). Address 0x8004a984.

**Unit:** class_3ac78 · **Size:** 35 instructions · **Result:** 35/35 words

## What it does

`Class866E8Methods` slot `+0x038`. Looks up an external table via
`GetSceneNodeMethods(self, arg1)` (not decompiled anywhere yet, in an uncarved
segment) and calls that table's OWN slot `+0x038` with `(self, arg1,
arg2)`. Then, if `arg1`'s own vtable header's low nibble is `1` (the
family/base-class tag pattern from `docs/research/class-framework.md`,
already used by `Class866E8__DispatchLinkCommand` in this unit), also calls
`self->methods->slot100(self, arg1, arg2)`.

Note this does NOT recurse into itself: the table returned by
`GetSceneNodeMethods` is a *different* class's vtable from `self->methods`
(`gClass866E8Methods`) — slot `+0x038` there happens to be `Class866E8__OnNotify` (this
very function) only in `Class866E8Methods`, not necessarily in whatever
`GetSceneNodeMethods` returns.

## GetSceneNodeMethods's inferred signature

Not decompiled (lives in the still-uncarved `asm/code_d294.s`). Its second
parameter is passed as a plain `s32` from `Class866E8__OnElementEvent` (compared there
against small integer literals 6/7 — not a pointer-shaped use), so it's
declared here as `extern void *GetSceneNodeMethods(Class866E8 *self, s32
arg1);` and this function casts its own `GenericObject *arg1` to `s32` at
the call site. The cast is a no-op at the machine level (both are 32-bit
register values) and costs nothing.

## Final source

```c
extern void *GetSceneNodeMethods(Class866E8 *self, s32 arg1);

void Class866E8__OnNotify(Class866E8 *self, GenericObject *arg1, s32 arg2)
{
    void (*fn)(Class866E8 *self, GenericObject *arg1, s32 arg2);

    fn = *(void (**)(Class866E8 *, GenericObject *, s32))
        ((u8 *)GetSceneNodeMethods(self, (s32)arg1) + 0x38);
    fn(self, arg1, arg2);

    if ((arg1->methods->header & 0xF) == 1) {
        self->methods->slot100(self, arg1, arg2);
    }
}
```

New struct knowledge: `Class866E8Methods::slot100` (called here, dispatched
by `self`), typed `(Class866E8 *, void *, s32)`.

## Residue

None — matched on the first attempt, no reshaping needed. The raw
pointer-cast call through `GetSceneNodeMethods`'s return is ugly but necessary:
the returned table's class is unknown (only the one slot at `+0x38` this
function reaches is typed), so it cannot reuse `Class866E8Methods` even
though the numeric offset happens to coincide.

### Proposed learning

**A helper function's second argument register can carry genuinely
different C types across different call sites in the same unit** (here,
`GetSceneNodeMethods`'s arg1 is a small `s32` tag at one call site and a
`GenericObject *` at another) with no compile-time conflict, as long as
the shared `extern` declaration picks ONE parameter type (word-sized) and
callers cast to it. The cast costs zero instructions on this architecture
since pointers and `s32` are both 32-bit registers.

## Provenance

round 2026-09-02, runner ALPHA, unit class_3ac78.

## Naming

Round 67 (track 3, naming pass).

| symbol | name | tier | evidence |
| --- | --- | --- | --- |
| `func_8004A984` | `Class866E8__OnNotify` | A | Occupant of vtable slot `+0x038`, which `include/code_8220.h` establishes as `BasicClassMethods::slot38` / `onNotify` -- the RECEIVING half of `+0x030 notifyParents`, with `(self, sender, event)`. The body is the standard override shape: call the base table's own `+0x038` with the same three arguments, then branch on the SENDER's class tag (`sender->methods->header & 0xF`). `SceneNode__OnNotify` in `code_d294` is the same shape one class up. |

Parameters renamed from the evidence: `arg1` -> `sender`, `arg2` -> `command`
(`code_8220.h` calls the pair sender/event; this class's own numbering is
described in `Class866E8__ForwardAcceptedCommand.md`).
