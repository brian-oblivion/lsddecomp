# Class86F88__ReleaseResources -- MATCHED (28/28 words)

> Renamed from `Class86F88_3bb8c_j__ReleaseResources` on 2026-09-24 (tools/rename.py). Address 0x800520a0.

> Renamed from `func_800520A0` on 2026-09-24 (tools/rename.py). Address 0x800520a0.

Unit: `src/class_3bb8c_j.c`. `self` is `Class86F88_3bb8c_j`.

## Body

```c
void Class86F88__ReleaseResources(Class86F88_3bb8c_j *self)
{
    if (self->unk50) {
        self->methods->slot90(self);
        self->unk50 = self->unk50->methods->slot4(self->unk50);
    }
}
```

Established `self->unk50`'s type as `Class86F88Handle_3bb8c_j *` (already
introduced this round from `Class86F88__LoadResources`'s evidence) and
`Class86F88Methods_3bb8c_j::slot90` (+0x090, self-only). `Class86F88HandleMethods_3bb8c_j::
slot4` (+0x004, self-only, returns a handle-typed pointer stored back into
`self->unk50` -- a "release, returns the successor/NULL" shape) was
already declared for `Class86F88__LoadResources`'s use; this is the second confirming
call site. Matched first try.

## Naming

- `Class86F88__ReleaseResources` -- tier B. The slot48 occupant (classtable.py gClass86F88Methods +0x048): if unk50 is set, calls slot90(self) then releases unk50 through its own slot4. Mirrors Class86F88__LoadResources's load in reverse -- mechanics clear, purpose not established beyond "release what LoadResources acquired".
