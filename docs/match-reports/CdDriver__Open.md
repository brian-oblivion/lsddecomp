# CdDriver__Open -- MATCHED (round 47, alpha)

> Renamed from `Class6D4E8__Open` on 2026-09-26 (tools/rename.py). Address 0x800272d0.

> Renamed from `func_800272D0` on 2026-09-25 (tools/rename.py). Address 0x800272d0.

108/108 words, byte-exact, file 0x17AD0-0x17C80. Cold ground. First function
in ROM order in this unit, so the shared `Obj80027480` local struct and
`gCdAsyncEnabled`/`gCdSyncQueueMode`/`gCdBusy`/`LockCd`/`StartCdOperation`/
`ResetCdStateMachine`/`EnqueueCdRequest`/`UnlockCd` externs were moved ahead of
it (they were previously declared between it and `CdDriver__Close`).

## Source

```c
typedef struct Pos18 {
    s16 unk0;
    s16 unk2;
} Pos18; /* alignment 2, matches the project's lwl/lwr+swl/swr idiom */

/* (Obj80027480's unk18 field is this type, not a raw byte array -- see
   below.) */

typedef struct Rec80028448 {
    u8 pad0[0x14];
    Pos18 unk14;
    u32 unk18;
} Rec80028448;

extern void *FindCdFileEntry(char *arg0);
extern s32 FindCdFileIndex(char *arg0);
extern void *gCdSeekParam;
extern s32 gCdTickStep;

extern void OpenCdFile(Obj80027480 *self, char *suffix);
extern char *BuildCdFilePath(char *dest, char *suffix);
extern s32 CdSearchFile(void *statBuf, char *path);
extern void CdControl(s32 arg0, void *buf, s32 arg2);
extern s32 CdSync(s32 mode, void *result);

typedef struct StatBuf80027 {
    Pos18 unk0;
    u32 unk4;
    u8 pad8[0x18 - 8];
} StatBuf80027;

void CdDriver__Open(Obj80027480 *self, char *suffix, s32 arg2, s32 arg3) {
    char path[0x40];
    StatBuf80027 statBuf;
    Rec80028448 *rec;
    s32 temp;
    s32 v0;

    if (gCdAsyncEnabled == 0 && gCdSyncQueueMode == 0) {
        OpenCdFile(self, suffix);
        return;
    }
    LockCd();
    if (self->unk28 != 0) {
        if (gCdBusy == 0 && self->unk0C == 0) {
            StartCdOperation(1, 1);
            if (gCdAsyncEnabled != 0) {
                rec = FindCdFileEntry(suffix);
                gCdSeekParam = rec;
                if (rec == NULL) {
                    return;
                }
                self->unk18 = rec->unk14;
                temp = ((Rec80028448 *)gCdSeekParam)->unk18;
                gCdTickStep = 1;
                self->unk0C = 1;
                self->unk1C = temp;
            } else {
                BuildCdFilePath(path, suffix);
                do {
                } while (CdSearchFile(&statBuf, path) == 0);
                self->unk18 = statBuf.unk0;
                self->unk1C = statBuf.unk4;
                do {
                    CdControl(2, &self->unk18, 0);
                    do {
                        v0 = CdSync(0, 0);
                    } while (v0 == 0);
                } while (v0 == 5);
                self->unk0C = 1;
                ResetCdStateMachine();
            }
        }
    } else {
        EnqueueCdRequest(self, FindCdFileIndex(suffix), 2, arg2, arg3);
    }
    UnlockCd();
}
```

## What it took, in order

1. **`self->unk18` was changed from `u8[4]` to a 2-field `s16` struct
   (`Pos18`)** so that `self->unk18 = rec->unk14;` and
   `self->unk18 = statBuf.unk0;` compile as whole-struct assignments (the
   established alignment-2 `lwl`/`lwr` + `swl`/`swr` idiom), matching
   retail's unaligned 4-byte copies at both call sites. This is a shared-file
   type used by three OTHER already-matched functions in the unit
   (`CdDriver__Close`, `CdDriver__Read`, `CdDriver__Seek`); only
   `CdDriver__Seek`'s `CdPosToInt(self->unk18)` needed updating to
   `CdPosToInt(&self->unk18)` since the field no longer decays to a pointer
   on its own. Re-verified all three still match after the type change.

