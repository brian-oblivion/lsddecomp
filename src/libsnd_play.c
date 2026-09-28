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

extern s32 SeqPlay(s16 a0, s16 a1); /* arity-ok: the definition's third parameter is never read (docs/match-reports/SeqPlay.md), and its return type is void there */

s32 Snd_play(s16 a0, s16 a1) {
    return SeqPlay(a0, a1);
}
