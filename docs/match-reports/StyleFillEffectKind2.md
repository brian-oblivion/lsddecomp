# StyleFillEffectKind2 -- MATCHED 79/79 (round 76), levers: one-field-struct store (scheduler alias), `% 6` not `(r/3)*6`, rand() result in its own local, and `val = x/20*20; if (x != val)` to stop jump.c's if/else-to-preset rewrite

REVISITED, round 76: MATCHED 79/79 in ~22 builds, no permuter; the one `__asm__("")` build was a diagnostic probe and is NOT in the committed body; names/types used (local struct view `S32BoxK2`; locals renamed `r`/`val`).

## Round 76 (charlie): MATCHED

**Preserved body rebuilt first**, verbatim from the `#if 0` block: `build
exit=2`, no compile-error grep hits, **49/79, `insertions 7 / deletions 7`
(positional skeleton diffs 28)** -- reproduced exactly.

Round 64's "levers marked spent" were about the arm order and the `idx`
local. Four separate things were wrong, found in this order:

| # | change | effect |
| --- | --- | --- |
| 1 | `D_8008E0BC = randval % 6;` | retail's magic is `0x2AAAAAAB` with no shift = signed **/6**; the preserved `randval - (randval / 3) * 6` computed a different function. 49 -> 51/79 |
| 2 | first `D_8008E0C0` store through `S32BoxK2 *slot` (`slot->v = ...`) | the whole `% 20` prefix (`lui 0x6666`, `lw gStyleCounter`, `mult`) now interleaves into the `% 3`'s `multu` latency exactly as retail. Same mechanism as StyleFillEffectKind3 this round: a store through a plain `s32 *` is an opaque `(mem (reg))` the scheduler will not hoist a global load above; an in-struct store at a varying address does not conflict with a scalar at a fixed address. Went 1 word short (the if/else, below). |
| 3 | `r = rand();` then `(u32) r % 3`, instead of inlining `rand()` or a separate `idx = rand() % 3` | inline put `slot` in `$s0`; `idx` put the remainder in `$a0`. With the rand RESULT in a local, both `$a2` (slot) and `$v0` (remainder) are retail's. |
| 4 | `val = (gStyleCounter / 20) * 20; if (gStyleCounter != val) val = D_80087430; else val = 0;` | **79/79**, `OK: build matches retail` |

### Step 4: the if/else was being rewritten by jump.c

Retail keeps both arms: `beq a1,v0 -> zero-arm; lw v0,D_80087430; j join;
nop; zero-arm: move v0,zero; join:`. Every `% 20` spelling with the load arm
first (if/else, `!= 0`, ternary either way, `switch` with `default` first)
compiled to `move a3,zero` ahead of the branch and a one-armed skip:
gcc's jump.c rewrites `if (c) x = a; else x = b;` into `x = b; if (c) x = a;`
whenever b is a constant/register and the test does not reference x. With
the zero arm first (round 64's body) the rewrite cannot fire (the else value
is a MEM) but the block order is then the reverse of retail's.

