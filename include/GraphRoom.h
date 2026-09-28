#ifndef GRAPHROOM_H
#define GRAPHROOM_H

#include "TaskCore.h"

/*
 * GraphRoom -- class id 0x2F130, method table gGraphRoomMethods, a TaskCore
 * subclass (`tools/classtable.py gGraphRoomMethods --vs gTaskCoreMethods`:
 * nine overrides and one slot of its own). No class derives from it.
 * src/world/ObjMStyleActor.c holds the whole class: allocator, ctor, every
 * override, ScoreDayLog, and the getter.
 *
 * The name is a reading of its data: reset loads "ETC\HGRAPH.TIM" as the
 * sub-handle, and the object owns 100 BoxFill dots (`points`) plotted from
 * the DreamSys's 365-day mood ring. What it is in the game (the graph
 * screen) is that reading; the mechanics below are measured.
 *
 * Who makes one: GameApplication__RunTitleMenu (src/app/GameApplicationFileResource.c), through
 * GameApplication__RunTask(New_GraphRoom, self->dreamSys, ...), so the ctor's
 * one argument, kept at +0x0A4, is the game's DreamSys. Its +0x1B0 is
 * DreamSys__GetSaveBlock, which returns &saveMagic; this unit reads that
 * block as DreamSaveBlock (include/dream_sys.h: +0x004 currentYear, +0x008
 * currentDay, +0x018 moodPreviousDays[365], the same offsets as DreamSys's
 * fields from saveMagic at DreamSys +0x178).
 *
 * Construction, ctor(dreamSys): TaskCore's ctor with (NULL, "ETC\ETCSE",
 * NULL), this class's table, the sound's +0x09C (VabStreamObj__SetPitchOffset)
 * with -1, dreamSys kept, then setTarget(NULL) -- which this class overrides
 * with BuildGraphPoints -- and a tail call of resetCounters (GraphRoom__Reset).
 * That last call passes dreamSys as a second argument the slot does not
 * have and Reset does not read ($a1 is loaded in the retail bytes), so the
 * ctor casts the slot to GraphRoomResetCallFn below. The ctor returns
 * nothing: INTERMEDIATEBASE_SLOTS's ctor type is kept.
 *
 * Overrides, each named for its slot unless the body does something else:
 *   +0x008 ctor           GraphRoom__GraphRoom
 *   +0x040 resetCounters  GraphRoom__Reset: fadeRate 5, unk2C 0x190,
 *                         setSubHandle("ETC\HGRAPH.TIM", NULL),
 *                         setFrameBound(10). No up-call to TaskCore__Reset.
 *   +0x044 init           GraphRoom__Init: TaskCore's, then 2 when `scored`
 *                         is set, else `result`.
 *   +0x05C update         GraphRoom__Update: TaskCore's; with inputMode 1 and
 *                         a non-empty log, points[0] blinks on frameCounter's
 *                         low bit; then tickHighlight.
 *   +0x078 onPadConfirm   GraphRoom__OnPadConfirm: unscored only,
 *                         playSound(0x10) and exit.
 *   +0x0D8 setTarget      GraphRoom__BuildGraphPoints: no target at all; the
 *                         100 BoxFill dots and the 4-byte matchedDayIndices.
 *   +0x0DC releaseTarget  GraphRoom__ReleaseGraphPoints: frees what
 *                         BuildGraphPoints made, then TaskCore's.
 *   +0x0E0 updateSlotElements GraphRoom__PopulateGraphPoints: TaskCore's,
 *                         then ScoreDayLog and one dot a logged day.
 *   +0x124 tickHighlight  (own slot) GraphRoom__TickHighlight.
 *
 * Two accessors read an inherited field at a type other than the parent's:
 * TickHighlight compares frameCounter unsigned (`sltiu`, `divu`) and casts
 * it to u32; the ctor calls `sound` (TaskCore's BasicClass *) past
 * BasicClass's slots and casts it to its own view of VabStreamObj.
 *
 * The object is 0x244 bytes (New_GraphRoom's allocation).
 */

typedef struct GraphRoom GraphRoom;
typedef struct GraphRoomMethods GraphRoomMethods;

struct DreamSys;
struct BoxFill;
struct DreamSaveBlock;

struct GraphRoomMethods {
    TASKCORE_SLOTS(GraphRoom, (GraphRoom * self, struct DreamSys *dreamSys));
    /* +0x124 */ void (*tickHighlight)(GraphRoom *self); /* GraphRoom__TickHighlight; update's last call */
};

struct GraphRoom {
    TASKCORE_FIELDS(GraphRoomMethods);
    /* +0x0A4 */ struct DreamSys *dreamSys; /* the ctor's; +0x1B0 (GetSaveBlock) is the day log */
    /* +0x0A8 */ struct BoxFill *points[100]; /* BuildGraphPoints' dots, one a logged day, newest first */
    /* +0x238 */ s32 scored;         /* ScoreDayLog's result, kept by PopulateGraphPoints */
    /* +0x23C */ u32 highlightCount; /* TickHighlight: targets highlighted so far, < 4 */
    /* +0x240 */ s8 *matchedDayIndices; /* BMemPMgrAlloc(4): per ScoreDayLog target, the dot it matched */
};

/* The ctor's resetCounters call, as the retail bytes make it: the slot is
 * (self), the call also passes dreamSys (see the banner). No code. */
typedef void (*GraphRoomResetCallFn)(GraphRoom *self, struct DreamSys *dreamSys);

extern GraphRoomMethods gGraphRoomMethods;
extern GraphRoomMethods *GetGraphRoomMethods(void); /* returns &gGraphRoomMethods */

/* GraphRoom__Init's result when `scored` is set; GameApplication__RunTitleMenu
 * then plays GameApplication__PlaySpecialDayMovies. */
#define GRAPHROOM_RESULT_SCORED 2

/* The class's own methods, in ROM order (ObjMStyleActor). */
GraphRoom *New_GraphRoom(struct DreamSys *dreamSys);
void GraphRoom__GraphRoom(GraphRoom *self, struct DreamSys *dreamSys);
void GraphRoom__Reset(GraphRoom *self);
void GraphRoom__Update(GraphRoom *self, BasicClass *sender, s32 event);
void GraphRoom__OnPadConfirm(GraphRoom *self);
void GraphRoom__BuildGraphPoints(GraphRoom *self);
void GraphRoom__ReleaseGraphPoints(GraphRoom *self);
s32 GraphRoom__Init(GraphRoom *self, IntermediateBaseInitArgs *args, s32 mode);
void GraphRoom__PopulateGraphPoints(GraphRoom *self, void *parent);
s32 GraphRoom__ScoreDayLog(GraphRoom *self, struct DreamSaveBlock *log);
void GraphRoom__TickHighlight(GraphRoom *self);

#endif
