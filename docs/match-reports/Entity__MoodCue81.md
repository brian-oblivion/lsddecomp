# Entity__MoodCue81 -- MATCHED, round 59 (2026-09-20)

> Renamed from `func_80063144` on 2026-09-24 (tools/rename.py). Address 0x80063144.

REVISITED, round 59: MATCHED 217/217 byte-exact; names/types not relevant

**Unit:** `Entity_e` · **Size:** 217 words · **Result:** 217/217 words,
`insertions 0 / deletions 0`, `./build-and-verify.sh` green
(`OK: build matches retail SLPS_015.56`), no out-of-range drift.

**`src/Entity_e.c` now contains zero `INCLUDE_ASM` -- the unit is complete.**

Closed in **2 builds** on top of the body this report already carried.
Divergence #1 had been closed since round 13 (the chained
`out->unk44 = out->unk30 = 0x17;` plus the store reorder) and is unchanged.
Divergence #2, open through rounds 13, 18, 19 and 20 and searched for 30485
permuter iterations, is closed below.

## On the revisit hypothesis (the measurement this round was buying)

`unkFC` is now `moodTimer`, `unk94` is `target`, `slot148` is
`getProximityRatio`, `slot160` is `deactivate`, `slot30` is
`notifyParents`, `slot16C` is `stopSoundCue`. **None of it mattered.**
Every one of those fields and slots was already correctly identified in the
round-13 derivation; the renames made the body readable and changed nothing
that compiled. The callers likewise added nothing.

What the revisit bought was the same thing it bought on this unit's other
stall: the chance to re-classify a residue whose *measurement* everyone had
re-verified and whose *class* nobody had re-questioned.

## Divergence #2 was never a scheduling or register problem

Four rounds classified it as a delay-slot/scheduler decision:

> ... retail schedules `li a1,0x1` ... into the delay slot of THAT CALL's
> own `jalr`, and defers `mod = 0` (`move s1,zero`) to the delay slot of a
> LATER branch ... Same instructions, same values, genuinely different
> POSITIONS across two different basic blocks.

Round 19 corrected the "same instruction count" half of that (the body is
218 words, one LONGER than retail) and reclassified it as a net insertion.
That correction was right. The *class* was still wrong: it is not a
scheduling residue at all. It is **two statement-placement errors in the
`mood != 1` arm, both of the same shape -- an assignment written as a
statement BEFORE a branch that retail writes as part of the branch
structure.**

### Placement error 1 -- `mod = 0` is the ELSE ARM, not a preceding statement

The body on file wrote:

```c
mod = 0;
if (this->unk44 == 2) {
    mod = -0x60;
    ...
}
```

Retail's shape is the `if`/`else`:

```
53b24:  lw   v1,0x44(s0)
53b28:  li   v0,0x2
53b2c:  bne  v1,v0,53c48        ; -> the OUTER join, not a separate else block
53b30:   move s1,zero           ; delay slot: the entire else arm
```

A one-instruction `else` arm gets stolen into the branch's own delay slot
by reorg, and the branch then targets the outer join directly -- so the
else arm leaves no block behind at all, which is exactly why it reads as
"deferred scheduling" rather than as an else. Written as a statement
before the `if`, GCC instead has `s1` available early, sinks `move s1,zero`
into the `slot130` call's `jalr` delay slot, and has to push that call's
own `li a1,0x1` argument up into the *mood-dispatch* `bne`'s delay slot --
where it is dead, and where it costs the extra word round 19 measured.
**All three of the round-13 report's observations are consequences of this
one error.**

```c
if (this->unk44 == 2) {
    ...
} else {
    mod = 0;
}
```

Result: **77/217 -> 208/217**, `insertions 2/2 -> 1/1`, and the 218-word
overshoot gone (length exact, no out-of-range warning).

### Placement error 2 -- `mod = -0x60` goes AFTER the inner `if`

One divergence remained: retail has `li s1,-0x60` in the `beqz` delay slot
*after* the `slot144` call; the body had it before the call.

```
53b4c:  slti v0,v0,0x960
53b50:  beqz v0,53b78           ; -> a 2-insn trampoline, not the outer join
53b54:   li  s1,-0x60           ; delay slot
53b58:  move a0,s0
...
53b78:  j    6344c
53b7c:   move a0,s0
```

`li s1,-0x60` cannot be reached by reorg from either thread at 53b78, so it
must genuinely sit in the block *after* the call, which means the
assignment follows the inner `if` in source. Writing it there puts it at
the inner join, where reorg hoists it into the `beqz` delay slot and
redirects the branch target past it to the `j` -- which is precisely the
trampoline retail has and the body did not:

```c
if (this->unk44 == 2) {
    if (this->methods->slot144(this, this->target) < 0x960) {
        this->unk44 = 0xB;
        this->methods->notifyParents(this, 0xC);
    }
    mod = -0x60;
} else {
    mod = 0;
}
```

Byte-exact on that build.

## The matched body (the whole `mood != 1` arm; the rest is unchanged from round 13)

```c
} else {
    this->target->methods->slot130(this->target, 1);
    SceneNode__FaceTarget((Entity *)this->target, this, 1, 1, 0);
    if (this->unk44 == 2) {
        if (this->methods->slot144(this, this->target) < 0x960) {
            this->unk44 = 0xB;
            this->methods->notifyParents(this, 0xC);
        }
        mod = -0x60;
    } else {
        mod = 0;
    }
}
```

## What was measured (2 builds)

| # | body | words | ins/del | length |
| --- | --- | --- | --- | --- |
| 0 | this report's "Best C now on file" (control) | 77/217 | 2/2 | **218** (+1) |
| 1 | `mod = 0` moved to an `else` arm | 208/217 | 1/1 | **217** (exact) |
| 2 | `mod = -0x60` moved after the inner `if` | **217/217** | **0/0** | 217 |

Build 0 reproduced this report's recorded 77/217 and round 19's 218-word
count exactly, so the inherited body was verified before anything was built
on it (project rule).

### Proposed learning

