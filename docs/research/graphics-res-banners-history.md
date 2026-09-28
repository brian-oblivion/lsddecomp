# area-graphics-res: the header and file banners before track 12

Track 12 (FINISHING-PLAN §3) turned the headers of `src/graphics/`'s
resource classes into Doxygen API documentation and split the long `.c`
banners into class and function docs. The banners below are the text those
comments held before, kept verbatim because they mix class documentation,
which now lives in the Doxygen blocks, with the project's process record:
the `classtable.py` / `typeviews.py` commands and slot counts each class was
measured with, register and instruction notes, and pointers at reports.
Nothing here is current documentation; read the header for that. A line
that belongs to one function's derivation went to that function's match
report instead, under a "History" heading.

Each section is the file's comment as it stood at commit 8f08d326f
(`git show 8f08d326f:<path>` prints the whole file).

## `include/bg_layer.h`

```c
/*
 * BgLayer -- class id 0x54, method table gBgLayerMethods: a SceneNode subclass
 * (its ctor chains to GetSceneNodeMethods()->ctor first, so the id tree
 * 0x4 -> 0x54 is the ctor chain) whose own fields, +0x044..+0x067, are
 * exactly libgs's GsBG (LIBGS.H: attribute, x, y, w, h, scrollx, scrolly,
 * r, g, b, map, mx, my, scalex, scaley, rotate). Methods in
 * src/graphics/graphics_resources.c; no class derives from it.
 *
 * Its GsBG is what makes it a background layer: Viewport__DrawNode
 * (src/graphics/viewport_draw.c) passes a class-0x54 node's +0x044 to GsSortBg, and
 * BgLayer__Reset lays that GsBG over a map source's GsMAP. Its one outside
 * user is TaskCore (src/app/task.c): TaskCore__TaskCore builds one over its
 * TileMap (New_BgLayer(tileMap, 1)) into TaskCore::bgLayer, OnInit attaches
 * it to the LightRig (IntermediateBase::lightRig) and sets its colour, OnDeinit detaches it,
 * Finalize releases it, and the colour fades (TaskCore__TickFadeIn,
 * TaskCore__TickFadeOut) call setColor every frame.
 *
 * SLOTS (`classtable.py gBgLayerMethods --vs gSceneNodeMethods`, 47 against 45):
 *  - +0x008 ctor, BgLayer__BgLayer(self, src, mode): SceneNode's, this
 *    table, then reset(self, src, mode). Returns nothing; the slot keeps
 *    SceneNode's `void *` ctor type, New_BgLayer ignoring the value (as
 *    include/grid_cell.h);
 *  - +0x040 reset, BgLayer__Reset(self, src, mode): lays the GsBG over
 *    src's GsMAP. Its parameter list differs from the inherited slot's
 *    (self only), so the slot keeps SceneNode's type and the ctor, its one
 *    caller, casts to BgLayerResetFn;
 *  - +0x044 updateRotation (BgLayer__UpdateRotation) and +0x048 updateScale
 *    (BgLayer__UpdateScale): set or add, from the same Ratio16
 *    {num, den} ratio table SceneNode's reads; the GsBG's rotate from entry
 *    [2] (the z angle, the only one a 2D layer has), its scalex/scaley from
 *    entries [0] and [1];
 *  - two own slots: +0x0B8 setColor (BgLayer__SetColor: the GsBG's r, g, b,
 *    when `enable`; every TaskCore caller passes 1 and a u8[3] buffer) and
 *    +0x0BC slotBC (BgLayer__NoOp, empty; no known caller).
 *
 * FIELDS: the GsBG, +0x044..+0x067; the object is 0x68 bytes (New_BgLayer).
 * `bgAttribute` because SceneNode's +0x010 (GsDOBJ2.attribute) already has
 * the name. r, g, b are one ColorRgb (include/draw_system.h): retail copies
 * them lb/lb/lb, sb/sb/sb (BgLayer__SetColor, BgLayer__Reset), a whole-struct
 * copy.
 *
 * The map source is a TileMap (gTileMapMethods, include/tile_map.h),
 * whose GsMAP starts at +0x02C. Only its tag is named here, as
 * include/trigger_world.h does for its descriptor.
 */
```

## `include/flat_light_obj.h`

