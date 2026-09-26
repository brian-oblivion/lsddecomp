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

/* ---- merged from code_179d8_n ---- */

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
 * code_179d8_n -- functions 80..82 of the original 274-function code_179d8
 * monolith, 0x1A1BC..0x1AB78 (vram 0x800299BC..0x8002A378).  Carved round 26
 * (2026-09-09) out of what had been the `code_179d8_mid` uncarved remainder;
 * renamed on carve because "mid" named a REMAINDER and this is no longer one
 * -- the whole remainder was consumed, so no `code_179d8_mid` segment exists
 * any more.  Do not look for one.
 *
 * Blocker census at carve time, four screens per function (gp_rel,
 * forward nop_mflo_mfhi, `jr $t2` trampoline, jtbl):
 *   CD_sync (161w)  CLEAN
 *   CD_ready (180w)  CLEAN
 *   CD_cw (282w)  CLEAN
 * 3 of 3 clean, zero trivial `jr $ra` leaves.  These are BIG bodies, so this
 * unit is far larger in work than 3 suggests -- budget accordingly.
 *
 * Owns NO switch jump table (zero `jtbl_` references in the slice), so no
 * rodata sub-slot is attached to this unit and the 0x120C slot in the splat
 * yaml stays standalone.
 *
 * It DOES reference plain rodata SYMBOLS: D_80010984, D_80010994,
 * D_80010A0C, D_80010A14, D_80010A20, D_80010A28, D_80010A38.  Several of
 * those are ASCII.  They are SYMBOLS to reference (`extern const char
 * D_XXXXXXXX[];`), never strings to re-type as C literals -- splat has
 * already emitted those bytes and a literal emits a second copy, which
 * shifts the whole image (see CLAUDE.md, the duplicated-rodata-string
 * trap).
 *
 * This unit's own extern declarations are kept LOCAL to this file per the
 * project's multiple-independent-local-views convention.  It shares no
 * project header with any other unit.
 *
 * Round 26 (echo): all three functions were worked to near-misses and
 * STALLED -- see docs/match-reports/CD_sync.md (162/161, 1 word LONG),
 * CD_ready.md (178/180, 2 words short) and CD_cw.md (278/282,
 * 4 words short).  Every residue is an already-characterized GCC 2.6.3
 * quirk (a hoisted-constant register choice, dead-code-eliminated redundant
 * masks, and delay-slot/addressing-mode scheduling) documented in
 * docs/DECOMPILATION_LEARNINGS.md as not fixable by hand C restructuring --
 * read the three reports before re-attempting; they carry the full
 * near-miss bodies and the exact levers already tried.
 *
 * Round 35 (echo): re-screened round 26's stalled bodies against the
 * current tree before re-attempting -- per CLAUDE.md's "BUILD any inherited
 * body ONCE" discipline. Four of the six `extern func_XXXXXXXX` helper
 * declarations round 26's preserved bodies used are now STALE placeholder
 * names -- SDK-object rounds since renamed them to their real Sony symbols
 * (`func_80025900` -> `VSync`, `func_80025AE4` -> `puts`,
 * `func_80024E64` -> `CheckCallback`, `func_80012C20` -> `printf`; see
 * `src/code_179d8_g.c`'s own already-updated local declarations for the
 * same globals). Round 26's bodies, spliced verbatim, would not have LINKED
 * under today's tree -- the "a preserved body's `jal` targets can go STALE
 * across an SDK-object round" hazard from round 31 (DECOMPILATION_LEARNINGS.md).
 * `CD_flush` and `getintr` were NOT renamed (still real game
 * code, still INCLUDE_ASM/matched under those names in sibling units).
 *
 * All three bodies were rebuilt this round with the four names corrected,
 * plus a couple of quick untried levers per rounds 31/33's newer findings
 * (routing the CD_sync hoisted-constant "2" through a separate named
 * local; reading CD_ready's two flag bytes through a `volatile u8 *`
 * cast). All three REPRODUCED their round-26 recorded scores exactly
 * (162/161, 178/180, 278/282) and neither new lever moved anything --
 * consistent with round 33's finding that a GCSE/value-availability hoist
 * (CD_sync's case) and a genuinely-redundant-mask DCE (CD_ready's
 * case, independently re-confirmed here against the live `.s` rather than
 * inherited from a citation to the now-SDK-owned `func_8002B94C`) are both
 * immune to source-level rescue by construction, not by insufficient
 * effort. Re-filed as STALLS; see the three match reports for the updated
 * verdicts and the round-35 addenda.
 *
 * Round 37 (echo): re-confirmed CD_ready's and CD_cw's
 * round-35 bodies by rebuilding each live and checking funcdiff/lsdde.map
 * before trusting either score (both reproduce exactly), then preserved
 * both verbatim in `#if 0` blocks ahead of a permuter search -- neither had
 * been permuter-searched before this round. See the two match reports for
 * the search results.
 */

