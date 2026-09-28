# Release review, 2026-09-28 (after round 102)

Six read-only reviewers, one per area (src/app+cd+main, src/graphics,
src/world, src/ui+sound+psyq, include A-L, include M-Z), each told the
image is byte-exact so only readability and consistency count. Every
finding below was checked against the tree by its reviewer (grep, cpp,
nm on the SDK objects). It is the starting list for phase 3's items
(FINISHING-PLAN §3, tracks 10 to 12), grouped by the item that fixes it.
Line numbers are as of commit b23bdf545 and drift; search for the name.

**Verdict.** The function bodies are in good shape: one-line `MATCHING:`
notes, Sony types under Sony's names, few raw casts. DrawSystem.c,
FlatLightObj.c, TimImage.c, Pad.c, CdStream.c, Application.c, StageGrid.c,
TodActor.c, DayTaskStageMap.c, WBgm.c and ScreenWidgets.c's FadeBox code
are close to release quality. The debt is in declarations, a few
conventions, names the code has outgrown, and headers written as analysis
rather than API.

## Track 10 `prototypes`: one declaration per name

- **Allocator.** `BMemPMgrAlloc` is re-declared locally in about 20 units
  and in DreamSys.h:376, Entity.h:308, DayTaskStageMap.h:30, Task.h:27.
  `BMemPMgrFree` returns `void` in Entity.h:309 and
  GameApplicationFileResource.h:16 but `void *` in BMemPMgr.h:83 and Task.h:28,
  so any unit including two of them fails with conflicting types. main.c:28
  declares `BMemPMgrInit`/`SetDefaultBMemPMgr` because BMemPMgr.h omits them.
  BMemPMgr.h:82 leaves Alloc unprototyped on purpose (arity); keep that as one
  `MATCHING:` line, and put a public prototype pair where BMemPMgr.c does not
  see it.
- **SoundCueSet.** `InitSoundCueSet`, `FlushSoundCueSet`,
  `ServiceSoundCueSet` have three spellings each (DreamSys.c:72-73, 497;
  Entity.c:298-299, Entity.h:310-311; ObjMStyleActor.c:1782, 1893, 1903);
  the definitions take `(VabStreamObj *, SoundCueSet *)`. Declare once in
  SoundCueSet.h.
- **GameFiles.** No header. `GetStageMapChunkRecord`/`...XY` are declared
  `s32`/`void` in ObjMStyleActor.c:462-473 but return `FilePathRecord *`
  (GameFiles.c:317, 322); `ObjM__GetGridRecord` is declared void and "returns"
  through $v0. `PickSoundBank` returns `s32` for a path (GameFiles.c:203),
  `PickStageBgm` likewise (ObjMStyleActor.c:525). `RecPick{group,sub}` and
  DreamSys.h's `CinematicCall{bank,entry}` are one packed pair, repacked by
  hand at GameApplicationFileResource.c:80, 317. Add GameFiles.h.
- **Full-width SJIS helpers.** `DecodeFullWidthSjis` is `u8 *(u8 *, u8 *)`
  (ScreenWidgets) but `void (void *, void *)` at TitleMenuTaskObjF.c:316 and
  `char *(char *, char *)` at TextEntryItemList.c:41, 511;
  `FormatFullWidthNumber` re-declared at TitleMenuTaskObjF.c:193. One header.
- **Others without a header or disagreeing:** `BuildFileName`
  (TextEntryItemList.c:155, 629; PlacementGridVabSound.c:185, with `const`);
  `GetSsSizeTableBuf` (`void *` vs `char *`); `GetSsTicksPerSecond` (WBgm.c:21
  only); `LockCd`/`UnlockCd` declared `s32` at GameApplicationFileResource.c:535,
  defined `void`; `SetActiveDataSourceDriverMode` `s32` in DayTaskStageMap.h:57
  vs `void` definition; `GetSoundEffectDir(s32)` vs `(void)`;
  `IsStyleVariantEven` `bool` vs `s32`; `ReleaseBasicClassArray` in Task.h:32
  `(void *, void *)` vs `(BasicClass **, s32)` at TmdRenderer.c:86 (plus three
  `void **` spellings); `GetActiveDataSourceMethods` local in 4 files;
  `GetSetBitField` duplicated (Sprite.c:41, Task.h:36 vs SceneNode.h:216);
  `ApplyMatrixToLVArray` extern at ViewportDraw.c:62 though SceneNode.h is
  included; `sHitHeightGate` declared twice in SceneNode.c. CdDriver.c
  re-declares its own functions two or three times (55/60/794/795, 58/463/907,
  ...): keep one forward block.
