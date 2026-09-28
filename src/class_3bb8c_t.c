/*
 * class_3bb8c_t -- the graph room, and the tail of VariantSprite.
 *
 * - VariantSprite (include/VariantSprite.h): four empty methods and the
 *   table getter. Its ctor is in class_3bb8c_k, two more methods in
 *   class_3bb8c_q.
 * - GraphRoom (include/GraphRoom.h, whose banner has the slots and fields),
 *   a TaskCore subclass, whole: allocator, ctor, every override, ScoreDayLog
 *   and the getter.
 *
 * GraphRoom loads "ETC\HGRAPH.TIM" and plots up to 100 days of the
 * DreamSys's mood ring (moodPreviousDays, read from the save block
 * DreamSys__GetSaveBlock returns), newest first, as 10-pixel BoxFill dots:
 * a day's two signed mood bytes, times 10, are its dot's centre, the upper
 * axis pointing up the screen. The newest dot is red and blinks while
 * inputMode is 1; the others fade from white. ScoreDayLog checks, once per
 * save, that four fixed moods (gGraphScoreMoods) all appear among the
 * plotted days, and TickHighlight then turns each one's dot green, one
 * every 24 frames once frameCounter passes 30.
 */
#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "GraphRoom.h"
#include "BoxFill.h"
#include "DreamSys.h"
#include "VariantSprite.h"
#include "VabStreamObj.h"

extern void *BMemPMgrAlloc(s32 size);

/* gGraphScoreMoods' length: ScoreDayLog's targets, one matchedDayIndices
 * byte and one highlight each. */
#define GRAPH_SCORE_MOOD_COUNT 4

/* A plotted dot's side, gGraphPointSize's {10, 10}: PopulateGraphPoints
 * subtracts half of it so each dot is centred on its mood. */
#define GRAPH_POINT_SIZE 10

/* Screen pixels per step of a mood axis (PopulateGraphPoints). */
#define GRAPH_PIXELS_PER_MOOD 10

/* VariantSprite's (include/VariantSprite.h) four empty leaves, `jr $ra; nop`
 * (splat matched them itself), and its table getter. VariantSprite__Update is
 * the +0x098 update override of Sprite__Update, typed as that slot; the
 * other three occupy the class's own slots +0x0BC/+0x0C0/+0x0C4, which
 * nothing calls. */
void VariantSprite__Update(VariantSprite *self, void *sender, s32 event) {}

void VariantSprite__NoOpSlotBC(void) {}

void VariantSprite__NoOpSlotC0(void) {}

void VariantSprite__NoOpSlotC4(void) {}

VariantSpriteMethods *GetVariantSpriteMethods(void) {
    return &gVariantSpriteMethods;
}

/* GraphRoom's object, table and methods: include/GraphRoom.h. The base
 * implementations are reached through Get_vtable_TaskCore() with `self`
 * upcast. */

/* What DreamSys__GetSaveBlock (DreamSys +0x1B0) returns: &saveMagic, the
 * 0x700-byte save block, viewed from there. The fields are DreamSys's own
 * (include/DreamSys.h), each at its DreamSys offset less saveMagic's
 * +0x178; graphScored is DreamSys +0x5DF, the last byte of
 * unknown_values_0x5d8[8]. */
typedef struct DreamSaveBlock {
    u8 pad00[0x4];
    /* +0x004 */ s32 currentYear; /* nonzero: the ring is full, plot all 100 days */
    /* +0x008 */ s32 currentDay;  /* days logged this year; the ring's write cursor */
    u8 pad0C[0x18 - 0xC];
    /* +0x018 */ MoodGraphPoint moodPreviousDays[DAYS_PER_YEAR]; /* walked backwards from currentDay - 1 */
    u8 pad2F2[0x467 - 0x2F2];
    /* +0x467 */ s8 graphScored; /* set once ScoreDayLog's scan has succeeded */
} DreamSaveBlock;

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
    Get_vtable_TaskCore()->ctor((TaskCore *)self, NULL, sGraphSoundBankPath, NULL);
    self->methods = GetGraphRoomMethods();
    ((VabStreamObj *)self->sound)->methods->setPitchOffset((VabStreamObj *)self->sound, -1); /* TaskCore::sound is a VabStreamObj */
    self->dreamSys = dreamSys;
    self->methods->setTarget(self, NULL);
    ((GraphRoomResetCallFn)self->methods->resetCounters)(self, dreamSys);
}

