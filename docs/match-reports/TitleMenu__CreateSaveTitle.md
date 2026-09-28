# TitleMenu__CreateSaveTitle -- MATCHED 60/60, round 43

> Renamed from `TitleMenu__CreateNameField` on 2026-09-26 (tools/rename.py). Address 0x8004db18.

> Renamed from `Class86B60__CreateNameField` on 2026-09-26 (tools/rename.py). Address 0x8004db18.

> Renamed from `func_8004DB18` on 2026-09-24 (tools/rename.py). Address 0x8004db18.

Unit `TitleMenuTaskObjF`, class `TitleMenu`. **REOPENED -- ASSIGNABLE** from
round 42's `gp_rel` resolution. The round-14 stub recorded 5 `gp_rel` hits
and no derivation; this round wrote and matched the function from scratch.

## Body

```c
typedef struct Arg1DB18_3bb8c_d Arg1DB18_3bb8c_d;
struct Arg1DB18_3bb8c_d {
    u8 pad0[0x004];
    void *unk4; /* +0x004 */
};

extern char *strcpy(char *dest, char *src);
extern s32 strlen(char *s);
extern void DecodeFullWidthSjis(void *dst, void *src);

void TitleMenu__CreateSaveTitle(TitleMenu *self, Arg1DB18_3bb8c_d *arg1)
{
    u32 size;
    char *buf;

    if (arg1 == NULL) {
        return;
    }
    if (self->unkA4->methods->slot1AC(self->unkA4)) {
        strcpy((char *)gSaveTitle + 0x18, (char *)sSaveTitleBlanks);
        StampSaveTitleFileLetter((s32)gSaveTitle, 0);
    }
    size = strlen((char *)gSaveTitle);
    size = (size >> 1) + 4;
    buf = BMemPMgrAlloc(size);
    DecodeFullWidthSjis(buf, gSaveTitle);
    self->nameField = (TitleMenuUnkB0Obj_3bb8c_d *)New_TextRow(arg1->unk4, size, buf);
    self->nameField->unkAB = 8;
    self->nameField->unkAC = 4;
    self->nameField->unkAA = 9;
    BMemPMgrFree(buf);
}
```

Byte-exact on the first build: `funcdiff.py TitleMenu__CreateSaveTitle` -> `60/60 words
match (file 0x3E318-0x3E408)`; whole-image `OK: build matches retail
SLPS_015.56`.

## Derivation

- `if (arg1 == NULL) return;` -- the whole body is skipped on a single
  `beqz $s3` gate at function entry.
- `self->unkA4->methods->slot1AC(self->unkA4)` -- the SAME slot this
  round's `TitleMenu__SaveToCard` established, reused verbatim.
- Inside that gate: `strcpy((char *)gSaveTitle + 0x18, (char *)sSaveTitleBlanks)`
  then `StampSaveTitleFileLetter(gSaveTitle, 0)` -- `StampSaveTitleFileLetter` is the
  already-canonical `extern s32 StampSaveTitleFileLetter(s32 arg0, s32 arg1);`
  (`include/class_3bb8c.h`); `gSaveTitle` (`void *`) converts to its `s32`
  parameter with an explicit cast (a value pass, not dereferenced, so the
  representation is unchanged).
- `size = strlen(gSaveTitle); size = (size >> 1) + 4;` -- the `srl` in the
  disassembly (not `sra`) is what fixes `size` as `u32`: an `s32` `/2` or
  `>>1` on a signed type would need sign-correction retail does not emit
  for this half-length-plus-pad allocation idiom.
- `buf = BMemPMgrAlloc(size);` (pool allocator, already canonical) then
  `DecodeFullWidthSjis(buf, gSaveTitle)` -- `DecodeFullWidthSjis` is already matched
  (`src/ScreenWidgets.c`, `u8 *DecodeFullWidthSjis(u8 *dst, u8 *src)`), return
  value unused here, so this unit's own local view stays `void`-returning
  per the project's independent-arities convention (same idiom already
  used for `Get_vtable_TaskCore`/`BaseTaskCtorTable_3bb8c_c`).
