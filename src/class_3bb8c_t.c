/*
 * class_3bb8c_t -- functions 96..112 of the 113-function `class_3bb8c_n`
 * remainder, 0x48738..0x48F74 (vram 0x80057F38..0x80058774).  Carved
 * MID-round 17 (2026-09-04) to re-staff a runner whose own unit was
 * exhausted.  This is the LAST slice of the class_3bb8c block.
 *
 * EXPECT THIS SLICE TO SPAN MORE THAN ONE CLASS.  It is cut at ROM
 * addresses, not class boundaries.  Identify each with tools/classtable.py.
 * It holds two classes:
 *
 * - Four empty leaves plus the table getter (VariantSprite__Update,
 *   VariantSprite__NoOpSlotBC/C0/C4, GetVariantSpriteMethods) of the
 *   unrelated VariantSprite (include/VariantSprite.h; its ctor is in
 *   `class_3bb8c_p`, two more methods in `class_3bb8c_q`).
 * - The WHOLE of `GraphRoom` (round 75 name; table `gGraphRoomMethods`,
 *   73 slots), a TaskCore subclass, unified in include/GraphRoom.h (track 4,
 *   round 87; the header's banner has the slots, fields and evidence).
 *   This unit owns the entire class: allocator, ctor, every override,
 *   ScoreDayLog and the getter.
 *
 * `GraphRoom`'s identity (round 75, track 3 naming pass; tier B -- the
 * MECHANICS below are certain, the in-game name is a strong but unconfirmed
 * read): `GraphRoom__Reset` sets the literal texture string
 * `"ETC\HGRAPH.TIM"` as the sub-handle. The class owns a 100-entry array of
 * small coloured `New_BoxFill` point objects (`points`) built by
 * `BuildGraphPoints` and positioned by `PopulateGraphPoints` from a
 * backwards walk of the DreamSys's 365-entry mood ring
 * (`DreamSaveBlock::moodPreviousDays`, reached through the save block
 * `dreamSys`'s GetSaveBlock returns) -- each day's two signed bytes become
 * an `{x, y}` point handed to a point's `attachAbsolute`. `ScoreDayLog`
 * separately scans that same ring for four fixed mood targets
 * (`gGraphScoreMoods`) and records, per target, the dot index it last matched at;
 * `TickHighlight` later highlights the matching point. Together this is
 * the in-game graph screen that plots mood history as coloured dots.
 * Round 87 correction: the ring IS DreamSys's `moodPreviousDays` -- the
 * ctor's argument is GameApplication's dreamSys, and the record's offsets are
 * DreamSys's fields relative to saveMagic (see DreamSaveBlock below). The
 * earlier "not DreamSys, the offsets don't line up" compared them against
 * the start of DreamSys rather than the save block.
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

/* GraphRoom's object, table and methods: include/GraphRoom.h (track 4,
 * round 87). The base implementations are reached through
 * Get_vtable_TaskCore() with `self` upcast. */

/* What DreamSys__GetSaveBlock (the DreamSys's +0x1B0) returns: &saveMagic,
 * the 0x700-byte save block. This record reads it from there; the offsets
 * are DreamSys's own fields relative to saveMagic (DreamSys +0x178):
 * currentYear, currentDay, moodPreviousDays[365] (include/DreamSys.h). The
 * round-24 reading of this record ("a separate day-log object, not
 * DreamSys") predates knowing who passes the ctor's argument:
 * GameApplication__PollGraphRoomStatus passes its dreamSys.
 *
 * +0x467 is DreamSys +0x5DF, the last byte of DreamSys's
 * unknown_values_0x5d8[8]: ScoreDayLog fails once it is set and sets it on
 * success, so the graph's highlight runs once per save. */
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
    GraphRoom *obj = BMemPMgrAlloc(0x244);
    if (obj != NULL) {
        GetGraphRoomMethods()->ctor(obj, dreamSys);
        return obj;
    }
    return NULL;
}

extern char sGraphSoundBankPath[];

void GraphRoom__GraphRoom(GraphRoom *self, struct DreamSys *dreamSys) {
    Get_vtable_TaskCore()->ctor((TaskCore *)self, 0, sGraphSoundBankPath, 0);
    self->methods = GetGraphRoomMethods();
    ((VabStreamObj *)self->sound)->methods->setPitchOffset((VabStreamObj *)self->sound, -1); /* TaskCore::sound is a VabStreamObj */
    self->dreamSys = dreamSys;
    self->methods->setTarget(self, 0);
    ((GraphRoomResetCallFn)self->methods->resetCounters)(self, dreamSys);
}