- **Sony names re-declared.** `ResetGraph` (Task.h:40), `rand`, `strlen`,
  `strcat`, `strcpy`, `memset`, `memcpy`, `printf` (declared `void (const char *)`
  at TitleMenuTaskObjF.c:980 and WBgm.c:18 against stdio.h's variadic `int`).
  Use include/psyq's headers (verify bytes: strings.h's `strlen` is K&R).
- **BIOS and card calls with no declaration at all** (cpp-confirmed in
  TitleMenuTaskObjF.c): OpenEvent, TestEvent, CloseEvent, EnableEvent,
  DisableEvent, InitCARD, StartCARD, _bu_init, _card_info/_load/_clear, open,
  read, write, lseek, close, delete, format; `EnterCriticalSection` used before
  its `extern void` at :1065. The comments at :501, 647, 870, 1046 claim
  kernel.h declares them; it has no prototypes. Bring libapi.h from the SDK
  disc into include/psyq.
- **Duplicate types.** `RotationRatio(s)` (DreamSys.h:172) is `Ratio16`;
  `SubObjE` (DayTaskStageMap.h:18) is a DrawSystem view; `SlotEntry`/`SrcDesc`
  (Task.c:658, 670) are one record; `CdStreamFile` (CdStream.h:44) is
  `CdlFILE`; a signed 3-byte colour is defined six times (`BgLayerRgb`,
  `BoxFillRgb`, `FlatLightColor`, `LightRigRgb`, `SpriteRgb`, `ViewportRgb`):
  one type. `FIX12_SHIFT` and `CD_SECTOR_SIZE` redefined at
  GraphicsResources.c:63-64. `ABS_fa50` (TmdModel.c:53) open-coded twice more.
- **Headers not self-contained:** six M-Z headers use `GsCOORDINATE2`,
  `SVECTOR`, `MATRIX`, `RECT` without including Sony's header.

## Track 10 `conventions`

- **Getters:** 13 `Get_vtable_X` against 46 `GetXMethods` (BasicClass,
  DreamSys, Entity, CdStream, DrawSystem, FlatLightObj, FrameClock,
  IntermediateBase, Pad, StreamTask, TaskCore, TmdModel, WBgm). Rename; add
  the form to §3's conventions.
- **Receiver:** `this` in DreamSys.c (93 methods), Entity.c (138), 7
  FileResource methods in GameApplicationFileResource.c, DreamSys.h's
  prototypes; `self` everywhere else.
- **Guards:** DreamSys.h `CLASS_DREAMSYS`, StageGrid.h `STAGE_GRID`.
- **`s` externs in headers:** 24 (StageMap.h:416-441, TitleMenu.h:147-170,
  Task.h:47/57, SceneNode.h:204-205 `sRotationZero`/`sSceneNodeScaleOne`,
  which are also typed `u8[0xC]` for `Ratio16[3]`). Move into the .c or
  rename `g`. Conversely 8 `g` symbols used by one unit only
  (`sCdStreamAudioMixSet`, `sCdFileNotFoundFmt`, `sFileTableRegistered`, ...).
- **Typos:** `DreamSys__InitMoodContributors`, `totalFlasbackUnlockScore` and
  siblings, DreamSys.h's @brief typos (flashabcks, appropiate, indicies,
  recieve, adquired, lank). `Test4*` (4 functions) for `TestFor*`.
