# ObjM__DispatchPadEvent

> Renamed from `ObjM__DispatchEvent` on 2026-09-26 (tools/rename.py). Address 0x80053358.

> Renamed from `func_80053358` on 2026-09-24 (tools/rename.py). Address 0x80053358.

**Unit:** class_3bb8c_l · **Size:** 38 words (0x98 bytes) ·
**Status: MATCHED 38/38**, whole-image SHA1 green.

## What it does

An event dispatcher: given a numeric `eventId`, calls one of four vtable
slots on `self` (or does nothing if `self->unk68` is zero, or the event
isn't one of the four recognised values).

```c
void ObjM__DispatchPadEvent(Obj87034_3bb8c_l *self, void *arg1, s32 eventId) {
    Obj87034Methods_3bb8c_l *m = self->methods;
    void (*fn)(Obj87034_3bb8c_l *);

    if (self->unk68 == 0) {
        return;
    }
    if (eventId == 0x16) {
        goto case_c8;
    }
    if (eventId < 0x17) {
        if (eventId == 0xC) {
            goto case_c0;
        }
        return;
    }
    if (eventId == 0x21) {
        goto case_74;
    }
    if (eventId == 0x2C) {
        goto case_c4;
    }
    return;
case_74:
    fn = m->slot74;
    goto call;
case_c0:
    fn = m->slotC0;
    goto call;
case_c8:
    fn = m->slotC8;
    goto call;
case_c4:
    fn = m->slotC4;
call:
    fn(self);
}
```

## This one took real work — record why

The event-ID comparisons (`0xC`, `0x16`, `0x21`, `0x2C`) are sparse and
irregular, and retail is **not** a dense `switch` — it's already screened
clean of `%lo(jtbl_*)`/`addiu $at` per this unit's carve-time blocker scan,
and this confirms it structurally: four independent `beq`/`slti` compares,
no jump table.

Three source forms were tried before this one matched, in order:

1. **A plain `switch (eventId) { case 0xC: ...; }`.** GCC lowered the
   sparse switch in ASCENDING VALUE order (`0xC, 0x16, 0x21, 0x2C`).
   Retail's actual comparison order is `0x16` first, THEN a `< 0x17`
   split into `{0xC}` vs `{0x21, 0x2C}` — the switch's case-value order
   has nothing to do with retail's tested order. Also missed hoisting
   `self->methods` before the `unk68` guard (see below). Scored 11/38.
2. **An `if`/`else if` chain in the derived comparison order,** written
   the "natural" way (`if (eventId == 0x16) fn = ...; else if (eventId <
   0x17) { if (eventId == 0xC) ...; else return; } else ...`). This fixed
   the comparison ORDER but every individual `==` test came out with the
   wrong branch polarity (body at fallthrough instead of target, same
   class of residue as `ObjM__GetGridRecord`/`ObjM__PollTimBlockLoad`) AND, worse, GCC
   inlined each case body at its own comparison site instead of placing
   all four bodies out-of-line after the full compare chain the way
   retail does. Scored the same 11/38, different residue shape.
3. **`goto`, one label per case, literal 1:1 transcription of the
   disassembly's control-flow graph** (shown above). This is what
   matched. Every comparison becomes `if (cond) goto case_X;` with the
   SAME polarity/target the retail branch actually uses (no inversion
   puzzle to solve, because a `goto` doesn't get relaid-out by an
   if/else-shape heuristic), and the four case bodies are placed in
   SOURCE label order, which is also retail's OWN body-placement order
   (`slot74`, `slotC0`, `slotC8`, `slotC4`-falls-into-call).

Also required, independent of the above: `self->methods` must be loaded
into `m` **before** the `self->unk68 == 0` early-return check, not after —
retail loads it unconditionally right alongside `self->unk68` itself, even
on the path that returns immediately without using it.

### Proposed learning

**When an `if`/`else`-chain residue involves MORE than two branches (a
real multi-way dispatch, not a single binary choice) and per-branch
polarity inversion isn't converging, stop guessing `if`/`else` shapes and
transcribe the disassembly's control-flow graph directly with `goto` and
one label per target block, in the SAME order the target blocks appear in
retail.** This sidesteps the "which written condition sense reproduces
this specific target/fallthrough" puzzle entirely (a `goto`'s branch
target and fallthrough are exactly what's written, nothing gets
re-derived by an if/else layout heuristic) and lets block ORDER be
controlled directly by label order in the source. Two structural attempts
at the "natural" chain form failed to converge before this; the goto
form matched on the first try. `docs/DECOMPILATION_LEARNINGS.md` already
documents `goto fail;` for the `New_X` allocator idiom — this generalises
it to "any multi-way vtable-slot dispatch with more than two arms".

## Naming

Round 78 (charlie), FINISHING-PLAN track 3.

| was | now | tier | evidence |
| --- | --- | --- | --- |
| `func_80053358` | `ObjM__DispatchPadEvent` | B | see below |

**Evidence.** vtable slot +0x058. A generic `eventId` dispatcher: four recognised numeric codes (0xC, 0x16, 0x21, 0x2C) each forward to one of self's own vtable slots; anything else, or `self->unk68 == 0`, is a no-op.


## Track 4 (2026-09-26, round 89, echo)

Renamed from `ObjM__DispatchEvent` (rename.py): it occupies IntermediateBase's `onPadEvent` (+0x058, onNotify's Pad case), so the codes are pad codes: 0x21 slot74 (TogglePause), 0xC updateCloseReadyFlag, 0x2C clearCloseReadyFlag, 0x16 closeAndNotifyD; only while `inSession`. Tier B (which buttons the codes are is not established).

## Round 95 (track 7, echo)

The four codes are named from include/Pad.h, where this round added
(additively) `PAD_EVENT_HELD/PRESSED/RELEASED` (0x02/0x12/0x22, Pad__DispatchEvents'
edge bases) and `enum PadButton` (sButtonMasks' indices; Pad__LoadButtonTable
copies the fixed table D_80010764, whose words are libetc's masks PADLup,
PADLdown, PADLleft, PADLright, PADRup, PADRdown, PADRleft, PADRright, PADi,
PADj, PADk, PADl, PADm, PADn, PADo, PADh in that order):

| code | spelled | slot |
| --- | --- | --- |
| 0x16 | PAD_EVENT_PRESSED + PAD_BUTTON_RUP (triangle) | closeAndNotifyD |
| 0x0C | PAD_EVENT_HELD + PAD_BUTTON_SELECT | updateCloseReadyFlag |
| 0x21 | PAD_EVENT_PRESSED + PAD_BUTTON_START | togglePause |
| 0x2C | PAD_EVENT_RELEASED + PAD_BUTTON_SELECT | clearCloseReadyFlag |

`code < 0x17` is spelled `code <= PAD_EVENT_PRESSED + PAD_BUTTON_RUP`
(same slti). The goto labels are named for the slot each reaches (were
`case_c8`, `case_c0`, `case_74`, `case_c4`, the slot offsets). A
`/* MATCHING: */` line keeps the goto form from being tidied into a switch
(see above). All byte-identical.
