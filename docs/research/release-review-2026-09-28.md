# Release review, 2026-09-28 (after round 102)

Six read-only reviewers, one per area (src/app+cd+main, src/graphics,
src/world, src/ui+sound+psyq, include A-L, include M-Z), each told the
image is byte-exact so only readability and consistency count. Every
finding below was checked against the tree by its reviewer (grep, cpp,
nm on the SDK objects). It is the starting list for phase 3's items
(FINISHING-PLAN §3, tracks 10 to 12), grouped by the item that fixes it.
Line numbers are as of commit b23bdf545 and drift; search for the name.

**Verdict.** The function bodies are in good shape: one-line `MATCHING:`
notes, Sony types under Sony's names, few raw casts. draw_system.c,
flat_light_obj.c, tim_image.c, pad.c, cd_stream.c, application.c, stage_grid.c,
tod_actor.c, dream_day.c, wbgm.c and screen_widgets.c's FadeBox code
are close to release quality. The debt is in declarations, a few
conventions, names the code has outgrown, and headers written as analysis
rather than API.

## Track 10 `prototypes`: one declaration per name

- **Allocator.** `BMemPMgrAlloc` is re-declared locally in about 20 units
  and in dream_sys.h:376, entity.h:308, dream_day.h:30, task.h:27.
  `BMemPMgrFree` returns `void` in entity.h:309 and
  data_source.h:16 but `void *` in bmem_pmgr.h:83 and task.h:28,
  so any unit including two of them fails with conflicting types. main.c:28
  declares `BMemPMgrInit`/`SetDefaultBMemPMgr` because bmem_pmgr.h omits them.
  bmem_pmgr.h:82 leaves Alloc unprototyped on purpose (arity); keep that as one
  `MATCHING:` line, and put a public prototype pair where bmem_pmgr.c does not
  see it.
- **SoundCueSet.** `InitSoundCueSet`, `FlushSoundCueSet`,
  `ServiceSoundCueSet` have three spellings each (dream_sys.c:72-73, 497;
  entity.c:298-299, entity.h:310-311; dream_scene.c:1782, 1893, 1903);
  the definitions take `(VabStreamObj *, SoundCueSet *)`. Declare once in
  sound_cue_set.h.
- **game_files.** No header. `GetStageMapChunkRecord`/`...XY` are declared
  `s32`/`void` in dream_scene.c:462-473 but return `FilePathRecord *`
  (game_files.c:317, 322); `ObjM__GetGridRecord` is declared void and "returns"
  through $v0. `PickSoundBank` returns `s32` for a path (game_files.c:203),
  `PickStageBgm` likewise (dream_scene.c:525). `RecPick{group,sub}` and
  dream_sys.h's `CinematicCall{bank,entry}` are one packed pair, repacked by
  hand at game_shell.c:80, 317. Add game_files.h.
- **Full-width SJIS helpers.** `DecodeFullWidthSjis` is `u8 *(u8 *, u8 *)`
  (screen_widgets) but `void (void *, void *)` at title_menu.c:316 and
  `char *(char *, char *)` at input_dialogs.c:41, 511;
  `FormatFullWidthNumber` re-declared at title_menu.c:193. One header.
- **Others without a header or disagreeing:** `BuildFileName`
  (input_dialogs.c:155, 629; vab_sound.c:185, with `const`);
  `GetSsSizeTableBuf` (`void *` vs `char *`); `GetSsTicksPerSecond` (wbgm.c:21
  only); `LockCd`/`UnlockCd` declared `s32` at game_shell.c:535,
  defined `void`; `SetActiveDataSourceDriverMode` `s32` in dream_day.h:57
  vs `void` definition; `GetSoundEffectDir(s32)` vs `(void)`;
  `IsStyleVariantEven` `bool` vs `s32`; `ReleaseBasicClassArray` in task.h:32
  `(void *, void *)` vs `(BasicClass **, s32)` at tmd_renderer.c:86 (plus three
  `void **` spellings); `GetActiveDataSourceMethods` local in 4 files;
  `GetSetBitField` duplicated (sprite.c:41, task.h:36 vs scene_node.h:216);
  `ApplyMatrixToLVArray` extern at viewport_draw.c:62 though scene_node.h is
  included; `sHitHeightGate` declared twice in scene_node.c. cd_driver.c
  re-declares its own functions two or three times (55/60/794/795, 58/463/907,
  ...): keep one forward block.
- **Sony names re-declared.** `ResetGraph` (task.h:40), `rand`, `strlen`,
  `strcat`, `strcpy`, `memset`, `memcpy`, `printf` (declared `void (const char *)`
  at title_menu.c:980 and wbgm.c:18 against stdio.h's variadic `int`).
  Use include/psyq's headers (verify bytes: strings.h's `strlen` is K&R).
