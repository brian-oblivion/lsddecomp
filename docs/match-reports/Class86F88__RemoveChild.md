# Class86F88__RemoveChild -- MATCHED (32/32 words)

> Renamed from `Class86F88_3bb8c_j__RemoveChild` on 2026-09-24 (tools/rename.py). Address 0x80051da0.

> Renamed from `func_80051DA0` on 2026-09-24 (tools/rename.py). Address 0x80051da0.

Unit: `src/class_3bb8c_j.c`. `self` is `Class86F88_3bb8c_j`. The "remove child"
counterpart to `Class86F88__AddChild`: clears whichever of the two tagged caches
`arg1` matches, THEN unregisters it from the inherited
`BasicClass::removeChild` (order reversed from `Class86F88__AddChild`'s
add/tag-check order -- see that function's report).

## Body

```c
void Class86F88__RemoveChild(Class86F88_3bb8c_j *self, void *arg1)
{
    s32 tag;

    if (arg1) {
        tag = **(s32 **)arg1 & 0xF;
        if (tag == 2) {
            self->unk34 = NULL;
        } else if (tag == 5) {
            self->unk38 = NULL;
        }
        Get_vtable_BasicClass()->removeChild(self, arg1);
    }
}
```

Matched first try.

## Naming

- `Class86F88__RemoveChild` -- tier A. The removeChild occupant (classtable.py gClass86F88Methods +0x014): clears the matching tag cache then chains the base removeChild. Mirrors Class86F88__AddChild.
