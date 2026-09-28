#ifndef GRAPH_ROOM_H
#define GRAPH_ROOM_H

#include "task_core.h"

/**
 * @file graph_room.h
 * @brief GraphRoom, the TaskCore that shows the mood graph: up to 100 days
 *        of the dreamer's mood plotted as dots over ETC\\HGRAPH.TIM.
 *
 * Methods in src/world/dream_scene.c, New_GraphRoom through
 * GetGraphRoomMethods.
 */

typedef struct GraphRoom GraphRoom;
typedef struct GraphRoomMethods GraphRoomMethods;

struct DreamSys;
struct BoxFill;
struct DreamSaveBlock;

/**
 * @brief GraphRoom's method table: TaskCore's slots, with a ctor that takes
 * the DreamSys, then one of its own.
 *
 * Overrides of TaskCore's slots: +0x008 GraphRoom__GraphRoom, +0x040
 * resetCounters (GraphRoom__Reset, with no call up to TaskCore__Reset),
 * +0x044 GraphRoom__Init, +0x05C GraphRoom__Update, +0x078
 * GraphRoom__OnPadConfirm, +0x0D8 setTarget (GraphRoom__BuildGraphPoints,
 * which takes no target at all), +0x0DC releaseTarget
 * (GraphRoom__ReleaseGraphPoints) and +0x0E0 updateSlotElements
 * (GraphRoom__PopulateGraphPoints).
 */
struct GraphRoomMethods {
    TASKCORE_SLOTS(GraphRoom, (GraphRoom * self, struct DreamSys *dreamSys));
    /* +0x124 */ void (*tickHighlight)(GraphRoom *self); /**< @see GraphRoom__TickHighlight */
};

/**
 * @brief The mood graph screen (class id 0x2F130), a TaskCore subclass. No
 * class derives from it.
 *
 * The name is a reading of its data: reset loads ETC\\HGRAPH.TIM as the
 * sub-handle, and the object owns 100 BoxFill dots (`points`) plotted from
 * the DreamSys's 365-day mood ring. That it is the game's graph screen is
 * that reading; the mechanics are measured.
 *
 * Who makes one: GameApplication__RunTitleMenu (src/app/game_shell.c),
 * through GameApplication__RunTask(New_GraphRoom, self->dreamSys, ...), so
 * the ctor's one argument is the game's DreamSys. Its getSaveBlock returns
 * the save block this class reads as DreamSaveBlock (include/dream_sys.h:
 * currentYear, currentDay, moodPreviousDays[365]).
 *
 * The plot: newest day first, each of up to 100 days is a 10-pixel dot
 * centred on its two signed mood bytes times 10, the upper axis pointing up
 * the screen. The newest dot is red and blinks while inputMode is 1; the
 * others fade from white. ScoreDayLog checks, once per save, that four fixed
 * moods (sGraphScoreMoods) all appear among the plotted days, and
 * TickHighlight then turns each one's dot green, one every 24 frames once
 * frameCounter passes 30. A scored graph cannot be left with Confirm, and
 * init then returns GRAPHROOM_RESULT_SCORED.
 *
 * The ctor calls `sound` (TaskCore's BasicClass *) past BasicClass's slots,
 * casting it to VabStreamObj, and TickHighlight reads frameCounter as
 * unsigned.
 */
struct GraphRoom {
    TASKCORE_FIELDS(GraphRoomMethods);
    /* +0x0A4 */ struct DreamSys *dreamSys; /**< The ctor's; its save block is the day log. */
    /* +0x0A8 */ struct BoxFill *points[100]; /**< BuildGraphPoints' dots, one a logged day, newest first. */
    /* +0x238 */ s32 scored;            /**< ScoreDayLog's result, kept by PopulateGraphPoints. */
    /* +0x23C */ u32 highlightCount;    /**< TickHighlight: targets highlighted so far, up to 4. */
    /* +0x240 */ s8 *matchedDayIndices; /**< Four bytes: per ScoreDayLog target, the dot it matched. */
}; /* 0x244 bytes: New_GraphRoom */

/**
 * @brief The ctor's resetCounters call: the slot is (self), and the call
 * also passes the DreamSys, which GraphRoom__Reset does not read.
 */