extern char sGraphTimPath[];

void GraphRoom__Reset(GraphRoom *self) {
    self->fadeRate = 5;
    self->unk2C = 400;
    self->methods->setSubHandle(self, sGraphTimPath, NULL);
    self->methods->setFrameBound(self, 10);
}

void GraphRoom__Update(GraphRoom *self, BasicClass *sender, s32 event) {
    Get_vtable_TaskCore()->update((TaskCore *)self, sender, event);
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
        self->methods->playSound(self, 1 << 4); /* VAB program 1, tone 0 */
        self->methods->refreshViewValue(self);
    }
}

/* A graph point's colour is New_BoxFill's colour argument, a BoxFillRgb
 * (include/BoxFill.h).
 * MATCHING: signed, and exactly three bytes -- the whole-struct copy of `rgb`
 * below is three lb/sb pairs. */
extern s32 gGraphPointSize[2];
extern BoxFillRgb gGraphPointNewestColor;
extern BoxFillRgb gGraphPointBaseColor;

void GraphRoom__BuildGraphPoints(GraphRoom *self) {
    BoxFillRgb rgb;
    s32 i;

    self->points[0] = New_BoxFill(gGraphPointSize, &gGraphPointNewestColor, 0);
    rgb = gGraphPointBaseColor;
    for (i = 1; i < ARRAY_COUNT(self->points); i++) {
        s32 step;

        self->points[i] = New_BoxFill(gGraphPointSize, &rgb, 0);
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

extern void BMemPMgrFree(void *arg);

void GraphRoom__ReleaseGraphPoints(GraphRoom *self) {
    s32 i;

    BMemPMgrFree(self->matchedDayIndices);
    for (i = 0; i < ARRAY_COUNT(self->points); i++) {
        self->points[i]->methods->release(self->points[i]);
    }
    Get_vtable_TaskCore()->releaseTarget((TaskCore *)self);
}

s32 GraphRoom__Init(GraphRoom *self, IntermediateBaseInitArgs *args, s32 mode) {
    s32 result;
    Get_vtable_TaskCore()->init((TaskCore *)self, args, mode);
    result = 2;
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

    Get_vtable_TaskCore()->updateSlotElements((TaskCore *)self, parent);
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
        /* MATCHING: indexed twice; through a `MoodGraphPoint *` to the day,
         * cc1 adds the array's +0x018 to the pointer first. */
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
extern MoodGraphPoint gGraphScoreMoods[GRAPH_SCORE_MOOD_COUNT];

/* Whether every gGraphScoreMoods entry appears among the plotted days (the
 * window PopulateGraphPoints walks, newest first), recording in
 * matchedDayIndices the oldest dot holding each. Fails at once when
 * graphScored is set, and sets it on success.
 * MATCHING: the `targets`/`days` caches, the dead else branch and the
 * chained assignment are all inert; without any one, cc1 strength-reduces
 * gGraphScoreMoods[i] into a pointer hoisted across the outer loop. */
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
                targets = gGraphScoreMoods;
            }
            targets = (days = gGraphScoreMoods);
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

extern BoxFillRgb gGraphPointHighlightColor;

void GraphRoom__TickHighlight(GraphRoom *self) {
    if (self->scored != 0) {
        if ((u32)self->frameCounter >= 31) {
            if (self->highlightCount < GRAPH_SCORE_MOOD_COUNT) {
                if (((u32)self->frameCounter % 24) == 0) {
                    s8 dot = self->matchedDayIndices[self->highlightCount];
                    self->points[dot]->methods->setColor(self->points[dot], 1, &gGraphPointHighlightColor);
                    self->highlightCount += 1;
                }
            }
        }
    }
}

GraphRoomMethods *GetGraphRoomMethods(void) {
    return &gGraphRoomMethods;
}
