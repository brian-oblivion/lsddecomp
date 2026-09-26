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
 * code_179d8_d -- window [100..119] of the original 274-function code_179d8
 * monolith, originally 0x1C440..0x1CC08 (vram 0x8002BC40..0x8002C408).
 *
 * ROUND 34 (head): the unit's FIRST SIX functions left it. CD_searchdir
 * (func_8002BC40), CD_cachefile (func_8002BCEC, the 175w stall), cd_read
 * (func_8002BFA8) and iso9660's own WEAK memcpy (func_8002C014) are the tail
 * of `libcd/iso9660.o` (Psy-Q 3.3), which starts in libcd_bios; strcmp
 * (func_8002C048) and strncmp (func_8002C0AC) are `libc2/strcmp.o` and
 * `libc2/strncmp.o`. Five had been matched as C -- they were Sony's the whole
 * time, and reclassifying them out of the game count is the correction
 * CLAUDE.md asks for, not a regression. The unit is now 0x1C92C..0x1CC08
 * (vram 0x8002C12C..), New_PlacementGrid onward, 14 functions. The ISO9660
 * directory-record views and diagnostic-string externs that lived here went
 * with the functions; the stall's preserved body is in its report.
 *
 * Carved MID-round 16, to re-staff a runner whose own unit was exhausted.
 * Screened 17/20 clean on the three-grep blocker census -- the highest of
 * any window in this monolith -- but that number overstates its value: 9 of
 * the 20 are 4-6 word leaves, and splat matched five of those itself as
 * empty `jr $ra; nop` bodies (VabDriver__VabDriver/Destroy/Close/Seek/NoOpSlot50 below).
 * Those five count as `matched` in tools/progress.py without having been
 * work, which is exactly the caveat CLAUDE.md attaches to that column.
 *
 * NEITHER OF THIS UNIT'S TWO "BLOCKED" FUNCTIONS IS BLOCKED.  Both were
 * filed as `addiu_at`, and `addiu_at` was RESOLVED in round 21 (maspsx
 * `--addiu-at`; docs/research/addiu-at-blocker.md).  Re-screened with
 * `python3 tools/nearmiss.py` on 2026-09-08 (round 24):
 *   func_8002BC40 (43w)   MATCHED round 24, 43/43, first attempt.
 *   func_8002BCEC (175w)  blocker-clean, and NOT GAME CODE. It lies fully
 *                         inside `libcd/iso9660.o` (Psy-Q 3.3), an object
 *                         already placed and verified against retail, so
 *                         no C matches it -- convert per
 *                         docs/SDK-OBJECTS-GUIDE.md, do not decompile.
 *                         553 lines of derivation were spent before
 *                         anyone asked. `tools/sdkstalls.py` asks.
 * Their stub reports are gone.  The previous version of this comment listed
 * both as "blocked, have stub reports", which by round 24 was a stale
 * DIRECTIVE over free ground -- the fourth unit in two rounds to carry one.
 * PlacementGrid__ResolveEntry was originally screened as a third (nop_mflo_mfhi) but
 * that screen was inverted (checked mult/div BEFORE mflo/mfhi instead of
 * after) -- the head corrected it mid-round and deleted the stub report.
 * It is fresh ground; the mult/mfhi pair in its body is retail's signed-
 * divide-by-constant idiom, not the blocked mflo/mfhi-then-mult direction.
 *
 * Unlike its siblings libcd_bios and code_179d8_c, this slice owns NO
 * jump table -- all seven jtbl blocks in the 0xFD8 rodata slot fall outside
 * 0x8002BC40..0x8002C408 -- so no rodata sub-slot is attached to it.
 *
 * ROUND 77 CORRECTION (naming pass, charlie): the paragraph below (round 16,
 * libcd_bios's sibling-slice finding) is WRONG for this unit and must not
 * be trusted for it again. `python3 tools/classtable.py --scan` DOES hit
 * this unit's own globals: `gPlacementGridMethods` is a real 30-slot FileResource-derived
 * vtable (header word 0x00000E03), confirmed by `tools/classtable.py
 * 0x8006D940` -- slots +0x004/+0x05C/+0x060 are the SAME
 * `FileResource__Release`/`FileResource__FreeBuffer`/`NoOp` symbols the base class and
 * its CD-driver sibling (`gCdDriverMethods`, code_179d8_q.c) share verbatim, +0x008
 * is a genuine ctor (`PlacementGrid__PlacementGrid`), +0x00C a genuine dtor
 * (`PlacementGrid__Finalize`), and `gPlacementGridMethods`'s own getter (`GetPlacementGridMethods`,
 * ex-`func_8002C3A8`) is registered in `gDataSourceClientGetters` (code_171e0.c) -- the
 * NULL-terminated array of "class-method-table getters of every
 * FileResource-derived client" -- as that array's FIRST entry
 * (`asm/data/5DB70.data.s`). So this unit's own class (kept address-named
 * `PlacementGrid`, no game-purpose evidence yet) is a real, registered
 * `SetActiveDataSource`-client sibling of `VabStreamObj`
 * (code_179d8_e.c) and the CD-read driver (code_179d8_q.c) -- do go looking
 * for `this->methods->slotN(this, ...)` dispatch here; it is real. This does
 * NOT extend to the REST of the unit's globals: no other classtable.py hit
 * exists in this window, so the "low-level control-word staging" read below
 * may still hold for whatever is not `gPlacementGridMethods`/`PlacementGridMethods`-shaped.
 *
 * Declarations: keep anything that encodes THIS unit's reading of the region
 * next to the code, in this file. Do NOT create a shared code_179d8*.h --
 * the sibling slices are staffed independently and a shared header is what
 * makes their merges collide.
 */
