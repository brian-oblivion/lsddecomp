# TaskCore__SetState — MATCHED (byte-exact, whole-image `build exit=0`)

> Renamed from `Obj86B60__SetState` on 2026-09-25 (tools/rename.py). Address 0x8003c63c.

> Renamed from `func_8003C63C` on 2026-09-24 (tools/rename.py). Address 0x8003c63c.

Unit: `Task` · Size: 100 instructions · Round 23 (2026-09-07), head.
Second of the five `REOPENED -- ASSIGNABLE` functions to be closed, sibling of
`TaskCore__OnPadEvent`.

## History

Filed round 2026-09-02 (runner echo) as **`addiu_at` toolchain blocked**, never
attempted — its dispatch is `addiu $at, $at, %lo(jtbl_800110D0)`. That verdict
was correct when written and expired when round 21 resolved `addiu_at`. See
`TaskCore__OnPadEvent.md` for the shared history; the jtbl-is-not-an-exception
discriminator this unit established still stands.

## The match

```c
void TaskCore__SetState(Obj86B60 *self, s32 a1)
{
    Obj86B60Methods *methods;

    methods = self->methods;
    GetIntermediateBaseMethods()->slot60(self, a1);
    switch (a1) {
    case 5:
        methods->slotE4(self, self->unk4C->unk10);
        methods->slotF0(self, self->unk4C->unk8, 0);
        self->unk1C = 0;
        self->unk3C = 1;
        break;
    case 6:
        self->unk38 = 1;
        methods->slot94(self);
        break;
    case 4:
    case 7:
        self->unk1C = 0;
        self->unk3C = 0;
        break;
    case 8:
        self->unk1C = 0;
        break;
    case 9:
    case 0xA:
    case 0xB:
    case 0xE:
    case 0xF:
    case 0x10:
    case 0x11:
        self->unk20 = 5;
        self->unk1C = 0;
        switch (a1) {
        case 0xB:
            methods->slot90(self);
            break;
        case 0xF:
            methods->slot10C(self);
            break;
        case 0x11:
            methods->slot110(self);
            break;
        }
        break;
    }
}
```

An event handler: forwards to the shared `IntermediateBase` `slot60` first, then
switches on the event code `[4, 0x11]` through `jtbl_800110D0` (14 entries).

Two structural readings worth recording because neither is guessable from the
control flow alone:

- **The nested `switch` is real, and the tell is `slti`.** Codes 9, 0xA, 0xB,
  0xE, 0xF, 0x10 and 0x11 all land on ONE outer arm that sets `unk20 = 5` and
  `unk1C = 0`, and then three of them additionally call a method. That inner
  three-way dispatch compiles to a BALANCED TREE — `beq 0xF` (the median) first,
  then `slti ..., 0x10` splitting into `beq 0xB` and `beq 0x11`. An
  `if / else if / else if` chain would emit three sequential `beq`s and no
  `slti`. **A range-split compare in the middle of what looks like a compare
  chain means `switch`, not `if`.**
- **Outer case order is 5, 6, 4/7, 8, group** — not numeric. Read off the arm
  block addresses per the source-order lever in `TaskCore__OnPadEvent.md`.

## The residue was a TYPE error in the header, not scheduling

The first attempt was three words long, with every instruction and every arm
address otherwise exact. The whole difference: retail cross-jumps **all four**
method calls (`slot94`, `slot90`, `slot10C`, `slot110`) into ONE `jalr $v0` site;
the attempt merged them **2 + 2**, giving the `slot90` arm its own `nop`/`jalr`/
`move $a0,$s0` and an explicit `j` to the epilogue.

The split fell exactly along the declared return types: `slot90`/`slot94` were
`void`, `slot10C`/`slot110` were `s32`. **GCC 2.6.3 will not cross-jump a `void`
call against a value-returning call whose result is discarded** — the call RTL
differs (`(call …)` versus `(set (reg) (call …))`) even though the emitted
instructions are identical — so the four calls formed two mergeable pairs
instead of one group of four.

Retyping `Obj86B60Methods::slot10C` and `::slot110` from `s32` to `void` closed
it with no other change.

**The retype was justified independently of the match, which matters because
this is a shared-header vtable slot edit** (`include/Task.h`, six units):

- `slot110`'s occupant `TaskCore__CancelElementScroll` is **already matched** in
  `src/app/Task.c` as `void TaskCore__CancelElementScroll(Obj86B60 *self)`. That is positive
  evidence, not inference from the function under test.
- Neither slot had any other caller anywhere: the header's own comment recorded
  both as `OBSERVED: TaskCore__SetState (STALL, not attempted)` — i.e. the `s32` had
  been read off the disassembly of a function nobody had ever compiled.
- Whole-image SHA1 green after the retype, which is the only thing that can see
  a slot retype breaking another unit's already-matched codegen.

### Proposed learning

**A vtable slot's return type inferred from an UNATTEMPTED function's
disassembly is a guess with no evidence behind it, and it can be the thing that
blocks the match.** A discarded return value is invisible in the bytes — the
runner prompt already says the byte match tells you nothing about the return
type of a tail call — so a slot annotated `OBSERVED: <fn> (STALL, not
attempted)` is annotated with a *hypothesis*. When such a slot is in the
function you are matching, treat its type as an open variable, and prefer
evidence from the slot's OCCUPANT (which may already be matched as C) over the
call site.

