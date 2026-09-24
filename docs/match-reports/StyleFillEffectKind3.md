# StyleFillEffectKind3 -- MATCHED 81/81 (round 76), lever: store the D_8008E0B0 slot through a pointer to a one-field STRUCT (scheduler alias rule), no shared `t`

REVISITED, round 76: MATCHED 81/81 in 12 builds, no permuter, no barrier; names/types used (local struct view `PtrBoxK3` for the store; D_8008E0xx names kept).

## Round 76 (charlie): MATCHED

**Preserved body rebuilt first**, verbatim from the `#if 0` block: `build
exit=2`, no compile-error grep hits, **79/81, `insertions 0 / deletions 0`
(positional skeleton diffs 2)** -- reproduced exactly, residue the 2-word
`$a2`/`$v1` colour of the else value (the shared `t`).

**The shared `t` was a workaround for the real residue, and the real residue
was aliasing, not registers.** Round 61 established (correctly) that the
78/81 body's problem was one instruction: the `*q = D_80087174` store has to
come AFTER the `lw a2, gStyleCueSelf` so it lands in the `jal` delay slot, and
that a plain `u8 **q` store is an opaque `(mem (reg))` the scheduler will not
move a global load across. Round 61 then forced the order by hoisting the
load into a variable by hand, which cost a register colour somewhere else.

The rule round 61 was one step from: gcc 2.6.3's `true_dependence` declares
**no conflict** between a MEM that is `MEM_IN_STRUCT_P` with a varying
address and a MEM that is a scalar at a fixed address. So store through the
pointer as a STRUCT FIELD and the load is free to schedule above it:

```c
typedef struct PtrBoxK3 { u8 *p; } PtrBoxK3;
PtrBoxK3 *q;
...
q = (PtrBoxK3 *) &D_8008E0B0;
q->p = D_80087174;
*arg0 = New_Class876FC((void *) 3, (u8 *) q - 0xC, (void *) gStyleCueSelf, arg1);
```

No `t` at all, and the else value goes back to being an anonymous
expression, which is what puts it in `$v1`. Family: **CSE/parameter walking
and first uses** is the nearest of round 75's five, but the lever itself is
new -- a TYPE on the store's lvalue controlling scheduler motion.

| build | body | score |
| --- | --- | --- |
| 1 | preserved (shared `t`) | 79/81, ins 0 / del 0 |
| 2 | struct view over D_8008E0A4..B0, direct field store `SP->fC = ...` | 73/81 (store goes direct `lui at`, loses retail's `q` register) |
| 3-5 | `p` reused as `q`, dedicated `t`, orders | 76/81, `li a0,3` falls out of the reorg-stolen join slot |
| 6 | no `t` at all, plain `u8 **q` | 75/81 |
| 7 | `q = &SP->fC` (struct view) + `&D_8008E0A4` arg | 78/81 -- CSE `related_value` reproduces `addiu a1,v1,-0xc` from `&base.fC`; residue back to the store placement |
| 9-10 | store inside a comma expression in arg 2 / arg 4 | 78/81, 73/81 |
| 11 | **`PtrBoxK3 *q`, `q->p = ...`, no `t`** | **81/81, `OK: build matches retail`** |
| 12 | same, with `q = &SP->fC` (struct view whose +0xC is a `PtrBoxK3`) and `&D_8008E0A4` as the argument | 81/81 too |

Build 11's form is the one committed (smaller local view, no struct laid
over four separately-declared externs). Build 12 is recorded because it says
the `- 0xC` was very probably `&struct` in the original: D_8008E0A4..C0
look like one spawn-parameter struct whose +0xC member is itself a struct.
Whole image green, `tools/check-nonmatching.sh` green.

### Proposed learning

