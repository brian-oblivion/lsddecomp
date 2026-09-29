#ifndef SOUND_CUE_SET_H
#define SOUND_CUE_SET_H

/**
 * @file sound_cue_set.h
 * @brief SoundCueSet, a three-voice sound cue an owner drives once per tick
 * through a callback, and the three functions that run it.
 *
 * The callback writes a slot's program (and optionally its octave and
 * volumes) on the ticks it wants a tone, timing them on `tick`, which it may
 * set to -1 to restart the count. The owners: Entity embeds one and its
 * callbacks are gEntityMoodHandlerTable's Entity__MoodCueNN handlers
 * (entity.h); dream_scene.c's style-cue slots embed one and install
 * sStyleCueCallbacks' StyleCueNN; DreamSys embeds one and installs
 * DreamSys__SoundCueCallback.
 *
 * Both proximity helpers that feed `attenuation` (Entity__GetProximityRatio,
 * ComputeStyleCueFalloff) scale a distance into 0..attenuationSteps, so
 * attenuation = attenuationSteps leaves each volume only its remainder
 * (vol % attenuationSteps).
 */

#include "common.h"

typedef struct SoundCueSet SoundCueSet;

/**
 * @brief A cue's per-tick callback: fills in the slots' requests for this
 * tick.
 */
typedef void (*SoundCueCallbackFn)(void *owner, SoundCueSet *set);

/**
 * @brief One voice of a cue: the request the callback fills in each tick,
 * and the voice that was keyed for it.
 */
typedef struct SoundCueSlot {
    /* +0x00 */ s32 voice; /**< playTone's result, stopVoice's argument; VAB_NO_VOICE when none. */
    /* +0x04 */ s32 program; /**< Request: >= 0 plays VAB program `program` (tone 0); SOUND_CUE_NONE; SOUND_CUE_STOP. */
    /* +0x08 */ s32 octave; /**< Request: setPitchOffset's argument; reset to 0 each tick. */
    /* +0x0C */ s32 vol;    /**< Request: the key-on volume before attenuation; reset each tick. */
    /* +0x10 */ s32 endVol; /**< Request: the volume the tone ramps to, before attenuation; reset each tick. */
} SoundCueSlot;             /* 0x14 bytes */

/** @name Cue requests
 * SoundCueSlot::program's two requests that are not a VAB program. @{ */
#define SOUND_CUE_NONE (-1) /**< no request (ServiceSoundCueSet's reset value) */
#define SOUND_CUE_STOP (-2) /**< stop the slot's voice */
/** @} */

/** SoundCueSet::attenuationSteps for a new cue (InitSoundCueSet). */
#define SOUND_CUE_ATTENUATION_STEPS 10

/** @name Default cue volumes
 * SoundCueSlot::vol and endVol as ServiceSoundCueSet resets them each tick:
 * playTone keys the tone at vol and ramps it to endVol (SsUtAutoVol);
 * 127 is libsnd's full volume. @{ */
#define SOUND_CUE_DEFAULT_VOL 127    /**< the key-on volume */
#define SOUND_CUE_DEFAULT_END_VOL 64 /**< the volume the tone ramps to */

/** @} */

/**
 * @brief A three-voice sound cue. Not a class: it has no method table, and
 * the functions that run it are free functions taking the sound object (a
 * VabStreamObj) first.
 */
struct SoundCueSet {
    /* +0x00 */ s32 tag; /**< The owner's tag while running (Entity: moodIndex + 1); 0 when stopped; serviced only while > 0. */
    /* +0x04 */ s32 tick;    /**< Zeroed by InitSoundCueSet, advanced after each service pass. */
    /* +0x08 */ void *owner; /**< The callback's first argument. */
    /* +0x0C */ SoundCueCallbackFn callback; /**< Called once per service pass. */
    /* +0x10 */ s32 attenuation; /**< Zeroed before each callback; each volume loses vol / attenuationSteps per step; < 0 skips keying this tick. */
    /* +0x14 */ s32 attenuationSteps;  /**< SOUND_CUE_ATTENUATION_STEPS, set by InitSoundCueSet. */
    /* +0x18 */ SoundCueSlot slots[3]; /**< The three voices. */
}; /* 0x54 bytes */

struct VabStreamObj; /* include/vab_stream_obj.h */

/**
 * @brief Starts a cue, unless one is already running: stores the tag, owner
 * and callback, frees every slot's voice (VAB_NO_VOICE), zeroes the tick and sets
 * attenuationSteps.
 * @param sound    The VabStreamObj whose voices the cue plays (unused here).
 * @param set      The cue.
 * @param tag      The owner's tag; serviced only while positive.
 * @param owner    The callback's first argument.
 * @param callback Called once per service pass.
 * @return 1 when the cue was started, 0 when one was already running.
 */
extern s32 InitSoundCueSet(struct VabStreamObj *sound, SoundCueSet *set, s32 tag, void *owner,
                           SoundCueCallbackFn callback);

/**
 * @brief Stops every slot's voice and clears the tag, so the cue may be
 * started again.
 * @param sound The VabStreamObj whose voices the cue plays.
 * @param set   The cue.
 */
extern void FlushSoundCueSet(struct VabStreamObj *sound, SoundCueSet *set);

/**
 * @brief One tick of a running cue (tag > 0).
 *
 * Resets every slot's request (SOUND_CUE_NONE, octave 0,
 * SOUND_CUE_DEFAULT_VOL, SOUND_CUE_DEFAULT_END_VOL) and the attenuation,
 * calls callback(owner, set), then, unless the callback left attenuation
 * negative, keys the requests: for each slot with a program it stops the
 * slot's old voice, sets the pitch offset from the octave and plays the
 * program's tone 0, each volume reduced by (vol / attenuationSteps) *
 * attenuation, keeping the voice; SOUND_CUE_STOP stops the slot's voice
 * instead. Last it advances `tick`.
 * @param sound The VabStreamObj whose voices the cue plays.
 * @param set   The cue.
 */
extern void ServiceSoundCueSet(struct VabStreamObj *sound, SoundCueSet *set);

#endif
