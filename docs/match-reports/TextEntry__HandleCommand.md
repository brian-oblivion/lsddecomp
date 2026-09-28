# TextEntry__HandleCommand -- MATCHED (147/147 words, byte-exact)

> Renamed from `Obj86ED0__HandleCommand` on 2026-09-26 (tools/rename.py). Address 0x800513d0.

> Renamed from `func_800513D0` on 2026-09-24 (tools/rename.py). Address 0x800513d0.

Unit `TextEntryItemList`. `Obj86ED0Methods::slot5C` (called by `TextEntry__OnNotify`'s
`tag==2` case). No other caller in this unit. Carved with the rodata slot
(`jtbl_80011628` at file `0x1E28`) already attached per the splat yaml, so no
carve work needed here.

**`addiu_at` is NOT a live blocker for this function.** It was filed blocked
at carve time, but `addiu_at` was resolved in round 21 (`maspsx --addiu-at`).
Screened clean on both live checks (no `gp_rel` hit, no forward
`mflo`/`mfhi`-into-`mult`/`div` hit) before starting.

## What it does

A dense `switch` on `arg2` (range 2..32, retail's own `sltiu $v0,$a2,0x1f`
after `$a2 -= 2` proves the bound), dispatching through `self->methods`:

- `case 25`: copies `self->unk24` into `self->unk28` via `EncodeFullWidthSjis`
  (when `self->unkC == 1`) or plain `strcpy` (otherwise), then
  `self->methods->slot60(self, 0x10)` and `self->methods->slot54(self, 2)`.
- `case 23`: the same two calls, `slot54`'s second argument `3` instead of
  `2`.
- `case 32`/`31`/`28`: unconditional `self->methods->slotA0`/`slot9C`/
  `slot98(self)`.
- `case 21`/`5`, `20`/`4`, `18`/`2`: each pair gates on `self->unk20` with
  OPPOSITE polarity (`21`/`20`/`18` fire when `unk20 == 0`; `5`/`4`/`2` fire
  when `unk20 != 0`) before calling the SAME slot (`slot88`/`slot8C`/
  `slot90(self)` respectively).
- `case 19`/`3`: same opposite-polarity-on-`unk20` shape, but retail
  genuinely SHARES the resolve+call code between them (case 19's positive
  branch jumps directly into case 3's fallthrough body) -- the only one of
  the four same-shaped pairs where this happens. Reproduced with an explicit
  `goto` into a label positioned inside case 3's body, matching retail's own
  branch target exactly (see below).
- Any other value (including every value in [2,32] with no case above):
  `default: return;`, matching retail's shared bounds-check/table fallback
  destination.

`arg1` (the method's own 2nd parameter, per `slot5C`'s already-established
signature) is read nowhere in this function -- confirmed dead in retail's
own register usage, not merely unused in this reading.

## Case ORDER, read directly off the jump table

Per `docs/DECOMPILATION_LEARNINGS.md`'s dense-switch case-order rule (source
order, not case-value order), sorted the 13 real arm labels in
`jtbl_80011628` by address:

```
8005140C(25) 8005147C(23) 800514B4(32) 800514C8(31) 800514DC(28)
800514F0(21) 80051514(5)  80051538(20) 8005155C(4)  80051580(18)
800515A4(2)  800515C8(19) 800515E0(3)
```

`default` was written FIRST in the C (a distinct clause is required --
without one, values inside [2,32] that map to the table's own default slot
would fall through to nothing, which is wrong -- and it must NOT be textually
last, since case `3` has to be the physically last arm to get its
fallthrough-based epilogue convergence right, per the case-order rule's own
"the falling-through arm is last in source order" corollary).

## The one real lever: avoid an UNWANTED cross-jump merge, not a wanted one

The first attempt (identical logic, but resolving each slot into a shared
local `void (*fn)(Obj86ED0 *self);` variable and using `break;` to reach one
`fn(self);` call after the `switch`) built CLEAN but **9 words SHORT and
badly drifted** (85/138, retail 147). Diffing showed GCC had cross-jump-
merged `case 21`'s "else" arm directly into `case 5`'s own resolve code --
turning `case 21`'s `bnez` into a `beqz` branching INTO case 5's block,
eliminating case 21's own copy of `fn = self->methods->slot88;` entirely.
Retail does NOT do this: both `case 21`'s and `case 5`'s full 9-word
guard+resolve+jump blocks are present, separately, in the `.s`, even though
they are byte-identical except for branch polarity.

