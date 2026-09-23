# SetupStyleSpawnParamsB -- MATCHED round 64, 87/87 words, ins 0 / del 0, length exact (0x15C)

> Renamed from `func_80055410` on 2026-09-23 (tools/rename.py). Address 0x80055410.

## Round 64 (charlie) -- REVISIT, closed by DELETING one named local

**REVISITED, round 64: MATCHED 87/87, ins 0 / del 0, whole-image oracle green
(`build exit=0`, `OK: build matches retail`); names/types NOT RELEVANT -- the
fix was the NUMBER OF LOCALS, and every name, type, constant and control-flow
shape rounds 46-48 recovered was already correct.**

### The revisit measurement

The inherited body did not rebuild at its recorded 25/87, for a reason that
has nothing to do with this function: earlier in the same session I retyped
the shared global `D_8008E0A4` from `extern u8 D_8008E0A4[]` to
`extern s32 D_8008E0A4` to close `SetupStyleSpawnParamsA`, and this body writes that
symbol too. So the first figure below is the inherited body under the
already-changed declaration, and the second is the inherited body's own
recorded state, which I recovered afterwards by experiment.

| body | `build exit=` | word score | `insertions / deletions` | length |
| --- | --- | --- | --- | --- |
| inherited, scalar decl (as rebuilt) | 2, no grep hits | **10/87** | **19 / 19**, 75 positional skeleton diffs | 90 words, **3 long** |
| inherited, array decl (round 48's state) | -- | 25/87 (round 46-48's figure) | scaffold ins 3 / del 1 | 89 words, 2 long |
| **shipped** | **0** | **87/87** | **0 / 0**, 0 skeleton diffs | **87 words, exact** |

`funcdiff.py` also raised its out-of-range warning (161570 bytes) on the
first row, correctly: at 3 words long the overrun shifts every data symbol in
the image by 0xC, which is why the diff showed `lw v0,0x732c(v0)` against
`0x7338(v0)` and `lw a1,0x46c(gp)` against `0x478(gp)`. **Those three
"wrong symbol offsets" were the length overrun, not three wrong symbols** --
worth saying because a `%gp_rel` offset that reads wrong is otherwise exactly
the shape of a `gp-symbols.txt` problem, and chasing it would have been a
detour into a resolved blocker.

### The cause: one named local, visible in a single asm-differ read

The body carried `s32 r` and reused it for all three `rand()` results:

```c
r = rand();
D_8008E0A4 = (r % 20) << 11;
...
r = rand();
D_8008E0B0 = D_80087174 + ((u32) r % 7) * 12;
r = rand();
D_8008E0B8 = r % 5;
```

`r` is one pseudo whose live range spans the calls, so cc1 2.6.3 cannot
coalesce `rand`'s `$v0` into it and emits an explicit copy after each call.
Straight off `tools/asm-differ/diff.py`, twice, at `45c3c` and `45cdc`:

```
TARGET                          CURRENT
                          >     move    a1,v0        <- retail has no such instruction
lui     v1,0x6666         r     lui     v0,0x6666
mult    v0,v1             r     mult    a1,v0        <- retail multiplies $v0 directly
```

Two literal insertions, and every downstream register in the body one colour
off as a consequence. **That is what rounds 46-48 filed as "pervasive
`$v0`/`$v1`/`$a0`/`$a1` temp-register renaming in a pattern too dense to be a
single swap".** The pattern was dense; the cause was two copies.

### The fix

Delete `r`; call `rand()` inline in each expression. `mod3` stays a local --
it has two genuine use points (`== 1` and `== 2`), which is the distinction
round 60's alias entry draws between a local that earns its name and one that
merely holds a value in transit.

```c
extern s32 D_8008E0A8;
extern s32 D_8008732C;
extern s32 D_8008E0A4;
extern s32 gStyleCounter;
extern s32 D_8008E0AC;
extern u8 *D_8008E0B0;
extern u8 D_80087174[];
extern s32 D_8008E0B8;

void SetupStyleSpawnParamsB(void *arg0, void *arg1) {
    s32 mod3;

    rand();
    D_8008E0A8 = D_8008732C;
    D_8008E0A4 = (rand() % 20) << 11;
    mod3 = gStyleCounter % 3;
    D_8008E0AC = 0xA000;
    if (mod3 == 1) {
        D_8008E0AC = -0xA000;
    } else if (mod3 == 2) {
        D_8008E0AC = 0x800;
    }
    D_8008E0B0 = D_80087174 + ((u32) rand() % 7) * 12;
    D_8008E0B8 = rand() % 5;
}
```

**This is the idiom the matched sibling `SetupStyleSpawnParamsA`, two functions
earlier in the same unit, already used** -- inline `rand()` in every
expression, no carrier local. Round 63's "check whether a sibling in the SAME
function already uses the correct idiom" generalises to a sibling in the same
unit, and this unit had the answer sitting in it for three rounds.

### Why 900s and 136,367 permuter iterations could not find this

Round 48 ran a bounded search (`-j 6 --stop-on-zero --best-only`, 900s,
136,367 iterations), reached 100 from an 850 base and never a zero. That
negative is **exactly** round 63's corollary, and this is a second
independent confirmation of it:

> a permuter mutates a body but never merges or deletes its locals, so local
> count is a PARAMETER of the search space, not a point in it. A validated
> high-iteration negative bounds the search, not the function.

