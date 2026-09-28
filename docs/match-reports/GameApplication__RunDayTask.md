# GameApplication__RunDayTask

> Renamed from `GameApplication__PollStatusObj` on 2026-09-27 (tools/rename.py). Address 0x80026698.

> Renamed from `Class6D3C8__PollStatusObj` on 2026-09-26 (tools/rename.py). Address 0x80026698.

> Renamed from `func_80026698` on 2026-09-24 (tools/rename.py). Address 0x80026698.

**Unit:** game_shell · **Size:** 57 words (0xE4 bytes) ·
**Status: MATCHED 57/57**, whole-image SHA1 green. Closed by the head in
round 8 (2026-09-02).

> **Kept in full below the RESOLUTION.** The derivation that took this from
> 1/57 to 53/57 is sound and its levers are still correct. The two "remaining
> residues" it names, however, were both artifacts of ONE wrong reading, and
> the elaborate switch-lowering theories built on them were explanations for
> something that was not happening.

## RESOLUTION — two misreadings, both in the source, neither in the compiler

```c
    case 3:
        self->unk24 = 1;          /* NOT 3 */
        break;
    ...
    if (outVal != 0) {
        result = (check == 1);    /* NOT (u32)(check ^ 1) < 1 */
    }
```

**1. `case 3` stores 1, not 3 — read the delay slot's effect on the TAKEN
path.** Retail:

```
    beq  s0, v0, 8c      ; status == 3 ?  -> case 3 body at 8c
    li   v0, 0x1         ; DELAY SLOT: runs whether or not the branch is taken
    ...
8c: sw   v0, 0x24(s1)    ; so $v0 is 1 here, NOT the 3 it held for the compare
```

A MIPS delay-slot instruction executes **before** control transfers, so it is
part of the taken path's register state. The old reading took the store's `$v0`
to still hold the comparison literal `3`, concluded retail was doing classic
CSE on it, and — because `= 3` then only matched if the discriminant register
were used — produced both of the reported residues at once:

- the "missing `li v0,0x1` filler" that retail supposedly materialised for an
  unused default arm. **There is no unused constant.** `li $v0, 0x1` is the
  value being stored, hoisted into the delay slot on the one path that needs
  it. Nothing is unused and nothing is being materialised speculatively.
- the "`case 3` store reuses the wrong register (`$s0` instead of `$v0`)"
  register mismatch. With the right stored value the register question does not
  arise: `$v0` is where the 1 already is.

The report's own attempts confirm the reading was the problem rather than the
spelling — `self->unk24 = status;` changed nothing (it is still 3), and the
hand-written nested-if that *did* force `$v0` did so by luck of allocation
while wrecking the branch shape.

**2. `(u32)(check ^ 1) < 1` is just `check == 1`.** GCC 2.6.3 lowers an
equality test against a small constant to `xori` + `sltiu`, so that pair reads
back out of a disassembly as an xor-then-unsigned-compare. Transcribing the
lowering instead of the comparison is arithmetically correct and cost two
instructions. Writing the comparison costs none.

### What this cost, and the general lesson

Two rounds classified this as switch-statement-lowering internals with "no
source-level lever", and recommended a permuter run. A permuter run WAS made
this round (215 → 130, never zero); it found `check = 3; self->unk24 = check;`
and a materialised `1`, both of which were it groping around the real answer
without reaching it. The fix came from re-reading four instructions of retail.

**Two reading rules, both cheap, both would have caught this:**

1. **A delay-slot instruction belongs to the taken path too.** When a branch's
   delay slot writes a register the target block reads, work out the value at
   the TARGET, not at the branch.
2. **Do not transcribe a lowering.** `xori`+`sltiu` is `== k`; `sltu` against
   `$zero` is `!= 0`; a shift pair is a cast. If your C reproduces the
   instruction sequence rather than the expression, you have written the
   compiler's output back into the source and it will usually cost you a word.

## What it does

