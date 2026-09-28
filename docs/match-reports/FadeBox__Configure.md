# FadeBox__Configure -- MATCHED (103/103)

> Renamed from `Class6E99C__Configure` on 2026-09-26 (tools/rename.py). Address 0x80040154.

> Renamed from `func_80040154` on 2026-09-20 (tools/rename.py). Address 0x80040154.

## Round 21 (runner delta): the final 2-word mflo-destination residue closed by an in-place division

Resumed from round 20's stall (101/103, residue isolated to the third
division's `mflo` destination register: retail `$v1`, every prior C
shape `$v0` -- see below for that round's own derivation). Restored the
exact 101/103 body and re-measured fresh: reproduces
`101/103 words match (file 0x30954-0x30AF0)` exactly, confirmed via
`tools/asm-differ/diff.py` to be the identical two-word residue this
report already described (`mflo v1`/`sw v1,0x84(s0)` in retail vs
`mflo v0`/`sw v0,0x84(s0)` in every prior build) -- every load, the
`div` itself, and every surrounding branch already matched byte-for-byte
before this round even started; only the `mflo`'s destination register
differed.

Eleven prior hand attempts (rounds 19/20/round-bravo) varied the
DIVIDEND's or DIVISOR's naming, casting, and reuse for this third
division (`q2 = self->unk68; self->unk84 = q2 / self->unk80;`), always
as a single combined statement assigning straight into the struct
field. None had tried writing the division **in place, back into the
dividend's own variable**, before copying it out to the field
separately:

```c
q2 = self->unk68;
q2 = q2 / self->unk80;
self->unk84 = q2;
```

This closes it: **103/103, whole-image SHA1 green**
(`build exit=0`, `OK: build matches retail SLPS_015.56`).

### Why this works: reusing the dividend's own dead register

