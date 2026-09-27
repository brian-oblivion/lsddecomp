# TaskCore__SetColors — MATCHED (20/20), round 19

> Renamed from `Obj86B60__SetColors` on 2026-09-25 (tools/rename.py). Address 0x8003cb68.

> Renamed from `func_8003CB68` on 2026-09-24 (tools/rename.py). Address 0x8003cb68.

**Unit:** code_2cc8c · **Size:** 20 instructions

## Round 19: closed with the untried "struct-typed argument" axis

The round-2 report (11 attempts across two sessions plus a ~13.5k-iteration
permuter run, all negative) explicitly named the one axis it never tried:
"a struct-typed RGB argument instead of three raw `s8*` pointers." That is
exactly what closes it. Whole-object assignment through a local 3-byte
struct type, one statement per group, reaches retail byte-for-byte:

```c
typedef struct { s8 r, g, b; } RGB8003CB68;

void TaskCore__SetColors(Obj86B60 *self, s8 *a1, s8 *a2, s8 *a3)
{
    *(RGB8003CB68 *)self->unk90 = *(RGB8003CB68 *)a1;
    *(RGB8003CB68 *)self->unk93 = *(RGB8003CB68 *)a2;
    *(RGB8003CB68 *)self->unk96 = *(RGB8003CB68 *)a3;
}
```

Verified via `cmp -l build/SLPS_015.56 disk/SLPS_015.56`: with this
function's C in place, every mismatched byte anywhere in the whole image
decodes into `func_8003FCFC`'s own still-open range
(`0x8003fcfc`-`0x8003fd4c`), a separate, already-known stall in a
different unit -- zero drift attributable to this function.

This closes BOTH residues the round-2 report identified as an unfixable
register-identity stall (group 1's third byte landing in `a1` instead of
a distinct register, and the trailing store scheduled outside the branch
delay slot): a single-statement whole-struct copy per group, rather than
three separate scalar byte assignments per group, gives GCC 2.6.3 no
opportunity to interleave the loads/stores of one group with the next,
which is exactly what was producing the register-reuse and scheduling
differences from the byte-by-byte shape.

### Proposed learning

A third confirmation, after `FadeBox__PushPosition` and `BoxFill__SetPosition`, that
**a scalar-field-by-scalar-field copy and a whole-aggregate copy of the
same bytes are not interchangeable to this compiler even when they are
semantically identical** -- and this instance sharpens the class further:
here the "aggregate" is a 3-byte, odd-sized, non-power-of-two struct with
no natural machine word/halfword shape, not a pair of aligned `s32`s. The
whole-struct-assignment lever is worth trying even when the fields don't
obviously bundle into a hardware-sized load/store -- GCC 2.6.3's aggregate
copy lowering picks its own byte/half/word decomposition and scheduling
independently of what the per-field C would have produced, and that
decomposition is what matched retail here, byte grouping included.

## Provenance

Round 2026-09-02, runner echo: original 11 attempts + permuter (negative,
restored to `INCLUDE_ASM`, see git history for the full text preserved
before this rewrite). Round 19, runner alpha: struct-copy axis, matched
first try.

## Boundary of the lever (coordinator's request, round 19 second pass)

Before this generalizes further than warranted: **all five of this
round's whole-struct-assignment closures are within units descended from
the same original `code_2cc8c` monolith** (`code_2cc8c`, `code_2cc8c_b`,
`code_2cc8c_c`, `code_2cc8c_e`, `code_2cc8c_f` -- all carved from one
segment across earlier rounds). I have not tried this lever, or seen it
tried, anywhere outside that family (`Entity.c`, `DreamSys.c`,
`class_3bb8c*.c`, etc.), so I cannot personally attest it holds there.

Within that scope, though, the evidence is broader than "one struct's
idiom" and worth stating precisely:

- **Two genuinely independent struct SHAPES.** A 2x`s32`, 4-byte-aligned
  pair (`BoxFillPos`: `FadeBox__PushPosition`, `BoxFill__SetPosition`, `TextRow__SetPosition`,
  `TextRow__AttachToParent`) and a 3x`s8`, 1-byte-aligned, non-power-of-two triple
  (this function's own RGB-shaped struct, and `BoxFill__ApplyColor`'s copy arm
  in the SAME unit). These have nothing in common at the ABI level
  (alignment, size, natural load/store width) -- if the lever only worked
  because of some accident specific to a 2-word-aligned pair, the 3-byte
  case should not have closed. It did, on the first attempt, with no
  adjustment.
