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
 * which runs on through the whole front of code_179d8_b.  All six had been
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
 *   - `Class6D430__InstallCdReadDriver`/`Class6D430__DestroyCdReadDriver`
 *     (ctor/dtor pair, `Class6D430*` self): chains `Class6D430`'s own
 *     ctor/dtor (`include/code_171e0.h`) then, on the ctor side, overwrites
 *     `self->methods` with `GetClass6D4E8Methods()`'s table -- `D_8006D4E8`,
 *     independently confirmed elsewhere (`src/code_179d8_q.c`) as "the
 *     CD-ROM read driver".  No caller is visible yet (referenced only from
 *     the still-uncarved `code_179d8` remainder), so WHICH broader purpose
 *     this reclassification serves is open; see both reports' `## Naming`.
 *   - `OpenCdFile`/`CloseCdFile`/`GetCdFileSize`/`ReadCdFile` (the
 *     `ObjA34_179D8H` quad, all four matched): resolves a CD-ROM file
 *     by name, tracks whether it is open, reports its sector-rounded size,
 *     and reads from it.  Not inferred from this unit alone --
 *     `src/code_179d8_s.c`'s `Class6D4E8__Open`/`Class6D4E8__Close`/
 *     `Class6D4E8__Seek`/`Class6D4E8__Read` call the sync version of exactly one
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
 * (Class6D4E8__RunRequestQueue, which will need the Gate 2 rodata attach/split when it is
 * carved).  The cut is placed here to leave both debts behind: THIS slice
 * owns no jump table and needs no rodata attach.
 *
 * Class-framework status, CORRECTED round 64: measured, not assumed, and the
 * prior claim here ("zero functions in this slice reference any of the 60
 * method tables") was wrong by the time it was written -- `python3
 * tools/classtable.py --scan` lists BOTH `D_8006D430` (44 slots, header 3)
 * and `D_8006D4E8` (29 slots, header 0x13) among the 60, and
 * `Class6D430__InstallCdReadDriver`/`Class6D430__DestroyCdReadDriver`
 * dispatch through both via `GetClass6D430Methods()`/`GetClass6D4E8Methods()`.
 * `ObjA34_179D8H` additionally carries its OWN per-instance `methods`
 * pointer (dispatched by `ReadCdFile`'s `close` slot), not yet tied to
 * either scanned table.  The sibling slice code_179d8_e also contains two
 * class-table accessors -- so "code_179d8 is not class-framework code" was
 * never true of this neighbourhood; run the check for your own functions
 * rather than inheriting any verdict here.
 */
#include "common.h"
/* code_171e0.h's Class6D430/Class6D430Methods already
 * describe D_8006D430's class exactly -- Class6D430__InstallCdReadDriver dispatches
 * GetClass6D430Methods()->ctor(self) (offset +0x008), matching that header's own
 * ctor slot. Reused UNCHANGED per CLAUDE.md's header discipline (a sibling
 * would use it unchanged), not redefined locally. */
#include "code_171e0.h"

/* GetClass6D4E8Methods is still uncarved (asm/code_179d8.s) -- returns &D_8006D4E8,
 * a DIFFERENT class table (tools/classtable.py --scan: 29 slots, header
 * 0x13) than D_8006D430. Class6D430__InstallCdReadDriver chains Class6D430's base ctor
 * then overwrites self->methods with this class's own table -- the
 * standard "call base ctor, then install the derived vtable" idiom. Typed
 * against Class6D430Methods for the assignment's sake; the two
 * tables are different classes but share the base's slot layout. */
extern Class6D430Methods *GetClass6D4E8Methods(void);

/* CloseCdFile/GetCdFileSize's own `self` -- offsets +0xC/+0x1C happen to
 * coincide with Class6D430::unk0C and its documented-unknown pad18
 * gap, but that header is code_171e0.c's shared reading and is off-limits
 * to edit here (out of unit) -- kept as this unit's own LOCAL, narrower
 * view per the project's multiple-independent-local-views convention. */
/* A 4-byte, alignment-2 pair -- the idiom CLAUDE.md documents for a struct
 * whose whole-struct assignment compiles to lwl/lwr + swl/swr instead of a
 * plain lw/sw (OpenCdFile's own unk18 copy needs this). Field meaning
 * unestablished beyond width/alignment. */
typedef struct Pair16_179D8H {
    s16 unk0;
    s16 unk2;
} Pair16_179D8H;

typedef struct ObjA34_179D8H ObjA34_179D8H;

/* ObjA34_179D8H's own methods table -- only the one slot ReadCdFile
 * dispatches through is named. Total leading padding through +0xC is
 * unchanged from before this slot was identified (0x4 + 0x8 = 0xC), so
 * this is not a shifting edit -- confirmed by re-verifying CloseCdFile
 * and GetCdFileSize (both already matched, both readers of this struct)
 * after adding it. Named `onError` in round 64 from two give-up call sites
 * at +0x48; renamed `close` in round 79 (charlie's code_179d8_s pass):
 * `tools/classtable.py D_8006D4E8` resolves +0x48 to Class6D4E8__Close,
 * and Class6D430__AllocBuffer also calls it on its SUCCESS path, so the
 * give-up paths were closing the file, not reporting an error. */
typedef struct MethodsA34_179D8H {
    u8 pad000[0x48];
    void (*close)(ObjA34_179D8H *self);
} MethodsA34_179D8H;

struct ObjA34_179D8H {
    MethodsA34_179D8H *methods;
    u8 pad4[0x0C - 0x04];
    s32 isOpen;   /* was unk0C -- 0/nonzero, set by OpenCdFile on a successful
                   * CdSearchFile, cleared by CloseCdFile; GetCdFileSize
                   * returns 0 when this is 0. Named from src/code_179d8_s.c's
                   * independent async reimplementation of the same three
                   * operations (Class6D4E8__Open/Class6D4E8__Close/Class6D4E8__Seek),
                   * which sets/clears the identical field (its own
                   * Obj80027480::unk0C) around the identical CD-search /
                   * CdControl+CdSync sequence -- not guessed from this unit
                   * alone. */
    u8 pad10[0x18 - 0x10];
    Pair16_179D8H pos;   /* was unk18 -- the resolved file's CD position,
                          * copied from CdSearchFile's stat buffer (below) by
                          * OpenCdFile and read by ReadCdFile's CdControl
                          * seek; matches code_179d8_s.c's own field `Pos18
                          * unk18` at the identical offset in its Obj80027480
                          * view, commented there as a CdlLOC-shaped position. */
    u32 size;    /* was unk1C -- the resolved file's byte size, copied from
                  * the same stat buffer; GetCdFileSize rounds this up to the
                  * next 0x800 (one CD sector) boundary. */
};

/* func_8002B640's own stat-like output buffer (OpenCdFile's local
 * `sp+0x10`). Only the two fields OpenCdFile itself copies out are
 * named; the buffer runs up to sp+0x28, where OpenCdFile's own path
 * string buffer starts, so it's at least 0x18 bytes -- the rest is
 * unestablished. */
typedef struct StatBuf179D8H {
    Pair16_179D8H pos;   /* was unk0 -- copied into ObjA34_179D8H::pos */
    u32 size;            /* was unk4 -- copied into ObjA34_179D8H::size */
    u8 pad8[0x18 - 0x8];
} StatBuf179D8H;

/* CdSearchFile (was func_8002B640): Sony's, lib/libcd/iso9660.o since round 34 -- declared
 * LOCAL here, per-call-site typed.
 *
 * This comment used to read "still uncarved in its own unit (code_179d8_g,
 * BLOCKED addiu_at there)", and both halves had gone stale: code_179d8_g has
 * been a carved C unit since round 17, and `addiu_at` was RESOLVED in round
 * 21 (maspsx `--addiu-at`; docs/research/addiu-at-blocker.md), which leaves
 * func_8002B640 blocker-clean and assignable (re-screened with
 * `python3 tools/nearmiss.py`, 2026-09-08).  And THAT went stale in round
 * 34: it is Sony's CdSearchFile, linked from the object, never matchable. */
extern s32 CdSearchFile(StatBuf179D8H *statBuf, char *path);   /* lib/libcd/iso9660.o (round 34) */
extern void printf(const char *fmt, void *arg1);
extern char gCdFileNotFoundFmt[];

/* Forward declaration: BuildCdFilePath is defined later in this file (ROM
 * order), but OpenCdFile (earlier in ROM order) calls it. */
char *BuildCdFilePath(char *dest, char *suffix);

/* func_800270B8 is code_171e0.c's; strcpy and strcat are Sony's
 * (lib/libc2/strcpy.o, lib/libc2/strcat.o, linked since round 34) --
 * declared LOCAL, per-call-site typed, never via a shared header. */
extern char *func_800270B8(void);
extern char *strcpy(char *dest, char *src);
extern char *strcat(char *dest, char *src);
extern char gCdFileVersionSuffix[]; /* ";1", the ISO9660 CD file-version suffix */

void Class6D430__InstallCdReadDriver(Class6D430 *self) {
    ((Class6D430Methods *)GetClass6D430Methods())->ctor(self);
    self->methods = GetClass6D4E8Methods();
    self->isOpen = 0;
}

void *Class6D430__DestroyCdReadDriver(Class6D430 *self) {
    return ((Class6D430Methods *)GetClass6D430Methods())->dtor(self);
}

void NoOp2(void) {
}

/* MATCHED round 74 (charlie). The retry loop is a label + backward goto,
 * not while/for: a real loop gets loop notes, loop.c hoists &path out of it
 * and CSEs it with BuildCdFilePath's argument (one word long, rotated
 * saved registers). Retail recomputes &path inside the loop body --
 * docs/match-reports/OpenCdFile.md. */
void OpenCdFile(ObjA34_179D8H *self, char *suffix) {
    s32 i;
    StatBuf179D8H statBuf;
    char path[0x40];

    i = 0;
    if (self->isOpen == 0) {
        BuildCdFilePath(path, suffix);
    retry:
        if (CdSearchFile(&statBuf, path) == 0) {
            if (i++ < 100) {
                goto retry;
            }
            printf(gCdFileNotFoundFmt, path);
            return;
        }
        self->pos = statBuf.pos;
        self->size = statBuf.size;
        self->isOpen = 1;
    }
}

char *BuildCdFilePath(char *dest, char *suffix) {
    dest[0] = '\\';
    strcpy(dest + 1, func_800270B8());
    strcat(dest, suffix);
    strcat(dest, gCdFileVersionSuffix);
    return dest;
}

void CloseCdFile(ObjA34_179D8H *self) {
    if (self->isOpen != 0) {
        self->isOpen = 0;
    }
}

s32 GetCdFileSize(ObjA34_179D8H *self) {
    u32 result;

    if (self->isOpen == 0) {
        result = 0;
    } else {
        result = ((self->size >> 11) + 1) << 11;
    }
    return result;
}

void NoOp3(void) {
}

/* libcd/sys entry points (lib/libcd/sys.o, linked since round 34) -- this
 * unit's own per-call-site typing for ReadCdFile's calls, kept local. */
extern void CdControl(s32 arg0, Pair16_179D8H *buf, s32 arg2);
extern s32 CdSync(s32 arg0, void *buf);
extern s32 CdRead(s32 arg0, void *arg1, s32 arg2);
extern s32 CdReadSync(s32 arg0, s32 arg1);

/* MATCHED round 74 (charlie). Two of its three loops are label + goto
 * (the seek retry and the CdSync wait); only the CdReadSync wait is a
 * do-while. The loop kind is readable from the back-edge: a do-while's
 * branch targets the jal with the argument setup copied into its delay
 * slot, a goto loop's branch targets the argument setup itself --
 * docs/match-reports/ReadCdFile.md. `scratch` is never touched; it only
 * sizes the frame (retail's `buf` sits at sp+0x810). */
s32 ReadCdFile(ObjA34_179D8H *self, char *arg1, s32 arg2) {
    s32 hi;
    s32 status;
    char scratch[0x800];
    char buf[0x10];

    if (self->isOpen != 0) {
    retry:
        hi = (u32)arg2 >> 11;
        CdControl(2, &self->pos, 0);
    sync:
        status = CdSync(0, buf);
        if (status == 0) {
            goto sync;
        }
        if (status == 5) {
            goto retry;
        }
        if (hi != 0) {
            CdRead(hi, arg1, 0x80);
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

void NoOp4(void) {
}

extern s32 gCdUseVSyncCallback;

s32 GetCdUseVSyncCallback(void) {
    return gCdUseVSyncCallback;
}
