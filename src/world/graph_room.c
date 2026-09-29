/*
 * GraphRoom's methods (include/graph_room.h: the TaskCore that shows the
 * mood graph), in ROM order: allocator, ctor, every override, ScoreDayLog
 * and TickHighlight, ending with its getter GetGraphRoomMethods.
 *
 * GraphRoom loads "ETC\HGRAPH.TIM" and plots up to 100 days of the
 * DreamSys's mood ring (moodPreviousDays, read from the save block
 * DreamSys__GetSaveBlock returns), newest first, as 10-pixel BoxFill dots:
 * a day's two signed mood bytes, times 10, are its dot's centre, the upper
 * axis pointing up the screen. The newest dot is red and blinks while
 * inputMode is 1; the others fade from white. ScoreDayLog checks, once per
 * save, that four fixed moods (sGraphScoreMoods) all appear among the
 * plotted days, and TickHighlight then turns each one's dot green, one
 * every 24 frames once frameCounter passes 30.
 */
#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "graph_room.h"
#include "dream_sys.h"
#include "vab_stream_obj.h"
#include "box_fill.h"
#include "bmem_pmgr.h"

/* sGraphScoreMoods' length: ScoreDayLog's targets, one matchedDayIndices
 * byte and one highlight each. */
#define GRAPH_SCORE_MOOD_COUNT 4

/* A plotted dot's side, sGraphPointSize's {10, 10}: PopulateGraphPoints
 * subtracts half of it so each dot is centred on its mood. */
#define GRAPH_POINT_SIZE 10

/* Screen pixels per step of a mood axis (PopulateGraphPoints). */
#define GRAPH_PIXELS_PER_MOOD 10

/* GraphRoom's object, table and methods: include/graph_room.h. The base
 * implementations are reached through GetTaskCoreMethods() with `self`
 * upcast. */

/* DreamSaveBlock, the save block GraphRoom plots, is include/dream_sys.h's. */

/* GraphRoom's method table, class id 0x2F130: TaskCore's slots, with
 * GraphRoom's overrides, then its own tickHighlight at +0x124. A slot whose
 * function is declared for another class's `self` takes a `void *` cast. */