INCLUDE_ASM("asm/nonmatchings/code_179d8_b", CD_sync);

/* Round 37 (echo): re-splice of the round-35 rebuild, verbatim, to confirm
 * the recorded 178/180 score before a permuter search -- per CLAUDE.md's
 * "build the inherited body before you trust its score" discipline. See
 * docs/match-reports/CD_ready.md for the full derivation; this is a
 * STALL (2 words short, both instances of a redundant `andi` mask after an
 * already-zero-extending `lbu` that GCC's instruction selection elides by
 * choosing the load's destination register directly). Restored to
 * INCLUDE_ASM per project rule -- no score short of byte-exact stays in
 * src/. */
#if 0
extern s32 D_8006D608;
extern u8 D_8006D61D;
extern const char *D_8006D620[];
extern const char *D_8006D6A0[];

extern volatile u8 *D_8006D8C0;
extern u8 D_8006D8D8[3];

extern s32 D_8008B3E4;
extern s32 D_8008B3E8;
extern const char *D_8008B3EC;
extern u8 D_8008B3CC[];
extern u8 D_8008B3D4[];
extern u8 D_8008B3DC[];                /* 8-byte record, this function's second flag's snapshot buffer */

extern void (*D_8006D600)(s32 arg0, void *arg1);
extern void (*D_8006D5FC)(s32 arg0, void *arg1);

extern void CD_flush(void);
extern s32 getintr(void);

extern const char D_80010984[];
extern const char D_80010994[];
extern const char D_80010A14[];        /* "CD_ready" */

