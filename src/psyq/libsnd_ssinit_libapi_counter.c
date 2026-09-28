/*
 * libsnd_ssinit_libapi_counter -- two Sony modules carried as C, both from
 * library builds no SDK disc carries, so neither can be linked from lib/.
 *
 * libsnd/ssinit's sequencer clock (Sony's names, identified in the symbols
 * file against the 3.3 disc): SsSetTickMode picks the tick rate
 * (VBLANK_MINUS) from a mode and the video mode; _SsStart arms either a
 * vsync callback or a root-counter interrupt at that rate, and SsStart /
 * SsStart2 are its two one-line entry points; SsEnd tears it down; SsQuit
 * forwards to SpuQuit; _SsTrapIntrVSync and _SsSeqCalledTbyT_1per2 are the
 * two interrupt-time handlers _SsStart can install (one chains the saved
 * vsync callback, the other runs the sequencer on every second tick).
 *
 * libapi/counter: SetRCnt, GetRCnt, StartRCnt, StopRCnt, ResetRCnt, the
 * root-counter and interrupt-mask accessors the clock above is built on.
 */
#include "common.h"
#include "libsnd_internal.h"
#include <libetc.h>
#include <kernel.h>

/* VBLANK_MINUS values SsSetTickMode's rate table selects between.
 * _snd_video_mode (Psy-Q `GetVideoMode`) is 0/1, and cases 0/4/5 pick between
 * SEQ_TICKRATE_50/SEQ_TICKRATE_60 by it -- named by value only, not by an
 * NTSC/PAL claim this file has no direct evidence for. */
#define SEQ_TICKRATE_50 0x32
#define SEQ_TICKRATE_60 0x3c
#define SEQ_TICKRATE_120 0x78
#define SEQ_TICKRATE_240 0xf0

void SsSetTickMode(long a0) {
    s32 cmd;

    if (a0 & SS_NOTICK) {
        _snd_seq_no_tick = 1;
        _snd_seq_tick_mode = a0 & 0xFFF;
    } else {
        _snd_seq_no_tick = 0;
        _snd_seq_tick_mode = a0;
    }

    cmd = _snd_seq_tick_mode;

    if (cmd < SS_TICKMODE_MAX) {
        if ((u32)cmd < SS_TICKMODE_MAX) {
            switch (cmd) {
                case SS_TICK50:
                    VBLANK_MINUS = SEQ_TICKRATE_50;
                    if (_snd_video_mode == 1) {
                        _snd_seq_tick_mode = SS_TICKVSYNC;
                    } else {
                        _snd_seq_tick_mode = SEQ_TICKRATE_50;
                    }
                    return;
                case SS_TICK60:
                    VBLANK_MINUS = SEQ_TICKRATE_60;
                    if (_snd_video_mode == 0) {
                        _snd_seq_tick_mode = SS_TICKVSYNC;
                    } else {
                        _snd_seq_tick_mode = SEQ_TICKRATE_60;
                    }
                    return;
                case SS_TICK120:
                    VBLANK_MINUS = SEQ_TICKRATE_120;
                    return;
                case SS_TICK240:
                    VBLANK_MINUS = SEQ_TICKRATE_240;
                    return;
                case SS_TICKVSYNC:
                    if (_snd_video_mode == 0) {
                        VBLANK_MINUS = SEQ_TICKRATE_60;
                    } else if (_snd_video_mode == 1) {
                        VBLANK_MINUS = SEQ_TICKRATE_50;
                    } else {
                        VBLANK_MINUS = SEQ_TICKRATE_60;
                    }
                    return;
                case SS_NOTICK0:
                    if (_snd_video_mode == 0) {
                        VBLANK_MINUS = SEQ_TICKRATE_60;
                    } else if (_snd_video_mode == 1) {
                        VBLANK_MINUS = SEQ_TICKRATE_50;
                    } else {
                        VBLANK_MINUS = SEQ_TICKRATE_60;
                    }
                    return;
            }
        } else {
            VBLANK_MINUS = SEQ_TICKRATE_60;
            return;
        }
    }
    VBLANK_MINUS = cmd;
}

INCLUDE_ASM("asm/nonmatchings/psyq/libsnd_ssinit_libapi_counter", _SsStart);

extern void _SsStart(s32 arg0);

void SsStart(void) {
    _SsStart(1);
}

void SsStart2(void) {
    _SsStart(0);
}

extern void (*InterruptCallback(s32 arg0, void (*callback)(void)))(void);