**A store through a pointer that must SINK below a global load: make the
store a struct-field store.** gcc 2.6.3's scheduler cannot disambiguate a
`(mem (reg))` from anything, but `true_dependence` treats an in-struct MEM
at a varying address and a scalar MEM at a fixed address as non-conflicting.
Screen: "one store sits above a global load / argument load where retail has
it below (often in a `jal` delay slot), everything else exact." Wrapping the
pointee in a one-field struct (`q->p = x` instead of `*q = x`) is
byte-neutral otherwise. The converse holds: a load that must NOT cross a
store wants both sides scalar. This is what round 61's hand-hoisted `t`
was approximating, and the approximation cost a colour.

---

(Previous title: StyleFillEffectKind3 -- STALL, 79/81 words, length EXACT, insertions 0 / deletions 0, first real diff at word 60 (0x458C8 / vram 0x800550C8))

> Renamed from `func_80054FD8` on 2026-09-23 (tools/rename.py). Address 0x80054fd8.

REVISITED, round 61: 38/81 -> 79/81, and the round-47 verdict is retracted as
wrong in both of its two claims; names/types not relevant (unit has not passed
track 3).

## Round 61 (bravo) -- what the old title got wrong

Round 47's headline said **"38/81 words (length matches exactly, no drift)"**
and its body said the residue was *"a pure `arg0`/`arg1` register-colour swap
with ZERO drift"*. Rebuilt the preserved body once before trusting it:
**38/81 reproduced exactly**, so the SCORE was right. Everything the report
said about the CAUSE was wrong, and the way it was wrong is the reason this
function sat for fourteen rounds.

- `tools/funcdiff.py` reports **insertions 11 / deletions 11** on that body.
  It is not a register-colour residue at all.
- The equal LENGTH was **two defects cancelling**, not zero defects:
  - my build **cross-jumped** (tail-merged) the two arms' shared
    `D_8008E0C0[0] = ...` store, where retail duplicates it: **2 words short**;
  - my build **recomputed `lui`/`%lo` for `D_8008E0AC` at each of its four
    accesses**, where retail caches `&D_8008E0AC` in `$v1`: **2 words long**.

This is the round-58 lesson arriving with a worked instance: **equal length
does not imply 0/0**, and a report that infers "no structural difference" from
"length matches" has measured nothing.

## Round 61 fixes, in the order they were found

Each is a one-line source change; the score after each is the real in-tree
build (`./build-and-verify.sh`, then `tools/funcdiff.py`).

| # | change | score |
| --- | --- | --- |
| 1 | baseline: round 46's preserved body, rebuilt | 38/81 |
| 2 | `s32 *p = &D_8008E0AC;` for the clamp block | 38, length 79 (2 short), drift |
| 3 | **+ inline the `rand() % 3` instead of an `idx` local** | **66/81** |
| 4 | + `u8 **q = &D_8008E0B0;` for the tail store/arg pair | **71/81** |
| 5 | + signature `void **StyleFillEffectKind3(void **arg0, void *arg1)` with `*arg0 = ...; arg0++; return arg0;` | **78/81** |
| 6 | + ONE shared local `t` holding the else branch's computed value AND the hoisted `gStyleCueSelf` read (found by the permuter) | **79/81, ins 0 / del 0** |

