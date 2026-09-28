# New_CharSprite -- MATCHED (27/27 words), round 82

> Renamed from `New_D8006EC74` on 2026-09-26 (tools/rename.py). Address 0x80041ab4.

> Renamed from `func_80041AB4` on 2026-09-25 (tools/rename.py). Address 0x80041ab4.

Round 82, runner alpha (fourth slot on Sprite). Unit `src/Sprite.c`. Fresh ground, no prior attempt.

- **Where:** not in any method table (allocator for CharSprite, the 8x8-cell sprite). Called from TextEntryItemList.c (`New_CharSprite(handle2, 0x5F)`) and ScreenWidgets.c (`New_CharSprite(a1, 0x20)`).
- **What:** `BMemPMgrAlloc(0xAC)`; if non-NULL, calls slot +0x008 (ctor, CharSprite__CharSprite) of `GetCharSpriteMethods()` (the gCharSpriteMethods table) with `(obj, texture, cell)` and returns obj, else NULL.
- **Result:** byte-exact, 27/27 words, 0 ins / 0 del, whole-image SHA1 green. Third build.
- **Levers, measured:**
  - `s32 cell` param: 24/27, equal length; the prologue's `li a0,0xAC` moved from before `sw s2 / move s2,a1` to after them (3 positional diffs, words 3-5). Same with `u32 cell`.
  - `u8 cell` param: 27/27. The `andi a2,s2,0xFF` in the jalr delay slot is the u8 PARAMETER being narrowed at its use, not a conversion into the callee's prototype; the declared parameter type is what reorders the prologue.
- **Types:** unit-local `CellCtorMethods_322b4` (ctor at +0x008 taking `(self, texture, u8 cell)`) and a prototype for `GetCharSpriteMethods`; no shared header touched. Callers elsewhere declare it `(s32, s32)` / `(ChildObj86ED0 *, s32)` locally; both are call-compatible and untouched.

## Source

```c
/* Allocate and construct a CharSprite (0xAC bytes): one character cell. */
CharSprite *New_CharSprite(void *texture, u8 cell) {
    CharSprite *obj = BMemPMgrAlloc(0xAC);

    if (obj != NULL) {
        GetCharSpriteMethods()->ctor(obj, texture, cell);
        return obj;
    }
    return NULL;
}
```

### Proposed learning

Allocator/wrapper whose prologue has the constant arg set (`li a0,K`) BEFORE a callee-saved copy of a later param, with an `andi 0xFF` on that copy at the call: declare the param `u8`. `s32`/`u32` params put the `li` after the save (equal length, 3 positional diffs).

## Naming

- `New_D8006EC74` -- tier A. Allocator: BMemPMgrAlloc(0xAC) then calls the ctor slot -- the "New_<Class>" allocator convention already used throughout the project (New_Sprite, New_StageMap); mechanics are the whole purpose.

## Track 4

2026-09-26, round 86 (bravo): class 0x1144 unified as CharSprite in `include/CharSprite.h`. Renamed from `New_D8006EC74`, tier A: the allocator, `New_<Class>`. Returns `CharSprite *` and calls the ctor through the unified table; the unit-local `CellCtorMethods_322b4` is gone, as are the callers' local declarations (include/Task.h's `Obj6EAC0 *(s32, s32)`, TextEntryItemList's `ChildObj86ED0 *(ChildObj86ED0 *, s32)`); both callers now cast the result to their own field types. The Source block above is the unified spelling. Image byte-identical.

## Track 7 (round 99, charlie)

