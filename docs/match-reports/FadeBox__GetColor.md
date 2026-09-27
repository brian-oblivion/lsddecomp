# FadeBox__GetColor -- MATCH (13/13 words, first attempt)

> Renamed from `Class6E99C__GetColor` on 2026-09-26 (tools/rename.py). Address 0x800403f8.

> Renamed from `func_800403F8` on 2026-09-20 (tools/rename.py). Address 0x800403f8.

Unit `code_2cc8c_e`, carved round 14. `FadeBoxMethods::getColor` (`+0x0E4`).

```c
void *FadeBox__GetColor(FadeBoxObj *self) {
    if (self->unk78 == 0xF) {
        return gFadeBoxBlackColors;
    }
    return &gFadeBoxMaskColors[self->unk78 * 3];
}
```

Same 3-byte-stride table lookup family as `FadeBox__StartFadeDown`/`FadeBox__StartFadeUp`,
using `self->unk78` directly as the index (with `0xF` as a distinguished
"use the fixed table" sentinel, matching `unk78`'s established use
elsewhere in this unit as a mode/flags word tested against `0xF`).

## Naming (round 61, track 3)

**`FadeBox__GetColor`** -- tier A. `FadeBoxMethods::getColor`
(`+0x0E4`). Pure getter: returns `gFadeBoxBlackColors` (the fixed/default table)
when `unk78 == 0xF`, else `&gFadeBoxMaskColors[unk78 * 3]` (the indexed table) --
the same two tables `FadeBox__StartFadeDown`/`StartFadeUp`
write through `slotB8`. A pure leaf whose mechanics ARE its purpose (a
getter) is tier A by this project's own naming rule.

## Track 6 (2026-09-26, round 93, charlie)

Renamed with the class: `Class6E99C` is now `FadeBox`
(`python3 tools/renametype.py Class6E99C FadeBox`, table
`python3 tools/rename.py D_8006E99C gFadeBoxMethods`), tier A; the evidence
is in New_FadeBox.md's Track 6 section. The method's own name was kept: it
already says what the body does. renametype.py rewrote the old class name in
this report's earlier history too (known, pending an operator decision).

