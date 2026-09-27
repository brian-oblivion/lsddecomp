# TextEntry__LoadCardResources -- MATCH (119/119 words, ~5 attempts)

> Renamed from `Obj86ED0__LoadCardResources` on 2026-09-26 (tools/rename.py). Address 0x80050f98.

> Renamed from `func_80050F98` on 2026-09-24 (tools/rename.py). Address 0x80050f98.

Unit `class_3bb8c_i`, 125-line body -- the largest function in this round's
batch. Obj86ED0's own "load the two memory-card TIM resources" method:
builds two `CARD\\<NAME>.TIM` paths, loads each through a short-lived
resource handle, and wraps/converts each into a `ChildObj86ED0`-shaped
object stashed on `self` (`unk48`, `unk44`, `unk40`).

```c
extern char *BuildFileName(char *dest, char *arg1, char *arg2, char *arg3);
extern ChildObj86ED0 *New_TimImage(char *path);
extern ChildObj86ED0 *New_ScreenSprite(ChildObj86ED0 *arg0, void *arg1, s32 arg2);

extern const char sStrComInput[]; /* "COMINPUT" */
extern const char sStrFontIcon[]; /* "FONTICON" */
extern const char sCardPathPrefix[]; /* "CARD\\" */
extern const char sTimExt[]; /* ".TIM" */
extern s32 gTextEntryPanelRect; /* 3-word opaque block, New_ScreenSprite's arg1, address-only here */
extern s32 gTextEntryTextColor; /* opaque block, slotB8's arg1, address-only here */
extern s32 gTextEntryPanelPos; /* opaque block, self->unk48's slot4C arg2, address-only here */
extern s32 gTextEntryTextPos; /* opaque block, self->unk44's slot4C arg2, address-only here */
extern s32 gTextEntryCursorPos; /* opaque block, self->unk40's slot4C arg2, address-only here */

void TextEntry__LoadCardResources(Obj86ED0 *self, void *arg1)
{
    char path[0x20];
    const char *dir;
    const char *ext;
    ChildObj86ED0 *handle1;
    ChildObj86ED0 *handle2;

    if (arg1 == NULL) {
        return;
    }
    if (self->unk48 != NULL) {
        return;
    }

    dir = sCardPathPrefix;
    ext = sTimExt;

    handle1 = New_TimImage(BuildFileName(path, sStrComInput, dir, ext));
    handle1->methods->slot78(handle1);
    self->unk48 = New_ScreenSprite(handle1, (void *)&gTextEntryPanelRect, 0);
    handle1->methods->release(handle1);
    self->unk48->methods->slot4C(self->unk48, arg1, (void *)&gTextEntryPanelPos);

    handle2 = New_TimImage(BuildFileName(path, sStrFontIcon, dir, ext));
    handle2->methods->slot78(handle2);
    self->unk44 = New_TextRow(handle2, self->unk10, self->unk28);
    self->unk40 = (ChildObj86ED0 *)New_CharSprite(handle2, 0x5F);
    handle2->methods->release(handle2);
    self->unk44->methods->slot4C(self->unk44, arg1, (void *)&gTextEntryTextPos);
    self->unk44->methods->slotB8(self->unk44, (void *)&gTextEntryTextColor);
    self->unk40->methods->slot4C(self->unk40, arg1, (void *)&gTextEntryCursorPos);
}
```

## Deriving the shape

`BuildFileName` is already matched (`src/code_171e0.c`):
`dest[0]=0; if (arg2) strcat(dest,arg2); strcat(dest,arg1); strcat(dest,arg3); return dest;`
-- i.e. it builds `arg2 + arg1 + arg3` into `dest`. The two call sites here
pass `(path, "COMINPUT", "CARD\\", ".TIM")` and
`(path, "FONTICON", "CARD\\", ".TIM")`, i.e. building
`CARD\COMINPUT.TIM` and `CARD\FONTICON.TIM` -- confirmed from the literal
rodata bytes at `sStrComInput`/`sStrFontIcon`/`sCardPathPrefix`/`sTimExt`
(`asm/data/1DD0.rodata.s`, `asm/data/7B008.sdata.s`).

