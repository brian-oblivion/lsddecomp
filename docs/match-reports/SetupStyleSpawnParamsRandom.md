# SetupStyleSpawnParamsRandom -- MATCHED round 64, 110/110 words, ins 0 / del 0, length exact (0x1B8)

> Renamed from `SetupStyleSpawnParamsA` on 2026-09-28 (tools/rename.py). Address 0x80055258.

> Renamed from `SetupStyleKind0Params` on 2026-09-23 (tools/rename.py). Address 0x80055258.

> Renamed from `func_80055258` on 2026-09-23 (tools/rename.py). Address 0x80055258.

## Round 64 (charlie) -- REVISIT, closed in ONE build

**REVISITED, round 64: MATCHED 110/110, ins 0 / del 0, whole-image oracle
green (`build exit=0`, `OK: build matches retail`); names/types USED -- the
fix was declaring a global as a SCALAR instead of an INCOMPLETE ARRAY.**

> **CAUSE CORRECTED mid-round, and the correction is the useful part.** My
> first write-up and my first broadcast both said "the DECLARED TYPE". The
> head reproduced three variants in isolation and found them byte-identical,
> which would have made that cause wrong. It is wrong as stated, and the
> four-point in-tree table below is what replaces it: the axis is **array vs
> scalar**, and BOTH element type and the cast spelling are measurably inert.
> The head's three variants were all arrays, so they could not see this axis;
> their real result is a genuine failure-to-reproduce, recorded as such under
> "Why the isolation reproducer disagrees" below.

### The revisit measurement, taken before changing anything

Spliced the inherited body in verbatim (it was preserved in
`src/world/ObjMStyleActor.c` in `#if 0`, and in this report as a ```c fence rather
than `#if 0` -- the brief's note was correct; the fence is now gone because
the function matches).

| | |
| --- | --- |
| `build exit=` | 2, no compile-error grep hits |
| word score | **5/110** (report title said 6/110, round 47 said 8/110) |
| **`insertions 5 / deletions 5`** | **positional skeleton diffs 102** |
| length | built 112 words vs retail 110 -- frame `-0x20` vs `-0x18`, extra `sw/lw $s1` |

So the recorded figure had drifted a third time (8 -> 6 -> 5) while the
CLASS had never been re-derived. The ins/del figure read as a pointer, per
the round-64 brief: **5/5 at a 2-word length difference is exactly the
prologue/epilogue pair**, i.e. the whole ins/del budget was accounted for by
the extra saved register, leaving 102 positional skeleton diffs that were
pure downstream shift. That is not a register-identity signature; it is one
structural cause with a long tail.

### The cause: an INCOMPLETE-ARRAY declaration, not register allocation

The inherited body accessed `sStyleSpawnOffsetX` through the unit's local view
`extern u8 sStyleSpawnOffsetX[];` plus a cast:

```c
*(s32 *) sStyleSpawnOffsetX = (rand() % 23) << 11;
if (rand() & 1) {
    *(s32 *) sStyleSpawnOffsetX = -*(s32 *) sStyleSpawnOffsetX;
}
```

That spelling makes the address an **array-decay value**, and cc1 2.6.3's
CSE promotes it into a genuine callee-saved register across the intervening
`rand()` calls. Read straight off the built words:

```
lui   $s0, 0x8009          /* 0980103c */
addiu $s0, $s0, -0x1f5c    /* a4e01026  -> $s0 = 0x8008E0A4 */
...
sw    $v0, 0x0($s0)        /* 000002ae */
lw    $v0, 0x0($s0)        /* 0000028e */
sw    $v0, 0x0($s0)        /* 000002ae */
```

Retail instead re-materialises the address absolutely at **each** of the
three accesses, which is what cc1 emits for a plain scalar global assigned
**by name**:

```
lui $at, %hi(sStyleSpawnOffsetX);  sw $v0, %lo(sStyleSpawnOffsetX)($at)
lui $v0, %hi(sStyleSpawnOffsetX);  lw $v0, %lo(sStyleSpawnOffsetX)($v0)
lui $at, %hi(sStyleSpawnOffsetX);  sw $v0, %lo(sStyleSpawnOffsetX)($at)
```

With `$s0` spent on the address, the magic constant `0xB21642C9` was pushed
to `$s1` (retail keeps it in `$s0`), which is the second saved register and
the 8 extra frame bytes. One cause, three visible symptoms.

### The four-point discriminator: ARRAYNESS, not element type and not spelling

Four in-tree variants of the same function, one build each, whole oracle
each. This table is the CAUSE claim; the paragraph above is only the
mechanism it implies.

