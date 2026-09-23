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
#include "common.h"

INCLUDE_ASM("asm/nonmatchings/code_179d8_n", CD_sync);

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
INCLUDE_ASM("asm/nonmatchings/code_179d8_n", CD_ready);

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
INCLUDE_ASM("asm/nonmatchings/code_179d8_n", CD_cw);