2. **`OpenCdFile` is void** (established already in its own match report,
   `CdDriver.c`), so the early-return branch is `OpenCdFile(self,
   suffix); return;`, not a forwarded return value -- no ambiguity here since
   the callee's void-ness was already on file, unlike the general wrapper
   caution.

3. **The `NEW STALL CLASS` from round 44 (`Viewport__InitDefaults`): retail
   recomputes/re-reads an address our GCC CSEs away.** After
   `gCdSeekParam = rec;`, retail reloads `gCdSeekParam` from memory a second
   time (`lw v0,%gp_rel(gCdSeekParam)`) to read `->unk18`, instead of reusing
   the register that still holds the identical value (`rec`/`a2`). My first
   attempt wrote `self->unk1C = ((Rec80028448 *)gCdSeekParam)->unk18;` as the
   LAST statement in the branch (after the two flag stores
   `gCdTickStep = 1; self->unk0C = 1;`), and GCC's CSE collapsed the global
   read into reusing `rec`'s register anyway -- 1 word short.
   **Moving that same statement to IMMEDIATELY after the first field copy
   (`self->unk18 = rec->unk14;`), before the two flag stores, was enough to
   make GCC keep it as a genuine reload** (matched exactly, no barrier or
   `volatile` needed). This is the reverse-direction sibling of round 44's
   finding: same underlying CSE, but here SOURCE ORDER (not a barrier)
   was the lever, unlike that entry's own negatives (barrier/volatile/
   per-field copies were all inert there). Worth recording as a THIRD
   axis: order-of-statements can defeat this CSE even when explicit
   anti-optimization constructs cannot.

