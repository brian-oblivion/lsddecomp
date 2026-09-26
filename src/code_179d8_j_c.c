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
 * code_179d8_j_c -- the TAIL of the old code_179d8_j slice, split off in round
 * 34 (2026-09-12) when Sony's `libsnd/ut_pb.o` was linked into the middle of
 * `code_179d8_j_b`.  Now 0x22244..0x2273C (vram 0x80031A44..0x80031F3C), eight
 * functions (SsUtChangePitch .. SsUtAutoPan).
 *
 * WHY THE SPLIT EXISTS.  `func_800319B4` is Sony's `SsUtPitchBend`
 * (`libsnd/ut_pb`, Psy-Q 3.6 -- the only disc carrying the module; 0x90 text
 * covering exactly that one function).  It had been MATCHED as C;
 * reclassifying it out of the game count is the correction CLAUDE.md asks for,
 * not a regression.  A placed object cannot live inside a `c` segment, so
 * `code_179d8_j_b` became [c][o][c] and this half needed its own name.
 *
 * This is the SECOND split of the same original slice in the same round --
 * `libsnd/vm_prog` took 0x20FF0..0x21180 first, which is what created
 * `code_179d8_j_b`.  Hence the `_c` suffix: `_b` was already taken.  The
 * precedent for a second-generation split name is the yaml's own note on
 * `<unit>_b` / `<unit>_c`.
 *
 * NO RODATA ATTACH CAME WITH THIS HALF, and that is measured, not assumed: the
 * old code_179d8_mid_c monolith contains zero `jtbl_` and zero `.word .L`
 * across its whole extent, and the splat yaml's rodata slot list names none of
 * `code_179d8_j`, `_j_b` or `_j_c`.  Unlike round 33's libsnd_ssinit_libapi_counter there
 * was nothing to move, and a link failure of the form
 * `undefined reference to '.L8003....'` would mean something else.
 *
 * DECLARATIONS: this file carries its own copy of what its functions use,
 * split out of the old shared block.  Keep it that way -- do NOT create a
 * shared code_179d8*.h.  The sibling slices are staffed independently and a
 * shared header is what makes their merges collide; see
 * `python3 tools/headercontention.py`.
 *
 * BLOCKER PROFILE: screen with `python3 tools/nearmiss.py`, never by
 * re-implementing the greps and never for `addiu_at` (resolved round 21).
 * `nearmiss.py` runs `tools/sdkstalls.py` for you.  SsUtChangePitch carries a
 * HEAD SALVAGE body in its report (84/88 words, round 31) -- read the report
 * before starting, it is not cold ground.
 *
 * Expect this slice to span more than one class; identify each with
 * tools/classtable.py rather than assuming the unit has one.  Keep every
 * function in strict ROM-address order.
 */

#include "common.h"
#include "SvmData.h"

/* ------------------------------------------------------------------------
 * Cross-unit calls, typed per-call-site from the registers loaded before
 * each `jal` -- none of these callees have an established prototype yet, so
 * these are local guesses, not authoritative.  (This note used to add "several
 * are themselves addiu-$at blocked in their own units"; that is stale as of
 * round 21 and was removed rather than left to be believed.)  See CLAUDE.md's note on this.
 * ------------------------------------------------------------------------ */
/* SpuVmKeyOn (round 76, was StartNote) is Sony libsnd/vmanager INTERNAL --
 * unlike SsUtKeyOn (code_179d8_e.c), it has no public prototype in
 * LIBSND.H (grep confirms no `Vm`-prefixed extern anywhere in that
 * header), so this stays the byte-exact local-guess signature rather
 * than a header copy. */
extern s32 SpuVmKeyOn(s32 a0, s16 a1, s16 a2, u16 a3, u16 a4, u16 a5);
extern s32 SpuVmKeyOff(s32 a0, s16 a1, s16 a2, u16 a3);
extern s32 SpuVmVSetUp(s16 a0, s16 a1);
extern s16 SpuVmPBVoice(s16 a0, s32 a1, s16 a2, s16 a3, u16 a4);
extern void SeAutoVol(s16 a0, s16 a1, s16 a2, s16 a3);
extern void SeAutoPan(s16 a0, s16 a1, s16 a2, s16 a3);

/* Base pointer for a table of 0x10-byte entries, indexed by a 0..0x17
 * id.  Only the two leading s16 fields this unit's own accessors touch
 * are named. */