/* clang-format off */
GraphRoomMethods gGraphRoomMethods = {
    /* +0x000 header */ 0x2F130,
    /* +0x004 release */ (void *)BasicClass__Release,
    /* +0x008 ctor */ GraphRoom__GraphRoom,
    /* +0x00C finalize */ (void *)TaskCore__Finalize,
    /* +0x010 addChild */ (void *)BasicClass__AddChild,
    /* +0x014 removeChild */ (void *)BasicClass__RemoveChild,
    /* +0x018 removeAllChildren */ (void *)BasicClass__RemoveAllChildren,
    /* +0x01C getNextChild */ (void *)BasicClass__GetNextChild,
    /* +0x020 addParentRef */ (void *)BasicClass__AddParentRef,
    /* +0x024 removeParentRef */ (void *)BasicClass__RemoveParentRef,
    /* +0x028 clearParentRefs */ (void *)BasicClass__ClearParentRefs,
    /* +0x02C getNextParentRef */ (void *)BasicClass__GetNextParentRef,
    /* +0x030 notifyParents */ (void *)BasicClass__NotifyParents,
    /* +0x034 slot34 */ BasicClass__NoOpSlot34,
    /* +0x038 onNotify */ (void *)IntermediateBase__OnNotify,
    /* +0x03C slot3C */ NULL,
    /* +0x040 resetCounters */ GraphRoom__Reset,
    /* +0x044 init */ GraphRoom__Init,
    /* +0x048 deinit */ (void *)IntermediateBase__Deinit,
    /* +0x04C onInit */ (void *)TaskCore__OnInit,
    /* +0x050 onDeinit */ (void *)TaskCore__OnDeinit,
    /* +0x054 onDrawSystemEvent */ (void *)IntermediateBase__OnDrawSystemEvent,
    /* +0x058 onPadEvent */ (void *)TaskCore__OnPadEvent,
    /* +0x05C update */ GraphRoom__Update,
    /* +0x060 setState */ (void *)TaskCore__SetState,
    /* +0x064 onStart */ (void *)IntermediateBase__OnStart,
    /* +0x068 onStop */ (void *)IntermediateBase__OnStop,
    /* +0x06C setFrameBound */ (void *)TaskCore__SetFrameBound,
    /* +0x070 playSound */ (void *)TaskCore__PlaySound,
    /* +0x074 onPadStart */ (void *)TaskCore__OnPadStart,
    /* +0x078 onPadConfirm */ GraphRoom__OnPadConfirm,
    /* +0x07C onPadCancel */ (void *)TaskCore__OnPadCancel,
    /* +0x080 onPadPrev */ (void *)TaskCore__OnPadPrev,
    /* +0x084 onPadNext */ (void *)TaskCore__OnPadNext,
    /* +0x088 slot88 */ NULL,
    /* +0x08C slot8C */ NULL,
    /* +0x090 confirmSlot */ (void *)TaskCore__ConfirmSlot,
    /* +0x094 exit */ (void *)TaskCore__Exit,
    /* +0x098 setExitCallback */ (void *)TaskCore__SetExitCallback,
    /* +0x09C setFadeInCallbackEnabled */ (void *)TaskCore__SetFadeInCallbackEnabled,
    /* +0x0A0 setFadeOutCallbackEnabled */ (void *)TaskCore__SetFadeOutCallbackEnabled,
    /* +0x0A4 setColors */ (void *)TaskCore__SetColors,
    /* +0x0A8 setFadeRate */ (void *)TaskCore__SetFadeRate,
    /* +0x0AC tickFadeInCallback */ (void *)TaskCore__TickFadeInCallback,
    /* +0x0B0 tickFadeIn */ (void *)TaskCore__TickFadeIn,
    /* +0x0B4 slotB4 */ NULL,
    /* +0x0B8 slotB8 */ NULL,
    /* +0x0BC slotBC */ NULL,
    /* +0x0C0 tickFadeOutCallback */ (void *)TaskCore__TickFadeOutCallback,
    /* +0x0C4 tickFadeOut */ (void *)TaskCore__TickFadeOut,
    /* +0x0C8 slotC8 */ NULL,
    /* +0x0CC slotCC */ NULL,
    /* +0x0D0 slotD0 */ NULL,
    /* +0x0D4 setSubHandle */ (void *)TaskCore__SetSubHandle,
    /* +0x0D8 setTarget */ (void *)GraphRoom__BuildGraphPoints,
    /* +0x0DC releaseTarget */ GraphRoom__ReleaseGraphPoints,
    /* +0x0E0 updateSlotElements */ GraphRoom__PopulateGraphPoints,
    /* +0x0E4 broadcastToSlots */ (void *)TaskCore__BroadcastToSlots,
    /* +0x0E8 findNextFreeSlot */ (void *)TaskCore__FindNextFreeSlot,
    /* +0x0EC findPrevFreeSlot */ (void *)TaskCore__FindPrevFreeSlot,
    /* +0x0F0 setActiveSlot */ (void *)TaskCore__SetActiveSlot,
    /* +0x0F4 getActiveSlot */ (void *)TaskCore__GetActiveSlot,
    /* +0x0F8 createSlotElements */ (void *)TaskCore__CreateSlotElements,
    /* +0x0FC releaseSlotElements */ (void *)TaskCore__ReleaseSlotElements,
    /* +0x100 refreshSlotView */ (void *)TaskCore__RefreshSlotView,
    /* +0x104 broadcastToSlotElements */ (void *)TaskCore__BroadcastToSlotElements,
    /* +0x108 beginElementScroll */ (void *)TaskCore__BeginElementScroll,
    /* +0x10C commitElementScroll */ (void *)TaskCore__CommitElementScroll,
    /* +0x110 cancelElementScroll */ (void *)TaskCore__CancelElementScroll,
    /* +0x114 advanceSlotCursor */ (void *)TaskCore__AdvanceSlotCursor,
    /* +0x118 retreatSlotCursor */ (void *)TaskCore__RetreatSlotCursor,
    /* +0x11C setSlotCursor */ (void *)TaskCore__SetSlotCursor,
    /* +0x120 getActiveItemCursor */ (void *)TaskCore__GetActiveItemCursor,
    /* +0x124 tickHighlight */ GraphRoom__TickHighlight,
};
/* clang-format on */