`asm-differ` shows retail's dividend load (`lw v1,0x68(s0)`, i.e. `q2 =
self->unk68`) and divisor load (`lw v0,0x80(s0)`) already matched
byte-for-byte in every prior attempt -- the divide instruction itself
(`div zero,v1,v0`) was never the problem. `q2` (in `$v1`) is dead
immediately after the division: its only use is as the dividend, and
retail's `mflo` writes the quotient back into that SAME now-dead
register (`$v1`) rather than allocating a fresh one. Every prior attempt
computed the quotient as a value assigned DIRECTLY to `self->unk84`
(`self->unk84 = q2 / self->unk80;`) -- a single statement in which the
quotient is a brand-new pseudo-register with no source-level connection
to `q2`'s own dead register. Writing `q2 = q2 / self->unk80;` first
gives the quotient the SAME C-level identity as the dying dividend, and
GCC 2.6.3's register allocator reuses `q2`'s hardware register (`$v1`)
for it, exactly matching retail. The subsequent `self->unk84 = q2;` is
then a plain register-to-memory store with no register choice left to
make.

### Proposed learning

**A "same total instructions, only the destination register of the
final arithmetic op differs" residue can close by writing the
expression IN PLACE into the operand whose register is about to die,
rather than assigning the expression's result directly into the
destination field/variable.** This is a different axis from every lever
already tried on this specific residue (naming the divisor, casting
either operand, reordering the dividend's load, a scheduling barrier)
-- all of those varied WHICH VALUE participates in the expression or
WHEN it is computed; this lever instead changes whether the RESULT
shares a C-level identity with a DYING OPERAND, which is what let the
register allocator reuse that operand's now-dead register for the
result. Worth trying "assign back into a dying operand, then copy out"
as its own axis on any remaining single-instruction destination-register
residue, alongside the existing "reuse an existing variable name" and
"(s16)-cast an operand" levers already documented for this project.
Note this is NOT the extended-asm/`register asm("$N")` mechanism CLAUDE.md
HARD RULE 6 bans -- it is ordinary C whose SHAPE happens to guide the
allocator's own choice, the same category of lever as the whole-struct-
assignment fix documented in `func_8003FC70.md`/`FadeBox__PushPosition.md`.

## Round 20 (runner delta): two more attempts on the third division, both negative

Drift check: rebuilt the exact 101/103 body (round-bravo's `q1 -
(s16)q2` fix) verbatim, reproduced cleanly. Two new attempts, both on an
axis round 19 hadn't tried (naming the DIVISOR, not just the dividend,
and reordering the dividend's load relative to the second division's
conditional block):

1. **Named the divisor into a fresh local** (`d = self->unk80; ...
   self->unk84 = q2 / d;`, keeping `q2` reused for the dividend exactly
   as round-bravo's body does): no change, 101/103, identical bytes to
   baseline. The compiler folds the named divisor straight back to a
   direct field read either way.
2. **Hoisted the dividend's load (`d = self->unk68`) to BEFORE the
   `if (self->altMode != 0)` block**, instead of immediately before the
   final division (its position in every prior attempt): regressed
   sharply to 78/103 -- this is NOT a register-identity residue, it is a
   real instruction INSERTION (confirmed via `funcdiff.py`'s own
   in-range diff: an extra `lw` appears mid-function and every
   subsequent instruction shifts by one word). Moving the load earlier
   changes which value is live across the conditional block in a way
   that costs a real instruction, not just a register choice -- reverted
   immediately, not a viable direction.

Restored to the 101/103 body verbatim, then to `INCLUDE_ASM`. Full
oracle re-confirmed green.

### Proposed learning (round 20)

Two more axes closed on the third division's `mflo`-destination
residue: naming the DIVISOR (as opposed to round 19's dividend/divisor-
cast attempts) is neutral, and REORDERING the dividend's load earlier is
actively harmful (a genuine length regression, not a register swap).
Combined with round 19's five attempts and round-bravo's four, this
residue has now seen eleven hand attempts plus one permuter run across
three rounds, all on the same isolated 2-word `mflo $v0`-vs-`$v1` pair,
with no lever found that touches it without regressing something else.
Worth flagging to the next round as effectively exhausted by hand;
a fresh permuter run seeded from THIS round's confirmed-clean 101/103
body (identical to round-bravo's, not re-derived) is the only
remaining untried lever, and even that already ran once (round-bravo,
best found: 10, not zero).

## Round 19: five more attempts on the third division's mflo swap, all negative

Re-verified the 101/103 body (round-bravo's `q1 - (s16)q2` fix kept) via
fresh build: confirmed reproducible. Tried five more variants
specifically targeting the remaining THIRD division
(`self->unk68 / self->unk80` -> `self->unk84`, `mflo` destination `$v0`
vs retail's `$v1`):

1. `(s16)` cast on the DIVIDEND (`(s16)q2 / self->unk80`): regressed to
   95/103 -- reproduces attempt-1's whole-function pattern, not a
   narrow fix.
2. `(s16)` cast on the DIVISOR (`q2 / (s16)self->unk80`): regressed to
   100/103 -- one word worse than baseline, and does not touch the
   right instruction.
3. Bare `__asm__("")` immediately before `q2 = self->unk68;`: no change
   (101/103) -- confirms this is genuinely register-identity, not an
   orderable scheduling artifact (a barrier only reorders, and CLAUDE.md
   rule 6's own test -- "if removing it changes WHICH REGISTER holds a
   value, it is banned" -- means this residue class is correctly
   off-limits to fix directly, consistent with every prior attempt).
4. Direct field access with no `q2` variable at all
   (`self->unk84 = self->unk68 / self->unk80;`): regressed to 95/103,
   reproducing attempt-1's original finding that removing the `q2`
   variable re-opens the SECOND division's residue too (the two remain
   entangled through this one variable's presence, not just its name).
5. Reusing `q1` instead of `q2` for the third division's dividend (on
   top of the round-bravo cast fix, unlike the original attempt 4 which
   tried this WITHOUT the cast): regressed to 98/103 -- confirms `q2`
   specifically (not "any reused name") is what the cast fix needs
   alongside it.

All five are firm negatives; 101/103 (the round-bravo body, restored
verbatim below) remains the best reached. Full build re-confirmed clean
(`build exit=0`) with this function restored to `INCLUDE_ASM`.

### Proposed learning (round 19 addition)

The remaining 2-word residue survives casting either operand of the
division, removing/renaming its intermediate variable, and a scheduling
barrier -- five attempts along the "vary the third division's own
expression" axis, all negative, following directly from round-bravo's
own four attempts along the same axis for the SECOND division (three of
which also failed before the `(s16)`-cast-on-subtraction lever worked).
Unlike that case, no analogous cast site exists here (the third division
has no subtraction to wrap a cast around -- it is a bare `a / b`), so
the axis that worked for division 2 has no direct transfer point for
division 3, as the round-bravo report already noted. This strengthens
(not weakens) that conclusion: worth trying a genuinely different axis
(e.g. the permuter, seeded from THIS round's 101/103 body specifically)
before spending further hand attempts on expression-level variants of
the third division alone.

Unit `ScreenWidgets`, carved round 14. `FadeBoxMethods::configure` (`+0x0DC`,
shared verbatim with `ClassEAC0Methods::configure`).

## Shape

```c
#if 0
s32 FadeBox__Configure(FadeBoxObj *self, s32 a1, s32 a2, s32 a3) {
    FadeBoxMethods *methods;
    s32 flag;
    s32 q1, q2;

    methods = self->methods;
    if (a2 < 0) {
        a2 = self->unk70;
    } else {
        self->unk70 = a2;
    }
    flag = 1;
    if (a2 != 0) {
        self->unk78 = a2;
    } else {
        flag = 2;
        self->unk78 = 0xF;
    }
    self->unk78 = a2;
    if (a2 == 0) {
        self->unk78 = 0xF;
    }
    q1 = 0x100 / self->step;
    self->unk7C = a3;
    self->unk80 = q1;
    if (self->altMode != 0) {
        q2 = q1 / self->divisor;
        self->unk80 = q1 - q2;
    }
    q2 = self->unk68;
    self->unk84 = q2 / self->unk80;
    methods->slot10(self);
    methods->slot64(self, 1);
    methods->slot68(self, flag);
    methods->slot60(self, 1);
    return a2;
}
#endif
```

Real 3-parameter occupant, NOT the 1-argument shape its two known callers
(`FadeBox__StartFadeDown`, `FadeBox__StartFadeUp`, both this unit) actually invoke it
with -- those callers set up only `self` before the `jalr`, so `a1`/`a2`/
`a3` are leftover register values from whatever preceded the call at each
site, unused by design at those two call sites. The vtable slot's own
DECLARED type therefore stays `s32 (*configure)(FadeBoxObj *self);`
(matching what the two known callers actually configure) while this
function's own top-level definition keeps the real 4-parameter signature
its body needs -- the two are independent per this project's established
"per-call-site arity" convention, since the data table itself is not
compiled C and enforces nothing.

## Residue

Two paired instructions differ (4 words total): the SECOND division
(`q1 / self->divisor`, guarded by `self->altMode != 0`) and the THIRD division
(`self->unk68 / self->unk80`) have their `mflo` DESTINATION and subsequent
`sw` SOURCE registers swapped relative to retail -- retail's second
division lands in `$v0`, third in `$v1`; every C shape tried lands the
second in `$v1`, third in `$v0`. Same instructions, same total length
(confirmed via `build/lsdde.map`: every symbol after this function links
at its exact retail address), pure register bank.

## Attempts (5)

1. Naive `self->unk84 = self->unk68 / self->unk80;` as the final
   statement: 95/103, registers swapped on BOTH the second and third
   divisions' `mflo` targets AND several intervening `bne`/`bnez` operand
   registers that read them.
2. Hoist the third division's dividend into a NAMED local reusing the
   already-declared `q2` (the SAME variable the second division's own
   result is stored in): 99/103 -- fixed the SECOND division's own
   residue as a side effect, isolating the remaining 4-word gap to the
   THIRD division alone. The two divisions' registers are evidently
   entangled through this one variable name.
3. Same, but with a FRESH third variable (`q3`) instead of reusing `q2`:
   regressed back to 95/103 -- reusing `q2` specifically (not merely
   "any named local") is what closes the second division's part.
4. Hoist the third division's dividend into `q1` instead (reusing the
   FIRST division's own variable, dead by that point): 98/103 -- worse
   than reusing `q2`, better than a fresh name.
5. A bare `__asm__("")` between the `if (self->altMode != 0) {...}` block
   and the `q2 = self->unk68;` reassignment, attempting to pin the
   register choice at the boundary: no change from attempt 2's 99/103.

## Round-bravo update: 99/103 -> tighter, still not closed (2 words left)

Ran the permuter (`permuter.py --debug` first to confirm base score --
confirmed clean: 0 stack/branch diffs, 4 register differences, 0
insertions/deletions, matching the reported 4-word residue exactly. Then
`-j 6 --stop-on-zero --best-only`, `timeout 400`, ~several hundred
iterations before the bound. **Best found: 10 (down from 20 permuter-scale,
i.e. real improvement), not zero.** The winning candidate's only change
from attempt 2's body: `self->unk80 = q1 - (s16)q2;` -- casting the
SECOND division's own quotient to `s16` before the subtraction. Verified
by hand (`--debug` on the isolated change, WITH `--stack-diffs` per the
coordinator's tooling-bug broadcast -- 0 stack differences, confirmed no
hidden frame-size component): this closes the SECOND division's residue
completely (its own `mflo`/`bne`-operand instructions now match retail
exactly, byte for byte), leaving ONLY the THIRD division
(`self->unk68 / self->unk80` -> `self->unk84`) with its `mflo` destination
still swapped (`$v0` vs `$v1`) -- down from a 4-word to a clean 2-word
residue.

Tried the same `(s16)` cast lever on the THIRD division to close the last
2 words:
- Casting the whole expression (`self->unk84 = (s16)(q2 / self->unk80);`):
  WORSE (210) -- `self->unk84` is a real `s32` field, so casting the
  RESULT forces an actual truncate-then-extend sequence retail does not
  have, growing the function.
- Storing through a fresh `s16` local first
  (`s16 q3 = q2 / self->unk80; self->unk84 = q3;`): also worse (210), same
  mechanism -- the local's own narrower type forces a real truncation,
  not just a scheduling nudge on the `mflo` destination choice.

Both attempts changed the VALUE-WIDTH of the stored result, which is not
what fixed the second division (there the cast wrapped only the
SUBTRACTED operand, `q1 - (s16)q2`, not the field `self->unk80` being
written). The third division has no analogous subtraction to wrap a cast
around -- it is a plain `field = a / b;` -- so the lever that worked for
division 2 has no direct analog to try on division 3 without changing the
stored value's width. **Best remaining: the 10-scoring body above (2
words), reported as the new stall state.** Not run further given the
round's wind-down; flagging the exact remaining instruction pair (division
3's `mflo` destination, `FadeBox__Configure.s`'s own third `mflo`/`sw` pair) for
the next attempt.

### Proposed learning (this round)

**A `(s16)` cast on one OPERAND of an arithmetic expression can fix a
register-identity residue on a DIFFERENT, textually-earlier division's
`mflo` destination** -- casting division 2's own quotient before
subtracting it closed division 2's OWN residue, exactly as expected, but
also had no effect on division 3 (they are apparently less entangled via
type than via the variable-reuse lever attempt 2 already found). This is
a genuine, positive instance of the coordinator's flagged type-axis lever
(distinct from `TaskObjF__WriteMemcardSaveFile`'s masked-byte-parameter case): the fix is
narrowing an intermediate VALUE at the point it is CONSUMED (the
subtraction), not narrowing a declaration or a parameter. Worth trying on
other multi-division register-identity residues in this project before
assuming variable-reuse (this function's OWN previously-recorded lever) is
the only axis that matters for this shape.

## What I did NOT try, and why

- **The permuter**, seeded from attempt 2's 99/103 body -- this is exactly
  the "base score made of register differences with zero
  insertions/deletions" shape the project's own docs identify as a live
  permuter search, and closer to the ideal case than most stalls filed
  this round (only 4 words, isolated to one arithmetic pair). Not run due
  to time budget at the point this was reached (last function attempted
  before the round's time ran out). **Superseded above -- now run, closed
  half the residue, other half still open.**
- **Renaming `flag`** or reordering the two `self->unk78` guard blocks
  (the first with `flag`, the second a plain re-test) -- these already
  reproduce retail exactly (confirmed: the diff shows zero residue in that
  region), so no attempt was spent perturbing a part that already matches.

## Proposed learning

Reusing an EXISTING variable name across two structurally-separate
divisions closed one of the two registers' identity but not the other,
and a THIRD, freshly-named variable did WORSE than either the reused name
or a different existing name -- i.e. which specific variable identity you
reuse for an unrelated later value measurably changes a compiler-2.6.3
register-coloring outcome for values several statements away. This is a
finer-grained version of the already-documented "declaration order decides
register/stack layout" family; worth testing named-variable REUSE as its
own lever on other multi-division register-identity stalls before assuming
a fresh name is neutral.

## Naming (round 61, track 3)

**`FadeBox__Configure`** -- tier B. `FadeBoxMethods::configure`
(`+0x0DC`, shared occupant with `ClassEAC0Methods::configure`). Sets up
`unk78`/`unk7C`/`unk80`/`unk84`/`unk68`-derived state from its own
`a1`/`a2`/`a3` (mode, count, and a divisor-flag path via `altMode`), then
dispatches `slot10`/`slot64`/`slot68`/`slot60` (bravo's own occupants).
Named "Configure" rather than "Start"/"Init" because it is ALSO reachable
through `configure` with only `self` (no real arguments) from
`FadeBox__StartFadeDown`/`FadeBox__StartFadeUp`, where its
return value is read back as a color-table index -- i.e. it is a
general-purpose "(re)configure and report" entry point, not a one-shot
initializer. The a2-garbage-on-1-arg-call nuance is inherited unchanged
from the matched body and already documented in this function's own
`## Notes`/report history; not re-derived here.

