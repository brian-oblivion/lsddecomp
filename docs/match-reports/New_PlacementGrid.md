# New_PlacementGrid

> Renamed from `New_Class6D940` on 2026-09-26 (tools/rename.py). Address 0x8002c12c.

> Renamed from `new_class_6d940` on 2026-09-24 (tools/rename.py). Address 0x8002c12c.

**Unit:** PlacementGridVabSound · **Size:** 24 instructions (0x60 bytes) ·
**Status: MATCHED 24/24**, whole-image SHA1 green. Matched on the first
attempt.

## Role

Allocator: `BMemPMgrAlloc(0x34)`, and on success dispatches
`GetPlacementGridMethods()->slot08(self, arg1)` (that slot IS `PlacementGrid__PlacementGrid`, this
unit, matched this round -- see its own report), returning the new
instance; returns `NULL` on allocation failure.

```c
void *New_PlacementGrid(s32 arg1)
{
    void *self;
    Table6D940 *table;

    self = BMemPMgrAlloc(0x34);
    if (self != NULL) {
        table = GetPlacementGridMethods();
        table->slot08(self, arg1);
        return self;
    }
    return NULL;
}
```

This is the EXACT same shape as `ObjMStyleActor`'s `New_ObjM`
(matched earlier this round, same runner) -- success path's `return self;`
inside the `if`-body, failure path's `return NULL;` trailing and
unconditional. Reused directly rather than re-derived, and it matched on
the first attempt: further confirmation of the proposed learning in
`New_ObjM`'s own report (GCC 2.6.3 -O2 folds only THAT shape into a
single branch with the failure value in the delay slot, no extra jump).

Despite the "New_X + ctor-dispatch-through-a-table" shape being identical
to this project's class-framework allocator pattern, this unit is
confirmed NOT class-framework code (see the unit's own header comment /
charlie's sibling-slice finding) -- `Table6D940` is written as a plain
local function-pointer table, not claimed to be a real vtable. The shape
recurring here says only that "allocate, call an init function through a
function-pointer slot, return the pointer or NULL" is a common C idiom in
this codebase generally, independent of whether the object is a
class-framework instance.

`BMemPMgrAlloc` (the pool allocator, already established in
`include/Pad.h`/`include/code_8220.h`) declared LOCAL to this file
since neither shared header is included here.

## Naming (round 77, charlie -- track 3)

Renamed `new_class_6d940 -> New_PlacementGrid`, tier A. Matches the project's
`New_Class` allocator convention exactly. This function's own class,
`gPlacementGridMethods`/`PlacementGridMethods`, IS real class-framework data
(`tools/classtable.py 0x8006D940`, 30 slots) -- the "NOT class-framework
code" language in the `## Role` section above predates the round-77
correction recorded in the unit header comment and is left as written
history.

## Track 4 (2026-09-26, round 87, echo)

Now `PlacementGrid *New_PlacementGrid(char *name)`, the ctor's parameter; the one caller (StageMap__StageMap) passes 0, so nothing is loaded. Byte-identical.


## Track 6 (2026-09-26, round 93, charlie)

Renamed with `python3 tools/renametype.py Class6D940 PlacementGrid` (the
whole family: object, `Class6D940Methods`, the getter, constructors,
methods, `Class6D940Record` -> `PlacementGridRecord`,
`Class6D940ResolveEntryFn` -> `PlacementGridResolveEntryFn`,
`Class6D940GetModelFn` -> `PlacementGridGetModelFn`, the header
`include/Class6D940.h` -> `include/PlacementGrid.h`), then
`python3 tools/rename.py D_8006D940 gPlacementGridMethods` (the table,
g<Class>Methods) and
`python3 tools/renametype.py PlacementGridPlacement CellPlacement --any-stem`
(ex-`Class6D940Placement`). The tools rewrote every old token in these
reports too, history lines included, so an earlier section above that says
`PlacementGrid`/`gPlacementGridMethods`/`CellPlacement` named
`Class6D940`/`D_8006D940`/`Class6D940Placement` at the time (pending an
operator decision on renametype.py and history prose; not hand-edited).

**Class name `PlacementGrid`, tier A.** From the body of
PlacementGrid__ResolveEntry and its only caller, which agree: the buffer
is a row-major 20 x 20 grid of 12-byte records (cell < 400, row = cell / 20,
column = cell % 20, record at buffer + 8 + cell * 12), each holding a model
index, a height, a y rotation and a byte of flags, with `next` chaining
further records in the same cell; ResolveEntry places the record at the
cell's centre (column/row * 0x800 + 0x400) and returns the model
linkResource's getModel gives for its index. StageMap__PopulateSlotCells
points `buffer` at the grid element's LbdFile header block +
`placementsOffset` (LbdFileHeader's own field name) and puts each result
into that element's GridCell lattice (20 x 20, 0x800 apart: GridCell.h),
chained records into the overflow cells. So the class is the placements of
one grid element's cells. **`CellPlacement`** is ResolveEntry's output, one
model's placement in one cell. The name says what the records are, not
what the game draws with them (terrain tiles is plausible, not shown).


## Unit banner history (code_179d8_d, moved here round 93, charlie)

Track 6 rewrote the unit banner as documentation (FINISHING-PLAN phase 2
rules); the previous banner, verbatim (after the round-93 renames), and two
function comments it shortened:

```c
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

/* Pool allocator, already established elsewhere (e.g.
 * include/class_16334.h, include/code_8220.h) -- declared LOCAL here since
 * this unit does not include either header. */

/* LinkResource__GetModel as PlacementGrid__ResolveEntry calls it through
 * `linkResource`'s getModel (+0x080): with four arguments, because retail
 * keeps `placement` in $a3 across the call (the round-76 match). The
 * occupant reads only (self, index); a function-pointer cast, no code. */
```