```c
/*
 * FlatLightObj -- one Psy-Q flat light, class id 0x6, method table
 * gFlatLightObjMethods, a direct BasicClass subclass (`tools/classtable.py
 * gFlatLightObjMethods --vs gBasicClassMethods`: overrides the ctor, adds three
 * slots). No class derives from it. Methods in src/graphics/flat_light_obj.c, which holds
 * the whole class: allocator, ctor, the three own slots and the getter.
 *
 * "FlatLight" is Sony's own name (LIBGS.H's GsF_LIGHT and GsSetFlatLight),
 * not a guess: the object keeps a light id at +0x00C and a GsF_LIGHT at
 * +0x010, and setColor/setDirection update the GsF_LIGHT and hand it by
 * address to GsSetFlatLight(lightId, &light). FlatLightParams is GsF_LIGHT's
 * layout (`int vx,vy,vz; unsigned char r,g,b;`, same offsets) with r,g,b
 * grouped as a ColorRgb (include/draw_system.h), which setColor's whole-struct
 * copy needs;
 * src/graphics/flat_light_obj.c takes GsSetFlatLight from <libgs.h> and casts to GsF_LIGHT *.
 *
 * Who holds one: LightRig__LightRig (src/graphics/sprite.c, include/light_rig.h)
 * makes three with New_FlatLightObj(0), (1), (2), keeps them in
 * LightRig::lights and adds each as a child; LightRig__Finalize releases
 * them. The one caller of setColor (+0x044) and setDirection (+0x048) is
 * StageMap__SetChildParams (src/world/dream_day.c), through LightRig's
 * getLight, with update = 1 and sources stepping 3 bytes (an r,g,b) and 6
 * bytes (an s16 vx,vy,vz) per light. That call site casts getLight's
 * BasicClass * to FlatLightObj * and its s32 sources to the slots' types.
 *
 * The object is 0x20 bytes (New_FlatLightObj's allocation).
 */
```

## `include/link_resource.h`

```c
/*
 * LinkResource -- a FileResource data source (class id 0xD03, method table
 * gLinkResourceMethods) over one loaded TMD file: it builds one TmdModel
 * (include/tmd_model.h) per object of the TMD and hands them out by index.
 * Methods in src/graphics/graphics_resources.c; no subclasses.
 *
 * What its own methods do: map the file's TMD
 * (LinkResource__MapModel: GsMapModelingData(&file->flags)) and build and
 * return the TmdModel objects the callers LINK -- tod_actor.c's TOD
 * model-id packet passes getModel's result to SceneNode__LinkModel, and
 * dream_day.c links the TmdObject behind it with GsLinkObject4.
 *
 * Holders: ModelData's `linkResource` (ModelData__BuildResources, over the
 * TMD sub-block of a MOM file), PlacementGrid's `linkResource`
 * (StageMap__PopulateSlotCells, the element's models), DayTask's
 * `dreamerTmd` ("ETC\DREAMER.TMD") and DreamSys's ctor argument
 * (GameApplication__GameApplication, "ETC\DREAME5.TMD": DreamSys__DreamSys adds
 * getModel(0) as its child).
 *
 * Its parent ctor is the active driver's (GetActiveDataSourceMethods()->ctor,
 * chosen at run time; see include/model_data.h), so the fields below assume
 * FileResource's own 0x2C-byte layout. The object is 0x30 bytes
 * (New_LinkResource).
 *
 * Inherited slots keep FileResource's names and types; this table's occupants
 * differ from them in two places, and the callers cast:
 *   +0x008 ctor: LinkResource__LinkResource returns self, or NULL when the
 *          buffer it adopted fails to build (New_LinkResource tests it,
 *          through graphics_resources.c's unprototyped UnprototypedCtorTable view).
 *   +0x064 onRequestDone: LinkResource__BuildModels(self), s32: 1 when an
 *          allocation fails, else 0 after the active driver's onRequestDone.
 *   +0x078 processBuffer (NULL in FileResource): LinkResource__MapModel(self).
 *
 * The ctor's descriptor is ResourceSource (include/file_resource.h): a buffer
 * to adopt, else a file name to request.
 */
```

## `include/model_data.h`

```c
/*
 * ModelData -- a FileResource data source (class id 0x5F03, method table
 * gModelDataMethods) that splits one loaded file into a LinkResource (gLinkResourceMethods,
 * an array of TMD models) and a TodSet (gTodSetMethods, an array of TOD
 * animations), and forwards TOD packet scanning to the TodSet. Methods in
 * src/graphics/graphics_resources.c; one subclass, TriggerWorld (gTriggerWorldMethods, 0x15F03), whose
 * ctor calls this class's first (TriggerWorld__TriggerWorld:
 * GetModelDataMethods()->ctor(self, arg, 0)).
 *
 * What its own methods do: build a
 * TMD model source and a TOD set over one buffer (ModelData__BuildResources:
 * New_LinkResource over the sub-block at the buffer's third word, New_TodSet
 * over the buffer past +0x0C) and forward TOD packet decoding to the set
 * (+0x080/+0x084); its outside users, TodActor (TodActor.modelData,
 * `tmd`/`tods`) and dream_aux.c (InitDreamAux's MOM files), hold it as the
 * model and animation data of an actor.
 *
 * PARENT BY CTOR CHAIN, NOT BY ID. The id 0x5F03 puts it under TimBlockSrc
 * (0xF03), but ModelData__ModelData's first call is
 * GetActiveDataSourceMethods()->ctor, as TimBlockSrc__TimBlockSrc's is: it is
 * TimBlockSrc's sibling under FileResource, and carries none of TimBlockSrc's
 * layout. That parent ctor is chosen at RUN TIME: the CD driver's
 * (CdDriver__CdDriver, which chains to FileResource__FileResource; its
 * object is 0x2C bytes, New_CdDriver) or, while sActiveDataSource is
 * DATASOURCE_NULL, the null driver's (NullDriver__NullDriver, an empty body).
 * The fields below assume FileResource's own 0x2C-byte layout, which is the
 * CD driver's whole object: ModelData's own fields start at +0x02C.
 *
 * The ctor returns self or NULL (New_ModelData tests it), but
 * FILERESOURCE_SLOTS declares +0x008 returning void, as FileResource's own ctor
 * does; the allocators reach the ctor through graphics_resources.c's unprototyped
 * UnprototypedCtorTable view instead.
 *
 * The ctor's descriptor is include/file_resource.h's ResourceSource ({buffer
 * to adopt, file name to request}).
 */
```

