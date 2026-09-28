/*
 * libcd_bios -- Sony's libcd `bios.c`, the low-level CD-ROM controller
 * driver, carried as C. The game links a libcd build (RCS ids of December
 * 1995) that no SDK disc carries, so this object cannot be linked from lib/
 * the way its neighbours are (docs/research/psyq-sdk-objects.md); every
 * function here is matched, or queued, as C instead. Sony's names
 * throughout.
 *
 * What it holds, in bios.c's own order:
 *   - getintr: reads the controller's interrupt cause and response bytes,
 *     records them per cause, and reports the event to its caller;
 *   - CD_sync / CD_ready: wait (with a timeout and a diagnostic print) for
 *     a command to complete or for data to become ready;
 *   - CD_cw: issue one controller command with its parameter bytes;
 *   - CD_vol, CD_shell, CD_flush, CD_initvol, CD_initintr, CD_init: volume
 *     set-up, shell-open recovery, draining the controller, and the reset
 *     sequence that installs `callback` as the CD interrupt handler;
 *   - cd_read_retry, CD_readm, CD_readsync, CD_datasync, CD_getsector: the
 *     sector-read state machine and its DMA transfer;
 *   - CD_set_test_parmnum, callback, cb_read: a one-word setter, the interrupt
 *     handler, and the per-sector read callback.
 */
#include "common.h"

#include <libetc.h>
#include <stdio.h>

/* A CD interrupt callback, as the driver calls it: the interrupt's status
 * byte and the 8-byte result it read. */
typedef void (*CdIntrCallback)(u8 status, u8 *result);

/* bios.o's named globals. CD_status and CD_status1 are the first two
 * response bytes of the last interrupt; CD_nopen counts shell openings. */
extern CdIntrCallback CD_cbsync;  /* called on a command's completion */
extern CdIntrCallback CD_cbready; /* called when data is ready */
extern CdIntrCallback CD_cbread;  /* called when a CD_readm read ends */
extern s32 CD_debug;              /* diagnostic print level */
extern s32 CD_status;
extern s32 CD_status1;
extern s32 CD_nopen;
extern u8 CD_pos[4];      /* the position CdlSetloc (command 2) last set */
extern u8 CD_mode;        /* the mode CdlSetmode (command 0xE) last set */
extern u8 CD_com;         /* the command last issued */
extern char *CD_comstr[]; /* each command's name, for the diagnostics */
extern char *CD_intstr[]; /* each interrupt state's name */

/* bios.o's unnamed per-command tables, 32 words each, indexed by the
 * command. CD_cw reads D_8006D840 both directly and as
 * D_8006D740[cmd + 0x40] (docs/match-reports/CD_cw.md). */
extern s32 D_8006D6C0[]; /* nonzero: a second interrupt, not the acknowledge, completes the command */
extern s32 D_8006D740[]; /* nonzero: issuing the command clears the ready state */
extern s32 D_8006D7C0[]; /* nonzero: the command's acknowledge interrupt updates CD_status */
extern s32 D_8006D840[]; /* the number of parameter bytes the command takes */

/* Pointers to the hardware registers the driver uses. */
extern volatile u8 *D_8006D8C0;  /* 0x1F801800, the CD-ROM controller's index/status port */
extern volatile u8 *D_8006D8C4;  /* 0x1F801801 */
extern volatile u8 *D_8006D8C8;  /* 0x1F801802 */
extern volatile u8 *D_8006D8CC;  /* 0x1F801803 */
extern volatile s32 *D_8006D8D0; /* 0x1F801020, the common bus delay */
extern volatile u16 *D_8006D8D4; /* 0x1F801C00, the SPU's register block, indexed by halfword */
extern volatile s32 *D_8006D924; /* 0x1F801018, the CD-ROM bus delay */
extern volatile s32 *D_8006D928; /* 0x1F8010F0, the DMA control register */
extern volatile s32 *D_8006D92C; /* 0x1F8010B0, DMA channel 3 (CD-ROM): address */
extern volatile s32 *D_8006D930; /* 0x1F8010B4, block control */
extern volatile s32 *D_8006D934; /* 0x1F8010B8, channel control */

/* The state getintr leaves for each kind of interrupt: [0] sync, [1] ready,
 * [2] the second ready flag CD_ready tests. The functions that store [1]
 * and [2] directly do it through their own symbols. */
extern u8 D_8006D8D8[3];
extern volatile u8 D_8006D8D9;
extern volatile u8 D_8006D8DA;

