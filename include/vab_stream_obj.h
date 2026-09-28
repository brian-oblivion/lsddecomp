#ifndef VAB_STREAM_OBJ_H
#define VAB_STREAM_OBJ_H

#include "file_resource.h"

/**
 * @file vab_stream_obj.h
 * @brief VabStreamObj, one VAB sound bank loaded from disc and played through
 * libsnd, and the libsnd state its instances share.
 */

typedef struct VabStreamObj VabStreamObj;
typedef struct VabStreamObjMethods VabStreamObjMethods;

/**
 * @brief The two bytes of Sony's VagAtr (<libsnd.h>, 32 bytes) that PlayTone
 * reads, at VagAtr's own offsets. Kept reduced so this header does not bring
 * in <libsnd.h>'s prototypes.
 */
typedef struct VabStreamVagAtr {
    /* +0x00 */ u8 pad0[0x4];
    /* +0x04 */ u8 center; /**< VagAtr::center, the tone's centre note. */
    /* +0x05 */ u8 shift;  /**< VagAtr::shift, the centre note's fine tune. */
    /* +0x06 */ u8 pad6[0x20 - 0x6];
} VabStreamVagAtr;

/** @brief The two fields of Sony's VabHdr (32 bytes) that LoadVagAttrs reads. */
typedef struct VabStreamVabHdr {
    /* +0x00 */ u8 pad0[0x12];
    /* +0x12 */ u16 ts; /**< Program count. */
    /* +0x14 */ u16 vs; /**< VAG count. */
    /* +0x16 */ u8 pad16[0x20 - 0x16];
} VabStreamVabHdr;

/**
 * @brief VabStreamObj's FileResource::loadState, advanced by
 * VabStreamObj__AdvanceLoadState. Nothing uses 2 to 5.
 */
enum VabStreamLoadState {
    VABSTREAM_LOAD_IDLE = 0,   /**< Nothing requested. */
    VABSTREAM_LOAD_HEADER = 1, /**< Waiting for "<base>.VH". */
    VABSTREAM_LOAD_BODY = 6    /**< Waiting for "<base>.VB". */
};

/**
 * @brief The type VabStreamObj's processBuffer slot (+0x078) is called
 * through: FileResource declares that slot untyped, and this class's
 * occupant is VabStreamObj__OnBodyReady.
 */
typedef s32 (*VabStreamObjOnBodyReadyFn)(VabStreamObj *self, s32 done);

/**
 * @brief VabStreamObj's method table: FileResource's slots, with a ctor that
 * takes the bank's base path (no extension), then ten of its own.
 *
 * loadFile (+0x058) and requestLoadFile (+0x06C) are NULL in the static
 * table; SetActiveDataSource fills them from the active driver.
 */
struct VabStreamObjMethods {
    FILERESOURCE_SLOTS(VabStreamObj, (VabStreamObj * self, char *path));
    /* +0x07C */ void (*loadVagAttrs)(VabStreamObj *self); /**< @see VabStreamObj__LoadVagAttrs */
    /* +0x080 */ s32 (*playTone)(VabStreamObj *self, s32 index, s32 vol,
                                 s32 endVol); /**< @see VabStreamObj__PlayTone */
    /* +0x084 */ s32 (*stopVoice)(VabStreamObj *self, s32 voice); /**< @see VabStreamObj__StopVoice */
    /* +0x088 */ s32 (*mute)(VabStreamObj *self);                 /**< @see VabStreamObj__Mute */
    /* +0x08C */ s32 (*unmute)(VabStreamObj *self);               /**< @see VabStreamObj__Unmute */
    /* +0x090 */ void (*slot90)(void); /**< @see VabStreamObj__NoOpSlot90 */
    /* +0x094 */ void (*slot94)(void); /**< @see VabStreamObj__NoOpSlot94 */
    /* +0x098 */ void (*slot98)(void); /**< @see VabStreamObj__NoOpSlot98 */
    /* +0x09C */ void (*setPitchOffset)(VabStreamObj *self, s32 octave); /**< @see VabStreamObj__SetPitchOffset */
}; /* 39 slots, 0xA0 bytes */

