# BasicClass__AddChild

> Renamed from `BasicClass__func_17f98` on 2026-09-24 (tools/rename.py). Address 0x80017f98.

**Unit:** BMemPMgr · **Size:** 22 instructions · **Status:** MATCHED (22/22 words)

BasicClass vtable slot `+0x010` (`addChild`) — see `BasicClass__BasicClass.md`
for the class's overall design.

## What it does

`AddChild`: pushes `child` onto `self->children` (via `PushBasicClassListNode`,
still `INCLUDE_ASM` this round but its extern signature is established —
see below), and if the push succeeded, notifies the child by dispatching
ITS OWN vtable slot `+0x020` (`addParentRef`) as `child->methods->
addParentRef(child, self)` — registering `self` in the child's
`parentRefs` back-reference list. This is the mechanism that makes the
`children`/`parentRefs` relationship bidirectional (see
`BasicClass__BasicClass.md`).

## The C

```c
void BasicClass__AddChild(BasicClass *self, BasicClass *child)
{
    if (PushBasicClassListNode(&self->children, child)) {
        child->methods->addParentRef(child, self);
    }
}
```

## `PushBasicClassListNode`'s signature, established here

`PushBasicClassListNode(BasicClassListNode **head, BasicClass *value)` returning
`s32` (1 on success, 0 if the pool allocation failed) — allocates an
8-byte `BasicClassListNode` from the pool (`BMemPMgrAlloc(0x8)`), sets
`node->value = value`, prepends it to `*head`. Confirmed non-void by this
function's own `beqz $v0,...` check on the call result immediately after.
Second argument typed `BasicClass *` rather than a generic `s32`/`void *`
because every call site in this unit passes an actual `BasicClass *` (this
function passes `child`; `BasicClass__AddParentRef` passes the parent
`self`) — see `include/code_8220.h` for the full declaration and the other
call sites.

## Provenance

round 11 (2026-09-03), runner delta, unit code_8220 (fresh carve). One
collateral-drift residue (1 word, the `jal PushBasicClassListNode` target address)
resolved itself once `BasicClass__RemoveAllChildren` reached its correct size —
see that function's own report; nothing needed changing here.

## Naming (round 74)

`BasicClass__AddChild`, **tier A**: matches vtable slot `+0x010`
(`addChild`), already documented in `code_8220.h`. Pushes `child` onto
`self->children` and, on success, registers `self` in the child's own
`parentRefs` via the child's `addParentRef` slot.