typedef struct EntryDAD4 {
    s16 unk0; /* +0x0 */
    s16 unk2; /* +0x2 */
    s16 unk4; /* +0x4 -- read by SsUtKeyOff, entry index 25 only */
    s16 unk6; /* +0x6 -- read by SsUtKeyOff, entry index 25 only */
    u8 pad8[0x10 - 0x8];
} EntryDAD4;

extern EntryDAD4 *D_8006DAD4;

/* STALL -- see docs/match-reports/SsUtChangePitch.md.  HEAD SALVAGE, round 31,
 * confirmed round 32 (permuter, ~54k iterations, not closed). Round 36:
 * rebuilt with SpuVmVSetUp's real name (was func_80032148 in the report's
 * preserved body -- round 34's SDK conversion renamed the callee, and the
 * report's body was never corrected). Measured 84/88 words, exact length,
 * matching the prior figure exactly; a barrier between the D_8008EA26/
 * D_8008EA22 stores (the one untested lever the report flagged) blows the
 * function up drastically instead of fixing the swap -- see this round's
 * report update. */
INCLUDE_ASM("asm/nonmatchings/code_179d8_j_c", SsUtChangePitch);

/* `dead` is never read and the write is unreachable; it exists to make GCC
 * allocate retail's empty 8-byte frame, which is what puts the two
 * stack-passed arguments at 0x18/0x1C($sp) instead of 0x10/0x14.  See this
 * function's match report -- the frame is the ONLY thing the idiom is for,
 * and adding anything else on top of it breaks the scheduling. */
s32 SsUtChangeADSR(s16 idx, s16 p1, s16 p2, s16 p3, u16 p4, u16 p5) {
    s32 dead[2];

    if ((u16)idx < 0x18) {
        if (_svm_voice[idx].unk16 != p1) {
            return -1;
        }
        if (_svm_voice[idx].unk12 != p2) {
            return -1;
        }
        if (_svm_voice[idx].unk0C != p3) {
            return -1;
        }
        _svm_sreg_buf[idx].unk8 = p4;
        _svm_sreg_buf[idx].unkA = p5;
        _svm_sreg_dirty[idx] |= 0x30;
        return 0;
    }
    if (0) {
        dead[0] = 1;
    }
    return -1;
}

s32 SsUtGetDetVVol(s16 idx, s16 *out1, s16 *out2) {
    if ((u16)idx < 0x18) {
        *out1 = D_8006DAD4[idx].unk0;
        *out2 = D_8006DAD4[idx].unk2;
        return 0;
    }
    return -1;
}

s32 SsUtSetDetVVol(s16 idx, s16 p1, s16 p2) {
    /* Retail reserves an 8-byte frame it never touches. Only an unused local
     * ARRAY of that size reproduces it -- a scalar is register-allocated and
     * eliminated, and a 4-byte array reserves the wrong amount. */
    s32 unused[2];

    if ((u16)idx < 0x18) {
        _svm_sreg_buf[idx].unk2 = p2;
        _svm_sreg_dirty[idx] |= 3;
        _svm_sreg_buf[idx].unk0 = p1;
        return 0;
    }
    return -1;
}

s32 SsUtGetVVol(s16 idx, s16 *out1, s16 *out2) {
    EntryDAD4 *e;
    s16 f0, f2;

    if ((u16)idx < 0x18) {
        e = &D_8006DAD4[idx];
        f0 = e->unk0;
        f2 = e->unk2;
        *out1 = f0 / 129;
        *out2 = f2 / 129;
        return 0;
    }
    return -1;
}

s32 SsUtSetVVol(s16 idx, s16 p1, s16 p2) {
    /* Retail reserves an 8-byte frame it never touches, same idiom as
     * SsUtSetDetVVol. */
    s32 unused[2];
    s16 t1, t2;

    if ((u16)idx < 0x18) {
        t1 = p1 * 129;
        t2 = p2 * 129;
        _svm_sreg_buf[idx].unk2 = t2;
        _svm_sreg_dirty[idx] |= 3;
        _svm_sreg_buf[idx].unk0 = t1;
        return 0;
    }
    return -1;
}

s32 SsUtAutoVol(s16 p0, s16 p1, s16 p2, s16 p3) {
    if ((u16)p0 < 0x18) {
        SeAutoVol(p0, p1, p2, p3);
        return 0;
    }
    return -1;
}

s32 SsUtAutoPan(s16 p0, s16 p1, s16 p2, s16 p3) {
    if ((u16)p0 < 0x18) {
        SeAutoPan(p0, p1, p2, p3);
        return 0;
    }
    return -1;
}
