/*
 * ROUND 42 CORRECTION (2026-09-15) -- READ BEFORE ANY "BLOCKED" LINE BELOW:
 * every claim in this comment that a function is BLOCKED by `gp_rel`,
 * `nop_mflo_mfhi` or `addiu_at` is STALE.  All three constructs are RESOLVED
 * by pinned maspsx flags (CLAUDE.md, "Open toolchain blockers");
 * `tools/nearmiss.py` reports them tagged (RESOLVED-not-a-blocker) and counts
 * none of them.  Any "do NOT spend attempts on these" directive below is
 * therefore RETRACTED: those functions are ordinary matching work, and most
 * carry a mechanism-correct partial derivation already.  The rest of this
 * comment still stands -- only the blocker verdicts are withdrawn.
 * Screen: `python3 tools/nearmiss.py`, round 43 (2026-09-15).
 *
 * code_179d8_b -- window [60..79] of the original 274-function code_179d8
 * monolith, originally 0x194E0..0x1A1BC (vram 0x80028CE0..0x800299BC).
 *
 * ROUND 34 (head): NINETEEN of the twenty functions left this unit.  Everything
 * from func_80028CE0 (CdSetDebug) through func_800293F8 (CdPosToInt) is
 * `libcd/sys.o` (Psy-Q 3.3), which starts four functions earlier in
 * code_179d8_h and is now linked from the object.  Sixteen of them had been
 * matched as C and three -- CdControl (func_80028DF0), CdControlF
 * (func_80028F38), CdControlB (func_80029074) -- were INCLUDE_ASM stalls with
 * about 1200 lines of derivation between them that could never have closed.
 * The C is gone because Sony's object owns those bytes now (CLAUDE.md: never
 * write C for a function a Sony object owns); the reports are kept, retitled
 * CONVERTED.  The unit is now 0x19C78..0x1A1BC and holds ONE function,
 * getintr, which still owns jtbl_800109F8 and so the 0x11F8 rodata
 * attach.  The "low-level serial/link driver" reading below was written
 * about the whole window and is now mostly a reading of libcd itself.
 *
 * Carved round 16 by blocker DENSITY, not by "next": code_179d8 is 44%
 * blocked in aggregate but the blockers CLUSTER, so the aggregate says
 * nothing about any particular window. This one screened 16/20 clean.
 * NONE OF THE THREE "BLOCKED" FUNCTIONS IS BLOCKED ANY MORE.  All three
 * were blocked on `addiu_at` ALONE, and `addiu_at` was RESOLVED in round 21
 * (maspsx `--addiu-at`; docs/research/addiu-at-blocker.md).  Re-screened
 * with `python3 tools/nearmiss.py` on 2026-09-08 (round 24):
 *   func_80028CF8  MATCHED    func_80028D30  MATCHED
 *   getintr  MATCHED round 70 (docs/match-reports/getintr.md)
 * The previous version of this comment said all three "are already stubbed
 * as match reports", which by round 24 was a stale DIRECTIVE over free
 * ground; their stubs are gone.
 * func_800292F4 was misclassified nop_mflo_mfhi by an inverted screen
 * (round 16 head correction) -- it is fresh ground, not blocked. It
 * contains mult->mfhi (the hazard-slot direction, not a blocker), retail's
 * signed-divide-by-constant idiom.
 *
 * getintr owns jtbl_800109F8, whose sub-slot of the 0xFD8 rodata
 * region is ATTACHED to this unit in the splat yaml. Leave that alone.
 *
 * This slice was cut at ROM-address boundaries, so it has no reason to
 * align with class boundaries -- expect it to span more than one class,
 * and identify each with tools/classtable.py rather than assuming one.
 *
 * Declarations: keep anything that encodes THIS unit's reading of a class
 * next to the code, in this file. Do not create a shared code_179d8*.h --
 * the sibling slices are staffed independently and a shared header is what
 * makes their merges collide.
 */
#include "common.h"

/* getintr -- CD-ROM interrupt-cause dispatcher.  This is libcd's
 * bios.c `getintr` (build 1.71, 1995-12, on no SDK disc, so it cannot be
 * linked as an object -- docs/research/psyq-sdk-objects.md) and is matched
 * as C instead.  Declarations are this unit's own view. */
