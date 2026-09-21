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
 * Blocker census, three-grep screen run per function at carve time:
 * 16 of 17 clean.
 *
 * BLOCKED, stub report already filed, do NOT spend attempts on it:
 *   gp_rel: GetCdUseVSyncCallback (only 3 instructions, so nothing is lost)
 *
 * Three of the entries carry real names inherited from FirecatFG's lsddecomp
 * (`strcpy`, `strstr`, `CdStatus`) -- treat those names as HYPOTHESES like any
 * other inherited symbol, but they are a strong hint about the shape.  Three
 * more are 2-instruction leaves that splat matched itself.
 *
 * The 43 functions in FRONT of this slice (still `code_179d8`) are
 * gp_rel-saturated -- 33 of 43 blocked -- and that remainder also owns this
 * segment's ONLY switch jump table (func_80027A24, which will need the Gate 2
 * rodata attach/split when it is carved).  The cut is placed here to leave
 * both debts behind: THIS slice owns no jump table and needs no rodata attach.
 *
 * Class-framework status: measured, not assumed.  Zero functions in this slice
 * reference any of the 60 method tables tools/classtable.py --scan finds.  The
 * sibling slice code_179d8_e DOES contain two class-table accessors, so the
 * "code_179d8 is not class-framework code" note is neighbourhood-scoped -- run
 * the check for your own functions rather than inheriting either verdict.
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
 * after adding it. Named `onError`: the ONE confirmed dispatch (ReadCdFile,
 * when `self->isOpen == 0`) matches the SAME slot number (+0x48) that
 * src/code_179d8_s.c's independent local view (Methods80027480::slot48)
 * dispatches on ITS OWN allocation-failure path (func_80027800) -- two
 * unrelated call sites landing on the identical offset for a give-up path
 * is evidence for "error/failure handler", not a guess at a specific
 * message; PROPOSED for code_179d8_s.c under the same name, not applied
 * there (out of unit). */
typedef struct MethodsA34_179D8H {
    u8 pad000[0x48];
    void (*onError)(ObjA34_179D8H *self);
} MethodsA34_179D8H;

struct ObjA34_179D8H {
    MethodsA34_179D8H *methods;
    u8 pad4[0x0C - 0x04];
    s32 isOpen;   /* was unk0C -- 0/nonzero, set by OpenCdFile on a successful
                   * CdSearchFile, cleared by CloseCdFile; GetCdFileSize
                   * returns 0 when this is 0. Named from src/code_179d8_s.c's
                   * independent async reimplementation of the same three
                   * operations (func_800272D0/func_80027480/func_80027528),
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
    self->pendingGeneration = 0;
}

void *Class6D430__DestroyCdReadDriver(Class6D430 *self) {
    return ((Class6D430Methods *)GetClass6D430Methods())->dtor(self);
}

void NoOp2(void) {
}

/* ROUND 36 (runner charlie): the round-17 preserved body (best 12/43,
 * structural -- path-address CSE across the loop's calls, plus a
 * register-role rotation, per docs/match-reports/OpenCdFile.md) spelled
 * its two callees func_8002B640/func_80012C20, which round 34's SDK-object
 * conversion retargeted to CdSearchFile/printf (already declared above,
 * per-call-site typed for this unit). Corrected and rebuilt: reproduces the
 * IDENTICAL structural residue (one extra cached-address instruction,
 * confirmed via asm-differ) -- the previously recorded figure is now
 * measured, not carried forward. Still genuinely stalled; restored to
 * INCLUDE_ASM. The report carries the corrected, linkable body. */
INCLUDE_ASM("asm/nonmatchings/code_179d8_h", OpenCdFile);

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

/* ROUND 36 (runner charlie): the round-17 preserved body (best 8/56
 * structural, via asm-differ realignment -- block-placement + a
 * register-role rotation, per docs/match-reports/ReadCdFile.md) spelled
 * its four callees func_80028DF0/func_80028D68/func_80029274/func_80029254,
 * which round 34's SDK-object conversion retargeted to
 * CdControl/CdSync/CdRead/CdReadSync (already declared above, per-call-site
 * typed for this unit). Corrected and rebuilt: reproduces the IDENTICAL
 * structural residue (one extra word, same block-placement/register-role
 * shape, confirmed via asm-differ) -- the previously recorded figure is now
 * measured, not carried forward. Tried one additional, previously-untested
 * reshape within budget -- writing the cold path as a single
 * `return self->methods->onError(self), 0;` expression instead of two
 * statements -- identical compiled length and shape, no improvement.
 * Still genuinely stalled; restored to INCLUDE_ASM. The report carries the
 * corrected, linkable body. */
INCLUDE_ASM("asm/nonmatchings/code_179d8_h", ReadCdFile);

void NoOp4(void) {
}

extern s32 gCdUseVSyncCallback;

s32 GetCdUseVSyncCallback(void) {
    return gCdUseVSyncCallback;
}
