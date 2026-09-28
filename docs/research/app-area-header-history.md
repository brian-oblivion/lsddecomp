# Header history: the app area (track 12)

Track 12 turned the app area's headers (`include/basic_class.h`,
`application.h`, `game_application.h`, `bmem_pmgr.h`, `data_source.h`,
`file_resource.h`, `intermediate_base.h`, `pad.h`, `stream_task.h`,
`task.h`, `task_core.h`, `viewport.h`) into Doxygen API documentation. What
their comments said about the project rather than the code, and that
belongs to no single function's match report, is kept here, per header,
as it stood before the pass. Function-specific process text went to that
function's report under a "History (moved from ..., track 12)" heading.

## History (moved from include/basic_class.h, track 12)

- The banner called BasicClass "the root of the game's hand-rolled class
  framework (docs/research/class-framework.md)". That note is the evidence
  that the framework is plain C and not C++.
- The class-id tree was pointed at with `python3 tools/typeviews.py --tree`;
  a subclass's overrides with
  `tools/classtable.py <table> --vs gBasicClassMethods`.
- The reason subclasses expand the parent's FIELDS/SLOTS macros instead of
  embedding a parent struct was given as: every accessor stays flat at any
  depth "and cc1 sees exactly the layout it saw when each function matched".
- Slot +0x03C was documented as "NULL in all 59 method tables". The release
  review (2026-09-28) counted 60 tables. Measured at this pass: of the 58
  tables `classtable.py --scan` names `g...Methods`, the 57 subclass tables
  all show +0x03C as a null slot (the scan ends gBasicClassMethods itself
  at +0x038). The header now says "NULL in every class's table" and quotes
  no count.

## History (moved from include/file_resource.h, track 12)

- The class banner pointed at `typeviews.py --tree` for the subclass list
  and said it "lists all sixteen"; the header now names examples and quotes
  no count.
- The static tables' interface slots were said to be copied into "the table
  of every class sDataSourceClientGetters lists"; that array is
  game_shell.c's own (a NULL-terminated list of table getters), so the
  header now says "every client class".
- The `flags` field said "bit 0 set by SetFlag"; the setter is
  FileResource__OnRequestDone, and the field doc says so.

## History (moved from src/app/game_shell.c's banners, track 12)

The file's two long banners (28 and 26 lines) were split: what
GameApplication does and the order of its hooks went to the class
documentation in include/game_application.h, the data-source layer's
description to include/data_source.h's `@file` block, and FileResource's
to its class documentation. The .c keeps a short banner per section. No
process text was in them.

## History (moved from include/pad.h, track 12)

- The class banner cited `tools/classtable.py gPadMethods --vs
  gBasicClassMethods` for "overrides ctor and finalize, adds six slots";
  the method table's doc now states that without the command.

## History (moved from include/task_core.h, track 12)

- The class banner cited `typeviews.py --tree` for the three subclasses
  whose ctors call TaskCore__TaskCore first.
- The fields bgLayer, tileMap and tileAtlas were marked "(tag only here)":
  the header declares them by struct tag, and the class documentation now
  says a caller includes bg_layer.h, tile_map.h or tile_atlas.h.
- Section 2 of src/app/task.c opened with a 22-line banner describing the
  slot and item-list picker; it is now the "picker" paragraph of TaskCore's
  class documentation, and the .c keeps a short section banner.

## History (moved from include/stream_task.h, track 12)

- The class banner cited `tools/classtable.py gStreamTaskMethods --vs
  gTaskCoreMethods` for "fourteen overrides and five slots of its own".
- It listed the MoviePlayer slots the class calls by offset: +0x040 play,
  +0x048 advance, +0x04C abort, +0x06C setAutoPlay, +0x004 release
  (movie_player.h). The class documentation names them without offsets.
- The field docs for autoPlay and keepActive gave MoviePlayer's field
  offsets (+0x068, +0x054); the names are kept, the offsets are
  movie_player.h's to state.

## History (moved from include/viewport.h, track 12)

- The slot macro's comment cited `tools/classtable.py gViewportMethods --vs
  gBasicClassMethods` for the list of inherited slots Viewport overrides.
- ViewportRefView's comment justified the local struct partly by bytes:
  setViewPoint/setViewRef copy vp and vr "as one LongVec3 (three field
  stores do not match)", and "Sony's flat vpx..vrz would put a LongVec3 cast
  at each copy to save the two at GsSetRefView2". The header keeps the
  reader's reason (the game treats vp and vr as vectors).
- Section 4 of src/app/task.c opened with a 27-line banner listing
  Viewport's methods by group; the list is now in the class documentation,
  and the .c keeps a short section banner.

## History (moved from include/intermediate_base.h, track 12)

- The class banner cited `typeviews.py --tree` for the two subclasses.
- It described init in the placeholder field names of an earlier
  IntermediateBaseInitArgs (`args->unk0`, `args->unk4`, "the +0x010 helper",
  "the +0x014 object", `initArgs->unk0`), which the release review
  (2026-09-28) listed as stale. The class documentation now uses the fields'
  names: drawSystem, pad, frameClock, lightRig.