The search could not delete `r`, so no number of iterations was going to
reach this body. Consistent with that, round 48's score-100 candidate went
the other way -- it *added* a local (`new_var2 = rand(); r = new_var2;`) and
regressed to 5/87 in-tree. The search was climbing away from the fix along
the only axis it had.

### An attribution result worth more than the match

The interim row in the table above is a trap I walked into and measured my way
out of, so it is recorded rather than quietly dropped.

The `D_8008E0A4` retype moved this function **25/87 -> 10/87** and 2 words
long -> 3 words long. Read at face value that is a regression, and the
standing rule (`DECOMPILATION_LEARNINGS` 3d, "Levers do not commute: if a
residue MOVES rather than SHRINKS, revert before the next") says revert it.
I could not, because `SetupStyleSpawnParamsA` requires the scalar declaration. Then
deleting the local closed the function *with the retype still in place*,
which makes it look as though the two levers combined.

**They did not.** One build settles it: I parked `SetupStyleSpawnParamsA` back on
`INCLUDE_ASM` (so it contributes retail's own bytes at exact length and
introduces no drift into this function's window), restored the array
declaration and the `*(s32 *)` casts, and kept the no-local body. Result:
**87/87, ins 0 / del 0**. So the retype is irrelevant to this function in
BOTH directions, and its apparent 25 -> 10 regression carried no information
at all.

The generalisable point: **a score move attributed to lever A is worthless
while the body still carries a known independent defect B.** The retype did
perturb the colouring of a body that was wrong for an unrelated reason, and
the resulting number was neither evidence for nor against the retype. The
standing "revert before the next lever" rule is undamaged -- reverting here
would also have matched -- but the corollary is new and is the one that
matters in a revisit: when an inherited body does not rebuild at its recorded
score because something *else* in the unit changed, treat the new number as
void rather than as a regression to explain.

### Proposed learning

**When a residue is filed as "pervasive/dense register renaming" AND the body
is N words long, look for N literal register-to-register COPIES before
accepting the class.** A `move rX,rY` that retail does not have is not a
colouring difference; it is an insertion, it is countable against the length
overrun, and it is visible in one `asm-differ` read. Here two `move a1,v0`
after two `jal rand` accounted for the "too dense to be a single swap"
verdict that had stood for three rounds and absorbed a 900s search.

The discriminator between the two readings is cheap and decisive:

- **Dense colouring with ins 0 / del 0 and exact length** -- a genuine
  allocation difference, and the LOCAL COUNT axis is the lever (round 63).
- **Dense colouring with nonzero ins/del and a length overrun** -- count the
  extra instructions and name them. If they are copies off a call's return
  register, the cause is a named local carrying that return value across a
  live range, and the lever is DELETING it. Round 59 already recorded "stop
  giving a call's return value its own named local"; this adds that **one such
  local reused across SEVERAL calls costs one copy per call**, so the
  insertion count tells you how many calls are feeding it before you change
  anything.

Round 48's own Check 3 had the number that distinguishes these -- scaffold
`Insertions: 3 (100)`, `Deletions: 1 (100)` -- and read it as confirmation of
the register class rather than as three insertions to go and name. That is the
round-64 brief's "ins/del is a pointer, not a verdict" in its exact intended
use.

## Prior rounds (measurements correct; the residue CLASS they recorded is retired)

- **Round 48 (alpha):** Check 3 both halves run, AGREE (scaffold
  `Stack Differences: 0`, `Register Differences: 90 (5)`, `Reorderings: 0`,
  `Insertions: 3 (100)`, `Deletions: 1 (100)`, base 850; in-tree rebuild
  reproduced 25/87 exactly). Permuter searched 900s / 136,367 iterations, best
  100, no zero. The score-100 candidate was hand-applied and regressed to
  5/87; the `mod3` retype inside it was isolated and measured inert. All of
  that stands -- and the search was on the one axis that could not reach the
  fix.
- **Round 47 (bravo):** widened the signature to two dead `void *` params
  (`StyleFillEffectKind0` dispatches this through a function pointer shared with
  `SetupStyleSpawnParamsA`, so the ABI slot is call-site-determined). Reproduced 25/87
  under the wider signature, confirming dead params cost nothing in the
  callee. **Correct and kept in the matched body.**
- **Round 46 (alpha):** recovered the structure and every value. The four
  `rand()` calls with the first result discarded; `0x66666667` with `sra 3`
  = `/20` not `/5`; `0x66666667` with `sra 1` = `/5`; unsigned `0x24924925`
  = `/7`; `0x55555556` = `/3`. Also found and fixed the `lui`/`ori`
  constant misread (`-0x6000` for `-0xA000`) that was costing a word, and
  recorded the still-valid learning that a `lui`+`ori` operand comment must be
  evaluated as the full 32-bit signed value because the wrong value is also
  the wrong instruction COUNT. **Every one of these was correct**; none of
  them was the residue.

## Naming

**`SetupStyleSpawnParamsB`, tier B.**

The other function-pointer target `StyleFillEffectKind0` dispatches
through (selected when `gStyleCounter % 7 == 0`, the ~1/7 branch). Same
scratch-global cluster as `SetupStyleSpawnParamsA`, different constants.
MATCHED, 87/87, ins 0/del 0.