- **BIOS and card calls with no declaration at all** (cpp-confirmed in
  title_menu.c): OpenEvent, TestEvent, CloseEvent, EnableEvent,
  DisableEvent, InitCARD, StartCARD, _bu_init, _card_info/_load/_clear, open,
  read, write, lseek, close, delete, format; `EnterCriticalSection` used before
  its `extern void` at :1065. The comments at :501, 647, 870, 1046 claim
  kernel.h declares them; it has no prototypes. Bring libapi.h from the SDK
  disc into include/psyq. **Wrong (round 103):** no disc ships a libapi.h;
  kernel.h does declare them, and plain `grep` missed it because the file's
  SJIS bytes make grep treat it as binary (`grep -a` finds them).
- **Duplicate types.** `RotationRatio(s)` (dream_sys.h:172) is `Ratio16`;
  `SubObjE` (dream_day.h:18) is a DrawSystem view; `SlotEntry`/`SrcDesc`
  (task.c:658, 670) are one record; `CdStreamFile` (cd_stream.h:44) is
  `CdlFILE`; a signed 3-byte colour is defined six times (`BgLayerRgb`,
  `BoxFillRgb`, `FlatLightColor`, `LightRigRgb`, `ColorRgb`, `ViewportRgb`):
  one type. `FIX12_SHIFT` and `CD_SECTOR_SIZE` redefined at
  graphics_resources.c:63-64. `ABS_fa50` (tmd_model.c:53) open-coded twice more.
- **Headers not self-contained:** six M-Z headers use `GsCOORDINATE2`,
  `SVECTOR`, `MATRIX`, `RECT` without including Sony's header.

## Track 10 `conventions`

- **Getters:** 13 `Get_vtable_X` against 46 `GetXMethods` (BasicClass,
  DreamSys, Entity, CdStream, DrawSystem, FlatLightObj, FrameClock,
  IntermediateBase, Pad, StreamTask, TaskCore, TmdModel, WBgm). Rename; add
  the form to §3's conventions.
- **Receiver:** `this` in dream_sys.c (93 methods), entity.c (138), 7
  FileResource methods in game_shell.c, dream_sys.h's
  prototypes; `self` everywhere else.
- **Guards:** dream_sys.h `CLASS_DREAMSYS`, stage_grid.h `STAGE_GRID`.
- **`s` externs in headers:** 24 (stage_map.h:416-441, title_menu.h:147-170,
  task.h:47/57, scene_node.h:204-205 `sRotationZero`/`sSceneNodeScaleOne`,
  which are also typed `u8[0xC]` for `Ratio16[3]`). Move into the .c or
  rename `g`. Conversely 8 `g` symbols used by one unit only
  (`sCdStreamAudioMixSet`, `sCdFileNotFoundFmt`, `sFileTableRegistered`, ...).
- **Typos:** `DreamSys__InitMoodContributors`, `totalFlasbackUnlockScore` and
  siblings, dream_sys.h's @brief typos (flashabcks, appropiate, indicies,
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
    20 casting sites in dream_scene.c); `onTag1Notify`/`onNotifyTag1`,
    `onState2`/`onState3` are the DrawSystem event and start/stop hooks, and
    task.c uses literal 2/3 where `INTERMEDIATEBASE_STATE_START/STOP` exist.
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
- `func_80038E44` (libspu_s_ih.c) is `_SpuInit`; `_ss_MarkCallback` (libsnd_ssinit.c)
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
- **graphics:** `TmdModel__NoOpSlot4C`, `BasicClass__NoOpSlot34`,
  `sSortLightOff`, `sSortNdiv`; `TimImage flag48`; SceneNode link events 2/3/4
  as literals (actor.h names 5-8); `New_TimBlockSrc(s32 name)` and
  `ModelData__ForwardScan*` take pointers as `s32`; the `+ 0x5C` at
  tmd_renderer.c:1194 is `offsetof(PolyDrawCtx, sxy) - sizeof(DVECTOR)`;
  stale `unk2A` in tim_block_src.h/tile_map.h/tile_atlas.h (now `loadState`);
  scene_node.c's two mid-file banners and self-reference (merge leftovers);
  tmd_renderer.c opens with BasicClass/BMemPMgr helpers.
- **world:** DreamSys `func_59590`, `func_59598`, `func_5ba20` (a get/set of
  `unk_0x924`); dream_sys.h's `unk_0x*` fields (snake/hex spelling, no offset
  comments; `unknown_values_0x922` looks like padding); dream_aux's
  `sDreamAuxSlots2` alias; `TestForStageTransition` and
  `EnableTeleportsForKind` goto ladders over raw stage/mood numbers with no
  MATCHING line; `*(s32 *)((u8 *)out + 4)` at dream_day.c:1096.
- **ui/sound:** class ids `0x10`/`0x20` and `kind == 2/5` at
  title_menu.c:589-611, 1284-1296 (PAD_/FRAMECLOCK_CLASS_ID exist),
  `setState(self, 5/0xA/0xB/0xF)` where TASKCORE_STATE_* exist;
  screen_widgets.c:103, 145 local `mask` is the channel set; GridCell `unk34`,
  CellPlacement `unk1`/`unk2C`; ss_score.h (39) and svm_data.h (32) `unkNN`
  fields are each described well enough to name.