extern s32 D_8006D610;
extern s32 D_8006D614;
extern u8 D_8006D61D;
extern s32 D_8006D6C0[]; /* 0/1 flag table, selector 0..0x1B, same index family as D_8006D620 */
extern s32 D_8006D7C0[]; /* 0/1 flag table, selector 0..0x1B */

extern s32 D_8006D60C; /* last status byte (resp[0]) */
extern s32 D_8006D608;
extern char *D_8006D620[];

/* CD-ROM controller port pointers. */
extern volatile u8 *D_8006D8C0;
extern volatile u8 *D_8006D8C4;
extern volatile u8 *D_8006D8C8;
extern volatile u8 *D_8006D8CC;
extern u8 D_8006D8D8[2];
extern volatile u8 D_8006D8D9;
extern volatile u8 D_8006D8DA;

/* Per-cause last-response mailboxes, 8 bytes each, contiguous. */
extern u8 D_8008B3CC[];
extern u8 D_8008B3D4[];
extern u8 D_8008B3DC[];

extern void puts(const char *arg0);
extern void printf(const char *fmt, ...);

/* Debug/log strings, all in FD8.rodata, referenced as symbols. */
extern const char D_800109B0[]; /* "DiskError: " */
extern const char D_800109BC[]; /* "com=%s,code=(%02x:%02x)\n" */
extern const char D_800109D8[]; /* "CDROM: unknown intr" */
extern const char D_800109EC[]; /* "(%d)\n" */

/* 8-byte response copy with a null guard on dst (2.6.3 does not fold
 * `&array != NULL`).  It must be an INLINE FUNCTION, not a macro: as a
 * do{}while(0) macro every site swapped the dst and counter registers
 * (round 70); the inline's parameter pseudos give retail's allocation. */
static __inline__ void copy8(u8 *d, const u8 *s)
{
    s32 i;
    if (d != NULL) {
        for (i = 7; i != -1; i--) {
            *d++ = *s++;
        }
    }
}

s32 getintr(void)
{
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

    if (cause != 3 || D_8006D7C0[D_8006D61D] != 0) {
        if (!(D_8006D60C & 0x10) && (resp[0] & 0x10)) {
            D_8006D614++;
        }
        /* The volatile read keeps resp[0] a QImode value, so its
         * zero-extension survives as retail's `andi v0,v0,0xff`; flags is
         * then CSE'd from the value just stored.  resp[1] is a plain read. */
        D_8006D60C = *(volatile u8 *)&resp[0];
        D_8006D610 = resp[1];
        flags = D_8006D60C & 0x1D;
    }

    if (cause == 5) {
        puts(D_800109B0);
        if (D_8006D608 > 0) {
            printf(D_800109BC, D_8006D620[D_8006D61D], D_8006D60C, D_8006D610);
        }
    }

    switch (cause) {
    case 3:
        if (flags != 0) {
            *(volatile u8 *)D_8006D8D8 = 5;
            copy8(D_8008B3CC, resp);
            return 2;
        }
        if (D_8006D6C0[D_8006D61D] != 0) {
            *(volatile u8 *)D_8006D8D8 = 3;
            copy8(D_8008B3CC, resp);
            return 1;
        }
        *(volatile u8 *)D_8006D8D8 = 2;
        copy8(D_8008B3CC, resp);
        return 2;

    case 2: {
        u8 v;
        if (flags != 0) {
            v = 5;
        } else {
            v = 2;
        }
        D_8006D8D8[0] = v;
        copy8(D_8008B3CC, resp);
        return 2;
    }

    case 1:
        D_8006D8D9 = (flags != 0) ? 5 : 1;
        copy8(D_8008B3D4, resp);
        return 4;

    case 4:
        D_8006D8DA = 4;
        *(volatile u8 *)&D_8006D8D9 = D_8006D8DA;
        copy8(D_8008B3DC, resp);
        copy8(D_8008B3D4, resp);
        return 4;

    case 5:
        D_8006D8D9 = 5;
        *(volatile u8 *)D_8006D8D8 = D_8006D8D9;
        copy8(D_8008B3CC, resp);
        copy8(D_8008B3D4, resp);
        return 6;

    default:
        puts(D_800109D8);
        printf(D_800109EC, cause);
        return -1;
    }
}
