/*
 * code_2a0e0 -- GAME code carved from psyq_2a0e0 on 2026-09-25 (FINISHING-PLAN
 * revision 18). 0x2A0E0..0x2A878 (vram 0x800398E0..0x8003A078). It was counted
 * as Psy-Q SDK by segment name; tools/gameinsdk.py measured it as game (a call
 * into game code, a method-table entry beside game methods, or contiguity with
 * those, and no Sony fingerprint). What it holds: 12 methods of D_8006E48C,
 * which call New_VabStreamObj; the yaml had called this gap "the game's own
 * libspu build".
 *
 * All 17 functions matched in round 81 (runners echo and delta). The class's
 * source-level name appears in D_80010FEC: "WBgmHandleMonitorEvent".
 */
#include "common.h"
#include "BasicClass.h"

/* Local view of D_8006E48C's objects: a SEQ player. Fields named from the
 * libsnd calls they feed. */
typedef struct SeqObj SeqObj;
typedef struct SeqObjMethods SeqObjMethods;

struct SeqObjMethods {
    BASICCLASS_SLOTS(SeqObj, (SeqObj *self, s32 vabArg, s32 seqArg, s32 autoPlay)); /* WBgm__WBgm */
    /* +0x040 */ void (*update)(SeqObj *self, s32 arg1, s32 arg2); /* WBgm__Update */
    /* +0x044 */ void (*play)(SeqObj *self);                       /* WBgm__Play */
    /* +0x048 */ void (*stop)(SeqObj *self);                       /* WBgm__Stop */
    /* +0x04C */ void (*pause)(SeqObj *self);                      /* WBgm__Pause */
    /* +0x050 */ void (*resume)(SeqObj *self);                     /* WBgm__Resume */
    /* +0x054 */ void (*setVol)(SeqObj *self, s16 l, s16 r);       /* WBgm__SetVol */
    /* +0x058 */ void (*crescendo)(SeqObj *self, s16 v, s32 s);    /* WBgm__Crescendo */
    /* +0x05C */ void (*setSeq)(SeqObj *self, s32 arg);            /* WBgm__SetSeq */
    /* +0x060 */ void (*setVab)(SeqObj *self, s32 arg);            /* WBgm__SetVab */
};

/* What +0x0C holds: a New_VabStreamObj object. Only the fields read here. */
typedef struct SeqVab {
    BASICCLASS_FIELDS(BasicClassMethods);
    /* +0x00C */ u8 padC[0x54 - 0xC];
    /* +0x054 */ s16 vabId;
    /* +0x056 */ u8 pad56[2];
    /* +0x058 */ u16 ready;
} SeqVab;

/* What +0x10 holds: a func_800422CC object. Only the fields read here. */
typedef struct SeqData {
    BASICCLASS_FIELDS(BasicClassMethods);
    /* +0x00C */ u8 padC[0x10 - 0xC];
    /* +0x010 */ unsigned long *addr;
    /* +0x014 */ u8 pad14[0x2C - 0x14];
    /* +0x02C */ s32 loaded;
} SeqData;

struct SeqObj {
    BASICCLASS_FIELDS(SeqObjMethods);
    /* +0x00C */ SeqVab *unkC;
    /* +0x010 */ SeqData *unk10;
    /* +0x014 */ s16 seqId;
    /* +0x016 */ u8 pad16[0x1A - 0x16];
    /* +0x01A */ u16 state;
    /* +0x01C */ u16 paused;
    /* +0x01E */ u16 playing;
    /* +0x020 */ s32 unk20;
};

/* libsnd (LIBSND.H) */
extern void SsSeqPlay(short, char, short);
extern void SsSeqPause(short);
extern void SsSeqReplay(short);
extern void SsSeqStop(short);
extern void SsSeqSetVol(short, short, short);
extern void SsSeqSetCrescendo(short, short, long);
extern void SsSeqClose(short);
extern short SsSeqOpen(unsigned long *addr, short vab_id);

extern void *BMemPMgrAlloc(s32 size);
extern BasicClass *func_80020C5C(void);
extern SeqData *func_800422CC(s32 arg);
extern SeqVab *New_VabStreamObj(s32 arg0);
extern void printf(const char *fmt);
extern const char D_80010FEC[]; /* "Seq Open error in WBgmHandleMonitorEvent" */
SeqObjMethods *Get_vtable_WBgm(void);

extern s32 func_8002CC28(void);
s32 WBgm__HandleMonitorEvent(SeqObj *self);

extern SeqObjMethods D_8006E48C;
extern s32 gWBgmActive;
extern u8 gSsSizeTableBuf[];