4. **The real find: reusing ONE C-level `s32` local across two MUTUALLY
   EXCLUSIVE `if`/`else` branches can still perturb register allocation in
   the OTHER branch, even though the two uses never execute on the same
   path.** My first draft used a single `s32 v0;` for both (a) the
   `gCdSeekParam` reload above and (b) the `CdSync` retry loop's return value
   in the sibling `else` branch. That produced an EXTRA `move v1,v0` right
   after the `CdSync` call, with the loop's own comparisons then reading
   `$v1` -- retail reads `$v0` directly, with no move at all (like
   `CdDriver__Seek`'s analogous loop). Isolated through the pinned pipeline
   (`/tmp/.../t6.c` through `t8.c`): the SAME retry-loop code, byte-for-byte,
   produces the extra `move` when the sibling branch also assigns into a
   variable named `v0`, and produces retail's exact no-move shape when that
   sibling branch's value is given a DIFFERENT name (`temp`) instead. Fixed
   by declaring a second local (`temp`) for use (3) above and reserving `v0`
   solely for the `CdSync` loop.

## Struct notes

`Obj80027480.unk18` is now `Pos18` (was `u8[4]`) -- a CdlLOC-shaped 4-byte
position, alignment 2. `Rec80028448` is a local view of the 0x1C-byte string
records at `gFileTable` (`src/cd/CdDriver.c`'s own comment already
describes this table); only the trailing two fields this function reads are
named. `StatBuf80027` is this unit's OWN local view of the CD stat buffer
`CdDriver.c`'s `OpenCdFile` already independently discovered as
`StatBuf179D8H` -- same shape, declared separately per the project's
multiple-local-views convention (not shared, since it is that OTHER unit's
own reading).

### Proposed learning

**Two independent axes both surfaced in one function and must not be
conflated:**

- **CSE-defeat by REORDERING, not by construct.** When retail re-reads a
  global that your GCC provably CSEs into an already-live register, moving
  the redundant read EARLIER in source order (right after the value that
  makes the two reads "obviously equal", rather than after unrelated
  intervening statements) can be enough on its own -- try this before
  reaching for `volatile`/barriers, which round 44 already found inert for
  the general case.
- **A single C variable reused across two branches that never execute
  together is not register-neutral.** If one branch's residue is an
  unexplained EXTRA `move $vN,$v0` (or the reverse) around a call return
  that retail reads raw, check whether that same-named local is ALSO
  assigned in a sibling branch earlier in the function -- giving the two
  uses independent names can remove the spurious `move` even though the
  branches are mutually exclusive and a from-source reading suggests no
  interaction. Confirmed via isolated pinned-pipeline reproduction, not
  just observed once.

## Naming

Round 79 (charlie), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_800272D0` | `CdDriver__Open` | A |

**Evidence.** With the driver in plain sync mode it tail-forwards to
`OpenCdFile` (CdDriver). Otherwise, outside a queue dispatch, it
enqueues request op 2 with `FindCdFileIndex(name)`; inside one, if the
object is not already open, it looks the name up (`FindCdFileEntry` on the
async path, `BuildCdFilePath` + `CdSearchFile` on the blocking path), copies
the entry's disc position and byte size into `self->pos`/`self->size`, seeks
there (CdlSetloc) and sets `self->isOpen = 1`. The base-class caller
`FileResource__LoadFile` calls this slot first, with the file name, before
sizing and reading. Every path is "resolve a file by name and make it the
open one", hence tier A. `arg2`/`arg3` are only passed through to the
queued request and are not read otherwise, so they stay unnamed.

**Class prefix.** `Class6D4E8` is the placeholder token for the method
table `gCdDriverMethods` (the convention `CdDriver__RequestLoadFile` and its two
siblings already use); `tools/classtable.py gCdDriverMethods` lists this function
at slot `+0x044`. The prefix names the table, not the developers' class.

## Field, slot and type names (this unit's local views, APPLIED)

All of these are local typedefs of CdDriver.c, which includes no project
header, so the compiler's accessor list after renaming the definitions was
this unit alone (69 `has no member` errors, all in CdDriver.c); both
oracles green after.

| type (was) | field (was) | now | tier | evidence |
| --- | --- | --- | --- | --- |
| `Obj80027480` | -- | `Class6D4E8` | A | the object whose methods are `gCdDriverMethods`'s slots |
| `Methods80027480` | -- | `Class6D4E8Methods` | A | `gCdDriverMethods` itself |
| `Pos18` | -- | `CdLoc16` | A | CdDriver's name for the same 2-aligned CdlLOC shape |
| `Rec80028448` | -- | `CdFileEntry` | A | CdDriver's name for the 0x1C-byte file-table record |
| `StatBuf80027` | -- | `CdFileInfo` | A | CdDriver's name for CdSearchFile's CdlFILE output |
| `Node8008A894` | -- | `CdRequestNode` | A | CdDriver's name for the queue node |
| `Class6D4E8` | `unk0C` | `isOpen` | A | set 1 by Open, 0 by Close; Seek/Read require it; CdDriver's name |
| `Class6D4E8` | `unk10` | `buffer` | A | LoadFile's BMemPMgrAlloc result / read target; FileResource's name |
| `Class6D4E8` | `unk14` | `bufferSize` | A | LoadFile stores the sector-rounded read size; FileResource's name |
| `Class6D4E8` | `unk18` | `pos` | A | the file's disc position, CdlSetloc target; CdDriver's name |
| `Class6D4E8` | `unk1C` | `size` | A | the file's byte size from the entry / CdSearchFile; CdDriver's name |
| `Class6D4E8` | `unk20` | `freeGuard` | B | FileResource's name; here only: nonzero lets LoadFile reuse an existing buffer |
| `Class6D4E8` | `unk22` | `pendingRequests` | A | CdDriver's name; RunRequestQueue decrements it per completion |
| `Class6D4E8` | `unk24` | `flags` | A | CdDriver's name; only ORed with CD_FLAG_* bits |
| `Class6D4E8` | `unk28` | `inQueueDispatch` | A | written only by the ctor (0) and RunRequestQueue (1 around the dispatch call, 0 after); every method starts its operation when set, enqueues when clear |
| `Class6D4E8Methods` | `slot44` | `open` | A | resolves to `CdDriver__Open` |
| `Class6D4E8Methods` | `onError` | `close` | A | resolves to `CdDriver__Close` (see that report) |
| `Class6D4E8Methods` | `slot4C` | `seek` | B | resolves to `CdDriver__Seek` |
| `Class6D4E8Methods` | `slot54` | `read` | A | resolves to `CdDriver__Read` |
| `Class6D4E8Methods` | `slot58` | `loadFile` | A | resolves to `CdDriver__LoadFile`; CdDriver's name |
| `Class6D4E8Methods` | `slot64` | `setFlag` | A | resolves to `FileResource__OnRequestDone` |
| `Class6D4E8Methods` | `slot70` | `stopCdService` | A | resolves to `CdDriver__StopService` |
| `CdFileEntry` | `pad0`/`unk14`/`unk18` | `name`/`pos`/`size` | A | CdDriver's CdFileEntry |
| `CdFileInfo` | `unk0`/`unk4` | `pos`/`size` | A | CdDriver's CdFileInfo |
| `CdRequestNode` | `unk0`,`unk8`,`unkC`,`unk10`,`unk14`,`unk18` | `active`,`op`,`owner`,`fileIndex`,`param0`,`param1` | A (params B) | EnqueueCdRequest's writes and StartCdOperation's `active = 1`, per their reports' proposals |

`CdRequestNode::unk4` keeps its placeholder: it is zeroed at allocation and
its one reader here ORs flags bit 0, but no writer of a nonzero value is
identified (CdDriver's `UnkC80::unk04` store goes through an
uninitialised pointer).

Globals: `D_8006D574` -> `gCdSeekLoc` (A: 8 bytes of .data written only by
`CdDriver__Seek`'s `CdIntToPos` and used as its CdlSetloc target).
`gCdSyncQueueMode` keeps its placeholder for CdDriver's stated reason: every
read here is the `gCdAsyncEnabled == 0 && gCdSyncQueueMode == 0` sync-mode test and
nothing names the second mode.


Track 4, 2026-09-26 (round 88). The class of gCdDriverMethods (was D_8006D4E8, id 0x13 = DATASOURCE_CD) is CdDriver, in include/CdDriver.h: its ctor calls InitCdDrive, its slots enqueue CD_OP_* requests and drive the CD read state machine, and it is VabDriver's sibling. The object views this function was typed against are replaced by CdDriver, whose fields are all FileResource's (the driver runs on its clients' objects; FileResource's +0x018/+0x01C were named pos/size for it). Byte-identical. `Class6D4E8__Open` -> `CdDriver__Open` by rename.py.

### Round 100 (charlie, track 7): `D_8008A860` -> `gCdSyncQueueMode`, tier B

The paragraph above predates this rename (the tool rewrote the name in it).
What the five methods' bodies show, read together: the mode word only ever
matters when `gCdAsyncEnabled` is 0. Both 0: each method forwards straight
to CdDriver's blocking call and never touches the queue. `gCdAsyncEnabled`
0 and this word nonzero: the call is enqueued like an async one, and when
`CdDriver__RunRequestQueue` dispatches it back the method runs it as a
blocking CdControl/CdSync/CdRead spin on the spot. So the word selects
"synchronous, but through the request queue". Its one nonzero writer is
`SetCdDriverMode(async, 1, 1)` reached from DayTaskStageMap through
`SetActiveDataSourceDriverMode(syncDriver == 0, 1, 1)`; every other caller
passes 0. Tier B: the mechanics are the bodies', why the game wants the
queued blocking mode is not established.

### History (moved from code_179d8_s.c, round 100)

Until round 100 the unit spelled Sony's libcd itself: its own prototypes for
CdControl, CdIntToPos, CdPosToInt, CdRead, CdReadSync, CdSearchFile and
CdSync, a local `CdFileInfo` view of CdlFILE (only `pos`/`size` typed), and
`CD_CMD_SETLOC 2`, `CD_MODE_DOUBLE_SPEED 0x80`, `CD_SYNC_DISK_ERROR 5` for
CdlSetloc, CdlModeSpeed and CdlDiskError. It now includes `<libcd.h>`;
`statBuf` is a CdlFILE, and since `self->pos` is still FileResource.h's
CdLoc16 the copy is `*(CdLoc16 *)&statBuf.pos` (the same 2-aligned struct
move as before, byte-identical). A comment block "CdDriver, its table and its
methods are include/CdDriver.h's (track 4, round 88)" was dropped from the
unit: the banner now says it, and the Track 4 paragraph above has the
history. `arg2`/`arg3` are now `param0`/`param1`, the queue node fields they
are stored in. Item 4's `temp` is today's `size`; its `MATCHING:` line is in
the source.
