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
 * code_179d8_h -- functions 43..59 of the original code_179d8 monolith's head,
 * originally 0x19098..0x194E0 (vram 0x80028898..0x800294E0).  Carved
 * MID-round 17 (2026-09-04) to re-staff a runner whose own unit was exhausted.
 *
 * ROUND 34 (head): the unit's LAST SIX functions left it.  strcpy (0x80028B78)
 * and strstr (0x80028BBC) are `libc2/strcpy.o` / `libc2/strstr.o`, and
 * CdStatus, CdLastCom (func_80028C44), CdReset (func_80028C54) and CdFlush
 * (func_80028CC0) are the first four functions of `libcd/sys.o` (Psy-Q 3.3),
 * which runs on through the whole front of libcd_bios.  All six had been
 * matched as C; they were Sony's the whole time, and reclassifying them out of
 * the game count is the correction CLAUDE.md asks for, not a regression.  The
 * unit is now 0x19098..0x19378 (11 functions).  The "three carry real names
 * inherited from FirecatFG" note below is now three CONFIRMED names.
 *
 * Blocker census, three-grep screen run per function at carve time (against
 * the original 17-function carve, before round 34's six departed): 16 of 17
 * clean.  `GetCdUseVSyncCallback`'s `gp_rel` screen hit was the one exception
 * (only 3 instructions, so nothing was lost either way) -- it was RESOLVED
 * and MATCHED round 45 (see the ROUND 42 CORRECTION above); no function in
 * this unit is blocked or stub-filed as of round 64.
 *
 * ROUND 64 (naming pass, runner alpha): what the unit IS, now that every
 * function has a report.  Eleven functions split into three groups:
 *   - `FileResource__InstallCdReadDriver`/`FileResource__DestroyCdReadDriver`
 *     (ctor/dtor pair, `FileResource*` self): chains `FileResource`'s own
 *     ctor/dtor (`include/code_171e0.h`) then, on the ctor side, overwrites
 *     `self->methods` with `GetCdDriverMethods()`'s table -- `gCdDriverMethods`,
 *     independently confirmed elsewhere (`src/code_179d8_q.c`) as "the
 *     CD-ROM read driver".  No caller is visible yet (referenced only from
 *     the still-uncarved `code_179d8` remainder), so WHICH broader purpose
 *     this reclassification serves is open; see both reports' `## Naming`.
 *   - `OpenCdFile`/`CloseCdFile`/`GetCdFileSize`/`ReadCdFile` (the
 *     `CdDriver *self` quad, all four matched; `ObjA34_179D8H` until round 88): resolves a CD-ROM file
 *     by name, tracks whether it is open, reports its sector-rounded size,
 *     and reads from it.  Not inferred from this unit alone --
 *     `src/code_179d8_s.c`'s `CdDriver__Open`/`CdDriver__Close`/
 *     `CdDriver__Seek`/`CdDriver__Read` call the sync version of exactly one
 *     of these apiece when CD-async mode is off, and independently
 *     reimplement the identical algorithm (same field offsets) for the
 *     async path otherwise -- see `OpenCdFile.md` for the full mapping.
 *     `BuildCdFilePath` is `OpenCdFile`'s own path-string helper.
 *   - `NoOp2`/`NoOp3`/`NoOp4`: the three 2-instruction (`jr $ra; nop`) leaves
 *     splat matched at carve time; no caller or vtable slot identified for
 *     any of them.  `GetCdUseVSyncCallback` is the twelfth matched function
 *     (a plain getter).
 * The three `strcpy`/`strstr`/`CdStatus` names the ROUND 34 note above
 * discusses are Sony's, per that note -- they are no longer entries of this
 * unit and are not renamed here (CLAUDE.md: Sony symbols are never renamed).
 *
 * The 43 functions in FRONT of this slice (still `code_179d8`) are
 * gp_rel-saturated -- 33 of 43 blocked, RESOLVED per the ROUND 42 CORRECTION
 * -- and that remainder also owns this segment's ONLY switch jump table
 * (CdDriver__RunRequestQueue, which will need the Gate 2 rodata attach/split when it is
 * carved).  The cut is placed here to leave both debts behind: THIS slice
 * owns no jump table and needs no rodata attach.
 *
 * Class-framework status, CORRECTED round 64: measured, not assumed, and the
 * prior claim here ("zero functions in this slice reference any of the 60
 * method tables") was wrong by the time it was written -- `python3
 * tools/classtable.py --scan` lists BOTH `gFileResourceMethods` (44 slots, header 3)
 * and `gCdDriverMethods` (29 slots, header 0x13) among the 60, and
 * `FileResource__InstallCdReadDriver`/`FileResource__DestroyCdReadDriver`
 * dispatch through both via `GetFileResourceMethods()`/`GetCdDriverMethods()`.
 * The quad's object (round 88: a `CdDriver`, include/CdDriver.h) dispatches
 * `ReadCdFile`'s `close` through its own `methods`, FileResource's +0x048.  The sibling slice code_179d8_e also contains two
 * class-table accessors -- so "code_179d8 is not class-framework code" was
 * never true of this neighbourhood; run the check for your own functions
 * rather than inheriting any verdict here.
 */
#include "common.h"
#include <libcd.h>
#include "CdDriver.h"
/* FileResource and its table come from include/FileResource.h, through code_171e0.h. */
#include "code_171e0.h"

/* GetCdDriverMethods, gCdDriverMethods and CdDriver are include/CdDriver.h's
 * (track 4, round 88). OpenCdFile/CloseCdFile/GetCdFileSize/ReadCdFile take
 * the object CdDriver's methods were handed (any FileResource client: see
 * CdDriver.h's banner); they were typed against this unit's own
 * ObjA34_179D8H view, whose isOpen/pos/size are FileResource's +0x00C/+0x018/
 * +0x01C and whose `close` slot is FileResource's +0x048 (CdDriver__Close
 * in gCdDriverMethods; FileResource__LoadFile calls it on its success path
 * too, so the give-up paths below close the file). */

extern void printf(const char *fmt, void *arg);
extern char gCdFileNotFoundFmt[];

/* Forward declaration: BuildCdFilePath is defined later in this file (ROM
 * order), but OpenCdFile (earlier in ROM order) calls it. */
char *BuildCdFilePath(char *dest, char *name);

/* func_800270B8 is code_171e0.c's; strcpy and strcat are Sony's
 * (lib/libc2/strcpy.o, lib/libc2/strcat.o, linked since round 34) --
 * declared LOCAL, per-call-site typed, never via a shared header. */
extern char *func_800270B8(void);
extern char *strcpy(char *dest, char *src);
extern char *strcat(char *dest, char *src);
extern char gCdFileVersionSuffix[]; /* ";1", the ISO9660 CD file-version suffix */

void FileResource__InstallCdReadDriver(FileResource *self) {
    GetFileResourceMethods()->ctor(self);
    self->methods = (FileResourceMethods *)GetCdDriverMethods();
    self->isOpen = 0;
}

void FileResource__DestroyCdReadDriver(FileResource *self) {
    GetFileResourceMethods()->finalize(self);
}

void NoOp2(void) {}

/* MATCHED round 74 (charlie). The retry loop is a label + backward goto,
 * not while/for: a real loop gets loop notes, loop.c hoists &path out of it
 * and CSEs it with BuildCdFilePath's argument (one word long, rotated
 * saved registers). Retail recomputes &path inside the loop body --
 * docs/match-reports/OpenCdFile.md. */
void OpenCdFile(CdDriver *self, char *name) {
    s32 retries;
    CdlFILE file;
    char path[0x40];

    retries = 0;
    if (self->isOpen == 0) {
        BuildCdFilePath(path, name);
    retry:
        if (CdSearchFile(&file, path) == 0) {
            if (retries++ < 100) {
                goto retry;
            }
            printf(gCdFileNotFoundFmt, path);
            return;
        }
        /* CdLoc16 is the project's spelling of CdlLOC's four bytes (FileResource.h). */
        self->pos = *(CdLoc16 *)&file.pos;
        self->size = file.size;
        self->isOpen = 1;
    }
}

char *BuildCdFilePath(char *dest, char *name) {
    dest[0] = '\\';
    strcpy(dest + 1, func_800270B8());
    strcat(dest, name);
    strcat(dest, gCdFileVersionSuffix);
    return dest;
}

void CloseCdFile(CdDriver *self) {
    if (self->isOpen != 0) {
        self->isOpen = 0;
    }
}

s32 GetCdFileSize(CdDriver *self) {
    u32 result;

    if (self->isOpen == 0) {
        result = 0;
    } else {
        result = ((self->size >> 11) + 1) << 11;
    }
    return result;
}

void NoOp3(void) {}

/* MATCHED round 74 (charlie). Two of its three loops are label + goto
 * (the seek retry and the CdSync wait); only the CdReadSync wait is a
 * do-while. The loop kind is readable from the back-edge: a do-while's
 * branch targets the jal with the argument setup copied into its delay
 * slot, a goto loop's branch targets the argument setup itself --
 * docs/match-reports/ReadCdFile.md. `scratch` is never touched; it only
 * sizes the frame (retail's `buf` sits at sp+0x810). */
s32 ReadCdFile(CdDriver *self, void *buf, s32 size) {
    s32 sectors;
    s32 status;
    char scratch[0x800];
    u_char syncResult[0x10];

    if (self->isOpen != 0) {
    retry:
        sectors = (u32)size >> 11;
        CdControl(CdlSetloc, (u_char *)&self->pos, 0);
    sync:
        status = CdSync(0, syncResult);
        if (status == 0) {
            goto sync;
        }
        if (status == 5) {
            goto retry;
        }
        if (sectors != 0) {
            CdRead(sectors, (u_long *)buf, CdlModeSpeed);
            do {
                status = CdReadSync(0, 0);
            } while (status > 0);
            if (status == -1) {
                goto retry;
            }
            return 0;
        }
    } else {
        self->methods->close(self);
    }
    return 0;
}

void NoOp4(void) {}

s32 GetCdUseVSyncCallback(void) {
    return gCdUseVSyncCallback;
}
