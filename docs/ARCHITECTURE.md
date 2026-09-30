# How the game is built

A map of LSD: Dream Emulator's code for a reader new to it: the objects the
game is made of, how control gets from `main` to a frame on screen, and how
one day of the dream runs. It is a guide to where things are, not a
reference. Each class's header in `include/` documents the class in full,
and each `.c` file opens with a banner saying what it holds. When this page
and a header disagree, the header is right.

## The framework in one paragraph

The game is C89 written as objects. Every object starts with a pointer to
its method table, a flat struct of function pointers, and a method is a
plain function `Class__Method(Class *self, ...)` called through the table:
`self->methods->update(self, ...)`. Word 0 of each table is a class id
whose nibbles encode the inheritance chain. `BasicClass` is the root. Every
object also keeps two lists: its `children`, and `parentRefs`, the objects
holding it. Objects talk upward with `notifyParents(self, event)`, which
calls each holder's `onNotify(holder, sender, event)`. Most of the game's
control flow is that call: a holder that hears an event decides what it
means by looking at the *sender's* class id. `include/basic_class.h`
explains the framework and how a class header is laid out, and it's the
place to start. All memory comes from one pool, `BMemPMgr`
(`src/app/bmem_pmgr.c`), which `main` creates.

## The classes

The tree, by the parent each header names (ids in hex):

```
BasicClass 0x0
├── DrawSystem 0x1           the screen: libgs setup, VRAM transfers, the VSync loop
├── Pad 0x2                  one controller: button edges, one event per button
├── FileResource 0x3         anything loaded from a file, through the data source
│   ├── CdDriver, NullDriver       the data-source drivers FileResource I/O goes to
│   ├── TimImage, TileMap, TileAtlas, TimArraySrc, TimBlockSrc   textures and 2-D
│   ├── LbdFile                    one stage map chunk, STGnn\Mnnn.LBD
│   ├── LinkResource               a TMD file split into TmdModels
│   ├── ModelData → TriggerWorld   a model file's TMD models and TOD animations
│   ├── Tod → TodSet               TOD animations
│   ├── VabStreamObj               one VAB sound bank
│   └── RequestedFile
├── SceneNode 0x4            anything positioned or drawn (a libgs GsCOORDINATE2 + GsDOBJ2)
│   ├── LightRig 0x14 → StageMap 0x114     the loaded part of a stage's map
│   ├── GridCell 0x24                      one cell of the map's grid
│   ├── Actor 0x34                         a SceneNode that moves and tests links
│   │   ├── TodActor → Entity              a TOD-animated thing in the dream
│   │   ├── DreamSys 0x1F34                the player and the dream in progress
│   │   └── StyleEffect                    the style layer's decorations
│   ├── Sprite → ScreenSprite → CharSprite → TextRow,  Sprite → VariantSprite
│   ├── BgLayer, BoxFill → FadeBox
├── FrameClock 0x5           ticks once a frame: running, paused or stopped
├── Viewport 0x7 → NodeGuardedViewport     renders a scene tree into an ordering table
├── TmdModel 0x9             one object of a TMD file, with collision helpers
├── IntermediateBase 0x30    a job the game runs to a result ("task")
│   ├── TaskCore 0x130       menus and screens: fades, a slot picker
│   │   ├── StreamTask       plays one FMV through a MoviePlayer
│   │   ├── TitleMenu        the menu between days
│   │   └── GraphRoom        the mood graph screen
│   └── TimedTask 0x230      a job with a frame timeout and a sound bank
│       ├── DayTask          one day of the dream
│       └── ObjM             one stage's scene within the day
├── Application 0x60 → GameApplication 0x1F60   the program and its sequence
├── CdStream 0x40, MoviePlayer 0x70             streamed FMV (CD + MDEC)
├── WBgm 0x50                background music: a SEQ on a VAB bank
├── TaskObjF, TextEntry, ItemList               the memory-card screens
└── FlatLightObj 0x6
```

`tools/classtable.py <table>` prints a class's method table slot by slot.

## From `main` to a frame

