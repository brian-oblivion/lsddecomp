/*
 * libsnd_play -- Sony libsnd `play`: Snd_play, the whole module.
 *
 * Snd_play forwards (access, seq) to SeqPlay, the per-tick scheduler in
 * libsnd_seqread.c. Every byte here is Sony's: libsnd/play.o is 0x2C of
 * .text, one function, byte-identical on the 3.0, 3.3 and 3.5 discs (3.5
 * calls it _SsSndPlay), and this unit is exactly 0x2C. It is carried as C
 * rather than linked because `psyq_sdk.py match` finds that body at four
 * places in retail (AMBIGUOUS x4); the Snd_play symbol is pinned here by
 * the libsnd object that calls it by relocation.
 */
#include "common.h"
#include "libsnd_internal.h"

void Snd_play(s16 access, s16 seq) {
    SeqPlay(access, seq);
}
