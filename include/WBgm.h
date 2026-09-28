#ifndef WBGM_H
#define WBGM_H

#include "BasicClass.h"
#include "DrawSystem.h"
#include "VabStreamObj.h"
#include "RequestedFile.h"

/*
 * WBgm -- class id 0x50, method table gWBgmMethods (24 slots), a direct
 * BasicClass subclass (its ctor calls Get_vtable_BasicClass()->ctor first;
 * `classtable.py gWBgmMethods --vs gBasicClassMethods` overrides the ctor, finalize
 * and onNotify and adds nine slots). Methods in src/WBgm.c. No class
 * derives from it. Name from rodata sSeqOpenErrorMsg, "Seq Open error in
 * WBgmHandleMonitorEvent", printed by WBgm__HandleMonitorEvent's own body.
 *
 * A background-music player: one libsnd SEQ played on one VAB bank. The
 * bank is a VabStreamObj (`vab`), the SEQ file a RequestedFile (`seqData`);
 * both load asynchronously, so the SEQ is SsSeqOpen'd (HandleMonitorEvent)
 * only once `vab->attrsReady` and `seqData->loaded` are both set, and
 * played at once if `autoPlay`. Until then `openState` is WBGM_OPEN_WAITING
 * and the DrawSystem retries: the ctor adds the DrawSystem as a child, so
 * the DrawSystem's per-VSync notifyParents(self, DRAWSYSTEM_EVENT_VSYNC)
 * (include/DrawSystem.h) reaches onNotify, which forwards a DrawSystem
 * sender (DRAWSYSTEM_CLASS_ID) to +0x040 update, which retries on that
 * event.
 *
 * Its one construction: DayTask__DayTask (src/DayTaskStageMap.c),
 * New_WBgm(PickSoundBank(0), NULL, 1): the VAB path is one of the seven
 * gSoundBankPaths strings ("SND\\AMBIENT" ... "SND\\STANDERD",
 * asm/data/1B84.rodata.s), no SEQ yet, autoPlay on. That caller keeps the
 * object as DayTask::bgm (include/DayTask.h), and
 * hands it to New_ObjM, whose ObjM keeps it at +0x054 and calls +0x04C
 * pause and +0x050 resume on it (ObjM__AdvancePauseSetup,
 * ObjM__TeardownPauseOverlay; include/ObjM.h).
 *
 * gWBgmActive is 1 from the ctor to finalize; IsWBgmActive returns it, and
 * VabStreamObj__Finalize (src/PlacementGridVabSound.c) shuts libsnd down (SsEnd,
 * SsQuit) only when the last VAB closes AND no WBgm is active.
 */

typedef struct WBgm WBgm;
typedef struct WBgmMethods WBgmMethods;

/* BasicClass's slots, then this class's own, named for their occupants. */
struct WBgmMethods {
    BASICCLASS_SLOTS(WBgm, (WBgm * self, char *vabPath, char *seqPath, s32 autoPlay)); /* WBgm__WBgm */
    /* +0x040 */ void (*update)(WBgm *self, DrawSystem *sender,
                                s32 event);  /* WBgm__Update; onNotify's DrawSystem case */
    /* +0x044 */ void (*play)(WBgm *self);   /* WBgm__Play */
    /* +0x048 */ void (*stop)(WBgm *self);   /* WBgm__Stop: SsSeqStop + SsSeqClose */
    /* +0x04C */ void (*pause)(WBgm *self);  /* WBgm__Pause */
    /* +0x050 */ void (*resume)(WBgm *self); /* WBgm__Resume */
    /* +0x054 */ void (*setVol)(WBgm *self, s16 left, s16 right);     /* WBgm__SetVol */
    /* +0x058 */ void (*crescendo)(WBgm *self, s16 vol, s32 seconds); /* WBgm__Crescendo */
    /* +0x05C */ void (*setSeq)(WBgm *self, char *seqPath); /* WBgm__SetSeq; NULL only drops the old one */
    /* +0x060 */ void (*setVab)(WBgm *self, char *vabPath); /* WBgm__SetVab; NULL only drops the old one */
};

/* WBgm::openState: how far the SEQ is from being SsSeqOpen'd. */
enum WBgmOpenState {
    WBGM_OPEN_IDLE = 0,    /* set by the ctor and by stop: nothing waiting to open */
    WBGM_OPEN_WAITING = 1, /* a path was set before both loads finished: the VSync update retries */
    WBGM_OPEN_DONE = 2     /* HandleMonitorEvent called SsSeqOpen (seqId -1 if it failed) */
};

struct WBgm {
    BASICCLASS_FIELDS(WBgmMethods);
    /* +0x00C */ VabStreamObj *vab; /* New_VabStreamObj(setVab's path) */
    /* +0x010 */ RequestedFile *seqData; /* New_RequestedFile(setSeq's path): the SEQ file, SsSeqOpen'd from its buffer once loaded */
    /* +0x014 */ s16 seqId;             /* SsSeqOpen's result; every SsSeq* call's access number */
    /* +0x016 */ u8 pad16[0x1A - 0x16]; /* no accessor in the class's methods */
    /* +0x01A */ u16 openState;         /* enum WBgmOpenState */
    /* +0x01C */ u16 paused;
    /* +0x01E */ u16 playing;
    /* +0x020 */ s32 autoPlay; /* the ctor's argument: play as soon as the SEQ opens */
}; /* 0x24 bytes: New_WBgm */

extern WBgmMethods gWBgmMethods;
extern WBgmMethods *Get_vtable_WBgm(void); /* returns &gWBgmMethods */

extern s32 gWBgmActive; /* 1 between WBgm__WBgm and WBgm__Finalize */

/* The class's own methods and helpers, in address order. */
WBgm *New_WBgm(char *vabPath, char *seqPath, s32 autoPlay); /* BMemPMgrAlloc(0x24), then ctor */
void WBgm__WBgm(WBgm *self, char *vabPath, char *seqPath, s32 autoPlay);
void WBgm__Finalize(WBgm *self);
void WBgm__OnNotify(WBgm *self, void *sender, s32 event);
void WBgm__Update(WBgm *self, DrawSystem *sender, s32 event);
s32 WBgm__HandleMonitorEvent(WBgm *self); /* not a slot: SsSeqOpen once both loads are done; 1 if opened */
void WBgm__Play(WBgm *self);
void WBgm__Stop(WBgm *self);
void WBgm__Pause(WBgm *self);
void WBgm__Resume(WBgm *self);
void WBgm__SetVol(WBgm *self, s16 left, s16 right);
void WBgm__Crescendo(WBgm *self, s16 vol, s32 seconds);
void WBgm__SetSeq(WBgm *self, char *seqPath);
void WBgm__SetVab(WBgm *self, char *vabPath);
s32 IsWBgmActive(void); /* returns gWBgmActive */

#endif
