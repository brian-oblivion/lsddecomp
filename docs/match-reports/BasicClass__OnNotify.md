# BasicClass__OnNotify

> Renamed from `BasicClass__func_18358` on 2026-09-17 (tools/rename.py). Address 0x80018358.

**Unit:** TmdRenderer · **Size:** 14 instructions · **Status:** MATCHED (14/14 words)

BasicClass vtable slot `+0x038` (`slot38` in `BasicClassMethods`). Called by
`BasicClass__NotifyParents` (slot `+0x030`, `notifyParents`, still `INCLUDE_ASM`
this round) once per entry in `self->parentRefs`, as
`parent->methods->slot38(parent, childBeingFinalized, 1)`.

## What it does

```c
void BasicClass__OnNotify(BasicClass *self, void *arg1, s32 arg2)
{
    if (arg2 == 1) {
        self->methods->removeChild(self, (BasicClass *)arg1);
    }
}
```

Reading it against its only caller: when a child object finalizes, it walks
its own `parentRefs` list and calls each parent's `slot38(parent, self, 1)`.
This function's job, from the parent's side, is to remove that now-dying
child from `self`'s own `children` list — a flag-gated notification hook
(`arg2 != 1` is a no-op; no other value is exercised by anything in this
unit, but the check exists in the retail binary so it's kept literally
rather than assumed always-true).

`arg1` is `void *` at the vtable-slot level (see `code_8220.h`'s
`BasicClassMethods::slot38`) and gets cast to `BasicClass *` only at the
`removeChild` call site — the caller and this callee agree on what's really
being passed, but the slot's own declared type stays generic.

## Provenance

round 12 (2026-09-03), runner charlie, unit code_8220_b (fresh carve).
Matched first attempt — the `if (arg2 == 1)` shape and the always-allocated
stack frame (needed because the true branch makes an indirect call) came
straight off the disassembly.

## Naming (round 51, bravo)

`BasicClass__func_18358` -> `BasicClass__OnNotify`, with `arg1` -> `sender`
and `arg2` -> `event`. **Tier B** -- the mechanics are exact and the
pairing with slot `+0x030` is established by code, but what the events
other than 1 mean is not.

Evidence:

- The body is `if (event == 1) self->methods->removeChild(self, sender);`.
- Its caller inside BasicClass is `BasicClass__NotifyParents` (slot
  `+0x030`, this unit), which calls it as `slot38(parent, child, event)` --
  so `self` here is the PARENT and `arg1` is the object doing the
  notifying. Hence `sender`.
- `arg2` is not a boolean, and that is why `event` and not `isFinalizing`.
  Two overrides in other units forward to the base and then keep using the
  same value: `SceneNode__OnNotify` (`src/graphics/scene_node.c`) calls
  `GetBasicClassMethods()->slot38(self, other, arg2)` and then dispatches
  to slot `+0x094`/`+0x098`/`+0x09C` **by the sender's class tag**, passing
  `arg2` through each time; `TodActor__OnNotify` (`src/world/TodActor.c`) calls the
  base and then tests `arg1->tagged->tag == 0x5F03 && arg2 == 1`.
- Slot census (`tools/classtable.py`, all 60 tables): 27 tables use this
  base implementation at `+0x038` and 14 more override it with
  `SceneNode__OnNotify` alone, so the slot is heavily used and genuinely
  polymorphic -- which is the reason the name describes the PROTOCOL
  ("something notified me") rather than the base's one action
  ("remove a child").

### Proposed field names

**APPLIED by the head at merge, round 52** -- all four fields, both types
and all five vtable slots below are now in the tree, each one applied
separately with `./build-and-verify.sh` green and byte-exact after it. One
mis-hit had to be resolved by receiver type: `src/cd/cd_driver.c:143`
accesses `pendingGeneration` on a `FileResource *self`, while the same file's
lines 97/174/175/182 are its OWN `ObjA34_179D8H::unk0C` and were left alone.
The compiler named that mis-hit (`structure has no member named 'unk0C'`),
which is the procedure working in the direction where it can work. for the head

`BasicClassMethods::slot38` -> `onNotify`, its `arg1` -> `sender`, its
`arg2` -> `event`. **Tier B**, same evidence. Cross-unit and the widest
replace of the round: 13 units access `->slot38`
(`TitleMenuTaskObjF/f/i/j/k.c`, `DayTaskStageMap.c`, `class_3ac78.c`,
`Task/d.c`, `TodActor.c`, `scene_node.c`, `code_d294_b.c`, plus
this unit). Worth doing alone rather than batched.

## Round 91 polish (delta, track 7)

The source comment no longer names the two overriding subclasses; they are
listed in this report above.