/* The read state machine (CD_readm, cd_read_retry, cb_read). */
extern s32 D_8006D8A4;     /* CD_set_test_parmnum's word */
extern s32 D_8006D8DC[10]; /* first of 10 consecutive words zeroed by a pointer walk;
                            * D_8006D8E0..D_8006D904 are the other nine, each
                            * already individually named -- not a real array. */
extern s32 D_8006D8E0;
extern s32 D_8006D8E4;
extern volatile s32 D_8006D8E8;
extern volatile s32 D_8006D8EC;
extern volatile s32 D_8006D8F0;
extern volatile s32 D_8006D8F4;
extern s32 D_8006D8F8;
extern CdIntrCallback D_8006D8FC; /* CD_cbsync, saved while a read runs */
extern CdIntrCallback D_8006D900; /* CD_cbready, saved while a read runs */
extern s32 D_8006D904;
extern u8 D_8006D908[];
extern u8 D_8006D90C[];

/* Each interrupt's 8-byte result: [0] sync, [1] ready, [2] the second
 * ready. */
extern u8 D_8008B3CC[3][8];

/* The timeout of the wait in progress: its deadline in VSync ticks, a spin
 * count, and the waiting function's name for the diagnostic. bios.o calls
 * the group Alarm; it stays three words here.
 * MATCHING: a struct over them makes CD_readsync share one base register. */
extern s32 D_8008B3E4;
extern s32 D_8008B3E8;
extern char *D_8008B3EC;

/* Diagnostic strings. */
extern char D_80010984[];
extern char D_80010994[];
extern char D_800109B0[]; /* "DiskError: " */
extern char D_800109BC[]; /* "com=%s,code=(%02x:%02x)\n" */
extern char D_800109D8[]; /* "CDROM: unknown intr" */
extern char D_800109EC[]; /* "(%d)\n" */
extern char D_80010A14[]; /* "CD_ready" */
extern char D_80010A20[]; /* "%s...\n" */
extern char D_80010A28[]; /* "%s: no param\n" */
extern char D_80010A38[]; /* "CD_cw" */
extern char D_80010A40[];
extern char D_80010A50[];
extern char D_80010A94[];
extern char D_80010AA0[];
extern char D_80010AAC[];
extern char D_80010ABC[];
extern char D_80010AD8[];
extern char D_80010AE0[];

/* libetc's intr.o; <libetc.h> does not declare it. */
extern void (*InterruptCallback(s32 arg0, void (*callback)(void)))(void);

/* This file's own functions, used before their definitions. */
s32 getintr(void);
s32 CD_sync(s32 arg0, s32 arg1);
s32 CD_cw(s32 arg0, u8 *arg1, u8 *arg2, s32 arg3);
void CD_flush(void);
s32 cd_read_retry(void);
s32 CD_datasync(s32 arg0);
void callback(void);
void cb_read(s32 arg0, s32 arg1);

/* 8-byte response copy with a null guard on dst (2.6.3 does not fold
 * `&array != NULL`). MATCHING: an inline function, not a do{}while(0)
 * macro, which swaps the dst and counter registers at every site. */
static __inline__ void copy8(u8 *d, const u8 *s) {
    s32 i;
    if (d != NULL) {
        for (i = 7; i != -1; i--) {
            *d++ = *s++;
        }
    }
}

