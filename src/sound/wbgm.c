/*
 * WBgm, the background-music player: one libsnd SEQ played on one VAB bank
 * (the class is documented in include/wbgm.h). The file holds its allocator,
 * constructor and methods in table order, with the non-slot helper
 * WBgm__HandleMonitorEvent (it SsSeqOpens the SEQ once both files have
 * loaded) after update; then the table getter; IsWBgmActive, which
 * VabStreamObj__Finalize checks before it shuts libsnd down; and
 * GetSsSizeTableBuf, the buffer vab_stream_obj.c passes to
 * SsSetTableSize. The method table ends the file.
 */
#include "common.h"
#include <libsnd.h>
#include "basic_class.h"
#include "draw_system.h"
#include "wbgm.h"
#include "bmem_pmgr.h"
#include <stdio.h>
#include "vab_stream_obj.h"

extern s32 sWBgmActive; /* 1 between WBgm__WBgm and WBgm__Finalize */

extern char sSeqOpenErrorMsg[]; /* "Seq Open error in WBgmHandleMonitorEvent" */

extern u8 sSsSizeTableBuf[];

/* The volume, left and right, a SEQ gets when it opens and again on every
 * play (libsnd's range is 0 to 127). */
#define WBGM_PLAY_VOL 52

WBgm *New_WBgm(char *vabPath, char *seqPath, s32 autoPlay) {
    WBgm *self;

    self = BMemPMgrAlloc(sizeof(WBgm));
    if (self != NULL) {
        GetWBgmMethods()->ctor(self, vabPath, seqPath, autoPlay);
        return self;
    }
    return NULL;
}

void WBgm__WBgm(WBgm *self, char *vabPath, char *seqPath, s32 autoPlay) {
    GetBasicClassMethods()->ctor((BasicClass *)self);
    self->methods = GetWBgmMethods();
    self->vab = NULL;
    self->seqData = NULL;
    self->seqId = 0;
    self->openState = WBGM_OPEN_IDLE;
    self->paused = 0;
    self->playing = 0;
    self->autoPlay = autoPlay;
    sWBgmActive = 1;
    self->methods->setSeq(self, seqPath);
    self->methods->setVab(self, vabPath);
    self->methods->addChild(self, (BasicClass *)GetDrawSystem());
}

void WBgm__Finalize(WBgm *self) {
    sWBgmActive = 0;
    self->methods->stop(self);
    SsSeqClose(self->seqId);
    if (self->vab != NULL) {
        self->vab->methods->release(self->vab);
    }
    if (self->seqData != NULL) {
        self->seqData->methods->release(self->seqData);
    }
    self->methods->removeChild(self, (BasicClass *)GetDrawSystem());
    GetBasicClassMethods()->finalize((BasicClass *)self);
}

void WBgm__OnNotify(WBgm *self, void *sender, s32 event) {
    GetBasicClassMethods()->onNotify((BasicClass *)self, sender, event);
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

WBgmMethods *GetWBgmMethods(void) {
    return &gWBgmMethods;
}

s32 IsWBgmActive(void) {
    return sWBgmActive;
}

void *GetSsSizeTableBuf(void) {
    return &sSsSizeTableBuf;
}

/* WBgm's method table (include/wbgm.h): BasicClass's slots with the ctor,
 * finalize and onNotify, then the player's slots. A (void *) entry is a
 * method declared on BasicClass *. */
WBgmMethods gWBgmMethods = {
    /* +0x000 header */ WBGM_CLASS_ID,
    /* +0x004 release */ (void *)BasicClass__Release,
    /* +0x008 ctor */ WBgm__WBgm,
    /* +0x00C finalize */ WBgm__Finalize,
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
    /* +0x038 onNotify */ WBgm__OnNotify,
    /* +0x03C slot3C */ NULL,
    /* +0x040 update */ WBgm__Update,
    /* +0x044 play */ WBgm__Play,
    /* +0x048 stop */ WBgm__Stop,
    /* +0x04C pause */ WBgm__Pause,
    /* +0x050 resume */ WBgm__Resume,
    /* +0x054 setVol */ WBgm__SetVol,
    /* +0x058 crescendo */ WBgm__Crescendo,
    /* +0x05C setSeq */ WBgm__SetSeq,
    /* +0x060 setVab */ WBgm__SetVab,
};