## `include/movie_player.h`

```c
/*
 * MoviePlayer -- CD-streamed, MDEC-decoded FMV playback (class id 0x70,
 * method table gMoviePlayerMethods, a direct BasicClass subclass). Methods in
 * src/graphics/graphics_resources.c; the object is 0x6C bytes (New_MoviePlayer). The one
 * holder is StreamTask (`player`, include/stream_task.h, src/app/task.c),
 * which builds it with New_MoviePlayer(GetDefaultMovieFrame(), 0, 0)
 * and calls +0x06C setAutoPlay, +0x040 play, +0x048 advance, +0x04C abort
 * and +0x004 release.
 *
 * The pipeline, measured from the methods:
 *   - the ctor builds a CdStream (New_CdStream(cdSpeed, MOVIE_FPS, 0)) at `stream`
 *     and hands it the 0x12000-byte `ring` (CdStream setRing);
 *   - pullFrame takes the next frame's sectors from the stream
 *     (getNextFrame), flips `frameIndex` and VLC-decodes them into
 *     frames[frameIndex] (DecDCTvlc), then gives the sectors back (freeRing);
 *   - decodeFrame, once a frame is pulled (`haveFrame`), waits for the
 *     previous frame's last strip (`frameDone`, WaitFrameReady), feeds
 *     frames[frameIndex] to the MDEC (DecDCTin) and asks for the first strip
 *     (DecDCTout into `strip`), then pulls the next frame;
 *   - the MDEC's output callback (OnMdecStripDone) runs the ACTIVE player's
 *     drawStrip: upload `strip` at `stripRect` (DrawSystem loadImage), step
 *     stripRect.x by its 16-pixel width, and either request the next strip
 *     or, past the frame's right edge, rewind and set `frameDone`.
 * One movie plays at a time: `sActiveMoviePlayer` is the player play made
 * active; rewind, advance, abort and decodeFrame do nothing for any other
 * object, and pollActive clears it when the movie is over.
 *
 * NO FIELDS/SLOTS MACROS: no class lies below 0x70 (`typeviews.py --tree`).
 */
```

## `include/tile_atlas.h`

```c
/*
 * TileAtlas -- a FileResource data source (class id 0x303, method table
 * gTileAtlasMethods) that builds, instead of loading, an array of 300 libgs
 * GsCELLs: the cell atlas a TileMap's GsMAP indexes (include/tile_map.h).
 * Methods in src/graphics/graphics_resources.c. No classes derive from it (`typeviews.py
 * --tree`), so there are no FIELDS/SLOTS macros.
 *
 * The ctor chain agrees with the id: TileAtlas__TileAtlas's first call is
 * GetActiveDataSourceMethods()->ctor, and finalize forwards to the active
 * driver's, as TileMap, TimImage and TimBlockSrc do.
 *
 * The name is the cells': TileAtlas__BuildCells allocates 300 libgs
 * GsCELLs (8 bytes each) and fills them as 16 x 16-texel cells over VRAM
 * from x 0x280, u,v stepping by 16, a new texture page every 64 x;
 * TileMap__BuildMap takes `cells` as its GsMAP's base and lays out a 20 x 15
 * index table 0..299 over it.
 *
 * How it is used, at the one New_TileAtlas call site (TaskCore__TaskCore,
 * src/app/task.c): New_TileAtlas(0), then New_TileMap(0, atlas), then
 * New_BgLayer(tileMap, 1); TaskCore__Finalize releases the three (+0x004).
 *
 * SLOTS (`classtable.py gTileAtlasMethods --vs gFileResourceMethods`, 30 against 30; the
 * words from +0x07C on are sDataSourceClientGetters, not this table):
 *  - +0x008 ctor, TileAtlas__TileAtlas(self, arg1): the active driver's
 *    ctor, this table, unk34 = 0, loaded = 0; with arg1 == 0,
 *    defaultCells = 1, loadState = 0 and onRequestDone (+0x064). What a nonzero arg1
 *    means is not shown: the one caller passes 0;
 *  - +0x00C finalize, TileAtlas__Finalize: frees unk34 and cells, then the
 *    active driver's finalize;
 *  - +0x058 loadFile is NULL in this table (a TileAtlas loads no file);
 *  - +0x064 onRequestDone, TileAtlas__Load: unless loadState is set, +0x078 and
 *    loaded = 1. It calls +0x078 with NO argument ($a0 is never set up,
 *    as in TileMap__Load), through TileAtlasBuildCellsFn;
 *  - +0x078 is FileResource's `processBuffer` (NULL there); this table's
 *    occupant is TileAtlas__BuildCells. No own slots past it.
 *
 * FIELDS: the cell array, two u16 flags and a word only Finalize frees;
 * the object is 0x38 bytes (New_TileAtlas).
 *
 * GsCELL is <libgs.h>'s, so an includer takes Sony's headers first
 * (`common.h`, <libgte.h>, <libgpu.h>, <libgs.h>).
 */
```