This is documented (`docs/DECOMPILATION_LEARNINGS.md`, "Splitting a fused
boolean condition... / a crossjump-mergeable tail needs the full call
statement written out in each branch rather than deferred through a
function-pointer local, when the branches share one slot with different
arguments" -- `TaskObjF__CheckCardStatus`/`TaskObjF__TickStateDelay`) from the OPPOSITE direction
(wanting a merge that wasn't happening); this function needed the same lever
applied to PREVENT an unwanted one. **Fix: drop the `fn` variable entirely
and write the full call inline in each case** (`self->methods->slot88(self);
return;` instead of `fn = self->methods->slot88; break;`). This alone took
the function to 147/147 byte-exact on the very next build -- GCC still
shares the actual `jalr $v0` call site across every qualifying case (that
part of the convergence is real and retail-confirmed), but no longer treats
each case's PRECEDING resolve code as a mergeable duplicate of another
case's.

A bare `__asm__("");` barrier, tried first (both at the end of the resolve
and at the top of the guarded cases), had **zero effect** on the merge --
consistent with `docs/DECOMPILATION_LEARNINGS.md`'s standing finding that
cross-jump operates at the basic-block level and a scheduling barrier has
nothing to bite on; the fix has to remove the RTL identity, not suppress the
pass.

## Header changes made this round (`include/class_3bb8c.h`)

All additive, `Obj86ED0Methods`, no existing field removed:

- Split the existing `pad060[0x0A4 - 0x060]` (0x44 bytes) into:
  `slot60` (`void (*)(Obj86ED0 *, s32)`, this function's own `slot60(self,
  0x10)` calls) + `pad064[0x088 - 0x064]` + seven single-`self`-argument
  slots `slot88`/`slot8C`/`slot90`/`slot94`/`slot98`/`slot9C`/`slotA0`
  (all `void (*)(Obj86ED0 *)`, confirmed identical shape directly off the
  shared call site: `jalr $v0; addu $a0,$s0,$zero`, no other register set).
  Total preserved: `4 + 0x24 + 4*7 = 0x44`.
- `slot5C`'s own comment updated (removed the stale "STALLED,
  addiu-$at/jump-table blocked" note -- `addiu_at` was resolved in round 21
  and this function is now matched).

No existing declaration's TYPE changed.

## Proposed learnings

- **A local function-pointer variable used only to defer a call past a
  `switch`'s cases (`fn = ...; break; ... fn(self);`) makes every
  resolve-then-break arm a maximally trivial cross-jump target, even for
  arms retail keeps separate.** Writing the full call directly in each arm
  (accepting that the trailing `jalr`/return sequence will still legitimately
  converge to one physical site on its own) avoids merging the PRECEDING
  resolve code that a shared-variable indirection tends to invite. Screen for
  this whenever a "resolve into a variable, call once at the end" pattern
  produces a build that is SHORTER than retail's declared length with the
  first divergence showing a flipped branch polarity into a later,
  structurally identical case.

## Naming

- `TextEntry__HandleCommand` -- tier B. gTextEntryMethods +0x05C (handleCommand slot, classtable.py), dispatched by TextEntry__OnNotify's tag==2 case. Dense switch on a command code (2..32): case 25 commits the edited name back into nameBuf (Encode/strcpy) then setState(2); case 23 setState(3); the remaining paired cases (21/5, 20/4, 18/2, 19/3, plus ungated 32/31/28), CONFIRMED against gTextEntryMethods's own table (classtable.py), dispatch to moveCursorRight/moveCursorLeft/advanceCharSelect/advanceCountdown/resetAllAndFinish/resetCountdown/toggleFlag20 respectively -- i.e. this is the full input-command router for the name-entry UI (cursor move, character-select cycle, and the countdown/blink group TextEntryItemList already named). Tier B: the routing mechanics and slot identities are now fully confirmed, but which physical button/event produces each numeric code is not.

## Proposed field names

**Head, round 77:** `unk10 -> nameLen` and `unk18 -> cursorIndex` APPLIED by type scope (set from `strlen(arg1)`; inc/dec by the cursor pair, capped by it). The rest remain PROPOSED for the base-class or track 4 pass; `altInputFlag` was self-rated low confidence.

Cross-unit fields/slots of `Obj86ED0` (shared header `include/class_3bb8c.h`)
that this unit's own functions read/write but that TextEntryItemList ALSO
accesses -- left unrenamed per the field-ownership rule (compiler-checked:
TextEntryItemList's `TextEntry__PrevChar`/`TextEntry__ToggleActOnHeld`/
`TextEntry__ResetChar`/`TextEntry__ResetAllChars`/
`TextEntry__SetCursorPos`/`TextEntry__SetCharAt` are the other
accessors). PROPOSED for whoever next runs track 3 on TextEntryItemList, or for
the head to apply by type scope:

- `unk10` -> `nameLen` (tier B). Set by `TextEntry__SetText` from
  `strlen(arg1)` (halved when `mode==1`); read as the upper bound in
  `TextEntry__MoveCursorRight`'s `unk18` test.
- `unk14` -> `charTableLen` (tier B). Set by `TextEntry__TextEntry`'s own hand
  counted walk over `sNameCharTable` until the NUL byte; read as the upper
  bound in `TextEntry__NextChar` and as the countdown's own reset
  value in TextEntryItemList's `TextEntry__PrevChar`.
- `unk18` -> `cursorIndex` (tier B). The name-buffer edit-cursor index,
  moved by `TextEntry__MoveCursorRight`/`Left`, forwarded as `arg1` to both
  `slotA4`/`slotA8` (TextEntryItemList's Dispatch* pair) and to `slot60`
  (`TextEntry__PlaySound`).
- `unk1C` -- **NOT proposed, dual-use ambiguity found and deliberately left
  alone.** In this unit (`TextEntry__NextChar`) it is INCREMENTED as
  a character-picker selection index, capped by `unk14`, wrapping to 0 on
  overflow. In TextEntryItemList (`TextEntry__PrevChar`) the SAME field is
  DECREMENTED as a countdown, resetting to `unk14` at 0 -- opposite
  direction, same bounds. Whether this is one overloaded "position in
  [0,charTableLen)" value serving both a picker index and a blink/highlight
  timer, or two genuinely different concerns sharing a slot by coincidence,
  is not established from either unit's own call sites alone. A wrong name
  here would assert a purpose the evidence does not support in one of the
  two units, so it stays `unk1C` until someone reads both usages together
  against the caller of `TextEntry__HandleCommand`/`TextEntry__OnNotify` (i.e.
  whatever drives the name-entry screen's per-frame tick) to settle it.
- `unk20` -> tentatively `altInputFlag` (tier C-ish B; NOT confident enough
  to apply even if it were unit-exclusive). Toggled by TextEntryItemList's
  `TextEntry__ToggleActOnHeld` (`^= 1`), zeroed by `TextEntry__AttachTarget`,
  tested with OPPOSITE polarity by every paired case in this function's own
  switch (`21` vs `5`, `20` vs `4`, `18` vs `2`, `19` vs `3`). Mechanics are
  solid; which real input condition it represents (a mode key held down? an
  edit-vs-insert toggle?) is not established -- offered as a starting point,
  not a confident proposal.
- `unk28` -> `workName` (tier B). `BMemPMgrAlloc`'d by `TextEntry__TextEntry`,
  freed by `TextEntry__Finalize`; the decode/copy destination in
  `TextEntry__SetText` and the encode SOURCE in this function's own `case 25`;
  also the per-index byte array TextEntryItemList's `TextEntry__SetCharAt`
  writes through `sNameCharTable`. Working (half-width-decoded) copy of the
  name the caller supplies via `nameBuf`.
- `unk40`/`unk44`/`unk48` -> `iconRes`/`fontRes`/`inputRes` (tier B).
  Resolved in `TextEntry__LoadCardResources`: `unk48` from the `COMINPUT.TIM`
  handle (also the readiness/"is attached" gate every other function in
  this unit tests), `unk44` from `New_TextRow` on the `FONTICON.TIM`
  handle with `nameLen`/`workName` forwarded (renders the name text), `unk40`
  from `New_CharSprite` on the same `FONTICON.TIM` handle with a `0x5F`
  literal (a distinct icon sub-resource of the same TIM). Also accessed by
  TextEntryItemList's Dispatch* pair (`unk40`/`unk44` only, not `unk48`).
- `slot60` -> `notifyTarget` (tier B). This IS `TextEntry__PlaySound`'s own
  vtable slot (`gTextEntryMethods +0x060`, classtable.py) -- dispatched by
  this function's `case 25`/`23` (arg1=`0x10`) and by TextEntryItemList's
  `TextEntry__SetCursorPos`/`TextEntry__SetCharAt` (arg1=`0`).
- `slotA4`/`slotA8` -> `dispatchIndexValue`/`dispatchLookupValue` (tier A --
  these names ALREADY EXIST as the confirmed implementations'
  `TextEntry__SetCursorPos`/`TextEntry__SetCharAt`,
  TextEntryItemList, round 45/75). Dispatched by this unit's own
  `TextEntry__MoveCursorRight`/`Left`/`TextEntry__NextChar` AND by
  TextEntryItemList's `TextEntry__ResetAllChars`/`TextEntry__ResetChar`.
  Naming the slot to match its implementation is the project's own stated
  convention; only left unapplied here because the accessor set spans two
  units' files.

Posted to `tools/broadcast.sh` for whoever is next on TextEntryItemList.

## Track 4 (2026-09-26, round 87)

Class unified as `TextEntry` (include/TextEntry.h; table gObj86ED0Methods -> gTextEntryMethods, type Obj86ED0 -> TextEntry). The class name is for what its methods do: setText keeps a caller's string buffer and a working copy, the cursor and char methods edit the copy, command 25 writes it back, 23 closes without writing (banner of include/TextEntry.h). Fields renamed from their accessors: unk14 charCount, unk1C charIndex, unk20 altCommands, nameLen textLen, nameBuf textBuf, unk28 editBuf, unk40 cursorSprite (CharSprite *), unk44 textRow, unk48 panelSprite (ScreenSprite *). Zero bytes changed.

## Track 7 (2026-09-27, round 98, bravo)

The commands are Pad events (include/pad.h: `PAD_EVENT_PRESSED` 0x12 /
`PAD_EVENT_HELD` 0x02 plus the button index), and the command's sender is
the child of class 2, the Pad (onNotify). Every case label is now spelled
that way, zero bytes changed:

| value | event | action |
| --- | --- | --- |
| 25 | PRESSED + RRIGHT (circle) | write back, sound, close ACCEPTED |
| 23 | PRESSED + RDOWN (cross) | sound, close CANCELLED |
| 32 | PRESSED + L2 | resetAllChars |
| 31 | PRESSED + L1 | resetChar |
| 28 | PRESSED + SELECT | toggleAltCommands |
| 21 / 5 | PRESSED / HELD + LRIGHT | moveCursorRight |
| 20 / 4 | PRESSED / HELD + LLEFT | moveCursorLeft |
| 18 / 2 | PRESSED / HELD + LUP | nextChar |
| 19 / 3 | PRESSED / HELD + LDOWN | prevChar |

So `altCommands` selects between acting on presses (clear) and on held
buttons (set); the field-name proposal is below. `setState(2)`/`(3)` are
`TEXTENTRY_RESULT_ACCEPTED`/`CANCELLED`; the sound `0x10` is `1 << 4`,
VAB program 1 tone 0 (playTone keys program index >> 4, tone index & 0xF;
TitleMenuTaskObjF/_t's spelling). The `slot94Call` label is `callPrevChar`.
Parameters `arg1`/`arg2` -> `sender`/`command`.

Moved here from the comment on `EncodeFullWidthSjis` (stale: it is matched
in screen_widgets.c since round 38): "TextEntry__HandleCommand's own
name-copy helper -- uncarved elsewhere (`screen_widgets`, still
`INCLUDE_ASM`), typed purely from this call site's own register usage:
`a0`/`a1` are `self->textBuf`/`self->editBuf` (both `char *`, the same pair
`strcpy` is fed in the other arm), return value unused. Same
declare-locally convention as `DecodeFullWidthSjis` above (a different unit
types this same-shaped function with a different signature from its own
call site)."

### Proposed field names (round 98)

- TextEntry +0x020 `altCommands` -> `actOnHeld` (accessors: this unit's
  AttachTarget/HandleCommand, TextEntryItemList's ToggleAltCommands); slot
  +0x098 `toggleAltCommands` and `TextEntry__ToggleActOnHeld` would follow
  (`toggleActOnHeld`). Evidence: the table above.

## Round 98: proposals applied

- slot +0x060 `notifyTarget` -> `playSound` (prototype `arg1` -> `tone`):
  its occupant is TextEntry__PlaySound, `playTone(target, tone, 96, 96)` on
  the attached VabStreamObj; this body calls it with `1 << 4` on accept and
  cancel, SetCursorPos/SetCharAt with 0.
- field +0x020 `altCommands` -> `actOnHeld`: every arrow case below runs on
  PAD_EVENT_PRESSED when it is 0 and on PAD_EVENT_HELD when it is set;
  AttachTarget zeroes it.
- slot +0x098 `toggleAltCommands` -> `toggleActOnHeld`, occupant renamed by
  rename.py to TextEntry__ToggleActOnHeld (`actOnHeld ^= 1`); SELECT's press
  calls it.

Accessors were the compiler's error list (TextEntryItemList, class_3bb8c_j);
zero bytes changed.