| # | declaration | expression | result |
| --- | --- | --- | --- |
| 1 | `extern u8 sStyleSpawnOffsetX[];` | `*(s32 *) sStyleSpawnOffsetX = v` | **5/110 RED** -- frame `-0x20`, extra `$s1`, ins 5 / del 5, 102 skeleton diffs |
| 2 | `extern s32 sStyleSpawnOffsetX[];` | `sStyleSpawnOffsetX[0] = v` | **5/110 RED** -- byte-for-byte the SAME residue as (1) |
| 3 | `extern s32 sStyleSpawnOffsetX;` | `*(s32 *) &sStyleSpawnOffsetX = v` | **110/110 GREEN**, image green |
| 4 | `extern s32 sStyleSpawnOffsetX;` | `sStyleSpawnOffsetX = v` | **110/110 GREEN**, image green -- shipped |

Read off the table directly:

- **(1) vs (2): the ELEMENT TYPE is inert.** `u8` and `s32` arrays give the
  identical residue. So "the declared type" is not the cause, and any report
  saying so -- including this one's first draft -- is naming a cause the next
  round would act on wrongly.
- **(3) vs (4): the CAST SPELLING is inert.** Explicitly taking the address
  and casting it, which is the loudest-looking half of the change, costs
  nothing once the declaration is scalar.
- **(2) vs (3): arrayness is the whole axis.** The only difference between
  the red and green halves of the table is whether the symbol is declared
  `T D_X[]` or `T D_X`.

### Why the isolation reproducer disagrees, and why that is also a result

The head ran three variants through the pinned pipeline and got
byte-identical output, all of it retail's absolute-per-access form, including
a variant with calls between the accesses to make an address cache
profitable. **All three were array declarations**, which is rows (1) and (2)
of the table above -- so the reproducer correctly found (1) == (2) and could
not see the axis, because it never included a scalar.

That leaves a real, useful negative: in a five-line file cc1 2.6.3 at
`-O2 -G0` emits absolute-per-access for the ARRAY form too. So the address
caching is not something an incomplete-array declaration does on its own; it
needs this body's context -- five calls, three accesses to the same symbol
with calls interleaved, and a live callee-saved magic constant competing for
the same registers. **Within that context arrayness is the discriminating
source axis; outside it, it is invisible.** Both statements are measured and
they do not conflict. Per CLAUDE.md, a minimal reproducer that fails to
reproduce is itself the result: it proves the toolchain innocent and the
trigger contextual, which is what bounds the generalisation below.

The closing reproducer, if anyone wants it: add `extern int D_X;` (scalar) as
a fourth variant against the head's (3). I did not run it -- my four points
are in-tree and answer the question that was in front of me.

### The fix

One declaration -- array to scalar. The casts went with it for readability,
but per row (3) of the table they were not carrying any weight:

```c
extern s32 sStyleSpawnOffsetX;          /* was: extern u8 sStyleSpawnOffsetX[]; */
...
sStyleSpawnOffsetX = (rand() % 23) << 11;
if (rand() & 1) {
    sStyleSpawnOffsetX = -sStyleSpawnOffsetX;
}
```

`sStyleSpawnOffsetX` is a file-scope local view shared by four bodies in this unit,
so the retype is not local to this function. The two live pointer-take call
sites were rewritten `New_StyleEffect(..., sStyleSpawnOffsetX, ...)` ->
`New_StyleEffect(..., &sStyleSpawnOffsetX, ...)` (array decay and `&scalar` both
compile to `lui`/`addiu`, so this is free), and the two preserved `#if 0`
bodies that use the symbol (`StyleFillEffectKind3`, `SetupStyleSpawnParamsDayMod7`) were updated
to the same spelling. **`StyleFillEffectKind1` is live and already matched and
contains one of those call sites** -- the whole-image oracle is green after
the retype, so the retype cost it nothing.

### The matched body

```c
extern s32 gStyleSpawnYChoices[];
extern s32 sStyleSpawnOffsetX;
extern s32 sStyleSpawnOffsetY;
extern s32 sStyleSpawnOffsetZ;
extern u8 *sStyleSpawnRotation;
extern u8 gStyleSpawnRotations[];
extern s32 sStyleSpawnModelLayout;

void SetupStyleSpawnParamsRandom(void *arg0, void *arg1) {
    if (arg1 == 0) {
        arg1 = (void *) gStyleSpawnYChoices[rand() & 3];
    }
    sStyleSpawnOffsetY = (s32) arg1;
    sStyleSpawnOffsetX = (rand() % 23) << 11;
    if (rand() & 1) {
        sStyleSpawnOffsetX = -sStyleSpawnOffsetX;
    }
    sStyleSpawnOffsetZ = (rand() % 23) << 11;
    if (rand() & 1) {
        sStyleSpawnOffsetZ = -sStyleSpawnOffsetZ;
    }
    sStyleSpawnRotation = gStyleSpawnRotations + ((u32) rand() % 7) * 12;
    sStyleSpawnModelLayout = rand() % 5;
}
```

