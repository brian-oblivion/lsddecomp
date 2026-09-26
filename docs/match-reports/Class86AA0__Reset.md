# Class86AA0__Reset

> Renamed from `func_8004D42C` on 2026-09-26 (tools/rename.py). Address 0x8004d42c.

**Unit:** class_3bb8c_c · **Size:** 2 words · **Status:** MATCHED (2/2)

## What it does

Empty vtable stub: `gClass86AA0Methods`'s own slot +0x040. Splat-generated
body (`jr $ra; nop`) -- no logic, no arguments read, nothing to derive.

## The C

```c
void Class86AA0__Reset(void) {}
```

## Notes

Matched trivially; not itself round-68 work (already matched before this
naming pass touched the unit). Filed here only because track 3 asks for a
report per function, matched ones included.

## Naming

**Class86AA0__Reset** -- tier C, kept deliberately. Same shape and same
precedent as `Class869D8__InitDefaults`/`func_8004D35C`/`func_8004D364`/
`func_8004D36C`/`func_8004D374` above (this unit's own `Class869D8`
siblings) and as `func_8004B324`/`SceneNode__NoOpSlot5C` elsewhere in the project:
a genuinely empty, no-argument, no-established-purpose vtable stub keeps
its bare `func_` name rather than a `Class86AA0__func_...` form that would
assert a class-specific tie the body does not support.

## Track 4 (2026-09-26, round 88, alpha)

Renamed `func_8004D42C` -> `Class86AA0__Reset` (tools/rename.py). The body
is still empty; the name is the slot's, per FINISHING-PLAN track 4 step 6
(an override is named for its slot). `classtable.py gClass86AA0Methods --vs
gSceneNodeMethods` puts it at +0x040, where SceneNode's table has
`SceneNode__Reset` (zeroes `tick` and the rotation/scale): Class86AA0
replaces the inherited reset with nothing. The "tier C, kept deliberately"
paragraph above predates the class's unification; the tie to the class is
the table slot, which is the evidence it lacked. Precedent:
`Class869D8__InitDefaults`, an empty occupant named for its slot.