/**
 * @brief One VAB sound bank, loaded from disc through the active data source
 * and played through Sony's libsnd (class id 0xA03). A FileResource subclass
 * and a sibling of the drivers (CdDriver, NullDriver), not derived from
 * either; no class derives from it. Methods in src/sound/vab_sound.c.
 *
 * **Loading.** Like every data-source client, its ctor and finalize chain to
 * the active driver's (GetActiveDataSourceMethods), and GetVabStreamObjMethods
 * is in sDataSourceClientGetters. The ctor copies the base path to
 * `baseFilename` and asks the driver for "<base>.VH" (requestLoadFile) in
 * VABSTREAM_LOAD_HEADER. The driver calls onRequestDone
 * (VabStreamObj__AdvanceLoadState) when a request completes: in the header
 * state, with CD_FLAG_LOAD_FILE_DONE in `flags`, it opens the header
 * (SsVabOpenHead) and loads "<base>.VB" (loadFile) in VABSTREAM_LOAD_BODY; in
 * that state it transfers the body (SsVabTransBody) and calls processBuffer,
 * VabStreamObj__OnBodyReady, which waits for the transfer and runs
 * loadVagAttrs to cache the bank's VagAtr records per program.
 *
 * **libsnd.** The first bank constructed initialises libsnd (SsInit, the
 * score size table, a 60 Hz tick); the first whose attributes load starts it
 * (SsStart, the master volume). Finalizing the last open bank while no WBgm
 * is active ends it (SsEnd, SsQuit).
 *
 * **Playing.** playTone(index, vol, endVol) keys on program `index >> 4`,
 * tone `index & 0xF`, at the tone's centre note plus `pitchOffset`, and
 * returns the voice or -1. stopVoice(voice) keys one voice off, or every
 * voice when `voice` >= 24. The holders keep the object as whatever their
 * own field type is (TaskCore::sound, TimedTask::sound, DreamSys::soundObj,
 * WBgm::vab) and cast to VabStreamObj * where they call through it. The
 * SoundCueSet functions take this object first, but are free functions.
 */
struct VabStreamObj {
    FILERESOURCE_FIELDS(VabStreamObjMethods);
    /* +0x02C */ VabStreamVabHdr vabHdr;       /**< SsUtGetVabHdr's copy of the bank's header. */
    /* +0x04C */ VabStreamVagAtr *vagAttrPool; /**< vabHdr.vs VagAtr records. */
    /* +0x050 */ VabStreamVagAtr **progVagTable; /**< vabHdr.ts pointers into vagAttrPool, one per program. */
    /* +0x054 */ s16 vabId; /**< SsVabOpenHead / SsVabTransBody's id; WBgm opens its SEQ on it. */
    /* +0x056 */ s16 muted; /**< Set by Mute, cleared by Unmute. */
    /* +0x058 */ u16 attrsReady;          /**< Set by OnBodyReady; WBgm waits on it. */
    /* +0x05A */ u16 bodyTransferPending; /**< Set when SsVabTransBody succeeds, cleared by OnBodyReady. */
    /* +0x05C */ void *baseFilename; /**< The ctor's copy of the path; freed once the .VB is requested. */
    /* +0x060 */ s32 pitchOffset; /**< Semitones added to a tone's centre note. */
}; /* 0x64 bytes: New_VabStreamObj. `buffer` holds the loaded .VH, then the .VB. */

/** @brief VabStreamObj's method table (see VabStreamObjMethods). */
extern VabStreamObjMethods gVabStreamObjMethods;

/**
 * @brief Returns VabStreamObj's method table.
 * @return &gVabStreamObjMethods.
 */
extern VabStreamObjMethods *GetVabStreamObjMethods(void);

/**
 * @brief Allocates a VabStreamObj from the BMemPMgr pool and constructs it.
 * @param path The bank's base path, without extension, or NULL.
 * @return The new object, or NULL when the pool is exhausted.
 */
VabStreamObj *New_VabStreamObj(char *path);

/**
 * @brief Constructor (slot +0x008): the active driver's ctor, the fields
 * cleared and the pitch offset set for octave 0; libsnd initialised if this
 * is the first bank; the open-bank count raised; and, with a path, the path
 * copied and "<path>.VH" requested.
 * @param self The object being constructed.
 * @param path The bank's base path, without extension, or NULL.
 */