**A one-instruction `else` arm leaves NO BLOCK in the output -- it is
stolen into the branch's own delay slot, and the branch then targets the
join. So "retail assigns this value in a delay slot, mine assigns it as a
plain instruction" is evidence about the SOURCE's if/else shape, not about
the scheduler.** The tell is exact: a conditional branch whose target is
the *outer* join (not a nearby block) with a real assignment in its delay
slot. Look for the arm, do not reach for a barrier.

This is the same family as the block-order rule in
`DECOMPILATION_LEARNINGS.md` section 3a, but it is not that rule and 3a
does not cover it. 3a is about which of two *existing* arms gets the
fallthrough; this is about an arm that has no block at all, and it is
invisible under 3a's own tell (a bare unconditional `j`), because there
isn't one.

**Corollary, and it is the reason this stood for four rounds: the permuter
cannot find it.** The 30485-iteration run recorded in this file plateaued
at 230 and, by its own account, "every improvement it found was inside
divergence #1's block". A permuter mutates expressions, operand order,
temporaries and control-flow spelling within the statements it is given; it
does not move a statement into an else arm that does not exist in its base.
**A residue that survives a long permuter run with no movement at all is
weak evidence for "scheduler-internal" and strong evidence for "the
statement structure is wrong in a way the permuter's mutation space does
not reach."** This report drew the opposite inference from the same fact.

---

# History (superseded by the round-59 match above)

Everything below is the stall record as it stood through rounds 13-20. It
is kept for its NEGATIVES -- the seven reshapes that did not close
divergence #1, the type-axis check, the `--stack-diffs` re-measurement, the
permuter logging pitfall -- all of which remain accurate. Its verdicts
about divergence #2 being a scheduling residue are **wrong**; see above.
The round-19 correction (218 words, a net insertion) is **right**, and was
the finding that pointed at a source-shape cause.

## (original title) Entity__MoodCue81 -- STALL (TWO divergences, not one; --debug base score 465, funcdiff 77/217 after closing divergence #1)