## `include/tile_map.h`

```c
/*
 * TileMap -- a FileResource data source (class id 0x203, method table
 * gTileMapMethods) whose own fields, +0x02C..+0x03B, are exactly libgs's GsMAP
 * (LIBGS.H: cellw, cellh, ncellw, ncellh, base, index). Methods in
 * src/graphics/graphics_resources.c. No classes derive from it (`typeviews.py --tree`), so
 * there are no FIELDS/SLOTS macros.
 *
 * The ctor chain agrees with the id: TileMap__TileMap's first call is
 * GetActiveDataSourceMethods()->ctor, and finalize forwards to the active
 * driver's, as TimImage and TimBlockSrc do.
 *
 * The name is the GsMAP's: TileMap__BuildMap fills cellw/cellh = 16,
 * ncellw = 20, ncellh = 15 (a 320 x 240 screen of 16 x 16 cells),
 * allocates the ncellw * ncellh u16 index table and fills it 0..n-1, and
 * takes `base` from its atlas's cells; BgLayer__Reset
 * (include/bg_layer.h) points a GsBG's map at &map and sizes the layer from
 * cellw * ncellw by cellh * ncellh.
 *
 * How it is used, at the one New_TileMap call site (TaskCore__TaskCore,
 * src/app/task.c): New_TileAtlas(0), then New_TileMap(0, atlas), then
 * New_BgLayer(tileMap, 1); TaskCore__Finalize releases the three (+0x004).
 * The atlas is a TileAtlas (gTileAtlasMethods, include/tile_atlas.h, by tag here):
 * its `cells` is the 300-GsCELL array BuildMap copies into map.base.
 *
 * SLOTS (`classtable.py gTileMapMethods --vs gFileResourceMethods`, 30 against 30):
 *  - +0x008 ctor, TileMap__TileMap(self, arg1, atlas): the active driver's
 *    ctor, this table, atlas at +0x03C, loaded = 0; with arg1 == 0,
 *    defaultGrid = 1, loadState = 0 and onRequestDone (+0x064). What a nonzero arg1
 *    means is not shown: the one caller passes 0;
 *  - +0x00C finalize, TileMap__Finalize: frees map.index, then the active
 *    driver's finalize;
 *  - +0x064 onRequestDone, TileMap__Load: unless loadState is set, +0x078 and
 *    loaded = 1. It calls +0x078 with NO argument ($a0 is never set up;
 *    TileMap__Load's report), through TileMapBuildMapFn;
 *  - +0x078 is FileResource's `processBuffer` (NULL there); this table's
 *    occupant is TileMap__BuildMap. No own slots past it.
 *
 * FIELDS: the GsMAP at +0x02C, then the atlas, then two u16 flags; the
 * object is 0x44 bytes (New_TileMap).
 *
 * GsMAP is <libgs.h>'s, so an includer takes Sony's headers first
 * (`common.h`, <libgte.h>, <libgpu.h>, <libgs.h>).
 */
```

## `include/tim_array_src.h`