s32 CD_ready(s32 arg0, s32 arg1)
{
    const char **table;
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
    table = D_8006D6A0;
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
        printf(D_80010994, D_8008B3EC, D_8006D620[D_8006D61D],
                      table[state[0]], table[state[1]]);
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
                    if (D_8006D600 != NULL) {
                        D_8006D600(*state1, D_8008B3D4);
                    }
                }
                if (flags & 2) {
                    if (D_8006D5FC != NULL) {
                        D_8006D5FC(*state, D_8008B3CC);
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
        src = D_8008B3DC;
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
        src = D_8008B3D4;
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
#endif
INCLUDE_ASM("asm/nonmatchings/code_179d8_b", CD_ready);

/* Round 37 (echo): STALL, now 282/282 (LENGTH exact, no drift into
 * anything downstream) -- up from 278/282, via two stacked permuter-found
 * levers: (1) declaring D_8006D8D8 (and the two local pointers into it,
 * `state`/`state1`) `volatile` closed 3 of the original 4 missing words
 * (278->281/282); (2) a second search from that improved body found that
 * materializing `table[state[1]]` into `src` as its own statement just
 * before the timeout printf call (a dead store -- `src` is unconditionally
 * overwritten before its value is ever read) closes the last word
 * (281->282/282). Raw word-match went DOWN in the process (132/282 ->
 * 98/282) even as length became exact -- see the match report's honest
 * discussion of why LENGTH is still the right thing to have adopted here.
 * Restored to INCLUDE_ASM per project rule. */
#if 0
extern s32 D_8006D608;
extern u8 D_8006D61C;
extern u8 D_8006D61D;
extern const char *D_8006D620[];
extern const char *D_8006D6A0[];
extern u8 D_8006D618[4];               /* 4-byte record, written here for cmd == 2 */
extern s32 D_8006D740[];               /* flag table, indexed by cmd; cmd+0x40 reaches the "needs param" table's
                                         * memory (see the addressing note in the match report) -- do NOT re-split
                                         * this into a second D_8006D840[cmd] access for the cmd+0x40 case */
extern s32 D_8006D840[];               /* "does this command need a param" flag table, indexed by cmd; ONLY
                                         * correct as a direct access at its own (non-+0x40) call site */

extern volatile u8 *D_8006D8C0;
extern volatile u8 *D_8006D8C4;
extern volatile u8 *D_8006D8C8;
extern volatile u8 D_8006D8D8[3];      /* round 37: permuter-found lever, see match report -- volatile here
                                         * (and on state/state1 below) closes 3 of the 4 missing words */
extern u8 D_8006D8D9;

extern s32 D_8008B3E4;
extern s32 D_8008B3E8;
extern const char *D_8008B3EC;
extern u8 D_8008B3CC[];
extern u8 D_8008B3D4[];

extern void (*D_8006D600)(s32 arg0, void *arg1);
extern void (*D_8006D5FC)(s32 arg0, void *arg1);

extern void CD_flush(void);
extern s32 getintr(void);
extern s32 CD_sync(s32 arg0, s32 arg1);          /* defined earlier in this unit, ROM order */

extern const char D_80010984[];
extern const char D_80010994[];
extern const char D_80010A20[];        /* "%s...\n" */
extern const char D_80010A28[];        /* "%s: no param\n" */
extern const char D_80010A38[];        /* "CD_cw" */

s32 CD_cw(s32 arg0, s32 arg1, s32 arg2, s32 arg3)
{
    const char **table;
    volatile u8 *state;
    volatile u8 *state1;
    s32 counter;
    s32 result;
    s32 flags;
    u8 savedState;
    u8 *dst;
    const u8 *src;
    s32 i;

    if (D_8006D608 >= 2) {
        printf(D_80010A20, D_8006D620[arg0 & 0xFF]);
    }

    if (D_8006D840[arg0 & 0xFF] != 0 && arg1 == 0) {
        if (D_8006D608 > 0) {
            printf(D_80010A28, D_8006D620[arg0 & 0xFF]);
        }
        return -2;
    }

    CD_sync(0, 0);

    if ((arg0 & 0xFF) == 2) {
        src = (const u8 *)arg1;
        for (i = 0; i < 4; i++) {
            D_8006D618[i] = *src;
            src++;
        }
    }

    D_8006D8D8[0] = 0;

    if (D_8006D740[arg0 & 0xFF] != 0) {
        D_8006D8D9 = 0;
    }

    *D_8006D8C0 = 0;

    if (D_8006D740[(arg0 & 0xFF) + 0x40] > 0) {
        for (i = 0; i < D_8006D740[(arg0 & 0xFF) + 0x40]; i++) {
            *D_8006D8C8 = ((u8 *)arg1)[i];
        }
    }

    D_8006D61D = (u8)arg0;
    *D_8006D8C4 = (u8)arg0;

    if (arg3 != 0) {
        return 0;
    }

    D_8008B3E4 = VSync(-1) + 0x1E0;
    state = D_8006D8D8;
    D_8008B3E8 = 0;
    D_8008B3EC = D_80010A38;

    if (*state == 0) {
        table = D_8006D6A0;
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
            printf(D_80010994, D_8008B3EC, D_8006D620[D_8006D61D],
                          table[state[0]], src);
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
                        if (D_8006D600 != NULL) {
                            D_8006D600(*state1, D_8008B3D4);
                        }
                    }
                    if (flags & 2) {
                        if (D_8006D5FC != NULL) {
                            D_8006D5FC(*state, D_8008B3CC);
                        }
                    }
                }
                *D_8006D8C0 = savedState;
            }
        } while (*state == 0);
    }

    if (D_8006D8D8[0] == 2 && (arg0 & 0xFF) == 0xE) {
        D_8006D61C = *(u8 *)arg1;
    }

    dst = (u8 *)arg2;
    src = D_8008B3CC;
    if (dst != NULL) {
        for (i = 7; i != -1; i--) {
            *dst = *src;
            dst++;
            src++;
        }
    }

    return (D_8006D8D8[0] == 5) ? -1 : 0;
}
#endif
INCLUDE_ASM("asm/nonmatchings/code_179d8_b", CD_cw);

