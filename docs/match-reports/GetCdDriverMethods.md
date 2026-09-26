# GetCdDriverMethods

> Renamed from `GetClass6D4E8Methods` on 2026-09-26 (tools/rename.py). Address 0x80027e68.

> Renamed from `func_80027E68` on 2026-09-17 (tools/rename.py). Address 0x80027e68.

**Unit:** code_179d8_q (fresh carve) · **Size:** 4 instructions · **Status:** MATCHED (4/4 words)

## Class identity

This function is the "get my own method table" accessor for the class whose
vtable is `gCdDriverMethods` (29 slots, header word `0x13`, resolved with
`tools/classtable.py gCdDriverMethods`). Slot map:

- `+0x004` `Class6D430__Release` (own-class slot, shared with `D_8006D430` at the
  identical offset)
- `+0x008` `CdDriver__CdDriver` (ctor, by the project's `+0x008` convention)
- `+0x00C` `CdDriver__Finalize` (dtor)
- `+0x010`..`+0x038` the 13 inherited `BasicClass__func_*` slots, verbatim
- `+0x040`..`+0x074` own slots, including `CdDriver__RequestLoadFile`/`CdDriver__StopService`/
  `CdDriver__CancelRequests` (this unit's next three queued functions, at `+0x06C`/
  `+0x070`/`+0x074`)

Compared against `D_8006D430` (`include/code_171e0.h`'s
`Class6D430Methods`) with `classtable.py gCdDriverMethods --vs D_8006D430`:
`Class6D430__Release` at `+0x004` and `Class6D430__FreeBuffer`/`NoOp`/
`Class6D430__SetFlag` at identical offsets (`+0x05C`/`+0x060`/`+0x064`) are shared
between the two tables, strongly suggesting `gCdDriverMethods`'s class is a
subclass or close sibling of `D_8006D430`'s, inheriting the same BasicClass
slot block and several of the same concrete method implementations.

`GetCdDriverMethods` itself is the same "return my own vtable's address"
accessor the project already names elsewhere: `GetClass6D3C8Methods` for
`D_8006D3C8` and `GetClass6D430Methods` for `D_8006D430` (both in
`include/code_171e0.h`'s doc comment).

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

round 45 (2026-09-15), runner echo, unit code_179d8_q (fresh carve).

## Naming

Round 51 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_80027E68` | `GetCdDriverMethods` | A |

**Evidence.** A two-instruction address-of: it returns `&gCdDriverMethods`, this
class's own 29-slot method table. A pure leaf whose mechanics are its
purpose, so tier A by the plan's own rule. The same accessor shape
`GetClass6D3C8Methods` has for `D_8006D3C8` and `GetClass6D430Methods` for `D_8006D430`.

**The class token `Class6D4E8` is deliberate, and this is the report that
says why.** What the class IS, is now well evidenced: every method reachable
from this table bottoms out in Psy-Q libcd (`CdSearchFile`, `CdRead`,
`CdControlF`, `CdSync`, `CdFlush`, `CdPosToInt`/`CdIntToPos`), its objects
cache a disc position and a byte size, and `code_171e0.c` selects this
class's module functions only when `gActiveDataSource == 0x13`, this table's own
header word -- the other value that gate takes, `0x23`, is `gVabDriverMethods`, the
SPU/VAB streamer in `code_179d8_e.c`. So the two are interchangeable data
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


Track 4, 2026-09-26 (round 88). The class of gCdDriverMethods (was D_8006D4E8, id 0x13 = DATASOURCE_CD) is CdDriver, in include/CdDriver.h: its ctor calls InitCdDrive, its slots enqueue CD_OP_* requests and drive the CD read state machine, and it is VabDriver's sibling. The object views this function was typed against are replaced by CdDriver, whose fields are all Class6D430's (the driver runs on its clients' objects; Class6D430's +0x018/+0x01C were named pos/size for it). Byte-identical. `GetClass6D4E8Methods` -> `GetCdDriverMethods` by rename.py. Now returns `CdDriverMethods *` (`&gCdDriverMethods`), not `s32 *`.