s32 getintr(void) {
    volatile u8 cause;
    u8 resp[8];
    s32 i;
    s32 flags;

    *D_8006D8C0 = 1;
    cause = *D_8006D8CC & 7;
    if (cause == 0) {
        return 0;
    }
    flags = 0;
    while (cause != (*D_8006D8CC & 7)) {
        cause = *D_8006D8CC & 7;
    }

    for (i = 0; i < 8 && (*D_8006D8C0 & 0x20); i++) {
        resp[i] = *D_8006D8C4;
    }
    for (; i < 8; i++) {
        resp[i] = 0;
    }

    *D_8006D8C0 = 1;
    *D_8006D8CC = 7;
    *D_8006D8C8 = 7;

    if (cause != 3 || D_8006D7C0[CD_com] != 0) {
        if (!(CD_status & 0x10) && (resp[0] & 0x10)) {
            CD_nopen++;
        }
        /* The volatile read keeps resp[0] a QImode value, so its
         * zero-extension survives as retail's `andi v0,v0,0xff`; flags is
         * then CSE'd from the value just stored.  resp[1] is a plain read. */
        CD_status = *(volatile u8 *)&resp[0];
        CD_status1 = resp[1];
        flags = CD_status & 0x1D;
    }

    if (cause == 5) {
        puts(D_800109B0);
        if (CD_debug > 0) {
            printf(D_800109BC, CD_comstr[CD_com], CD_status, CD_status1);
        }
    }

    switch (cause) {
        case 3:
            if (flags != 0) {
                *(volatile u8 *)D_8006D8D8 = 5;
                copy8(D_8008B3CC[0], resp);
                return 2;
            }
            if (D_8006D6C0[CD_com] != 0) {
                *(volatile u8 *)D_8006D8D8 = 3;
                copy8(D_8008B3CC[0], resp);
                return 1;
            }
            *(volatile u8 *)D_8006D8D8 = 2;
            copy8(D_8008B3CC[0], resp);
            return 2;

        case 2: {
            u8 v;
            if (flags != 0) {
                v = 5;
            } else {
                v = 2;
            }
            D_8006D8D8[0] = v;
            copy8(D_8008B3CC[0], resp);
            return 2;
        }

        case 1:
            D_8006D8D9 = (flags != 0) ? 5 : 1;
            copy8(D_8008B3CC[1], resp);
            return 4;

        case 4:
            D_8006D8DA = 4;
            *(volatile u8 *)&D_8006D8D9 = D_8006D8DA;
            copy8(D_8008B3CC[2], resp);
            copy8(D_8008B3CC[1], resp);
            return 4;

        case 5:
            D_8006D8D9 = 5;
            *(volatile u8 *)D_8006D8D8 = D_8006D8D9;
            copy8(D_8008B3CC[0], resp);
            copy8(D_8008B3CC[1], resp);
            return 6;

        default:
            puts(D_800109D8);
            printf(D_800109EC, cause);
            return -1;
    }
}

INCLUDE_ASM("asm/nonmatchings/psyq/libcd_bios", CD_sync);

#ifdef NON_MATCHING
/* NON_MATCHING: 178 words against retail's 180
 * (docs/match-reports/CD_ready.md). */
s32 CD_ready(s32 arg0, s32 arg1) {
    char **table;
    u8 *state;
    u8 *state1;
    u8 *state2;
    s32 counter;
    s32 result;
    s32 flags;
    u8 savedState;
    u8 flag1;
    u8 flag2;
    u8 *dst;
    const u8 *src;
    s32 i;

    D_8008B3E4 = VSync(-1) + 0x1E0;
    table = CD_intstr;
    state = D_8006D8D8;
    state1 = state + 1;
    state2 = state + 2;
    D_8008B3E8 = 0;
    D_8008B3EC = D_80010A14;

    do {
        if (D_8008B3E4 < VSync(-1)) {
            goto timeout;
        }
        counter = D_8008B3E8;
        D_8008B3E8 = counter + 1;
        if (counter <= 0x1E0000) {
            goto success;
        }
    timeout:
        puts(D_80010984);
        printf(D_80010994, D_8008B3EC, CD_comstr[CD_com], table[state[0]], table[state[1]]);
        CD_flush();
        result = -1;
        goto skip_timeout;
    success:
        result = 0;
    skip_timeout:
        if (result != 0) {
            return result;
        }

        if (CheckCallback() != 0) {
            savedState = *D_8006D8C0 & 3;
            for (;;) {
                flags = getintr();
                if (flags == 0) {
                    break;
                }
                if (flags & 4) {
                    if (CD_cbready != NULL) {
                        CD_cbready(*state1, D_8008B3CC[1]);
                    }
                }
                if (flags & 2) {
                    if (CD_cbsync != NULL) {
                        CD_cbsync(*state, D_8008B3CC[0]);
                    }
                }
            }
            *D_8006D8C0 = savedState;
        }

        flag2 = *state2;
        if (flag2 == 0) {
            goto checkFlag1;
        }
        *state2 = 0;
        __asm__("");
        src = D_8008B3CC[2];
        if (arg1 == 0) {
            goto ret2;
        }
        dst = (u8 *)arg1;
        for (i = 7; i != -1; i--) {
            *dst = *src;
            dst++;
            src++;
        }
    ret2:
        return flag2;

    checkFlag1:
        flag1 = state2[-1];
        if (flag1 == 0) {
            continue;
        }
        state2[-1] = 0;
        __asm__("");
        dst = (u8 *)arg1;
        src = D_8008B3CC[1];
        if (dst == 0) {
            goto ret1;
        }
        for (i = 7; i != -1; i--) {
            *dst = *src;
            dst++;
            src++;
        }
    ret1:
        return flag1;
    } while (arg0 == 0);

    return 0;
}
#else
INCLUDE_ASM("asm/nonmatchings/psyq/libcd_bios", CD_ready);
#endif

