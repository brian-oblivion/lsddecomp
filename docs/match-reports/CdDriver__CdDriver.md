# CdDriver__CdDriver -- MATCHED (19/19 words)

> Renamed from `Class6D4E8__Class6D4E8` on 2026-09-26 (tools/rename.py). Address 0x80027228.

> Renamed from `func_80027228` on 2026-09-25 (tools/rename.py). Address 0x80027228.

Unit `CdDriver`, round 26 (2026-09-09). This class's constructor --
confirmed via `tools/classtable.py 0x8006D4E8 --vs 0x8006B58C` as the
override of the base "BasicClass" hierarchy's constructor slot (`+0x008`);
see `New_CdDriver.md` for the full table-comparison finding shared by
this whole unit.

## What it is

```c
void CdDriver__CdDriver(Obj6D4E8 *self)
{
    GetFileResourceMethods(self)->ctor(self);
    self->methods = GetCdDriverMethods();
    self->unk28 = 0;
    InitCdDrive();
}
```

Standard "further-base constructor first" idiom (same shape as
`DayTaskStageMap.c`'s own local `BaseCtorTable_3ac78`, independently
established for a different class hierarchy in that unit): `GetFileResourceMethods`
(still `INCLUDE_ASM` in the `code_179d8` remainder) returns a further-base
class's own ctor-dispatch table, called through its `+0x008` slot with
`self`. The rest is straight-line: set `self->methods` (offset 0, the
project's usual table-pointer-at-object-offset-0 convention) to this
class's own table, clear the halfword field at `+0x028`, and call
`InitCdDrive()` (also still `INCLUDE_ASM`, zero-argument -- confirmed by
the retail `jal`'s having no register load anywhere above it that a0 could
be attributed to).

Matched on the first attempt -- a direct, source-order translation of the
retail instruction sequence (no scheduling residue to fight).

## Naming

Round 79 (delta).

- **`CdDriver__CdDriver`** (was `func_80027228`) -- **tier A**. It is
  table slot +0x008 of gCdDriverMethods (`tools/classtable.py 0x8006D4E8 --vs
  0x8006D430`), the slot that holds `FileResource__FileResource` in the parent
  table and `BasicClass__BasicClass` in the root table. The body is the
  project's constructor shape exactly: parent ctor first
  (`GetFileResourceMethods()->ctor(self)`), then install its own table
  (`self->methods = GetCdDriverMethods()`), then its own fields
  (`unk28 = 0`), then `InitCdDrive()` (CdDriver: one-shot
  `CdSetDebug(0)` + set double-speed mode). `New_CdDriver` dispatches it.
- The two externs' notes that `GetCdDriverMethods` and `InitCdDrive` are
  "still INCLUDE_ASM" were stale (both are matched C in cd_driver.c) and
  are corrected in the unit.
- `unk28` (+0x28, s16) is kept: cleared here and by FileResource's own ctor,
  read nowhere in this unit, so nothing establishes a meaning.
- The parent-table view `BaseCtorTable6D4E8` is renamed `FileResourceCtorView`
  (it IS gFileResourceMethods seen down to +0x008).


Track 4, 2026-09-26 (round 88). The class of gCdDriverMethods (was D_8006D4E8, id 0x13 = DATASOURCE_CD) is CdDriver, in include/cd_driver.h: its ctor calls InitCdDrive, its slots enqueue CD_OP_* requests and drive the CD read state machine, and it is NullDriver's sibling. The object views this function was typed against are replaced by CdDriver, whose fields are all FileResource's (the driver runs on its clients' objects; FileResource's +0x018/+0x01C were named pos/size for it). Byte-identical. `Class6D4E8__Class6D4E8` -> `CdDriver__CdDriver` by rename.py.

Track 7, round 101 (charlie). The `unk28` bullet above is out of date: the
field at +0x028 is FileResource's `inQueueDispatch` (u16), which
CdDriver__RunRequestQueue sets around its dispatch back into a request method
(cd_driver.c), and the ctor's store is `self->inQueueDispatch = 0`. No
source change for this function in round 101 beyond the unit banner.

## History (moved from include/CdDriver.h, round 102)

Comment text moved verbatim out of the header, which now says only
what the code is.

```c
 * queue, the file table and the module state its units share (track 4b,
 * round 85).
```

```c
 * freeGuard, pendingRequests, flags, inQueueDispatch -- are all FileResource's
 * (pos and size, +0x018/+0x01C, were its pad18 until round 88), and the
```

```c
/* The module state. Every global here was declared in two or more of the
 * units above, with up to three different types; the types below are the
 * ones their accessors need. A global only one unit touches stays a local
 * extern in that unit (track 4b, round 85). */
```

```c
 * --vs gFileResourceMethods`); the slot names are FileResource's, which were named for
 * these occupants. Slot types are FileResource's too. Where the old local
 * views disagreed (open/close/loadFile/setFlag/stopService returning s32,
 * read's buf as s32), the occupants return void / take void * and the
 * bytes did not move when the views were replaced.
```