**CORRECTION (round 19): divergence #2 is NOT a zero-cost pure reordering.**
Rebuilding the exact "Best C now on file for the next attempt" body from
this report (divergence #1 closed, divergence #2 still open) shows the
compiled function is genuinely **218 words, 1 LONGER than retail's 217**
(confirmed via `objdump` word count on `build/src/Entity_e.c.o`), and
`funcdiff.py` reports the usual outside-range drift warning
(107390 bytes) -- contradicting this file's own prior claim of "an 8-word
improvement matching the in-range word count exactly, with no separate
outside-range shift". See the round-19 section near the end for the full
re-derivation; read it before trusting the "in-range, +8 words, clean"
claim just below.

## CURRENT STATE (read this first; the rest of the file is history in arrival order)

**Two independent divergences, not one.** The round-13 report below claims
a single one-word residue (216/217) -- that count was correct but hid a
SECOND divergence of equal size. `permuter.py --debug` on the round-13
body gives base score 465 (not the 200 that a genuine single-instruction
residue should give): `Register Differences: 9, Reorderings: 2,
Insertions: 2, Deletions: 1`.

- **Divergence #1 -- CLOSED.** Retail materializes `li v1,-1` ahead of
  three stores (`out->unk20`/`unk34 = -1`); the round-13 body collapses it
  to one reused register. Fix: `out->unk44 = out->unk30 = 0x17;` (chained
  assignment) plus reordering the `unk1C`/`unk10`/`unk20`/`unk30`/`unk44`/
  `unk34`/`unk48` stores to `1C, 10, 20, (44=30), 34, 48`.
  **`INCLUDE_ASM` is still in place** in `src/Entity_e.c` -- divergence #2
  remains open and no score short of byte-exact stays in `src/`. The
  closing form for divergence #1 is preserved below as the "Best C now on
  file for the next attempt."
- **Divergence #2 -- OPEN, concrete description for the next attempt.**
  In the `mood != 1` (else) branch, retail schedules `li a1,0x1` (the
  literal `1` argument to `this->unk94->methods->slot130(this->unk94, 1)`)
  into the delay slot of THAT CALL's own `jalr`, and defers `mod = 0`
  (`move s1,zero`) to the delay slot of a LATER branch (`bne v1,v0,...`
  testing `this->unk44 == 2`). Every C form tried instead has GCC hoist
  `li a1,0x1` up into the delay slot of the EARLIER `bne s2,v0` branch
  (the mood-dispatch test, where it is dead/overwritten immediately by
  `move a1,zero` for the `slot48` call) and place `mod = 0` immediately
  after the `slot130` call instead. Same instructions, same values,
  genuinely different POSITIONS across two different basic blocks --
  `--debug`'s `Reorderings: 2` bucket, confirmed by raw `objdump`
  comparison (not a register-identity mismatch: checked, both consumer
  types are plain `s32`, see the type-axis note near the end of this
  file). Not a type/width issue. Untried: a longer/re-seeded permuter run
  (30485 iterations at `-j 6` under this round's contention plateaued at
  230 without touching divergence #2 at all -- every improvement it found
  was inside divergence #1's block); a scheduling barrier placed inside
  the else-branch specifically around the `slot130` call and the
  `mod = 0` assignment has not been tried by hand.
- **funcdiff after closing #1 only: 69/217 -> 77/217** (in-range, +8
  words), verified against the real oracle (`./build-and-verify.sh`
  fails as expected -- divergence #2 remains -- `INCLUDE_ASM` restored).
- Permuter run: 30485 iterations, `-j 6 --stop-on-zero --best-only`,
  bounded by `timeout 600`. Exit code could not be read directly from the
  log (a logging defect, documented below); wall-clock/behavior evidence
  points to **124**. Never reached zero.

---


Unit: `Entity_e` (round 13). The largest function in this round's queue
(237 asm lines / 0x364 bytes / 217 words). Not a toolchain blocker -- the
blocker screen is clean:

```
$ grep -nE 'gp_rel|addiu *\$at, *\$at, *%lo' asm/nonmatchings/Entity_e/Entity__MoodCue81.s
(no output)
```

**The entire control-flow structure, every field, every vtable slot, both
`SceneNode__FaceTarget` call shapes (including one with REVERSED argument roles),
and three separate division-by-constant idioms were all derived and
verified correct.** The only residue is ONE missing instruction in a run
of seven struct-field stores: retail materializes the constant `-1` into
its own register (`$v1`) ahead of three unrelated stores, and no C-level
reshaping tried reproduces that extra register.

## Best-reached source (compiles, builds green, scores 69/217 in-range
## due to the one-word drift -- restored to `INCLUDE_ASM` per project rule)

```c
extern u8 D_80089E08[];

void Entity__MoodCue81(Entity *this, EntityMoodHandlerArg *out) {
    s32 mod;
    s32 mood;

    if (this->unkFC == 0 && (rand() & 1)) {
        this->unk44 = rand() % 3;
    }
    if (this->unk44 != 0 && this->unkF4 != 0) {
        if (out->unk4 >= 0x3D) {
            out->unk4 = 0;
        }
        if (out->unk4 % 20 == 0) {
            out->unk1C = 0x17;
            out->unk30 = 0x17;
            out->unk44 = 0x17;
            out->unk10 = 0;
            out->unk20 = -1;
            out->unk34 = -1;
            out->unk48 = -2;
        } else if (out->unk4 % 20 == 0xE) {
            out->unk1C = -2;
            out->unk30 = -2;
            out->unk44 = -2;
        }
        SceneNode__FaceTarget(this, this->unk94, 1, 0, 0);
        mood = this->unk44;
        if (mood == 1) {
            this->methods->slot48(this, 0, D_80089E08);
            mod = -0x176;
            if (this->methods->slot144(this, this->unk94) < 0x200) {
                this->methods->slot160(this);
                this->unk44 = mood;
            }
        } else {
            this->unk94->methods->slot130(this->unk94, 1);
            SceneNode__FaceTarget((Entity *)this->unk94, this, 1, 1, 0);
            mod = 0;
            if (this->unk44 == 2) {
                mod = -0x60;
                if (this->methods->slot144(this, this->unk94) < 0x960) {
                    this->unk44 = 0xB;
                    this->methods->slot30(this, 0xC);
                }
            }
        }
    } else {
        out->unk10 = this->methods->slot148(this);
        if (out->unk4 % 22 == 0) {
            out->unk1C = 0x1C;
        }
        if (this->unkFC >= 0x1F5) {
            SceneNode__FaceTarget(this, this->unk94, 1, 0, 0);
        }
        if (this->methods->slot144(this, this->unk94) < 0x800) {
            this->unk94->methods->slotC4(this->unk94, -0x800, 0);
        }
        mod = -0x14;
    }
    this->methods->slotD0(this, mod, 1);
    if (this->unk28 != 0) {
        this->methods->slotCC(this, -0xC8, 0);
    }
}
```

Preserved inline per project convention (`#if 0`, never a block comment,
with every declaration it needs, positioned where it would compile back
into `src/Entity_e.c` in place of the current `INCLUDE_ASM`):

```c
#if 0
extern u8 D_80089E08[];

void Entity__MoodCue81(Entity *this, EntityMoodHandlerArg *out) {
    s32 mod;
    s32 mood;

    if (this->unkFC == 0 && (rand() & 1)) {
        this->unk44 = rand() % 3;
    }
    if (this->unk44 != 0 && this->unkF4 != 0) {
        if (out->unk4 >= 0x3D) {
            out->unk4 = 0;
        }
        if (out->unk4 % 20 == 0) {
            out->unk1C = 0x17;
            out->unk30 = 0x17;
            out->unk44 = 0x17;
            out->unk10 = 0;
            out->unk20 = -1;
            out->unk34 = -1;
            out->unk48 = -2;
        } else if (out->unk4 % 20 == 0xE) {
            out->unk1C = -2;
            out->unk30 = -2;
            out->unk44 = -2;
        }
        SceneNode__FaceTarget(this, this->unk94, 1, 0, 0);
        mood = this->unk44;
        if (mood == 1) {
            this->methods->slot48(this, 0, D_80089E08);
            mod = -0x176;
            if (this->methods->slot144(this, this->unk94) < 0x200) {
                this->methods->slot160(this);
                this->unk44 = mood;
            }
        } else {
            this->unk94->methods->slot130(this->unk94, 1);
            SceneNode__FaceTarget((Entity *)this->unk94, this, 1, 1, 0);
            mod = 0;
            if (this->unk44 == 2) {
                mod = -0x60;
                if (this->methods->slot144(this, this->unk94) < 0x960) {
                    this->unk44 = 0xB;
                    this->methods->slot30(this, 0xC);
                }
            }
        }
    } else {
        out->unk10 = this->methods->slot148(this);
        if (out->unk4 % 22 == 0) {
            out->unk1C = 0x1C;
        }
        if (this->unkFC >= 0x1F5) {
            SceneNode__FaceTarget(this, this->unk94, 1, 0, 0);
        }
        if (this->methods->slot144(this, this->unk94) < 0x800) {
            this->unk94->methods->slotC4(this->unk94, -0x800, 0);
        }
        mod = -0x14;
    }
    this->methods->slotD0(this, mod, 1);
    if (this->unk28 != 0) {
        this->methods->slotCC(this, -0xC8, 0);
    }
}
#endif
```

## The residue, precisely

`asm/nonmatchings/Entity_e/Entity__MoodCue81.s` lines around `.L80063220`
(`out->unk4 % 20 == 0` branch body):

```
li      v0,0x17
li      v1,-1        <-- retail materializes -1 into v1 HERE, before ANY store
sw      v0,0x1c(s1)
sw      v0,0x30(s1)
sw      v0,0x44(s1)
li      v0,-2
sw      zero,0x10(s1)
sw      v1,0x20(s1)
sw      v1,0x34(s1)
j       .L80063260
 sw     v0,0x48(s1)   ; delay slot
```

My best C reproduces every STORE (target offset, value, and order) exactly
but collapses to a SINGLE register (`v0`) reused for `0x17`, then `-1`,
then `-2` in sequence -- one `li` short, at exactly this spot. Confirmed
via `tools/asm-differ/diff.py Entity__MoodCue81`: past this one missing
instruction, EVERY SUBSEQUENT instruction in the function (and in every
function compiled after it in the same translation unit, checked out to
several KB past this function) is byte-identical, just uniformly shifted
by 4 bytes -- textbook "one instruction short, everything after it
shifts" (CLAUDE.md / MATCHING-GUIDE's residue list), confirming this is
the ONLY residue in the whole 217-word function.

## What was tried (does not exhaust the 30-attempt budget, but exhausts the
## productive search directions)

All of the following were verified against either the pinned `cc1`
directly (isolated probes, near-instant) or a full `./build-and-verify.sh`
against the real function (also near-instant):

1. **Baseline** (the source above): 69/217 in-range, missing exactly the
   one `li v1,-1`.
2. **Named remainder** (`s32 r = out->unk4 % 20; if (r == 0) ... else if
   (r == 0xE) ...`) -- the technique that fixed `Entity__MoodCue71`'s
   register-identity residue in this same unit. Made it WORSE (62/217):
   it perturbs the shared division computation itself, which was already
   matching.
3. **Scheduling barriers** (`__asm__("")`) at three different positions
   inside the block (after the first store, after the third store, after
   the zero-store) -- CLAUDE.md's permitted lever for order-only residues.
   No effect on the register split in any position.
4. **Named locals for the constants** (`s32 c17 = 0x17; s32 cNeg1 = -1;`,
   declared either at the top of the block or hoisted to the enclosing
   `if`) -- collapses back to identical codegen under `-O2` constant
   propagation, both in isolated `cc1` probes and in the real build.
5. **Chained assignment** (`out->unk20 = out->unk34 = -1;`) -- confirmed
   via the real build to evaluate RIGHT-TO-LEFT (stores `unk34` before
   `unk20`), which contradicts retail's observed store order regardless of
   any register benefit. Ruled out on store-order grounds alone.
6. **Re-scoping the `mod`/`mood` locals** (function-top vs. declared at the
   top of the enclosing `if` block) -- no effect.
7. **Isolated `cc1` reproducers** matching the real byte offsets exactly
   (`0x1C`/`0x30`/`0x44`/`0x10`/`0x20`/`0x34`/`0x48`), with and without a
   preceding division computation and a following external call to
   simulate the real function's surrounding register pressure -- GCC 2.6.3
   collapses to one register in every variant tried; the register split
   was not reproduced in isolation at any scale tested.

**None of these are a register-identity mismatch** (CLAUDE.md's banned
`register T v asm("$N")` / operand-constraint class does not apply --
nothing here pins WHICH register holds a value; the question is whether a
SECOND register gets used at all). It is the OTHER documented residue
class instead.

## Classification: the project's own "redundant move" class

This matches `docs/MATCHING-GUIDE.md`'s already-documented residue exactly:

> A redundant `move` retail emits and you do not, same register, same
> value... resists `goto`/`return` spelling, temp placement, barriers and
> `volatile`. It is the project's best-posed permuter target -- do not
> spend a fresh attempt budget re-deriving it.

Every element of that description matches here: same VALUE (`-1`), an
extra materialization retail has and my C does not, immune to barriers,
temp placement, and (newly confirmed this round) named-local hoisting and
chained-assignment restructuring too. The only difference from the
guide's prior three instances is that this is a plain constant rather than
a variable's value, and the extra instruction is a full `li` rather than a
bare `move` -- the same shape at slightly larger scale.

### Proposed learning

**The "redundant move" residue generalizes to "redundant constant
materialization" -- a `li` retail has and you don't, immune to reordering,
naming, and barriers, is the same class as the documented redundant
`move`, just for an immediate instead of a loaded value.** Do not spend
further attempts trying to force it via C reshaping; it is a genuine
register-allocator/scheduler decision this project's toolchain makes based
on state invisible at the source level (confirmed: reproducible neither in
isolation at matching byte offsets, with or without realistic surrounding
register pressure, nor via any of the five distinct C-level restructurings
tried). Flag it as a permuter target and move on -- consistent with, and
now with a fourth confirmed instance supporting, the standing guidance in
`docs/MATCHING-GUIDE.md`.

## HEAD FINDING, round 13: the reversed `SceneNode__FaceTarget` arguments are a DIRECTION FLAG

Runner echo (`Entity__MoodCue115`, `Entity_d`) and runner bravo
(`Entity__MoodCue81`, `Entity_e`) each independently flagged a
`SceneNode__FaceTarget` call site whose first two arguments are swapped relative
to every other known site. Both verified it against raw disassembly. The
head then surveyed **every** call site in the executable, and the swap is
not an outlier convention -- it is perfectly correlated with the FOURTH
argument:

| `$a3` | `$a0` | `$a1` | sites |
| --- | --- | --- | --- |
| 0 | `this` | `this->unk94` | 9 in asm + 19 already matched as C |
| 1 | `this->unk94` | `this` | 2 (the two flagged sites) |

Nine of nine `a3 == 0` sites pass `(this, unk94)`; two of two `a3 == 1`
sites pass `(unk94, this)`. Every matched C call site in `src/Entity_b.c`,
`src/Entity_c.c`, `src/Entity_d.c` and `src/Entity_e.c` is
`SceneNode__FaceTarget(this, this->unk94, 1, 0, 0)` -- byte-verified, so those 19
are evidence, not transcription.

Reading: `SceneNode__FaceTarget` is **symmetric in its first two parameters**, and
`$a3` names which way round the operation runs. The caller pre-swaps the
operands rather than the callee branching on the flag.

**Consequences, and the first one is why neither runner should have
retyped anything:**

- The extern signature is CORRECT as it stands. Both flagged sites were
  left untouched, which was the right call: this is not one site
  disagreeing with the type, it is two parameters that each accept either
  object.
- **It is a hypothesis about TYPES, not a conclusion.** `this` is an
  `Entity *` and `this->unk94` is an `Unk94Obj *`, and `Entity.h` records
  that `Unk94Obj` is explicitly NOT another `Entity` despite sharing the
  `+0x14 EntityPos *` convention. If both fit either slot, the real
  parameter type is most likely a COMMON BASE the two share rather than
  `Entity *`. That is worth resolving with `tools/classtable.py` against
  both objects tables before anyone declares it, and it is NOT resolved
  here.
- Do not "fix" a swapped site back to the common order. It is real.

Method note: two runners in different units flagging the same oddity is
what promoted this from a curiosity to a survey. A single sighting reads
as a transcription error; the second one is what justifies the corpus
sweep, and the sweep is what found the flag correlation that neither
sighting could see alone.

## Round: permuter pass (runner delta) -- IN PROGRESS, interim update

**`--debug` against the report's exact preserved body does NOT show the
"one insertion + one deletion, score 200" signature this class of residue
is supposed to give.** Measured base score: **465**. Breakdown:

```
Stack Differences:             0  (1)
Branch Differences:            0  (1)
Register Differences:          9  (5)
Reorderings:                   2  (60)
Insertions:                    2  (100)
Deletions:                     1  (100)
```

**This is not a scaffold artifact.** Verified by temporarily building this
report's exact preserved C into the real `src/Entity_e.c`, running the real
oracle, and diffing `objdump -d` output instruction-for-instruction against
`target.o` with addresses stripped (so an address-drift illusion can't
produce a false match): there are genuinely **TWO** divergences from
retail, not one.

1. **The originally-documented one** (unchanged): retail materializes
   `li v1,-1` ahead of three stores (`out->unk20`/`unk34 = -1`); this body
   collapses it to a single reused register.
2. **NOT in the original report.** In the `mood != 1` (else) branch,
   retail schedules `li a1,0x1` (the literal `1` argument to
   `this->unk94->methods->slot130(this->unk94, 1)`) as the delay slot of
   that call's own `jalr`, and defers `mod = 0` (`move s1,zero`) all the
   way to the delay slot of a LATER branch (`bne v1,v0,...` testing
   `this->unk44 == 2`). This report's C instead has GCC hoist `li a1,0x1`
   up into the delay slot of the EARLIER `bne s2,v0` branch (the
   mood-dispatch test, where the value is dead/overwritten immediately by
   `move a1,zero` for the `slot48` call) and place `mod = 0` immediately
   after the `slot130` call instead. Same instructions, same values,
   genuinely different POSITIONS across two different basic blocks. This
   is the permuter's "Reorderings: 2" bucket.

Confirmed via raw `mipsel-linux-gnu-objdump -d` on both `target.o` and a
built `debug_compiled_object.o`, comparing mnemonic+operand text with
addresses stripped -- everything else in the function, before and after
both divergences, is byte-identical, so these two are the ENTIRE residue
(the report's word count and this `--debug` score describe the same two
divergences, just measured differently).

### Permuter progress

`tools/setup-permuter.sh Entity__MoodCue81 <this report's preserved body>`,
then `-j 6 --stop-on-zero --best-only`, bounded by `timeout 600`.

**Best found so far (still running at time of this update): score 230**,
via `permuter-work/Entity__MoodCue81/output-230-1/source.c`. The winning
mutation: a chained assignment plus a store reorder in the
`out->unk4 % 20 == 0` block --

```c
out->unk1C = 0x17;
out->unk10 = 0;
out->unk20 = -1;
out->unk44 = (out->unk30 = 0x17);
out->unk34 = -1;
out->unk48 = -2;
```

Confirmed via `objdump` that this candidate closes **divergence #1
completely, byte-for-byte** -- the `li v1,-1` now appears exactly where
retail has it, reused for both `unk20` and `unk34`. **Divergence #2 (the
`li a1,0x1` / `move s1,zero` cross-block swap) is still open** in this
230-point candidate; inspection says it accounts for the entire remaining
score.

Preserved verbatim (includes the project's typedefs/struct declarations
that `strip_other_fns.py` pulls in; positioned to compile in place of the
current `INCLUDE_ASM`):

```c
#if 0
extern u8 D_80089E08[];

void Entity__MoodCue81(Entity *this, EntityMoodHandlerArg *out) {
    s32 mod;
    s32 mood;

    if (this->unkFC == 0 && (rand() & 1)) {
        this->unk44 = rand() % 3;
    }
    if (this->unk44 != 0 && this->unkF4 != 0) {
        if (out->unk4 >= 0x3D) {
            out->unk4 = 0;
        }
        if (out->unk4 % 20 == 0) {
            out->unk1C = 0x17;
            out->unk10 = 0;
            out->unk20 = -1;
            out->unk44 = (out->unk30 = 0x17);
            out->unk34 = -1;
            out->unk48 = -2;
        } else if (out->unk4 % 20 == 0xE) {
            out->unk1C = -2;
            out->unk30 = -2;
            out->unk44 = -2;
        }
        SceneNode__FaceTarget(this, this->unk94, 1, 0, 0);
        mood = this->unk44;
        if (mood == 1) {
            this->methods->slot48(this, 0, D_80089E08);
            mod = -0x176;
            if (this->methods->slot144(this, this->unk94) < 0x200) {
                this->methods->slot160(this);
                this->unk44 = mood;
            }
        } else {
            this->unk94->methods->slot130(this->unk94, 1);
            SceneNode__FaceTarget((Entity *)this->unk94, this, 1, 1, 0);
            mod = 0;
            if (this->unk44 == 2) {
                mod = -0x60;
                if (this->methods->slot144(this, this->unk94) < 0x960) {
                    this->unk44 = 0xB;
                    this->methods->slot30(this, 0xC);
                }
            }
        }
    } else {
        out->unk10 = this->methods->slot148(this);
        if (out->unk4 % 22 == 0) {
            out->unk1C = 0x1C;
        }
        if (this->unkFC >= 0x1F5) {
            SceneNode__FaceTarget(this, this->unk94, 1, 0, 0);
        }
        if (this->methods->slot144(this, this->unk94) < 0x800) {
            this->unk94->methods->slotC4(this->unk94, -0x800, 0);
        }
        mod = -0x14;
    }
    this->methods->slotD0(this, mod, 1);
    if (this->unk28 != 0) {
        this->methods->slotCC(this, -0xC8, 0);
    }
}
#endif
```

Still `INCLUDE_ASM` -- not byte-exact yet. Search ongoing at time of this
commit; a follow-up update will record the final permuter exit code
(124/0/137), iteration count, and whether divergence #2 closed.

### Proposed learning (supersedes this report's earlier one)

**A near-miss report's claimed word-count residue is a hypothesis about
COUNT, not about the number of distinct divergences.** This function's
prior report correctly counted "1 missing word" but that arithmetic
coincidence (216/217) hid a SECOND divergence of equal size elsewhere in
the same function -- the asm-differ read that concluded "everything past
this point is byte-identical, just shifted" was not re-checked after the
shift, and a second 4-byte swap between two basic blocks reads identically
to "uniformly shifted" under a word-count diff that does not compare
content, only position. Always cross-check a claimed single-residue
function against `permuter.py --debug`'s penalty breakdown before trusting
"the rest is untouched" -- `Reorderings`/`Insertions`/`Deletions` counts
that don't collapse to a clean 100+100 are a sign there is more than one
divergence, regardless of what the word-count arithmetic suggests.

### Search concluded: still a STALL, divergence #1 CLOSED, divergence #2 open

**Iterations: 30485. Best score: 230 (down from base 465), never reached
zero.** Lowest score observed across the entire run was 230 (confirmed by
scanning every `score = N` emitted plus every `found new best score!`
line) -- the search plateaued there from iteration ~526 onward and never
improved further in the remaining ~30000 iterations.

**Exit code: could not be read directly** -- the `echo "permuter exit=$?"
>> log` appended after the bounded `timeout 600` never appeared in the
log file (confirmed by byte-inspecting the file's end: it stops mid­-
warning-text with no trailing echo line at all), which is a defect in
this run's own logging, not a search anomaly. Circumstantial evidence is
unambiguous that this is a **124** (timeout-enforced kill), not a 0
(`--stop-on-zero` never fired -- no zero was ever reached) or a 137
(external kill): the run's wall-clock duration matches the 600s bound
almost exactly, the last logged iteration (30485) is mid-computation with
an actively fluctuating score, immediately followed by a Python
`multiprocessing.resource_tracker` "leaked semaphore" warning -- the
textbook signature of workers being SIGTERM'd mid-flight rather than
exiting cleanly.

**Translated the 230-point candidate to idiomatic C and re-verified with
the real oracle** (built into `src/Entity_e.c` in place of the
`INCLUDE_ASM`, `./build-and-verify.sh`, `funcdiff.py`): the chained
assignment `out->unk44 = out->unk30 = 0x17;` (idiomatic form of the
permuter's parenthesized `out->unk44 = (out->unk30 = 0x17);`) plus the
store reorder shown above. **Confirmed this closes divergence #1
completely and only divergence #1** -- `funcdiff` moved from 69/217 to
**77/217** (an in-range improvement of exactly 8 words). `./build-and-verify.sh`
confirms the whole-image SHA1 still fails (as expected -- divergence #2
remains), so `INCLUDE_ASM` was restored; no non-matching C was left in
`src/`.

**This function is NOT permuter-exhausted.** 30485 iterations at
`-j 6` under machine contention (four other runners' searches sharing the
same 32 cores, confirmed via `ps`) is a real but bounded sample of the
mutation space, and the search was producing genuine progress throughout
(465 -> 460 -> 430 -> 250 -> 230 across the run, each a real new-best),
not simply stuck at the starting point. The remaining residue
(divergence #2, the cross-block `li a1,0x1`/`move s1,zero` scheduling
swap) is a different mutation shape than the one that closed divergence
#1 (a value/assignment-expression change vs. a cross-basic-block
statement-ordering change) and may respond to a longer run, repeat
attempts with a different random seed, or a manually-added hint (e.g.
explicit early materialization of the `mod=0`/`a1=1` values) that a
future attempt should try before escalating this as a toolchain-level
scheduling question.

**Best C now on file for the next attempt** (217 words, 77/217 measured
in-range, closes exactly one of the two known divergences -- supersedes
the report's original best-reached body above, which closed neither):

```c
#if 0
extern u8 D_80089E08[];

void Entity__MoodCue81(Entity *this, EntityMoodHandlerArg *out) {
    s32 mod;
    s32 mood;

    if (this->unkFC == 0 && (rand() & 1)) {
        this->unk44 = rand() % 3;
    }
    if (this->unk44 != 0 && this->unkF4 != 0) {
        if (out->unk4 >= 0x3D) {
            out->unk4 = 0;
        }
        if (out->unk4 % 20 == 0) {
            out->unk1C = 0x17;
            out->unk10 = 0;
            out->unk20 = -1;
            out->unk44 = out->unk30 = 0x17;
            out->unk34 = -1;
            out->unk48 = -2;
        } else if (out->unk4 % 20 == 0xE) {
            out->unk1C = -2;
            out->unk30 = -2;
            out->unk44 = -2;
        }
        SceneNode__FaceTarget(this, this->unk94, 1, 0, 0);
        mood = this->unk44;
        if (mood == 1) {
            this->methods->slot48(this, 0, D_80089E08);
            mod = -0x176;
            if (this->methods->slot144(this, this->unk94) < 0x200) {
                this->methods->slot160(this);
                this->unk44 = mood;
            }
        } else {
            this->unk94->methods->slot130(this->unk94, 1);
            SceneNode__FaceTarget((Entity *)this->unk94, this, 1, 1, 0);
            mod = 0;
            if (this->unk44 == 2) {
                mod = -0x60;
                if (this->methods->slot144(this, this->unk94) < 0x960) {
                    this->unk44 = 0xB;
                    this->methods->slot30(this, 0xC);
                }
            }
        }
    } else {
        out->unk10 = this->methods->slot148(this);
        if (out->unk4 % 22 == 0) {
            out->unk1C = 0x1C;
        }
        if (this->unkFC >= 0x1F5) {
            SceneNode__FaceTarget(this, this->unk94, 1, 0, 0);
        }
        if (this->methods->slot144(this, this->unk94) < 0x800) {
            this->unk94->methods->slotC4(this->unk94, -0x800, 0);
        }
        mod = -0x14;
    }
    this->methods->slotD0(this, mod, 1);
    if (this->unk28 != 0) {
        this->methods->slotCC(this, -0xC8, 0);
    }
}
#endif
```

Restored to `INCLUDE_ASM` -- not byte-exact.

### Proposed learning (a logging pitfall, distinct from the codegen one above)

**`echo "... $?" >> log` chained after a backgrounded, piped/redirected
`timeout` invocation is not guaranteed to reach the log file** even
though the shell's own reported exit status for the whole chain looks
normal (0, from the echo itself, which always succeeds) -- observed here
with a `-j 6` multiprocessing permuter run specifically, where leftover
worker processes and unflushed multiprocessing internals around a
SIGTERM-triggered shutdown are the suspected cause. Do not trust an
"exit code 0" reported by an outer harness for a compound `A; echo $?`
command as evidence about `A`'s own exit code -- that 0 belongs to the
echo, not to `A`. Corroborate with wall-clock duration and the log's own
tail content (iteration count still climbing vs. a clean stop message)
instead.

## Round: coordinator-directed type-axis consideration (runner delta)

Checked whether divergence #2 (the cross-block `li a1,0x1` / `move
s1,zero` scheduling swap) could be a type-axis issue. It is not the same
shape as the 62-function register-identity cohort: `--debug`'s own
breakdown classified it as `Reorderings: 2`, not `Register Differences`
-- the SAME register holds the SAME value in both my build and retail
(`$a1`=1 for `slot130`'s arg, `$s1`=0 for `mod`); what differs is WHICH
BASIC BLOCK contains each assignment, not which register was chosen.
Confirmed both values' consumer types are already exactly `s32`
(`Unk94Methods::slot130`'s second parameter and `EntityMethods::slotD0`'s
first parameter, both declared `s32` in `include/Entity.h`) -- there is
no declared-type ambiguity to vary. This divergence is a scheduler
delay-slot-filling decision, the same class as `Entity__MoodCue111`'s
redundant-rematerialization residue, not a candidate for the type/width
lever. No source changes.

## Round: coordinator-directed re-measurement with `--stack-diffs` (runner delta) -- CONFIRMED, no hidden frame-size component

Re-ran `--debug` on the exact same scaffold with `--stack-diffs` added,
per charlie's finding that plain `--debug` can falsely score zero on a
pure frame-size residue:

```
Stack Differences:             0  (1)
Branch Differences:            0  (1)
Register Differences:          9  (5)
Reorderings:                   2  (60)
Insertions:                    2  (100)
Deletions:                     1  (100)
[Entity__MoodCue81] base score = 465
```

**Identical to the original measurement -- `Stack Differences: 0` both
with and without the flag, same 465 total, same two-bucket
(`Reorderings`/`Insertions`+`Deletions`) split.** The two-divergence
finding above does not hide a third, frame-size component. This is also
independently corroborated by the REAL-oracle result already in this
report: closing divergence #1 alone moved `funcdiff` from 69/217 to
77/217 with `./build-and-verify.sh`'s own whole-image SHA1 check (which
does not go through the permuter's diffing at all, so it could not have
missed a frame-size issue) -- an 8-word improvement matching the
in-range word count exactly, with no separate "outside range but same
function" shift reported. Confirmed correct as stated.

## Round: found divergence #2 costs a REAL extra word, not a pure reorder (runner delta, round 19)

Rebuilt this report's exact "Best C now on file for the next attempt" body
(divergence #1 closed via the chained `out->unk44 = out->unk30 = 0x17;`
assignment, divergence #2 left as-is) directly into `src/Entity_e.c`, as a
check against the kind of misread found elsewhere this round
(`DreamSys__SoundCueCallback`, `DreamSys__AdvanceMoveCycle`). **The previous claim of "clean, +8
words in-range, no outside-range shift" does not hold up:**

```
$ tools/binutils/bin/mipsel-linux-gnu-objdump -d build/src/Entity_e.c.o
000016b4 <Entity__MoodCue81>:
...
    1a18:	00000000 	nop
```
`(0x1a18 + 4 - 0x16b4) / 4 = 0xda = 218 words` -- retail is 217
(`asm/nonmatchings/Entity_e/Entity__MoodCue81.s` header: `nonmatching
Entity__MoodCue81, 0x364` = 868 bytes = 217 words). **My build is 1 word
LONGER.** `funcdiff.py` confirms with its own outside-range warning
(107390 bytes) on this exact body, and `asm-differ`'s branch-target
column shows the drift starting as early as word 19 (`beqz v0,53b80` in
retail vs `beqz v0,53b84` in mine -- a target address 4 bytes further out,
because everything between that branch and its target is 1 word longer
in my build than in retail).

Tracing where the extra word actually lives: the region matches
divergence #2's own location exactly (starting at retail's `addu
a0,s0,zero` right after the mood-dispatch `bne s2,v0`, i.e. right where
this report already says GCC hoists `li a1,0x1` early instead of leaving
it in the `slot130` call's own delay slot). The permuter's `--debug`
bucket for this exact residue is `Reorderings: 2`, which reads as "same
instructions, different positions, same total count" -- **that reading is
wrong for the REAL oracle,** even though it may be an accurate
description of the specific edit-distance operations the permuter's
scorer chose internally. A "reordering" penalty bucket is not a proof of
equal length; only `objdump`'s word count and `funcdiff`'s drift check
are.

**Consequence: divergence #2 is best re-classified as a net INSERTION (my
C is missing whatever retail does that removes the extra word), not a
pure delay-slot-filler swap.** This changes what's worth trying next --
the existing "for the next attempt" guidance (a scheduling barrier around
the `slot130` call and `mod = 0`) is written for a same-length reordering
and was not attempted against this corrected understanding. No new C was
tried this round given the size of this re-derivation and this round's
remaining queue; `INCLUDE_ASM` restored, no source changes landed.

### Proposed learning

**A permuter `--debug` bucket name ("Reorderings") describes which
edit-distance OPERATION the scorer's internal diff algorithm chose, not a
guarantee about word count.** The only trustworthy word-count checks are
`objdump`'s literal instruction count and `funcdiff.py`'s outside-range
warning against the REAL compiled object -- both were available the whole
time this residue was classified as "same total instruction count, pure
reordering" across two prior rounds, and neither was run against the
divergence-#1-only body specifically (both rounds checked total counts
against DIFFERENT bodies: the round-13 single-divergence body, and the
final combined 230-point permuter candidate -- never the intermediate
"#1 closed, #2 still open" body that is what actually sits in this
report as "the next thing to attempt"). **When a report's own preserved
"next attempt" body has not itself been round-tripped through the real
oracle, do not trust its stated word-count/drift claims -- rebuild it.**

## ROUND 20 (runner echo): re-verified the round-19 finding directly; answering "independent or coupled"

Per the coordinator's standing warning about permuter numbers vs retail
words, rebuilt the exact "Best C now on file for the next attempt" body
(divergence #1 closed via the chained `out->unk44 = out->unk30 = 0x17;`
assignment plus the reordered stores, divergence #2 left untouched) fresh
into `src/Entity_e.c` myself, not trusting either this report's prose or
its predecessor's re-derivation. **Confirmed exactly what round 19
found:** `objdump` shows the compiled body is genuinely **218 words, 1
longer than retail's 217**, and `funcdiff.py` fires its outside-range
warning (107390 bytes). `asm-differ` shows the drift starting exactly
where this report already places divergence #2 (the branch target
`0x53b80` in retail vs `0x53b84` in this build, both testing the
`unk44==0`/`unkF4==0` early-exit path right after the `mood`-assignment
block that divergence #1's fix touches) -- confirmed independently, not
assumed from the prior write-up. Reverted immediately after the check;
`INCLUDE_ASM` restored, `git diff --stat` empty before continuing.

**Answering the coordinator's structural question directly: the two
divergences are INDEPENDENT, not coupled.** The evidence, now confirmed
twice (round 19's derivation and this round's from-scratch rebuild):

- Divergence #1's fix (the chained assignment + store reorder) moves
  `funcdiff`'s in-range score from 69/217 to 77/217 -- **exactly the +8
  words its own block accounts for**, with the outside-range byte count
  staying essentially where it started (still showing the SAME
  107390-byte drift figure both before and after divergence #1's fix,
  which is itself evidence the fix didn't touch anything downstream of
  its own block: if closing #1 had shifted or interacted with #2's
  region, the drift byte count would have changed by more than #1's own
  contribution). Divergence #1's block (the `out->unk4 % 20 == 0` store
  sequence) is upstream of and structurally unrelated to divergence #2's
  block (the `mood != 1` else-branch's `slot130`/`SceneNode__FaceTarget`
  sequence) -- they sit in different, non-overlapping basic blocks with
  no shared value or register between them (confirmed: divergence #1's
  fix touches only `out->`-relative stores; divergence #2 is entirely
  about `$a1`/`$s1` scheduling around two unrelated calls further down).
- Divergence #2, isolated this way (with #1 definitively closed), is a
  clean, single, self-contained residue: a genuine 1-word net insertion
  in the `mood != 1` branch, not entangled with anything in the
  `mood == 1` branch or the `out->unk4 % 20` block above it.

**This means the practical consequence the coordinator asked about is
settled: whoever attempts divergence #2 next can work on it in
isolation, using the round-19/this-round-confirmed 218-word body as the
starting point, without worrying that a fix might need to also touch
divergence #1's already-closed region or vice versa.** They are two
separate defects that happen to sit in the same function, not one
defect manifesting in two places or a fix-here-breaks-there coupling.

**Did not attempt to close divergence #2 itself this round** (time
budget went to confirming the independence question precisely, per this
round's efficiency instruction, rather than a fresh attempt on an
already-permuter-searched residue). Disposition unchanged: STALL,
divergence #1 closed (verified, isolable), divergence #2 open (confirmed
independent, reclassified as a net insertion per round 19, not yet
closed). `INCLUDE_ASM` restored; no source changes landed.

## Naming

Why `MoodCueNN`: the function's address sits in `gEntityMoodHandlerTable` row 81 (`asm/data/79528.data.s`, base 0x80089EB0, stride 0x10; row = (slot address - 0x80089EB0) / 0x10), confirmed by reading `disk/SLPS_015.56` directly rather than trusting address proximity (Entity_d/round 76 measured that row order does not track code address). Tier B: the row-to-function mapping is a compiler fact, not a guess, but which dream object or mood state each row represents is not established -- the row number is kept decimal and zero-padded so the names sort in table order, same convention as Entity_d.

**This handler also occupies row 120** of `gEntityMoodHandlerTable` (same `handler` word at both `0x80089EB0+0x10*81` and `0x80089EB0+0x10*120`; the row's other three words -- data0/data1/data2 -- differ between the two rows, so it is one function shared by two distinct mood-row configurations, not a naming collision). Named for its lower/first row per the existing convention; not a second name.

**`D_80089E08` left unnamed this round.** s16-pair-decoded it reads (8,7, 8,7, 8,7, 1,1) -- X=Y=Z=8/7, a uniform ~1.14x enlarge, but 8/7 is not one of the round ratios (1/2, 6/1, 3/1, ...) any named `SCALE_*` table uses, so there is no clean `SCALE_EIGHTSEVENTHS`-style name to give it with confidence.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `ENTITY_METHODS`) is unified as `Entity` in `include/Entity.h`: a Class65650 subclass whose table and object expand `CLASS65650_SLOTS`/`CLASS65650_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
