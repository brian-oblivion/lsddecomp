# Class86F88__ResetView -- MATCHED (4/4 words)

> Renamed from `Class86F88__ResetCounters` on 2026-09-26 (tools/rename.py). Address 0x80051f14.

> Renamed from `Class86F88_3bb8c_j__ResetCounters` on 2026-09-24 (tools/rename.py). Address 0x80051f14.

> Renamed from `func_80051F14` on 2026-09-24 (tools/rename.py). Address 0x80051f14.

Unit: `src/class_3bb8c_j.c`. `self` is `Class86F88_3bb8c_j`.

## Body

```c
void Class86F88__ResetView(Class86F88_3bb8c_j *self)
{
    self->unk20 = 0;
    self->unk24 = 0;
    self->unk28 = 0;
}
```

Trivial three-field reset, an unrelated field group from `Class86F88__ClearCachedRefs`'s
(`unk34`/`unk38`/`unk50`). Matched first try.

## Naming

- `Class86F88__ResetView` -- tier A. The slot40 occupant (classtable.py gClass86F88Methods +0x040, the ctor's own tail dispatch): zeroes unk20/unk24/unk28. A pure leaf, tier A by the track-3 rule.

## Track 4 (2026-09-26, round 89)

Renamed from `Class86F88__ResetCounters`: the three words it zeroes are
`topIndex`, `column` and `cursorIndex` (+0x020..+0x028, named from
class_3bb8c_k's accessors), the trio Class86F88__SetView sets. They are
the list's view position, not counters.
