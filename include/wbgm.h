#ifndef WBGM_H
#define WBGM_H

#include "basic_class.h"
#include "draw_system.h"
#include "vab_stream_obj.h"
#include "requested_file.h"

/**
 * @file wbgm.h
 * @brief WBgm, the background-music player: one libsnd SEQ played on one VAB
 * bank.
 */

typedef struct WBgm WBgm;
typedef struct WBgmMethods WBgmMethods;

/** WBgm's class id (gWBgmMethods word +0x000). */
#define WBGM_CLASS_ID 0x50

/**
 * @brief WBgm's method table (24 slots): BasicClass's fifteen, with the ctor,
 * finalize and onNotify overridden, then nine of its own.
 */
struct WBgmMethods {
    BASICCLASS_SLOTS(WBgm, (WBgm * self, char *vabPath, char *seqPath, s32 autoPlay));
    /* +0x040 */ void (*update)(WBgm *self, DrawSystem *sender, s32 event); /**< @see WBgm__Update */
    /* +0x044 */ void (*play)(WBgm *self);                                  /**< @see WBgm__Play */
    /* +0x048 */ void (*stop)(WBgm *self);                                  /**< @see WBgm__Stop */
    /* +0x04C */ void (*pause)(WBgm *self);                                 /**< @see WBgm__Pause */
    /* +0x050 */ void (*resume)(WBgm *self);                          /**< @see WBgm__Resume */
    /* +0x054 */ void (*setVol)(WBgm *self, s16 left, s16 right);     /**< @see WBgm__SetVol */
    /* +0x058 */ void (*crescendo)(WBgm *self, s16 vol, s32 seconds); /**< @see WBgm__Crescendo */
    /* +0x05C */ void (*setSeq)(WBgm *self, char *seqPath);           /**< @see WBgm__SetSeq */
    /* +0x060 */ void (*setVab)(WBgm *self, char *vabPath);           /**< @see WBgm__SetVab */
};

/** @brief WBgm::openState: how far the SEQ is from being opened. */
enum WBgmOpenState {
    WBGM_OPEN_IDLE = 0,    /**< Set by the ctor and by stop: nothing waiting to open. */
    WBGM_OPEN_WAITING = 1, /**< A path was set before both loads finished: the VSync update retries. */
    WBGM_OPEN_DONE = 2     /**< HandleMonitorEvent called SsSeqOpen (seqId -1 if it failed). */
};

/**
 * @brief The background-music player (class id 0x50), a direct BasicClass
 * subclass; no class derives from it. Named for its error message, "Seq Open
 * error in WBgmHandleMonitorEvent". Methods in src/sound/wbgm.c.
 *
 * The bank is a VabStreamObj (`vab`) and the SEQ file a RequestedFile
 * (`seqData`). Both load asynchronously, so the SEQ is opened
 * (HandleMonitorEvent) only once `vab->attrsReady` and `seqData->loaded` are
 * both set, and played at once if `autoPlay`. Until then `openState` is
 * WBGM_OPEN_WAITING and the DrawSystem retries it: the ctor adds the
 * DrawSystem as a child, so the DrawSystem's per-VSync
 * notifyParents(self, DRAWSYSTEM_EVENT_VSYNC) reaches onNotify, which
 * forwards a DrawSystem sender to update.
 *
 * Lifecycle: DayTask__DayTask (src/world/day_task.c) makes the one instance,
 * New_WBgm(PickSoundBank(0), NULL, 1): one of the seven sound bank paths,
 * no SEQ yet, autoPlay on. It keeps it as DayTask::bgm and hands it to
 * New_ObjM, whose ObjM calls pause and resume on it around its pause overlay
 * (ObjM__AdvancePauseSetup, ObjM__TeardownPauseOverlay). While any WBgm
 * exists, VabStreamObj__Finalize leaves libsnd running (IsWBgmActive).
 */
struct WBgm {
    BASICCLASS_FIELDS(WBgmMethods);
    /* +0x00C */ VabStreamObj *vab; /**< The bank: New_VabStreamObj(setVab's path). */
    /* +0x010 */ RequestedFile *seqData; /**< The SEQ file: New_RequestedFile(setSeq's path), opened from its buffer once loaded. */
    /* +0x014 */ s16 seqId; /**< SsSeqOpen's result; every SsSeq* call's access number. */
    /* +0x016 */ u8 pad16[0x1A - 0x16];
    /* +0x01A */ u16 openState; /**< enum WBgmOpenState. */
    /* +0x01C */ u16 paused;    /**< Set by pause, cleared by resume. */
    /* +0x01E */ u16 playing;   /**< Set by play, cleared by stop. */
    /* +0x020 */ s32 autoPlay;  /**< The ctor's argument: play as soon as the SEQ opens. */
}; /* 0x24 bytes: New_WBgm */

/** @brief WBgm's method table (see WBgmMethods). */
extern WBgmMethods gWBgmMethods;