SeqObj *New_WBgm(s32 vabArg, s32 seqArg, s32 autoPlay) {
    SeqObj *self;

    self = BMemPMgrAlloc(0x24);
    if (self != NULL) {
        Get_vtable_WBgm()->ctor(self, vabArg, seqArg, autoPlay);
        return self;
    }
    return NULL;
}
void WBgm__WBgm(SeqObj *self, s32 vabArg, s32 seqArg, s32 autoPlay) {
    Get_vtable_BasicClass()->ctor((BasicClass *)self);
    self->methods = Get_vtable_WBgm();
    self->unkC = NULL;
    self->unk10 = NULL;
    self->seqId = 0;
    self->state = 0;
    self->paused = 0;
    self->playing = 0;
    self->unk20 = autoPlay;
    gWBgmActive = 1;
    self->methods->setSeq(self, seqArg);
    self->methods->setVab(self, vabArg);
    self->methods->addChild(self, func_80020C5C());
}
void WBgm__Finalize(SeqObj *self) {
    gWBgmActive = 0;
    self->methods->stop(self);
    SsSeqClose(self->seqId);
    if (self->unkC != NULL) {
        self->unkC->methods->release((BasicClass *)self->unkC);
    }
    if (self->unk10 != NULL) {
        self->unk10->methods->release((BasicClass *)self->unk10);
    }
    self->methods->removeChild(self, func_80020C5C());
    Get_vtable_BasicClass()->finalize((BasicClass *)self);
}
void WBgm__OnNotify(SeqObj *self, void *sender, s32 event) {
    Get_vtable_BasicClass()->onNotify((BasicClass *)self, sender, event);
    if ((((BasicClass *)sender)->methods->header & 0xF) == 1) {
        self->methods->update(self, (s32)sender, event);
    }
}
void WBgm__Update(SeqObj *self, s32 arg1, s32 arg2) {
    if (arg2 == 2 && self->state == 1 && WBgm__HandleMonitorEvent(self) && self->unk20 != 0) {
        self->methods->play(self);
    }
}
s32 WBgm__HandleMonitorEvent(SeqObj *self) {
    SeqVab *vab;
    SeqData *seq;

    vab = self->unkC;
    if (vab == NULL) {
        return 0;
    }
    seq = self->unk10;
    if (seq == NULL) {
        return 0;
    }
    if (vab->ready == 0) {
        return 0;
    }
    if (seq->loaded == 0) {
        return 0;
    }
    self->seqId = SsSeqOpen(seq->addr, vab->vabId);
    if (self->seqId == -1) {
        printf(D_80010FEC);
    }
    SsSeqSetVol(self->seqId, 0x34, 0x34);
    self->state = 2;
    return 1;
}
void WBgm__Play(SeqObj *self) {
    if (self->playing == 0) {
        SsSeqSetVol(self->seqId, 0x34, 0x34);
        SsSeqPlay(self->seqId, 1, 0);
        self->playing = 1;
    }
}
void WBgm__Stop(SeqObj *self) {
    if (self->playing != 0) {
        SsSeqStop(self->seqId);
        SsSeqClose(self->seqId);
        self->playing = 0;
        self->state = 0;
    }
}
void WBgm__Pause(SeqObj *self) {
    if (self->paused == 0) {
        SsSeqPause(self->seqId);
        self->paused = 1;
    }
}
void WBgm__Resume(SeqObj *self) {
    if (self->paused != 0) {
        SsSeqReplay(self->seqId);
        self->paused = 0;
    }
}
void WBgm__SetVol(SeqObj *self, s16 left, s16 right) {
    SsSeqSetVol(self->seqId, left, right);
}
void WBgm__Crescendo(SeqObj *self, s16 vol, s32 scale) {
    SsSeqSetCrescendo(self->seqId, vol, func_8002CC28() * scale);
}
void WBgm__SetSeq(SeqObj *self, s32 arg) {
    if (self->playing != 0) {
        self->methods->stop(self);
    }
    if (self->unk10 != NULL) {
        self->unk10->methods->release((BasicClass *)self->unk10);
        self->unk10 = NULL;
    }
    if (arg != 0) {
        self->unk10 = func_800422CC(arg);
        if (WBgm__HandleMonitorEvent(self)) {
            if (self->unk20 != 0) {
                self->methods->play(self);
            }
        } else if (self->state == 0) {
            self->state = 1;
        }
    }
}
void WBgm__SetVab(SeqObj *self, s32 arg) {
    if (self->playing != 0) {
        self->methods->stop(self);
    }
    if (self->unkC != NULL) {
        self->unkC->methods->release((BasicClass *)self->unkC);
        self->unkC = NULL;
    }
    if (arg != 0) {
        self->unkC = New_VabStreamObj(arg);
        if (WBgm__HandleMonitorEvent(self)) {
            if (self->unk20 != 0) {
                self->methods->play(self);
            }
        } else if (self->state == 0) {
            self->state = 1;
        }
    }
}
SeqObjMethods *Get_vtable_WBgm(void) {
    return &D_8006E48C;
}
s32 IsWBgmActive(void) {
    return gWBgmActive;
}
void *GetSsSizeTableBuf(void) {
    return &gSsSizeTableBuf;
}