```c
/*
 * TimArraySrc -- a FileResource data source (class id 0xC03, method table
 * gTimArraySrcMethods) whose buffer holds one block of TIM images -- a count, then
 * that many byte offsets from the block's start -- and which turns it into
 * an array of TimImage objects (include/tim_image.h). Methods in
 * src/graphics/graphics_resources.c. No classes derive from it (`typeviews.py --tree`), so
 * there are no FIELDS/SLOTS macros.
 *
 * The ctor chain agrees with the id: TimArraySrc__TimArraySrc's first call
 * is GetActiveDataSourceMethods()->ctor, and finalize forwards to the
 * active driver's, as TimBlockSrc, TimImage and TileAtlas do.
 *
 * BuildImages makes one New_TimImage(NULL) per offset, each adopting its
 * TIM in place.
 *
 * How it is used, at the one New_TimArraySrc call site
 * (TimBlockSrc__AdvanceLoadState, include/tim_block_src.h): New_TimArraySrc(0)
 * per block, then the block's sector buffer as `buffer` (size 0, so the
 * TimArraySrc never owns it), `clutBase` = the address of the TimBlockSrc's
 * four CLUT fade ramps, then onRequestDone (+0x064, BuildImages) and +0x078
 * (UploadImages); the TimBlockSrc keeps it in `blocks` and releases it.
 *
 * SLOTS (`classtable.py gTimArraySrcMethods --vs gFileResourceMethods`, 30 against 30; the
 * words from +0x07C on are sDataSourceClientGetters, not this table):
 *  - +0x008 ctor, TimArraySrc__TimArraySrc(self, name): the active
 *    driver's ctor, this table, count/images/ready cleared, and
 *    requestLoadFile(name) when name is not NULL (the one caller passes 0);
 *  - +0x00C finalize, TimArraySrc__Finalize: ReleaseBasicClassArray the
 *    images, free the array, then the active driver's finalize;
 *  - +0x058 loadFile is NULL in this table;
 *  - +0x064 onRequestDone, TimArraySrc__BuildImages (named for what it does; the
 *    driver runs onRequestDone when a read completes, and TimBlockSrc calls it
 *    directly);
 *  - +0x078 is FileResource's `processBuffer` (NULL there); this table's
 *    occupant is TimArraySrc__UploadImages, called through
 *    TimArraySrcUploadFn (no code). No own slots past it.
 *
 * FIELDS: the object is 0x3C bytes (New_TimArraySrc).
 */
```

## `include/tim_block_src.h`

```c
/*
 * TimBlockSrc -- a FileResource data source (class id 0xF03, method table
 * gTimBlockSrcMethods) that loads a file of TIM blocks one sector-buffer at a time
 * and fades up to four 256-colour CLUT rows. Methods in src/graphics/graphics_resources.c.
 *
 * Loading (onRequestDone, +0x064, is TimBlockSrc__AdvanceLoadState: the driver
 * runs it when a read completes). The ctor reads the file's first sector;
 * its first 0x24 bytes are a header -- a block count, the blocks' file
 * offsets from +0x04, their sizes from +0x14 (FindMaxTimBlockSize) -- copied
 * into `buffer`. Each block is then read into `sector` and handed to a new
 * TimArraySrc (gTimArraySrcMethods, include/tim_array_src.h), whose clutBase is
 * `entries`, into `blocks`.
 *
 * Fading. `entries[i]` is CLUT row i's fade ramp: `mask` (1 << shift) rows
 * from VRAM y 0x1E0 + i * mask, the first the CLUT itself and the rest
 * FadeClutRow's steps toward `color`.
 *
 * NO FIELDS/SLOTS MACROS, although `typeviews.py --tree` puts four classes
 * below 0xF03 (Tod 0x4F03, TodSet 0x14F03, ModelData 0x5F03, TriggerWorld
 * 0x15F03). Their objects are 0x2C, 0x2C, 0x38 and 0x3C bytes (New_Tod,
 * New_TodSet, New_ModelData, New_TriggerWorld) against this class's 0x84
 * (New_TimBlockSrc), their constructors chain to the active driver's, not
 * to TimBlockSrc__TimBlockSrc, and their +0x07C/+0x080 occupants have other
 * signatures (ScanTodPackets returns a u8 from four arguments where
 * TimBlockSrc__FadeAllEntries takes two and returns nothing). None of them
 * carries a byte of this class's layout, so they expand FILERESOURCE's macros
 * directly.
 */
```

## `include/tim_image.h`

```c
/*
 * TimImage -- a FileResource data source (class id 0x103, method table
 * gTimImageMethods) whose buffer holds one TIM image. Methods in
 * src/graphics/tim_image.c. No classes derive from it (`typeviews.py --tree`), so
 * there are no FIELDS/SLOTS macros.
 *
 * The ctor chain agrees with the id: TimImage__TimImage's first call is
 * GetActiveDataSourceMethods()->ctor, and finalize forwards to the active
 * driver's, as TimBlockSrc and LbdFile do.
 *
 * What its own methods do: getTimInfo (+0x09C, TimImage__GetTimInfo)
 * describes the TIM in `buffer` (past its id word) into a GsIMAGE with
 * Sony's GsGetTimInfo; TimImage__Upload, at +0x078, describes it into `tim`
 * and uploads the pixel block and, when pmode bit 3 says there is one, the
 * CLUT through the draw singleton's loadImage (include/draw_system.h).
 *
 * How it is used, at every New_TimImage call site: New_TimImage(path) with
 * a ".TIM" path (the ctor requests the file), then +0x078 (upload), then
 * usually freeBuffer (+0x05C) or release (+0x004) once the sprites made
 * from it hold what they need. A Sprite's `texture` is a TimImage: its
 * reset keeps &texture->tim (include/sprite.h). TimArraySrc (gTimArraySrcMethods,
 * src/graphics/graphics_resources.c) makes them with New_TimImage(NULL), points `buffer`
 * into its own block and sets `clutBase`.
 *
 * +0x078 is FileResource's `processBuffer` (NULL there); this table's occupant
 * is TimImage__Upload, called through TimImageUploadFn (no code).
 *
 * `tim` is <libgs.h>'s GsIMAGE.
 */
```