#ifdef NON_MATCHING
/* NON_MATCHING: 284 words against retail's 282, 97 equal at the same
 * index (docs/match-reports/CD_cw.md). */
s32 CD_cw(s32 arg0, u8 *arg1, u8 *arg2, s32 arg3) {
    char **table;
    volatile u8 *state;
    volatile u8 *state1;
    s32 counter;
    s32 result;
    s32 flags;
    u8 savedState;
    u8 *dst;
    const u8 *src;
    s32 i;

    if (CD_debug >= 2) {
        printf(D_80010A20, CD_comstr[arg0 & 0xFF]);
    }

    if (D_8006D840[arg0 & 0xFF] != 0 && arg1 == 0) {
        if (CD_debug > 0) {
            printf(D_80010A28, CD_comstr[arg0 & 0xFF]);
        }
        return -2;
    }

    CD_sync(0, 0);

    if ((arg0 & 0xFF) == 2) {
        src = (const u8 *)arg1;
        for (i = 0; i < 4; i++) {
            CD_pos[i] = *src;
            src++;
        }
    }

    ((volatile u8 *)D_8006D8D8)[0] = 0;

    if (D_8006D740[arg0 & 0xFF] != 0) {
        D_8006D8D9 = 0;
    }

    *D_8006D8C0 = 0;

    if (D_8006D740[(arg0 & 0xFF) + 0x40] > 0) {
        for (i = 0; i < D_8006D740[(arg0 & 0xFF) + 0x40]; i++) {
            *D_8006D8C8 = ((u8 *)arg1)[i];
        }
    }

    CD_com = (u8)arg0;
    *D_8006D8C4 = (u8)arg0;

    if (arg3 != 0) {
        return 0;
    }

    D_8008B3E4 = VSync(-1) + 0x1E0;
    state = D_8006D8D8;
    D_8008B3E8 = 0;
    D_8008B3EC = D_80010A38;

    if (*state == 0) {
        table = CD_intstr;
        state1 = state + 1;
        do {
            if (D_8008B3E4 < VSync(-1)) {
                goto timeout3;
            }
            counter = D_8008B3E8;
            D_8008B3E8 = counter + 1;
            if (counter <= 0x1E0000) {
                goto success3;
            }
        timeout3:
            puts(D_80010984);
            src = table[state[1]];
            printf(D_80010994, D_8008B3EC, CD_comstr[CD_com], table[state[0]], src);
            CD_flush();
            result = -1;
            goto skip_timeout3;
        success3:
            result = 0;
        skip_timeout3:
            if (result != 0) {
                return result;
            }
            if (CheckCallback() != 0) {
                savedState = *D_8006D8C0 & 3;
                for (;;) {
                    flags = getintr();
                    if (flags == 0) {
                        break;
                    }
                    if (flags & 4) {
                        if (CD_cbready != NULL) {
                            CD_cbready(*state1, D_8008B3CC[1]);
                        }
                    }
                    if (flags & 2) {
                        if (CD_cbsync != NULL) {
                            CD_cbsync(*state, D_8008B3CC[0]);
                        }
                    }
                }
                *D_8006D8C0 = savedState;
            }
        } while (*state == 0);
    }

    if (((volatile u8 *)D_8006D8D8)[0] == 2 && (arg0 & 0xFF) == 0xE) {
        CD_mode = *(u8 *)arg1;
    }

    dst = (u8 *)arg2;
    src = D_8008B3CC[0];
    if (dst != NULL) {
        for (i = 7; i != -1; i--) {
            *dst = *src;
            dst++;
            src++;
        }
    }

    return (((volatile u8 *)D_8006D8D8)[0] == 5) ? -1 : 0;
}
#else
INCLUDE_ASM("asm/nonmatchings/psyq/libcd_bios", CD_cw);
#endif