- **Misleading names** (rename; each quoted against its code by a reviewer):
  - FileResource slot `setFlag`: the base sets bit 1, every subclass uses it
    as its "data arrived, build it" hook: `onLoaded`.
  - MoviePlayer `Stop` rewinds, `MarkStopped` requests a restart,
    `MarkPlaying` a start: `Rewind`, `MarkRestartPending`, `RequestStart`.
    `OnMdecFrameReady` fires per 16-pixel strip: `OnMdecStripDone`.
  - FrameClock `flag14`/`FRAMECLOCK_EVENT_FLAG14`/`SetFlag14`: stopped.
  - TaskCore `slotCounts`/`GetActiveSlotCount` hold cursors;
    `externalRecords` are slot positions, `registrationSlots` disabled
    markers; `TickColorFade`/`TickFadeColor` are fade in/out; `Tick` runs on
    confirm; `RefreshViewValue` finishes.
  - IntermediateBase `unk10`/`unk14` are the frameClock and lightRig (about
    20 casting sites in ObjMStyleActor.c); `onTag1Notify`/`onNotifyTag1`,
    `onState2`/`onState3` are the DrawSystem event and start/stop hooks, and
    Task.c uses literal 2/3 where `INTERMEDIATEBASE_STATE_START/STOP` exist.
  - `SelectCallback80/98`, `LOOK_CALLBACK_SLOT14C/150` (named for offsets).
  - `StageMap__GetUnk1CC`; NullDriver/`DATASOURCE_SPU` for a driver whose
    methods are all empty; `withSound` typed `void *` in TaskCore's functions
    and `s32` in its slots.
  - Code-suffixed method families: ObjM `EnterState4..A`,
    `CloseAndNotifyC/D`, `NotifyParentsCodeB`, TodActor `TickCallbackA/B/C`,
    `TickStaircaseCase0..3`, `SetupStyleSpawnParamsRandom/B`,
    `StyleEffect__ReleaseJitterSprites`, numbered `NoOp2..5`.

## Track 10 `sony-code`

- libcd_bios.c's 12 globals (366 references) have Sony's names in bios.o:
  CD_cbsync D_8006D5FC, CD_cbready ..600, CD_cbread ..604, CD_debug ..608,
  CD_status ..60C, CD_status1 ..610, CD_nopen ..614, CD_pos ..618, CD_mode
  ..61C, CD_com ..61D, CD_comstr ..620, CD_intstr ..6A0.
- `func_80038E44` (libspu_s_ih.c) is `_SpuInit`; `D_80090368` (libsnd_ssinit.c)
  is `_ss_MarkCallback` (0x800 bytes); the `_svm_cur` bytes (vmanager:57-75,
  15 `D_` externs) could be one struct view.
- SpuVm* prototypes disagree across the libsnd files (`SpuVmPBVoice`,
  `SpuVmKeyOff`, `SpuVmAlloc`; Get/SetSeqVol twice): one libsnd-internal
  header matching the definitions. `SeqPlay` declared `s32 (s16, s16)` at
  libsnd_play.c:14, defined `void (s16, s16, s16)`.
- libgs_gs_101.c and _124.c claim no header carries GsSetNearClip; libgs.h
  does. libcd_bios.c:548-557 cites a nonexistent `asm/psyq_15d04.s` and
  re-declares libetc.h's ResetCallback/VSync/CheckCallback.
- `#if 0` + INCLUDE_ASM bodies (libsnd_decre.c:34, libsnd_ut_ako.c:58,
  libcd_bios.c:178, 319) to the `#ifdef NON_MATCHING` form, which
  check-nonmatching.sh then compiles; unexplained `#if 1` wrappers around
  matched functions (libsnd_seqread.c:493, 826): delete.
- Derivation essays (about 75 lines): libsnd_vmanager.c:887-899, 69-72;
  libsnd_ssinit.c:50-60; libsnd_seqread.c:43-48; libsnd_decre.c:31-33;
  libcd_bios.c:1034; libsnd_ut_ako.c:27, 45-46 (stale).
- libsnd_vmanager.c holds game code (ServiceSoundCueSet): split it out.

## Track 10 debt passes (per area)

- **app/cd:** TaskCore `unk2C` (maxPackets), `unk34` (clear on deinit),
  `unk93` (clear colour), `unk96`; TaskCoreTarget `unk8` (initial slot),
  `unk24` (item lists); GameApplication `config->unk14`/`slot228`;
  `APPLICATION_LOOP_SLOT5C`; CdDriver's "part N" notes are off by one.
