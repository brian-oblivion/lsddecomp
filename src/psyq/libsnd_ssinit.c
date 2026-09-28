/*
 * libsnd_ssinit -- the head of Sony's libsnd/ssinit module, carried as C
 * because no SDK disc carries the build retail linked.
 *
 * _SsInit is the sound-system init: it resets the interrupt callbacks,
 * brings up the SPU cold (SpuInit) or hot (SpuInitHot), writes the
 * templates D_8006DC5C (into all 24 voice register blocks) and D_8006DC6C
 * (into the control block), starts the voice manager for 24 voices
 * (SpuVmInit), clears the mark-callback table and resets libsnd's globals. SsInit and
 * SsInitHot are its two one-line entry points, _SsInit(0) and _SsInit(1).
 *
 * The rest of libsnd/ssinit is the first half of
 * libsnd_ssinit_libapi_counter.c: retail links Sony's sstable object
 * (SsSetTableSize) between the two.
 *
 * Data: _snd_openflag and _snd_ev_flag are pinned by name in
 * config/psyq-objects.ld. D_8006DC5C/D_8006DC6C are read only by _SsInit,
 * a Sony function, so they keep their placeholder names. D_80090368 is
 * libsnd's mark-callback table (SsMarkCallbackProc [32][16], <libsnd.h>'s
 * type), which SsSetMarkCallback fills and ContNrpn1 calls through.
 *
 * The jump table at rodata 0x14D8 is SsSetTickMode's and is attached to
 * libsnd_ssinit_libapi_counter in the yaml, not to this unit.
 */
#include "common.h"
#include <libetc.h>
#include <libsnd.h>
#include <libspu.h>

extern void SpuInitHot(void); /* Sony libspu/s_ih; not in this SDK's LIBSPU.H */
extern void SpuVmInit(s32 arg0);
extern u16 D_8006DC5C[8];
extern u16 D_8006DC6C[0x10];
extern s32 VBLANK_MINUS;
extern s32 _snd_openflag;
extern s32 _snd_use_vsync_cb;
extern s32 _snd_use_interrupt_id;
extern s32 _snd_1per2;
extern void (*_snd_vsync_cb)(void);
extern s32 _snd_video_mode;
extern s32 _snd_ev_flag;

/* The mark callbacks SsSetMarkCallback installs, one per (access number,
 * sequence number); _SsInit clears them all.  Same table ContNrpn1
 * (libsnd_seqread) calls through. */
extern SsMarkCallbackProc D_80090368[0x20][16];

/*
 * Sound-system init.  Reached only through SsInit (arg0 = 0) and
 * SsInitHot (arg0 = 1) below.
 *
 * DO NOT "TIDY" THE LOOP VARIABLES -- the pairing is byte-load-bearing.
 * Retail reuses exactly two counter pseudos across all three loops and SWAPS their outer/inner roles in the last one: `i` is the outer
 * counter of loops 1-2 and the INNER counter of loop 3, `j` the inner counter
 * of loop 1 and the OUTER counter of loop 3.  Writing loop 3 as
 * `for (i ...) for (j ...)` costs 35 words to register renames; splitting
 * them into per-loop names costs the function's size outright.  Likewise the
 * `i = 0;` before each loop is a statement in its own right, not a `for`
 * init clause: retail zeroes the counter BEFORE loading the source base.
 */
void _SsInit(s32 arg0) {
    s32 i, j;
    u16 *base;
    u16 *src;
    u16 *reg;

    ResetCallback();

    if (arg0 == 0) {
        SpuInit();
    } else {
        SpuInitHot();
    }

    /* Stamp the same 8-halfword template into all 24 SPU voice register
       blocks (0x1F801C00, stride 0x10 -- `reg` runs straight through). */
    reg = (u16 *)0x1F801C00;
    i = 0;
    base = D_8006DC5C;
    for (; i < 0x18; i++) {
        j = 0;
        src = base;
        for (; j < 8; j++) {
            *reg = *src;
            src++;
            reg++;
        }
    }

    /* Then 16 consecutive halfwords into the SPU control block at 0x1F801D80. */
    reg = (u16 *)0x1F801D80;
    i = 0;
    src = D_8006DC6C;
    for (; i < 0x10; i++) {
        *reg = *src;
        src++;
        reg++;
    }

    SpuVmInit(0x18);

    for (j = 0; j < 0x20; j++) {
        for (i = 15; i >= 0; i--) {
            D_80090368[j][i] = NULL;
        }
    }

    VBLANK_MINUS = 0x3C;
    _snd_openflag = 0;
    _snd_use_vsync_cb = 0;
    _snd_use_interrupt_id = -1;
    _snd_1per2 = 0;
    _snd_vsync_cb = NULL;
    _snd_video_mode = GetVideoMode();
    _snd_ev_flag = 0;
}

void SsInit(void) {
    _SsInit(0);
}

void SsInitHot(void) {
    _SsInit(1);
}
