# ObjM__OnNotify

> Renamed from `func_80052D10` on 2026-09-24 (tools/rename.py). Address 0x80052d10.

**Unit:** class_3bb8c_k · **Size:** 52 instructions (0xD0 bytes) ·
**Status: MATCHED 52/52**, whole-image SHA1 green.

## Role

Another method of the `gObjMMethods`-vtable class (`New_ObjM`/
`ObjM__ObjM`'s own class, see those reports) -- forwards to the shared
base-class event handler (`GetTimedTaskMethods()->slot38`), then reads the
event's type tag (`arg1->target->header`, both `EventArg`/`HeaderObj`
already established in `include/class_39e08.h`) and dispatches to ONE of
three of self's own vtable slots depending on which range the tag falls
in, or does nothing if it matches none of them.

## The control-flow trap

The three-way tag dispatch is NOT a clean if/else-if/else-if with one
comparison per branch -- reading the raw `.s` labels naively produces the
WRONG branch structure. The first attempt (which compiled clean but
diverged hard, 28/52 words with a large outside-range shift) nested the
`(tag & 0xFFFF) == 0x1F34` check UNDER the `(tag & 0xFFF) == 0x164` arm --
a natural misreading, since that's where `andi $v1,$v1,0xFFFF` sits
textually in the `.s`. It is NOT conditional on `0x164`.

Reading the actual branch targets (not just linear order) gives the real
shape:

- `tag & 0xFFF == 0x114` -> call `slotB4`.
- else `tag & 0xFFF == 0x164` -> call `slotB0` UNCONDITIONALLY (no further
  check -- the `andi $v1,$v1,0xFFFF` on this path computes a value that is
  never read, dead code retail still emits because it's on the shared
  instruction stream before the branch that separates the two remaining
  cases).
- else (neither) -> if `tag & 0xFFFF == 0x1F34`, call `slot90`; otherwise
  no call at all.

```c
void ObjM__OnNotify(Obj87034_3bb8c_k *self, EventArg *arg1, s32 arg2)
{
    s32 tag;

    GetTimedTaskMethods()->slot38((Obj865C8 *)self, arg1, arg2);
    tag = arg1->target->header;
    if ((tag & 0xFFF) == 0x114) {
        self->methods->slotB4(self, arg1, arg2);
    } else if ((tag & 0xFFF) == 0x164) {
        self->methods->slotB0(self, arg1, arg2);
    } else if ((tag & 0xFFFF) == 0x1F34) {
        self->methods->slot90(self, arg1, arg2);
    }
}
```

This matched on the first attempt once the branch structure above was
corrected -- the earlier (wrong) nesting was a control-flow misreading, not
a residue to reshape away from.

## Struct additions

Extended this unit's own local `Class87034Methods_3bb8c_k` (see
`New_ObjM`/`ObjM__ObjM`'s reports) with three more slots this
function reaches: `+0x090` (`slot90`), `+0x0B0` (`slotB0`), `+0x0B4`
(`slotB4`), all `void (*)(void *self, EventArg *arg1, s32 arg2)`. Stays
local to this unit for the same header-contention reason as before.

### Proposed learning

For a `.s` with several `bne`/`beqz` branches into a shared tail (a common
"dispatch to one of N handlers" shape), read the ACTUAL BRANCH TARGETS
before writing the C -- an instruction that sits textually "under" a
branch in the `.s` listing is not necessarily conditional on it; it may be
on the FALLTHROUGH path of an earlier branch and simply share layout with
a later, unrelated check. This is the same family of trap
`docs/MATCHING-GUIDE.md` already documents for delay slots ("branch
targets disagree, not just delay slots") but for a THREE-way dispatch
specifically: worth checking each comparison's true predecessor/successor
in the CFG before assuming linear `.s` order matches nesting order.

## Naming

Round 75 (bravo, track 3). `func_80052D10` -> `ObjM__OnNotify`, **tier B**.

Slot +0x038 of gObjMMethods (`tools/classtable.py 0x80087034`), the slot called OnNotify elsewhere in this family (Class865C8__OnNotify, IntermediateBase__OnNotify). Calls the base's slot38, then dispatches on arg1->target->header: (tag & 0xFFF) == 0x114 -> ObjM__OnStageMapNotify (+0x0B4), 0x164 -> ObjM__OnFadeNotify (+0x0B0), (tag & 0xFFFF) == 0x1F34 -> ObjM__OnDreamSysNotify (+0x090). Tier B: which objects carry those header tags is not established.


## Track 4 (2026-09-26, round 89, echo)

The class is unified as ObjM in include/ObjM.h (table gObjMMethods, was D_80087034); the class_3bb8c_k/_l/_m views (ObjM_3bb8c_k, Obj87034_3bb8c_l, ObjM) and class_39e08.h's Obj4C/SubObjB/EventArg are gone. Byte-identical. The sender is `BasicClass *` (its methods->header is the class id); the three targets are onStageMapNotify (0x114), onFadeNotify (0x164, cast to FadeBox *) and onDreamSysNotify (0x1F34).