## `include/tmd_model.h`

```c
/*
 * TmdModel -- one object of a TMD model file (class id 0x9, method table
 * gTmdModelMethods): a BasicClass subclass with no subclasses of its own.
 * Methods in src/graphics/tmd_model.c.
 *
 * The object is built on ONE entry of a TMD's object table (New_TmdModel's
 * argument; LinkResource__BuildModels, graphics_resources.c, builds one per entry of a
 * loaded TMD). `data` is that entry minus 0xC, i.e. the file header when the
 * entry is the first one; GetObject indexes the table from there and
 * MapModelingData hands `&data->flags` to GsMapModelingData. SceneNode's
 * `model` (+0x020) holds one: SceneNode__LinkModel links `object` into its
 * GsDOBJ2 (GsLinkObject4(data->objects, ...), GsDOBJ2.tmd = object).
 *
 * Besides its table the class has non-virtual methods SceneNode's collision
 * code calls directly: a bounding box over the object's vertices
 * (ComputeBounds, GetHull), a shared bounds buffer holding
 * sTmdModelBoundsCount boxes (always 1, set by the ctor), and a segment cast
 * against every face (RaycastFaces, walking primitives with NextPrimitive).
 *
 * The object is 0x24 bytes (New_TmdModel's allocation). Slots +0x040..+0x04C
 * are named for their occupants; no caller of them has been found in C.
 */
```

## `include/tmd_renderer.h`

```c
/*
 * The TMD renderer, src/graphics/tmd_renderer.c: the per-face projection,
 * lighting and subdivision a GsDOBJ2's TMD goes through into an ordering
 * table. The BasicClass methods and the pool allocator's busy flag at the
 * head of that file are declared by include/basic_class.h and
 * include/bmem_pmgr.h.
 */
```

## `include/tod.h`

```c
/*
 * Tod -- a FileResource data source (class id 0x4F03, method table gTodMethods)
 * over one TOD animation's packet stream. Methods in src/graphics/graphics_resources.c; one
 * subclass, TodSet (gTodSetMethods, 0x14F03), whose ctor calls this class's
 * first (TodSet__TodSet: GetTodMethods()->ctor(self, arg)) and whose
 * TodSet__BuildTods makes one Tod per sub-block of its buffer (New_Tod).
 *
 * What its own methods do: walk
 * the buffer's packet words (ScanTodPackets, from buffer +8 with the u16
 * packet count at +2 before it) decoding each into a low byte, the nibbles
 * at bits 16 and 20 and a top-byte length in words (DecodeTodPacketWord);
 * ModelData forwards its TOD packet scans to the TodSet it holds
 * (ModelData__ForwardScanPackets: todSet's +0x078), and TodActor plays
 * TODs through that ModelData.
 *
 * PARENT BY CTOR CHAIN, NOT BY ID. The id 0x4F03 puts it under TimBlockSrc
 * (0xF03), but Tod__Tod's first call is GetActiveDataSourceMethods()->ctor,
 * as TimBlockSrc__TimBlockSrc's is: it is TimBlockSrc's sibling under
 * FileResource and carries none of TimBlockSrc's layout (include/tim_block_src.h).
 *
 * NO OWN FIELDS. The object is 0x2C bytes (New_Tod), FileResource's own size;
 * TodSet's is 0x2C too (New_TodSet). Everything a Tod reads is in the
 * adopted or loaded `buffer`.
 *
 * +0x078 is FileResource's processBuffer (NULL there): this table's occupant is
 * Tod__ScanPackets(self, out, sel), u8, which runs +0x07C over the buffer
 * past its first two words (TodSet's occupant runs it past its counted
 * array); ModelData__ForwardScanPackets casts it (an inherited slot keeps
 * the parent's name).
 *
 * The ctor's descriptor is include/file_resource.h's ResourceSource ({buffer
 * to adopt, file name to request}). The allocators reach the ctor through graphics_resources.c's unprototyped
 * UnprototypedCtorTable view.
 */
```

## `include/tod_set.h`

