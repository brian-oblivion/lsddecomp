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

### include/text_row.h

- The banner opened its evidence list with "The name is for what the
  class's own methods do, and the evidence is this:".
- On the reset override: "the ctor, which passes it, casts to
  TextRowResetFn (no code), as CharSprite's ctor does for its cell", i.e.
  the cast emits no instructions.
- The method table's override list cited its command: "`tools/classtable.py
  gTextRowMethods --vs gCharSpriteMethods` lists the overrides of the
  inherited ones: TextRow__TextRow, __Finalize, __Reset, __AttachToParent,
  __DetachFromParent, __SetDisplay, __SetColor, __SetPosition, __SetCellAt
  and __NoOpGetCell (empty, at getCell)."
- The field comments called TitleMenu's TextRow its "name field"; it is
  the save title (TitleMenu__CreateSaveTitle).

### include/text_entry.h

- The banner opened its evidence list with "The name is for what its own
  methods do, and the evidence is this:" and closed "What the string is in
  the game is not established." The class doc now says what it is: its one
  maker, TaskObjF__AttachTextEntry, hands it `&title[titleEditPos * 2]`,
  and TitleMenu__SaveToCard passes sSaveTitle as that title, so the string
  is the memory-card save title.
- The method table's override list cited its command: "`tools/classtable.py
  gTextEntryMethods --vs gBasicClassMethods`".

### include/task_objf.h

- The method table's override list cited its command: "BasicClass's slots
  (overrides: +0x008 TaskObjF__TaskObjF, +0x00C Finalize, +0x010 AddChild,
  +0x014 RemoveChild, +0x018 RemoveAllChildren, +0x038 OnNotify;
  `tools/classtable.py gTaskObjFMethods --vs gBasicClassMethods`), then this
  class's own, every one of them filled."
- The banner split the methods by the unit files they lived in before the
  units were merged into src/ui/title_menu.c ("... (+0x064..+0x078,
  +0x038) in src/ui/title_menu.c, the state machine (+0x07C..+0x0B0) in
  src/ui/title_menu.c"); every method is now in the one file.
- McDevicePath, IconPaletteHalf and IconFrame carried `MATCHING:` notes on
  their alignment; they moved to BuildMemcardPath's and
  TaskObjF__TryWriteMemcardSaveFile's reports, with one-line `MATCHING:`
  comments at the copies in title_menu.c.

### include/title_menu.h

- The banner said TitleMenu has "fourteen overrides and six slots of its
  own". Measured with `python3 tools/classtable.py gTitleMenuMethods --vs
  gTaskCoreMethods` in round 106: 12 slots OVERRIDDEN against TaskCore and 6
  ONLY IN FIRST; the class doc now says twelve and lists them.
- The TitleMenu::dreamSys field comment named the DreamSys slots the class
  calls by offset: "its +0x0F0/+0x19C/+0x1A0/+0x1A8/+0x1AC/+0x1B0 are
  called"; the field doc now names what they are for.