Everything rounds 46-48 recovered about the body -- the `& 3` (not `% 4`)
index, the shared `0xB21642C9`/`sra 4` `%23` reciprocal, the unsigned
`0x24924925` `%7` with stride 12, the plain `0x66666667`/`sra 1` `%5`, the
dead `arg0`, the `void` return -- was correct and is unchanged. Only the
declaration moved.

### What this retires, and why round 48's negatives were not wrong

Round 47/48 recorded two negatives on this residue and both stand:

1. `*(volatile s32 *) sStyleSpawnOffsetX` -- inert. Correct: `volatile` governs
   whether the MEMORY ACCESS may be elided or reordered, and the thing being
   cached here is the ADDRESS-OF computation, which is not the access.
2. A bare `__asm__("")` after the first store -- inert. Correct, and per
   CLAUDE.md HARD RULE 6's own test this was the right thing to try: a
   barrier moves instruction ORDER and this residue changed which register
   held a value.

Both negatives were aimed one layer too high. The address-take was not
something the optimiser did to a correct source shape; it was *in* the
source shape, spelled as a cast.

### The interesting asymmetry from round 48, now explained

Round 48 flagged, unexplained, that the textually identical
write/conditional-negate pattern on `sStyleSpawnOffsetZ` did **not** get an address
cached in either build, and guessed at register-pressure/CSE-table state.
The real reason is that `sStyleSpawnOffsetZ` was declared `extern s32 sStyleSpawnOffsetZ;`
and assigned by name all along, one statement below the `u8[]`-plus-cast
spelling. The two globals differed only in their declarations, and that
difference was the whole residue. The discriminator was sitting in the same
function.

### Proposed learning

**A scalar global declared as an INCOMPLETE ARRAY (`extern T D_XXXX[];`) is a
FALSE "register identity" generator: every reference is an array decay, i.e.
an address-take VALUE, and in a body with several accesses and calls between
them cc1 2.6.3's CSE will promote that address into a callee-saved register,
costing a saved register and 8 frame bytes and recolouring the whole body
downstream.** Declared `extern T D_XXXX;` the same accesses emit retail's
absolute `lui $at, %hi / sw %lo($at)` fresh at each one. **The lever is
ARRAY -> SCALAR in the declaration. It is NOT the element type and NOT the
cast spelling -- both measured inert in the four-point table above.** Cheap,
mechanical screen, one grep per unit:

```sh
grep -nE '\*\((s32|u32|s16|u16) \*\) *&?D_[0-9A-F]{8}|D_[0-9A-F]{8}\[0\]' src/<unit>.c
```

Every hit is a symbol being used as a single scalar through an array
declaration. Retype the declaration to the accessed scalar type and assign by
name. The retype is file-scope, so it touches siblings -- but a pointer-take
site (`D_X` -> `&D_X`) is byte-identical, which makes the conversion free in
practice. Confirmed here whole-image green with an already-matched sibling
(`StyleFillEffectKind1`) holding one of those sites.

**Discriminators, so this is not applied blind:**

- **The tell is an `addiu` that materialises a full DATA SYMBOL address into
  an `$s` register** (`lui $sN,0x8009; addiu $sN,$sN,%lo(sym)`) where retail
  has `%hi`/`%lo` pairs against `$at`. That is not a register-colour
  difference, it is an extra live value.
- **A second symptom is the frame growing by exactly 8 with one extra
  `sw/lw $sN`**, and `insertions N / deletions N` where N is fully explained
  by that pair. When ins/del is entirely prologue/epilogue, the residue is
  ONE structural cause, not a diffuse allocation difference.
- **Look for a sibling global in the SAME function that already gets retail's
  shape.** Here `sStyleSpawnOffsetZ`, one statement later, was already correct; the
  only difference between the two was the declaration. Round 63's "check
  whether a sibling loop in the SAME function already uses the correct idiom"
  applies to declarations too.
