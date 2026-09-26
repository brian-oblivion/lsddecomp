/*
 * code_2a0e0 -- GAME code carved from psyq_2a0e0 on 2026-09-25 (FINISHING-PLAN
 * revision 18). 0x2A0E0..0x2A878 (vram 0x800398E0..0x8003A078). It was counted
 * as Psy-Q SDK by segment name; tools/gameinsdk.py measured it as game (a call
 * into game code, a method-table entry beside game methods, or contiguity with
 * those, and no Sony fingerprint). What it holds: WBgm, a background-music
 * SEQ player built on New_VabStreamObj; the yaml had called this gap "the
 * game's own libspu build".
 *
 * All 17 functions matched in round 81 (runners echo and delta); named round
 * 82 (runner bravo, FINISHING-PLAN track 3). Class named WBgm from rodata
 * D_80010FEC ("Seq Open error in WBgmHandleMonitorEvent"): the string is part
 * of WBgm__HandleMonitorEvent's own matched body (the printf sits right where
 * it is read), so it is body evidence for that function's name and, via its
 * "WBgm" prefix, a lead for the class -- weighed as evidence, not proof.
 *
 * Track 4 (round 88): the class is declared once, in include/WBgm.h (table
 * gWBgmMethods); this unit keeps no view of it.
 */
#include "common.h"
#include "BasicClass.h"
#include "DrawSystem.h"
#include "WBgm.h"

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
extern void printf(const char *fmt);
extern const char D_80010FEC[]; /* "Seq Open error in WBgmHandleMonitorEvent" */

extern s32 func_8002CC28(void);

extern u8 gSsSizeTableBuf[];

WBgm *New_WBgm(char *vabPath, char *seqPath, s32 autoPlay) {
    WBgm *self;

    self = BMemPMgrAlloc(0x24);
    if (self != NULL) {
        Get_vtable_WBgm()->ctor(self, vabPath, seqPath, autoPlay);
        return self;
    }
    return NULL;
}

void WBgm__WBgm(WBgm *self, char *vabPath, char *seqPath, s32 autoPlay) {
    Get_vtable_BasicClass()->ctor((BasicClass *)self);
    self->methods = Get_vtable_WBgm();
    self->vab = NULL;
    self->seqData = NULL;
    self->seqId = 0;
    self->openState = 0;
    self->paused = 0;
    self->playing = 0;
    self->autoPlay = autoPlay;
    gWBgmActive = 1;
    self->methods->setSeq(self, seqPath);
    self->methods->setVab(self, vabPath);
    self->methods->addChild(self, (BasicClass *)GetDrawSystem());
}

void WBgm__Finalize(WBgm *self) {
    gWBgmActive = 0;
    self->methods->stop(self);
    SsSeqClose(self->seqId);
    if (self->vab != NULL) {
        self->vab->methods->release(self->vab);
    }
    if (self->seqData != NULL) {
        self->seqData->methods->release(self->seqData);
    }
    self->methods->removeChild(self, (BasicClass *)GetDrawSystem());
    Get_vtable_BasicClass()->finalize((BasicClass *)self);
}

void WBgm__OnNotify(WBgm *self, void *sender, s32 event) {
    Get_vtable_BasicClass()->onNotify((BasicClass *)self, sender, event);
    if ((((BasicClass *)sender)->methods->header & 0xF) == 1) {
        self->methods->update(self, (DrawSystem *)sender, event);
    }
}

void WBgm__Update(WBgm *self, DrawSystem *sender, s32 event) {
    if (event == 2 && self->openState == 1 && WBgm__HandleMonitorEvent(self) && self->autoPlay != 0) {
        self->methods->play(self);
    }
}

s32 WBgm__HandleMonitorEvent(WBgm *self) {
    VabStreamObj *vab;
    Class6EED8 *seq;

    vab = self->vab;
    if (vab == NULL) {
        return 0;
    }
    seq = self->seqData;
    if (seq == NULL) {
        return 0;
    }
    if (vab->attrsReady == 0) {
        return 0;
    }
    if (seq->loaded == 0) {
        return 0;
    }
    self->seqId = SsSeqOpen(seq->buffer, vab->vabId);
    if (self->seqId == -1) {
        printf(D_80010FEC);
    }
    SsSeqSetVol(self->seqId, 0x34, 0x34);
    self->openState = 2;
    return 1;
}

void WBgm__Play(WBgm *self) {
    if (self->playing == 0) {
        SsSeqSetVol(self->seqId, 0x34, 0x34);
        SsSeqPlay(self->seqId, 1, 0);
        self->playing = 1;
    }
}

void WBgm__Stop(WBgm *self) {
    if (self->playing != 0) {
        SsSeqStop(self->seqId);
        SsSeqClose(self->seqId);
        self->playing = 0;
        self->openState = 0;
    }
}

void WBgm__Pause(WBgm *self) {
    if (self->paused == 0) {
        SsSeqPause(self->seqId);
        self->paused = 1;
    }
}

void WBgm__Resume(WBgm *self) {
    if (self->paused != 0) {
        SsSeqReplay(self->seqId);
        self->paused = 0;
    }
}

void WBgm__SetVol(WBgm *self, s16 left, s16 right) {
    SsSeqSetVol(self->seqId, left, right);
}

void WBgm__Crescendo(WBgm *self, s16 vol, s32 scale) {
    SsSeqSetCrescendo(self->seqId, vol, func_8002CC28() * scale);
}

void WBgm__SetSeq(WBgm *self, char *seqPath) {
    if (self->playing != 0) {
        self->methods->stop(self);
    }
    if (self->seqData != NULL) {
        self->seqData->methods->release(self->seqData);
        self->seqData = NULL;
    }
    if (seqPath != NULL) {
        self->seqData = New_Class6EED8(seqPath);
        if (WBgm__HandleMonitorEvent(self)) {
            if (self->autoPlay != 0) {
                self->methods->play(self);
            }
        } else if (self->openState == 0) {
            self->openState = 1;
        }
    }
}

void WBgm__SetVab(WBgm *self, char *vabPath) {
    if (self->playing != 0) {
        self->methods->stop(self);
    }
    if (self->vab != NULL) {
        self->vab->methods->release(self->vab);
        self->vab = NULL;
    }
    if (vabPath != NULL) {
        self->vab = New_VabStreamObj(vabPath);
        if (WBgm__HandleMonitorEvent(self)) {
            if (self->autoPlay != 0) {
                self->methods->play(self);
            }
        } else if (self->openState == 0) {
            self->openState = 1;
        }
    }
}

WBgmMethods *Get_vtable_WBgm(void) {
    return &gWBgmMethods;
}

s32 IsWBgmActive(void) {
    return gWBgmActive;
}

void *GetSsSizeTableBuf(void) {
    return &gSsSizeTableBuf;
}
