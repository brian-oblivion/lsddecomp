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
