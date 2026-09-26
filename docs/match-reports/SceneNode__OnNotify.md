# SceneNode__OnNotify -- MATCHED (52/52 words)

> Renamed from `Class6B5CC__OnNotify` on 2026-09-26 (tools/rename.py). Address 0x8001cd60.

> Renamed from `func_8001CD60` on 2026-09-23 (tools/rename.py). Address 0x8001cd60.

Unit: `code_d294` (round 14, first function of a fresh 3-function queue in
this carve). Occupies `SceneNodeMethods` vtable slot `+0x038` (an
override, per the file banner's `classtable.py` census). Forwards
unconditionally to a new `BasicClassMethodsD294` slot, then dispatches to
one of three new `SceneNodeMethods` slots based on `other`'s own
vtable-header tag nibble. `void SceneNode__OnNotify(SceneNodeObj *self,
GenericObj_d294 *other, s32 arg2)`.

## Final source

```c
void SceneNode__OnNotify(SceneNodeObj *self, GenericObj_d294 *other, s32 arg2) {
    s32 tag;

    Get_vtable_BasicClass()->slot38(self, other, arg2);
    tag = other->methods->header & 0xF;
    if (tag == 2) {
        self->methods->slot94(self, other, arg2);
    } else if (tag == 5) {
        self->methods->slot98(self, other, arg2);
    } else if (tag == 4) {
        self->methods->slot9C(self, other, arg2);
    }
}
```

## New struct knowledge (`include/code_d294.h`, additive)

- **`BasicClassMethodsD294` gains `slot38`** (`void(void*,void*,s32)`),
  carved out of what was previously the struct's own trailing padding (the
  struct ended at `slot18`/`+0x018` with no declared size past it). Called
  unconditionally as this function's first action.
- **`SceneNodeMethods` gains three new slots, `slot94`/`slot98`/`slot9C`**
  (`void(SceneNodeObj*, GenericObj_d294*, s32)`), split out of the
  previously-opaque `pad060[0x0A0-0x060]` range (now `pad060[0x094-0x060]`
  followed by the three new slots, ending exactly at `+0x0A0` where the
  existing `slotA0` already begins -- no size change to the struct).

No existing field was retyped or renamed.

## Derivation notes

- The tag dispatch (`other->methods->header & 0xF`) reuses the existing
  `GenericMethods_d294::header` field and its documented "low nibble is a
  class-tag" convention (already established by `SceneNode__AddChild`/
  `SceneNode__RemoveChild` comparing the same nibble against `9`).
- **Residue: a function-pointer local (`fn = self->methods->slot94; ...;
  fn(self, other, arg2);`) scored 23/52 with a redundant `move a0,s0`
  retail has in each branch that the function-pointer-variable version
  does not.** Writing each branch as a DIRECT call
  (`self->methods->slot94(self, other, arg2);` etc., one per branch, no
  shared `fn` variable) matched immediately. Reading retail's disassembly
  after the fact: it computes each branch's function pointer AND sets up
  `a1`/`a2` for the call independently, then all three branches converge
  on ONE shared `jalr` site -- i.e. GCC 2.6.3's own tail-merging folded
  three structurally-identical direct calls into one shared call
  instruction, which is a different (and, for this compiler, apparently
  more literal) code shape than caching the resolved pointer in a
  variable first. The extra `move a0,s0` is part of what each branch
  independently materializes before falling into the merged tail; it
  disappears once the branches are simple direct calls again, since then
  there's no separate "resolve the pointer" step distinct from "set up
  the call."

No other new struct or vtable-slot knowledge.

### Proposed learning

**When retail dispatches to one of several "same signature" callees
picked by a branch, prefer a direct call written once per branch over
caching the resolved function pointer in a local -- even when the calls
are textually identical (same arguments) across every branch.** GCC 2.6.3
tail-merges the identical trailing call sequence across branches on its
own; caching the pointer in a variable first changes the code shape
(loses a redundant-but-retail-has-it `move`) rather than reproducing it.

## Naming

Round 71 (alpha). `func_8001CD60` -> `SceneNode__OnNotify`, **tier A**. Overrides BasicClass slot +0x038, BasicClass__OnNotify (receiving half of notifyParents; include/code_8220.h). Forwards to the base first, then dispatches on the SENDER's class tag: 2 (the pad class gPadMethods) -> slot +0x094, 5 (FrameClock) -> +0x098, 4 (SceneNode family) -> +0x09C dispatchLinkCommand. Slots 94/98 keep placeholders: their occupants (func_8001D6A4/func_8001D6AC, code_d294_b) are not named.

## Proposed field names

For the head to apply by type scope. Each one fails to compile in another unit when renamed in the definition, so this unit did not apply it.

- `SceneNodeMethods.slot30` -> `notifyParents` (tier A): the occupant is BasicClass__NotifyParents (inherited verbatim), and BasicClassMethods names the slot notifyParents. Accessor: code_d294_b (TransformAndNotifyParents).
- `GenericMethods_d294.slot38` -> `onNotify` (tier A): BasicClass slot +0x038, called as `(other, self, 4)`, i.e. sender self, event 4. Accessor: only the NON_MATCHING body of SceneNode__TryAttachNearby in code_d294_b (the default build does not see it; check-nonmatching does).