typedef void (*GraphRoomResetCallFn)(GraphRoom *self, struct DreamSys *dreamSys);

/** @brief GraphRoom's method table (see GraphRoomMethods). */
extern GraphRoomMethods gGraphRoomMethods;

/**
 * @brief Returns GraphRoom's method table.
 * @return &gGraphRoomMethods.
 */
extern GraphRoomMethods *GetGraphRoomMethods(void);

/** GraphRoom__Init's result when `scored` is set; GameApplication__RunTitleMenu
 * then plays GameApplication__PlaySpecialDayMovies. */
#define GRAPHROOM_RESULT_SCORED 2

/**
 * @brief Allocates a GraphRoom from the BMemPMgr pool and constructs it.
 * @param dreamSys The game's DreamSys, whose save block is plotted.
 * @return The new object, or NULL when the pool is exhausted.
 */
GraphRoom *New_GraphRoom(struct DreamSys *dreamSys);

/**
 * @brief Constructor (slot +0x008): TaskCore's ctor with the sound bank
 * ETC\\ETCSE, this class's table, the sound's pitch offset -1, the DreamSys
 * kept, then setTarget(NULL), which builds the dots, and resetCounters. It
 * returns nothing, as the inherited ctor slot's type says.
 * @param self     The object being constructed.
 * @param dreamSys The game's DreamSys.
 */
void GraphRoom__GraphRoom(GraphRoom *self, struct DreamSys *dreamSys);

/**
 * @brief resetCounters (slot +0x040): fade rate 5, 400 packets, the
 * ETC\\HGRAPH.TIM sub-handle, and a ten-second frame bound.
 * @param self The graph.
 */
void GraphRoom__Reset(GraphRoom *self);

/**
 * @brief update (slot +0x05C): TaskCore's; with inputMode 1 and a non-empty
 * log the newest dot blinks on frameCounter's low bit; then tickHighlight.
 * @param self   The graph.
 * @param sender The FrameClock.
 * @param event  The event code.
 */
void GraphRoom__Update(GraphRoom *self, BasicClass *sender, s32 event);

/**
 * @brief onPadConfirm (slot +0x078): unless the graph scored, plays a sound
 * and exits.
 * @param self The graph.
 */
void GraphRoom__OnPadConfirm(GraphRoom *self);

/**
 * @brief setTarget (slot +0x0D8): makes the 100 dots, the newest in red and
 * the rest fading from the base colour (by 20 a dot to the seventh, then by
 * 1), and matchedDayIndices, four bytes.
 * @param self The graph.
 */
void GraphRoom__BuildGraphPoints(GraphRoom *self);

/**
 * @brief releaseTarget (slot +0x0DC): frees what BuildGraphPoints made, then
 * TaskCore's releaseTarget.
 * @param self The graph.
 */
void GraphRoom__ReleaseGraphPoints(GraphRoom *self);

/**
 * @brief init (slot +0x044): TaskCore's init.
 * @param self The graph.
 * @param args The init args.
 * @param mode The init mode.
 * @return GRAPHROOM_RESULT_SCORED when the graph scored, else `result`.
 */
s32 GraphRoom__Init(GraphRoom *self, IntermediateBaseInitArgs *args, s32 mode);

/**
 * @brief updateSlotElements (slot +0x0E0): TaskCore's, then scores the day
 * log and places one dot a logged day under `parent`, newest first, the
 * newest attached last.
 * @param self   The graph.
 * @param parent The node the dots attach to.
 */
void GraphRoom__PopulateGraphPoints(GraphRoom *self, void *parent);

/**
 * @brief Whether every sGraphScoreMoods entry appears among the plotted
 * days, recording in matchedDayIndices the oldest dot holding each. Fails at
 * once when the log is already scored, and marks it scored on success.
 * @param self The graph.
 * @param log  The DreamSys's save block.
 * @return 1 when all four moods were found, else 0.
 */
s32 GraphRoom__ScoreDayLog(GraphRoom *self, struct DreamSaveBlock *log);

/**
 * @brief tickHighlight (slot +0x124): once a scored graph's frameCounter
 * passes 30, turns the next matched dot green every 24 frames, four in all.
 * @param self The graph.
 */
void GraphRoom__TickHighlight(GraphRoom *self);

#endif
