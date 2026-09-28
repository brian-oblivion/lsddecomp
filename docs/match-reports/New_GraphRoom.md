# New_GraphRoom -- MATCHED (24/24)

> Renamed from `New_GraphRoomObj` on 2026-09-26 (tools/rename.py). Address 0x80057f68.

> Renamed from `func_80057F68` on 2026-09-24 (tools/rename.py). Address 0x80057f68.

Unit: `src/class_3bb8c_t.c`. Class: `gVariantSpriteMethods` family -- plain
allocator/constructor wrapper (`New_X` shape), not a vtable slot.

## Signature

```c
void *New_GraphRoom(void *arg1);
```

## Body

```c
void *New_GraphRoom(void *arg1) {
    void *obj = BMemPMgrAlloc(0x244);
    if (obj != NULL) {
        GetGraphRoomMethods()->ctor(obj, arg1);
        return obj;
    }
    return NULL;
}
```

Textbook `New_X`: allocate `0x244` bytes, null-check, call the
constructor fetched from `gGraphRoomMethods`'s `+0x008` slot (via this unit's
own `GetGraphRoomMethods`, still queued -- forward-declared here) with `(obj,
arg1)`, return the allocation regardless of the ctor's own return value.
`return obj;` sits INSIDE the success `if`, with a trailing `return
NULL;` -- the shape established as necessary for this exact pattern in
`class_3bb8c_p`'s `New_VariantSprite` report.

This introduces this unit's own view of `gGraphRoomMethods` (73 slots,
`D_80087AACMethods`/`D_80087AACObj`, currently typing only the ctor slot
`+0x008`) -- this unit owns the WHOLE class (ctor, dtor, every slot are
all in this file), so the struct will grow as more of its functions are
matched.

## Verify

```
./build-and-verify.sh   # build exit=0, OK: build matches retail
tools/funcdiff.py New_GraphRoom   # 24/24
```

## Naming (round 75, track 3)

**`New_GraphRoom`** -- tier B. `New_X`-shaped allocator (allocate
`0x244` bytes, null-check, dispatch the ctor slot) for the class named
`GraphRoomObj` this round -- see `src/class_3bb8c_t.c`'s own header
comment for the class-identity evidence (loads "ETC\HGRAPH.TIM", builds
100 coloured points from a day-type ring).

## Track 4 (2026-09-26, round 87, alpha): renamed `New_GraphRoomObj` -> `New_GraphRoom`

The class is unified in `include/GraphRoom.h` (class id 0x2F130, table `gGraphRoomMethods`, parent TaskCore; `tools/classtable.py gGraphRoomMethods --vs gTaskCoreMethods`). The class name drops round 75's `Obj` suffix (FINISHING-PLAN track 4 step 2); every function prefix moved with it. The unit's GraphRoomObj/GraphRoomMethods views are deleted and `self` is `GraphRoom *`; inherited fields and slots carry TaskCore's and IntermediateBase's names. Zero bytes changed (build-and-verify OK, typeviews --warnings 0 new). Prefix only. Now returns `GraphRoom *` and takes `struct DreamSys *`: its one caller, GameApplication__RunTitleMenu (src/GameApplicationFileResource.c), passes `self->dreamSys` through GameApplication__RunTask, and casts New_GraphRoom to PollTaskCtor there; include/GameApplication.h's own `extern PollTask *New_GraphRoom(void *)` is deleted.

## Track 7 (2026-09-27, round 97, delta): the unit banner, moved here

The unit banner of `src/class_3bb8c_t.c` was rewritten to say what the file holds. Its history, verbatim as it stood before the pass (the class-identity reading and the round-87 correction are this class's, so they live with its allocator):

```c
/*
 * class_3bb8c_t -- functions 96..112 of the 113-function `class_3bb8c_n`
 * remainder, 0x48738..0x48F74 (vram 0x80057F38..0x80058774).  Carved
 * MID-round 17 (2026-09-04) to re-staff a runner whose own unit was
 * exhausted.  This is the LAST slice of the class_39e08 block.
 *
 * EXPECT THIS SLICE TO SPAN MORE THAN ONE CLASS.  It is cut at ROM
 * addresses, not class boundaries.  Identify each with tools/classtable.py.
 * It holds two classes:
 *
 * - Four empty leaves plus the table getter (VariantSprite__Update,
 *   VariantSprite__NoOpSlotBC/C0/C4, GetVariantSpriteMethods) of the
 *   unrelated VariantSprite (include/VariantSprite.h; its ctor is in
 *   `class_3bb8c_p`, two more methods in `class_3bb8c_q`).
 * - The WHOLE of `GraphRoom` (round 75 name; table `gGraphRoomMethods`,
 *   73 slots), a TaskCore subclass, unified in include/GraphRoom.h (track 4,
 *   round 87; the header's banner has the slots, fields and evidence).
 *   This unit owns the entire class: allocator, ctor, every override,
 *   ScoreDayLog and the getter.
 *
 * `GraphRoom`'s identity (round 75, track 3 naming pass; tier B -- the
 * MECHANICS below are certain, the in-game name is a strong but unconfirmed
 * read): `GraphRoom__Reset` sets the literal texture string
 * `"ETC\HGRAPH.TIM"` as the sub-handle. The class owns a 100-entry array of
 * small coloured `New_BoxFill` point objects (`points`) built by
 * `BuildGraphPoints` and positioned by `PopulateGraphPoints` from a
 * backwards walk of the DreamSys's 365-entry mood ring
 * (`DreamSaveBlock::moodPreviousDays`, reached through the save block
 * `dreamSys`'s GetSaveBlock returns) -- each day's two signed bytes become
 * an `{x, y}` point handed to a point's `attachAbsolute`. `ScoreDayLog`
 * separately scans that same ring for four fixed mood targets
 * (`gGraphScoreMoods`) and records, per target, the dot index it last matched at;
 * `TickHighlight` later highlights the matching point. Together this is
 * the in-game graph screen that plots mood history as coloured dots.
 * Round 87 correction: the ring IS DreamSys's `moodPreviousDays` -- the
 * ctor's argument is GameApplication's dreamSys, and the record's offsets are
 * DreamSys's fields relative to saveMagic (see DreamSaveBlock below). The
 * earlier "not DreamSys, the offsets don't line up" compared them against
 * the start of DreamSys rather than the save block.
 */
```

Unchanged in the pass: this function's body, except `BMemPMgrAlloc(0x244)` is `BMemPMgrAlloc(sizeof(GraphRoom))` (the struct is 0x244 bytes; zero bytes changed).