/* ---- merged from code_179d8_g ---- */

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
 * code_179d8_g -- functions 83..99 of the original 274-function code_179d8
 * monolith, 0x1AB78..0x1C440 (vram 0x8002A378..0x8002BC40).  Carved round 17
 * (2026-09-04) off the back of what was then `code_179d8_mid`, the three
 * functions in front of this slice. Those three were left as "all addiu-$at
 * blocked, so there is nothing left to staff there"; re-censused round 24
 * (2026-09-08) that is FALSE -- CD_sync (161w), CD_ready (180w)
 * and CD_cw (282w) are ALL THREE blocker-clean now that `addiu_at`
 * is resolved. Round 26 (2026-09-09) acted on that and CARVED them as the
 * C unit `code_179d8_b`; no `code_179d8_mid` segment exists any more.
 *
 * Blocker census, three-grep screen run per function at carve time:
 * 14 of the 17 clean, zero trivial leaves.  These are BIG bodies -- 196,
 * 223, 186 and 189 instructions among them -- so this unit is smaller in
 * count and considerably larger in work than 17 suggests.  Budget fewer
 * functions per pass here than in a leaf-heavy unit.
 *
 * NO LONGER BLOCKED -- all three of this unit's blocked functions were
 * blocked on `addiu_at` ALONE, and `addiu_at` was RESOLVED in round 21
 * (maspsx `--addiu-at`; docs/research/addiu-at-blocker.md). Re-screened with
 * `python3 tools/nearmiss.py` on 2026-09-08 (round 24):
 *   CD_readsync (174w)  func_8002B640 (186w)  func_8002B94C (189w)
 * The previous version of this comment read "BLOCKED, stub reports already
 * filed, do NOT spend attempts on these" -- a stale DIRECTIVE over free
 * ground.
 *
 * ROUND 32 (2026-09-12) CORRECTION -- that reopening WORKED, and the
 * "FRESH and assignable / their stub reports are already gone" wording it
 * left behind is now stale in the OPPOSITE direction. All three have since
 * been attempted and all three carry full worked stall reports (174/174
 * length-exact at 153 words; 3 words short; 2 words long respectively).
 * They are near-misses, NOT cold ground: read
 * docs/match-reports/<func>.md before spending an attempt, or you will
 * re-derive several hundred lines of someone else's derivation. Verified
 * by `tools/nearmiss.py` and by the presence of the report files, not by
 * reading this comment.
 *
 * AND A SECOND ROUND-32 CORRECTION, made the same day as the one above:
 * func_8002B640 and func_8002B94C are NOT GAME CODE AT ALL. Both lie fully
 * inside `libcd/iso9660.o` (Psy-Q 3.3), an object already placed in
 * config/psyq-objects.txt and verified against retail. No C matches them;
 * the correct disposition is conversion per docs/SDK-OBJECTS-GUIDE.md.
 * Only CD_readsync of the three is real game ground.
 *
 * ROUND 34 (head): CONVERTED. The unit's last three functions -- CdSearchFile
 * (func_8002B640), _cmp (func_8002B928, which had been matched as C) and
 * CD_newmedia (func_8002B94C) -- are linked from `lib/libcd/iso9660.o`, which
 * runs on into code_179d8_d (CD_searchdir, CD_cachefile, cd_read and a WEAK
 * memcpy). The unit is now 0x1AB78..0x1BE40 (vram 0x8002A378..0x8002B640),
 * 14 functions. The iso9660-only declarations that used to sit below (the
 * CD_* diagnostic strings, the directory-cache views, UWord) went with them.
 *
 * Note what happened here, because it is the reason this comment now
 * carries three verdicts: round 24 reopened all three as free ground and
 * round 32's first pass "corrected" that to near-misses -- both times
 * without asking whether Sony owned them. `python3 tools/sdkstalls.py`
 * answers that in one command and did not exist until round 32.
 *
 * Owns NO switch jump table -- zero `jtbl_` references in the slice -- so no
 * rodata sub-slot is attached to this unit.
 */

/* code_179d8_g -- this window's globals continue code_179d8_b's reading:
 * plain scalar/pointer driver state, not object fields (no classtable.py
 * hit near D_8006D5FC..D_8006D934). This unit's own extern declarations,
 * kept local per the project's multiple-independent-local-views convention
 * -- see code_179d8_b.c's header comment for why no shared header. */

extern s32 D_8006D8A4;
extern s32 D_8006D5FC;
extern s32 D_8006D600;
extern s32 D_8006D604;
extern s32 D_8006D60C;
extern s32 D_8006D610;
extern s32 D_8006D614;
extern u8 D_8006D618;
extern u8 D_8006D619;
extern u8 D_8006D61A;
extern u8 D_8006D61C;
extern u8 D_8006D61D;
extern s32 D_8006D6A0[];       /* lookup table, indexed by a byte field << 2 */

extern volatile u8 *D_8006D8C0;
extern volatile u8 *D_8006D8C4;
extern volatile u8 *D_8006D8C8;
extern volatile u8 *D_8006D8CC;
extern volatile s32 *D_8006D8D0;
extern volatile u16 *D_8006D8D4;   /* HW register block; offsets are byte offsets */
extern u8 D_8006D8D8[2];
extern u8 D_8006D8D9;
extern volatile u8 D_8006D8DA;
extern s32 D_8006D8DC[10];   /* first of 10 consecutive words zeroed by a pointer walk;
                          * D_8006D8E0..D_8006D900 are the other nine, each
                          * already individually named -- not a real array. */