**And the discriminator that makes this diagnosable rather than a guessing game:
GCC 2.6.3 cross-jumps calls only when the return-value RTL matches.** So if N
identical-shaped indirect calls should merge into one site and instead merge
into groups, **the grouping partitions them by declared return type** — a 2+2
split is a TYPE mismatch, not a scheduling residue, and no barrier or reordering
will fix it. Count the groups, line them up against the slot declarations, and
the odd one out is the wrong type. Here: 3 words, one edit, and the header was
wrong rather than the C.

## Naming (round 78, delta)

**Tier A.** `func_8003C63C` -> `Obj86B60__SetState`. Occupies slot60 in
`gTaskCoreMethods`; `gTitleMenuMethods` overrides the SAME slot with the
independently-named `TitleMenu__SetState` (`tools/classtable.py
gTitleMenuMethods`), which is what settles both the slot's role (a
`reason`-coded state-transition entry point, signature `(self, s32 reason)`
matching `Obj86B60Methods::slot60`) and this function's name as the class
family's DEFAULT implementation a sibling class overrides -- not something
`TitleMenu` or `GraphRoomObj` introduces (both keep it: `GraphRoomObj`'s own
table has this exact occupant at slot60, unoverridden). See
`include/Task.h`'s round-78 header comment for the full derivation and
why this rules out a `TitleMenu__`/`GraphRoomObj__` prefix.

## Proposed field names (round 78, delta -- NOT applied, cross-unit)

`Obj86B60::unk3C` (s32, +0x03C) -> `notifyMode`. Tier B. Grep shows
`Task.c`, `code_2cc8c_d.c` and several unrelated `class_3bb8c_*`/
`libsnd_seqread.c`/`Task.c` files also contain an `unk3C` textual hit, so
per CLAUDE.md's "textual search over-counts" warning this is NOT renamed in
the shared header -- only the compiler (a definition-only rename + rebuild)
can settle which of those are the SAME struct. Evidence for the name from
this unit alone: `TaskCore__SetState` sets it to 0 or 1 (cases 4/7 and 5);
`TaskCore__OnPadEvent` gates its whole body on `!= 0`;
`TaskCore__OnPadConfirm`/`TaskCore__OnPadCancel` compare it `==1`/`!=1`;
`TaskCore__OnPadPrev`/`TaskCore__OnPadNext` branch `==1` vs `==2`. Reads as a small mode
enum selecting which of two/three notify-handling paths applies; "notifyMode"
describes that mechanic without asserting which in-game states 1/2 are.

**UPDATE (round 78, delta, same session):** echo posted a competing proposal
for this exact field on the broadcast -- `unk3C -> scrollState` -- from
Task, with more context than this report has alone (that unit's own
`unk4C -> target`/`Unk4CObj::unk24 -> slotEntries` proposals, and functions
named `BeginElementScroll`/`SetTarget`, suggest `Obj86B60`'s `unk4C` target
is a scrollable list and `unk3C` may be that scroll's own state). Echo's
proposal is the better-corroborated one (cross-function agreement across two
units' worth of naming rather than this unit's alone) and should win at
merge if the head applies either -- this report's own `notifyMode` evidence
above stands as independent confirmation of the VALUE SET (0/1/2, gating
notify-handling) but not of the name. Do not apply both.


**Head disposition, round 78.** `unk3C` DECLINED (see TaskCore__BeginElementScroll.md): two competing readings, value range unconfirmed.

## Track 4 (2026-09-25, round 84, alpha)

Renamed from Obj86B60__SetState (tools/rename.py): the class prefix. Occupant of +0x060 (`setState`), named for the slot. The class (id 0x130, table gTaskCoreMethods) is unified as `TaskCore` in `include/TaskCore.h`; `self` is `TaskCore *` (it was the `Obj86B60` or `StreamTaskObj` view). Any source block above is the pre-unification spelling; the live body takes the unified types and slot names, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 7 (round 98, alpha)

Every case is now a TaskCoreState member (include/TaskCore.h, added this
round; evidence on each member's comment) and inputMode's values are
TaskCoreInputMode. States 9..17 are what the handlers and the slot methods
report (setState hands each to notifyParents first); this function folds
them all back into ACTIVE and runs the follow-up for three of them.
Byte-identical.

## Track 10 (2026-09-28, round 104, echo)

TaskCore fields renamed (include/TaskCore.h): `unk2C` -> `maxPackets` (the value onInit passes to the viewport's setMaxPackets), `unk34` -> `clearOnDeinit` (onDeinit clears the screen only while it is nonzero), `unk93` -> `clearColor` (setColors' `clear` argument, the colour onDeinit clears to); TaskCoreTarget `unk8` -> `initialSlot` (setState(ACTIVE)'s setActiveSlot argument). Byte-identical (whole image green). The 300/400 packet counts stay literal: they are per-class tuning values beside the field that names them, like fadeRate and otLength.