s32 CD_vol(u8 *arg0) {
    *D_8006D8C0 = 2;
    *D_8006D8C8 = arg0[0];
    *D_8006D8CC = arg0[1];
    *D_8006D8C0 = 3;
    *D_8006D8C4 = arg0[2];
    *D_8006D8C8 = arg0[3];
    *D_8006D8CC = 0x20;
    return 0;
}

void CD_shell(void) {
    CdIntrCallback saved;
    s32 counter = 0;

    if (D_8006D904 < CD_nopen) {
        saved = CD_cbsync;
        CD_cbsync = 0;

        while (CD_status & 0x10) {
            if ((u8)counter == 0) {
                puts(D_80010A40);
            }
            counter++;
            CD_cw(1, 0, 0, 0);
        }

        while (CD_cw(0x16, D_8006D908, 0, 0)) {
            CD_cw(1, 0, 0, 0);
            puts(D_80010A50);
        }

        CD_cbsync = saved;
        D_8006D904 = CD_nopen;
    }
}

void CD_flush(void) {
    volatile u8 *q;

    *D_8006D8C0 = 1;
    while (*D_8006D8CC & 7) {
        *D_8006D8C0 = 1;
        *D_8006D8CC = 7;
        *D_8006D8C8 = 7;
    }

    D_8006D8DA = 0;
    q = &D_8006D8D9;
    CD_mode = 0;
    *q = D_8006D8DA;
    /* Keeps the D_8006D8C0 pointer load and the D_8006D8D8[0] = 2 store
     * below the CD_mode / D_8006D8D9 stores; without it GCC hoists them above. */
    __asm__("");
    D_8006D8D8[0] = 2;
    *D_8006D8C0 = 0;
    *D_8006D8CC = 0;
    *D_8006D8D0 = 0x1325;
}

s32 CD_initvol(void) {
    u8 buf[4];

    if (D_8006D8D4[0xDC] == 0 && D_8006D8D4[0xDD] == 0) {
        D_8006D8D4[0xC0] = 0x3FFF;
        D_8006D8D4[0xC1] = 0x3FFF;
    }
    D_8006D8D4[0xD8] = 0x3FFF;
    D_8006D8D4[0xD9] = 0x3FFF;
    D_8006D8D4[0xD5] = 0xC001;

    buf[2] = 0x80;
    buf[0] = 0x80;
    buf[3] = 0;
    buf[1] = 0;
    *D_8006D8C0 = 2;
    *D_8006D8C8 = buf[0];
    *D_8006D8CC = buf[1];
    *D_8006D8C0 = 3;
    *D_8006D8C4 = buf[2];
    *D_8006D8C8 = buf[3];
    *D_8006D8CC = 0x20;
    return 0;
}

void CD_initintr(void) {
    s32 *p;
    s32 i;

    CD_cbready = 0;
    CD_cbsync = 0;
    CD_status1 = 0;
    CD_status = 0;
    p = D_8006D8DC;
    for (i = 9; i != -1; i--) {
        *p = 0;
        p++;
    }
    ResetCallback();
    InterruptCallback(2, callback);
}

#ifdef NON_MATCHING
/* NON_MATCHING: 171/196 words, length exact. Residue: list scheduling:
 * retail splits CD_cw(1,0,0,0)'s argument set-up from its own a3/jal by
 * about 90 bytes, and neither call position tried reproduces it
 * (docs/match-reports/CD_init.md). */
s32 CD_init(void) {
    s32 *p;
    s32 i;
    volatile u8 *q;
    CdIntrCallback saved;
    s32 counter;

    puts(D_80010A94);
    printf(D_80010AA0, D_8006D90C);

    CD_com = 0;
    CD_mode = 0;
    CD_cbready = 0;
    CD_cbsync = 0;
    CD_status1 = 0;
    CD_status = 0;
    p = D_8006D8DC;
    for (i = 9; i != -1; i--) {
        *p = 0;
        p++;
    }
    ResetCallback();
    InterruptCallback(2, callback);

    *D_8006D8C0 = 1;
    while (*D_8006D8CC & 7) {
        *D_8006D8C0 = 1;
        *D_8006D8CC = 7;
        *D_8006D8C8 = 7;
    }

    CD_cw(1, 0, 0, 0);

    D_8006D8DA = 0;
    q = &D_8006D8D9;
    CD_mode = 0;
    *q = D_8006D8DA;
    __asm__("");
    D_8006D8D8[0] = 2;
    *D_8006D8C0 = 0;
    *D_8006D8CC = 0;
    *D_8006D8D0 = 0x1325;

    counter = 0;
    if (CD_status & 0x10) {
        CD_cw(1, 0, 0, 0);
    }

    if (D_8006D904 < CD_nopen) {
        saved = CD_cbsync;
        CD_cbsync = 0;

        while (CD_status & 0x10) {
            if ((u8)counter == 0) {
                puts(D_80010A40);
            }
            counter++;
            CD_cw(1, 0, 0, 0);
        }

        while (CD_cw(0x16, D_8006D908, 0, 0)) {
            CD_cw(1, 0, 0, 0);
            puts(D_80010A50);
        }

        CD_cbsync = saved;
        D_8006D904 = CD_nopen;
    }

    if (CD_cw(0xA, 0, 0, 0) != 0) {
        return -1;
    }
    if (CD_cw(0xC, 0, 0, 0) != 0) {
        return -1;
    }
    return -(CD_sync(0, 0) != 2);
}
#else
INCLUDE_ASM("asm/nonmatchings/psyq/libcd_bios", CD_init);
#endif