extern s32 D_8006D8E0;
extern s32 D_8006D8E4;
extern volatile s32 D_8006D8E8;
extern volatile s32 D_8006D8EC;
extern volatile s32 D_8006D8F0;
extern volatile s32 D_8006D8F4;
extern s32 D_8006D8F8;
extern s32 D_8006D8FC;
extern s32 D_8006D900;
extern s32 D_8006D904;
extern u8 D_8008B3CC[];
extern u8 D_8008B3D4[];
extern s32 D_8008B3E4;
extern s32 D_8008B3E8;
extern s32 D_8008B3EC;
extern u8 D_80010AE0[];
extern u8 D_80010AD8[];
extern u8 D_80010984[];
extern u8 D_80010994[];
extern u8 D_80010A40[];
extern u8 D_80010A50[];
extern u8 D_80010A94[];
extern u8 D_80010AA0[];
extern u8 D_80010AAC[];
extern u8 D_80010ABC[];
extern u8 D_8006D908[];
extern u8 D_8006D90C[];
extern volatile s32 *D_8006D924;
extern volatile s32 *D_8006D928;
extern volatile s32 *D_8006D92C;
extern volatile s32 *D_8006D930;
extern volatile s32 *D_8006D934;

/* Still INCLUDE_ASM elsewhere -- not this unit's to carve. */
extern void ResetCallback(void);                              /* lib/libetc/intr.o */
extern void (*InterruptCallback(s32 arg0, void (*callback)(void)))(void); /* lib/libetc/intr.o, per code_179d8_c.c */
extern s32 VSync(s32 arg0);                            /* asm/psyq_15d04.s */
extern void puts(const char *arg0);                   /* asm/psyq_15d04.s */
extern void printf(const char *fmt, ...);                /* Psy-Q printf wrapper */
extern s32 CD_cw(s32 arg0, s32 arg1, s32 arg2, s32 arg3); /* defined in code_179d8_b */
extern s32 CD_sync(s32 arg0, s32 arg1);                   /* defined in code_179d8_b, per code_179d8_b.c */
extern s32 getintr(void);                                /* code_179d8_b.c, MATCHED round 70
                                                                   (libcd getintr by its strings) */
extern s32 CheckCallback(void);                                /* lib/libetc/intr.o -- trivial
                                                                   (u16)D_8006C272 getter */
/* Still INCLUDE_ASM in THIS unit (not yet converted) -- INCLUDE_ASM leaves no
 * C-level prototype of its own, so callers within this file need one. */
extern s32 cd_read_retry(void);
extern s32 CD_datasync(s32 arg0);

/* Forward declarations: taken by address before their own ROM-order definition
 * further down this file (CD_initintr/CD_init hand callback to
 * InterruptCallback as a thread entry; cd_read_retry hands cb_read to
 * D_8006D600 as a callback). */
void callback(void);
void cb_read(s32 arg0, s32 arg1);

s32 CD_vol(u8 *arg0)
{
    *D_8006D8C0 = 2;
    *D_8006D8C8 = arg0[0];
    *D_8006D8CC = arg0[1];
    *D_8006D8C0 = 3;
    *D_8006D8C4 = arg0[2];
    *D_8006D8C8 = arg0[3];
    *D_8006D8CC = 0x20;
    return 0;
}

