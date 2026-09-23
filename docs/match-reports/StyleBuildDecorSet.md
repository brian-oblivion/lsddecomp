# StyleBuildDecorSet -- STALL, 17/86 words (best), 1 word SHORT (map-measured 85 words), first real diff at word 15 (0x4508C / vram 0x8005488C)

> Renamed from `func_80054850` on 2026-09-23 (tools/rename.py). Address 0x80054850.

REVISITED, round 61: 12/86 and 6 words short -> 17/86 and 1 word short; the
two residues rounds 46/47 classified as uncontrollable register pressure were
both a single source-shape difference and are CLOSED; names/types not relevant
(unit has not passed track 3).

## Round 61 (bravo) -- the two "register-pressure" residues were one construct

Round 46 named two effects and round 47 re-affirmed both:

1. *"A stack-resident local (`paramA[1]`) gets reloaded from memory in retail
   immediately after an intervening `gStyleDecorVariant` check reuses the register that
   held it ... register-pressure-driven, not something a source rewrite
   obviously controls."*
2. Retail also reloads `gStyleDecorVariant` itself for the `== 2` test rather than
   reusing the value the guard already read.

Neither is register pressure and both are the same thing: **retail copies each
adjacent pair of globals into its local pair with a WHOLE-STRUCT assignment,
not two scalar stores.**

```c
typedef struct PairXY { s32 x; s32 y; } PairXY;

PairXY paramA, paramB;

paramA = *(PairXY *) &D_8008AB68;      /* NOT paramA[0] = ..; paramA[1] = ..; */
if (gStyleDecorVariant == 2) {
    paramA.y += 0x1E;
}
paramB = *(PairXY *) &D_8008AB70;
```

**Mechanism.** A struct assignment is a BLKmode `set`. gcc 2.6.3's `cse.c`
cannot reason about the extent of a BLKmode destination, so `invalidate()`
falls back to `invalidate_memory()` -- it throws away **every** cached memory
value in the hash table, not just the ones that could overlap. So the read of
`gStyleDecorVariant` the guard performed is no longer available for the `== 2` test
(reload 2), and the value just written into `paramA.y` is no longer available
for the `+= 0x1E` (reload 1). Written as scalar stores, the stack slot is a
fixed frame address, CSE keeps everything, and **both reloads vanish** -- which
is exactly the 3-word hole rounds 46/47 were staring at.

With that one change, retail's whole setup block matches byte for byte,
reloads, load-delay `nop` and all:

```
45070  lw    v0,%gp_rel(D_8008AB68)     45078  sw  v0,0x10(sp)
45074  lw    v1,%gp_rel(D_8008AB6C)     4507c  sw  v1,0x14(sp)
45080  lw    v1,%gp_rel(gStyleDecorVariant)   <-- reload 2
45084  li    v0,0x2
45088  bne   v1,v0,450a0
4508c   li   a2,0x1fff
45090  lw    v0,0x14(sp)              <-- reload 1
45094  nop
45098  addiu v0,v0,0x1e
4509c  sw    v0,0x14(sp)
```

## The other structural corrections this round

Round 46's body was 6 words short and 12/86. Four separate corrections, each
read straight off the raw `.s`:

| # | correction | evidence in retail |
| --- | --- | --- |
| 1 | the pair copies are struct assignments (above) | the two reloads |
| 2 | the trailing double dispatch reads the GLOBAL `D_8008E10C[0]` twice, not the cached `arr[0]` | `lui v1/lw %lo(D_8008E10C)` at 45160 **and again** `lui a0/lw %lo` at 45170, where the loop body uses `lw a1,0(s3)` |
| 3 | the array is walked with a separate pointer starting at `arr + 1`, not indexed by the loop counter | `addiu s0,s3,4` before the loop; `sw a0,0(s0); addiu s0,s0,4` inside |
| 4 | `arr` is assigned AFTER the first call, not before | `lui v1/addiu v1` sit at 450c4, past the `jal` at 450bc |

Correction 2 is the interesting one: the same array is referenced two
different ways in one function, and the difference is not cosmetic -- the
cached `void **arr` is `$s3` and costs one word per use, the global is a
`lui`/`lw` pair and costs two. Round 46 used `arr[0]` in the tail and was two
words short there.

Correction 4 needs a bare `__asm__("")` immediately after the first call to
hold, because the scheduler otherwise hoists the `lui`/`addiu` above the `jal`
(`arr` is callee-saved, so hoisting is legal). **Verified as the permitted
form of the lever**: removing it changes only WHERE those two instructions
sit, not which register anything lives in -- `$s3 = arr`, `$s0 = wp`,
`$s1 = step`, `$s2 = i` in both builds.

## What is left: one redundant `move`, and two reorderings

```
retail                          built
450c4  lui   v1,0x8009          450c4  lui   s3,0x8009
450c8  addiu v1,v1,%lo(E10C)    450c8  addiu s3,s3,%lo(E10C)
450cc  move  s3,v1            <-- MISSING
450d0  addiu s0,s3,4           450cc  addiu s0,s3,4
```