#ifdef NON_MATCHING
/* NON_MATCHING: 215/223 words, length exact. Residue: a loop-setup
 * scheduling swap (p2 computed from $a0 before vs. after the move into $s5)
 * and a register-identity swap in the final D_8006D8F4 = -1 block
 * (docs/match-reports/cd_read_retry.md). The n/saved sinks below are
 * written on every path before their next read. */
s32 cd_read_retry(void) {
    s32 n;
    s32 *tmp;
    s32 *pRetry;
    volatile s32 *p2;
    s32 saved;
    s32 counter;
    volatile u8 *q;
    u8 buf;

    tmp = D_8006D8DC;
    n = *tmp;
    CD_cbready = 0;
    CD_cbsync = 0;
    *tmp = n - 1;
    __asm__("");

    if (n > 0) {
        pRetry = tmp;
        p2 = pRetry + 4;
        do {
            if (*pRetry < 7) {
                counter = 0;
                puts(D_80010AAC);
                printf(D_80010ABC, *pRetry, CD_pos[0], CD_pos[1], CD_pos[2]);

                if (D_8006D904 < CD_nopen) {
                    saved = (s32)CD_cbsync;
                    CD_cbsync = 0;

                    while (CD_status & 0x10) {
                        if ((u8)counter == 0) {
                            puts(D_80010A40);
                        }
                        counter++;
                        CD_cw(1, 0, 0, 0);
                    }

                    while (CD_cw(0x16, D_8006D908, 0, 0)) {
                        CD_cw(1, 0, 0, 0);
                        puts(D_80010A50);
                    }

                    CD_cbsync = (CdIntrCallback)saved;
                    D_8006D904 = CD_nopen;
                }

                if (CD_cw(9, 0, 0, 0) != 0) {
                    goto tail;
                }
                if (CD_cw(2, CD_pos, 0, 0) != 0) {
                    goto tail;
                }
            }

            *D_8006D8C0 = 1;
            while (*D_8006D8CC & 7) {
                *D_8006D8C0 = 1;
                *D_8006D8CC = 7;
                *D_8006D8C8 = 7;
            }

            D_8006D8DA = 0;
            q = &D_8006D8D9;
            CD_mode = 0;
            *q = D_8006D8DA;
            __asm__("");
            D_8006D8D8[0] = 2;
            *D_8006D8C0 = 0;
            *D_8006D8CC = 0;
            *D_8006D8D0 = 0x1325;

            {
                s32 v0 = p2[0];
                buf = (u8)v0;
                n = ((u8)v0 != CD_mode);
                if (n) {
                    saved = (s32)&buf;
                    if (CD_cw(0xE, (u8 *)saved, 0, 0) != 0) {
                        goto tail;
                    }
                }
            }

            CD_cbready = (CdIntrCallback)cb_read;
            p2[-1] = p2[-2];
            CD_cw(6, 0, 0, 1);
            p2[2] = p2[-3];
            p2[3] = VSync(-1) + 0x1E0;
            return p2[2];

        tail:
            tmp = D_8006D8DC;
            n = *tmp;
            *tmp = n - 1;
            __asm__("");
        } while (n > 0);
    }

    {
        volatile s32 *pF4 = &D_8006D8F4;
        *pF4 = -1;
        return *pF4;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/psyq/libcd_bios", cd_read_retry);
#endif

s32 CD_readm(s32 arg0, s32 arg1, s32 arg2) {
    s32 t;
    volatile s32 *p;

    p = &D_8006D8EC;
    *p = arg2;
    t = *p & 0x30;

    /* MATCHING: the goto layout and the local `volatile s32 *` pointers
     * keep the three D_8006D8F0 stores as separate blocks, each with its
     * own address computation; an if/else chain or a switch lets GCC merge
     * them into one store. */
    if (t == 0) {
        goto case1;
    }
    if (t == 0x20) {
        goto case2;
    }
    goto case3;
case1:
    D_8006D8F0 = 0x200;
    goto join;
case2:
    D_8006D8F0 = 0x249;
    goto join;
case3: {
    volatile s32 *q3 = &D_8006D8F0;
    *q3 = 0x246;
}
join:

{
    volatile s32 *q4 = &D_8006D8E4;
    *q4 = arg0;
}
    D_8006D8E0 = arg1;
    D_8006D8DC[0] = 8;
    D_8006D8FC = CD_cbsync;
    D_8006D900 = CD_cbready;

    if (CD_status & 0xE0) {
        CD_cw(9, 0, 0, 0);
    }
    CD_sync(0, 0);
    return -(cd_read_retry() < 1);
}

s32 CD_readsync(s32 arg0, s32 arg1) {
    s32 now;
    s32 old;
    s32 flags;
    char **pEC;
    u8 status;
    char **p6A0;
    u8 *p8D8;
    u8 *p8D9;
    s32 *pF8;
    u8 *dst;
    u8 *src;
    s32 i;
    s32 idx0;
    s32 idx1;
    s32 result;

    now = VSync(-1);
    p6A0 = CD_intstr;
    p8D8 = D_8006D8D8;
    p8D9 = &D_8006D8D8[1];
    pF8 = &D_8006D8DC[7];

    D_8008B3E4 = now + 0x1E0;
    D_8008B3E8 = 0;
    D_8008B3EC = D_80010AD8;

    for (;;) {
        now = VSync(-1);
        if (D_8008B3E4 < now) {
            goto timeout;
        }
        old = D_8008B3E8;
        D_8008B3E8 = old + 1;
        if (0x1E0000 >= old) {
            goto success;
        }

    timeout:
        puts(D_80010984);
        idx0 = p8D8[0];
        idx1 = p8D8[1];
        /* Keeps both p8D8 byte loads directly after the puts call, ahead of
         * the printf argument loads; without it the p8D8[0] load sinks below them. */
        __asm__("");
        /* MATCHING: &D_8008B3EC through a local pointer keeps retail's
         * unfolded lui/addiu for this argument; a plain reference folds. */
        pEC = &D_8008B3EC;
        printf(D_80010994, *pEC, CD_comstr[CD_com], p6A0[idx0], p6A0[idx1]);
        CD_flush();
        result = -1;
        goto after_diag;

    success:
        result = 0;

    after_diag:
        if (result != 0) {
            return result;
        }
        if (CheckCallback() != 0) {
            status = (u8)(*D_8006D8C0 & 3);
            for (;;) {
                flags = getintr();
                if (flags == 0) {
                    break;
                }
                if ((flags & 4) && CD_cbready != 0) {
                    CD_cbready(p8D9[0], D_8008B3CC[1]);
                }
                if ((flags & 2) && CD_cbsync != 0) {
                    CD_cbsync(p8D8[0], D_8008B3CC[0]);
                }
            }
            /* MATCHING: a known-bad construct, do not copy it. p6A0 and pF8
             * are both addresses of globals, so the test is always true and
             * both arms are the same store; cross-jumping folds them into
             * retail's single sb, and the construct only steers register
             * allocation (`status` in $s1 across the loop). The report has
             * the alternatives tried. */
            if (p6A0 || pF8) {
                *D_8006D8C0 = status;
            } else {
                *D_8006D8C0 = status;
            }
        }

        dst = (u8 *)arg1;
        src = D_8008B3CC[1];
        if (dst != 0) {
            for (i = 7; i != -1; i--) {
                *dst = *src;
                src++;
                dst++;
            }
        }

        if (VSync(-1) > pF8[0] + 0x3C) {
            cd_read_retry();
        }
        if (pF8[-1] == 0) {
            CD_datasync(0);
        }
        if (arg0 != 0 || pF8[-1] <= 0) {
            break;
        }
    }

    return pF8[-1];
}

#ifdef NON_MATCHING
/* NON_MATCHING: length exact, 91 words; register identity on the three
 * hoisted table pointers (docs/match-reports/CD_datasync.md). */
s32 CD_datasync(s32 arg0) {
    s32 now;
    s32 ok;
    char **p620;
    u8 *p8D8;
    char **p6A0;

    D_8008B3E4 = VSync(-1) + 0x1E0;
    p620 = CD_comstr;
    p6A0 = CD_intstr;
    p8D8 = D_8006D8D8;
    D_8008B3E8 = 0;
    D_8008B3EC = D_80010AE0;

    for (;;) {
        now = VSync(-1);
        ok = 1;
        if (D_8008B3E4 < now) {
            ok = 0;
        } else {
            D_8008B3E8 = D_8008B3E8 + 1;
            if (0x1E0000 < D_8008B3E8) {
                ok = 0;
            }
        }
        if (!ok) {
            puts(D_80010984);
            /* NON_MATCHING: the last argument reuses the dead `ok` as its
             * register target; a fresh local compiles further from retail. */
            printf(D_80010994, p8D8[0], p6A0[p8D8[1]], p620[CD_com], ok = (s32)p6A0[p8D8[0]]);
            CD_flush();
            return -1;
        }
        if ((*D_8006D934 & 0x1000000) == 0) {
            return 0;
        }
        if (arg0 == 0) {
            continue;
        }
        return 1;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/psyq/libcd_bios", CD_datasync);
#endif

s32 CD_getsector(s32 arg0, s32 arg1) {
    *D_8006D8C0 = 0;
    *D_8006D8CC = 0x80;
    *D_8006D924 = 0x20943;
    *D_8006D8D0 = 0x1323;
    *D_8006D928 |= 0x8000;
    *D_8006D92C = arg0;
    *D_8006D930 = arg1 | 0x10000;
    *D_8006D934 = 0x11000000;

    while (*D_8006D934 & 0x1000000) {
    }

    *D_8006D8D0 = 0x1325;
    return 0;
}

void CD_set_test_parmnum(s32 arg0) {
    D_8006D8A4 = arg0;
}

#ifdef NON_MATCHING
/* NON_MATCHING: 30/56 words, length 1 short. Residue: instruction
 * selection (retail computes &D_8006D8D8 unfolded inside the loop; this
 * folds it) (docs/match-reports/callback.md). */
void callback(void) {
    u8 status;
    s32 flags;
    CdIntrCallback handler;
    volatile u8 *pd9;

    status = (*D_8006D8C0) & 3;
    pd9 = &D_8006D8D9;

    for (;;) {
        flags = getintr();
        if (flags == 0) {
            break;
        }
        if (flags & 4) {
            handler = CD_cbready;
            if (handler != 0) {
                handler(*pd9, D_8008B3CC[1]);
            }
        }
        if (flags & 2) {
            if (CD_cbsync != 0) {
                CD_cbsync(D_8006D8D8[0], D_8008B3CC[0]);
            }
        }
    }
    *D_8006D8C0 = status;
}
#else
INCLUDE_ASM("asm/nonmatchings/psyq/libcd_bios", callback);
#endif

void cb_read(s32 arg0, s32 arg1) {
    volatile s32 *p;
    s32 code;
    s32 dummy;
    volatile s32 *new_var;

    if (arg0 != 1) {
        goto elseBranch;
    }
    p = &D_8006D8F4;
    if (*p <= 0) {
        goto shared;
    }
    CD_getsector(D_8006D8E8, D_8006D8F0);
    D_8006D8E8 = D_8006D8E8 + D_8006D8F0 * 4;
    *p = *p - 1;
    dummy = *p;
    (void)dummy;
    goto shared;
elseBranch: {
    volatile s32 *p2 = &D_8006D8F4;
    *p2 = -1;
}
shared: {
    volatile s32 *pF8 = &D_8006D8F8;
    *pF8 = VSync(-1);
}

    if (D_8006D8F4 < 0 && D_8006D8DC[0] > 0) {
        cd_read_retry();
    }

    if ((*(new_var = &D_8006D8F4)) <= 0) {
        CD_cbsync = D_8006D8FC;
        CD_cbready = D_8006D900;
        CD_cw(9, 0, 0, 0);
        if (CD_cbread != 0) {
            if ((*new_var) == 0) {
                code = 2;
            } else {
                code = 5;
            }
            CD_cbread(code, (u8 *)arg1);
        }
    }
}
