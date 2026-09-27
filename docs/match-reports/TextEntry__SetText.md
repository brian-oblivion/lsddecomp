# TextEntry__SetText -- MATCHED (28/28 words)

> Renamed from `TextEntry__SetName` on 2026-09-26 (tools/rename.py). Address 0x80050f28.

> Renamed from `Obj86ED0__SetName` on 2026-09-26 (tools/rename.py). Address 0x80050f28.

> Renamed from `func_80050F28` on 2026-09-24 (tools/rename.py). Address 0x80050f28.

Unit `class_3bb8c_i`, carved round 14.

`Obj86ED0`'s vtable slot 0x040. Stores its `mode` argument and name pointer,
resets two counters, then either transliterates the name into the owned
buffer (`DecodeFullWidthSjis`, an uncarved `ScreenWidgets` helper -- byte-translate
+ return-dest, same convention as `strcpy`) and halves `unk10` (a plain
signed `/2`, which this GCC compiles to the classic `srl`+`addu`+`sra`
round-toward-zero sequence), or falls back to a straight `strcpy` when
`mode != 1`.

```c
void TextEntry__SetText(Obj86ED0 *self, char *arg1, s32 mode)
{
    self->unkC = mode;
    self->unk24 = arg1;
    self->unk18 = 0;
    self->unk1C = 0;
    if (mode == 1) {
        DecodeFullWidthSjis(self->unk28, arg1);
        self->unk10 /= 2;
    } else {
        strcpy(self->unk28, arg1);
    }
}
```

## Residue and how it was closed

First attempt zeroed `unk1C` INSIDE the `mode == 1` branch (mirroring where
it reads in the raw `.s` file, which places the `sw zero,0x1C` in the
branch's delay slot). That scored 7/28: retail zeroes `unk1C`
UNCONDITIONALLY, alongside `unk18`, before the branch -- the delay slot is
just where the scheduler put an instruction that has no ordering dependency
on the branch, not where the SOURCE placed it. Moving `self->unk1C = 0;` up
next to `self->unk18 = 0;`, both unconditional and before the `if`, closed
it to 28/28.

### Proposed learning

A store sitting in a branch's delay slot in the `.s` listing is not evidence
it belongs inside that branch's C block -- MIPS delay slots hold whatever
instruction had no dependency on the branch outcome, which is frequently an
instruction that logically belongs BEFORE the branch. Check whether the
value/target is used on BOTH sides of the branch (here: `unk1C` is zeroed
regardless of which branch runs) before deciding it is conditional.

## Naming

- `TextEntry__SetText` -- tier A. gTextEntryMethods +0x040 (setName slot, classtable.py), the ctor's own tail dispatch: sets mode and copies/decodes the name string into unk28 (DecodeFullWidthSjis when mode==1, else plain strcpy). Pure setter, mechanics are the purpose.

## Track 4 (2026-09-26, round 87)

Class unified as `TextEntry` (include/TextEntry.h; table gObj86ED0Methods -> gTextEntryMethods, type Obj86ED0 -> TextEntry). The class name is for what its methods do: setText keeps a caller's string buffer and a working copy, the cursor and char methods edit the copy, command 25 writes it back, 23 closes without writing (banner of include/TextEntry.h). Fields renamed from their accessors: unk14 charCount, unk1C charIndex, unk20 altCommands, nameLen textLen, nameBuf textBuf, unk28 editBuf, unk40 cursorSprite (CharSprite *), unk44 textRow, unk48 panelSprite (ScreenSprite *). Zero bytes changed.

Renamed from Obj86ED0__SetName (tools/rename.py): nothing shows the string is a name, so the method is named for the buffer it takes. Slot +0x040 setText.

## Track 7 (2026-09-27, round 98, bravo)

Parameter `arg1` -> `text`. `mode == 1` is `TEXTENTRY_MODE_FULLWIDTH`
(new `enum TextEntryMode` in include/TextEntry.h: mode 1 decodes the
caller's full-width SJIS into editBuf and halves textLen, mode 0 is a
plain strcpy; HandleCommand's circle arm encodes back in the same mode).
`strcpy` comes from Sony's `<strings.h>` (libc2/strcpy.o is linked), not a
local prototype. Zero bytes changed.

Moved here from src/class_3bb8c_i.c (stale history; DecodeFullWidthSjis
is matched in ScreenWidgets.c since round 38, typed `u8 *(u8 *dst, u8
*src)`): "This project's own strcpy (matched elsewhere) -- TextEntry__SetText's
own caller, same local-declaration convention as class_3bb8c_e.c/others."
and "Uncarved helper, `ScreenWidgets`, still INCLUDE_ASM --
TextEntry__SetText's own call. Translates each byte of `src` (a name
string) into `dest` (folding a couple of special-case byte ranges) and
returns `dest`, same convention as `strcpy`. Typed purely from this call
site's own register usage. Declared HERE, not in include/class_3bb8c.h:
src/class_3bb8c_j.c types the same (still undefined) function as `void
(void *, void *)` from its own call site, and two call-site typings of one
function cannot share a header."
