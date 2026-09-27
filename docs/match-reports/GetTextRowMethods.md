# GetTextRowMethods — MATCHED (4/4 words)

> Renamed from `Obj6EAC0__GetDerivedMethods` on 2026-09-26 (tools/rename.py). Address 0x80040fb0.

> Renamed from `func_80040FB0` on 2026-09-18 (tools/rename.py). Address 0x80040fb0.

Unit: `src/ScreenWidgets.c`. First attempt.

```c
Obj6EAC0Methods *GetTextRowMethods(void) {
    return &gTextRowMethods;
}
```

The override-table twin of `GetBoxFillMethods` (returns `&gBoxFillMethods`).
Called by `New_TextRow` (this unit, the class's `New_X`-shaped
allocator) to fetch the constructor at `slot0x08`.

### Proposed learning

None beyond what's already recorded.

## Naming

Round 54 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_80040FB0` | `GetTextRowMethods` | A |

**Evidence.** A pure leaf getter (tier A by definition): returns
`&gTextRowMethods`, the override method table. Twin of
`GetBoxFillMethods` (returns `&gBoxFillMethods`, the base table).

## Track 4 (2026-09-25, round 85, charlie)

Not a twin of GetBoxFillMethods in the class tree: gTextRowMethods is class 0x11144, below CharSprite (0x1144) and ScreenSprite (0x144), and gBoxFillMethods is class 0x64 (include/BoxFill.h). Round 14's "base/override" pairing of the two tables does not hold.

## Track 4 (2026-09-26, round 88, charlie)

2026-09-26, round 88 (charlie): class 0x11144 unified as TextRow in `include/TextRow.h` (a row of CharSprite cells: the ctor makes `count` New_CharSprite cells, setText hands each the next byte of a string, the layout slots step `cellPitch` along x). The view `Obj6EAC0` (named after BoxFill's old table address) is gone; its +0x00C `hasChildren` is SceneNode's `parent` (--merge CONFLICT s32 vs pointer: only tested against 0, bytes unchanged), `children` (+0x0B4) is `CharSprite **cells`, and the per-cell calls go through CharSprite's slots by name. Renamed from `Obj6EAC0__GetDerivedMethods`: the class getter, returns &gTextRowMethods (was D_8006EB90, renamed with rename.py). Image byte-identical; the current source is src/ScreenWidgets.c.