extern char sGraphTimPath[];

void GraphRoom__Reset(GraphRoom *self) {
    self->fadeRate = 5;
    self->unk2C = 0x190;
    self->methods->setSubHandle(self, sGraphTimPath, 0);
    self->methods->setFrameBound(self, 0xA);
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
        self->methods->playSound(self, 0x10);
        self->methods->refreshViewValue(self);
    }
}

/* A graph point's colour, New_BoxFill's colour argument: BoxFill__SetColor
 * copies the three bytes into the box's GsBOXF r, g, b. Not Sony's CVECTOR,
 * which is four bytes and unsigned.
 * MATCHING: signed, and exactly three bytes -- the whole-struct copy is three
 * lb/sb pairs. */
typedef struct GraphPointColor {
    s8 r;
    s8 g;
    s8 b;
} GraphPointColor;

extern s32 gGraphPointSize[2];
extern GraphPointColor gGraphPointNewestColor;
extern GraphPointColor gGraphPointBaseColor;

void GraphRoom__BuildGraphPoints(GraphRoom *self) {
    GraphPointColor rgb;
    s32 i;

    self->points[0] = New_BoxFill(gGraphPointSize, &gGraphPointNewestColor, 0);
    rgb = gGraphPointBaseColor;
    for (i = 1; i < 100; i++) {
        s32 step;

        self->points[i] = New_BoxFill(gGraphPointSize, &rgb, 0);
        step = 1;
        if (i < 7) {
            step = 0x14;
        }
        rgb.r -= step;
        rgb.g -= step;
        rgb.b -= step;
    }
    self->matchedDayIndices = BMemPMgrAlloc(4);
}

extern void BMemPMgrFree(void *arg);

void GraphRoom__ReleaseGraphPoints(GraphRoom *self) {
    s32 i;

    BMemPMgrFree(self->matchedDayIndices);
    for (i = 0; i < 100; i++) {
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
        count = 100;
    } else {
        count = save->currentDay;
        if (count >= 0x65) {
            count = 100;
        }
    }

    day = save->currentDay - 1;
    for (i = 0; i < count; i++, day--) {
        s8 dx, dy;
        s32 ndy;

        if (day < 0) {
            day = 0x16C;
        }
        dx = save->moodPreviousDays[day].axis.dynamic;
        point.x = dx * 10 - 5;
        dy = save->moodPreviousDays[day].axis.upper;
        ndy = -dy;
        point.y = ndy * 10 - 5;

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

/* Four halfword targets, 0x01FF/0x0101/0x0000/0xFD00 -- exactly the i < 4
 * bound below, which is why the loop count is the table's length and not a
 * coincidence. */
extern MoodGraphPoint gGraphScoreMoods[4];

/* Round 41 (2026-09-14): matched from a permuter-found lead. `targets` and `days`
 * are LOCAL pointer caches of gGraphScoreMoods and log->moodPreviousDays respectively -- not
 * because retail's semantics need them (both globals are re-derivable
 * without a temporary), but because caching them THIS WAY is what makes
 * cc1 2.6.3 stop strength-reducing gGraphScoreMoods[i] into a pointer induction
 * variable hoisted across the outer loop (see the match report for the
 * full derivation). The `else { targets = gGraphScoreMoods; }` branch below and the
 * `targets = (days = gGraphScoreMoods);` chained assignment are BOTH semantically
 * inert -- targets is unconditionally overwritten with the same value either
 * way -- but removing either one measurably regresses the codegen (round
 * 41 confirmed both empirically, byte-exact with them, off by dozens of
 * words without). Do not "simplify" this without re-running
 * ./build-and-verify.sh. */
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
        limit = 100;
    } else {
        limit = log->currentDay;
        if (limit > 100) {
            limit = 100;
        }
    }

    for (i = 0; i < 4; i++) {
        matches = 0;
        day = log->currentDay - 1;
        for (dot = 0; dot < limit; dot++) {
            if (day < 0) {
                day = 0x16C;
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

extern GraphPointColor gGraphPointHighlightColor;

void GraphRoom__TickHighlight(GraphRoom *self) {
    if (self->scored != 0) {
        if ((u32)self->frameCounter >= 0x1F) {
            if (self->highlightCount < 4) {
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
