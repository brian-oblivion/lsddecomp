/*
 * The SoundCueSet functions (include/sound_cue_set.h: a three-voice sound
 * cue an owner drives once per tick through a callback), in ROM order:
 * InitSoundCueSet, FlushSoundCueSet and ServiceSoundCueSet, which plays
 * each slot's request on the cue's VabStreamObj.
 */
#include "common.h"
#include <libgte.h>
#include "sound_cue_set.h"
#include "vab_stream_obj.h"

s32 InitSoundCueSet(VabStreamObj *sound, SoundCueSet *set, s32 tag, void *owner,
                    SoundCueCallbackFn callback) {
    SoundCueSlot *slot;
    s32 count;
    s32 sentinel;

    if (set->tag != 0) {
        return 0;
    }
    slot = set->slots;
    sentinel = VAB_NO_VOICE;
    count = ARRAY_COUNT(set->slots) - 1;
    set->tag = tag;
    set->owner = owner;
    set->callback = callback;
    do {
        slot->voice = sentinel;
        count--;
        slot++;
    } while (count >= 0);
    set->tick = 0;
    set->attenuationSteps = SOUND_CUE_ATTENUATION_STEPS;
    return 1;
}

void FlushSoundCueSet(VabStreamObj *sound, SoundCueSet *set) {
    s32 i;
    SoundCueSlot *slot;

    slot = set->slots;
    for (i = 0; i < ARRAY_COUNT(set->slots); i++) {
        if (slot->voice >= 0) {
            slot->voice = sound->methods->stopVoice(sound, slot->voice);
        }
        slot++;
    }
    set->tag = 0;
}

void ServiceSoundCueSet(VabStreamObj *sound, SoundCueSet *set) {
    s32 i;
    SoundCueSlot *slot;
    s32 vol;
    s32 endVol;
    s32 toneIndex;

    if (set->tag > 0) {
        /* MATCHING: both loops are do-whiles, entered without a test; a for loop adds
         * a test on entry. This one also steps its count first in the body. */
        i = 0;
        slot = &set->slots[0];
        do {
            i++;
            slot->program = SOUND_CUE_NONE;
            slot->octave = 0;
            slot->vol = SOUND_CUE_DEFAULT_VOL;
            slot->endVol = SOUND_CUE_DEFAULT_END_VOL;
            slot++;
        } while (i < ARRAY_COUNT(set->slots));

        set->attenuation = 0;
        if (set->callback != NULL) {
            set->callback(set->owner, set);
        }

        if (set->attenuation >= 0) {
            slot = &set->slots[0];
            i = 0;
            do {
                if (slot->program >= 0) {
                    if (slot->voice >= 0) {
                        sound->methods->stopVoice(sound, slot->voice);
                    }
                    sound->methods->setPitchOffset(sound, slot->octave);
                    toneIndex = slot->program * VAB_TONES_PER_PROG;
                    vol = slot->vol - (slot->vol / set->attenuationSteps) * set->attenuation;
                    endVol = slot->endVol - (slot->endVol / set->attenuationSteps) * set->attenuation;
                    slot->voice = sound->methods->playTone(sound, toneIndex, vol, endVol);
                } else if (slot->program == SOUND_CUE_STOP && slot->voice >= 0) {
                    sound->methods->stopVoice(sound, slot->voice);
                }
                i++;
                slot++;
            } while (i < ARRAY_COUNT(set->slots));
        }
        set->tick++;
    }
}
