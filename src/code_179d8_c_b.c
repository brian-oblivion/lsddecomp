/*
 * code_179d8_c_b -- the sound-driver "sequencer timer" cluster: a software
 * playback clock built on the PSX root counters (RCnt) and interrupt
 * controller (IRQ), used to pace the game's music sequencer independently
 * of vsync. `SsSetTickMode`/`_SsStart` (the latter still a stall)
 * pick a tick rate and arm a root counter at it; `SsStart`/
 * `SsStart2` are its two callers' one-line wrappers; `_SsTrapIntrVSync`/
 * `_SsSeqCalledTbyT_1per2` are the two interrupt-time handlers it can
 * register (one chains a previously-saved handler, the other halves the
 * firing rate by toggling); `SsEnd` tears the whole thing down.
 * `SetRCnt`/`GetRCnt`/`StartRCnt`/`StopRCnt`/`ResetRCnt` are Sony's
 * libapi/counter module compiled into game text from a library build the
 * SDK discs do not carry (identified by module order and KERNEL.H; see the
 * symbols file), and everything above is built on them. `QuitSpu` is
 * unrelated -- a one-line forwarder to Sony's `SpuQuit` (libspu/s_q) -- kept
 * here only because it falls in this address range.
 *
 * the tail half of the old code_179d8_c slice, split off in round 33
 * (2026-09-12) when Sony's `libsnd/sstable.o` was linked into the middle of
 * it. File 0x22D88..0x23500, vram 0x80032588..0x80032D00.
 *
 * WHY THE SPLIT EXISTS. `func_800323A8` (120w) sat between SsInitHot and
 * SsSetTickMode and is Sony's `SsSetTableSize`; the object covers exactly
 * those 120 words. A placed object cannot live inside a `c` segment, so the
 * slice had to become [c][o][c] and the second `c` needed its own name. The
 * first half kept `code_179d8_c`.
 *
 * THE RODATA ATTACH CAME WITH THIS HALF, AND THAT IS THE WHOLE REASON THIS
 * COMMENT EXISTS. `SsSetTickMode` owns `jtbl_80010CD8`, whose sub-slot of the
 * 0xFD8 rodata region is attached in the splat yaml. That attach pointed at
 * `code_179d8_c`; SsSetTickMode is now HERE, so the attach was moved to
 * `code_179d8_c_b`. Left behind it would have produced
 * `undefined reference to '.L800325xx'` -- the routine carve failure Gate 2
 * in docs/PARALLEL-RUNS.md documents. Leave it alone.
 *
 * This slice was cut at ROM-address boundaries, so it has no reason to align
 * with class boundaries -- expect it to span more than one class, and
 * identify each with tools/classtable.py rather than assuming one.
 *
 * Declarations: keep anything that encodes THIS unit's reading of a class
 * next to the code, in this file. Do not create a shared code_179d8*.h -- the
 * sibling slices are staffed independently and a shared header is what makes
 * their merges collide.
 */
#include "common.h"

extern s32 gSeqTimerModeFlag;
extern s32 gSeqTimerRateMode;
extern s32 gVideoMode;
extern u32 VBLANK_MINUS;

/* VBLANK_MINUS values SsSetTickMode's rate table selects between.
 * gVideoMode (Psy-Q `GetVideoMode`) is 0/1, and cases 0/4/5 pick between
 * SEQ_TICKRATE_50/SEQ_TICKRATE_60 by it -- named by value only, not by an
 * NTSC/PAL claim this file has no direct evidence for. */
#define SEQ_TICKRATE_50  0x32
#define SEQ_TICKRATE_60  0x3c
#define SEQ_TICKRATE_120 0x78
#define SEQ_TICKRATE_240 0xf0

