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