Diagnosed with a PROBE, not a fix: an `__asm__("")` after the then-arm's
assignment blocks the rewrite (it becomes the arm's last active insn) and
the function went **79/79**. That was not kept -- removing it changes which
register holds the value (`$a3` vs `$v0`), which is the rule-6 test for a
banned use, and it is not a C form anyway.

The C form uses the rewrite's own precondition: **the test must not
reference x.** Written `v0 = x % 20; if (v0 != 0)` the test references x but
CSE then knows `v0 == 0` on the else path and deletes `v0 = 0`. Written as
`val = (x / 20) * 20; if (x != val)` the test references `val`, CSE knows
only `x == val` on the else path, so the else arm's `val = 0` survives and
the rewrite never fires. It is also exactly retail's register use: the
`q*20` product is computed into `$v0`, the result register, which is what
`beq a1,v0` shows. **Do not "simplify" it back to `% 20`.**

Negatives measured on the way (all on the build-3 body): stores in both arms
(`sw zero` in the else arm, no cross-jump); `v0 = x % 20; if (v0 != 0)` (else
arm deleted); ternary `!= 0 ? D : 0` and `== 0 ? 0 : D` (rewritten);
`switch` with `case 0` first (zero-first order) and `default` first
(rewritten); sharing `r` for both `rand()` results (41 differing lines).

Whole image green, `tools/check-nonmatching.sh` green. `class_3bb8c_n.c`
now has no `INCLUDE_ASM`.

```c
typedef struct S32BoxK2 {
    s32 v;
} S32BoxK2;

void **StyleFillEffectKind2(void **arg0, void *arg1) {
    s32 r;
    s32 val;
    S32BoxK2 *slot;
    u8 **q;

    r = rand();
    slot = (S32BoxK2 *) D_8008E0C0;
    slot->v = (s32) (gStyleKind2Colors + ((u32) r % 3) * 3);
    slot++;
    val = (gStyleCounter / 20) * 20;
    if (gStyleCounter != val) {
        val = D_80087430;
    } else {
        val = 0;
    }
    slot->v = val;
    SetupStyleSpawnParamsA(arg1, (void *) D_80087330);
    q = &D_8008E0B0;
    *q = gStyleSpawnRotations;
    D_8008E0BC = rand() % 6;
    *arg0 = New_Class876FC((void *) 2, (u8 *) q - 0xC, (void *) gStyleCueSelf, arg1);
    arg0++;
    return arg0;
}
```

### Proposed learning

1. **`if (c) x = a; else x = b;` with a constant b and a load-first layout in
   retail: jump.c rewrote yours into `x = b; if (c) x = a;`.** Screen: retail
   has `j join; nop` after the then-arm and a separate `move reg,zero` (or
   `li`) block; yours has the constant set ABOVE the branch and no `j`. The
   rewrite is blocked when the TEST references x -- so look for a spelling
   where the compared value lives in the result variable (here
   `val = x/20*20; if (x != val)`, with retail computing the product into
   the result register as the tell). A test of `x` against 0 does not work,
   because CSE then deletes the else arm's `x = 0` as redundant.
2. **Read the magic constant before trusting a preserved body's arithmetic.**
   `0x2AAAAAAB` with `sra 31`/`subu` and no shift is `/ 6`; `0x55555556` is
   `/ 3`. The inherited body had carried a wrong `%` for three rounds.
3. The one-field-struct store (StyleFillEffectKind3's learning) applied
   here unchanged; it is a family, not a one-off.

---

(Previous title: StyleFillEffectKind2 -- STALL, length EXACT (0x13C / 79 words), 49/79 words, first real diff at word 11 (file 0x45948 / vram 0x80055148))

> Renamed from `func_8005511C` on 2026-09-23 (tools/rename.py). Address 0x8005511c.

## Round 64 (charlie) -- REVISIT: 16/79 and 1 word short -> 49/79 and length EXACT

**REVISITED, round 64: STALL, improved 16/79 (1 word short) -> 49/79 (length
EXACT), ins 7 / del 7, two levers landed and two firm negatives measured;
names/types USED -- one of the two levers was the parameter type.**

### The revisit measurement, taken before changing anything

Rebuilt the inherited body verbatim: `build exit=2`, no compile-error grep
hits, **16/79 reproduced exactly**, and `build/lsdde.map` puts the next
function `SetupStyleSpawnParamsA` at `0x80055254` against retail's `0x80055258`,
confirming the recorded "1 word short" precisely.

**`insertions 10 / deletions 10`, positional skeleton diffs 59.** Round 48
recorded the scaffold at ins 12 / del 13 and called it AGREE, which was right.
What nobody had done was read the diff for *what* those ten insertions were.

### Lever 1 (+26 words and the whole length gap): retail caches `&D_8008E0B0`

Round 47 filed the residue as "`arg1` capture timing" plus "instruction
scheduling interleave". The length arithmetic says otherwise, and it closes
exactly. From the disassembly, retail:

```
lui   s0,%hi(D_8008E0B0) ; addiu s0,s0,%lo(D_8008E0B0)   /* $s0 = &D_8008E0B0 */
lui   v0,%hi(gStyleSpawnRotations) ; addiu v0,v0,%lo(gStyleSpawnRotations)
jal   rand
 sw   v0,0x0($s0)                      /* the store rides rand's delay slot */
...
addiu a1,s0,-0xc                       /* D_8008E0A4's address, ONE word */
```

The inherited body wrote the global by name and re-materialised the second
address absolutely, which costs a word and wastes the delay slot:

```
lui v0,%hi(gStyleSpawnRotations) ; addiu v0,v0,%lo(gStyleSpawnRotations)
lui at,%hi(D_8008E0B0) ; sw v0,%lo(D_8008E0B0)(at)      /* 2 words */
jal rand
 nop                                                     /* delay slot wasted */
...
lui a1,%hi(D_8008E0A4) ; addiu a1,a1,%lo(D_8008E0A4)    /* 2 words */
```

Retail 6 words with a free store; built 7 words. **And the prologue was 2
words SHORT** because retail uses three saved registers (`$s0` for the cached
address, `$s1`/`$s2` for the parameters) and the build used two, so one
`sw`/`lw` pair was missing. `+1 - 2 = -1` -- the whole recorded "1 word short"
accounted for, with no scheduling hypothesis needed.

The fix is the idiom the sibling `StyleFillEffectKind3` in this same unit already
carries in its own preserved body:

```c
u8 **q;
...
q = &D_8008E0B0;
*q = gStyleSpawnRotations;
...
New_Class876FC((void *) 2, (u8 *) q - 0xC, (void *) gStyleCueSelf, arg1);
```

**16/79 and 1 short -> 42/79 and LENGTH EXACT**, skeleton diffs 59 -> 35.

Round 48's attempt 6 tried this exact lever (`u8 **ptr = &D_8008E0B0;`) and
recorded "regressed badly (8/79, +47000 more bytes of drift)". I cannot
reproduce that; on this body it is worth +26 words and the entire length gap.
Whatever differed, **the recorded negative on this lever is wrong and should
not be cited.** This is the one place where a stall report actively cost a
later round: the correct lever was on file, marked as tried and failed.

### Lever 2 (+7 words, and the entire prologue): the sibling's arg0 shape

Retail's parameter colours were still swapped -- retail `$s1 = arg0`,
`$s2 = arg1`; built the reverse. The matched sibling `StyleFillEffectKind1` takes
`void **arg0` and ends `*arg0 = ...; arg0++; return arg0;`, where this body
had the narrower `void *arg0` with `*(void **) arg0 = ...` and
`return (u8 *) arg0 + 4;`.

Retyping to the sibling's shape:

```c
void **StyleFillEffectKind2(void **arg0, void *arg1) {
    ...
    *arg0 = New_Class876FC((void *) 2, (u8 *) q - 0xC, (void *) gStyleCueSelf, arg1);
    arg0++;
    return arg0;
}
```

**42/79 -> 49/79**, skeleton diffs 35 -> 28, and the prologue is now
byte-exact through all seven words including both parameter copies and the
`sw $s0` in `rand`'s delay slot.

The mechanism is use COUNT, not the type: `*arg0 = x; arg0++; return arg0;`
mentions `arg0` three times where the cast form mentions it twice, which is
enough to reorder the allocno priority that decides `$s1` vs `$s2`. Consistent
with section 3d's "any change to the set of named locals is a new allocno" --
a parameter's mention count is on the same axis.

The caller `StyleBuildEffectSlots` is live and already matched, passes a `void **`,
and discards the return, so widening the forward declaration is free; whole
image verified green with it in place.

### Two firm negatives

**Negative 1: inverting the if/else arm order. Measured twice, on two
different bodies, both worse and both reintroducing length drift.**

Retail's block layout is unambiguous -- the `D_80087430` load is physically
FIRST and jumps (`j` with a `nop` delay), the `move v0,zero` arm is physically
LAST and falls through -- which by section 3a's rule ("GCC 2.6.3 gives the
fallthrough to whichever candidate is LAST in source order") says retail's
source is `if (gStyleCounter % 20 != 0) { v0 = D_80087430; } else { v0 = 0; }`,
the inverse of the inherited body. Applying it:

| body it was applied to | before | after |
| --- | --- | --- |
| inherited (16/79, 1 short) | 16/79 | **9/79**, ins 13/13, drift back |
| lever-1+2 body (49/79, exact) | 49/79 | **13/79**, ins 11/11, drift back |

**Why the rule does not reach it, measured:** cc1 collapses the inverted form
instead of emitting two blocks. `v0 = 0` is a single free instruction, so with
it in the `else` cc1 puts `move a1,zero` in the *branch* delay slot and lets
the load overwrite it conditionally -- no `j` at all. That evicts
`addiu a2,a2,4` (the `slot++`), which the inherited body had *matching*
retail in that same delay slot. So the arm inversion trades a correct delay
slot for a wrong block shape.

Retail avoids the collapse because its branch delay slot is already occupied
by `addiu a2,a2,4`. **The block-order rule's stated tell is "a bare `j` whose
delay slot carries REAL WORK"; retail's `j` here carries a `nop`.** That is
the discriminator this instance fails, and it is worth adding to the rule:
when retail's unconditional `j` has an EMPTY delay slot, the arm order is not
what is holding the layout -- something else is occupying the branch delay
slot and forcing the two-block form.

**Negative 2: deleting the single-use `idx` local is exactly INERT.**
`idx = (u32) rand() % 3;` used once, inlined into the store expression:
42/79 -> 42/79, ins 8/8, 35 skeleton diffs -- byte-identical, not merely
similar. This is a useful boundary on the lever that closed `SetupStyleSpawnParamsB`
in this same session and same unit: **that lever needs a local whose LIVE
RANGE CROSSES A CALL** (`r` there carried three `rand()` results across
intervening calls, costing one `move` per call). `idx`'s live range is
call-free -- `rand` returns and the value is consumed immediately -- so there
is no copy to eliminate and nothing to gain. Phrase the lever as "a local
carrying a value across a call", not "a call's return value with its own
name".

### Gate 3, all three checks re-run on the NEW body

PARALLEL-RUNS 3.5: "a negative is a verdict about the BODY it was measured
on: after a rewrite that moves the score, re-run checks 2 and 3 before citing
the old negative." The body moved 16/79 -> 49/79 and from 1-word-short to
length-exact, so round 48's negative does not cover it.

1. **Scaffold compiles and scores** -- yes (one expected implicit-declaration
   warning for `rand`).
2. **`--debug --stack-diffs`:** `Stack Differences: 0`, `Branch Differences:
   0`, `Register Differences: 24 (5)`, `Reorderings: 3 (60)`,
   `Insertions: 8 (100)`, `Deletions: 8 (100)`, **base score 1900** -- against
   round 48's 2899 on the old body. Stack and branch penalties are now zero,
   which is the prologue and the length gap closing.
3. **Real build:** `funcdiff.py` gives `insertions 7 / deletions 7`, 28
   positional skeleton diffs. **AGREE** -- both sides nonzero, same direction,
   within one. Search is meaningful.

The `Reorderings: 3` matches the diff by eye exactly: retail hoists
`ori v1,v1,0x6667`, `mfhi a0`, `lw a1,%gp_rel(gStyleCounter)` and `mult a1,v1`
into the `/3` `multu`'s latency window, *ahead* of the `lui/addiu` for
`&D_8008E0C0`, and finishes the `/3` chain afterwards. The build computes
`&D_8008E0C0` first and defers the `/20` `mult` until after the first store.

**One thing this rules out cheaply:** the `% 20 == 0` test must stay INLINE in
the `if`. Retail compares `a1 == (a1/20)*20` with no `subu`, which is cc1's
expansion of `x % 20 == 0` when the remainder is not otherwise live. A named
`mod20` local would force the `subu` and cost a word, so the "hoist it into a
local to move it earlier" reshape is structurally excluded before building it
-- the same style of screen round 62 got from reading the frame size.

**Search:** one bounded search on this body (`-j 6 --stop-on-zero
--best-only`, `timeout 900`, exit status to
`permuter-work/StyleFillEffectKind2/rc.txt`). Outcome in "Search outcome" below.
The residue is now purely statement order, expression form and register
colour -- `Insertions 8 / Deletions 8` with zero stack and branch penalty --
which is inside the permuter's search space, unlike the local-COUNT axis that
closed this unit's other two functions.

### Search outcome: bounded negative, and a well-defined ins-3/3 PLATEAU

`-j 6 --stop-on-zero --best-only`, `timeout 900`. **`rc.txt` = 124**, so the
wall-clock bound fired and the search is BOUNDED, not exhausted -- unlike
rounds 46-48's three searches in this unit, whose wrapping shell was reaped
before the status landed and which had to be treated as rc=124-equivalent by
inference. **145,225 iterations**, best score **430** against the 1900 base
(a 77% reduction, where round 48 reached 830/2899 = 71%), **no zero**.
Candidates written at 430, 495, 530, 555, 880 and 1550.

Both of the top two were UB-screened and measured in-tree, and they converge:

| candidate | permuter score | in-tree words | in-tree ins / del | in-tree length |
| --- | --- | --- | --- | --- |
| `output-495-1` | 495 | 28/79 | **3 / 3** | 1 word short |
| `output-430-1` | 430 | 34/79 | **3 / 3** | 1 word short |

`output-430-1` differs from `output-495-1` in where it puts the alias -- on
the SECOND store (`new_var = slot;` after `slot++`, then `*new_var = v0;`)
rather than the first -- plus a `do { } while (0);` in the `== 0` arm and the
inlining of `idx` I had already measured inert.

**Two independently-found candidates landing on the identical in-tree
signature -- ins 3/3 and exactly one word short -- is the informative
result.** It says the reachable part of this function's source-shape space
has a floor at ins 3/3 that costs one word, and 145k iterations never found a
way off it. That is a much sharper negative than "900s, no zero": it names
the plateau and its price. A future round should treat "get from ins 3/3 to
ins 0/0 while recovering that one word" as the open question, and should NOT
spend another undirected search -- the axis that is missing from the plateau
is one the permuter cannot reach, which on this corpus has twice now meant
local COUNT (this session's other two matches) or a declaration.

### Two INERT levers that are NOT jointly inert -- and the lowest ins/del reached

`output-495-1` made two changes, both using one extra local `new_var`:

- **(a)** an explicit alias for the first store:
  `new_var = slot; *new_var = (s32) (gStyleKind2Colors + idx * 3);`
- **(b)** a named local for the `gStyleCueSelf` load, passed as
  `New_Class876FC`'s third argument.

Screened for UB first (no use-before-init, no staleness across a back-edge,
each assignment consumed immediately -- it passes the forward-trace screen),
then measured in-tree, three builds:

| in-tree variant | word score | ins / del | length |
| --- | --- | --- | --- |
| baseline (levers 1+2) | 49/79 | 7 / 7 | exact |
| **(b) alone** | 49/79 | 7 / 7 | exact -- **byte-inert** |
| **(a) alone** | 49/79 | 7 / 7 | exact -- **byte-inert** |
| **(a) + (b) together** | 28/79 | **3 / 3** | **1 word short** |

**Both halves are individually byte-identical to the baseline, and their
combination is not.** The declaration of `new_var` is present in all three
variants, so this is not the "dummy unused scalar" case (already recorded
INERT): the difference is that in the combination the SAME pseudo carries two
unrelated values in two disjoint live ranges, which splits its lifetime and
re-allocates around it.

Section 3d already records the converse -- "a residue surviving two levers
INDEPENDENTLY has not been shown to survive their COMBINATION". This is the
mirror image and it is the more surprising half: **two levers each measured
byte-INERT are not jointly inert.** A single-use alias is copy-propagated
away (as the existing entry says); a local REUSED for two unrelated values
cannot be, and reuse is a property of the pair, not of either change.

**And the number to carry forward is `ins 3 / del 3`** -- the lowest reached
on this function, down from 7/7, at the price of one word. Per round 59's
corollary ("on a length-defective function insertions/deletions is the signal
and the word count misleads", where a correct fix ran 27/27 -> 11/11 -> 7/7
-> 0/0 while the word score went 61 -> 56 -> 91 -> 213), the 28/79 word score
is the misleading figure here and 3/3 is the informative one. **The next
round's best lead on this function is: start from the (a)+(b) combination at
ins 3/3 and find the one word it gives up**, rather than from the 49/79
baseline. I did not ship it, because 49/79-and-length-exact is the better
state to hand over under this project's rule that length-exactness comes
first, and because a body one word short re-poisons every later function's
window.

## Preserved near-miss body (49/79, length exact, `#if 0` in src/class_3bb8c_n.c)

```c
extern s32 gStyleCounter;
extern s32 gStyleCueSelf;
extern s32 D_80087430;
extern s32 D_80087330;
extern u8 gStyleKind2Colors[];
extern s32 D_8008E0C0[];
extern u8 *D_8008E0B0;
extern u8 gStyleSpawnRotations[];
extern s32 D_8008E0BC;
extern void SetupStyleSpawnParamsA(void *arg0, void *arg1);
extern void *New_Class876FC(void *arg0, void *arg1, void *arg2, void *arg3);

void **StyleFillEffectKind2(void **arg0, void *arg1) {
    s32 idx;
    s32 randval;
    s32 v0;
    s32 *slot;
    u8 **q;

    idx = (u32) rand() % 3;
    slot = D_8008E0C0;
    *slot = (s32) (gStyleKind2Colors + idx * 3);
    slot++;
    if (gStyleCounter % 20 == 0) {
        v0 = 0;
    } else {
        v0 = D_80087430;
    }
    *slot = v0;
    SetupStyleSpawnParamsA(arg1, (void *) D_80087330);
    q = &D_8008E0B0;
    *q = gStyleSpawnRotations;
    randval = rand();
    D_8008E0BC = randval - (randval / 3) * 6;
    *arg0 = New_Class876FC((void *) 2, (u8 *) q - 0xC, (void *) gStyleCueSelf, arg1);
    arg0++;
    return arg0;
}
```

The forward declaration in `StyleBuildEffectSlots`'s block is widened to match:
`extern void **StyleFillEffectKind2(void **arg0, void *arg1);`. That widening ships
(the whole image is green with it and `INCLUDE_ASM` restored) because it is
the correct signature regardless of whether the body is live.

### Proposed learning

**1. "Retail caches an address in a callee-saved register" and "retail
re-materialises it per access" are BOTH real shapes, they occur in adjacent
functions in one unit, and the source spellings are different.** This session
matched `SetupStyleSpawnParamsA` by STOPPING an address from being cached (an
incomplete-array declaration was producing the cache) and improved
`StyleFillEffectKind2` by FORCING one (a `u8 **q` local). Same unit, same symbol
group, opposite directions. So neither is a rule about the codebase; read
which one retail's own words show, then pick the spelling:

- retail `lui $sN / addiu $sN` on a data symbol, reused -> a POINTER LOCAL.
- retail `lui $at, %hi / sw %lo($at)` fresh per access -> assign the global
  BY NAME, and make sure it is declared a SCALAR, not an incomplete array.

The tell that a pointer local is wanted is a **second address derived from the
first by a small constant** -- here `addiu a1,s0,-0xc` for a symbol 0xC below
the cached one. One `addiu` where you emit a `lui`/`addiu` pair means retail
had a pointer you do not.

**2. The block-order rule needs its stated tell checked, not just its
symptom.** Section 3a's tell is "a bare `j` whose delay slot carries REAL
WORK", plus being short by a few words, plus a barrier having no effect. This
instance had the symptoms (short, layout visibly inverted) but retail's `j`
carries a **`nop`**, and arm inversion regressed it twice. Diagnosis:
retail's *branch* delay slot is occupied (`addiu a2,a2,4`), which is what
forces the two-block form; invert the arms and cc1 puts the cheap arm in that
slot instead and collapses the whole `if`. **An empty delay slot on retail's
`j` is a counter-indication to the arm-order lever**, and it is cheap to
check before spending a build.

**3. A stall report's per-lever negatives decay too, not just its score and
its class.** Round 48's attempt 6 recorded the `&D_8008E0B0` pointer local as
"regressed badly (8/79)". It is worth +26 words and the entire length gap.
Section 4 already says the CLASS goes stale unnoticed while the measurement
stays good; this adds that an individual lever recorded as FAILED is the most
expensive kind of stale entry, because it is read as a reason not to try
again. Cost here: 16 rounds. **On a revisit, re-derive the mechanism from the
disassembly before reading the attempts list** -- the attempts list is a
record of what someone did, not a proof of what does not work.

## Prior rounds

- **Round 48 (alpha):** Check 3 both halves run, AGREE (scaffold `Stack 24
  (1)`, `Register 39 (5)`, `Reorderings 3 (60)`, `Insertions 12 (100)`,
  `Deletions 13 (100)`, base 2899; in-tree rebuild 16/79). Permuter searched
  900s / 131,794 iterations, best 830, no zero; candidate at
  `permuter-work/StyleFillEffectKind2/output-830-1` left uninspected, and correctly
  so -- 830/2899 was a weak signal. That negative is superseded: it was
  measured on the 1-word-short body whose scaffold base was 2899, and this
  round's body bases at 1900 with zero stack and branch penalty.
- **Round 47 (bravo):** reproduced 16/79 and map-measured the 1-word gap
  (`0x138` = 78 words vs 79). Also found and fixed a real defect in this file
  -- the preserved body had never been inlined here as compilable source
  despite a note claiming it was.
- **Round 46 (alpha):** recovered the structure and all three division idioms
  -- unsigned `0xAAAAAAAB` `/3`, signed `0x66666667` with `sra 3` = `/20`,
  signed `0x2AAAAAAB` `/3` with the literal `randval - (randval/3)*6` form
  (not a plain `% 6`). **All correct and all unchanged.** Attempt 3's `slot`
  pointer-walk (3/79 -> 16/79) is also correct and still in the body: two
  array-element stores compile to two independent address computations, where
  retail reuses one base with a `+4`.

## Naming

**`StyleFillEffectKind2`, tier B.**

Called only when `gStyleVariant == 2`, from `StyleBuildEffectSlots`, as
the kind-2-exclusive finishing fill (the sibling of `StyleFillEffectKind3`,
mirrored for the other variant). Passes a literal kind argument of `2` to
`New_Class876FC`. STALL, 49/79 words (length exact), residue at ins 3/del 3
per the round-64 revisit.

## Track 4 (2026-09-26, round 88, charlie)

`gStyleEffectSlots` holds Class876FC objects (New_Class876FC), so the walking pointer is `Class876FC **` and the position `LongVec3 *`; `kind` is passed as a plain `s32` (was `(void *) N`), the params block as `(Class876FCParams *)` over the separately-declared D_8008E0A4.. symbols (one 0x24-byte Class876FCParams in the bytes; left as they are, a track 4b job), and gStyleCueSelf as the `SceneNode *` parent. Image byte-identical.
