# Class86F88__DetachTarget -- MATCHED (22/22 words)

> Renamed from `Class86F88__RemoveCachedChildren` on 2026-09-26 (tools/rename.py). Address 0x8005217c.

> Renamed from `Class86F88_3bb8c_j__RemoveCachedChildren` on 2026-09-24 (tools/rename.py). Address 0x8005217c.

> Renamed from `func_8005217C` on 2026-09-24 (tools/rename.py). Address 0x8005217c.

Unit: `src/class_3bb8c_j.c`. `self` is `Class86F88_3bb8c_j`.

## Body

```c
void Class86F88__DetachTarget(Class86F88_3bb8c_j *self)
{
    self->methods->slot14(self, self->unk34);
    self->methods->slot14(self, self->unk38);
    self->unk3C = 0;
}
```

Calls `Class86F88Methods_3bb8c_j::slot14` (established this round from
`Class86F88__AddChild`'s asm, self+one pointer arg) twice, once per tagged-child
cache, then clears `self->unk3C` (also established this round, from
`Class86F88__AttachTarget`). Matched first try.

## Naming

- `Class86F88__DetachTarget` -- tier A. The slot50 occupant (classtable.py gClass86F88Methods +0x050): calls slot14 (this class's own removeChild override) on unk34 then unk38, then clears unk3C. Purpose (release the two tag-cached children) follows directly from the mechanics, matching the addChild/removeChild caching pattern established in Class86F88__AddChild -- tier A.

## Track 4 (2026-09-26, round 89)

Renamed from `Class86F88__RemoveCachedChildren`: the body is
TextEntry__DetachTarget's at the same slot (+0x050), removeChild on the two
cached children then `target = NULL`, and it undoes Class86F88__AttachTarget.
Its caller TaskObjF__DetachChildB drives it where TaskObjF__DetachChildA drives
TextEntry's detachTarget.
