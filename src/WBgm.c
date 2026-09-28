/*
 * WBgm, the background-music player: one libsnd SEQ played on one VAB bank
 * (the class is documented in include/WBgm.h). The file holds its allocator,
 * constructor and methods in table order, with the non-slot helper
 * WBgm__HandleMonitorEvent (it SsSeqOpens the SEQ once both files have
 * loaded) after update; then the table getter; IsWBgmActive, which
 * VabStreamObj__Finalize checks before it shuts libsnd down; and
 * GetSsSizeTableBuf, the buffer PlacementGridVabSound.c passes to
 * SsSetTableSize.
 *
 * Edges: Sony objects on both sides, libspu/s_sav before and libsnd/ssvol
 * after, so the file is exactly this unit; its one string (80010FEC,
 * HandleMonitorEvent's) is its own. tuboundary.py's "a forced boundary lies
 * in this stretch" note is satisfied by those Sony edges. Named for its
 * class.
 */
#include "common.h"
#include <libsnd.h>
#include "BasicClass.h"
#include "DrawSystem.h"
#include "WBgm.h"

extern void *BMemPMgrAlloc(s32 size);
extern void printf(const char *fmt);
extern const char sSeqOpenErrorMsg[]; /* "Seq Open error in WBgmHandleMonitorEvent" */

extern s32 GetSsTicksPerSecond(void);

extern u8 gSsSizeTableBuf[];

/* The volume, left and right, a SEQ gets when it opens and again on every
 * play (libsnd's range is 0 to 127). */
#define WBGM_PLAY_VOL 52

WBgm *New_WBgm(char *vabPath, char *seqPath, s32 autoPlay) {
    WBgm *self;

    self = BMemPMgrAlloc(sizeof(WBgm));
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
    self->openState = WBGM_OPEN_IDLE;
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
    if ((((BasicClass *)sender)->methods->header & CLASS_ID_ROOT_MASK) == DRAWSYSTEM_CLASS_ID) {
        self->methods->update(self, (DrawSystem *)sender, event);
    }
}

void WBgm__Update(WBgm *self, DrawSystem *sender, s32 event) {
    if (event == DRAWSYSTEM_EVENT_VSYNC && self->openState == WBGM_OPEN_WAITING &&
        WBgm__HandleMonitorEvent(self) && self->autoPlay != 0) {
        self->methods->play(self);
    }
}

s32 WBgm__HandleMonitorEvent(WBgm *self) {
    VabStreamObj *vab;
    RequestedFile *seq;

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
        printf(sSeqOpenErrorMsg);
    }
    SsSeqSetVol(self->seqId, WBGM_PLAY_VOL, WBGM_PLAY_VOL);
    self->openState = WBGM_OPEN_DONE;
    return 1;
}

void WBgm__Play(WBgm *self) {
    if (self->playing == 0) {
        SsSeqSetVol(self->seqId, WBGM_PLAY_VOL, WBGM_PLAY_VOL);
        SsSeqPlay(self->seqId, SSPLAY_PLAY, SSPLAY_INFINITY);
        self->playing = 1;
    }
}

void WBgm__Stop(WBgm *self) {
    if (self->playing != 0) {
        SsSeqStop(self->seqId);
        SsSeqClose(self->seqId);
        self->playing = 0;
        self->openState = WBGM_OPEN_IDLE;
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

void WBgm__Crescendo(WBgm *self, s16 vol, s32 seconds) {
    SsSeqSetCrescendo(self->seqId, vol, GetSsTicksPerSecond() * seconds);
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
        self->seqData = New_RequestedFile(seqPath);
        if (WBgm__HandleMonitorEvent(self)) {
            if (self->autoPlay != 0) {
                self->methods->play(self);
            }
        } else if (self->openState == WBGM_OPEN_IDLE) {
            self->openState = WBGM_OPEN_WAITING;
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
        } else if (self->openState == WBGM_OPEN_IDLE) {
            self->openState = WBGM_OPEN_WAITING;
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
