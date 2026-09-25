# GetClass6D4E8Methods

> Renamed from `func_80027E68` on 2026-09-17 (tools/rename.py). Address 0x80027e68.

**Unit:** code_179d8_q (fresh carve) · **Size:** 4 instructions · **Status:** MATCHED (4/4 words)

## Class identity

This function is the "get my own method table" accessor for the class whose
vtable is `D_8006D4E8` (29 slots, header word `0x13`, resolved with
`tools/classtable.py D_8006D4E8`). Slot map:

- `+0x004` `DestroyChained` (own-class slot, shared with `D_8006D430` at the
  identical offset)
- `+0x008` `Class6D4E8__Class6D4E8` (ctor, by the project's `+0x008` convention)
- `+0x00C` `func_80027274` (dtor)
- `+0x010`..`+0x038` the 13 inherited `BasicClass__func_*` slots, verbatim
- `+0x040`..`+0x074` own slots, including `Class6D4E8__RequestLoadFile`/`Class6D4E8__StopCdService`/
  `Class6D4E8__CancelRequests` (this unit's next three queued functions, at `+0x06C`/
  `+0x070`/`+0x074`)

Compared against `D_8006D430` (`include/code_171e0.h`'s
`Class6D430Methods`) with `classtable.py D_8006D4E8 --vs D_8006D430`:
`DestroyChained` at `+0x004` and `Class6D430__FreeBuffer`/`NoOp`/
`Class6D430__SetFlag` at identical offsets (`+0x05C`/`+0x060`/`+0x064`) are shared
between the two tables, strongly suggesting `D_8006D4E8`'s class is a
subclass or close sibling of `D_8006D430`'s, inheriting the same BasicClass
slot block and several of the same concrete method implementations.

`GetClass6D4E8Methods` itself is the same "return my own vtable's address"
accessor the project already names elsewhere: `GetClass6D3C8Methods` for
`D_8006D3C8` and `GetClass6D430Methods` for `D_8006D430` (both in
`include/code_171e0.h`'s doc comment).

## The C

```c
/* D_8006D4E8's own method table -- see class-identity note above. */
extern s32 D_8006D4E8[];

s32 *GetClass6D4E8Methods(void)
{
    return D_8006D4E8;
}
```

## Why `lui`/`addiu`, not `%gp_rel`

`D_8006D4E8` lives in `.data` (confirmed in `asm/data/5DB70.data.s`), not
`.sdata`, so retail takes its address with an absolute `lui $v0,
%hi(D_8006D4E8)` / `addiu $v0, $v0, %lo(D_8006D4E8)` pair rather than a
`$gp`-relative load. Declaring it `extern s32 D_8006D4E8[];` and returning
the array (which decays to its address) reproduces that exactly.

## Provenance

round 45 (2026-09-15), runner echo, unit code_179d8_q (fresh carve).

## Naming

Round 51 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_80027E68` | `GetClass6D4E8Methods` | A |

**Evidence.** A two-instruction address-of: it returns `&D_8006D4E8`, this
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
already carries as `Class6B5CC__RotateLocalVector`. **Proposed for track 4,
when the class's views are unified:** `CdReader` or `CdFile`, on the evidence
above. That is a proposal, not a name.

### Proposed learning

**A class whose behaviour is fully established can still be the wrong thing
to name, because a class name is not one name -- it is the prefix of every
method in every unit that touches the class.** The cost of being wrong scales
with the class's method count, while the cost of a placeholder token is one
`rename.py` run per method later. Where the plan's "a wrong tier-A name is
worse than a placeholder" bites hardest is exactly here, and the existing
`Class6B5CC__` spelling shows the project already settled on the hedge:
placeholder CLASS, evidence-based METHOD.
