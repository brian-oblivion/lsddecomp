# Class86F88__ClearCachedRefs -- MATCHED (4/4 words)

> Renamed from `Class86F88_3bb8c_j__ClearCachedRefs` on 2026-09-24 (tools/rename.py). Address 0x80051c74.

> Renamed from `func_80051C74` on 2026-09-24 (tools/rename.py). Address 0x80051c74.

Unit: `src/class_3bb8c_j.c`. `self` is `Class86F88_3bb8c_j`.

## Body

```c
void Class86F88__ClearCachedRefs(Class86F88_3bb8c_j *self)
{
    self->unk34 = NULL;
    self->unk38 = NULL;
    self->unk50 = NULL;
}
```

Trivial three-field reset (the two "tagged child" caches plus `unk50`).
Matched first try -- the trailing store naturally lands in the `jr $ra`
delay slot.

## Naming

- `Class86F88__ClearCachedRefs` -- tier A. Plain three-field reset (unk34=unk38=unk50=NULL), called once from the ctor. A pure leaf whose mechanics ARE its purpose (tier A by the track-3 rule). Not a classtable.py slot -- called directly by name from the ctor.