- **SCOPE: it needs the context, so verify in-tree, never in isolation.** In a
  five-line reproducer the array form emits absolute-per-access too (the
  head's measurement). This body has five calls, three accesses to the symbol
  with calls interleaved, and a competing live callee-saved constant. On a
  single-access or call-free body, expect this lever to be inert -- and a
  reproducer that fails to show it is not evidence against it.
- **It is above the permuter's search space.** A permuter mutates a body but
  never retypes a file-scope declaration, so round 48's cleared-but-unspent
  search would not have found this even at high iteration count. Same shape
  as round 63's local-COUNT finding: a PARAMETER of the search space, not a
  point in it.
- **A negative measured on ONE array spelling does not cover the scalar.**
  This is the generalisable half of the mid-round correction: three variants
  that differ in element type and cast spelling but agree on arrayness are
  ONE point on this axis, not three. When you sweep a declaration, make sure
  the sweep crosses the axis you are claiming.

**Second, smaller finding (procedural).** `funcdiff.py` decides "still
`INCLUDE_ASM`" by grepping the unit for `^INCLUDE_ASM(...)` without
preprocessor awareness, so while iterating with the body live under `#if 1`
and the `INCLUDE_ASM` parked in `#else`, it printed its "a full match means
NOTHING" warning on a genuine 110/110. Harmless here because
`build-and-verify.sh` went green and `nm` showed a real `T SetupStyleSpawnParamsRandom` in
the object, but the warning is the exact opposite of reassuring at the moment
you close a function. Reported, not acted on -- it is a tool heuristic, and
the safe iteration form is `#if 1` / `#else` / `#endif` with the guard
removed on the way to commit, which is what shipped.

## Prior rounds (measurements kept; the CLASS they recorded is retired)

- **Round 48 (alpha):** re-measured 6/110, Check 3 both halves run, AGREE
  (scaffold `Stack Differences: 8 (1)`, `Register Differences: 13 (5)`,
  `Insertions: 5 (100)`, `Deletions: 6 (100)`, base score 1173; in-tree
  rebuild showed the identical frame growth and extra `$s1` save). Not
  searched -- and the search it cleared was never needed, for the reason in
  the last discriminator above.
- **Round 47 (bravo):** cold fresh. Recovered the full body, every divisor
  idiom, the dead `arg0`, the `void` return; length exact at 0x1B8/110 words.
  First real diff at word 2, `sw $ra,0x14($sp)` vs `sw $ra,0x18($sp)` --
  correctly identified as an extra saved register in the prologue.

## Naming

**`SetupStyleSpawnParamsRandom`, tier B.**

One of two function-pointer targets `StyleFillEffectKind0` dispatches
through per iteration, selected when `sStyleDay % 7 != 0` (the more
common ~6/7 branch; the other is `SetupStyleSpawnParamsDayMod7`). Sets a cluster of
`gStyleE0*`-region scratch globals (spawn range/offset parameters consumed
by the `New_StyleEffect` allocator's `ctx` argument) from `rand()`. Named "A"
rather than by its selection condition because the condition is a plain
modulo test with no established game meaning -- naming it "the common one"
or "the 6/7 one" would assert more than the mechanics show. Renamed away
from an earlier `SetupStyleKind0Params`, which wrongly implied a link to the
`Obj876FC` kind-tag axis (`StyleFillEffectKind0`/`1`/`2`/`3`'s literal
first-argument values) -- this function has no such tag, it is selected by
an unrelated modulo test. MATCHED, 110/110, ins 0/del 0.

## Round 93 polish (delta, track 7)

### Naming

Parameters: `(LongVec3 *pos, s32 offsetY)` -- the type StyleFillEffectKind0's shared function pointer calls both setups with (round 93; `void *` before). `pos` is unused; `offsetY == 0` picks one of `gStyleSpawnYChoices`.

### Comments moved here from src/world/ObjMStyleActor.c

Verbatim as they stood before the round-93 comment pass (identifiers already carry this round's renames).

```c
/* MATCHED round 64 (charlie), 110/110, ins 0 / del 0, one build.  The
 * round-46..48 residue (an extra callee-saved register caching
 * `sStyleSpawnOffsetX`'s address, frame -0x18 -> -0x20) was NOT register identity:
 * `sStyleSpawnOffsetX` was declared as an INCOMPLETE ARRAY.  Every reference to
 * `extern T sStyleSpawnOffsetX[]` is an array decay, i.e. an address-take VALUE,
 * which cc1 2.6.3's CSE promotes into a callee-saved register across the
 * intervening `rand()` calls; declared `extern s32 sStyleSpawnOffsetX` it emits
 * retail's absolute `lui $at, %hi / sw %lo($at)` fresh at each of the three
 * accesses.  Four in-tree variants pin the axis to ARRAY vs SCALAR: the
 * element type (`u8[]` vs `s32[]`) and the cast spelling (`*(s32 *) &D_X`
 * vs `D_X`) are both measurably INERT.  Do not restate this as "the
 * declared type" -- that was the first, wrong, reading.
 * See docs/match-reports/SetupStyleSpawnParamsRandom.md. */
```
