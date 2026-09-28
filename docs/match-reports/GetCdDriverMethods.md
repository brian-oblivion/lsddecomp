# GetCdDriverMethods

> Renamed from `GetClass6D4E8Methods` on 2026-09-26 (tools/rename.py). Address 0x80027e68.

> Renamed from `func_80027E68` on 2026-09-17 (tools/rename.py). Address 0x80027e68.

**Unit:** CdDriver (fresh carve) · **Size:** 4 instructions · **Status:** MATCHED (4/4 words)

## Class identity

This function is the "get my own method table" accessor for the class whose
vtable is `gCdDriverMethods` (29 slots, header word `0x13`, resolved with
`tools/classtable.py gCdDriverMethods`). Slot map:

- `+0x004` `FileResource__Release` (own-class slot, shared with `gFileResourceMethods` at the
  identical offset)
- `+0x008` `CdDriver__CdDriver` (ctor, by the project's `+0x008` convention)
- `+0x00C` `CdDriver__Finalize` (dtor)
- `+0x010`..`+0x038` the 13 inherited `BasicClass__func_*` slots, verbatim
- `+0x040`..`+0x074` own slots, including `CdDriver__RequestLoadFile`/`CdDriver__StopService`/
  `CdDriver__CancelRequests` (this unit's next three queued functions, at `+0x06C`/
  `+0x070`/`+0x074`)

Compared against `gFileResourceMethods` (`include/data_source.h`'s
`FileResourceMethods`) with `classtable.py gCdDriverMethods --vs gFileResourceMethods`:
`FileResource__Release` at `+0x004` and `FileResource__FreeBuffer`/`NoOp`/
`FileResource__OnRequestDone` at identical offsets (`+0x05C`/`+0x060`/`+0x064`) are shared
between the two tables, strongly suggesting `gCdDriverMethods`'s class is a
subclass or close sibling of `gFileResourceMethods`'s, inheriting the same BasicClass
slot block and several of the same concrete method implementations.

`GetCdDriverMethods` itself is the same "return my own vtable's address"
accessor the project already names elsewhere: `GetGameApplicationMethods` for
`gGameApplicationMethods` and `GetFileResourceMethods` for `gFileResourceMethods` (both in
`include/data_source.h`'s doc comment).

## The C

```c
/* gCdDriverMethods's own method table -- see class-identity note above. */
extern s32 gCdDriverMethods[];

s32 *GetCdDriverMethods(void)
{
    return gCdDriverMethods;
}
```

## Why `lui`/`addiu`, not `%gp_rel`

`gCdDriverMethods` lives in `.data` (confirmed in `asm/data/5DB70.data.s`), not
`.sdata`, so retail takes its address with an absolute `lui $v0,
%hi(gCdDriverMethods)` / `addiu $v0, $v0, %lo(gCdDriverMethods)` pair rather than a
`$gp`-relative load. Declaring it `extern s32 gCdDriverMethods[];` and returning
the array (which decays to its address) reproduces that exactly.

## Provenance

round 45 (2026-09-15), runner echo, unit CdDriver (fresh carve).

## Naming

Round 51 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_80027E68` | `GetCdDriverMethods` | A |

**Evidence.** A two-instruction address-of: it returns `&gCdDriverMethods`, this
class's own 29-slot method table. A pure leaf whose mechanics are its
purpose, so tier A by the plan's own rule. The same accessor shape
`GetGameApplicationMethods` has for `gGameApplicationMethods` and `GetFileResourceMethods` for `gFileResourceMethods`.

**The class token `Class6D4E8` is deliberate, and this is the report that
says why.** What the class IS, is now well evidenced: every method reachable
from this table bottoms out in Psy-Q libcd (`CdSearchFile`, `CdRead`,
`CdControlF`, `CdSync`, `CdFlush`, `CdPosToInt`/`CdIntToPos`), its objects
cache a disc position and a byte size, and `GameApplicationFileResource.c` selects this
class's module functions only when `sActiveDataSource == 0x13`, this table's own
header word -- the other value that gate takes, `0x23`, is `gNullDriverMethods`, the
SPU/VAB streamer in `PlacementGridVabSound.c`. So the two are interchangeable data
sources behind one dispatch layer, and this one is the CD-ROM source.

What is NOT established is what the developers CALLED it. Naming it
`CdStream` or `CdFile` would be a tier-A assertion drawn from behaviour
alone, and it would propagate into every method name in three units, so this
round used the address token instead -- the convention the symbols file
already carries as `SceneNode__RotateLocalVector`. **Proposed for track 4,
when the class's views are unified:** `CdReader` or `CdFile`, on the evidence
above. That is a proposal, not a name.

### Proposed learning

**A class whose behaviour is fully established can still be the wrong thing
to name, because a class name is not one name -- it is the prefix of every
method in every unit that touches the class.** The cost of being wrong scales
with the class's method count, while the cost of a placeholder token is one
`rename.py` run per method later. Where the plan's "a wrong tier-A name is
worse than a placeholder" bites hardest is exactly here, and the existing
`SceneNode__` spelling shows the project already settled on the hedge:
placeholder CLASS, evidence-based METHOD.


Track 4, 2026-09-26 (round 88). The class of gCdDriverMethods (was D_8006D4E8, id 0x13 = DATASOURCE_CD) is CdDriver, in include/cd_driver.h: its ctor calls InitCdDrive, its slots enqueue CD_OP_* requests and drive the CD read state machine, and it is NullDriver's sibling. The object views this function was typed against are replaced by CdDriver, whose fields are all FileResource's (the driver runs on its clients' objects; FileResource's +0x018/+0x01C were named pos/size for it). Byte-identical. `GetClass6D4E8Methods` -> `GetCdDriverMethods` by rename.py. Now returns `CdDriverMethods *` (`&gCdDriverMethods`), not `s32 *`.

## Round 96 (track 6, echo): unit comment moved here

Moved from code_179d8_s.c, as history: CdDriver, its table and its methods are
include/cd_driver.h's since track 4, round 88; the per-call-site views
CdDriver declared (Obj6D4E8_C80, Obj6D4E8_D70, Obj6D4E8_282AC, and the
table views Methods6D4E8_C80 / Methods6D4E8_80EC) were that one class. Round
96 removed the unit's last two local views: CdRequest_282AC (the writing-side
view of CdRequestNode) and UnkC80 (CdDriver__RequestLoadFile's report).

## Track 7 (round 101, echo): comments moved here, and names

The unit carried this comment above the getter; it is derivation (the
"Why `lui`/`addiu`" section above), so it moved here verbatim:

```c
/* The class's own table getter (include/cd_driver.h) -- an address-of, not
 * gp_rel: gCdDriverMethods lives in .data, not .sdata. */
```