This is the project's documented one-word **redundant-`move`** class
(MATCHING-GUIDE, "A redundant `move` retail emits and you do not, same
register, same value, branch targets agreeing"), and it is the whole of the
1-word shortfall. Six spellings were tried and every one produced
byte-identical output, i.e. the residue is invariant to all of them rather
than merely unimproved:

- `arr = D_8008E10C; wp = arr + 1;`
- `wp = D_8008E10C; arr = wp; wp = arr + 1;`
- an extra `void **base;` temp: `base = D_8008E10C; arr = base;`
- `wp = D_8008E10C + 1;` (derive the walker from the global, not from `arr`)
- `wp[0] = obj; wp = wp + 1;` instead of `*wp = obj; wp++;`
- `obj` typed `ObjSlot4C *` instead of `void *`, dropping the cast at the
  dispatch

Two reorderings remain beside it, neither changing the instruction multiset:

- `li s2,1` / `li s1,3` land in different delay slots (2 words);
- in the loop body retail schedules `move a0,v0; addiu a2,sp,0x10;
  sw a0,0(s0); addiu s0,s0,4` where this build emits the store first
  (4 words). Note retail's store is of `$a0`, the copy that is also the
  dispatch receiver, where this build stores `$v0`, the raw return register --
  so `obj`'s pseudo coalesced with the argument register in retail and with
  the return register here.

A `for (i = 1; i < 0x12; i++)` loop instead of the `do`/`while` reaches
`insertions 3 / deletions 3` (vs 4/4) but rotates `$s0`/`$s1`/`$s2` away from
retail's assignment, so it is worse where it counts; the `do`/`while` body
below is the one to build on.

**Why the raw score is only 17/86 while the body is one word short.**
`funcdiff` compares by file offset, so a 1-word shortfall makes every word
after it mismatch, and a further nine `lw ..(gp)` immediates shift as a
side effect of the image moving. The honest figures are the two in the title:
**1 word short**, and the residue enumerated above. Do not read 17/86 as
"69 words wrong".

## Preserved near-miss body (1 word short, `#if 0` in `src/class_3bb8c_n.c`)

Needs, already present earlier in the unit in strict ROM order:
`extern s32 gStyleDecorVariant, D_8008AB68, D_8008AB6C, D_8008AB70, D_8008AB74,
gStyleTargetObj, gStyleColorTable;`, `extern void *D_8008E10C[];`,
`extern void *New_ClassEAC0(void *a0, void *a1, s32 a2);`, and the
`ObjSlot4C` / `ObjSlotAC` method-table views. `PairXY` is declared just above
the function in the unit.

```c
typedef struct PairXY PairXY;
struct PairXY {
    s32 x; /* +0x000 */
    s32 y; /* +0x004 */
};

void StyleBuildDecorSet(void) {
    PairXY paramA;
    PairXY paramB;
    s32 i;
    s32 s1;
    void **arr;
    void **wp;
    void *obj;
    ObjSlotAC *self2;
    void *result;

    if (gStyleDecorVariant == 0) {
        return;
    }
    paramA = *(PairXY *) &D_8008AB68;
    if (gStyleDecorVariant == 2) {
        paramA.y += 0x1E;
    }
    paramB = *(PairXY *) &D_8008AB70;
    i = 1;
    s1 = 3;
    obj = New_ClassEAC0(&paramB, (void *) gStyleColorTable, 0x1FFF);
    __asm__("");
    arr = D_8008E10C;
    wp = arr + 1;
    *arr = obj;
    do {
        obj = New_ClassEAC0(&paramB, (void *) (s1 + gStyleColorTable), 0x1FFF);
        *wp = obj;
        wp++;
        ((ObjSlot4C *) obj)->methods->slot4C(obj, arr[0], &paramA);
        s1 += 3;
        paramA.y += 3;
        paramB.y -= 7;
        i++;
    } while (i < 0x12);

    self2 = *(ObjSlotAC **) (gStyleTargetObj + 0xC);
    result = self2->methods->slotAC(self2);
    ((ObjSlot4C *) D_8008E10C[0])->methods->slot4C(D_8008E10C[0], result, &paramA);
}
```

## Gate 3 / permuter

**Not run this round, and deliberately.** One bounded search was spent on
`StyleFillEffectKind3` (this session's first assignment, one search at a time), and
by the time this body reached 1 word short that search was still live. This
function is now a much better search candidate than it was when rounds 46 and
47 recommended one: the base has gone from 6 words short with a mis-derived
structure to 1 word short with a residue that is a single well-understood
redundant `move`.

**Re-run checks 2 and 3 on THIS body before citing any earlier verdict about
it** -- a negative is a verdict about the body it was measured on, and the
body has changed completely.

### Proposed learning

**An unexplained RELOAD is evidence of a BLKmode assignment, not of register
pressure.** If retail re-reads a global it already read, or re-reads a stack
slot it just wrote, and there is no call in between, look upstream for a
struct or array assignment: gcc 2.6.3's `cse.c` responds to a BLKmode `set` by
calling `invalidate_memory()`, which discards every cached memory value rather
than the ones that could alias. Two or more adjacent globals loaded and then
stored to adjacent stack slots is the signature to look for -- written field
by field it is CSE-transparent, written as one struct assignment it is a
memory barrier for CSE.

The reason this is worth writing down as a rule rather than an anecdote is the
failure mode it replaces. "Register pressure" is unfalsifiable and terminal: it
names no construct, suggests no experiment, and both round 46 and round 47
stopped on it. The discriminator here costs one build.