### Left after round 103 (the debt items carry these too)

Round 103 did `prototypes`, `conventions` and `sony-code`; what they left,
by the debt item that owns it. A duplicate type is a MERGE, not a rename:
`renametype.py` refuses a target that exists, and the field names differ,
so merge by hand, one commit per type, the accessors from the compiler.

- **app/cd:** `SlotEntry`/`SrcDesc` (task.c) into one record;
  `CdStreamFile` (cd_stream.h) into `CdlFILE`; TaskCore's `withSound` is
  `void *` in its functions and `s32` in its slots; `SetTickCallbacks`'
  header prototype still names its parameters `arg1`/`arg2`.
- **graphics:** the six signed 3-byte colour types (`BgLayerRgb`,
  `BoxFillRgb`, `FlatLightColor`, `LightRigRgb`, `ColorRgb`,
  `ViewportRgb`) into one, in a graphics header (box_fill.h is ui's, and
  this item may edit it for that).
- **world:** `RotationRatio(s)` (dream_sys.h) into `Ratio16`; `SubObjE`
  (dream_day.h) into its DrawSystem type; dream_sys.h's
  `CinematicCall` and game_files.c's `RecPick` into one packed pair (this
  item may edit game_files.c/h for it; `GetSpecialDayOrEventRecord` stays
  unprototyped, see its MATCHING line); IntermediateBase's `frameClock` /
  `lightRig` are still `BasicClass *`, so about 20 casts in
  dream_scene.c remain.
- **ui/sound/psyq:** src/psyq/libsnd_ssinit_libapi_counter.c defines the
  root counters (`SetRCnt`, `GetRCnt`, `StartRCnt`, `StopRCnt`,
  `ResetRCnt`) with types that conflict with kernel.h, so it cannot include
  kernel.h for `Enter/ExitCriticalSection`; align them if the bytes allow.
  bios.o's own names for libcd_bios's statics (`Result`, `Alarm`) apply to
  groups of three words each, and so stay as `D_` placeholders unless a
  struct or array view covers the whole group. CD_cw's report title says
  282 words and length-exact, but its preserved body builds to 284: make
  the title say what the body builds to.

### After round 104

Round 104 did the four debt passes; each item's list above is done or
measured wrong (the per-finding verdicts are in PROGRESS.md, round 104).
Still open, none a track 10 item: what IsDaySpecial's `% 12` counts;
whether TodActor's `TodSetBuffer` and graphics_resources' `SubBlockTable` are
one type; the write-only `unk` fields readability.py still lists (nothing
reads them, so nothing names them).

## Track 12: what the documentation pass needs

- About 1134 prototypes in include/, 240 with an adjacent comment; no header
  uses `/**`, and dream_sys.h's and stage_grid.h's `/* @brief` lines are
  invisible to Doxygen. Header comment words outnumber code words 3 to 1,
  most of it good class documentation written as analysis.
- Style: `/** @file */`; a 5-15 line class block (what, parent, class id,
  methods' file, object size); `/** @brief @param @return */` per prototype;
  `/**< */` for fields and slots, the `/* +0xNN */` offset tags kept; slot
  tables point at the method (`@see`), not a second description. Macro
  field lists (`*_FIELDS`) need `MACRO_EXPANSION` in the Doxyfile.
- Process text to move out of headers (to the .c as one `MATCHING:` line,
  or to the report): register and ABI notes (`$a0`-`$v0`) in scene_node.h,
  task_core.h, text_entry.h, text_row.h, tile_atlas.h, tile_map.h, sprite.h,
  task.h, item_list.h, stream_task.h; lwl/lwr and "retail reloads" in
  stage_map.h, title_menu.h, task_objf.h, tmd_model.h, sprite.h, style_effect.h,
  viewport.h, movie_player.h, graph_room.h, lbd_file.h, common.h; GCC and splat
  notes in entity.h:46-51, 102-107, 148-153; about 120 lines in gte.h; 32
  pointers at `tools/` commands and `docs/` files; "(no code)" jargon (8).
- Stale names inside header prose: stage_map.h's `buildRateEntries`
  (`loadChunksAround`), `ChunkSlotSpec::key`, `LbdFile::ownerKey`; entity.h's
  merged-unit names ("(Entity, then Entity)"); intermediate_base.h's
  `args->unk0..unkC`; null_driver.h and task.h name one file twice; cd_driver.h
  "that unit still spells them as literals"; basic_class.h "all 59 method
  tables" (60).
- Unit-private headers (dream_day.h, data_source.h,
  dream_aux.h) fold into their .c files or become real class headers first,
  so the pass documents public API only.
- `types.h:4` `typedef char int8_t` is unsigned under `-funsigned-char`
  (unused; make it `signed char`); common.h's `MoodGraphPoint` belongs in
  stage_grid.h; stage_grid.h's `struct simplePair` is unused.