- `self->nameField = (TitleMenuUnkB0Obj_3bb8c_d *) New_TextRow(arg1->unk4,
  size, buf);` -- `New_TextRow` is ALREADY declared, unconditionally, in
  this very header (`include/class_3bb8c.h`, a different unit's section)
  as `extern FieldM7C *New_TextRow(void *ctx, s32 len, char *name);` --
  since that declaration is already visible via the `#include` this unit
  shares, an explicit cast from `FieldM7C *` to this unit's own
  `TitleMenuUnkB0Obj_3bb8c_d *` is required at the assignment (both are
  this unit's or a sibling unit's independent views of the same real
  external return value; not a struct edit, just a local reinterpretation
  at one call site).
- `self->nameField->unkAA/unkAB/unkAC` -- three new opaque `u8` fields at
  +0x0AA/+0x0AB/+0x0AC on `TitleMenuUnkB0Obj_3bb8c_d`, set to the literal
  values `9`/`8`/`4` right after construction. The store/reload/store
  pattern in the disassembly (three separate `lw self->nameField`, not one
  cached pointer) is exactly what three independent field-assignment
  statements through `self->nameField->...` compile to.
- `BMemPMgrFree(buf);` -- frees the temporary name buffer after
  `New_TextRow` has consumed it (already-canonical pool-free).

## Header changes

`include/class_3bb8c.h`, all additive:

- `TitleMenuUnkB0Obj_3bb8c_d`: added `unkAA`/`unkAB`/`unkAC` (three `u8`
  fields, offsets +0x0AA/+0x0AB/+0x0AC) via a `pad004[0x0AA-0x004]` gap.
  Noted in the new field's comment: `TitleMenuUnkB0ObjMethods_3bb8c_d`'s
  own slot offsets (`release`+0x004, `slot4C`+0x04C) numerically match
  `Task.h`'s `Unk64ElemMethods`/this header's own `FieldM7CMethods`
  (slot4/slot4C/slotB8 at 0x004/0x04C/0x0B8) -- suggestive that `nameField` is
  the SAME real class those units call `Unk64Elem`/`FieldM7C`, consistent
  with `New_TextRow`'s return value landing there.
- New extern `sSaveTitleBlanks` (`void *`, VALUE-of `%gp_rel`, same pattern as
  `sSaveFileName`/`gSaveTitle`).
- Expanded comment on `gSaveTitle`/`sSaveFileName`: this function proves
  `gSaveTitle`'s RUNTIME value must be a writable buffer (it is both a
  `strcpy` destination and a `strlen` argument here), which cannot be the
  ROM image's own `.rodata` initial value -- nothing in this unit
  reassigns it, so whatever sets the real value is outside this unit's
  ground. Flagged as a placeholder-value caveat, not resolved further.

`src/ui/TitleMenuTaskObjF.c`: local (not shared-header) type `Arg1DB18_3bb8c_d`
and externs for `strcpy`/`strlen` (Sony's, linked from `lib/libc2`, same
per-unit convention as `src/ui/TextEntryItemList.c`/`src/class_3bb8c_j.c`) and
`DecodeFullWidthSjis` (already matched elsewhere; independent local arity).

No EXISTING declaration was retyped or resized.

### Proposed learning

A `%gp_rel`-loaded global whose static ROM-image value points into
`.rodata` is not necessarily read-only in practice: `TitleMenu__CreateSaveTitle` writes
through `gSaveTitle` (a `strcpy` destination) despite its ROM-image value
sitting inside an unowned rodata string table. When a function treats a
gp-relative global's value as a destination/buffer rather than a source,
check for this before assuming the static initializer is the value that
matters at runtime -- some other, likely-uncarved code reassigns it first.

## Naming (round 77, naming runner delta)

Renamed `func_8004DB18` -> `TitleMenu__CreateSaveTitle`. **Tier B**: SJIS-decodes `gSaveTitle` (a writable name-text buffer, per its own header comment) into a pool buffer and constructs `self->nameField` through `New_TextRow`, tagging it with three literal flag bytes. Named for what it builds (a text-field sub-object); the field's role in the larger UI is not established.

## Track 4 (2026-09-26, round 88, bravo)

TitleMenu is unified in include/TitleMenu.h (TASKCORE_SLOTS/TASKCORE_FIELDS plus its own). The setTarget override (+0x0D8). Its argument is the TaskCoreTarget the ctor passes (&sTitleMenuTarget); the one field read, +0x004, is `handle`, New_TextRow's texture. Arg1DB18_3bb8c_d is gone. `nameField` is a `struct TextRow *` now: unkAB/unkAC/unkAA are visibleCount/firstVisible/gapIndex; the New_TextRow cast is gone. Byte-identical (whole image green, 0 new warnings, nonmatching green).

## Track 7 (round 96, echo)

Naming: `D_8008AA18` -> `gSaveTitle` (tier A: the pointer every reader
uses as the save title's full-width text; TitleMenuTaskObjF.c writes the day
into it, hence `g`); `D_8008AA14` -> `sSaveTitleBlanks` (tier A: its ROM
value points at 19 full-width spaces). Both retyped `void *` -> `char *`
in include/class_3bb8c.h, which removes the `(char *)` casts. `+ 0x18` is
`SAVE_TITLE_PADDING * 2`, the same character index TitleMenuTaskObjF.c's
SAVE_TITLE_PADDING (12) names; each character is 2 bytes. Locals:
`size` -> `cellCount`, `buf` -> `text`.

Comments moved here from the unit:

- strcpy/strlen are Sony's, linked from libc2 (config/psyq-objects.txt:
  libc2/strcpy, libc2/strlen), declared locally per the per-unit
  convention for these two (src/ui/TextEntryItemList.c, src/class_3bb8c_j.c).
- DecodeFullWidthSjis is this unit's own view of the matched function in
  src/ScreenWidgets.c: `void`, because the return value is unused at these
  call sites, unlike that unit's own `u8 *` view (independent-arities
  convention).