Allocation size spelled `sizeof(CharSprite)` (0xAC, the struct's size with word padding after `cellIndex`). Byte-exact.

### History: code_322b4's unit banner before round 99

The unit's banner was rewritten as documentation of what the file holds; the previous text, with the unit's carving and matching history, is kept here verbatim.

```c
/*
 * code_322b4 -- GAME code carved from psyq_322b4 on 2026-09-25 (FINISHING-PLAN
 * revision 18). 0x322B4..0x330F4 (vram 0x80041AB4..0x800428F4). It was counted
 * as Psy-Q SDK by segment name; tools/gameinsdk.py measured it as game (a call
 * into game code, a method-table entry beside game methods, or contiguity with
 * those, and no Sony fingerprint). Owns jtbl_80011290 (attached rodata
 * sub-slot 0x1A90).
 *
 * Round 82 matched every function in the unit (getters, accessors, empty
 * overrides, then the 11- to 20-word bodies: the sprite attribute-bit
 * setters, cell selection, finalize chains, the FrameClock allocator), then
 * named it (track 3): every function is real C, not `func_`.
 *
 * Sprite (include/Sprite.h, gSpriteMethods), its direct subclass
 * ScreenSprite (include/ScreenSprite.h, gScreenSpriteMethods, id 0x144: the
 * screen-space sprite, adding setPosition (+0x0BC, screenPos) and a
 * pivot-anchor setter (+0x0C0, centre/left/right/top/bottom)) and ITS
 * subclass CharSprite (include/CharSprite.h, gCharSpriteMethods, id 0x1144:
 * one 8x8 font character, adding setCell/getCell (+0x0C4/+0x0C8); GetCellRect
 * is the free helper its ctor and setCell use to turn a cell index into a
 * rect) are unified; their own methods (New_Sprite, Sprite__*, InitGsSprite,
 * GetSpriteMethods, New_ScreenSprite, ScreenSprite__*, GetScreenSpriteMethods,
 * New_CharSprite, CharSprite__*, GetCharSpriteMethods) live here. Every
 * class owning methods in this unit is now unified (track 4; FrameClock, the
 * last, round 88). gTextRowMethods (0x11144, below CharSprite) and
 * gVariantSpriteMethods (0x1F44, class_3bb8c_p/q/t) are Sprite subclasses too but own
 * no methods in this unit.
 * FrameClock (include/FrameClock.h, gFrameClockMethods, id 0x5) is unified:
 * a BasicClass subclass ticked once per DrawSystem frame that tells its
 * parents event 2 (counted), 3 (paused) or 4 (flag14); its own methods
 * (New_FrameClock, FrameClock__*, Get_vtable_FrameClock) live here.
 * RequestedFile (include/RequestedFile.h, gRequestedFileMethods, id 0xB03) is
 * unified: a FileResource that requests one named file from the active driver
 * at construction and sets `loaded` when the driver's setFlag reports it
 * read (WBgm's SEQ file); its own methods (New_RequestedFile,
 * RequestedFile__*, GetRequestedFileMethods) live here.
 * LightRig (include/LightRig.h, gLightRigMethods, id 0x14) is unified too: a
 * SceneNode subclass owning three FlatLightObj children and an ambient
 * colour (SetAmbientColor -> GsSetAmbient); its own methods (New_LightRig,
 * LightRig__*, GetLightRigMethods) live here. Its getLight (+0x0B8) is
 * inherited unchanged by StageMap's own table (gStageMapMethods), which is why
 * one function occupies the same slot in both.
 */
```

## History (moved from src/Sprite.c, comments pass)

The file's banner carried its edge evidence and the reason it is parked:

> What decided its edges (python3 tools/tuboundary.py): the placed object
> libc2/memmove precedes it ("start edge possible") and libgs/gs_110
> (GsSetAmbient) follows it, so there is nothing to merge with. Inside, 47
> of the 50 edges are "boundary possible" and the three around
> ScreenSprite's position and pivot methods "boundary unlikely (single-user
> data)". The "forced boundary lies in this stretch" notes are the
> jump-table pairs 0x800111dc / 0x80011290 (libc2/sprintf's table, then
> ScreenSprite__SetPivotAnchor's) and 0x80011290 / 0x8001140c; both
> intervals cross placed Sony objects, whose edges satisfy them, so they say
> nothing about a boundary inside this file. PARKED: the content would split
> it (the sprite classes, then RequestedFile, FrameClock and LightRig, which
> are not sprites), but a split is a new carve, not a merge or rename, so the
> file keeps its carve edges and is named for the class family that fills
> nearly half of it and heads it.