```c
/*
 * TodSet -- a Tod subclass (class id 0x14F03, method table gTodSetMethods) over
 * a buffer holding several TOD animations: a counted offset table, one Tod
 * per entry, then the packet data. Methods in src/graphics/graphics_resources.c; no
 * subclasses. Its parent is its id parent: TodSet__TodSet's first call is
 * GetTodMethods()->ctor.
 *
 * What its own methods do: TodSet__BuildTods (its
 * +0x064) makes one Tod per entry of the buffer's counted offset table
 * (New_Tod over buffer + entries[i]) and stores each back into the table's
 * own word; TodSet__Finalize releases that array (ReleaseBasicClassArray);
 * TodSet__ScanPackets (+0x078) runs Tod's +0x07C scanner over the data past
 * the counted array. ModelData builds one over its buffer past +0x0C
 * (ModelData__BuildResources: New_TodSet) and forwards its TOD packet scans
 * to it (ModelData.todSet, still declared `FileResource *` in
 * include/model_data.h).
 *
 * NO OWN SLOTS: the table is Tod's 0x84 bytes, with +0x008, +0x00C, +0x064
 * and +0x078 overridden (`classtable.py gTodSetMethods --vs gTodMethods`).
 * +0x064 keeps the inherited name `onRequestDone` and its void type; the occupant
 * TodSet__BuildTods returns s32 (0 when every Tod was built), and the ctor
 * casts the call, as ModelData__ModelData casts its own. +0x078 is
 * FileResource's processBuffer (NULL there); TodSet__ScanPackets occupies it, as
 * Tod__ScanPackets does in Tod's table.
 *
 * NO OWN FIELDS: the object is 0x2C bytes (New_TodSet), Tod's size.
 *
 * The ctor returns self or NULL (New_TodSet tests it), but TOD_SLOTS
 * declares +0x008 returning void, as Tod's own ctor does; the allocator
 * reaches it through graphics_resources.c's unprototyped UnprototypedCtorTable view, as every
 * allocator in that unit does. The descriptor is include/file_resource.h's
 * ResourceSource.
 */
```

## `include/trigger_world.h`

```c
/*
 * TriggerWorld -- a ModelData subclass (class id 0x15F03, method table
 * gTriggerWorldMethods) over a buffer holding several model files: a counted offset
 * table, one ModelData per entry, then the data. Methods in
 * src/graphics/graphics_resources.c; no subclasses. Its parent is its id parent:
 * TriggerWorld__TriggerWorld's first call is GetModelDataMethods()->ctor
 * (with a third argument 0, so ModelData's own resource build and release
 * never act on it).
 *
 * What its own methods do: TriggerWorld__BuildResources (+0x078) makes one
 * ModelData per entry of the buffer's counted offset table (New_ModelData
 * over buffer + entries[i], not owning) and stores each back into the
 * table's own word, counting them at +0x038; TriggerWorld__ReleaseResources
 * (+0x07C) releases that array (ReleaseBasicClassArray); and
 * TriggerWorld__GetModelData (+0x088) returns entry `index`, 0 out of range.
 * Its one outside user, dream_aux.c's FireDreamAuxTriggerEntries, builds one
 * over a trigger group's buffer, and ProcessDreamAuxTriggerRecord passes
 * getModelData(record->parity) on as New_Entity's descriptor word +0x00C,
 * which TodActor__AcquireModelData borrows as the entity's ModelData.
 *
 * SLOTS (`classtable.py gTriggerWorldMethods --vs gModelDataMethods`: 34 against 33): the
 * overrides are +0x008 (ctor), +0x00C (TriggerWorld__Finalize), +0x064
 * (onRequestDone: TriggerWorld__Load, which only runs +0x078), +0x078 (FileResource's
 * processBuffer: TriggerWorld__BuildResources, s32, as ModelData__BuildResources
 * there; callers cast it) and +0x07C (releaseResources:
 * TriggerWorld__ReleaseResources). +0x080/+0x084 are ModelData's forwarders
 * to its todSet, inherited unchanged although this class never builds one.
 * One own slot, +0x088.
 *
 * FIELDS: one own field, +0x038. The object is 0x3C bytes (New_TriggerWorld);
 * ModelData's is 0x38.
 *
 * The ctor returns self or NULL (New_TriggerWorld tests it), but
 * MODELDATA_SLOTS declares +0x008 returning void, as FileResource's own ctor
 * does; the allocator reaches it through graphics_resources.c's unprototyped
 * UnprototypedCtorTable view, as every allocator in that unit does. The descriptor is
 * include/file_resource.h's ResourceSource ({buffer to adopt, file name to request}).
 */
```

## `src/graphics/graphics_resources.c` (file banner)

