# DreamSys__DispatchLinkCommandAndTryAttach -- MATCHED (33/33)

> Renamed from `func_80057B90` on 2026-09-19 (tools/rename.py). Address 0x80057b90.

Unit: `src/class_3bb8c_p.c`. Class: `DreamSys`, own vtable slot `+0x0DC`
(base-class-inherited at this offset; `include/DreamSys.h`'s
`DreamSysBaseMethods::slot0xDC` already carried a comment naming this
exact function as its `+0xDC` resolution, from before this unit converted
it).

## Signature

```c
void DreamSys__DispatchLinkCommandAndTryAttach(DreamSys *self, void *arg1, s32 count);
```

## Body

```c
void DreamSys__DispatchLinkCommandAndTryAttach(DreamSys *self, void *arg1, s32 count) {
    GetClass6B5CCMethods()->dispatchLinkCommand(self, arg1, count);
    if (count < 9) {
        if (count >= 5) {
            self->vt->slotA0(self, arg1, count);
        }
    }
}
```

Two calls with the SAME (self, arg1, count) shape but through DIFFERENT
tables:
- The first, unconditional, dispatches through
  `GetClass6B5CCMethods()`'s return -- the shared base-class table at
  `gClass6B5CCMethods` (already established with the project's "per-call-site
  signature" precedent in `include/code_d294.h`) -- at its `+0x09C` slot.
  This unit's own local view (`Class6B5CCBaseTable`, declared in this file)
  types only that one slot.
- The second, conditional on `5 <= count < 9`, dispatches through
  `self`'s OWN vtable (`self->vt->slotA0`, `include/DreamSys.h`) at
  `+0x0A0` -- ordinary polymorphic dispatch, resolves to `Class6B5CC__TryAttachNearby`
  currently (not overridden at the `DreamSys` level, per
  `tools/classtable.py DREAMSYS_METHODS`), but written as a real vtable
  call rather than a fixed symbol.

## Shape note: the range check must be nested `if`s, not `&&`

`if (count >= 5 && count < 9)` compiles to a strength-reduced unsigned
range check (`addiu v0,count,-5; sltiu v0,v0,4`) -- one instruction
shorter than retail's two separate `slti`/`slti` comparisons, and it also
calls `GetClass6B5CCMethods()` a SECOND time for the polymorphic dispatch
instead of reading `self->vt` directly (an artifact of how the code was
written when this was mis-attributed to the shared table, not a language
issue -- see below). Nesting as `if (count < 9) { if (count >= 5) ... }`
reproduces retail's two-`slti` shape exactly.

## What was mis-diagnosed first

The first attempt routed BOTH calls through `GetClass6B5CCMethods()`'s table,
assuming the second call was just another `+0xA0` slot on the same shared
base object. That scored 15/33 with the tail completely displaced by one
word. Reading the disassembly closely: the second call's `lw v0,0(a0)`
loads from `a0` == `s1` == `self` (not from `GetClass6B5CCMethods()`'s return,
which was never re-fetched) -- i.e. it is `self->vt->slotA0`, a genuinely
different dispatch mechanism from the first call, not a repeat of it.

## Naming

**`DreamSys__DispatchLinkCommandAndTryAttach` -- tier B.** Mechanics fully
confirmed (33/33): unconditionally forwards through the shared
`Class6B5CCBaseTable::dispatchLinkCommand` slot, then -- only for
`5 <= count < 9` -- ALSO dispatches through the object's own inherited
`vt->slotA0` (resolves to `Class6B5CC__TryAttachNearby` via
`tools/classtable.py gClass6B5CCMethods`, confirmed this round). The name states
both calls and their conditional relationship; why `[5,9)` specifically
gates the attach attempt is not established.

## Proposed field names

`vtable_DreamSys::slotA0` is accessed from SEVEN other units too
(`grep -rln -- '->slotA0\b' src/` lists `class_3bb8c_g.c`,
`class_3bb8c_l.c`, `class_3bb8c_c.c`, `code_d294_b.c`, `code_2c054.c`,
`class_3bb8c_i.c`, `code_2cc8c_d.c`, besides this unit), so per
FINISHING-PLAN.md track 3 step 3 it is proposed here, not renamed, and
posted to the broadcast for the head to apply at merge.

- **`vtable_DreamSys::slotA0` -> `tryAttachNearby`** (tier B). Evidence:
  `tools/classtable.py gClass6B5CCMethods` resolves the SAME offset (`+0x0A0`) in
  the fixed base table `GetClass6B5CCMethods()` returns to
  `Class6B5CC__TryAttachNearby`, and this function's own report already
  established the slot is unoverridden at the `DreamSys` level (still
  resolves to that same function) -- matching this project's convention
  of naming a resolved base-class slot after the method it dispatches to
  (e.g. `BaseObjO__AddVec14`, `LinkWall` in the same struct). Not tier A:
  the other seven call sites' own purpose for calling it is not reviewed
  here, only this unit's own.

## Verify

```
./build-and-verify.sh   # build exit=0, OK: build matches retail
tools/funcdiff.py DreamSys__DispatchLinkCommandAndTryAttach   # 33/33
```

### Proposed learning

**Two calls with the identical argument shape are not necessarily the
same dispatch mechanism.** Check the register the vtable pointer is
LOADED FROM at each call site, not just the slot offset and argument
list -- one may go through a shared/base table fetched via a getter
function, and a superficially identical sibling call may instead go
through the object's OWN `self->vt`. They can resolve to the same function
today (inherited, unoverridden) while being byte-different call shapes.
