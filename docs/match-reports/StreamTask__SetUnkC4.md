# StreamTask__SetUnkC4

> Renamed from `StreamTaskObj__SetUnkC4` on 2026-09-26 (tools/rename.py). Address 0x8003be5c.

> Renamed from `func_8003BE5C` on 2026-09-23 (tools/rename.py). Address 0x8003be5c.

**Unit:** code_2c054 · **Size:** 2 instructions (0x8 bytes) · **Status:** MATCHED (2/2 words, whole-image SHA1 green), first attempt

## What it does

A plain setter: `self->unkC4 = value;`. One of a run of five consecutive
9-line, 2-instruction `.s` bodies (`StreamTask__SetUnkC4`/`64`/`6C`/`74`/`7C`) that
turned out to be ordinary field setters, not BIOS trampolines or anything
toolchain-blocked (checked first per the runner brief: no `jr $t2`, no
`gp_rel`, no `addiu $at,$at,%lo`).

All five (plus `Get_vtable_StreamTask` right after them) are consecutive slots
`+0x124`.."+0x134` of this class's own method table `gStreamTaskMethods` (confirmed
with `tools/classtable.py gStreamTaskMethods`), which is how their object type was
identified: `New_StreamTask`'s allocator call sizes the object at `0xDC`
bytes and constructs it through `Get_vtable_StreamTask`'s slot `+0x008`, so all five
setters, plus `Get_vtable_StreamTask` itself, operate on that same `0xDC`-byte
class (already named `StreamTask`/`StreamTaskMethods` in
`include/GameApplication.h`, established independently by a different unit from
`New_StreamTask`'s cross-unit call site).

## Derivation

```
jr   $ra
 sw  $a1, 0xC4($a0)
```

```c
void StreamTask__SetUnkC4(StreamTaskObj *self, s32 a1) {
    self->unkC4 = a1;
}
```

Matched first attempt: a plain setter compiles directly to "store in the
return's delay slot", no residue.

## New struct/header knowledge

Added `include/code_2c054.h`: this unit's own local view of the `StreamTask`
class (named `StreamTaskObj`/`StreamTaskObjMethods` here, independent of
`GameApplication.h`'s same-named-concept `StreamTask`, per the `Entity.h` /
`code_55dd4.h` precedent of keeping local views separate rather than editing
another unit's header) — fields `unkC4`/`unkC8`/`unkCC`/`unkD0`/`unkD4` (this
run of setters), plus everything else this unit's other queued functions in
the same batch needed. See the sibling reports for the rest.

## Proposed learning

Confirms the runner brief's own warning was correctly aimed: a run of
same-size tiny bodies in this unit turned out to be case (a) (trivial
setters), not case (b) (BIOS trampoline) — worth checking `jr $t2` first,
but don't assume every such run is a trap; sometimes it really is just
setters, and `tools/classtable.py` on the enclosing method table confirms it
cheaply (all five sit in five consecutive table slots).

## Naming

**StreamTask__SetUnkC4** -- tier A. Plain single-field setter (one of a
run of five identical-shape setters at consecutive table slots
`+0x124`..`+0x134`); `Class__SetUnkNN` convention, same precedent as
`StreamTask__SetFrameBound`.

## Track 4 (2026-09-26, round 87)

Renamed with the class unification (gStreamTaskObjMethods -> class StreamTask, include/StreamTask.h): the `Obj` suffix is dropped (track 4 step 2; include/GameApplication.h already viewed the class as `StreamTask`). Own slot +0x124. unkC4 is MoviePlayer__Play's third argument (stored at the player's +0x054); left unnamed until MoviePlayer's class is.