```c
/*
 * graphics_resources.c -- the FileResource data sources that turn loaded files
 * into graphics objects, the tile-map background layer, and the FMV player.
 *
 * FileResource subclasses, each reached through one of
 * gFileResourceMethods's table getters: an allocator (New_<Class>), a ctor
 * that adopts a buffer or requests a file, finalize, and the class's own
 * load steps.
 *  - TimBlockSrc reads a file of TIM blocks one CD read at a time (a 36-byte
 *    header of block offsets and sizes, then each block into a new
 *    TimArraySrc) and fades up to four 256-colour CLUT rows at VRAM y 480
 *    toward a colour (FadeClutRow).
 *  - TimArraySrc builds one TimImage per image of its buffer and uploads
 *    them.
 *  - LinkResource builds one TmdModel per object of a TMD, NULL-ended.
 *  - Tod walks one TOD animation's packets: ScanTodPackets lists a frame's
 *    object-create packets and finds the object a TMD id belongs to.
 *    TodSet is a Tod over a counted array of Tods.
 *  - ModelData builds a LinkResource and a TodSet from one buffer;
 *    TriggerWorld, its subclass, a counted array of ModelData.
 *  - TileMap and TileAtlas are built rather than loaded: a GsMAP over a
 *    20 x 15 grid of 16 x 16 cells, and the 300 GsCELLs it indexes.
 * Two more classes:
 *  - BgLayer, a SceneNode wrapping one GsBG over a TileMap's GsMAP; its
 *    rotation and scale follow SceneNode's ratio tables.
 *  - MoviePlayer, CD-streamed and MDEC-decoded FMV (CdStream frames,
 *    DecDCTvlc, then DecDCTin/DecDCTout in 16-pixel strips uploaded as they
 *    finish), one movie at a time (sActiveMoviePlayer).
 * The ctors take include/file_resource.h's ResourceSource; the build steps
 * that make them fill one as a ResourceRequest's `src`. The unit's own
 * types: UnprototypedCtorTable, the view the allocators call a ctor slot
 * through when they test its result; TimBlockHeader (and its byte copy,
 * TimBlockHeaderBytes), ModelDataHeader and SubBlockTable, the layouts of
 * TimBlockSrc's, ModelData's and TodSet's / TriggerWorld's buffers.
 *
 * The file holds more than one subject (BgLayer and MoviePlayer are not
 * FileResources); where the original files inside it began is not known.
 */
```

## `src/graphics/tmd_model.c` (file banner)

```c
/*
 * TmdModel (include/tmd_model.h; method table
 * gTmdModelMethods, class tag 9): one object of a TMD file (the "model"
 * SceneNode__LinkModel, src/graphics/scene_node.c, links into a GsDOBJ2). Its
 * methods map the TMD to the GS (TmdModel__MapModelingData), walk its
 * primitives one packet at a time (TmdModel__NextPrimitive, which reads
 * each packet through Sony's own <libgs.h> layouts: GPU_COM_* mode codes,
 * TMD_P_* structs, GsTMDFlagGRD), compute an axis-aligned bounding box or
 * its eight corners (TmdModel__ComputeBounds, TmdModel__GetHull, and the
 * shared buffer of sTmdModelBoundsCount boxes, TmdModel__UpdateBoundsBuffer /
 * TmdModel__GetBoundsBuffer / TmdModel__GetBoundsCount), and ray-cast a
 * segment against every face (TmdModel__RaycastFaces) for SceneNode's own
 * collision helpers in scene_node.c.
 *
 * RotateAndOffsetHullList is a free function over a TmdHull (a counted list
 * of box corners, the buffer Actor__NotifyMove fills through getModelHull):
 * it turns each box a quarter turn and offsets one face. The last two,
 * TmdModel__AddFirstPrimClut and TmdModel__SetFirstPrimClut, move or set the
 * CLUT id of the model's first primitive (TMD_P_TF3's clut) from a VRAM
 * position; SetStyleEffectSources (dream_scene.c) calls the second.
 */
```

## `src/graphics/viewport_draw.c` (file banner)

```c
/*
 * viewport_draw.c -- Viewport__DrawNode: draws one SceneNode into the Viewport's
 * current ordering table, after first drawing each of its SceneNode children
 * the same way.
 *
 * It is slot +0x0A0 (drawNode) of gViewportMethods, inherited unchanged by
 * gNodeGuardedViewportMethods (include/viewport.h). The node's class id
 * picks the draw path, and each path reads the node as the class it tests
 * for:
 *  - a BgLayer: its GsBG to GsSortBg, at the OT's last tag;
 *  - a BoxFill (FadeBox too): its GsBOXF placed from posX/posY, in percent
 *    of the half-screen while `relative` is set and in pixels otherwise, to
 *    GsSortBoxFill at the object's own pri;
 *  - a ScreenSprite (and its subclasses): its GsSPRITE placed from
 *    screenPos, percent of the half-screen from the centre, plus its pivot
 *    (mx, my), to GsSortSprite at tag 0;
 *  - any other Sprite: a world-space sprite. Its coord2's position goes
 *    through the local-screen matrix and a perspective divide by the
 *    Viewport's projH. It is dropped unless its depth is in 1..0xFFFF (the
 *    GTE's 16-bit screen z) and past nearZ, its x/y are clamped to
 *    SPRITE_POS_LIMIT, and its depth past nearZ over zDiv is its OT tag;
 *  - anything else: the node's own GsDOBJ2 (+0x010), under GsGetLws's light
 *    and local-screen matrices, to SortTmdObject (tmd_renderer.c), the
 *    game's replacement for GsSortObject4, when it has a TMD.
 * A GridCell whose GsDOFF bit is set is skipped, children and all.
 *
 * Before drawing, a node whose coord2 is dirty (flg == 0) rebuilds its
 * matrix from its GsCOORD2PARAM's rotate and scale and marks each child it
 * draws dirty in turn.
 *
 * The subclasses spell their GsBG/GsBOXF/GsSPRITE field by field, hence the
 * casts to Sony's types at the libgs calls.
 *
 * The rest of the Viewport class is in task.c.
 */
```
