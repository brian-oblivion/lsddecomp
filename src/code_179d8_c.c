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
 * code_179d8_c -- window [200..219] of the original 274-function code_179d8
 * monolith, now 0x22A1C..0x22BA8 (vram 0x8003221C..0x800323A8).
 *
 * ROUND 33: this slice lost TWO functions to Sony and was split in half.
 *   - func_80032148, its old FIRST function, is `SpuVmVSetUp`
 *     (`libsnd/vm_vsu.o`, Psy-Q 3.3), linked from the object.
 *   - func_800323A8 (120w) is `SsSetTableSize` (`libsnd/sstable.o`, Psy-Q
 *     3.5), linked from the object. It sat in the MIDDLE, so the slice became
 *     [c code_179d8_c][o sstable][c libsnd_ssinit_libapi_counter] and everything from
 *     SsSetTickMode on now lives in `src/libsnd_ssinit_libapi_counter.c`.
 * Neither was ever matchable as C; both stall reports are kept, re-titled
 * CONVERTED. This unit is now three functions: _SsInit and its two
 * one-line callers.
 *
 * THE 0x14D8 RODATA ATTACH IS NO LONGER OURS. It belongs to SsSetTickMode
 * (jtbl_80010CD8), which went to libsnd_ssinit_libapi_counter, and the yaml attach moved
 * with it. Do not move it back.
 *
 * Carved round 16 by blocker DENSITY (see libcd_bios's header for the
 * full window census). This window screened 16/20 clean.
 *
 * STALE CLAIM REMOVED, round 23 (2026-09-07): this comment listed
 * func_80032148, SsSetTickMode, StartRCnt and StopRCnt as blocked,
 * "all addiu_at". **`addiu_at` was resolved in round 21** (maspsx
 * `--addiu-at`; docs/research/addiu-at-blocker.md), and round 23 MATCHED
 * StartRCnt and StopRCnt byte-exact and took the other two to
 * 48/53 and ~90/96 with characterised non-toolchain residues. (The 48/53 was
 * func_80032148 -- round 32 then found it is Sony's, so that effort was spent
 * on library code; see docs/match-reports/func_80032148.md.) Nothing in
 * this window is toolchain-blocked. The two live blockers are `gp_rel` and
 * `nop_mflo_mfhi`; screen with `python3 tools/nearmiss.py`, never by
 * re-implementing the greps, and never for `addiu_at`.
 *
 * Round 23's runner bravo spotted this line as stale and correctly did not
 * edit it (parallel-mode rules); the head fixed it at consolidation.
 *
 * Note _SsSeqCalledTbyT_1per2/SetRCnt: this window holds what look like Psy-Q root
 * counter routines linked into game text rather than into a psyq_* segment.
 * They are ordinary work, but do not generalise a finding from them to the
 * game's own code.
 *
 * This slice was cut at ROM-address boundaries, so it has no reason to
 * align with class boundaries -- expect it to span more than one class,
 * and identify each with tools/classtable.py rather than assuming one.
 *
 * Declarations: keep anything that encodes THIS unit's reading of a class
 * next to the code, in this file. Do not create a shared code_179d8*.h --
 * the sibling slices are staffed independently and a shared header is what
 * makes their merges collide.
 *
 * ROUND 78 (track 3 naming pass): this unit has no class (not in
 * `classtable.py --scan`) -- it is Sony's sound-system init, reconstructed
 * as C rather than linked as an object, matching the `libsnd/vm_vsu` and
 * `libsnd/sstable` objects placed immediately before and after it.
 * `_SsInit` is already Sony-identified and MATCHED (round 63); leave it.
 * `SsInit`/`SsInitHot` (its arg=0/arg=1 wrappers) are Sony's too: the
 * head identified them under track 2 (EXACT fingerprint, the ssinit slot
 * position, and the `_SsInit(0)`/`_SsInit(1)` bodies), and `_SsInit`'s
 * mode!=0 arm calls Sony's `SpuInitHot` (libspu_s_ih), identified the
 * same way. All three count as library.
 *
 * `_SsInit`'s data: `_snd_openflag`/`_snd_ev_flag` are Sony-pinned in
 * `config/psyq-objects.ld` and carry those names (the head applied them
 * with `rename.py` in round 78). The two SPU
 * register-init templates `D_8006DC5C`/`D_8006DC6C` and the per-voice
 * state array `D_80090368` are read only by `_SsInit` itself (a Sony
 * function), so per the "field only Sony functions read" rule they are
 * left unnamed rather than given game names, even though the two
 * templates are textually unique to this unit.
 */