void CD_shell(void)
{
    s32 saved;
    s32 counter = 0;

    if (D_8006D904 < D_8006D614) {
        saved = D_8006D5FC;
        D_8006D5FC = 0;

        while (D_8006D60C & 0x10) {
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

        D_8006D5FC = saved;
        D_8006D904 = D_8006D614;
    }
}

void CD_flush(void)
{
    volatile u8 *q;

    *D_8006D8C0 = 1;
    while (*D_8006D8CC & 7) {
        *D_8006D8C0 = 1;
        *D_8006D8CC = 7;
        *D_8006D8C8 = 7;
    }

    D_8006D8DA = 0;
    q = &D_8006D8D9;
    D_8006D61C = 0;
    *q = D_8006D8DA;
    /* Keeps the D_8006D8C0 pointer load and the D_8006D8D8[0] = 2 store
     * below the D_8006D61C / D_8006D8D9 stores; without it GCC hoists them above. */
    __asm__("");
    D_8006D8D8[0] = 2;
    *D_8006D8C0 = 0;
    *D_8006D8CC = 0;
    *D_8006D8D0 = 0x1325;
}

s32 CD_initvol(void)
{
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

void CD_initintr(void)
{
    s32 *p;
    s32 i;

    D_8006D600 = 0;
    D_8006D5FC = 0;
    D_8006D610 = 0;
    D_8006D60C = 0;
    p = D_8006D8DC;
    for (i = 9; i != -1; i--) {
        *p = 0;
        p++;
    }
    ResetCallback();
    InterruptCallback(2, callback);
}

#ifdef NON_MATCHING
/* NON_MATCHING: 171/196 words, length exact. Residue: pure list-scheduling
 * (round-25 --debug breakdown: Reorderings: 3, Register Differences: 0) --
 * retail splits CD_cw(1,0,0,0)'s argument materialization from its
 * own a3/jal by ~90 bytes; neither call position tried reproduces the split
 * (docs/match-reports/CD_init.md). Hand-derived. */
s32 CD_init(void)
{
    s32 *p;
    s32 i;
    volatile u8 *q;
    s32 saved;
    s32 counter;

    puts(D_80010A94);
    printf(D_80010AA0, D_8006D90C);

    D_8006D61D = 0;
    D_8006D61C = 0;
    D_8006D600 = 0;
    D_8006D5FC = 0;
    D_8006D610 = 0;
    D_8006D60C = 0;
    p = &D_8006D8DC;
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
    D_8006D61C = 0;
    *q = D_8006D8DA;
    __asm__("");
    D_8006D8D8[0] = 2;
    *D_8006D8C0 = 0;
    *D_8006D8CC = 0;
    *D_8006D8D0 = 0x1325;

    counter = 0;
    if (D_8006D60C & 0x10) {
        CD_cw(1, 0, 0, 0);
    }

    if (D_8006D904 < D_8006D614) {
        saved = D_8006D5FC;
        D_8006D5FC = 0;

        while (D_8006D60C & 0x10) {
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

        D_8006D5FC = saved;
        D_8006D904 = D_8006D614;
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
INCLUDE_ASM("asm/nonmatchings/code_179d8_b", CD_init);
#endif

#ifdef NON_MATCHING
/* NON_MATCHING: 215/223 words, length exact. Residue: two small isolated
 * clusters -- a loop-setup scheduling swap at 0x8002AABC (p2 computed from
 * $a0 before vs. after the move into $s5) and a register-identity swap in
 * the final D_8006D8F4=-1 block at 0x8002ADAC -- neither reachable by any
 * reorder or spelling variant tried (docs/match-reports/cd_read_retry.md).
 * Hand-derived structure (rounds 17-39); the n/saved throwaway-sink reuse
 * a few lines below is a permuter find (round 41) reviewed here and
 * confirmed sound (both are freshly written on every path before their
 * next read) against a rejected sibling candidate that hoisted a value
 * across a loop boundary unsoundly. */
s32 cd_read_retry(void)
{
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
    D_8006D600 = 0;
    D_8006D5FC = 0;
    *tmp = n - 1;
    __asm__("");

    if (n > 0) {
        pRetry = tmp;
        p2 = pRetry + 4;
        do {
            if (*pRetry < 7) {
                counter = 0;
                puts(D_80010AAC);
                printf(D_80010ABC, *pRetry, D_8006D618, D_8006D619, D_8006D61A);

                if (D_8006D904 < D_8006D614) {
                    saved = D_8006D5FC;
                    D_8006D5FC = 0;

                    while (D_8006D60C & 0x10) {
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

                    D_8006D5FC = saved;
                    D_8006D904 = D_8006D614;
                }

                if (CD_cw(9, 0, 0, 0) != 0) {
                    goto tail;
                }
                if (CD_cw(2, (s32)&D_8006D618, 0, 0) != 0) {
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
            D_8006D61C = 0;
            *q = D_8006D8DA;
            __asm__("");
            D_8006D8D8[0] = 2;
            *D_8006D8C0 = 0;
            *D_8006D8CC = 0;
            *D_8006D8D0 = 0x1325;

            {
                s32 v0 = p2[0];
                buf = (u8)v0;
                n = ((u8)v0 != D_8006D61C);
                if (n) {
                    saved = (s32)&buf;
                    if (CD_cw(0xE, saved, 0, 0) != 0) {
                        goto tail;
                    }
                }
            }

            D_8006D600 = (s32)cb_read;
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
INCLUDE_ASM("asm/nonmatchings/code_179d8_b", cd_read_retry);
#endif

s32 CD_readm(s32 arg0, s32 arg1, s32 arg2)
{
    s32 t;
    volatile s32 *p;

    p = &D_8006D8EC;
    *p = arg2;
    t = *p & 0x30;

    /* Retail keeps all three D_8006D8F0 stores as separate, unmerged blocks
     * (three distinct address computations -- two folded through $at, one
     * unfolded through a real GPR) instead of the single shared store GCC's
     * cross-jump/tail-merge pass produces from the equivalent if/else-if/else
     * or switch. The match report records how the `goto` layout and the
     * local `volatile s32 *` pointers below were found to keep the stores
     * apart. A bare `__asm__("")` after case1's store, once part of that
     * recipe, was retired in round 89: removing it left the object
     * byte-identical. */
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
case3:
    {
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
    D_8006D8FC = D_8006D5FC;
    D_8006D900 = D_8006D600;

    if (D_8006D60C & 0xE0) {
        CD_cw(9, 0, 0, 0);
    }
    CD_sync(0, 0);
    return -(cd_read_retry() < 1);
}

s32 CD_readsync(s32 arg0, s32 arg1)
{
    s32 now;
    s32 old;
    s32 flags;
    s32 *pEC;
    u8 status;
    s32 *p6A0;
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
    p6A0 = D_8006D6A0;
    p8D8 = D_8006D8D8;
    p8D9 = &D_8006D8D8[1];
    pF8 = &D_8006D8DC[7];

    D_8008B3E4 = now + 0x1E0;
    D_8008B3E8 = 0;
    D_8008B3EC = (s32)D_80010AD8;

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
        /* &D_8008B3EC routed through a local pointer -- forces the same
         * unfolded lui/addiu addressing retail uses for this argument;
         * a plain `D_8008B3EC` reference here compiles FOLDED instead.
         * See docs/match-reports/CD_readsync.md's round-36 entry. */
        pEC = &D_8008B3EC;
        printf(D_80010994, *pEC, D_8006D620[D_8006D61D],
               p6A0[idx0], p6A0[idx1]);
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
                if ((flags & 4) && D_8006D600 != 0) {
                    ((void (*)(s32, u8 *))D_8006D600)(p8D9[0], D_8008B3D4);
                }
                if ((flags & 2) && D_8006D5FC != 0) {
                    ((void (*)(s32, u8 *))D_8006D5FC)(p8D8[0], D_8008B3CC);
                }
            }
            /* ------------------------------------------------------------
             * KNOWN-BAD CONSTRUCT, KEPT ONLY BECAUSE IT IS BYTE-EXACT.
             * DO NOT COPY THIS SHAPE INTO ANOTHER FUNCTION.
             *
             * `p6A0` and `pF8` are both addresses of globals, so the
             * condition is a TAUTOLOGY and both arms are IDENTICAL. GCC
             * 2.6.3 cross-jumps the two arms back into the single `sb`
             * retail has, so this compiles to no extra instruction -- its
             * whole effect is to perturb register allocation, forcing
             * `status` into $s1 across the inner loop. Worth 12 words:
             * 162/174 without it, 174/174 with it.
             *
             * docs/PARALLEL-RUNS.md Gate 3 names duplicate-arm forms
             * alongside UB as the signature of an EXHAUSTED class rather
             * than a solution, and says a permuter zero is a LEAD to be
             * translated into idiomatic C and re-verified. Round 39's head
             * tried eleven such translations and none reached 174/174 --
             * every declaration- and assignment-order permutation of the
             * four pointer locals, `status` retyped to s32, a real
             * (non-tautological) guard, a re-masked store, and an explicit
             * live-range extension. All tabulated in the match report.
             *
             * THE MIS-MODELLING LEAD IS TESTED AND THE ANSWER IS SPLIT
             * (round 40, head). The round-39 form of this comment said a
             * tautological null check is what a mis-modelled global looks
             * like, and asked whether either operand is really a POINTER
             * global. Measured from the DATA, not from attempts:
             *
             *   - D_8006D6A0 is a fixed 8-element table of rodata string
             *     addresses (asm/data/5DDFC.data.s:121). An array.
             *   - D_8006D8F8 is one zero word that the sibling
             *     cb_read stores VSync()'s return into
             *     (cb_read.s:49-51). An s32 timestamp.
             *
             * Neither is a pointer global, so the condition CANNOT become
             * an honest null test by that route. That half is closed.
             *
             * A DIFFERENT mis-modelling was real and IS now corrected:
             * `pF8[-1]` (used four times below) reaches D_8006D8F4 by
             * negative indexing off D_8006D8F8, which nobody writes -- the
             * ten consecutive words ARE one array, as the zeroing walk in
             * CD_readm already implied. They are now declared
             * `s32 D_8006D8DC[10]` and indexed, and that model is
             * BYTE-IDENTICAL (174/174, whole image green).
             *
             * BUT IT DOES NOT DISSOLVE THIS CONSTRUCT. Under the corrected
             * model, removing the construct still scores 162/174 -- the
             * same figure as before -- and every residual diff is a pure
             * register swap ($s2/$s3, $a0/$a2, $v0/$v1). So the 12 words
             * are REGISTER ALLOCATION, not data modelling, and the data
             * model was never what this construct was standing in for.
             * Do not re-run the data-modelling axis; it is measured.
             * ------------------------------------------------------------ */
            if (p6A0 || pF8) {
                *D_8006D8C0 = status;
            } else {
                *D_8006D8C0 = status;
            }
        }

        dst = (u8 *)arg1;
        src = D_8008B3D4;
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
/* NON_MATCHING: 49/91 words, length exact. Residue: register identity
 * (the three hoisted pointers p620/p6A0/p8D8 land in different
 * callee-saved registers than retail's $s3/$s1/$s0) (docs/match-reports/
 * CD_datasync.md). Structure is hand-derived; the diagnostic call's
 * `ok =` sink is a permuter find (round 36), reviewed as a semantically
 * inert dead-store reuse and oracle-confirmed. */
s32 CD_datasync(s32 arg0)
{
    s32 now;
    s32 ok;
    char **p620;
    u8 *p8D8;
    s32 *p6A0;

    D_8008B3E4 = VSync(-1) + 0x1E0;
    p620 = D_8006D620;
    p6A0 = D_8006D6A0;
    p8D8 = D_8006D8D8;
    D_8008B3E8 = 0;
    D_8008B3EC = (s32)D_80010AE0;

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
            /* retail reuses the (dead, about-to-be-overwritten) `ok` slot as
             * the register target for this last argument's value -- a fresh
             * local here compiles worse (45/91 vs 49/91); see this report's
             * round-36 entry. */
            printf(D_80010994, p8D8[0], p6A0[p8D8[1]], p620[D_8006D61D],
                   ok = p6A0[p8D8[0]]);
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
INCLUDE_ASM("asm/nonmatchings/code_179d8_b", CD_datasync);
#endif

s32 CD_getsector(s32 arg0, s32 arg1)
{
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

void func_8002B3E4(s32 arg0)
{
    D_8006D8A4 = arg0;
}

#ifdef NON_MATCHING
/* NON_MATCHING: 30/56 words, length 1 short. Residue: instruction-selection
 * (retail computes &D_8006D8D8 unfolded inside the loop; this folds it)
 * (docs/match-reports/callback.md). Hand-derived. */
void callback(void)
{
    u8 status;
    s32 flags;
    s32 handler;
    u8 *pd9;

    status = (*D_8006D8C0) & 3;
    pd9 = &D_8006D8D9;

    for (;;) {
        flags = getintr();
        if (flags == 0) {
            break;
        }
        if (flags & 4) {
            handler = D_8006D600;
            if (handler != 0) {
                ((void (*)(s32, u8 *))handler)(*pd9, D_8008B3D4);
            }
        }
        if (flags & 2) {
            if (D_8006D5FC != 0) {
                ((void (*)(s32, u8 *))D_8006D5FC)(D_8006D8D8[0], D_8008B3CC);
            }
        }
    }
    *D_8006D8C0 = status;
}
#else
INCLUDE_ASM("asm/nonmatchings/code_179d8_b", callback);
#endif

void cb_read(s32 arg0, s32 arg1)
{
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
elseBranch:
    {
        volatile s32 *p2 = &D_8006D8F4;
        *p2 = -1;
    }
shared:
    {
        volatile s32 *pF8 = &D_8006D8F8;
        *pF8 = VSync(-1);
    }

    if (D_8006D8F4 < 0 && D_8006D8DC[0] > 0) {
        cd_read_retry();
    }

    if ((*(new_var = &D_8006D8F4)) <= 0) {
        D_8006D5FC = D_8006D8FC;
        D_8006D600 = D_8006D900;
        CD_cw(9, 0, 0, 0);
        if (D_8006D604 != 0) {
            if ((*new_var) == 0) {
                code = 2;
            } else {
                code = 5;
            }
            ((void (*)(s32, s32))D_8006D604)(code, arg1);
        }
    }
}
