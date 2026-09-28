/*
 * libsnd_play -- Sony libsnd `play`: Snd_play, the whole module.
 *
 * Snd_play forwards (access, seq) to SeqPlay, the per-tick scheduler in
 * libsnd_seqread.c. libsnd/play.o is this one function on the 3.0, 3.3 and
 * 3.5 discs (3.5 calls it _SsSndPlay). It is carried as C rather than
 * linked because the same body occurs at four places in the executable, so
 * the object alone does not say which one is Snd_play.
 */
#include "common.h"
#include "libsnd_internal.h"

void Snd_play(s16 access, s16 seq) {
    SeqPlay(access, seq);
}
