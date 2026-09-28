# TaskObjF__OnTextEntryResult -- MATCH

> Renamed from `TaskObjF__OnCommand` on 2026-09-26 (tools/rename.py). Address 0x800504d0.

> Renamed from `Class86E00_3bb8c_g__OnCommand` on 2026-09-23 (tools/rename.py). Address 0x800504d0.

> Renamed from `func_800504D0` on 2026-09-23 (tools/rename.py). Address 0x800504d0.

Unit `TitleMenuTaskObjF`, round 14. `./build-and-verify.sh` exit 0; whole-image
SHA1 matches retail. `funcdiff.py TaskObjF__OnTextEntryResult`: 54/54 words match.

## Source

```c
void TaskObjF__OnTextEntryResult(Class86E00_3bb8c_g *self, void *arg1, s32 arg2)
{
    switch (arg2) {
    case 2:
        self->methods->slotA0(self);
        self->methods->slot78(self, self->unk40, self->unk44, self->unk48,
                               self->unk4C, self->unk50, self->unk54, self->unk58);
        break;
    case 3:
        self->methods->slotA0(self);
        self->methods->slot7C(self, 0x17);
        break;
    }
}
```

`arg1` (`$a1`) is never read anywhere in the body -- confirmed genuinely
unused. `self->unk4C` (a `u8` field) is passed to `slot78` and integer-
promoted to a full word automatically, matching the call site's own
`lbu` + full-word stack store.

## One near-miss: `if`/`else if` versus `switch`

**First attempt (8/54, badly drifted):** wrote the two-value dispatch as
`if (arg2 == 2) {...} else if (arg2 == 3) {...}`. This compiled to
INVERTED branch polarity relative to retail -- a `bne`-to-skip layout with
each case's body placed INLINE, right after its own check -- where retail
uses `beq`-to-jump layout with both case bodies placed OUT OF LINE, after
both checks, reached by forward branches (`beq $a2,2,CASE2` /
`beq $a2,3,CASE3` / `j END`). A plain `switch` with `case 2:`/`case 3:`
reproduced retail's out-of-line layout exactly on the first try. This is
the same "GCC 2.6.3 lays out case bodies in TEXTUAL SOURCE ORDER" rule
already documented, just as a reminder that a `switch` and its "equivalent"
`if`/`else if` chain are NOT interchangeable at the LAYOUT level even when
every value and branch target matches logically.

## Struct changes (additive, `include/class_3bb8c.h`)

None new -- `slotA0`, `slot78`, and `slot7C` were all already declared
while surveying the unit (`TaskObjF__ReleaseCardIcon`'s report); this is the function
that exercises `slot78`'s full 8-argument (`self`+7) signature and
confirms `slotA0`'s `(self)`-only arity (called identically from both
cases).

### Proposed learning

**A two-or-three-value dispatch with inline case bodies is a case where
`if`/`else if` and `switch` reliably diverge in LAYOUT, not just in
comparison order.** The existing rule ("GCC lays out case bodies in
source order but picks its own comparison tree") is usually invoked for
comparison ORDER; this instance shows the same rule governing whether the
bodies are placed inline (if/else-if) or out-of-line with forward
branches (switch) even for as few as two values. When a residue's
FIRST diff word is a flipped branch condition (`beq`<->`bne`) with a
completely different offset immediately after a comparison chain, try the
`switch` spelling before reshaping the conditions further.

## Naming

`TaskObjF__OnTextEntryResult` (was `func_800504D0`), tier B: dispatches
on a small externally-supplied code (`arg2` in `{2, 3}`, `arg1` unread) into
`slotA0` plus either the full `slot78` transition or a `slot7C(self, 0x17)`
("force idle", per `TaskObjF__AbortFromState`'s own naming
evidence) -- read as an external caller telling this object to act (2:
proceed / 3: cancel), though nothing in the body itself says who calls it
or what the two codes represent in the game.

## Track 4 (2026-09-26, round 89)

Renamed from `TaskObjF__OnCommand`. Slot +0x0A4, which TaskObjF__OnNotify calls for a sender of class id 0x10, i.e. the TextEntry, whose setState(4) notifies its parents with closeState 2 (the edit was written back) or 3 (include/TextEntry.h). 2 detaches it and calls beginSave (+0x078) again with the edited title; 3 detaches it and sets state 0x17.

## Track 7 (2026-09-27, round 95)

Parameters `arg1`/`arg2` -> `sender`/`result`; the cases are
`TEXTENTRY_RESULT_ACCEPTED`/`_CANCELLED` (enum TextEntryResult, added to
include/TextEntry.h this round: command 25 writes `editBuf` back and closes
with 2, command 23 closes with 3), the target ABORTED. Image byte-identical.
