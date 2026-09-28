# UI headers: notes moved out of the API docs

Track 12 (round 106, area-ui) rewrote the headers of `src/ui/` as Doxygen
API documentation. Process text that explained how a header was derived,
rather than what it declares, and that belongs to no single function's
match report, moved here. Text tied to one function went to that
function's report under "History (source comments moved in track 12,
round 106)".

## History (source comments moved in track 12, round 106)

### include/box_fill.h

- The override list was a command's output: "`tools/classtable.py
  gBoxFillMethods --vs gSceneNodeMethods` lists the overrides of the
  inherited ones: +0x008 (BoxFill__BoxFill), +0x040 (BoxFill__Reset), +0x04C
  (BoxFill__AttachToParent), +0x060 (BoxFill__SetDisplay), +0x064
  (BoxFill__SetSemiTrans), +0x068 (BoxFill__SetSemiTransRate)."
- On overrides whose parameter list differs from the inherited slot: "a
  caller reaching the override through the slot casts to the typedef below
  it (no code)", i.e. the cast emits no instructions.
- The banner's naming evidence opened "The name is for what the class does,
  and the evidence is this:", followed by the Viewport__DrawNode path, the
  own slots and the users, which the class doc keeps as description.

### include/fade_box.h

- The method table's override list cited its command: "BoxFill's slots
  (overrides: +0x008 FadeBox__FadeBox, +0x040 FadeBox__Reset, +0x098
  FadeBox__Update; `tools/classtable.py gFadeBoxMethods --vs
  gBoxFillMethods`), then this class's own."

### include/grid_cell.h

- The linking list was headed "Linking (`classtable.py gGridCellMethods
  --vs gSceneNodeMethods`):".

### include/node_guarded_viewport.h

- The override list was headed "What it changes (`classtable.py
  gNodeGuardedViewportMethods --vs gViewportMethods`):".