#include "common.h"


extern s32 ResetCallback(void);
extern void SpuInit(void); /* Psy-Q LIBSPU.H: extern void SpuInit (void); -- track 2 identification, round 64 */
extern void SpuInitHot(void); /* Sony libspu/s_ih, track 2 identification, round 78 (not in this SDK's LIBSPU.H) */
extern void SpuVmInit(s32 arg0);
extern s32 GetVideoMode(void);
extern u16 D_8006DC5C[8];
extern u16 D_8006DC6C[0x10];
extern s32 VBLANK_MINUS;
extern s32 _snd_openflag;
extern s32 _snd_use_vsync_cb;
extern s32 _snd_use_interrupt_id;
extern s32 _snd_1per2;
extern void (*_snd_vsync_cb)(void);
extern s32 _snd_video_mode;
extern s32 _snd_ev_flag;

/* One 0x40-byte per-voice software state slot; the init below just zeroes it. */
typedef struct {
    s32 pad0[0x10];
} VoiceState80090368;

extern VoiceState80090368 D_80090368[0x20];

/*
 * Sound-system init.  Reached only through SsInit (arg0 = 0) and
 * SsInitHot (arg0 = 1) below.
 *
 * DO NOT "TIDY" THE LOOP VARIABLES -- the pairing is byte-load-bearing
 * (round 63).  Retail reuses exactly two counter pseudos across all three
 * loops and SWAPS their outer/inner roles in the last one: `i` is the outer
 * counter of loops 1-2 and the INNER counter of loop 3, `j` the inner counter
 * of loop 1 and the OUTER counter of loop 3.  Writing loop 3 as
 * `for (i ...) for (j ...)` costs 35 words to register renames; splitting
 * them into per-loop names costs the function's size outright.  Likewise the
 * `i = 0;` before each loop is a statement in its own right, not a `for`
 * init clause: retail zeroes the counter BEFORE loading the source base.
 */
void _SsInit(s32 arg0) {
    s32 i, j;
    u16 *base;
    u16 *src;
    u16 *reg;

    ResetCallback();

    if (arg0 == 0) {
        SpuInit();
    } else {
        SpuInitHot();
    }

    /* Stamp the same 8-halfword template into all 24 SPU voice register
       blocks (0x1F801C00, stride 0x10 -- `reg` runs straight through). */
    reg = (u16 *)0x1F801C00;
    i = 0;
    base = D_8006DC5C;
    for (; i < 0x18; i++) {
        j = 0;
        src = base;
        for (; j < 8; j++) {
            *reg = *src;
            src++;
            reg++;
        }
    }

    /* Then 16 consecutive halfwords into the SPU control block at 0x1F801D80. */
    reg = (u16 *)0x1F801D80;
    i = 0;
    src = D_8006DC6C;
    for (; i < 0x10; i++) {
        *reg = *src;
        src++;
        reg++;
    }

    SpuVmInit(0x18);

    for (j = 0; j < 0x20; j++) {
        for (i = 15; i >= 0; i--) {
            D_80090368[j].pad0[i] = 0;
        }
    }

    VBLANK_MINUS = 0x3C;
    _snd_openflag = 0;
    _snd_use_vsync_cb = 0;
    _snd_use_interrupt_id = -1;
    _snd_1per2 = 0;
    _snd_vsync_cb = NULL;
    _snd_video_mode = GetVideoMode();
    _snd_ev_flag = 0;
}

void SsInit(void) {
    _SsInit(0);
}

void SsInitHot(void) {
    _SsInit(1);
}