## Track 4 (2026-09-26, round 87, echo)

Slots under their unified names. The call at +0x010 was `slot10(self)`
and is now `addChild(self, source)`: the occupant is SceneNode__AddChild,
and this function never writes `$a1` before that jalr, so its own second
argument (now `BasicClass *source`) is the child; the build stayed
byte-identical with the argument spelled. The two StartFade functions
forward their `source` to it, and FadeBox__Stop removes the same object
with `removeChild` (its own second argument, which FadeBox__Update
passes as the notifying `sender`). The `configure` slot is now typed with
its occupant's four parameters, so the StartFade callers' file-local
`Configure6E99CFn` cast is gone. Fields: unk70 -> defaultChannels, unk78 ->
channels, unk80 -> ticksLeft, unk68 -> BoxFill's `mask`; slot64/68/60 ->
setSemiTrans/setSemiTransRate/setDisplay.

## Track 6 (2026-09-26, round 93, charlie)

Renamed with the class: `Class6E99C` is now `FadeBox`
(`python3 tools/renametype.py Class6E99C FadeBox`, table
`python3 tools/rename.py D_8006E99C gFadeBoxMethods`), tier A; the evidence
is in New_FadeBox.md's Track 6 section. The method's own name was kept: it
already says what the body does. renametype.py rewrote the old class name in
this report's earlier history too (known, pending an operator decision).