void SsSetTickMode(s32 a0)
{
    s32 cmd;

    if (a0 & 0x1000) {
        gSeqTimerModeFlag = 1;
        gSeqTimerRateMode = a0 & 0xFFF;
    } else {
        gSeqTimerModeFlag = 0;
        gSeqTimerRateMode = a0;
    }

    cmd = gSeqTimerRateMode;

    if (cmd < 6) {
        if ((u32)cmd < 6) {
            switch (cmd) {
            case 4:
                VBLANK_MINUS = SEQ_TICKRATE_50;
                if (gVideoMode == 1) {
                    gSeqTimerRateMode = 5;
                } else {
                    gSeqTimerRateMode = SEQ_TICKRATE_50;
                }
                return;
            case 1:
                VBLANK_MINUS = SEQ_TICKRATE_60;
                if (gVideoMode == 0) {
                    gSeqTimerRateMode = 5;
                } else {
                    gSeqTimerRateMode = SEQ_TICKRATE_60;
                }
                return;
            case 3:
                VBLANK_MINUS = SEQ_TICKRATE_120;
                return;
            case 2:
                VBLANK_MINUS = SEQ_TICKRATE_240;
                return;
            case 5:
                if (gVideoMode == 0) {
                    VBLANK_MINUS = SEQ_TICKRATE_60;
                } else if (gVideoMode == 1) {
                    VBLANK_MINUS = SEQ_TICKRATE_50;
                } else {
                    VBLANK_MINUS = SEQ_TICKRATE_60;
                }
                return;
            case 0:
                if (gVideoMode == 0) {
                    VBLANK_MINUS = SEQ_TICKRATE_60;
                } else if (gVideoMode == 1) {
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

INCLUDE_ASM("asm/nonmatchings/code_179d8_c_b", _SsStart);

extern void _SsStart(s32 arg0);

void SsStart(void)
{
    _SsStart(1);
}

void SsStart2(void)
{
    _SsStart(0);
}

extern void EnterCriticalSection(void);
extern void VSyncCallback(void (*cb)(void));
extern void (*InterruptCallback(s32 arg0, void (*callback)(void)))(void);
extern void ExitCriticalSection(void);
extern s32 gSeqTimerModeFlag;
extern s32 gSeqTimerRateFlag;
extern s32 gSeqTimerStopPending;
extern s32 gSeqTimerId;
extern void (*gSeqTimerChainedCallback)(void);

void SsEnd(void)
{
    s32 v;

    if (gSeqTimerModeFlag != 0) {
        return;
    }

    gSeqTimerRateFlag = 0;
    EnterCriticalSection();

    if (gSeqTimerStopPending != 0) {
        VSyncCallback(0);
        gSeqTimerStopPending = 0;
    } else {
        v = gSeqTimerId;
        if (v != -1) {
            if (v != 0) {
                InterruptCallback(v, NULL);
            } else {
                InterruptCallback(0, gSeqTimerChainedCallback);
            }
            gSeqTimerId = -1;
        }
    }

    ExitCriticalSection();
}

extern void SpuQuit(void);

void QuitSpu(void)
{
    SpuQuit();
}

/* Sony's `SsSeqCalledTbyT` (`libsnd/sscall`), linked from the SDK object
 * since round 34. Local view, never a shared header. */
extern void SsSeqCalledTbyT(void);
extern void (*gSeqTimerChainedCallback)(void);

void _SsTrapIntrVSync(void)
{
    if (gSeqTimerChainedCallback != NULL) {
        gSeqTimerChainedCallback();
    }
    SsSeqCalledTbyT();
}

extern s32 gSeqTimerDividerFlag;

void _SsSeqCalledTbyT_1per2(void)
{
    if (gSeqTimerDividerFlag == 0) {
        gSeqTimerDividerFlag = 1;
    } else {
        gSeqTimerDividerFlag = 0;
        SsSeqCalledTbyT();
    }
}

/* Shadow copy of the three PSX root-counter register blocks (COUNT/MODE/
 * TARGET, each a hardware halfword, 0x10 apart -- matches the real
 * 0x1F801100/0x1F801110/0x1F801120 hardware spacing). gRCntRegs is a
 * pointer to this table, not the table itself.
 *
 * The three hardware fields are `volatile` because they ARE memory-mapped
 * registers, and that is load-bearing for matching as well as correct:
 * without it GCC reorders the table load against the index arithmetic and
 * hoists stores into unconditional-jump delay slots retail leaves as `nop`.
 * It closed GetRCnt and ResetRCnt in round 32 -- the first of
 * which had been filed for three rounds as an unfixable register-identity
 * residue -- and it SUBSUMES the `__asm__("")` barrier SetRCnt used to
 * carry (removed in the same round; SetRCnt still verifies 40/40).
 * See docs/match-reports/ResetRCnt.md for the mechanism. */
typedef struct {
    volatile u16 count;              /* 0x0 */
    u8  pad2[0x4 - 0x2];
    volatile u16 mode;                /* 0x4 */
    u8  pad6[0x8 - 0x6];
    volatile u16 target;               /* 0x8 */
    u8  padA[0x10 - 0xA];
} RCntEntry;

extern RCntEntry *gRCntRegs;

s32 SetRCnt(s32 n, s16 target, u32 mode)
{
    s32 idx = (u16)n;
    u16 md = 0x48;
    u32 isLow;

    if (idx >= 3) {
        return 0;
    }

    isLow = (u32)idx < 2;
    gRCntRegs[idx].mode = 0;
    gRCntRegs[idx].target = target;

    if (isLow) {
        if (mode & 0x10) {
            md = 0x49;
        }
        if (!(mode & 1)) {
            md |= 0x100;
        }
    } else if (idx == 2) {
        if (!(mode & 1)) {
            md = 0x248;
        }
    }

    if (mode & 0x1000) {
        md |= 0x10;
    }

    gRCntRegs[idx].mode = md;
    return 1;
}

s32 GetRCnt(s32 n)
{
    s32 idx = (u16)n;
    RCntEntry *base;

    if (idx >= 3) {
        return 0;
    }
    base = gRCntRegs;
    return base[idx].count;
}

/* Shadow of the PSX interrupt controller pair at 0x1F801070/0x1F801074
 * (I_STAT/I_MASK). gIrqRegs is a pointer to this pair, not the pair
 * itself -- same "pointer-to-hardware-block" idiom as gRCntRegs above.
 * gRCntIrqMasks holds the per-index IRQ mask bit (root counters 0/1/2 use
 * indices 0-2 -> Tmr0/Tmr1/Tmr2 IRQ bits 0x10/0x20/0x40; index 3 is the
 * 0x1 VBLANK bit). */
typedef struct {
    volatile u32 stat; /* 0x0, I_STAT */
    volatile u32 mask; /* 0x4, I_MASK */
} IrqRegs;

extern IrqRegs *gIrqRegs;
extern u32 gRCntIrqMasks[4];

s32 StartRCnt(u16 which)
{
    s32 idx = which;
    IrqRegs *reg = gIrqRegs;

    reg->mask |= gRCntIrqMasks[idx];
    return idx < 3;
}

s32 StopRCnt(u16 which)
{
    s32 idx = which;
    IrqRegs *reg = gIrqRegs;

    reg->mask &= ~gRCntIrqMasks[idx];
    return 1;
}

s32 ResetRCnt(s32 n)
{
    s32 idx = (u16)n;
    RCntEntry *base;

    if (idx >= 3) {
        return 0;
    }
    base = gRCntRegs;
    base[idx].count = 0;
    return 1;
}