void SsEnd(void) {
    s32 v;

    if (_snd_seq_no_tick != 0) {
        return;
    }

    _snd_1per2 = 0;
    EnterCriticalSection();

    if (_snd_use_vsync_cb != 0) {
        VSyncCallback(0);
        _snd_use_vsync_cb = 0;
    } else {
        v = _snd_use_interrupt_id;
        if (v != -1) {
            if (v != 0) {
                InterruptCallback(v, NULL);
            } else {
                InterruptCallback(0, _snd_vsync_cb);
            }
            _snd_use_interrupt_id = -1;
        }
    }

    ExitCriticalSection();
}

extern void SpuQuit(void);

void SsQuit(void) {
    SpuQuit();
}

/* Sony's `SsSeqCalledTbyT` (`libsnd/sscall`). Local view, never a shared
 * header. */

void _SsTrapIntrVSync(void) {
    if (_snd_vsync_cb != NULL) {
        _snd_vsync_cb();
    }
    SsSeqCalledTbyT();
}

extern s32 D_8006DCA0;

void _SsSeqCalledTbyT_1per2(void) {
    if (D_8006DCA0 == 0) {
        D_8006DCA0 = 1;
    } else {
        D_8006DCA0 = 0;
        SsSeqCalledTbyT();
    }
}

/* Shadow copy of the three PSX root-counter register blocks (COUNT/MODE/
 * TARGET, each a hardware halfword, 0x10 apart -- matches the real
 * 0x1F801100/0x1F801110/0x1F801120 hardware spacing). D_8006DCB0 is a
 * pointer to this table, not the table itself.
 *
 * The three hardware fields are `volatile` because they are memory-mapped
 * registers. MATCHING: without it GCC reorders the table load against the
 * index arithmetic and hoists stores into unconditional-jump delay slots
 * retail leaves as `nop` (docs/match-reports/ResetRCnt.md). */
typedef struct {
    volatile u16 count; /* 0x0 */
    u8 pad2[0x4 - 0x2];
    volatile u16 mode; /* 0x4 */
    u8 pad6[0x8 - 0x6];
    volatile u16 target; /* 0x8 */
    u8 padA[0x10 - 0xA];
} RCntEntry;

extern RCntEntry *D_8006DCB0;

long SetRCnt(unsigned long spec, unsigned short target, long mode) {
    s32 idx = (u16)spec;
    u16 md = 0x48;
    u32 isLow;

    if (idx >= 3) {
        return 0;
    }

    isLow = (u32)idx < 2;
    D_8006DCB0[idx].mode = 0;
    D_8006DCB0[idx].target = target;

    if (isLow) {
        if (mode & RCntMdGATE) {
            md = 0x49;
        }
        if (!(mode & RCntMdSC)) {
            md |= 0x100;
        }
    } else if (idx == 2) {
        if (!(mode & RCntMdSC)) {
            md = 0x248;
        }
    }

    if (mode & RCntMdINTR) {
        md |= 0x10;
    }

    D_8006DCB0[idx].mode = md;
    return 1;
}

long GetRCnt(unsigned long spec) {
    s32 idx = (u16)spec;
    RCntEntry *base;

    if (idx >= 3) {
        return 0;
    }
    base = D_8006DCB0;
    return base[idx].count;
}

/* Shadow of the PSX interrupt controller pair at 0x1F801070/0x1F801074
 * (I_STAT/I_MASK). D_8006DCAC is a pointer to this pair, not the pair
 * itself -- same "pointer-to-hardware-block" idiom as D_8006DCB0 above.
 * D_8006DCB4 holds the per-index IRQ mask bit (root counters 0/1/2 use
 * indices 0-2 -> Tmr0/Tmr1/Tmr2 IRQ bits 0x10/0x20/0x40; index 3 is the
 * 0x1 VBLANK bit). */
typedef struct {
    volatile u32 stat; /* 0x0, I_STAT */
    volatile u32 mask; /* 0x4, I_MASK */
} IrqRegs;

extern IrqRegs *D_8006DCAC;
extern u32 D_8006DCB4[4];

long StartRCnt(unsigned long spec) {
    s32 idx = (u16)spec;
    IrqRegs *reg = D_8006DCAC;

    reg->mask |= D_8006DCB4[idx];
    return idx < 3;
}

long StopRCnt(unsigned long spec) {
    s32 idx = (u16)spec;
    IrqRegs *reg = D_8006DCAC;

    reg->mask &= ~D_8006DCB4[idx];
    return 1;
}

long ResetRCnt(unsigned long spec) {
    s32 idx = (u16)spec;
    RCntEntry *base;

    if (idx >= 3) {
        return 0;
    }
    base = D_8006DCB0;
    base[idx].count = 0;
    return 1;
}
