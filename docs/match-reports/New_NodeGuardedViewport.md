# New_NodeGuardedViewport

> Renamed from `New_Class869D8` on 2026-09-26 (tools/rename.py). Address 0x8004d254.

> Renamed from `func_8004D254` on 2026-09-22 (tools/rename.py). Address 0x8004d254.

**Unit:** TitleMenuTaskObjF · **Size:** 20 words · **Status:** MATCHED (20/20)

## What it does

`New_NodeGuardedViewport`: the allocator for `NodeGuardedViewport`. Standard
allocate-null-check-ctor shape, identical to DayTaskStageMap.c's
`New_StageMap`/DayTaskStageMap.c's several `New_X` functions -- allocate a
fixed-size block (`0xDC` bytes here), and on success run the class's ctor
(`NodeGuardedViewport__NodeGuardedViewport`, fetched through `GetNodeGuardedViewportMethods()->ctor`) and return the
new instance; return `NULL` on allocation failure.

## The C

```c
NodeGuardedViewport *New_NodeGuardedViewport(void)
{
    NodeGuardedViewport *self;

    self = BMemPMgrAlloc(0xDC);
    if (self != NULL) {
        GetNodeGuardedViewportMethods()->ctor(self);
        return self;
    }
    return NULL;
}
```

## Notes

Matched on the first attempt -- this is a direct copy of the established
New_X idiom already confirmed several times elsewhere in this codebase
(see docs/MATCHING-GUIDE.md, "Writing a class method" / "Allocation sites
look like..."). No residue.

## Naming

**New_NodeGuardedViewport** -- tier A. Pure `New_X` allocator idiom (allocate fixed
size, null-check, ctor, return); the allocator's mechanics ARE its purpose
by the tier-A leaf rule. Matches the project's established `New_StageMap`/
`New_X` naming convention (class_3ac78.c, DayTaskStageMap.c) exactly, and this
name was already in use in this function's own report prose before the
round-68 rename made it real.

## Track 6 (2026-09-26, round 92, echo)

The class was renamed `Class869D8` -> `NodeGuardedViewport` (`tools/renametype.py`, tier B): its one behavioural override, update, runs Viewport__Update only while a view node is attached, which is what lets ObjM leave it detached after ExitSceneStyle. The name says the mechanism, not the viewport's role in the game. (renametype also rewrote the historical token `Class869D8__ForwardIfUnk10AndUnk70` above.)

## History: the unit banner before track 7 (round 100)

Track 7's pass on class_3bb8c_c (round 100, alpha) rewrote the unit banner as
documentation. The banner as it stood at the start of the round, verbatim
(before `CheckSaveScoreFlag` became UpdateFlashbackLock and
`FormatNumberIntoBuffer` became StampSaveTitleDay):

```
/*
 * class_3bb8c_c -- three small sibling classes, each built by its own
 * New_X/ctor pair (allocate, chain a base ctor, install the class's own
 * vtable): NodeGuardedViewport, GridCell and TitleMenu. All three follow
 * the same class-framework shape documented in
 * docs/research/class-framework.md and already used elsewhere in this
 * codebase (e.g. class_3ac78.c's StageMap).
 *
 * NodeGuardedViewport (include/NodeGuardedViewport.h) is a Viewport whose
 * update skips the frame while no view node is attached; its seven table
 * methods, New_ and the getter are all here. GridCell (include/GridCell.h,
 * a SceneNode) is one cell of StageMap's grid, carrying the model placed
 * there; its five table methods, New_ and the getter are all here too.
 * TitleMenu (include/TitleMenu.h, a TaskCore) is the menu between days;
 * only its allocator and ctor are here, its other methods in
 * class_3bb8c_d.c.
 *
 * Two free functions serve TitleMenu: CheckSaveScoreFlag (called from
 * TitleMenu__RefreshMenu) writes the menu's FLASHBACK lock,
 * registrationSlots[1], from two words of the save block; and
 * FormatNumberIntoBuffer (called from the ctor with the current day) writes
 * the day as three full-width digits into the save title, "LSD   Day001"
 * (FullWidthChars3, include/TitleMenu.h's type for the title's characters).
 *
 * All 20 definitions here are matched, 0 INCLUDE_ASM.
 */
```

Dropped from the new banner: the pointer to docs/research/class-framework.md
and class_3ac78.c's StageMap as the shape's example, the register slot index
`registrationSlots[1]` (the code now says TITLEMENU_FLASHBACK), and the
status line "All 20 definitions here are matched, 0 INCLUDE_ASM", which
`tools/progress.py` measures.

## Constants (round 100, track 7)

`BMemPMgrAlloc(0xDC)` is `BMemPMgrAlloc(sizeof(NodeGuardedViewport))`:
the struct ends at +0x0DC (`pad0BC[0x0DC - 0x0BC]`), and the image is
byte-identical.