The field this method writes at +0x084 is now `maskPerTick` (was `unk84`):
`BoxFill::mask / ticksLeft`, stored and never read by any code.


## Track 7 (round 100, charlie)

### Naming

- **`FadeBox::mode`** (+0x07C, was `unk7C`) -- tier B. Written by this
  function from its third argument, zeroed by Reset, read only by Update,
  which skips the colour step while it is 9 (the tick countdown and the stop
  still run, so 9 holds the colour for the fade's length). Every caller in
  the tree passes 0 (Entity/_f/_g, ObjMStyleActor, and ObjM__StartFadeUp's
  `fadeMode`, whose own callers all pass 0), so no other value is observed;
  "mode" says only that it selects a variant.
- Parameter `arg3` -> `mode` here, in StartFadeDown and StartFadeUp, and in
  FadeBox.h's three slots and prototypes; locals `q1` -> `ticks`.

### History: the in-place division is no longer load-bearing (round 100)

Round 21 closed this function by writing the third division in place
(`q2 = self->unk68; q2 = q2 / self->unk80; self->unk84 = q2;`), because
the same `q2` also held the first division's quotient. With the two
quotients given their own locals (`cut = ticks / self->divisor;`, and no
local at all for the third), the plain spelling
`self->maskPerTick = self->mask / self->ticksLeft;` is byte-exact
(whole image green, 103/103), and so is `ticks - cut` without round 20's
`(s16)` cast. The register residue came from sharing one variable across
the two divisions, not from the division's spelling. The double store of
`channels` (both branches, then again with the 0xF fix-up) is retail's:
dropping the second store and its test builds 99 words, measured.
