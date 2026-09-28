# TextEntry__SetCursorPos — MATCHED (round 45, 41/41 words)

> Renamed from `Obj86ED0__DispatchIndexValue` on 2026-09-26 (tools/rename.py). Address 0x800518f4.

> Renamed from `func_800518F4` on 2026-09-24 (tools/rename.py). Address 0x800518f4.

**Unit:** class_3bb8c_i · **Size:** 41 words (0xA4 bytes)

Filed as a `gp_rel`-blocked stub in round 15. That blocker was RESOLVED in
round 42 (`--gp-symbols`, pinned in the Makefile). Matched on the first
attempt once rebuilt against the fixed toolchain.

## ROUND 75 CORRECTION

This report originally typed `self` as `Obj866E8` (gStageMapMethods) and typed
`self->unk40` through a unit-local `Unk40Obj866E8`/`Unk40Obj866E8Methods`
duplicate. Both were wrong. `tools/classtable.py gTextEntryMethods` places this
function at that table's +0x0A4 (`gStageMapMethods`'s 80 slots hold none of this
group's six addresses) -- `self` is `Obj86ED0` (class_3bb8c_i's shared
type), whose OWN struct in `include/class_3bb8c.h` already types
`self->unk40` as `ChildObj86ED0 *`. The `+0x0BC` slot this function
dispatches through was simply missing a name on the shared
`ChildMethods86ED0` -- added additively there instead of duplicated
locally. See `src/class_3bb8c_i.c`'s file header comment and
`TextEntry__PrevChar.md` for the full evidence trail. Zero bytes
affected (type names are not codegen).

## Derivation

```c
typedef struct {
    s32 unk0;
    s32 unk4;
} SlotBCArg866E8_3bb8c_j;

extern s32 gTextEntryCursorPos;
extern s32 D_8008AAE0;

void TextEntry__SetCursorPos(Obj86ED0 *self, s32 arg1, s32 arg2)
{
    SlotBCArg866E8_3bb8c_j local;
    ChildObj86ED0 *obj;

    if (self->unk48) {
        local.unk4 = D_8008AAE0;
        local.unk0 = arg1 * 7 + gTextEntryCursorPos;
        obj = self->unk40;
        obj->methods->slotBC(obj, &local);
        self->unk18 = arg1;
        if (arg2) {
            self->methods->slot60(self, 0);
        }
    }
}
```

Part of the same `Obj86ED0` "countdown/flush" group established in round 15
(`TextEntry__PrevChar`/`TextEntry__ToggleActOnHeld`/`TextEntry__ResetChar`/`TextEntry__ResetAllChars`, same
unit): tests `self->unk48` as a readiness gate, then calls through
`self->unk40`'s own method table at slot `0xBC` with a 2-word stack-local
argument block, then records `self->unk18 = arg1` and, if `arg2` is
non-zero, notifies through `self->methods->slot60(self, 0)`.

`self->unk18 = arg1` sits in the retail delay slot of the `beqz arg2, end`
branch, i.e. it executes unconditionally whenever `self->unk48` is set —
matching ordinary C where the assignment is a plain statement before the
`if (arg2)` block, which the scheduler is free to move into the branch's
delay slot since it doesn't depend on the branch outcome.

**Header additions** (`include/class_3bb8c.h`, additive):
- `ChildMethods86ED0::slotBC` (offset 0x0BC) — round 75, this function's own
  dispatch target on the ALREADY-shared `ChildObj86ED0` (`self->unk40`).
  `unk28`/`unk40`/`unk44`/`Obj86ED0Methods::slot60` were already present on
  the shared `Obj86ED0`/`Obj86ED0Methods`, established by class_3bb8c_i —
  no edit needed for those.

`gTextEntryCursorPos` is read here as a plain VALUE (`s32`, used arithmetically:
`arg1 * 7 + gTextEntryCursorPos`), a different reading from `class_3bb8c_i.c`'s own
`extern s32 gTextEntryCursorPos;` (there, only its ADDRESS is taken, as an opaque
`slot4C` argument). Both are legitimate independent local views of the same
global per the project's convention — they're in separate translation
units and never collide.

### Proposed learning

None beyond what `StampSaveTitleDay`'s report already covers this round; this
one matched cleanly on the first attempt once the stub's stale `gp_rel`
verdict was set aside.

## Naming

- `TextEntry__SetCursorPos` -- tier B. slotA4 occupant. Computes arg1*7+gTextEntryCursorPos into a 2-word stack block together with the D_8008AAE0 constant, dispatches it through self->unk40's own slotBC, records self->unk18=arg1, and optionally notifies via slot60. "Index"/"Dispatch" describe the mechanics; what the computed value represents in-game is not established. classtable.py gTextEntryMethods +0x0A4 (the real occupant -- corrects this unit's earlier Obj866E8 misattribution, see the ROUND 75 CORRECTION section above).

## Track 4 (2026-09-26, round 87)

Class unified as `TextEntry` (include/TextEntry.h; table gObj86ED0Methods -> gTextEntryMethods, type Obj86ED0 -> TextEntry). The class name is for what its methods do: setText keeps a caller's string buffer and a working copy, the cursor and char methods edit the copy, command 25 writes it back, 23 closes without writing (banner of include/TextEntry.h). Fields renamed from their accessors: unk14 charCount, unk1C charIndex, unk20 altCommands, nameLen textLen, nameBuf textBuf, unk28 editBuf, unk40 cursorSprite (CharSprite *), unk44 textRow, unk48 panelSprite (ScreenSprite *). Zero bytes changed.

Renamed from Obj86ED0__DispatchIndexValue: moves cursorSprite with its setPosition slot (ScreenSprite +0x0BC) to x = pos * 7 + gTextEntryCursorPos, y = D_8008AAE0 (the stack block is a ScreenSpritePos), stores cursorIndex, and calls notifyTarget(0) when `notify`. Occupant of slot +0x0A4, called by MoveCursorRight/Left and ResetAllChars. Tier A (the body is the name).

## Round 98: gTextEntryCursorPos unified (track 4b)

`gTextEntryCursorPos` (0x8008AADC) now has one declaration, `extern
ScreenSpritePos gTextEntryCursorPos;` in include/TextEntry.h; this unit's
`extern s32 gTextEntryCursorPos` and `extern s32 D_8008AAE0` are gone. The
body reads `local.y = gTextEntryCursorPos.y; local.x = pos * 7 +
gTextEntryCursorPos.x;`. `D_8008AAE0` was that position's y (-12, the word at
+4, `ScreenSpritePos.y`): the `%gp_rel(gTextEntryCursorPos+4)` load assembles
to the same bytes as `%gp_rel(D_8008AAE0)`, and the whole image stayed
byte-exact. The splat label D_8008AAE0 remains in the data listing with no C
reference.

## Track 7 (round 100, charlie)

Locals `local`/`obj` renamed `screenPos`/`cursor`. The `* 7` is
TEXTROW_DEFAULT_PITCH (include/TextRow.h): gTextEntryCursorPos.x (-62)
equals gTextEntryTextPos.x (-62, class_3bb8c_i), so the cursor steps across
the text row's cells at the row's own default pitch. Zero bytes changed.
