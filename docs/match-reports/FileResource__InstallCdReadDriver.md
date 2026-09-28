# FileResource__InstallCdReadDriver -- MATCHED (18/18 words)

> Renamed from `Class6D430__InstallCdReadDriver` on 2026-09-26 (tools/rename.py). Address 0x80028898.

> Renamed from `func_80028898` on 2026-09-21 (tools/rename.py). Address 0x80028898.

Unit: `code_179d8_h`. Runner: echo, round 17 (second assignment).

## Result

```c
void FileResource__InstallCdReadDriver(FileResource *self) {
    ((FileResourceMethods *)GetFileResourceMethods())->ctor(self);
    self->methods = GetCdDriverMethods();
    self->pendingGeneration = 0;
}
```

(`self->unk0C` above was `GameApplicationFileResource.h`'s own field, renamed to
`pendingGeneration` by that unit's owner before this round; this report's
code sample was stale and is corrected here, round 64.)

with `#include "GameApplicationFileResource.h"` (already-established header, reused
UNCHANGED -- not copied or redefined) and a new local
`extern FileResourceMethods *GetCdDriverMethods(void);`.

Byte-exact, 18/18 words.

## Notes

The standard "chain to base ctor, then install the derived vtable" idiom:
calls `gFileResourceMethods`'s own ctor slot (`GetFileResourceMethods()->ctor`, i.e.
`FileResource__FileResource`, matched in `GameApplicationFileResource.c`) directly rather than through
`self->methods` (since `self->methods` isn't set up yet), then overwrites
`self->methods` with `GetCdDriverMethods()` -- a DIFFERENT class table
(`gCdDriverMethods`, confirmed via `tools/classtable.py --scan`: 29 slots, header
`0x13`, vs. `gFileResourceMethods`'s 0x03). `GetCdDriverMethods` itself is still uncarved
(`asm/code_179d8.s`); typed against `FileResourceMethods` for the
assignment only -- the two classes are different but share the base's
leading slot layout, which is all the type is asked to express here.

Needed an explicit cast (`(FileResourceMethods *)`) on
`GetFileResourceMethods()`'s result: it returns plain `void *` (per its own
established signature in `GameApplicationFileResource.c`), so `->ctor` on the bare call
doesn't compile without one -- first attempt failed with `request for
member 'ctor' in something not a structure or union`.

### Proposed learning

`GameApplicationFileResource.h`'s `FileResource`/`FileResourceMethods` describe a
class that OTHER units' functions construct/chain into, not just
`GameApplicationFileResource.c`'s own methods -- worth checking this header before
redefining a local struct whenever a function dispatches through
`GetFileResourceMethods()` or receives a `self` whose fields line up with its
offsets. `GetFileResourceMethods()` itself returns bare `void *`, so every external
call site needs its own cast to the slot-bearing type; this is not
`GameApplicationFileResource.c`'s problem to fix (its own call sites go through
`this->methods`, already correctly typed).

## Naming (round 64, runner alpha)

`func_80028898` -> `FileResource__InstallCdReadDriver`, tier B. `self` is
`FileResource*`, the header's own established type (this match is byte-exact
against that typing) -- so `FileResource__` follows track 3's convention
letter-for-letter ("methods `Class__Method`, where `Class` is the struct's
type name"). The rest of the name describes only confirmed MECHANICS: chain
to `FileResource`'s own ctor, then overwrite `self->methods` with
`GetCdDriverMethods()`'s table -- `gCdDriverMethods`, independently named
elsewhere in the tree (`src/code_179d8_q.c`'s own header comment) as "the
CD-ROM read driver", not a guess coined here. WHICH broader class or game
subsystem this function itself belongs to (why a `FileResource` instance gets
reclassified this way here, distinct from `src/code_179d8_o.c`'s own
confirmed ctor `CdDriver__CdDriver` for the same `gCdDriverMethods` class) is NOT
established -- no caller is visible yet (only referenced from the
still-uncarved `code_179d8` remainder) and this is flagged as such rather
than guessed at. See `FileResource__DestroyCdReadDriver.md` for the paired
dtor.

## Unit banner history (round 99, echo, track 7)

`src/code_179d8_h.c`'s banner and file-level comments were rewritten as
documentation in round 99 (track 7); the project history they carried is
kept here verbatim. Names in them are as of round 98.

The banner:

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
```

The note above the unit's local declarations:

```c
/* GetCdDriverMethods, gCdDriverMethods and CdDriver are include/CdDriver.h's
 * (track 4, round 88). OpenCdFile/CloseCdFile/GetCdFileSize/ReadCdFile take
 * the object CdDriver's methods were handed (any FileResource client: see
 * CdDriver.h's banner); they were typed against this unit's own
 * ObjA34_179D8H view, whose isOpen/pos/size are FileResource's +0x00C/+0x018/
 * +0x01C and whose `close` slot is FileResource's +0x048 (CdDriver__Close
 * in gCdDriverMethods; FileResource__LoadFile calls it on its success path
 * too, so the give-up paths below close the file). */
```

The note on the unit's local externs:

```c
/* func_800270B8 is code_171e0.c's; strcpy and strcat are Sony's
 * (lib/libc2/strcpy.o, lib/libc2/strcat.o, linked since round 34) --
 * declared LOCAL, per-call-site typed, never via a shared header. */
```

## Naming (round 99, echo, track 7)

Measured: the executable holds no `jal` to this function and no 32-bit word
equal to its address (a scan of every aligned word of `disk/SLPS_015.56`'s
image for both encodings), so it has no caller and sits in no table. The
round 64 note that it is "referenced only from the still-uncarved
`code_179d8` remainder" no longer holds; it is unreferenced, at least by
`jal`, stored pointer, or `lui`/`addiu`/`ori` address build (no `addiu`
or `ori` anywhere in the image carries its address's low half).
