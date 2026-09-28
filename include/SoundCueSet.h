#ifndef SOUNDCUESET_H
#define SOUNDCUESET_H

/*
 * SoundCueSet -- a three-voice sound cue that an owner object drives once
 * per tick through a callback. Not a class: no method table, and the three
 * functions that operate on it are free functions taking the sound object
 * (a VabStreamObj, include/VabStreamObj.h) first.
 *
 *  - InitSoundCueSet (PlacementGridVabSound.c) starts the cue, unless one is already
 *    running (tag != 0): it stores the owner's tag, owner and callback,
 *    frees every slot's voice (-1), zeroes the tick and sets
 *    attenuationSteps to 10. Returns 1 when it started the cue.
 *  - ServiceSoundCueSet (PlacementGridVabSound.c), once per tick while tag > 0:
 *    resets every slot's request (program -1, octave 0, vol 0x7F,
 *    endVol 0x40) and attenuation, calls callback(owner, set), and then,
 *    unless the callback left attenuation negative, keys the requests: for
 *    each slot with program >= 0 it stops the slot's old voice, sets the
 *    sound object's pitch offset from octave and plays program (tone 0),
 *    each volume reduced by (vol / attenuationSteps) * attenuation, keeping
 *    the returned voice; program -2 stops the slot's voice instead. Last it
 *    advances tick.
 *  - FlushSoundCueSet (PlacementGridVabSound.c) stops every slot's voice and clears
 *    tag, so the set may be started again.
 *
 * The callback therefore writes a slot's program (and optionally octave
 * and volumes) on the ticks it wants a tone, timing them on `tick`, which
 * it may set to -1 to restart the count. The owners: Entity embeds one at
 * +0x09C and its callbacks are gEntityMoodHandlerTable's Entity__MoodCueNN
 * handlers (include/Entity.h); ObjMStyleActor's style-cue slots embed one and
 * install sStyleCueCallbacks' StyleCueNN (ObjMStyleActor.c); DreamSys embeds
 * one and installs DreamSys__SoundCueCallback.
 *
 * Both proximity helpers that feed attenuation (Entity__GetProximityRatio,
 * ComputeStyleCueFalloff) scale a distance into 0..attenuationSteps, so
 * attenuation = attenuationSteps leaves each volume only its remainder
 * (vol % attenuationSteps).
 */

#include "common.h"

typedef struct SoundCueSet SoundCueSet;

typedef void (*SoundCueCallbackFn)(void *owner, SoundCueSet *set);

/* One voice of the cue: the request the callback fills in each tick and the
 * voice that was keyed for it. */
typedef struct SoundCueSlot {
    /* +0x00 */ s32 voice;   /* playTone's result, stopVoice's argument; -1 when none */
    /* +0x04 */ s32 program; /* request: >= 0 plays VAB program `program` (playTone index program << 4, tone 0); -1 none; -2 stops `voice` */
    /* +0x08 */ s32 octave; /* request: setPitchOffset's argument (pitchOffset = octave * 12 - 24); reset to 0 each tick */
    /* +0x0C */ s32 vol; /* request: playTone's vol before attenuation; reset to 0x7F each tick */
    /* +0x10 */ s32 endVol; /* request: playTone's endVol before attenuation; reset to 0x40 each tick */
} SoundCueSlot;             /* 0x14 bytes */

/* SoundCueSlot::program's two requests that are not a VAB program. */
#define SOUND_CUE_NONE (-1) /* no request (ServiceSoundCueSet's reset value) */
#define SOUND_CUE_STOP (-2) /* stop the slot's voice */

/* SoundCueSet::attenuationSteps for a new cue (InitSoundCueSet). */
#define SOUND_CUE_ATTENUATION_STEPS 10

/* SoundCueSlot::vol and endVol as ServiceSoundCueSet resets them each tick:
 * playTone keys the tone at vol and ramps it to endVol (SsUtAutoVol);
 * 127 is libsnd's full volume. */
#define SOUND_CUE_DEFAULT_VOL 127
#define SOUND_CUE_DEFAULT_END_VOL 64

struct SoundCueSet {
    /* +0x00 */ s32 tag; /* the owner's tag while running (Entity: moodIndex + 1); 0 when stopped; serviced only while > 0 */
    /* +0x04 */ s32 tick;    /* zeroed by InitSoundCueSet, advanced after each service pass */
    /* +0x08 */ void *owner; /* the callback's first argument */
    /* +0x0C */ SoundCueCallbackFn callback;
    /* +0x10 */ s32 attenuation; /* zeroed each tick before the callback; each volume loses vol / attenuationSteps per step; < 0 skips keying this tick */
    /* +0x14 */ s32 attenuationSteps; /* 10, set by InitSoundCueSet */
    /* +0x18 */ SoundCueSlot slots[3];
}; /* 0x54 bytes */

struct VabStreamObj; /* include/VabStreamObj.h */

/* The three functions on a cue, all in src/sound/PlacementGridVabSound.c;
 * `sound` is the VabStreamObj whose voices the cue plays. */
extern s32 InitSoundCueSet(struct VabStreamObj *sound, SoundCueSet *set, s32 tag, void *owner,
                           SoundCueCallbackFn callback);
extern void FlushSoundCueSet(struct VabStreamObj *sound, SoundCueSet *set);
extern void ServiceSoundCueSet(struct VabStreamObj *sound, SoundCueSet *set);

#endif
