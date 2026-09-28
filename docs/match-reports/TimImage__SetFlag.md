# TimImage__SetFlag -- MATCHED (3/3 words), round 81

> Renamed from `TimImage__SetFlag48` on 2026-09-28 (tools/rename.py). Address 0x8003b5e4.

> Renamed from `TimImage__func_8003B5E4` on 2026-09-27 (tools/rename.py). Address 0x8003b5e4.

> Renamed from `func_8003B5E4` on 2026-09-25 (tools/rename.py). Address 0x8003b5e4.

Round 81, runner echo. Unit `src/graphics/tim_image.c` (carved from `psyq_2bb9c` in
FINISHING-PLAN revision 18). This was fresh ground with no prior attempt.

- **Where:** TimImage's table (`gTimImageMethods`) slot +0x098 (resolved with `tools/classtable.py D_8006E558`).
- **What:** sets the subclass field +0x048 to 1 (`ori $v0,$zero,1; jr $ra; sw $v0,0x48($a0)`). TimImage__TimImage clears the same field. No other function in the unit reads it, and no caller outside the unit reaches this slot directly, so its purpose is not established; it keeps `unk48`.
- **Result:** byte-exact on the first build. `funcdiff.py` reports 3/3,
  and the whole-image SHA1 is green (`OK: build matches retail`).
- **Name:** `TimImage__SetFlag`, tier C (round 81 naming pass, runner
  bravo): the class is confirmed as `TimImage` (see `## Naming` below), but
  the field it sets (`unk48`) is written here and nowhere read within the
  unit, so what the flag MEANS is unknown -- tier-C `Class__func_xxxxx`.

## Source

```c
void TimImage__SetFlag(TimImage *self) {
    self->unk48 = 1;
}
```

The unit-local view it needs, from the top of `src/graphics/tim_image.c`:

```c
#include "file_resource.h"

typedef struct TimImage {
    FILERESOURCE_FIELDS(FileResourceMethods);
    /* +0x02C */ u8 pad2C[0x1C];
    /* +0x048 */ s32 unk48;
} TimImage;

typedef struct GsIMAGE GsIMAGE;
void GsGetTimInfo(u32 *im, GsIMAGE *tim);
extern FileResourceMethods gTimImageMethods;
```

## Naming

- **`TimImage__SetFlag`**, tier C: class confirmed as `TimImage` (this
  round), slot +0x098. The body sets `unk48 = 1`, and `TimImage__TimImage`
  clears the same field, but nothing in this unit reads `unk48`, and no
  cross-unit caller reaches this slot directly (`TimImage` is always seen as
  an opaque handle typed per call site elsewhere in the project), so what
  the flag gates is unestablished -- `Class__func_xxxxx`.

## Naming (track 7, round 100, delta)

Renamed `TimImage__func_8003B5E4` -> `TimImage__SetFlag` with `tools/rename.py`
(the tier-C line above predates it). **Tier A**: a pure leaf setter,
`self->flag48 = 1` (`ori $v0,$zero,1; jr $ra; sw $v0,0x48($a0)`), named as
`FrameClock__Stop` is for the same shape. The field +0x048 is renamed
`unk48` -> `flag48` and the table slot +0x098 `slot98` -> `setFlag48` in
`include/tim_image.h`; the compiler's accessor set for both was this unit only
(the ctor, which clears it, and this setter). No reader of TimImage +0x048 is
found in `src/` (the eleven tim_image.h includers and graphics_resources.c's
TimArraySrc, which builds TimImages), and no C calls +0x098, so what the flag
gates is not established: the field name says only that it is a flag.

## Naming (track 10 debt pass, round 104, bravo)

`TimImage__SetFlag48` -> `TimImage__SetFlag` (`tools/rename.py`), and by hand
in `include/tim_image.h` the field `flag48` -> `flag` and the slot
`setFlag48` -> `setFlag`: the offset in the name said nothing the offset
comment does not. The accessor set is unchanged, this unit only (the ctor
clears it, this setter sets it), and still nothing reads it. The body is now
`self->flag = 1;`.
