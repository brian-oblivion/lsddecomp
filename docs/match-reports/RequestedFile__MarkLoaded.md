# RequestedFile__MarkLoaded -- MATCHED (3/3 words), round 82

> Renamed from `RequestedFile__SetFlag` on 2026-09-26 (tools/rename.py). Address 0x800423e4.

> Renamed from `Class6EED8__SetFlag` on 2026-09-26 (tools/rename.py). Address 0x800423e4.

> Renamed from `D8006EED8__SetFlag2C` on 2026-09-26 (tools/rename.py). Address 0x800423e4.

> Renamed from `func_800423E4` on 2026-09-25 (tools/rename.py). Address 0x800423e4.

Round 82, runner alpha (re-staffed slot). Unit `src/graphics/Sprite.c`. Fresh
ground (carved in FINISHING-PLAN revision 18), no prior attempt.

- **Where:** gRequestedFileMethods slot +0x064 (a FileResource-derived table, id 0xB03) (`tools/classtable.py`).
- **What:** sets the object's +0x02C word to 1.
- **Result:** byte-exact on the FIRST build; 3/3 words, 0 insertions /
  0 deletions, whole-image SHA1 green. No levers needed.
- **Types:** SceneNode-derived methods take `SceneNode *` from the UNIFIED
  `include/SceneNode.h` (untouched). The FrameClock and RequestedFile objects use
  unit-local views (`D_8006EF50Obj`, `D_8006EED8Obj`) declared at the top of
  the unit; nothing was added to a shared header.

## Source

```c
/* gRequestedFileMethods slot +0x064. */
void RequestedFile__MarkLoaded(D_8006EED8Obj *self) {
    self->flag2C = 1;
}
```

## Naming

- `RequestedFile__MarkLoaded` -- tier A. Slot +0x064: sets flag2C = 1. A pure setter; the deeper game meaning of flag2C is not established (kept as flagNN rather than invented), but the setter's own mechanics ARE its purpose.

## Track 4 (2026-09-26, round 87, delta)

Class id 0xB03 is unified as `RequestedFile` in `include/RequestedFile.h`
(FILERESOURCE_SLOTS/FIELDS, 0x30 bytes, one own field `loaded` at +0x02C). The
unit-local views `D_8006EED8Obj`/`D_8006EED8Methods` and the single-slot cast
views `Slot0CMethods_322b4`, `Slot08Arg0Methods_322b4` and
`CtorArg1Methods_322b4` are gone; `GetActiveDataSourceMethods` is declared
`FileResourceMethods *`. The Source block above is the round-82 text; the live
body in `src/graphics/Sprite.c` is byte-identical.

Renamed from `D8006EED8__SetFlag2C` with rename.py: the occupant of
FileResource's +0x064 `setFlag`, named for its slot as `PlacementGrid__SetFlag`
is; the body does nothing beyond what the slot says. The field it sets is
`loaded` (see RequestedFile__RequestedFile's Track 4 paragraph for the evidence:
setFlag is the driver's completion callback).

## Track 6 (2026-09-26, round 93, alpha)

The class `Class6EED8` (table `gClass6EED8Methods`, id 0xB03) is now
`RequestedFile` (`python3 tools/renametype.py Class6EED8 RequestedFile`,
tier A): its whole behaviour is to request one named file from the active
driver at construction (the ctor's requestLoadFile, +0x06C) and record in
`loaded` that the driver's setFlag (+0x064) reported it read; the ctor and
finalize clear `loaded`. It adds no buffer-consuming step, so the name says
what the methods do and no more. Round 87's header kept the address-derived
name because the only thing known beyond those mechanics was the caller's use
(WBgm loads SEQ files through it); a mechanics name sidesteps that objection
rather than overriding it, and `SeqFile` was rejected for the same reason.
The table and getter followed (`gRequestedFileMethods`,
`GetRequestedFileMethods`), and the header moved to `include/RequestedFile.h`.
renametype.py also rewrote the old class name inside earlier sections'
history prose in this and sibling reports (known, pending an operator
decision; not hand-reverted).

`RequestedFile__SetFlag` -> `RequestedFile__MarkLoaded`
(`python3 tools/rename.py RequestedFile__SetFlag RequestedFile__MarkLoaded`,
tier A): the whole body is `self->loaded = 1`, a pure setter. The slot it
occupies stays `setFlag`: the drivers call it after every completed request
(CdDriver__RunRequestQueue on a queued op, CdDriver__LoadFile's synchronous
branch), and this class's override is what turns that into `loaded`.