void VabStreamObj__VabStreamObj(VabStreamObj *self, char *path);

/**
 * @brief Finalizer (slot +0x00C): closes the bank, lowers the open-bank
 * count, ends libsnd when it reaches 0 and no WBgm is active, frees the
 * attribute tables and the path, then the active driver's finalize.
 * @param self The object being finalized.
 */
void VabStreamObj__Finalize(VabStreamObj *self);

/**
 * @brief Slot +0x064, onRequestDone: advances the load. Header state, file
 * loaded: opens the header and loads the body. Body state, file loaded:
 * transfers the body to the SPU and, when that starts, calls processBuffer.
 * @param self The bank whose request completed.
 */
void VabStreamObj__AdvanceLoadState(VabStreamObj *self);

/**
 * @brief Slot +0x078 (processBuffer): with a body transfer pending and
 * `done` set, waits for the transfer, sets `attrsReady` and runs
 * loadVagAttrs.
 * @param self The bank.
 * @param done Nonzero to finish the transfer.
 * @return 1 when the transfer was finished, else 0.
 */
s32 VabStreamObj__OnBodyReady(VabStreamObj *self, s32 done);

/**
 * @brief Slot +0x07C: once `attrsReady`, frees the body buffer, takes the
 * header buffer back as `buffer`, reads the bank's header and caches every
 * program's VagAtr records in `vagAttrPool` and `progVagTable`; the first
 * bank to get here starts libsnd and sets the master volume. Any libsnd or
 * allocation failure stops it where it is.
 * @param self The bank.
 */
void VabStreamObj__LoadVagAttrs(VabStreamObj *self);

/**
 * @brief Slot +0x080: keys on the tone a packed index names, at `vol`, and
 * ramps it to `endVol`.
 * @param self   The bank.
 * @param index  `program << 4 | tone`; negative plays nothing.
 * @param vol    Key-on volume, 0..127.
 * @param endVol Volume the tone ramps to, 0..127.
 * @return The voice, or -1.
 */
s32 VabStreamObj__PlayTone(VabStreamObj *self, s32 index, s32 vol, s32 endVol);

/**
 * @brief Slot +0x084: keys one voice off, or every voice when `voice` is 24
 * or more.
 * @param self  The bank.
 * @param voice The voice PlayTone returned.
 * @return -1, the "no voice" value callers store back.
 */
s32 VabStreamObj__StopVoice(VabStreamObj *self, s32 voice);

/**
 * @brief Slot +0x088: mutes libsnd, unless this bank already did.
 * @param self The bank.
 * @return The `muted` flag afterwards (nonzero).
 */
s32 VabStreamObj__Mute(VabStreamObj *self);

/**
 * @brief Slot +0x08C: unmutes libsnd, if this bank muted it.
 * @param self The bank.
 * @return SsSetMute's result when it unmuted, else 0.
 */
s32 VabStreamObj__Unmute(VabStreamObj *self);

/** @brief Slot +0x090: empty; nothing calls it. */
void VabStreamObj__NoOpSlot90(void);
/** @brief Slot +0x094: empty; nothing calls it. */
void VabStreamObj__NoOpSlot94(void);
/** @brief Slot +0x098: empty; nothing calls it. */
void VabStreamObj__NoOpSlot98(void);

/**
 * @brief Slot +0x09C: sets `pitchOffset` to `octave * 12 - 24`, so octave 2
 * plays a tone at its centre note and each step away shifts it an octave.
 * @param self   The bank.
 * @param octave Octave number.
 */
void VabStreamObj__SetPitchOffset(VabStreamObj *self, s32 octave);

/**
 * @brief The number of VabStreamObjs constructed and not yet finalized.
 * @return The open-bank count.
 */
extern s32 GetOpenVabCount(void);

/**
 * @brief The libsnd tick rate the first bank's ctor set, for callers timing
 * in ticks.
 * @return 60 (SS_TICK60) once a bank exists.
 */
extern s32 GetSsTicksPerSecond(void);

#endif
