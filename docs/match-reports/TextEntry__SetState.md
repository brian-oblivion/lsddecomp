# TextEntry__SetState -- MATCHED (42/42 words)

> Renamed from `Obj86ED0__SetState` on 2026-09-26 (tools/rename.py). Address 0x800512c8.

> Renamed from `func_800512C8` on 2026-09-24 (tools/rename.py). Address 0x800512c8.

Unit `input_dialogs`, carved round 14.

`Obj86ED0`'s vtable slot 0x054, called by `TextEntry__TickState` (that unit's own
sibling function, still `INCLUDE_ASM` this round) with a literal `4`.
Resets `unk30`, returns early for `arg1 < 2`, then dispatches on `arg1`:
`{2,3}` detaches `unk34` and calls this class's own `slot48`
(`TextEntry__ReleaseCardResources`) before stashing `arg1` into `unk2C`; `4` calls the
UNMODIFIED base `notifyParents` (`BasicClass__NotifyParents`, confirmed not
overridden via `tools/classtable.py gTextEntryMethods --vs gBasicClassMethods`) with
`unk2C` as its argument, reached through `self->methods` since that table
slot is identical to the base's either way.

```c
void TextEntry__SetState(Obj86ED0 *self, s32 arg1)
{
    self->unk30 = 0;
    if (arg1 < 2) {
        return;
    }
    switch (arg1) {
    case 2:
    case 3:
        self->methods->removeChild(self, self->unk34);
        self->methods->slot48(self);
        self->unk2C = arg1;
        break;
    case 4:
        self->methods->notifyParents(self, self->unk2C);
        break;
    }
}
```

## Residue and how it was closed (2 attempts)

Retail's own branch shape is unusual for a simple range dispatch: after the
early `arg1 < 2` return, it tests `arg1 < 4` and branches TO the `{2,3}`
body on TRUE (rather than skipping AROUND it on false), then falls through
to an `arg1 == 4` test, then an EXPLICIT unconditional jump past both bodies
for anything else.

- **Attempt 1**: `if (arg1 < 4) { ... } else if (arg1 == 4) { ... }` scored
  14/42 (also shorter than retail by 8 bytes, drifting `New_TextEntry`'s own
  `jal` target downstream). Disassembly showed GCC inverted the first test
  (`beqz`, skip-around) rather than retail's `bnez`-to-body, and merged the
  `else if` into one fallthrough chain with an extra `bne` retail does not
  have.
- **Attempt 2**: rewrote as two independent `if` blocks, the first ending in
  an explicit `return;` (semantically identical to the `else if` for a
  `void` function). Scored IDENTICALLY (14/42, same shape) -- GCC 2.6.3's
  control-flow lowering treats `if(A){...return;} if(B){...}` and
  `if(A){...} else if(B){...}` as the same construct once it reaches this
  stage, so this was not actually a different lever from attempt 1.
- **Attempt 3**: `switch (arg1) { case 2: case 3: ...; break; case 4: ...;
  break; }`. This matched 42/42 immediately. A `switch` lowering for a
  small case set produces exactly retail's "test true jumps INTO the case
  body, explicit jump PAST all bodies for no match" shape -- the two
  adjacent case labels sharing one body is what generates the `arg1 < 4`
  range test (GCC already knows `arg1 >= 2` is established going in) instead
  of an `==` chain.

### Proposed learning

When a residue shows retail branching TOWARD a body on the taken branch
(rather than skipping around it) AND an explicit unconditional jump past
ALL bodies for the unmatched case, suspect a `switch` even if the case
values look like they would fit an `if`/`else if` chain just as well --
under this GCC, the two source forms are NOT equivalent at the RTL level
once integer promotion/multiple-case-labels are involved, even though an
`if`/`else if` chain and a `return`-terminated sequential-`if` chain (which
ARE semantically identical for a void function) compile IDENTICALLY to each
other. Don't spend a second attempt re-testing that particular pair.

## Naming

- `TextEntry__SetState` -- tier B. gTextEntryMethods +0x054 (setState slot, classtable.py). arg1-driven: zeroes closeTickCount; arg1<2 no-ops; arg1 in {2,3} detaches childType2, releases card resources, records closeState=arg1; arg1==4 notifies parents with closeState. Mechanics fully traced; the game meaning of the 2/3/4 codes (a close/commit sequence for the name-entry UI) is inferred from data flow, not confirmed by any string or external caller.

## Track 4 (2026-09-26, round 87)

Class unified as `TextEntry` (include/text_entry.h; table gObj86ED0Methods -> gTextEntryMethods, type Obj86ED0 -> TextEntry). The class name is for what its methods do: setText keeps a caller's string buffer and a working copy, the cursor and char methods edit the copy, command 25 writes it back, 23 closes without writing (banner of include/text_entry.h). Fields renamed from their accessors: unk14 charCount, unk1C charIndex, unk20 altCommands, nameLen textLen, nameBuf textBuf, unk28 editBuf, unk40 cursorSprite (CharSprite *), unk44 textRow, unk48 panelSprite (ScreenSprite *). Zero bytes changed.

## Track 7 (2026-09-27, round 98, bravo)

Parameter `arg1` -> `state`. States 2/3 are `TEXTENTRY_RESULT_ACCEPTED`/
`CANCELLED`, 4 is `TEXTENTRY_STATE_REPORT` (new in include/text_entry.h:
notify the parents with closeState). Zero bytes changed.