`New_TimImage` (a resource loader taking a path, returning a handle) is
already typed at several OTHER call sites in the project
(`code_2cc8c.h`/`class_39e08.h`/`class_3bb8c_j.c`, all with their own local
return-type view per this project's established convention) -- confirmed
this is the same function by address, given its own local reading here.

The temp handle (`handle1`/`handle2`) dispatches `slot78` (self only, void)
right after loading, and `release`/`slot4` (self only, return discarded)
right before its own scope ends -- this matches `ChildObj86ED0`'s existing
`release` slot (established elsewhere in this unit, `TextEntry__ReleaseCardResources`) in
both offset (+0x004) and shape, so the temp handle and `self->unk40/44/48`
share the SAME `ChildObj86ED0` type. Extended `ChildMethods86ED0`
additively with `slot4C` (+0x04C, `(self, void *arg1, void *arg2)`, called
on `self->unk48`/`unk44`/`unk40`) and `slotB8` (+0x0B8,
`(self, void *arg1)`, called on `self->unk44` only) and `slot78` (+0x078,
`(self)`, called on the temp handle only). `include/class_3bb8c.h` already
has an UNRELATED type (`FieldM7CMethods`) with the identical offsets/arity
for `slot4C`/`slotB8` (established by a different function,
`ObjM__AdvancePauseSetup`, elsewhere in this same header) -- per the project's
documented multiple-independent-local-views convention, this is NOT
unified with `ChildMethods86ED0`; they stay two separate local readings of
what may well be the same underlying vtable.

`New_TextRow` is ALREADY declared in the shared `include/class_3bb8c.h`
(a different unit's local view, return type `FieldM7C *`), so
`self->unk44 = New_TextRow(...)` is an implicit-conversion assignment
(`FieldM7C *` into `ChildObj86ED0 *`) -- a harmless warning under this
project's `-Wall`-without-`-Werror` build, not a compile error, and zero
bytes of cost (pointer reinterpretation is free). `New_CharSprite` and
`New_ScreenSprite` are NOT reachable from any header this unit includes, so
they got fresh local `extern` declarations here, typed purely from this
call site's own register usage (same convention as `DecodeFullWidthSjis` above
in this file) -- and diverge from `New_CharSprite`'s OTHER call-site typing
in `code_2cc8c.h` (`(s32, s32)`), which is expected and fine.

## The one real residue: register identity from live-range shape, not code shape

The first working version of the body (semantically identical to the one
above, but without the split into `handle1`/`handle2` -- a single reused
`handle` local across both blocks) scored 98/119, with every mismatch a
pure register swap: retail assigns `handle=$s0`, `dir=$s2`, `ext=$s1`; the
single-`handle` version assigned `handle=$s2`, `dir=$s1`, `ext=$s0` --
exactly reversed. No amount of reordering the `dir`/`ext` assignment
statements, or their declaration order, moved this at all (tried: swapping
declaration order of `dir`/`ext`/`handle`; swapping assignment-statement
order of `dir`/`ext`; both independently -- zero effect on the generated
registers in every case).

What DID move it: splitting the reused `handle` local into two
non-overlapping-lifetime locals (`handle1` for the first block, `handle2`
for the second), leaving `dir`/`ext` untouched. That alone produced a
byte-exact match. Best working theory (not verified against GCC 2.6.3
internals, offered as a lead for the next person who hits this): a value
whose live range spans BOTH blocks from its very first assignment (`dir`,
`ext` -- assigned once, used in both) is a structurally different pseudo
than one reassigned once per block (`handle`), and GCC's register allocator
assigns hardware `s`-registers by pseudo CREATION order with parameters
(`self`, `arg1` here) taking the highest slots (`$s3`, `$s4`) and locals
filling downward from there in the order their pseudo is created -- but
"created" for a CSE-recognized repeated constant may not mean "first
assigned in source," it may mean "first recognized as needing a persistent
register," which can differ when the same local is written to twice across
two blocks versus once. This is offered as a diagnostic lead, not a proven
mechanism.

### Proposed learning

When a "temp handle, used and released within one block, pattern repeated
in a second block" shows up, DON'T default to reusing one local across both
blocks even though it looks tidier and is semantically identical -- try two
separately-named locals (one per block) FIRST if the function also has
other values (like `dir`/`ext` here) that genuinely persist across both
blocks unchanged. Reusing the single local measurably changed those OTHER
values' register assignment too, not just the reused local's own -- the
three-value swap was resolved by touching only the non-persistent one.

## Naming

- `TextEntry__LoadCardResources` -- tier A. gTextEntryMethods +0x044 (classtable.py). Resolves 'CARD\\COMINPUT.TIM'/'CARD\\FONTICON.TIM' memory-card paths (sCardPathPrefix/sStrComInput/sStrFontIcon/sTimExt, all this unit's own strings) and loads/wraps them into the three resource handles (unk48/unk44/unk40). String evidence is direct, not inferred.

## Track 4

2026-09-25, round 84 (charlie): The class `New_D8006ED4C` constructs is unified as ScreenSprite in `include/ScreenSprite.h`; the unit includes it and its local extern is gone. The call reads `self->unk48 = (ChildObj86ED0 *)New_ScreenSprite(handle1, (SpriteRect *)&gTextEntryPanelRect, 0)`: gTextEntryPanelRect is the rect (words 0, 224, 120), and unk48's +0x04C call passes the screen position gTextEntryPanelPos = (-70, -60). Image byte-identical.

2026-09-26, round 86 (bravo): CharSprite (class 0x1144, formerly D_8006EC74) is unified in `include/CharSprite.h`. The local `extern ChildObj86ED0 *New_CharSprite(ChildObj86ED0 *, s32)` is gone; the unit includes the header and casts the result to `unk40`'s `ChildObj86ED0 *`, as it does for New_ScreenSprite. The 0x5F cell it asks for on FONTICON.TIM is '_', one of the facts behind the class name. Image byte-identical.

## Track 4 (2026-09-26, round 87)

Class unified as `TextEntry` (include/TextEntry.h; table gObj86ED0Methods -> gTextEntryMethods, type Obj86ED0 -> TextEntry). The class name is for what its methods do: setText keeps a caller's string buffer and a working copy, the cursor and char methods edit the copy, command 25 writes it back, 23 closes without writing (banner of include/TextEntry.h). Fields renamed from their accessors: unk14 charCount, unk1C charIndex, unk20 altCommands, nameLen textLen, nameBuf textBuf, unk28 editBuf, unk40 cursorSprite (CharSprite *), unk44 textRow, unk48 panelSprite (ScreenSprite *). Zero bytes changed.

## Track 4 (2026-09-26, round 88)

TimImage is unified (`include/TimImage.h`); this unit's local `extern
ChildObj86ED0 *New_TimImage(char *)` is deleted. `handle1`/`handle2` are
`TimImage *`: +0x078 (FileResource's `void *slot78`, occupant
TimImage__Upload) through `TimImageUploadFn`, +0x004 the inherited
`release`. `ChildObj86ED0` stays for `textRow`. Image byte-identical.

## Round 94 (track 6, charlie): textRow is a TextRow

`ChildObj86ED0`/`ChildMethods86ED0` (include/class_3bb8c.h) are deleted: the
object behind them is the `New_TextRow` result, so TextEntry::textRow
(+0x044, include/TextEntry.h) is `struct TextRow *`. The slots map onto
TextRow's table offset for offset: +0x004 `release`, +0x04C `attachToParent`
(position cast to `LongVec3 *`, as ScreenSprite's banner describes), +0x0B8
`setColor` (`gTextEntryTextColor`, the 0x80/0x80/0x00 word, passed as `SpriteRgb *`),
+0x0C4 `setCell`, called through `TextRowSetCellAtFn` because
TextRow__SetCellAt takes the index too. Zero bytes changed.
