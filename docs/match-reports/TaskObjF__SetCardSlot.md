# TaskObjF__SetCardSlot — MATCH (4/4 words)

> Renamed from `func_8004E5D4` on 2026-09-24 (tools/rename.py). Address 0x8004e5d4.

**Unit:** class_3bb8c_e (round 14, `Node3bb8cE` class).

## What it does

`void TaskObjF__SetCardSlot(Node3bb8cE *self, s32 val)`. Sets `self->unkC = val`
and `self->unk10 = val << 4`. The only setter in this unit for `unkC`,
which `TaskObjF__FormatCard` later reads as a nonzero-tested flag and
`TaskObjF__OpenAndReadMemcardFile`/`TaskObjF__ProbeCardFreeSpace` forward opaquely as `BuildMemcardPath`'s
2nd argument.

## Result

Matched on the first attempt.

```c
void TaskObjF__SetCardSlot(Node3bb8cE *self, s32 val)
{
    self->unkC = val;
    self->unk10 = val << 4;
}
```

### Proposed learning

None — trivial straight-line store pair.

## Naming (round 78, track 3)

`func_8004E5D4` -> `TaskObjF__SetCardSlot`. **Tier A.** Sits at `gTaskObjFMethods` +0x040 -- the exact slot `TaskObjF__TaskObjF`'s own ctor calls directly as `self->methods->slot40(self, arg2)` (class_3bb8c_d.c), which is the strongest single piece of evidence in this unit that `Node3bb8cE` is `TaskObjF`. Sets `self->cardSlot` (which the shared `TaskObjF` struct already names at the identical offset +0x00C) and derives `self->cardHandle` (`cardSlot << 4`).

## Proposed field names (round 78, cross-unit -- head to apply)

`include/class_3bb8c.h`'s `TaskObjFMethods` struct (shared, owned by
class_3bb8c_d/_f/_g) mis-describes the slot this function occupies:

```c
u8 pad3C[0x044 - 0x03C];
```

marks `+0x03C`..`+0x044` (two words) as padding. `+0x03C` really is `0x00000000`
in `gTaskObjFMethods` (asm/data/76DC8.data.s), but `+0x040` is NOT padding --
it holds `TaskObjF__SetCardSlot` (this function), the exact slot
`TaskObjF__TaskObjF`'s own ctor calls as `self->methods->slot40(self, arg2)`
(class_3bb8c_d.c). **Proposed:** split the pad and add a named slot:

```c
u8 pad3C[0x040 - 0x03C];
void (*slot40)(TaskObjF *self, s32 arg1);   /* +0x040, TaskObjF__SetCardSlot (class_3bb8c_e), TaskObjF__TaskObjF's own ctor */
/* the existing slot44 field immediately follows -- no other offset moves */
```

(concretely: shrink `pad3C` to cover only `0x03C`, insert the `slot40`
field, and the existing `slot44` field immediately follows -- no other
offset in the struct moves). Posted to the broadcast.