- **graphics:** `TmdModel__func_8001F37C`, `BasicClass__func_18350`,
  `D_8008E248`, `D_80090C18`; `TimImage flag48`; SceneNode link events 2/3/4
  as literals (Actor.h names 5-8); `New_TimBlockSrc(s32 name)` and
  `ModelData__ForwardScan*` take pointers as `s32`; the `+ 0x5C` at
  TmdRenderer.c:1194 is `offsetof(PolyDrawCtx, sxy) - sizeof(DVECTOR)`;
  stale `unk2A` in TimBlockSrc.h/TileMap.h/TileAtlas.h (now `loadState`);
  SceneNode.c's two mid-file banners and self-reference (merge leftovers);
  TmdRenderer.c opens with BasicClass/BMemPMgr helpers.
- **world:** DreamSys `func_59590`, `func_59598`, `func_5ba20` (a get/set of
  `unk_0x924`); DreamSys.h's `unk_0x*` fields (snake/hex spelling, no offset
  comments; `unknown_values_0x922` looks like padding); DreamAux's
  `sDreamAuxSlots2` alias; `TestForStageTransition` and
  `EnableTeleportsForKind` goto ladders over raw stage/mood numbers with no
  MATCHING line; `*(s32 *)((u8 *)out + 4)` at DayTaskStageMap.c:1096.
- **ui/sound:** class ids `0x10`/`0x20` and `kind == 2/5` at
  TitleMenuTaskObjF.c:589-611, 1284-1296 (PAD_/FRAMECLOCK_CLASS_ID exist),
  `setState(self, 5/0xA/0xB/0xF)` where TASKCORE_STATE_* exist;
  ScreenWidgets.c:103, 145 local `mask` is the channel set; GridCell `unk34`,
  CellPlacement `unk1`/`unk2C`; SsScore.h (39) and SvmData.h (32) `unkNN`
  fields are each described well enough to name.

## Track 12: what the documentation pass needs

- About 1134 prototypes in include/, 240 with an adjacent comment; no header
  uses `/**`, and DreamSys.h's and StageGrid.h's `/* @brief` lines are
  invisible to Doxygen. Header comment words outnumber code words 3 to 1,
  most of it good class documentation written as analysis.
- Style: `/** @file */`; a 5-15 line class block (what, parent, class id,
  methods' file, object size); `/** @brief @param @return */` per prototype;
  `/**< */` for fields and slots, the `/* +0xNN */` offset tags kept; slot
  tables point at the method (`@see`), not a second description. Macro
  field lists (`*_FIELDS`) need `MACRO_EXPANSION` in the Doxyfile.
- Process text to move out of headers (to the .c as one `MATCHING:` line,
  or to the report): register and ABI notes (`$a0`-`$v0`) in SceneNode.h,
  TaskCore.h, TextEntry.h, TextRow.h, TileAtlas.h, TileMap.h, Sprite.h,
  Task.h, ItemList.h, StreamTask.h; lwl/lwr and "retail reloads" in
  StageMap.h, TitleMenu.h, TaskObjF.h, TmdModel.h, Sprite.h, StyleEffect.h,
  Viewport.h, MoviePlayer.h, GraphRoom.h, LbdFile.h, common.h; GCC and splat
  notes in Entity.h:46-51, 102-107, 148-153; about 120 lines in gte.h; 32
  pointers at `tools/` commands and `docs/` files; "(no code)" jargon (8).
- Stale names inside header prose: StageMap.h's `buildRateEntries`
  (`loadChunksAround`), `ChunkSlotSpec::key`, `LbdFile::ownerKey`; Entity.h's
  merged-unit names ("(Entity, then Entity)"); IntermediateBase.h's
  `args->unk0..unkC`; NullDriver.h and Task.h name one file twice; CdDriver.h
  "that unit still spells them as literals"; BasicClass.h "all 59 method
  tables" (60).
- Unit-private headers (DayTaskStageMap.h, GameApplicationFileResource.h,
  DreamAux.h) fold into their .c files or become real class headers first,
  so the pass documents public API only.
- `types.h:4` `typedef char int8_t` is unsigned under `-funsigned-char`
  (unused; make it `signed char`); common.h's `MoodGraphPoint` belongs in
  StageGrid.h; StageGrid.h's `struct simplePair` is unused.
