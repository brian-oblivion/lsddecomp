/*
 * libsnd_ssinit -- the head of Sony's libsnd/ssinit module, carried as C
 * because no SDK disc carries the game's build of it: _SsInit, the
 * sound-system init, and its two entry points SsInit and SsInitHot.
 *
 * The rest of libsnd/ssinit is the first half of
 * libsnd_ssinit_libapi_counter.c: Sony's sstable object (SsSetTableSize)
 * is linked between the two. _ss_MarkCallback is libsnd's mark-callback
 * table (SsMarkCallbackProc [32][16], <libsnd.h>'s type), which
 * SsSetMarkCallback fills and ContNrpn1 calls through.
 */
#include "common.h"
#include "libsnd_internal.h"
#include <libetc.h>
#include <libspu.h>

/* libspu's s_i and s_ih; <libspu.h> declares no functions. */
extern void SpuInit(void);
extern void SpuInitHot(void);

/* The register templates _SsInit writes: one voice's, into all 24, and the
 * control block's. */
extern u16 D_8006DC5C[8];
extern u16 D_8006DC6C[0x10];

/*
 * The sound-system init, reached only through SsInit (arg0 = 0) and
 * SsInitHot (arg0 = 1) below: resets the interrupt callbacks, brings up
 * the SPU cold (SpuInit) or hot (SpuInitHot), writes the voice template
 * into all 24 voice register blocks and the control template into the
 * control block, starts the voice manager for 24 voices (SpuVmInit),
 * clears the mark-callback table and resets libsnd's globals.
 */
/* MATCHING: i and j are shared by all three loops (swapped in the third), each zeroed before its loop. */
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
            _ss_MarkCallback[j][i] = NULL;
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