GraphRoom *New_GraphRoom(struct DreamSys *dreamSys) {
    GraphRoom *obj = BMemPMgrAlloc(sizeof(GraphRoom));
    if (obj != NULL) {
        GetGraphRoomMethods()->ctor(obj, dreamSys);
        return obj;
    }
    return NULL;
}

extern char sGraphSoundBankPath[];

void GraphRoom__GraphRoom(GraphRoom *self, struct DreamSys *dreamSys) {
    GetTaskCoreMethods()->ctor((TaskCore *)self, NULL, sGraphSoundBankPath, NULL);
    self->methods = GetGraphRoomMethods();
    ((VabStreamObj *)self->sound)->methods->setPitchOffset((VabStreamObj *)self->sound, -1); /* TaskCore::sound is a VabStreamObj */
    self->dreamSys = dreamSys;
    self->methods->setTarget(self, NULL);
    ((GraphRoomResetCallFn)self->methods->resetCounters)(self, dreamSys);
}

extern char sGraphTimPath[];

void GraphRoom__Reset(GraphRoom *self) {
    self->fadeRate = 5;
    self->maxPackets = 400;
    self->methods->setSubHandle(self, sGraphTimPath, NULL);
    self->methods->setFrameBound(self, 10);
}

void GraphRoom__Update(GraphRoom *self, BasicClass *sender, s32 event) {
    GetTaskCoreMethods()->update((TaskCore *)self, sender, event);
    if (self->inputMode == 1) {
        DreamSaveBlock *save =
            (DreamSaveBlock *)self->dreamSys->methods->getSaveBlock(self->dreamSys, 0);
        if (save->currentYear != 0 || save->currentDay != 0) {
            self->points[0]->methods->setDisplay(self->points[0], self->frameCounter & 1);
        }
    }
    self->methods->tickHighlight(self);
}

void GraphRoom__OnPadConfirm(GraphRoom *self) {
    if (self->scored == 0) {
        self->methods->playSound(self, TASKCORE_TONE_BUTTON);
        self->methods->exit(self);
    }
}

/* A graph point's colour is New_BoxFill's colour argument, a ColorRgb
 * (include/box_fill.h). */
extern s32 sGraphPointSize[2];
extern ColorRgb sGraphPointNewestColor;
extern ColorRgb sGraphPointBaseColor;

void GraphRoom__BuildGraphPoints(GraphRoom *self) {
    ColorRgb rgb;
    s32 i;

    self->points[0] = New_BoxFill(sGraphPointSize, &sGraphPointNewestColor, 0);
    rgb = sGraphPointBaseColor;
    for (i = 1; i < ARRAY_COUNT(self->points); i++) {
        s32 step;

        self->points[i] = New_BoxFill(sGraphPointSize, &rgb, 0);
        step = 1;
        if (i < 7) {
            step = 20;
        }
        rgb.r -= step;
        rgb.g -= step;
        rgb.b -= step;
    }
    self->matchedDayIndices = BMemPMgrAlloc(GRAPH_SCORE_MOOD_COUNT * sizeof(s8));
}

void GraphRoom__ReleaseGraphPoints(GraphRoom *self) {
    s32 i;

    BMemPMgrFree(self->matchedDayIndices);
    for (i = 0; i < ARRAY_COUNT(self->points); i++) {
        self->points[i]->methods->release(self->points[i]);
    }
    GetTaskCoreMethods()->releaseTarget((TaskCore *)self);
}

s32 GraphRoom__Init(GraphRoom *self, IntermediateBaseInitArgs *args, s32 mode) {
    s32 result;
    GetTaskCoreMethods()->init((TaskCore *)self, args, mode);
    result = GRAPHROOM_RESULT_SCORED;
    if (self->scored == 0) {
        result = self->result;
    }
    return result;
}

