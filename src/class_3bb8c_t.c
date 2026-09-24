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
 * - Four leaf overrides plus a getter (func_80057F38/40/48/50, func_80057F58)
 *   of the STILL-UNCARVED, unrelated sibling class whose table is
 *   `D_800879C4` (49 slots; its ctor and remaining slots live in the
 *   neighbouring `class_3bb8c_p` unit, which carries its own independent
 *   local view). None of these five functions has enough of a body to name
 *   past its own address; see the naming pass note above each.
 * - The WHOLE of `GraphRoomObj` (round 75 name; table `gGraphRoomMethods`,
 *   73 slots, resolved via `tools/classtable.py`), a `TaskCoreObj` subclass
 *   that is itself a derived override of `Obj86B60`/`gClass86B60Methods`
 *   (`include/code_2cc8c.h`). This unit owns the entire class -- ctor, dtor
 *   and every slot referenced from within it are all in this file.
 *
 * `GraphRoomObj`'s identity (round 75, track 3 naming pass; tier B -- the
 * MECHANICS below are certain, the in-game name is a strong but unconfirmed
 * read): `GraphRoomObj__InitDisplay` loads the literal texture string
 * `"ETC\HGRAPH.TIM"`. The class owns a 100-entry array of small coloured
 * `New_ClassEAC0` point objects (`points`) built by `BuildGraphPoints` and
 * positioned by `PopulateGraphPoints` from a backwards walk of a 365-entry
 * day-type ring (`DayLog::days`, reached through `dayLog`) -- each day's
 * two signed bytes become a `{x, y}` point handed to a point's own
 * `setPosition` slot. `ScoreDayLog` separately scans that same ring for
 * four fixed day-type targets (`D_80087BD4`) and records, per target, the
 * most recent day it occurred; `TickHighlight` later highlights the
 * matching point once per in-game day-ish tick. Together this is the
 * in-game "Graph Room" screen that plots dream-type history as a ring of
 * coloured dots -- but `DayLog` is explicitly NOT `DreamSys`'s own mood-graph
 * state (`include/DreamSys.h`'s `MoodGraphPoint`/`MoodGraphContributor`):
 * the offsets don't line up, so this is a separate day-log object with its
 * own copy, not DreamSys under another name (kept LOCAL to this unit; do
 * not include DreamSys.h to chase the resemblance, see DayLog's own
 * comment below).
 */
#include "common.h"

/* The class allocated by this unit's own New_GraphRoomObj, table D_800879C4
 * (49 slots, resolved via tools/classtable.py). Its ctor (D800879C4__D800879C4)
 * and its own funcs 80057F38/40/48/50 live in the neighbouring
 * `class_3bb8c_p` unit (round 2026-09-04 earlier this round), which
 * already carries its own local view of this table
 * (`D_800879C4Methods`/`D_800879C4Obj` in that file). This unit's own
 * view is kept separate per the multiple-independent-local-views
 * convention -- func_80057F58 itself needs no fields, only the address.
 * Round 75 naming pass: this class is still uncarved and unrelated to
 * GraphRoomObj below, so it has no name past its address; the four leaf
 * overrides below are empty bodies (`jr $ra; nop`, no arguments visible)
 * and give no evidence of purpose beyond "does nothing" -- kept `func_`. */
typedef struct D_800879C4Table D_800879C4Table;
extern D_800879C4Table D_800879C4;

extern void *BMemPMgrAlloc(s32 size);

typedef struct GraphRoomObj GraphRoomObj;

/* This unit's own local view of the shared base-class table returned by
 * Get_vtable_TaskCore() (a plain no-argument getter, established elsewhere --
 * e.g. include/code_2c054.h, include/class_3bb8c.h -- as returning
 * &gTaskCoreMethods). Only the slots this unit's own functions dispatch
 * through are typed, per the project's "per-call-site signature"
 * convention (multiple units already carry independent local views of
 * this same table with different slot arities). Slot names past +0x008
 * match the field names include/code_2c054.h's own independent view of
 * this same table already uses, for cross-unit readability; +0x008 is
 * renamed `ctor` here since `tools/classtable.py gTaskCoreMethods` resolves
 * it to `TaskCoreObj__TaskCoreObj`, an already-established real name. */
typedef struct TaskCoreBaseTable {
    u8 pad00[0x8];
    /* +0x008, called by this unit's own GraphRoomObj__GraphRoomObj as (self, 0, str,
     * 0). Resolves to TaskCoreObj__TaskCoreObj (tools/classtable.py gTaskCoreMethods). */
    void (*ctor)(GraphRoomObj *self, s32 arg1, char *str, s32 arg2);
    u8 pad0C[0x44 - 0xC];
    /* +0x044, called by this unit's own GraphRoomObj__func_80058390 as (self, arg1,
     * arg2). Not this unit's own function; still func_8003C1DC elsewhere. */
    void (*slot44)(GraphRoomObj *self, void *arg1, void *arg2);
    u8 pad48[0x5C - 0x48];
    /* +0x05C, called by this unit's own GraphRoomObj__UpdateFromLog as (self, arg1,
     * arg2). Not this unit's own function; still func_8003C51C elsewhere. */
    void (*slot5C)(GraphRoomObj *self, void *arg1, void *arg2);
    u8 pad60[0xDC - 0x60];
    /* +0x0DC, called by this unit's own GraphRoomObj__Destroy as (self) -- the
     * base-class dtor step. Not this unit's own function; still func_8003D050
     * elsewhere. */
    void (*slotDC)(GraphRoomObj *self);
    /* +0x0E0, called by this unit's own GraphRoomObj__PopulateGraphPoints as (self, arg1) --
     * the FIRST thing that function does, before touching anything else
     * (round 19). Not this unit's own function; still func_8003D194 elsewhere. */
    void (*slotE0)(GraphRoomObj *self, void *arg1);
} TaskCoreBaseTable;
extern TaskCoreBaseTable *Get_vtable_TaskCore(void);

/* Round 75 naming pass: no body to read past `jr $ra; nop` -- splat matched
 * these itself. They are D_800879C4's own leaf overrides
 * (tools/classtable.py: +0x098/+0x0BC/+0x0C0/+0x0C4), an unrelated,
 * still-uncarved class, so even the tier-C `Class__func_xxxxx` form does
 * not apply (no confirmed class name to prefix with). Kept bare `func_`. */
void func_80057F38(void) {
}

void func_80057F40(void) {
}

void func_80057F48(void) {
}

void func_80057F50(void) {
}

/* Round 75 naming pass: plain no-argument getter for D_800879C4, same
 * "no class name yet" reasoning as the four leaves above. Kept bare `func_`
 * rather than a guessed class prefix. */
D_800879C4Table *func_80057F58(void) {
    return &D_800879C4;
}

/* The class allocated below, table gGraphRoomMethods (73 slots, resolved via
 * tools/classtable.py). This unit owns the whole class -- ctor, dtor and
 * every slot referenced from within it are all in this file. Only the
 * fields/slots each function actually touches are typed; the rest stay
 * opaque so the struct keeps the right size without requiring every
 * method to be named up front. Slot names below are round 75's naming
 * pass: `ctor`/`postConstruct`/`loadTexture`/`tick` are tier B (mechanics
 * clear from the call site); the rest stay `slotNN`, evidence too thin to
 * name. */
typedef struct GraphRoomMethods {
    u8 pad00[0x8];
    /* +0x008, this unit's own ctor (GraphRoomObj__GraphRoomObj). */
    GraphRoomObj *(*ctor)(GraphRoomObj *self, void *arg1);
    u8 pad0C[0x40 - 0xC];
    /* +0x040, called by this unit's own GraphRoomObj__GraphRoomObj as (self, arg1) --
     * a TAIL CALL, its return value forwarded as GraphRoomObj__GraphRoomObj's own.
     * Same "ctor ends by calling another of its own class's slot +0x040"
     * shape, at the SAME offset, as the sibling class_3bb8c_p's own
     * D800879C4Methods::postConstruct -- named to match. */
    void *(*postConstruct)(GraphRoomObj *self, void *arg1);
    u8 pad44[0x6C - 0x44];
    /* +0x06C, called by this unit's own GraphRoomObj__InitDisplay as (self, flag). */
    void (*slot6C)(GraphRoomObj *self, s32 arg1);
    /* +0x070, called by this unit's own GraphRoomObj__HandleUnscored as (self, size). */
    void (*slot70)(GraphRoomObj *self, s32 arg1);
    u8 pad74[0x94 - 0x74];
    /* +0x094, called by this unit's own GraphRoomObj__HandleUnscored as (self). */
    void (*slot94)(GraphRoomObj *self);
    u8 pad98[0xD4 - 0x98];
    /* +0x0D4, called by this unit's own GraphRoomObj__InitDisplay as
     * (self, "ETC\HGRAPH.TIM", 0) -- a resource-path string, so this reads
     * as the texture loader for the room's own graph background. */
    void (*loadTexture)(GraphRoomObj *self, char *str, s32 arg2);
    /* +0x0D8, called by this unit's own GraphRoomObj__GraphRoomObj as (self, 0). */
    void (*slotD8)(GraphRoomObj *self, s32 arg1);
    u8 padDC[0x124 - 0xDC];
    /* +0x124, this unit's own GraphRoomObj__TickHighlight, called by
     * GraphRoomObj__UpdateFromLog as (self) on every log update. */
    void (*tick)(GraphRoomObj *self);
} GraphRoomMethods;
extern GraphRoomMethods *GetGraphRoomMethods(void);

/* This unit's own view of one entry of GraphRoomObj::points -- one of the
 * 100 small coloured `New_ClassEAC0`-allocated dots the graph plots. Only
 * the slots this unit's own functions dispatch through are typed. */
typedef struct GraphRoomPoint GraphRoomPoint;
typedef struct GraphRoomPointMethods {
    u8 pad00[0x4];
    /* +0x004, called by this unit's own GraphRoomObj__Destroy as (self) -- a
     * per-entry destructor, in a 100-iteration loop over `points`. */
    void (*destroy)(GraphRoomPoint *self);
    u8 pad08[0x60 - 0x8];
    /* +0x060, called by this unit's own GraphRoomObj__UpdateFromLog as
     * (self, self->elapsedHours & 1) -- an odd/even flag, purpose past
     * that not established. */
    void (*slot60)(GraphRoomPoint *self, s32 arg1);
    u8 pad64[0xB8 - 0x64];
    /* +0x0B8, called by this unit's own GraphRoomObj__TickHighlight as
     * (self, 1, &D_8008ABBC) -- fires the "this day matched"
     * visual highlight. */
    void (*highlight)(GraphRoomPoint *self, s32 arg1, void *arg2);
    u8 padBC[0xC4 - 0xBC];
    /* +0x0C4, called by this unit's own GraphRoomObj__PopulateGraphPoints as
     * (self, arg1, &point, 0), where `point` is a 2-word {x, y}-shaped
     * local (round 19) -- places this dot on the graph. */
    void (*setPosition)(GraphRoomPoint *self, void *arg1, s32 *point, s32 arg3);
} GraphRoomPointMethods;
struct GraphRoomPoint {
    GraphRoomPointMethods *methods;
};

/* Object pointed to by GraphRoomObj::unk48 -- only the one slot this
 * unit's own GraphRoomObj__GraphRoomObj dispatches through is typed.
 * Called with -1 right after the ctor sets its own vtable and before
 * anything else touches `self`, which reads as some kind of reset/detach
 * step, but that is not established evidence of what the object itself
 * is -- kept opaque. */
typedef struct GraphRoomUnk48Obj GraphRoomUnk48Obj;
typedef struct GraphRoomUnk48Methods {
    u8 pad00[0x9C];
    /* +0x09C, called by this unit's own GraphRoomObj__GraphRoomObj as (self, -1). */
    void (*slot9C)(GraphRoomUnk48Obj *self, s32 arg1);
} GraphRoomUnk48Methods;
struct GraphRoomUnk48Obj {
    GraphRoomUnk48Methods *methods;
};

/* Object pointed to by GraphRoomObj::dayLog -- passed in as this unit's
 * own ctor's `arg1` (GraphRoomObj__GraphRoomObj) and dispatched through by
 * GraphRoomObj__UpdateFromLog/GraphRoomObj__PopulateGraphPoints (both
 * still queued at slot +0x1B0). Only that one slot is typed. */
typedef struct DayLogObj DayLogObj;
/* Return type of DayLogMethods::getData.
 *
 * It is a DAY-LOG object, established by GraphRoomObj__ScoreDayLog (round 24): the two
 * fields GraphRoomObj__UpdateFromLog reads at +0x4/+0x8 are a mode flag and a live day
 * count, and +0x18 is a 365-entry halfword year ring whose length is fixed
 * by GraphRoomObj__ScoreDayLog's wrap constant (the index resets to 0x16C == 364 when
 * it goes negative, so 365 entries).  Extended ADDITIVELY -- +0x4 and +0x8
 * keep their offsets, so GraphRoomObj__UpdateFromLog's codegen is unaffected.
 *
 * LEAD, not a claim: a 365-entry log of 2-byte points is the shape of
 * `MoodGraphPoint moodPreviousDays[365]` in include/DreamSys.h, and the
 * `lh` accesses here are consistent with MoodGraphPoint being 2 bytes.  But
 * the OFFSETS do not line up -- DreamSys puts that array far deeper than
 * +0x18 -- so this is a different object keeping its own year log, not
 * DreamSys under another name.  Kept LOCAL to this unit; do not include
 * DreamSys.h to chase the resemblance, it would create header contention
 * this unit does not currently have. */
typedef struct DayLog {
    u8 pad00[0x4];
    /* +0x004, nonzero means "scan the full 100-day window regardless of how
     * many days are actually logged". */
    s32 fullScan;
    /* +0x008, days logged so far; also the ring's write cursor. */
    s32 dayCount;
    u8 pad0C[0x18 - 0xC];
    /* +0x018, the year ring, walked backwards from dayCount - 1. */
    s16 days[365];
    u8 pad2F2[0x467 - 0x2F2];
    /* +0x467, set once GraphRoomObj__ScoreDayLog's scan has succeeded. */
    s8 scored;
} DayLog;
typedef struct DayLogMethods {
    u8 pad00[0x1B0];
    /* +0x1B0, called by this unit's own GraphRoomObj__UpdateFromLog/GraphRoomObj__PopulateGraphPoints as
     * (self, 0). */
    DayLog *(*getData)(DayLogObj *self, s32 arg1);
} DayLogMethods;
struct DayLogObj {
    DayLogMethods *methods;
};

struct GraphRoomObj {
    GraphRoomMethods *methods;
    u8 pad04[0x1C - 0x4];
    /* +0x01C, read by this unit's own GraphRoomObj__TickHighlight (unsigned
     * comparisons -- `sltiu`, >= 0x1F and, elsewhere, % 24) -- reads as an
     * elapsed-hours counter (24 hours/day), gating the highlight tick. */
    u32 elapsedHours;
    u8 pad20[0x2C - 0x20];
    /* +0x02C, written by this unit's own GraphRoomObj__InitDisplay. */
    s32 unk_0x2C;
    u8 pad30[0x38 - 0x30];
    /* +0x038, read by this unit's own GraphRoomObj__func_80058390. */
    s32 unk_0x38;
    /* +0x03C, read by this unit's own GraphRoomObj__UpdateFromLog. */
    s32 unk_0x3C;
    u8 pad40[0x48 - 0x40];
    /* +0x048, read by this unit's own GraphRoomObj__GraphRoomObj. */
    GraphRoomUnk48Obj *unk48;
    u8 pad4C[0x84 - 0x4C];
    /* +0x084, written by this unit's own GraphRoomObj__InitDisplay. */
    s32 unk_0x84;
    u8 pad88[0xA4 - 0x88];
    /* +0x0A4, set by this unit's own ctor (GraphRoomObj__GraphRoomObj) to its own
     * `arg1`; dispatched through by GraphRoomObj__UpdateFromLog/GraphRoomObj__PopulateGraphPoints. */
    DayLogObj *dayLog;
    /* +0x0A8, a 100-entry array of `GraphRoomPoint *` -- the graph's own
     * coloured dots, built by this unit's own
     * GraphRoomObj__BuildGraphPoints, destroyed by GraphRoomObj__Destroy,
     * positioned by GraphRoomObj__PopulateGraphPoints and indexed by
     * GraphRoomObj__TickHighlight. */
    GraphRoomPoint *points[100];
    /* +0x238, GraphRoomObj__ScoreDayLog's own success/fail return, stashed
     * here by GraphRoomObj__PopulateGraphPoints; read by
     * GraphRoomObj__HandleUnscored/GraphRoomObj__func_80058390 and
     * GraphRoomObj__TickHighlight. */
    s32 scored;
    /* +0x23C, how many of the 4 ScoreDayLog targets have been highlighted
     * so far; read/written by this unit's own GraphRoomObj__TickHighlight
     * (unsigned comparison -- `sltiu`, < 4). */
    u32 highlightCount;
    /* +0x240, a 4-entry array (BMemPMgrAlloc(4) in BuildGraphPoints) of the
     * day-of-graph index each of ScoreDayLog's 4 targets matched at;
     * written by GraphRoomObj__ScoreDayLog, read by
     * GraphRoomObj__TickHighlight indexed by highlightCount. */
    s8 *matchedDayIndices;
};

void *New_GraphRoomObj(void *arg1) {
    void *obj = BMemPMgrAlloc(0x244);
    if (obj != NULL) {
        GetGraphRoomMethods()->ctor(obj, arg1);
        return obj;
    }
    return NULL;
}

extern char D_8001176C[];

void *GraphRoomObj__GraphRoomObj(GraphRoomObj *self, void *arg1) {
    Get_vtable_TaskCore()->ctor(self, 0, D_8001176C, 0);
    self->methods = GetGraphRoomMethods();
    self->unk48->methods->slot9C(self->unk48, -1);
    self->dayLog = arg1;
    self->methods->slotD8(self, 0);
    return self->methods->postConstruct(self, arg1);
}

extern char D_80011778[];

void GraphRoomObj__InitDisplay(GraphRoomObj *self) {
    self->unk_0x84 = 5;
    self->unk_0x2C = 0x190;
    self->methods->loadTexture(self, D_80011778, 0);
    self->methods->slot6C(self, 0xA);
}

void GraphRoomObj__UpdateFromLog(GraphRoomObj *self, void *arg1, void *arg2) {
    Get_vtable_TaskCore()->slot5C(self, arg1, arg2);
    if (self->unk_0x3C == 1) {
        DayLog *result = self->dayLog->methods->getData(self->dayLog, 0);
        if (result->fullScan != 0 || result->dayCount != 0) {
            self->points[0]->methods->slot60(self->points[0], self->elapsedHours & 1);
        }
    }
    self->methods->tick(self);
}

void GraphRoomObj__HandleUnscored(GraphRoomObj *self) {
    if (self->scored == 0) {
        self->methods->slot70(self, 0x10);
        self->methods->slot94(self);
    }
}

/* A 3-byte colour-ish triple, read/written strictly byte-for-byte in
 * DECLARATION order (round 19, verified against retail byte-for-byte --
 * the natural sequential order is what matches, no reordering needed).
 * Field names are a plausible RGB reading of a colour-cycling table
 * builder, not confirmed evidence; see this function's own match report. */
typedef struct D_8008ABB8Color {
    s8 r;
    s8 g;
    s8 b;
} D_8008ABB8Color;
extern u8 D_8008ABAC;
extern u8 D_8008ABB4;
extern D_8008ABB8Color D_8008ABB8;
extern GraphRoomPoint *New_ClassEAC0(void *a0, void *a1, s32 a2);

void GraphRoomObj__BuildGraphPoints(GraphRoomObj *self) {
    D_8008ABB8Color rgb;
    s32 i;

    self->points[0] = New_ClassEAC0(&D_8008ABAC, &D_8008ABB4, 0);
    rgb = D_8008ABB8;
    for (i = 1; i < 100; i++) {
        s32 dec;

        self->points[i] = New_ClassEAC0(&D_8008ABAC, &rgb, 0);
        dec = 1;
        if (i < 7) {
            dec = 0x14;
        }
        rgb.r -= dec;
        rgb.g -= dec;
        rgb.b -= dec;
    }
    self->matchedDayIndices = BMemPMgrAlloc(4);
}

extern void BMemPMgrFree(void *arg);

void GraphRoomObj__Destroy(GraphRoomObj *self) {
    s32 i;

    BMemPMgrFree(self->matchedDayIndices);
    for (i = 0; i < 100; i++) {
        self->points[i]->methods->destroy(self->points[i]);
    }
    Get_vtable_TaskCore()->slotDC(self);
}

s32 GraphRoomObj__func_80058390(GraphRoomObj *self, void *arg1, void *arg2) {
    s32 result;
    Get_vtable_TaskCore()->slot44(self, arg1, arg2);
    result = 2;
    if (self->scored == 0) {
        result = self->unk_0x38;
    }
    return result;
}

extern s32 GraphRoomObj__ScoreDayLog(GraphRoomObj *self, DayLog *arg1);

/* A 2-word {x, y}-shaped point, matching what this unit's own GraphRoomObj__PopulateGraphPoints
 * passes to GraphRoomPointMethods::setPosition (round 19). */
typedef struct Point2 {
    s32 x, y;
} Point2;

void GraphRoomObj__PopulateGraphPoints(GraphRoomObj *self, void *arg1) {
    DayLog *result;
    s32 count;
    s32 i;
    s32 idx;
    s32 flag;
    Point2 point;
    Point2 firstPoint;

    Get_vtable_TaskCore()->slotE0(self, arg1);
    result = self->dayLog->methods->getData(self->dayLog, 0);
    self->scored = GraphRoomObj__ScoreDayLog(self, result);

    flag = 0;
    if (result->fullScan != 0) {
        count = 100;
    } else {
        count = result->dayCount;
        if (count >= 0x65) {
            count = 100;
        }
    }

    idx = result->dayCount - 1;
    for (i = 0; i < count; i++, idx--) {
        s8 *p;
        s8 dx, dy;
        s32 ndy;

        if (idx < 0) {
            idx = 0x16C;
        }
        p = (s8 *)((u8 *)result + idx * 2);
        dx = p[0x18];
        point.x = dx * 10 - 5;
        dy = p[0x19];
        ndy = -dy;
        point.y = ndy * 10 - 5;

        if (i == 0) {
            firstPoint = point;
            flag = 1;
        } else {
            self->points[i]->methods->setPosition(self->points[i], arg1, (s32 *)&point, 0);
        }
    }

    if (flag) {
        self->points[0]->methods->setPosition(self->points[0], arg1, (s32 *)&firstPoint, 0);
    }
}

/* Four halfword targets, 0x01FF/0x0101/0x0000/0xFD00 -- exactly the i < 4
 * bound below, which is why the loop count is the table's length and not a
 * coincidence. */
extern s16 D_80087BD4[4];

/* Round 41 (2026-09-14): matched from a permuter-found lead. `p` and `days`
 * are LOCAL pointer caches of D_80087BD4 and log->days respectively -- not
 * because retail's semantics need them (both globals are re-derivable
 * without a temporary), but because caching them THIS WAY is what makes
 * cc1 2.6.3 stop strength-reducing D_80087BD4[i] into a pointer induction
 * variable hoisted across the outer loop (see the match report for the
 * full derivation). The `else { p = D_80087BD4; }` branch below and the
 * `p = (days = D_80087BD4);` chained assignment are BOTH semantically
 * inert -- p is unconditionally overwritten with the same value either
 * way -- but removing either one measurably regresses the codegen (round
 * 41 confirmed both empirically, byte-exact with them, off by dozens of
 * words without). Do not "simplify" this without re-running
 * ./build-and-verify.sh. */
s32 GraphRoomObj__ScoreDayLog(GraphRoomObj *self, DayLog *log)
{
    u32 i;
    s16 *days;
    s32 j;
    s16 *p;
    s32 idx;
    s32 found;
    s32 limit;

    if (log->scored != 0) {
        goto fail;
    }

    if (log->fullScan != 0) {
        limit = 100;
    } else {
        limit = log->dayCount;
        if (limit > 100) {
            limit = 100;
        }
    }

    for (i = 0; i < 4; i++) {
        found = 0;
        idx = log->dayCount - 1;
        for (j = 0; j < limit; j++) {
            if (idx < 0) {
                idx = 0x16C;
            } else {
                p = D_80087BD4;
            }
            p = (days = D_80087BD4);
            days = log->days;
            if (p[i] == days[idx]) {
                self->matchedDayIndices[i] = j;
                found++;
            }
            idx--;
        }
        if (found == 0) {
            goto fail;
        }
    }

    log->scored = 1;
    self->highlightCount = 0;
    return 1;

fail:
    return 0;
}

extern s32 D_8008ABBC;

void GraphRoomObj__TickHighlight(GraphRoomObj *self) {
    if (self->scored != 0) {
        if (self->elapsedHours >= 0x1F) {
            if (self->highlightCount < 4) {
                if ((self->elapsedHours % 24) == 0) {
                    s8 idx = self->matchedDayIndices[self->highlightCount];
                    self->points[idx]->methods->highlight(self->points[idx], 1, &D_8008ABBC);
                    self->highlightCount += 1;
                }
            }
        }
    }
}

extern GraphRoomMethods gGraphRoomMethods;

GraphRoomMethods *GetGraphRoomMethods(void) {
    return &gGraphRoomMethods;
}