Note step 2 on its own is the change round 47 recorded as a FAILURE ("16/81
with 106238 bytes of drift -- reverted"). It is not a failure; it is
**correct and incomplete**. On its own it removes two words and the function
goes 2 short, because the cross-jump is still there eating the other two. Add
step 3 and both defects close together and the whole `if`/`else` body --
words 12 through 63 -- matches EXACTLY. Round 47 measured the right thing,
read the drift as a verdict, and reverted the fix.

### Why step 3 is load-bearing (the mechanism, not the anecdote)

The `idx` local changed nothing semantically; it changed which registers the
`% 3` sequence landed in. Retail leaves the remainder in `$v0` and builds
`idx*3 + &D_8008721C` into `$v1`; the `idx` local made my build route the
remainder through `$a0` and build the sum in `$v0`. That single colour
difference is what enabled the cross-jump: **gcc 2.6.3 cross-jumps after
reload, comparing HARD registers**, so retail's `sw $v1, %lo(D_8008E0C0)($at)`
and my `sw $v0, ...` are different instructions in one case and identical in
the other. With identical ones, jump.c merges the tails and arm 1 loses its
own copy plus its `li $a0, 3`.

**A register-colour difference can change INSTRUCTION COUNT, not just
colour.** That is the generalisable finding here, and it inverts the usual
triage reflex ("colour residue = cosmetic, look elsewhere for the missing
word").

### Why step 5 is load-bearing

`$s0`/`$s1` really were swapped (retail `$s0 = arg0`, `$s1 = arg1`), and the
fix is the idiom the already-MATCHED sibling `StyleFillEffectKind1` in this same unit
already uses: `void **arg0`, `*arg0 = ...; arg0++; return arg0;`. Writing the
pointer walk gives `arg0` two more references, which lifts its allocno
priority above `arg1`'s and restores retail's colouring. Round 47 declined to
retry this residue at all, citing `TickStyle` having spent five
rephrasings on "the equivalent problem" -- but those five rephrasings were on
a different function with a different parameter shape, and the sibling with
the answer was forty lines up in the same file.

## The remaining residue after the search: 2 words of register identity

```
458c8  addu v1,v1,v0                 |  addu a2,v1,v0
458d0  sw   v1,%lo(D_8008E0C0)(at)   |  sw   a2,%lo(D_8008E0C0)(at)
```

Nothing else differs. `t`'s pseudo has two disjoint live ranges (the else
branch's value, then `gStyleCueSelf`), gcc 2.6.3 does no live-range splitting, so
one hard register serves both -- and it picks `$a2`, the third-argument
register the second use needs, where retail uses `$v1` and loads `$a2`
directly at the second use. Both spellings are the same instruction COUNT;
only the register differs. **That is a register-identity residue and a STALL
by project rule.**

Five further spellings were tried against it, all 79/81 and all
byte-identical, so the colour is invariant rather than merely unimproved:
`t` typed `s32` vs `void *`; the else value written as
`(s32) &D_8008721C[n*3]` vs `(s32) (D_8008721C + n*3)`; `t = gStyleCueSelf`
before vs after `q = &D_8008E0B0`; `t` declared first vs last. Reusing the
`s32 *p` pointer for all three roles instead of adding `t` regresses to
73/81.

## How the placement residue was closed (the 78/81 body's problem)

Three words differ, and they are one instruction moved:

```
retail                              built
458e8  addiu v0,v0,%lo(D_80087174)  458e8  addiu v0,v0,%lo(D_80087174)
458ec  lw    a2,%gp_rel(gStyleCueSelf) 458ec  sw    v0,0(v1)          <-- here
458f0  move  a3,s1                  458f0  lw    a2,%gp_rel(gStyleCueSelf)
458f4  jal   New_Class876FC          458f4  jal   New_Class876FC
458f8   sw   v0,0(v1)   (delay)     458f8   move a3,s1   (delay)
```

Same instructions, same registers, same multiset. The store has to sink below
the `lw` and the `move` so that `gas`'s `.set reorder` pulls it into the jal's
delay slot.

**The mechanism is a scheduler memory dependence, and it is MEASURED, not
reasoned.** A store through a pointer variable is `(mem (reg))` -- an opaque
address gcc 2.6.3's `sched_analyze` will not disambiguate -- so the
gp-relative load of `gStyleCueSelf` cannot hoist across it and the store cannot
sink below it. Two independent experiments prove it is this and nothing else:

- Write the store as a plain global (`D_8008E0B0 = D_80087174;`, a
  `(mem (symbol_ref))` the scheduler CAN disambiguate) and **the load hoists
  immediately** -- but the `q` pointer then folds away and the address
  argument regresses to `lui a1; addiu a1,%lo(D_8008E0A4)` (73/81).
- Keep the pointer store and hoist the load by hand instead
  (`t = gStyleCueSelf;` as a local placed BEFORE the store): the ordering becomes
  **byte-for-byte retail's**, delay slot included, with *zero* structural
  difference -- see the variant below.

For most of this session the two properties looked individually reachable and
not jointly reachable. **That was wrong, and the permuter is what falsified
it** -- see "Search" below. Sharing the `t` local with the else branch's value
gets both at once (79/81). The intermediate finding is kept below because its
MEASUREMENTS are what identify the mechanism, but read it as a waypoint, not
as a limit.

### Waypoint: the `t`-hoist with a dedicated `t` -- 75/81, PURE register colour, zero structural diff

```c
    q = &D_8008E0B0;
    t = gStyleCueSelf;                                    /* s32 t; */
    *q = D_80087174;
    *arg0 = New_Class876FC((void *) 3, (u8 *) q - 0xC, (void *) t, arg1);
```

Every instruction and every placement matches retail. The only diff is a
`$v0`/`$v1` exchange across exactly six words -- retail puts the POINTER in
`$v1` and the stored VALUE in `$v0`, this build does the reverse:

```
458d8  lui   v1,%hi(D_8008E0B0)     |  lui   v0,%hi(D_8008E0B0)
458dc  addiu v1,v1,%lo(D_8008E0B0)  |  addiu v0,v0,%lo(D_8008E0B0)
458e0  addiu a1,v1,-0xc             |  addiu a1,v0,-0xc
458e4  lui   v0,%hi(D_80087174)     |  lui   v1,%hi(D_80087174)
458e8  addiu v0,v0,%lo(D_80087174)  |  addiu v1,v1,%lo(D_80087174)
458f8  sw    v0,0(v1)               |  sw    v1,0(v0)
```

Introducing a DEDICATED `t` is what flips it: without `t` (78/81) the colours
are retail's, and sharing `t` with the else branch's value (79/81) keeps them
retail's while also fixing the placement. So this `$v0`/`$v1` exchange is an
artifact of the extra pseudo, not a property of the function -- do not carry
it forward as the residue.

Axes varied against it, all 75/81, all identical output (so the colour is
invariant to every one of them, not merely unimproved):

- statement order over `{q = &D_8008E0B0, t = gStyleCueSelf, val = D_80087174}` --
  every order that keeps the load before the store;
- declaration order of `q`, `t`, `val`;
- naming the stored value in a local vs leaving it anonymous;
- `t` typed `s32` vs `void *`;
- the address argument as `(u8 *) q - 0xC`, `(void *) ((s32) q - 0xC)`,
  `(void *) (q - 3)`;
- `p` declared at function scope vs inside the `else` block; `q` declared at
  function scope vs inside a trailing block;
- reusing ONE pointer variable for both `&D_8008E0AC` and `&D_8008E0B0`
  (73/81 -- worse, and the only one of these that moved the score).

Two further axes were tried on the 78/81 body and made it worse, recorded so
nobody re-spends them: a bare `__asm__("")` before the call (70/81), and an
initialiser-form declaration `u8 **q = &D_8008E0B0;` (11/81 -- the address
computation moves above the branch).

## Signature (corrected this round)

```c
void **StyleFillEffectKind3(void **arg0, void *arg1);
```

Widened from round 46's `void *StyleFillEffectKind3(void *arg0, void *arg1)`. The
forward declaration in `StyleBuildEffectSlots`'s block was widened to match; the call
site already passed a `void **`. This is not cosmetic -- it is what fixes the
`$s0`/`$s1` colouring (step 5 above).

## Gate 3 (all three checks run, round 61)

Base = the **78/81** body, i.e. the one that was current when the search was
launched, NOT the 79/81 body preserved below (which the search itself
produced). Anyone citing this table must re-run checks 2 and 3 on the new
body first: a negative is a verdict about the body it was measured on. For
reference, the 79/81 body's check-3 line is **insertions 0 / deletions 0**,
and its residue is register identity, which is a different class from the
reordering the search was launched against.

| check | result |
| --- | --- |
| 1. scaffold compiles and scores | OK, base score 180 |
| 2. scaffold `--debug --stack-diffs` | Stack 0, Branch 0, **Register 0**, **Reorderings 3**, **Insertions 0**, **Deletions 0** |
| 3. real build `funcdiff.py` ins/del | **insertions 2 / deletions 2** |

**Verdict: AGREE, search meaningful.** The numbers are not equal and that is
expected: both tools are looking at the SAME three-instruction reordering, and
`funcdiff`'s opcode-level alignment charges a moved instruction as one
insertion plus one deletion where the permuter's scorer charges it as a
reordering. What matters is that both say **zero register differences and no
instruction added or dropped**, which is exactly the residue `asm-differ`
shows. This is the first check-3 measurement this function has ever had --
round 47 ran checks (a) and (b) only, read 1 reordering / 6 insertions /
6 deletions off the scaffold against a body whose real residue it had
mis-recorded as zero-drift, and declined the search as a "mismatched
scaffold". **That decline was an artifact of the wrong baseline, not of the
scaffold**: on the corrected body the scaffold's insertions and deletions are
both 0.

### Search -- NOT CLOSED, but it moved the function

One bounded search, `-j 6 --stop-on-zero --best-only`, `timeout 900`, seeded
with the 78/81 body (base score 180). Ended on the timeout at **165218
iterations**, no zero. Best saved candidate scored **10** (two saved at 10,
plus 35 and 45) against the base's 180 -- i.e. it eliminated all three
reorderings and left only register-field differences.

**Translated in-tree and it is a real improvement: 78/81 -> 79/81, and
`insertions 2 / deletions 2` -> `insertions 0 / deletions 0`.** The candidate
was my own `t`-hoist (a local holding `gStyleCueSelf`, placed before the store,
which is what lets the load hoist past the opaque pointer store) plus **one
twist I had not tried: the SAME local also holds the else branch's computed
value.** That single shared pseudo is worth 4 words -- splitting it into two
variables (`u` for the else value, `t` for the load) drops straight back to
75/81 with `insertions 3 / deletions 3`, twice-measured, in both declaration
orders.

This is the "translate every promising candidate AND measure it in-tree"
half of Gate 3 paying off on a search that did not reach zero. **A permuter
run that times out is not automatically a negative** -- its best-saved
candidates are still a ranked list of source shapes nobody tried by hand, and
this one carried a lever (variable reuse across disjoint live ranges) that
no amount of the statement-order and declaration-order sweeping I had already
done would have produced.

## Preserved near-miss body (79/81, `#if 0` in `src/class_3bb8c_n.c`)

```c
extern s32 gStyleDecorVariant;
extern s32 gStyleColorTable;
extern u8 D_8008726C[];
extern u8 D_8008E0A4[];
extern s32 gStyleCueSelf;
extern s32 D_80087330;
extern void SetupStyleSpawnParamsA(void *arg0, void *arg1);
extern s32 D_8008E0C0[];
extern u8 *D_8008E0B0;
extern u8 D_80087174[];
extern s32 D_8008E0A8;
extern s32 D_8008E0AC;
extern u8 D_8008721C[];
extern s32 rand(void);
extern void *New_Class876FC(void *arg0, void *arg1, void *arg2, void *arg3);

void **StyleFillEffectKind3(void **arg0, void *arg1) {
    s32 t;
    s32 *p;
    u8 **q;

    SetupStyleSpawnParamsA(arg1, (void *) D_80087330);
    if (gStyleDecorVariant != 0 && gStyleColorTable == (s32) D_8008726C) {
        *(s32 *) D_8008E0A4 = 0xFFFF5000;
        D_8008E0A8 = -0x2000;
        D_8008E0AC = 0;
        D_8008E0C0[0] = (s32) (D_8008721C + 3);
    } else {
        p = &D_8008E0AC;
        if (*p > 0) {
            *p = -*p;
        }
        if (*p < -0x7800) {
            *p = -0x7800;
        }
        t = (s32) (D_8008721C + ((u32) rand() % 3) * 3);
        D_8008E0C0[0] = t;
    }
    t = gStyleCueSelf;
    q = &D_8008E0B0;
    *q = D_80087174;
    *arg0 = New_Class876FC((void *) 3, (u8 *) q - 0xC, (void *) t, arg1);
    arg0++;
    return arg0;
}
```

**Do not "clean up" the shared `t`.** Giving the two uses their own variables
is the obvious tidying and it costs 4 words.

### Proposed learning

**1. A register-colour difference can change instruction COUNT, because gcc
2.6.3 cross-jumps after reload.** `jump_optimize`'s cross-jumping compares
hard registers, so two `if`/`else` arms ending in the same store merge into
one copy when the stored value happens to land in the same register in both,
and stay duplicated when it does not. The observable is: **you are N words
SHORT and retail has a bare `j` into a shared tail that you reach by falling
through.** Discriminator against the existing "an arm that must JUMP has to be
written NOT-LAST" entry (DECOMPILATION_LEARNINGS 3a): there, retail's arm
jumps and yours falls through, and the fix is textual block order. Here BOTH
sides jump correctly; what differs is whether the tail is DUPLICATED, and the
fix is upstream, in whatever made the register colours agree. Look for the
duplicated instruction pair present in retail and absent in yours.

**2. Round 47's "cache a global's address in a local pointer is NOT uniformly
positive -- it depends on whether a CALL sits between the accesses" is
RETRACTED.** That learning was derived from exactly this function, from step 2
above, and it was derived from a drift number rather than from a diff. There
is no call between the three `D_8008E0AC` accesses and the pointer cache is
nevertheless correct -- it is precisely what retail does. The rule it was
generalising from (`StyleFillEffectKind1`'s and `StyleFillEffectKind2`'s wins) may still
hold on its own evidence, but this function is not an instance of it and must
not be cited as the negative half. **A source change that makes the score go
down while the LENGTH moves toward a known-missing construct is a partial fix,
not a refutation** -- read the diff before reverting.

**3. Reusing ONE local across two disjoint live ranges can be the source
shape.** Here a single `s32 t` holds the else branch's computed value and then
the `gStyleCueSelf` read; two separate variables cost 4 words. gcc 2.6.3 does no
live-range splitting, so a shared variable is a shared hard register, and that
is an allocation decision the source controls directly. This is the axis my
own hand sweep did not have -- I varied statement order, declaration order,
types and expression spelling, and every one of those keeps the variables
distinct. The permuter found it because `RandomizeReuseVariable` is one of its
mutations, which is a good reason to spend a search even on a body you have
already swept hard by hand.

**4. A store through a pointer variable blocks scheduler motion that a store
to a named global permits.** `(mem (reg))` is opaque to gcc 2.6.3's
`sched_analyze`, so no load can hoist across it and it cannot sink; `(mem
(symbol_ref))` is disambiguated against other symbols and moves freely. When a
residue is "one instruction is in the wrong place near a call, everything else
exact", check whether you introduced a pointer store that retail does not
have -- or, as here, whether retail has one and got the motion anyway, which
means the real difference is upstream of the schedule.

## Naming

**`StyleFillEffectKind3`, tier B.**

Called only when `gStyleVariant == 0`, from `StyleBuildEffectSlots`, as
the kind-0-exclusive finishing fill. Passes a literal kind argument of `3`
to `New_Class876FC` and appends exactly one slot (`*arg0 = ...; arg0++;
return arg0;`). STALL, 79/81 words (length exact), register-identity
residue only; naming from the literal `3` argument, confirmed the same way
as `StyleFillEffectKind0`/`1`/`2`.