`GameApplicationMethods` slot `+0x060`. Builds a `StatusObj` (`New_DayTask`,
New_X shape, 0x50 bytes), dispatches `slot44(obj)` (return kept) then
`slot4(obj)` (return discarded -- via the same "delay slot after `jalr`
captures the *preceding* call's return" idiom `GameApplication__RunTask` uses), and
switches on that status: `2` runs `GameApplication__PlayCinematic`, `3` latches
`self->unk24`. Then queries the `DreamSys` status slot again
(`DreamSys__GetCurrentDayAndYear`, the same slot `GameApplication__RunTitleMenu` uses) with an out-parameter
this time, and derives a 0/1 result from both the call's return and the
out-param.

## Best body reached (53/57, preserved below)

```c
#if 0
s32 GameApplication__RunDayTask(GameApplication *self) {
    s32 status;
    StatusObj *obj;
    s32 outVal;
    s32 check;
    s32 result;

    obj = New_DayTask(self->unk1C, self->dreamSys, self->arg->unk04);
    status = obj->methods->slot44(obj);
    obj->methods->slot4(obj);

    switch (status) {
    case 2:
        GameApplication__PlayCinematic(self);
        break;
    case 3:
        self->unk24 = 3;
        break;
    }

    check = self->dreamSys->vt->DreamSys__GetCurrentDayAndYear(self->dreamSys, &outVal);
    result = 0;
    if (outVal != 0) {
        result = (u32) (check ^ 1) < 1;
    }
    return result;
}
#endif
```

This needs `GameApplication.h`'s `StatusObj`/`StatusObjMethods` (already
committed) and the `GameApplication__PlayCinematic` forward declaration (already committed).

## Derivation and the levers that got this from 1/57 to 53/57

The raw disassembly showed (paraphrased): allocate `obj`; call
`obj->methods->slot44(obj)`; call `obj->methods->slot4(obj)`; the **delay
slot of `slot4`'s own `jalr` moves `$v0` into `$s0`** -- and since nothing
writes `$v0` between `slot44`'s return and this point, `$s0` ends up holding
**`slot44`'s** return value, not `slot4`'s (which is discarded). Getting the
call-order/return-source right was step one; the harder problem was register
allocation, in three layers:

1. **A `switch`, not `if`/`else if`, matches retail's direct-branch-to-handler
   shape.** `if (status==2) {...} else if (status==3) {...}` compiles to
   *inverted* guard branches (`bne`, skip-over) that jump PAST the handler
   bodies; `switch` compiles to retail's shape (`beq status,2,handler1;
   beq status,3,handler2; falls through`). This one swap was worth roughly
   30 of the 57 words by itself.
2. **Two named variables with overlapping-looking liveness force separate
   registers, even when the machine-level ranges don't actually overlap.**
   `status` (holds `slot44`'s result, needed across the `slot4` call) and
   `obj` (needed as `slot4`'s argument, dead immediately after) are, at the
   C level, simultaneously "in scope" across the `obj->methods->slot4(obj)`
   statement. Retail's actual object code reuses `$s0` for both anyway --
   `$s0` (`obj`) is fully consumed into `$a0` for the argument *before* the
   `jalr`, so the delay slot is free to immediately repurpose `$s0` for
   `status`. GCC 2.6.3 did **not** perform this reuse for any C phrasing
   tried here (separate variables, reassigning `obj` itself, hoisting the
   shared `slot4(obj)` into every `switch` case so it might get hoisted back
   out -- none worked; the last one only proved GCC 2.6.3 does not do
   cross-case code hoisting for `switch`, since it tripled the emitted code
   instead). This is presumably an instruction-scheduling-level decision
   (which delay slot filler is "available") rather than something a source
   rewrite reaches, matching the class of residue in `New_GameApplication` and
   the two instances the head's round-3 broadcast #3 already confirmed.
3. **Reusing the SAME variable name across two semantically-unrelated
   purposes forces GCC to keep it alive across everything in between.**
   The second `DreamSys` status query originally reused the `status`
   variable (`status = self->dreamSys->vt->DreamSys__GetCurrentDayAndYear(...)`); GCC then
   treated `status`'s live range as spanning from the *switch* all the way
   to this *second, unrelated* assignment, forcing a third callee-saved
   register (frame grew to `-0x30`/`-0x40` and beyond, with the whole rest
   of the function's register numbering shifted). Splitting it into a
   fresh `check` variable confined the second use's live range to just
   after its own call (no intervening calls), closing that specific
   3-register blowup and recovering the correct `-0x28` frame with zero
   address drift.
4. **A `result` variable's live range starts at its assignment, not at its
   declaration.** Declaring `s32 result = 0;` at the top forced it to
   survive every call in the function (needing its own persistent
   register); moving the `= 0` assignment down to immediately before the
   `if` (declaration stays at C89-required block top, per
   `docs/DECOMPILATION_LEARNINGS.md`'s "default value hoisted into the
   guarding branch" idiom) let it live in a scratch register instead.

## The two residues that remain unresolved (53/57)

1. **Missing `li v0,0x1` filler (offset `0x16F08`).** Retail's `switch`
   lowering materializes an unused default-arm constant (`1`) in the
   `beq`'s delay slot even though nothing ever reads it (same "unused
   value in a delay slot" idiom documented elsewhere in this unit, e.g.
   `GameApplication__RunTitleMenu`'s `pollDone`). No `switch`/`if` reshaping reproduced it;
   adding an explicit `case 1: break;` regressed badly (27/57, GCC grew the
   whole switch, presumably crossing a density threshold into a different
   lowering strategy).
2. **`case 3`'s store reuses the wrong register (`$s0` instead of `$v0`).**
   Retail's `case 3` handler does `sw $v0, 0x24($s1)`, reusing `$v0` --
   which, at that point, still holds the literal `3` **just loaded for the
   comparison** (classic CSE: the same constant appears in the guard and
   the store). My build's `switch` lowering instead stores `$s0` (the
   switch discriminant register, which also happens to equal `3` at that
   point, since it *is* the value being switched on) -- semantically
   identical, byte-different. Tried: `self->unk24 = status;` (no change),
   and a hand-written nested-if reproducing retail's literal-reuse pattern
   directly (`caseVal = 2; if (status==caseVal) ... else { caseVal = 3; if
   (status==caseVal) self->unk24 = caseVal; ...}` -- this DID fix the
   register choice, confirming the CSE-reuse theory, but it reverted the
   branches to the wrong (`if`/`else if`) inverted shape from lever 1,
   trading one residue for a much larger one. No form found both branch
   shape and this register choice simultaneously.

Both remaining residues look like `switch`-statement-lowering internals
(which register a literal case value lives in, and whether an unused
default arm gets materialized) that this compiler's switch codegen doesn't
expose a source-level lever for, as distinct from the ordinary
delay-slot/declaration-order levers that closed everything else in this
unit. Recommended next step if revisited: a permuter run seeded with the
53/57 body above, since the remaining diff is narrow, low-risk, and
concentrated in exactly two spots.

## Proposed learning

`switch` on a small (2-3 value) dense-ish set of integer constants in this
compiler reuses a per-case literal's just-loaded register for that case's
own body when a matching value happens to be re-needed (a CSE artifact
internal to switch lowering) -- and this is not reliably reachable by
rephrasing the case body to reference the switch's own discriminant
variable instead of repeating the literal; both read as "the same value"
to a human, but the switch-lowering machinery keeps its own separate
tracking of "the literal I just compared against" versus "the variable I
switched on," and only the former gets the free reuse.

## HEAD BROADCAST cross-check (this round's two levers, plus #3's branch-target check)

- **goto/return early-exit lever:** applicable and used successfully for
  the `result` default-value shape (see lever 4 above) -- not via `goto`
  itself, but the same underlying idea (defer the assignment to where the
  branch actually needs it, not the function's top).
- **hand-hoisted loop invariant lever:** not applicable -- no loop.
- **Branch-target check (broadcast #3):** performed. Every remaining
  residue is instruction CONTENT (a missing filler instruction, a register
  choice for an otherwise-correct store) -- every branch target in the
  53/57 body matches retail exactly, confirmed via `asm-differ`. This is
  not a CFG problem; both remaining diffs are switch-lowering register/filler
  choices, consistent with the delay-slot-scheduling class of residue, not
  a wrong-shape stall.

## Naming history (before round 100)
**`GameApplication__RunDayTask` -- tier B.** Mechanics: builds a `StatusObj`
(`New_DayTask`), reads one status code off it (`slot44`), tears it down
(`slot4`), reacts to two of the codes (2 -> `GameApplication__PlayCinematic`,
3 -> latch `self->unk24`), then separately queries the owned `DreamSys`'s
day/year status and derives a 0/1 result. Named for the StatusObj query
mechanic, matching this unit's `GameApplication__RunTitleMenu` naming
shape (both are "poll an object for a status code and react to it"), since
what the two status-code values actually MEAN in the game is not
established from this body alone.

## Track 4 (2026-09-26, round 88, DayTask)

The "StatusObj" is DayTask (gDayTaskMethods, include/DayTask.h):
`New_Obj865C8` became `New_DayTask`, and this unit's local
`StatusObj`/`StatusObjMethods` view was deleted. The body now reads
`((DayTaskInitFn)obj->methods->init)(obj)` then
`obj->methods->release(obj)`: slot +0x044 is IntermediateBase's
`init(self, args, mode)`, and DayTask's occupant, `DayTask__Init`,
takes self alone, so the call keeps passing only `$a0` through a typedef of
the override (a pointer cast, no code). The status code is TimedTask's
`result`: 2 and 3 are values `DayTask__OnObjMNotify` sets when the ObjM
it built ends the day (`endDay` returned 0 with a cinematic entry: 2;
`endDay` failed, or events 0xC/0xD: 3). Byte-identical.

## Track 7 polish (round 100, echo)

### Naming

**`GameApplication__RunDayTask` -- tier A** (renamed from `GameApplication__PollStatusObj` with tools/rename.py). Evidence: builds one DayTask (New_DayTask), runs its init to the end and releases it; "StatusObj" was an older name of the DayTask class. It then returns (year != 0 && day == 1) from DreamSys's getCurrentDayAndYear, i.e. a year has gone by, which makes Application__RunMainLoop run +0x064, PlayEndingMovie.

Body changes, all byte-identical: obj/outVal/check -> dayTask/year/day; GameApplicationConfig.unk04 -> dayTaskSyncDriver (DayTask's ctor parameter syncDriver).

### History: code_1677c.c comments before the round-100 polish

Moved here from the source, verbatim (names as they stood then, where the tools had not already rewritten them).

```c
/* Builds a DayTask (include/DayTask.h), runs its init with self
 * alone (DayTask__Init takes nothing else, hence DayTaskInitFn) and
 * releases it; init's return, TimedTask::result, is a status code: 2 runs GameApplication__StartCinematicStream, 3 latches self->skipGraphRoomPoll.
 * Then queries the DreamSys status slot again (as GameApplication__PollGraphRoomStatus does),
 * this time passing an out-param, and derives a 0/1 result from both the
 * call's return and the out-param. */
/* Builds a DayTask for this instance's current state, reads one status
 * code off it, tears it down, and reacts to two of the codes. Then asks the
 * owned DreamSys a question and reports whether its answer was 1.
 *
 * Two things here were long-standing misreadings, both worth keeping written
 * down (docs/match-reports/GameApplication__PollStatusObj.md):
 *
 *  - `case 3` stores 1, NOT 3. Retail's `li $v0, 0x1` sits in the delay slot
 *    of the case-3 branch, so it executes before the jump is taken and $v0
 *    holds 1 -- not the 3 it held for the comparison -- by the time the
 *    store runs. Reading the store as `unk24 = 3` (the discriminant) was
 *    what produced the old 53/57 and the "the compiler materialises an
 *    unused default-arm constant" theory attached to it. There is no unused
 *    constant: `li $v0, 0x1` is the value being stored, hoisted into a delay
 *    slot on the only path that needs it.
 *  - `result = (check == 1)` is the whole comparison. GCC 2.6.3 lowers an
 *    equality test against a small constant to `xori` + `sltiu`, which reads
 *    back out of the disassembly as `(u32)(check ^ 1) < 1`. That transcription
 *    is arithmetically right and cost two instructions; the plain `== 1` is
 *    what the source said. */
```