void GraphRoom__PopulateGraphPoints(GraphRoom *self, void *parent) {
    DreamSaveBlock *save;
    s32 count;
    s32 i;
    s32 day;
    s32 haveNewest;
    BoxFillPos point;
    BoxFillPos firstPoint;

    GetTaskCoreMethods()->updateSlotElements((TaskCore *)self, parent);
    save = (DreamSaveBlock *)self->dreamSys->methods->getSaveBlock(self->dreamSys, 0);
    self->scored = GraphRoom__ScoreDayLog(self, save);

    haveNewest = 0;
    if (save->currentYear != 0) {
        count = ARRAY_COUNT(self->points);
    } else {
        count = save->currentDay;
        if (count > ARRAY_COUNT(self->points)) {
            count = ARRAY_COUNT(self->points);
        }
    }

    day = save->currentDay - 1;
    for (i = 0; i < count; i++, day--) {
        s8 dx, dy;
        s32 ndy;

        if (day < 0) {
            day = DAYS_PER_YEAR - 1;
        }
        /* MATCHING: indexed twice; a MoodGraphPoint pointer to the day does not match */
        dx = save->moodPreviousDays[day].axis.dynamic;
        point.x = dx * GRAPH_PIXELS_PER_MOOD - GRAPH_POINT_SIZE / 2;
        dy = save->moodPreviousDays[day].axis.upper;
        ndy = -dy;
        point.y = ndy * GRAPH_PIXELS_PER_MOOD - GRAPH_POINT_SIZE / 2;

        if (i == 0) {
            firstPoint = point;
            haveNewest = 1;
        } else {
            self->points[i]->methods->attachAbsolute(self->points[i], parent, &point, 0);
        }
    }

    if (haveNewest) {
        self->points[0]->methods->attachAbsolute(self->points[0], parent, &firstPoint, 0);
    }
}

/* The four moods ScoreDayLog looks for, {dynamic, upper}. */
MoodGraphPoint sGraphScoreMoods[GRAPH_SCORE_MOOD_COUNT] = {{{-1, 1}}, {{1, 1}}, {{0, 0}}, {{0, -3}}};

/* Whether every sGraphScoreMoods entry appears among the plotted days (the
 * window PopulateGraphPoints walks, newest first), recording in
 * matchedDayIndices the oldest dot holding each. Fails at once when
 * graphScored is set, and sets it on success. */
/* MATCHING: the targets/days caches, the dead else and the chained assignment take the
 * table's address on every pass; indexing sGraphScoreMoods takes it once, before the loops */
s32 GraphRoom__ScoreDayLog(GraphRoom *self, DreamSaveBlock *log) {
    u32 i; /* MATCHING: unsigned, as retail tests it; a signed counter compiles a signed test */
    MoodGraphPoint *days;
    s32 dot;
    MoodGraphPoint *targets;
    s32 day;
    s32 matches;
    s32 limit;

    if (log->graphScored != 0) {
        return 0;
    }

    if (log->currentYear != 0) {
        limit = ARRAY_COUNT(self->points);
    } else {
        limit = log->currentDay;
        if (limit > ARRAY_COUNT(self->points)) {
            limit = ARRAY_COUNT(self->points);
        }
    }

    for (i = 0; i < GRAPH_SCORE_MOOD_COUNT; i++) {
        matches = 0;
        day = log->currentDay - 1;
        for (dot = 0; dot < limit; dot++) {
            if (day < 0) {
                day = DAYS_PER_YEAR - 1;
            } else {
                targets = sGraphScoreMoods;
            }
            targets = (days = sGraphScoreMoods);
            days = log->moodPreviousDays;
            if (targets[i].value == days[day].value) {
                self->matchedDayIndices[i] = dot;
                matches++;
            }
            day--;
        }
        if (matches == 0) {
            goto fail; /* MATCHING: to the shared return 0; returning here lays the loop out differently */
        }
    }

    log->graphScored = 1;
    self->highlightCount = 0;
    return 1;

fail:
    return 0;
}

extern ColorRgb sGraphPointHighlightColor;

void GraphRoom__TickHighlight(GraphRoom *self) {
    if (self->scored != 0) {
        /* MATCHING: frameCounter tested unsigned, as retail does; its base class declares it s32 */
        if ((u32)self->frameCounter >= 31) {
            if (self->highlightCount < GRAPH_SCORE_MOOD_COUNT) {
                if (((u32)self->frameCounter % 24) == 0) {
                    s8 dot = self->matchedDayIndices[self->highlightCount];
                    self->points[dot]->methods->setColor(self->points[dot], 1, &sGraphPointHighlightColor);
                    self->highlightCount += 1;
                }
            }
        }
    }
}

GraphRoomMethods *GetGraphRoomMethods(void) {
    return &gGraphRoomMethods;
}
