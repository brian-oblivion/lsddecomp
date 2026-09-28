# Dream-day headers: process text moved out by the API pass

Track 12 (item `area-dream-day`, round 106, runner delta) rewrote these
headers as Doxygen API documentation:

    include/day_task.h include/dream_day.h include/entity.h
    include/stage_map.h include/timed_task.h

and cut the long banners of src/world/dream_day.c and src/world/entity.c
down to what each file and section holds, the class explanations having
moved into the headers' class docs. Text that justified a C spelling in one
function went to that function's match report, under "History: track 12",
and left a one-line `MATCHING:` note in the .c. This note holds what
belongs to no single function: the tool pointers the headers carried, the
derivations behind header-wide types and slot types, and one section
banner's table provenance. `python3 tools/apidoc.py --item area-dream-day`
measures what is left in the files.

The pass changed comments only, so the C spells each construct as it did;
the one-line notes in the .c (for EntityPlayTodFn and the moveLocalZ/Y slot
types, at the top of src/world/entity.c) point back to the reason.

## History: the passages as they stood before the pass, verbatim

The source of every excerpt is commit `0403db82c`, the pass's first, which
had not yet touched stage_map.h, entity.h or entity.c.

### Tool pointers

`include/stage_map.h`, the class banner:

```c
 * Inherited slots it overrides (`tools/classtable.py gStageMapMethods --vs
 * gLightRigMethods`): +0x008 the ctor, +0x00C Finalize, +0x038 OnNotify,
 * +0x040 Reset, +0x088 notifyWithHull (StageMap__OnSlotEvent, see
 * below), +0x098 update (StageMap__UpdateIfEnabled) and +0x09C
 * DispatchLinkCommand.
```

`include/entity.h`, above struct EntityMethods:

```c
/* TodActor's slots (overrides: +0x008 Entity__Entity, +0x00C
 * Entity__Finalize, +0x040 Entity__Reset, +0x04C Entity__AttachToParent,
 * +0x050 Entity__DetachFromParent, +0x098 Entity__Update, +0x0DC
 * Entity__NotifyLinkStage, +0x0E0 Entity__OnGridCellLinkCommand, +0x11C
 * Entity__TickSoundCue; `tools/classtable.py gEntityMethods --vs
 * gTodActorMethods`), then this class's own. */
```

### entity.h: inherited slot types settled by the callers' code

The class banner:

```c
 * Inherited slot types, settled by callers' bytes (keep them):
 *  - moveLocalZ (+0x0C4) and moveLocalY (+0x0CC) return void. Entity__MoodCue00
 *    tail-merges two moveLocalZ calls and Entity__MoodCue115 merges a
 *    moveLocalY call with void siblings; GCC 2.6.3 cannot cross-jump a
 *    value-returning call with a void one (docs/match-reports/
 *    Entity__MoodCue115.md). Entity__MoodCue32/37 compile the same either way.
 *  - applyTodFrame (+0x134) returns the next frame: Entity__MoodCue91/92
 *    thread todFramePtr through it.
 *  - attachToParent (+0x04C) keeps SceneNode's type; callers of Entity's
 *    occupant cast to TodActorAttachToParentFn (tod_actor.h's banner).
 *
```

Above EntityPlayTodFn:

```c
/* playTod (+0x12C) is TodActor's slot and returns the flag its occupant
 * sets, but Entity's callers call it as void: Entity__MoodCue39/57 (Entity)
 * and Entity__MoodCue86 (Entity) cross-jump a playTod call with a void
 * sibling (stopTod), which GCC 2.6.3 does only when both are void (the
 * moveLocalZ case in the banner); through the s32 slot they grow 3 words
 * each. Every Entity call site casts the slot to this typedef, which emits
 * no code. */
typedef void (*EntityPlayTodFn)(Entity *self);
```

### entity.h: the mood row's signed columns and its column tables

```c
/* One row of the mood table (16 bytes): New_Entity's moodIndex selects it, and
 * every per-mood setting of an Entity is a column of it. Signed columns are
 * `s8` (`lb`); plain `char` would be unsigned here (-funsigned-char).
 *
 * sEntityLinkStageTable and sEntityEventVideoTable are two of its columns
 * seen as flat arrays (the row base + 7 and + 8, indexed moodIndex * 16):
 * GCC spells a constant-offset field of a global array as `%hi`/`%lo(sym +
 * off)`, which splat labels as a symbol of its own. Entity still reads them
 * that way; the field spelling compiles to the same bytes. */
```

The column-table note is why src/world/entity.c carries no `MATCHING:` line on
sEntityLinkStageTable and sEntityEventVideoTable: by that measurement the field
spelling compiles to the same bytes.

### stage_map.h: types whose layout serves other areas' copies

CellKey, CellOffset and CellKeyDesc are built by src/world/dream_aux.c and
src/world/dream_scene.c, outside this item, so their note has no report here:

```c
/* A Descriptor10 as dream_aux.c and the style layer build it for
 * computeCellOffsets, from two halves each copied whole: `key` (the chunk's
 * column/row bytes as one u16, then the cell's) and `offset`, the s16 x/y/z
 * inside the cell. MATCHING: x and y are one struct, so each copy is one
 * lwl/lwr pair (plus the z halfword). */
```

The slot-shape note, whose "(no code)" meant the cast emits no instruction:

```c
 * +0x088: StageMap__OnSlotEvent takes (self, command, slot); the slot
 * keeps SceneNode's (self, event). Its four callers (Finalize,
 * UnloadAllSlots, ApplyChunkLoads, OnDrawSystemEvent) pass (self, 6 or 7,
 * slot, index) through StageMapOnSlotEventFn below (no code).
 * +0x0D4 getCurrentCellKey and +0x0F8 loadChunksAround keep their CALLERS'
 * shapes: DreamSys__WallLink passes getCurrentCellKey a second word the
 * occupant never reads, and SetTargetAndLoadChunks returns
 * loadChunksAround's value although the occupant returns nothing.
 *
```

ChunkLoadEntryTail:

```c
/* The same 0xC stride based at +0x4: ApplyChunkLoads' second walker
 * (strength-reduced from the parameter; its report). */
```

The notes on Descriptor10, SplitLongVec3, SplitCoord2 and CellRectSet went to
the reports of the functions that copy or read them
(StageMap__SetTargetAndLoadChunks, StageMap__ComputeFootprintDescriptor,
StageMap__ApplyToSenderFootprint).

### entity.c: the banner of the rows 19 to 38 section

It named the rodata file the handler table was checked against, and the
rename history of the fields:

```c
```