1. **`main`** (`src/main.c`) sets up the memory pool, builds the one
   `GameApplication` from `sGameApplicationConfig`, a `DrawSystem` and a
   `Pad`, hands the last two to `initSystems`, and calls `runMainLoop`,
   which never returns.
2. **`Application__RunMainLoop`** (`src/app/application.c`) is the game's
   outer loop. It calls `GameApplication`'s hooks: the intro logos once,
   then forever an opening movie and the title menu loop, which runs the
   days (and, once a year of days has gone by, the ending movie).
   `include/game_application.h` lists the hooks in the order they run.
3. **Every screen is a task.** A hook builds an `IntermediateBase` subclass
   (a `StreamTask` for a movie, a `TaskCore` for an image, `TitleMenu`,
   `GraphRoom`, `DayTask`), calls its `init`, reads its result, and releases
   it. With `INTERMEDIATEBASE_INIT_RUN` the whole job runs *inside* `init`:
   it adds the DrawSystem, the Pad and a FrameClock as children, attaches
   them to the viewport, calls `setState(START)`, and `START` starts the
   DrawSystem.
4. **The frame loop is the DrawSystem's.** `DrawSystem`'s start runs a
   VSync loop that notifies its parents with `DRAWSYSTEM_EVENT_VSYNC` on
   every pass, until a task's `STOP` stops it and `init` returns. On each
   VSync, `IntermediateBase__OnDrawSystemEvent` ticks the FrameClock and
   polls the Pad.
5. **The FrameClock fans the frame out.** Its tick notifies everything that
   holds it. The task's `update` runs its state machine, `Viewport` redraws,
   `DreamSys__TimerTick` advances the dream clock, and each `TodActor`
   steps its animation. The Pad's events reach `onPadEvent` on whoever
   holds the Pad.
6. **The Viewport draws.** On the FrameClock tick, `Viewport__Update` sets
   the projection, lighting and fog, and `drawNode`
   (`src/graphics/viewport_draw.c`) walks the scene tree, sorting each
   node into one half of a double-buffered ordering table. Models go
   through the game's own TMD renderer (`src/graphics/tmd_renderer.c`),
   which replaces libgs's `GsSortObject4` and subdivides faces that are too
   large on screen. On the next VSync the Viewport's flip clears and draws
   that half (`GsDrawOt`) and swaps.

So nothing in the game polls in a loop of its own. A frame is a VSync
event travelling up the object graph, and each object reacts to whichever
sender it hears from.

## One day of the dream

`GameApplication__RunDayTask` runs one `DayTask` (`src/world/day_task.c`).

- **The DayTask** loads the day's shared resources: `ETC\ETC.TIM`,
  `ETC\DREAMER.TMD` and the week's BGM. It builds the day's FrameClock, a
  `StageMap` (the light rig the scene hangs from) and a
  `NodeGuardedViewport`, and adopts the one `DreamSys`. On its first VSync
  it calls `DreamSys`'s `startDay`, which picks the stage to start on, and
  starts an **ObjM** on that stage.
- **The ObjM** (`src/world/objm.c`) runs one stage's scene. It points the
  StageMap at the stage's chunk records, picks the stage's BGM, loads the
  day's TIM block, and asks the style layer (`src/world/style_layer.c`)
  for the scene's colours, fog and decoration. While the scene is live it
  maps Start, Select and Triangle to the pause overlay and to closing the
  scene, and hears the DreamSys's link and time-up codes. It passes those
  up to the DayTask as its own states.
- **The StageMap** (`src/world/stage_map.c`) keeps seven map chunks loaded
  around the player, the one they're in and its six neighbours (odd rows
  are offset half a chunk, so the grid is hexagonal). Each chunk is an
  `LbdFile` read from the CD whose models and placements are linked into a
  lattice of `GridCell`s. As the player crosses into another chunk, the
  StageMap notifies `STAGEMAP_EVENT_CHUNK_CHANGED` and reloads the slots
  around the new chunk.