#include "common.h"
#include "VabDriver.h"
#include "PlacementGrid.h"
#include "LinkResource.h"

/* FileResource's, code_171e0.c: the active driver's table, through which
 * PlacementGrid's ctor, finalize and setFlag reach their parent's. */
extern FileResourceMethods *GetActiveDataSourceMethods(void);

/* Pool allocator, already established elsewhere (e.g.
 * include/class_16334.h, include/code_8220.h) -- declared LOCAL here since
 * this unit does not include either header. */
extern void *BMemPMgrAlloc(s32 size);

/* LinkResource__GetModel as PlacementGrid__ResolveEntry calls it through
 * `linkResource`'s getModel (+0x080): with four arguments, because retail
 * keeps `placement` in $a3 across the call (the round-76 match). The
 * occupant reads only (self, index); a function-pointer cast, no code. */
typedef s32 (*PlacementGridGetModelFn)(LinkResource *self, s32 model, s32 cell,
                                    PlacementGridPlacement *placement);

PlacementGrid *New_PlacementGrid(char *name) {
    PlacementGrid *self;
    PlacementGridMethods *table;

    self = BMemPMgrAlloc(0x34);
    if (self != NULL) {
        table = GetPlacementGridMethods();
        table->ctor(self, name);
        return self;
    }
    return NULL;
}

void PlacementGrid__PlacementGrid(PlacementGrid *self, char *name) {
    GetActiveDataSourceMethods()->ctor((FileResource *)self);
    self->methods = GetPlacementGridMethods();
    self->linkResource = NULL;
    self->loaded = 0;
    if (name != NULL) {
        self->methods->requestLoadFile(self, name);
    }
}

void PlacementGrid__Finalize(PlacementGrid *self) {
    GetActiveDataSourceMethods()->finalize((FileResource *)self);
}

void PlacementGrid__SetFlag(PlacementGrid *self) {
    self->loaded = 1;
    GetActiveDataSourceMethods()->setFlag((FileResource *)self);
}

s32 PlacementGrid__ResolveEntry(PlacementGrid *self, PlacementGridPlacement *placement, s32 cell) {
    PlacementGridRecord *rec;
    LinkResource *link;
    s32 row;
    s32 col;
    s32 model;

    if (cell < 0x190) {
        if (placement->next != 0) {
            rec = (PlacementGridRecord *)((u8 *)self->buffer + placement->next);
            placement->chained = 1;
        } else {
            rec = (PlacementGridRecord *)(cell * 12 + 8 + (u8 *)self->buffer);
            placement->chained = 0;
        }
        placement->next = rec->next;
        if (rec->present != 0) {
            row = cell / 20;
            col = cell - row * 20;
            placement->x = (col << 11) + 0x400;
            placement->y = (s32)rec->y << 11;
            placement->z = (row << 11) + 0x400;
            placement->rotY = rec->rotY << 10;
            placement->unk2C = rec->unk1;
            placement->unk2E = rec->unk4;
            model = rec->model;
            placement->model = model;
            link = self->linkResource;
            return ((PlacementGridGetModelFn)link->methods->getModel)(link, model, cell, placement);
        }
        return -1;
    }
    return 0;
}

PlacementGridMethods *GetPlacementGridMethods(void) {
    return &gPlacementGridMethods;
}

s32 func_8002C3B8(void) {
    return 0;
}

void VabDriver__VabDriver(void) {}

void VabDriver__Destroy(void) {}

void VabDriver__NoOpSlot40(void) {
    char buf[0x40];
}

void VabDriver__Open(void) {
    char buf[0x40];
}

void VabDriver__Close(void) {}

void VabDriver__Seek(void) {}

void VabDriver__NoOpSlot50(void) {}