- **Two genuinely independent class/vtable families.** `BoxFillPos` copies
  live in `Obj6EAC0`/`FadeBoxObj` (units `code_2cc8c_e`/`_f`); THIS
  function's 3-byte struct lives in `Obj86B60` (unit `code_2cc8c`,
  matched by a DIFFERENT runner in an EARLIER round before this round's
  fix) while `BoxFill__ApplyColor`'s matching 3-byte case lives in `Obj6EAC0`
  (unit `code_2cc8c_f`) -- so the SAME struct shape closed in two
  unrelated classes, and two DIFFERENT struct shapes closed within the
  same class family. The lever's success does not track any one
  struct's identity or any one class's field layout.

**Best-supported claim: the underlying MECHANISM (GCC 2.6.3 lowers an
aggregate assignment via a different RTL path than N separate scalar
assignments, with knock-on effects on surrounding register allocation)
is very likely a general compiler property, not a one-struct idiom** --
but the supporting evidence is still entirely within one segment's
descendant units. Treat it as: safe to try broadly as a lever (cheap,
mechanistic, not tied to a specific field layout), NOT yet safe to
promote to "closes any two-or-more-statement scalar copy in this
codebase" without a confirmation outside the `code_2cc8c` family. Round
18's over-promotion of the type/declaration-order lever on one success is
the cautionary precedent this is deliberately not repeating.

## Naming (round 78, delta)

**Tier A** (pure setter). `func_8003CB68` -> `Obj86B60__SetColors`. Body:
copies three independent 3-byte (`s8 r,g,b`) triples from `a1`, `a2`, `a3`
into `self->unk90`, `self->unk93`, `self->unk96` respectively -- no other
logic, a pure multi-field assignment.

## Proposed field names (round 78, delta -- NOT applied, cross-unit)

`Obj86B60::unk90` (`u8[3]`, +0x090) -> `baseColor`. Tier B: the only field
of the three `SetColors` writes that is independently READ elsewhere in this
unit -- `TaskCore__TickColorFade` adds `frameCounter * unk84` to each of its
three bytes as a fade base. `unk93`/`unk96` are set the same way by
`SetColors` but never independently read in this unit's evidence, so no
distinguishing name is proposed for them (kept `unk93`/`unk96`). Grep shows
`unk90` textual hits in several genuinely-shared code_2cc8c_* siblings plus
unrelated units, so proposal only.


**Head disposition, round 78.** `unk90` -> `baseColor` APPLIED (type scope, 4 accessors).

## Track 4 (2026-09-25, round 84, alpha)

Renamed from Obj86B60__SetColors (tools/rename.py): the class prefix. Occupant of its gTaskCoreMethods slot, named for it in TASKCORE_SLOTS (`classtable.py gTaskCoreMethods`). The class (id 0x130, table gTaskCoreMethods) is unified as `TaskCore` in `include/TaskCore.h`; `self` is `TaskCore *` (it was the `Obj86B60` or `StreamTaskObj` view). Any source block above is the pre-unification spelling; the live body takes the unified types and slot names, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 6 (2026-09-27, round 98, delta)

The local copy type `RGB8003CB68` (named for this function's address, not a
global) is retired for `BgLayerRgb` (`include/BgLayer.h`, already included by
the unit). Evidence: the copy compiles to `lb`/`sb` per byte, so the record is
a signed 3-byte `s8` triple -- not Sony's `CVECTOR` (4 bytes, `u_char`); and
`baseColor`, the first of the three, is what `TaskCore__OnInit`
(`code_2c054.c`) hands to `BgLayer`'s `setColor` as a `BgLayerRgb *`, and
`TaskCore__TickColorFade` hands it the faded copy of it the same way. `unk93`/`unk96` are filled from the same rodata table
(`D_8006E860`, three records 3 bytes apart) and `unk93` goes to the same
TaskTextObj slot78 as `baseColor`, so one record type covers all three.
Byte-identical: whole image green, 0 new warnings, nonmatching green.

Proposed, not applied (header edit, other units): `TaskCore.h`'s `baseColor`,
`unk93`, `unk96` retyped from `u8[3]` to `BgLayerRgb`, which would drop the
casts here and in `code_2c054.c`. `code_2cc8c_f.c`'s `RGB80040790`
(`BoxFill__ApplyColor`) is the same shape (s8 r, g, b, struct-copied); a
separate job.
