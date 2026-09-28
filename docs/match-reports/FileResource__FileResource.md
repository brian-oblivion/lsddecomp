# FileResource__FileResource

> Renamed from `Class6D430__Class6D430` on 2026-09-26 (tools/rename.py). Address 0x80026a50.

> Renamed from `func_80026A50` on 2026-09-18 (tools/rename.py). Address 0x80026a50.

**Unit:** GameApplicationFileResource · **Size:** 25 instructions · **Status:** MATCHED (25/25 words, whole-image build verified byte-exact)

## What it does

The constructor for the `gFileResourceMethods` class (its own vtable slot `+0x008`,
per `include/GameApplicationFileResource.h`'s `FileResourceMethods`). Chains the base
class's constructor first (`GetBasicClassMethods()->ctor(this)`), then installs
this class's own vtable pointer (fetched via the already-matched
`GetFileResourceMethods`, which just returns `&gFileResourceMethods`), then zeroes every field
this unit currently knows about.

## Derivation

```
jal   GetBasicClassMethods
 addu $s0, $a0, $zero        ; s0 = this
lw    $v0, 0x8($v0)          ; v0 = (base table)->ctor
jalr  $v0                    ; GetBasicClassMethods()->ctor(this)
jal   GetFileResourceMethods          ; v0 = &gFileResourceMethods
sw    $v0, 0x0($s0)          ; this->methods = v0
sw    $zero, 0xC($s0)        ; this->unk0C = 0
sw    $zero, 0x10($s0)       ; this->unk10 = 0
sw    $zero, 0x14($s0)       ; this->unk14 = 0
sh    $zero, 0x20($s0)       ; this->unk20 = 0
sh    $zero, 0x22($s0)       ; this->unk22 = 0
sw    $zero, 0x24($s0)       ; this->flags = 0
sh    $zero, 0x28($s0)       ; this->unk28 = 0
sh    $zero, 0x2A($s0)       ; this->unk2A = 0
```