/**
 * @brief Returns WBgm's method table.
 * @return &gWBgmMethods.
 */
extern WBgmMethods *GetWBgmMethods(void);

/**
 * @brief Allocates a WBgm from the BMemPMgr pool and constructs it.
 * @param vabPath  The VAB bank's base path, or NULL.
 * @param seqPath  The SEQ file's path, or NULL.
 * @param autoPlay Nonzero: play as soon as the SEQ opens.
 * @return The new player, or NULL when the pool is exhausted.
 */
WBgm *New_WBgm(char *vabPath, char *seqPath, s32 autoPlay);

/**
 * @brief Constructor (slot +0x008): BasicClass's ctor, the fields cleared,
 * the player marked active (IsWBgmActive), then setSeq, setVab, and the
 * DrawSystem added as a child so its VSync events reach onNotify.
 * @param self     The object being constructed.
 * @param vabPath  The VAB bank's base path, or NULL.
 * @param seqPath  The SEQ file's path, or NULL.
 * @param autoPlay Nonzero: play as soon as the SEQ opens.
 */
void WBgm__WBgm(WBgm *self, char *vabPath, char *seqPath, s32 autoPlay);

/**
 * @brief Finalizer (slot +0x00C): marks the player inactive, stops and
 * closes the SEQ, releases the bank and the SEQ file, removes the DrawSystem
 * child, then BasicClass's finalize.
 * @param self The object being finalized.
 */
void WBgm__Finalize(WBgm *self);

/**
 * @brief Slot +0x038, onNotify: BasicClass's, then update when the sender is
 * a DrawSystem.
 * @param self   The player.
 * @param sender The notifying object.
 * @param event  The event number.
 */
void WBgm__OnNotify(WBgm *self, void *sender, s32 event);

/**
 * @brief Slot +0x040: on a VSync event while the SEQ waits to open, retries
 * HandleMonitorEvent and plays if it opened and `autoPlay` is set.
 * @param self   The player.
 * @param sender The DrawSystem.
 * @param event  The event number.
 */
void WBgm__Update(WBgm *self, DrawSystem *sender, s32 event);

/**
 * @brief Opens the SEQ once both the bank's attributes and the SEQ file have
 * loaded, printing "Seq Open error in WBgmHandleMonitorEvent" when SsSeqOpen
 * fails, and sets its volume. Not a slot.
 * @param self The player.
 * @return 1 when SsSeqOpen was called (even if it failed), 0 while either
 *         load is missing or pending.
 */
s32 WBgm__HandleMonitorEvent(WBgm *self);

/**
 * @brief Slot +0x044: sets the volume and plays the SEQ, looping forever,
 * unless it is already playing.
 * @param self The player.
 */
void WBgm__Play(WBgm *self);

/**
 * @brief Slot +0x048: stops and closes the SEQ if it is playing; the SEQ
 * must then be opened again.
 * @param self The player.
 */
void WBgm__Stop(WBgm *self);

/**
 * @brief Slot +0x04C: pauses the SEQ, unless it is paused.
 * @param self The player.
 */
void WBgm__Pause(WBgm *self);

/**
 * @brief Slot +0x050: resumes a paused SEQ.
 * @param self The player.
 */
void WBgm__Resume(WBgm *self);

/**
 * @brief Slot +0x054: sets the SEQ's volume.
 * @param self  The player.
 * @param left  Left volume, 0..127.
 * @param right Right volume, 0..127.
 */
void WBgm__SetVol(WBgm *self, s16 left, s16 right);

/**
 * @brief Slot +0x058: ramps the SEQ's volume by `vol` over `seconds`.
 * @param self    The player.
 * @param vol     Volume change, SsSeqSetCrescendo's.
 * @param seconds Ramp time, converted to libsnd ticks.
 */
void WBgm__Crescendo(WBgm *self, s16 vol, s32 seconds);

/**
 * @brief Slot +0x05C: replaces the SEQ file, stopping playback and releasing
 * the old one; opens the new one at once if everything has loaded, and
 * otherwise leaves it waiting for the VSync retry.
 * @param self    The player.
 * @param seqPath The new SEQ's path, or NULL to only drop the old one.
 */
void WBgm__SetSeq(WBgm *self, char *seqPath);

/**
 * @brief Slot +0x060: replaces the VAB bank, as SetSeq does the SEQ file.
 * @param self    The player.
 * @param vabPath The new bank's base path, or NULL to only drop the old one.
 */
void WBgm__SetVab(WBgm *self, char *vabPath);

/**
 * @brief Whether a WBgm exists: set by the ctor, cleared by finalize.
 * @return 1 while a player exists, else 0.
 */
s32 IsWBgmActive(void);

/**
 * @brief The buffer VabStreamObj's ctor hands SsSetTableSize for libsnd's
 * score table.
 * @return The buffer.
 */
void *GetSsSizeTableBuf(void);

#endif
