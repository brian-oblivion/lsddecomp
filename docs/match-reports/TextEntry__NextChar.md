# TextEntry__NextChar -- MATCH (25/25 words, 2 real attempts after the header fix)

> Renamed from `Obj86ED0__AdvanceCharSelect` on 2026-09-26 (tools/rename.py). Address 0x80051720.

> Renamed from `func_80051720` on 2026-09-24 (tools/rename.py). Address 0x80051720.

Unit `TextEntryItemList`. Obj86ED0's own "advance `unk1C` counter, clamped at
`unk14`, dispatch `slotA8` with `unk18`" method. Same family as
`TextEntry__MoveCursorRight`/`TextEntry__MoveCursorLeft` (increment/decrement clamp pairs on
`unk18`), but on a different pair of fields (`unk1C`/`unk14`) and resetting
to 0 rather than restoring the old value on overflow.

```c
void TextEntry__NextChar(Obj86ED0 *self)
{
    s32 v;

    if (self->unk48 != NULL) {
        v = self->unk1C + 1;
        self->unk1C = v;
        if (v < self->unk14) {
            self->methods->slotA8(self, self->unk18, v, 1);
        } else {
            self->unk1C = 0;
        }
    }
}
```

Note the store (`self->unk1C = v;`) is UNCONDITIONAL, written before the
`if` -- the exact same shape as `TextEntry__MoveCursorRight`/`TextEntry__MoveCursorLeft`'s clamp
idiom, not something written differently inside the true branch. That part
was right from the very first attempt.

## The actual defect: arity, not placement

`Obj86ED0Methods::slotA8` was declared with 2 params after `self`
(matching its sibling `slotA4`). The retail call sets `$a1`
(`self->unk18`) and `$a3` (`1`), while `$a2` is never freshly loaded or
`li`'d for the call -- it's still holding the just-computed incremented
`unk1C` value from a few instructions earlier. Same "leftover register is
actually a forwarded argument" shape as `TextEntry__PlaySound`'s `slot80` fix
earlier this round. The first attempt (correct store placement, but the
call written as `self->methods->slotA8(self, self->unk18, 1)` against the
stale 2-arg header) scored only 5/25, with a register swap AND a length
mismatch, because the compiler had no reason to keep `v` alive into the
call at all.

Retyped `slotA8` to `(Obj86ED0 *self, s32 arg1, s32 arg2, s32 arg3)` in
`include/class_3bb8c.h` and changed the call to
`self->methods->slotA8(self, self->unk18, v, 1)`.

**One wrong turn on the way there, worth recording so it isn't retried:**
applying the arity fix while ALSO moving the store from "before the `if`"
to "first statement of the true branch" (reasoning, by analogy with a
different function in this same batch, that the store needed to be visibly
tied to the branch since `v` is now a live call argument) made the score
*worse* (11/25), and stacking a `__asm__("")` scheduling barrier on top of
that shape made it worse again (9/25). Reverting the placement back to
"unconditional store before the `if`" -- while KEEPING the arity fix --
matched immediately (25/25). The store's position in the branch's delay
slot is retail's scheduler doing its own job on straight-line code; it
does not track which C-level branch "owns" the store.

### Proposed learning

Don't reach for `__asm__("")` scheduling barriers as an incremental probe
when a score gets WORSE after a change -- that's a signal the immediately
preceding change (not a missing barrier) was the wrong direction. Revert
the last structural change first and re-measure before adding anything.
Here, undoing the store's relocation (not adding a barrier) was what
fixed it.

## Naming

- `TextEntry__NextChar` -- tier B. gTextEntryMethods +0x090 (advanceCharSelect slot, classtable.py -- HandleCommand's case 18/2). Increments the character-picker index unk1C, bounded by unk14 (sNameCharTable's own length, counted by the ctor); WRAPS to 0 on overflow (unlike the cursor pair's revert), forwarding to TextEntry__SetCharAt (TextEntryItemList). Tier B: the wrap-vs-revert asymmetry is measured, exact on-screen semantics (cycling a soft-keyboard character list) is inferred.

## Track 4 (2026-09-26, round 87)

Class unified as `TextEntry` (include/TextEntry.h; table gObj86ED0Methods -> gTextEntryMethods, type Obj86ED0 -> TextEntry). The class name is for what its methods do: setText keeps a caller's string buffer and a working copy, the cursor and char methods edit the copy, command 25 writes it back, 23 closes without writing (banner of include/TextEntry.h). Fields renamed from their accessors: unk14 charCount, unk1C charIndex, unk20 altCommands, nameLen textLen, nameBuf textBuf, unk28 editBuf, unk40 cursorSprite (CharSprite *), unk44 textRow, unk48 panelSprite (ScreenSprite *). Zero bytes changed.

Renamed from Obj86ED0__AdvanceCharSelect: charIndex + 1, written through setCharAt while below charCount, else wrapped to 0 without writing. Tier B.

## Track 7 (2026-09-27, round 98, bravo)

Local `v` -> `next`. Zero bytes changed.
