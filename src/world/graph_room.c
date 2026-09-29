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

/* The four moods ScoreDayLog looks for, as (dynamic, upper): (-1, 1),
 * (1, 1), (0, 0), (0, -3). */
extern MoodGraphPoint sGraphScoreMoods[GRAPH_SCORE_MOOD_COUNT];

/* Whether every sGraphScoreMoods entry appears among the plotted days (the
 * window PopulateGraphPoints walks, newest first), recording in
 * matchedDayIndices the oldest dot holding each. Fails at once when
 * graphScored is set, and sets it on success. */
/* MATCHING: the targets/days caches, the dead else and the chained assignment take the
 * table's address on every pass; indexing sGraphScoreMoods takes it once, before the loops */
s32 GraphRoom__ScoreDayLog(GraphRoom *self, DreamSaveBlock *log) {
    u32 i;
    MoodGraphPoint *days;
    s32 dot;
    MoodGraphPoint *targets;
    s32 day;
    s32 matches;
    s32 limit;

    if (log->graphScored != 0) {
        goto fail;
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
            goto fail;
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