- **The DreamSys** (`src/world/dream_sys.c`) is the player and the state
  of the dream. It owns the dream clock (the day ends at `dreamTimeLimit`),
  movement and looking (Pad commands consumed by per-tick callbacks), the
  mood graph (each map chunk owns a point on it, `src/world/stage_grid.c`,
  and the chunks visited are logged into two running averages), flashbacks,
  the save block, and the **links**. Links are how a stage ends: walking
  into a wall, a tunnel, a staircase or a teleporter, or an entity sending
  the dream a link effect, ends in `ExecuteLink`. The ObjM hears the link code, and the
  DayTask replaces the ObjM with one on the new stage. That's how the dream
  moves from place to place without the day ending.
- **Entities** (`src/world/entity.c`, spawned by `src/world/dream_aux.c`
  as chunks load) are the things met in the dream. Each is a TOD-animated
  actor driven by one row of `sEntityMoodTable`: when it appears and
  disappears, which sound cue it runs, and what it tells the dream when the
  player reaches it (a link, a video, the end of the dream). The row's
  **MoodCue handler** is a small per-tick script. It runs once per tick
  through the entity's `SoundCueSet` and moves, turns or scales the entity
  (or the player) while requesting tones. `include/entity.h` explains how
  the handlers are named.
- **The day ends** on time-up or when the dream closes. The DayTask calls
  `endDay`, its `init` returns a `DayTaskResult`, and the outer loop goes
  back to the title menu. `GraphRoom` (`src/world/graph_room.c`) plots the
  days' moods on the mood graph screen.

## The other systems

- **Files and the CD** (`src/cd/`, `src/app/data_source.c`). Every
  `FileResource` loads through the active data source, a driver object
  whose slots FileResource's calls are rebound to. On retail that's
  `CdDriver`, a request queue and read state machines serviced from the
  DrawSystem's callback. `src/cd/game_files.c` is the table of every file
  on the disc (`STG00`..`STG13` hold each stage's textures, music and map
  chunks) with its getters.
- **Movies** (`src/graphics/movie_player.c`, `src/cd/cd_stream.c`). A
  `StreamTask` drives a `MoviePlayer`, which streams sectors with libcd's
  streaming library and decodes frames on the MDEC strip by strip.
- **Sound** (`src/sound/`). `WBgm` plays a stage's background SEQ on its
  VAB bank, `VabStreamObj` wraps one VAB bank and plays tones, and
  `SoundCueSet` is the three-voice cue an owner (an Entity, the style
  layer) services once per tick.
- **Menus and saves** (`src/ui/`). `TitleMenu` and `GraphRoom` are
  `TaskCore`s. TaskCore supplies fades, a slot picker and a scrolling item
  list. `TaskObjF` is the memory-card save and load controller; the save
  data is DreamSys's save block.
- **Sony's libraries** (`src/psyq/` and `lib/`). libgs, libgpu, libgte,
  libcd, libsnd, libspu, libpress (MDEC), libcard, libetc and libapi are
  linked from Sony's own objects. `src/psyq/` holds the few Sony functions
  no shipped SDK object matches. Nothing Sony owns takes a game name.

## Where the code's meaning runs out

The mechanics are all in the code. What they stand for in the game is not
always established, and the code says so rather than guess:

- **Stage ids** are still numbers (`currentStage == 9`), and which id is
  which area hasn't been confirmed in-game.
- **ObjM's states and the style configs** have mechanical names. The
  headers say which readings the evidence doesn't settle.
- **Entities** are named for what their MoodCue handler does, not for what
  the player sees; which dream object owns each mood row is open.

## Reading order

1. `include/basic_class.h`: the framework.
2. `src/main.c`, `include/game_application.h`, `src/app/application.c`:
   the program's shape.
3. `include/intermediate_base.h`, `include/task_core.h`: how a screen runs.
4. `include/day_task.h`, `include/objm.h`, `include/dream_sys.h`: a day.
5. `include/stage_map.h`, `include/entity.h`: the world and what's in it.
6. `include/viewport.h`, `include/scene_node.h`,
   `src/graphics/tmd_renderer.c`: drawing.

To go from a name to its code: `grep -rn 'Name' src/ include/` finds the
definition, the prototype and the direct calls, and `tools/classtable.py`
finds the method-table slot a function fills (calls through the table
don't mention the function's name).