This is where every offset in `FileResource` beyond `+0x24` (the only
field known before this unit's round) was derived — a straight-line
field-by-field zeroing matches straight-line C with no reordering needed.

## Final C

```c
void FileResource__FileResource(FileResource *this) {
    GetBasicClassMethods()->ctor(this);
    this->methods = (FileResourceMethods *) GetFileResourceMethods();
    this->unk0C = 0;
    this->unk10 = NULL;
    this->unk14 = 0;
    this->unk20 = 0;
    this->unk22 = 0;
    this->flags = 0;
    this->unk28 = 0;
    this->unk2A = 0;
}
```

## Attempt log (abbreviated)

Matched on the second attempt in isolation — the first showed 24/25 in-range
with one call-target (`jal GetFileResourceMethods`) word differing purely from
address drift caused by `FileResource__LoadFile` (below) still being the wrong size
at that point. No change to this function was needed; fixing the drift
source elsewhere resolved it to 25/25.

## Head broadcast levers — applicability

- **goto-vs-return:** not applicable, no branches, single return path (void).
- **loop-invariant hoisting:** not applicable, no loop.
- **prologue store-order barrier:** not applicable, no residue of this class
  was observed.

## Proposed learning

See `FileResource__LoadFile.md` for the real finding from this round: `BMemPMgrAlloc`
(the allocator) takes **one** argument (`size`), not two. This function's own
"one word off, call-target only" symptom while a sibling function in the same
unit had a genuine size bug is a useful diagnostic pattern worth naming: a
lone call-target-encoding mismatch, with everything else in a function's
window matching, means look for a wrong-sized function *elsewhere in the same
translation unit*, not in the function funcdiff is currently pointing at.

## Naming

Round 52 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_80026A50` | `FileResource__FileResource` | A |

**Evidence.** The class's own constructor, `+0x008` slot by the project's
convention: chains `GetBasicClassMethods()->ctor`, installs
`GetFileResourceMethods()` as `this->methods`, then zeroes every field this
unit derived. A constructor's mechanics (chain base, install vtable,
initialise fields) ARE its purpose, so tier A by the plan's own rule.
`Class__Class` is the project's constructor-naming convention.

## Proposed field names

**APPLIED by the head at merge, round 52** -- all four fields, both types
and all five vtable slots below are now in the tree, each one applied
separately with `./build-and-verify.sh` green and byte-exact after it. One
mis-hit had to be resolved by receiver type: `src/cd/cd_driver.c:143`
accesses `pendingGeneration` on a `FileResource *self`, while the same file's
lines 97/174/175/182 are its OWN `ObjA34_179D8H::unk0C` and were left alone.
The compiler named that mis-hit (`structure has no member named 'unk0C'`),
which is the procedure working in the direction where it can work.

`FileResource` is shared with `code_179d8_h.c`/`cd_driver.c`
(see `FileResource__LoadFile.md`); every field this constructor zeroes is
therefore checked, and only `flags` (this round's own rename, zero
cross-unit hits) renamed outright. The rest:

| field | proposed name | tier | evidence |
| --- | --- | --- | --- |
| `unk22` | -- (no change proposed) | C | Zeroed here, never read or written anywhere else in this unit. No evidence beyond "exists, width 2 bytes". Left as `unk22` rather than guess. |
| `unk28` | -- (no change proposed) | C | Same as `unk22`: write-only in this unit, no read site found. |
| `unk2A` | -- (no change proposed) | C | Same as `unk22`/`unk28`. |

Not posting these three to the broadcast as renames (there is nothing to
apply); noting them here so the next reader does not re-derive "these are
zeroed and otherwise untouched" from scratch.

## Track 6 (round 91, charlie): the class is named FileResource

- `Class6D430` -> `FileResource` (`renametype.py`, the whole family: type,
  `Class6D430Methods`, `CLASS6D430_FIELDS`/`_SLOTS`, the `Class6D430__`
  methods, `GetClass6D430Methods`, the header `include/FileResource.h`), and
  `D_8006D430` -> `gFileResourceMethods` (`rename.py`). **Tier A.** Evidence:
  the body of `FileResource__LoadFile` alone (open a name, size it with
  `seek(0, 2)`, allocate, read the whole file, close), and every non-driver
  subclass agrees: each constructor names a file (`loadFile` or
  `requestLoadFile`), and each fills slot +0x078 with the step that consumes
  the loaded buffer (`TimImage__Upload`, `TileMap__BuildMap`,
  `TileAtlas__BuildCells`, `ModelData__BuildResources`,
  `TriggerWorld__BuildResources`, `Tod__ScanPackets`, `TodSet__ScanPackets`,
  `LinkResource__MapModel`, `TimArraySrc__UploadImages`,
  `TimBlockSrc__SetEntryShift`, `LbdFile__LoadHeader`,
  `VabStreamObj__OnBodyReady`, `PlacementGrid__ResolveEntry`). Callers use it
  as New_<Sub>(name), slot +0x078, `freeBuffer` (`TaskCore__SetSubHandle`,
  `TitleMenuTaskObjF.c`). The name was chosen over `DataSource` because in this
  code's existing vocabulary (`SetActiveDataSource`,
  `sDataSourceClientGetters`) the data SOURCE is the active driver and the
  asset classes are its clients.
- **History moved out of the header** (phase 2 comment rule): the open
  file's disc position and size (+0x018/+0x01C) were found to be base-class
  fields, not CdDriver's, in round 88, because the CD driver's methods run on
  every client object once `SetActiveDataSource` binds them; round 88 also
  found that `stopService`'s two occupants never read `self` but
  `CdDriver__RunRequestQueue` loads `$a0 = self` before its `jalr`, which is
  why the slot keeps a `Self *` parameter.
- **CdLoc16 is Sony's `CdlLOC`, substitution parked.** Layout (4 bytes at
  +0x018) and use agree: `CdControl(CdlSetloc, &self->pos, 0)`
  (CdDriver/s), `CdSearchFile`'s `CdlFILE.pos` copied into it
  (code_179d8_q/s), `CdPosToInt(&self->pos)` (CdDriver). The
  2-alignment of the s16 pair is NOT needed: measured through the pinned
  pipeline, a whole-struct copy of Sony's 1-aligned
  `struct { u_char minute, second, sector, track; }` compiles to the same
  `lwl`/`lwr` + `swl`/`swr` as `CdLoc16`, at a 4-aligned field and from a
  pointer. The blocker is the include: adding `<libcd.h>` to
  `include/FileResource.h` (measured with `MAKEFLAGS=-k`) makes exactly four
  units fail with `conflicting types`, each re-declaring Cd* functions its
  own way: CdDriver (CdSearchFile, CdControl, CdSync, CdRead,
  CdReadSync), CdDriver (CdControlB, CdSearchFile), code_179d8_r
  (CdControlF, CdRead, CdReadSync, CdSync), CdDriver (CdControl,
  CdIntToPos, CdPosToInt, CdRead, CdReadSync, CdSearchFile, CdSync). Those
  are those units' polish passes (`sonyheaders.py`). Once they take Sony's
  prototypes, `CdLoc16` is deleted, `FileResource.h` includes `<libcd.h>`,
  and `renametype.py --any-stem CdLoc16 CdlLOC` (or a hand edit of the four
  users: cd_driver.h, code_179d8_s/q/s) finishes it; no field accessor
  changes, since the halves are never read apart.

### Proposed field and slot names (not applied: accessors outside the job)

| member | proposed | tier | evidence | accessors |
| --- | --- | --- | --- | --- |
| slot `+0x078` `slot78` | `processBuffer` | B | every subclass occupant consumes the loaded buffer (list above); callers invoke it right after `New_<Sub>` and before `freeBuffer` | TitleMenuTaskObjF/i/j.c, DayTaskStageMap.c, task.c, PlacementGridVabSound.c |
| field `+0x02A` `unk2A` | `loadState` | B | the base ctor zeroes it; LbdFile steps it 0 -> 9 (header) -> 0 / 0 -> 10 (data block) -> 0 and VabStreamObj 1 (VH) -> 6 (VB); both advance it from slot +0x064 on a flags completion bit | game_files.c, PlacementGridVabSound.c |

The table in the section above lists `unk22`/`unk28`/`unk2A` as write-only:
that was true of this unit only. `unk22` and `unk28` have since been named
(`pendingRequests`, `inQueueDispatch`); `unk2A` is the row just above.

`FileResource__LoadFile`'s local `savedPendingGeneration` keeps `isOpen`
across the load; its name predates the field's and is GameApplicationFileResource's polish
work, not a type change.
