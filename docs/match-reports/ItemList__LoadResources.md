# ItemList__LoadResources -- MATCHED (95/95, round 73)

> Renamed from `Class86F88__LoadResources` on 2026-09-26 (tools/rename.py). Address 0x80051f24.

> Renamed from `ItemList_3bb8c_j__LoadResources` on 2026-09-24 (tools/rename.py). Address 0x80051f24.

> Renamed from `func_80051F24` on 2026-09-24 (tools/rename.py). Address 0x80051f24.

Unit: `src/class_3bb8c_j.c`. `self` is `ItemList_3bb8c_j`.

## Semantics (established with reasonable confidence from the disassembly)

```c
#if 0
extern void *BuildFileName(void *out, void *a1, void *a2, void *a3);
extern ItemListHandle_3bb8c_j *New_TimImage(void *arg0);
extern ItemListHandle_3bb8c_j *New_ScreenSprite(ItemListHandle_3bb8c_j *arg0, void *arg1, s32 arg2);
extern s32 sStrSelect;
extern s32 D_8008AB1C;
extern s32 D_8008AB24;
extern s32 gItemListPanelRect;
extern s32 gItemListPanelPos;
extern s32 D_800116E4;

void ItemList__LoadResources(ItemList_3bb8c_j *self, void *arg1)
{
    s32 local[8];
    ItemListHandle_3bb8c_j *h;

    if (!arg1) {
        return;
    }
    if (self->unk50) {
        return;
    }

    h = New_TimImage(BuildFileName(local, &sStrSelect, &D_8008AB1C, &D_8008AB24));
    h->methods->slot78(h);
    self->unk50 = New_ScreenSprite(h, &gItemListPanelRect, 0);
    h->methods->slot4(h);
    self->unk50->methods->slot4C(self->unk50, arg1, &gItemListPanelPos);

    h = New_TimImage(BuildFileName(local, &D_800116E4, &D_8008AB1C, &D_8008AB24));
    h->methods->slot78(h);
    self->methods->slot8C(self, arg1, h, self->unk20, self->unk24, self->unk28);
    h->methods->slot4(h);
}
#endif
```

If (`arg1` non-NULL AND `self->unk50` not already set): builds a
16-byte-ish stack descriptor via `BuildFileName(&local, ...)` fed into
`New_TimImage`, producing an opaque "handle" (`ItemListHandle_3bb8c_j *`, the
same type `ItemList__ReleaseResources` -- matched this round -- also uses via
`self->unk50`). Calls the handle's own `slot78` (init?), stores a SECOND
derived handle into `self->unk50` via `New_ScreenSprite`, releases the FIRST
handle (`slot4`), then forwards `arg1` and a literal global pointer into
`self->unk50`'s own `slot4C`. Repeats the whole build (with a different
global, `D_800116E4` instead of `sStrSelect`) to make a THIRD handle,
which is passed into `self->methods->slot8C` (established this round)
alongside `arg1` and three of `self`'s own fields, then released.

This established `ItemListHandle_3bb8c_j`/`ItemListHandleMethods_3bb8c_j` (`slot4`,
`slot4C`, `slot78`) and `ItemListMethods_3bb8c_j::slot8C` (+0x08C, 6 args).

## Residue: pure register-identity rotation, NOT a size/instruction defect

The compiled length is EXACTLY 95 words on the very first attempt --
`funcdiff` reports no "differs outside range" warning, and a permuter
`--debug` run confirms it structurally: **0 reorderings, 0 insertions, 0
deletions, only 28 register differences** (base score 140, purely
`REGDIFF` penalties). Every mismatching word is the identical instruction
on a different register:

- Retail: the address of `D_8008AB1C` lives in `$s2`, `D_8008AB24` in
  `$s1`, and each of the two "handle" values in `$s0` in turn.
- This body: `D_8008AB1C` lands in `$s1`, `D_8008AB24` in `$s0`, and the
  handle in `$s2`.

A clean three-way rotation of the same three long-lived values across
the same three registers. `self` (`$s3`) and `arg1` (`$s4`) matched
retail exactly on the first attempt, so the parameter-facing part of the
allocation is right; only the LOCALLY-INTRODUCED values (the two global
addresses feeding every `BuildFileName` call, plus the handle) rotate
differently.

## What was tried

- Naming the two repeated global addresses as their own locals
  (`p1 = &D_8008AB1C; p2 = &D_8008AB24;`, reused across both
  `BuildFileName` calls instead of writing `&D_8008AB1C`/`&D_8008AB24`
  twice inline) -- no change at all (identical 75/95, byte-for-byte
  identical diff list). Register allocation is apparently indifferent to
  this particular source-level factoring.
- Permuter (`tools/setup-permuter.sh`, `-j 6 --stop-on-zero`, ~2.5
  minutes / ~7800 iterations): did not reach zero, but found a
  score-30 lead (down from the 140 base) whose only structural change
  from this body was caching `h->methods` into a separate local
  (`ItemListHandleMethods_3bb8c_j *hm = h->methods; hm->slot4(h);`) before the
  FINAL `slot4` call only. Translating that lead by hand back into the
  real toolchain reproduced NO improvement at all (still 75/95,
  identical diff) -- the permuter's own mutated/stripped scaffold and
  the real header-including build appear to score this differently, or
  the lead genuinely doesn't transfer. Did not have budget to
  investigate the discrepancy further (would need to diff the
  permuter's own `base.c` scaffold against the real compile to find
  what's different about its environment).

## Suggested next step

Given the residue is a THREE-WAY rotation with `self`/`arg1` already
correct, the lever is most likely in exactly WHICH new value gets
introduced FIRST after `self`/`arg1` are already live -- try writing the
FIRST `BuildFileName`/`New_TimImage` pair's result assignment before
computing `&D_8008AB1C`/`&D_8008AB24` at all (i.e. reorder so the
call happens before the two address-of expressions are bound to named
locals, forcing GCC to allocate the handle's register before the two
addresses' rather than after) -- not yet tried due to time. A longer
permuter run (the 7800-iteration run was still finding fresh non-zero
scores when it was stopped, not stalled at a plateau) is also a
reasonable next step given the debug penalty list is this clean (pure
register diffs, nothing else).

### Proposed learning

**A permuter lead that scores well against the permuter's own stripped
scaffold does not always transfer to the real build.** Confirmed here:
a score-30 candidate (down from 140) translated by hand back into
`src/class_3bb8c_j.c` and rebuilt through `./build-and-verify.sh`
reproduced the ORIGINAL 75/95 score exactly, no improvement. Worth a
`--debug` re-check of the CANDIDATE (not just the base) before trusting
a non-zero permuter score as a real lead -- this project's existing
guidance ("Always run `--debug` first and check the base score...")
covers the base case but not this one.

---

## Round 18 (echo) — two more declaration-order variants, both fully inert

Confirmed this is the SAME class CLAUDE.md/`docs/DECOMPILATION_LEARNINGS.md`
document via `StageMap__BuildFootprintRects` (7 variants, zero movement) and
`TaskObjF__WriteMemcardSaveFile` (this same round, also stalled): "full register-identity
PERMUTATION with zero address drift." Two more attempts, both real-oracle
verified (`build-and-verify.sh`), both **byte-for-byte identical** to the
existing 75/95 diff -- not one instruction moved:

1. Named locals `p1 = &D_8008AB1C; p2 = &D_8008AB24;` assigned in
   REVERSED order (`p2` first, then `p1`) immediately before the first
   `BuildFileName` call, hypothesis being that GCC 2.6.3's local-alloc
   assigns pseudo-hard-registers in an order tied to which value is
   FIRST "created" during RTL expansion (self/arg1, the incoming
   parameters, get the two HIGHEST live registers `$s3`/`$s4` despite
   being logically "first" in the source, suggesting an inverse
   creation-order-to-register-number relationship worth testing on the
   three locally-introduced values too) -- **no change, identical diff**.
2. Same two locals, declared in NORMAL order (`p1` then `p2`) but with
   the handle variable `h`'s OWN declaration moved to LAST (after both
   `p1`/`p2`) instead of first, on the chance that DECLARATION order
   among the three contested locals (not just among `p1`/`p2`) is the
   lever -- **no change, identical diff, byte-for-byte**.

Both confirm this round's `TaskObjF__WriteMemcardSaveFile` finding independently: for this
class, GCC 2.6.3's register assignment for the "extra" simultaneously-live
values is **not driven by any source-level ordering signal tried across
three different functions and two different runners now (declaration
order, first-use order, `self`/`arg1` already fixed as the control)**.
Restored to `INCLUDE_ASM`, unchanged from the prior state. No further
hand-reshaping attempts budgeted for this function without a genuinely new
lever (not a reordering) to test -- the "collapse into one call
expression" lever CLAUDE.md notes moved a SIBLING function
(`StageMap__ComputeFootprintFromRotation`) by one word, not by a register, so even known lever
outside "ordering" have a track record of not helping this class.

---

## Round 19 (bravo) — a fourth axis tried (statement splitting, not just ordering), also inert

This round's brief lists "split one combined expression into two sequential
statements" as a lever that closed `GetSetBitField` elsewhere, distinct from
mere declaration-order shuffling. Tried here: split the nested
`h = New_TimImage(BuildFileName(...));` expression into two statements
(`desc = BuildFileName(...); h = New_TimImage(desc);`) for BOTH
occurrences, rather than only renaming the two address locals or
reordering their declarations (which is all round 18 tried). **Result:
byte-for-byte identical 75/95, same three-register rotation.** No
`--debug` re-run needed -- `funcdiff.py`'s diff list is identical
word-for-word to the existing one.

Also checked whether any of the three rotating values is a
mistyped-narrow parameter (this round's `TaskObjF__WriteMemcardSaveFile` lever): `arg1` is
`void *` (no masking visible anywhere in the body), and the three
`s32 self->unk20/24/28` fields forwarded to `slot8C` are plain full-word
fields (one of them XOR'd with `1` elsewhere in the unit, consistent with
this project's established "`bool` is a full word" convention, not a
byte). No candidate for a width retype here -- this residue has no
masked/narrow value at all, unlike `TaskObjF__WriteMemcardSaveFile`'s. The type axis does
not apply to this function's shape.

So now **ten attempts across four functions and two runners** (the nine the
report already tallied plus this one) confirm the negative for "reshape
how the three long-lived values are introduced" in this specific
three-register-rotation shape. Disposition unchanged: `INCLUDE_ASM`,
stall, permuter candidate as already noted (report's own prior permuter
run found a non-transferring lead; a longer run under this round's
`--stack-diffs` discipline is the honest next step, not more hand
reshaping).

### Proposed learning (reinforces existing entry, does not add a new class)

**Three independent functions across two different header families
(`class_3bb8c_b`'s `StageMap__BuildFootprintRects`, `class_3bb8c_f`'s `TaskObjF__WriteMemcardSaveFile`,
`class_3bb8c_j`'s `ItemList__LoadResources`) now confirm the same negative result
for the SAME lever (declaration/introduction order of the contested
locals).** This is strong enough evidence to stop treating "try a
different declaration order" as a live lever for this residue class at
all -- it has been tried nine times total (7+1+1) across three functions
with zero effect in any instance. Future rounds hitting this class should
either go straight to the permuter (a legitimate use — it varies SOURCE,
not register pinning) or file it as a stall without spending a manual
reordering attempt first.

---

## Round 73 (charlie) — NON_MATCHING body promoted

Track 1b: the preserved body above (75/95, pure register-identity
rotation of the two repeated global addresses and the handle across the
same three registers) is now live in `src/class_3bb8c_j.c` under
`#ifdef NON_MATCHING`, with the verified build still taking the `#else`
`INCLUDE_ASM` branch. `./build-and-verify.sh` stayed green (no bytes
changed) and `tools/check-nonmatching.sh` compiles and link-resolves it.
Hand-derived, not a permuter candidate -- reviewed rounds 18 and 19
already, both fully inert on declaration/introduction-order reshaping.

NON_MATCHING body promoted, round 73

This does not retire the still-outstanding track-1 REVISIT.

---

## Round 73 (delta) -- REVISIT: MATCHED 95/95

**Preserved body rebuilt first, as given in the `#ifdef NON_MATCHING` block
(compiled live):** 75/95, `insertions 3 / deletions 3`, positional skeleton
diffs 20. That is the figure the stall carried since round 18, so the body
was faithful.

**Lever: two handle variables, not one.** Retail's two handles are two
separate C variables with disjoint live ranges (`handle1` for the first
load, `handle2` for the second). The preserved body reused one `h` for
both. One pseudo spanning both halves of the function has a live range
covering the whole body. That lowers its global-alloc priority
(`floor_log2(refs) * refs / live_length`) below the two address pseudos
(`"CARD\\"`, `".TIM"`), so it got `$s2` and the addresses got `$s0`/`$s1`,
which is exactly the three-way rotation on file. Split into two, each
handle has a short, dense live range, outranks the addresses, and takes `$s0`
in turn, as retail does.

The template was the MATCHED sibling `TextEntry__LoadCardResources` in
`src/class_3bb8c_i.c`: the same "CARD\\<name>.TIM" resource loader, with
`char path[0x20]`, `dir`/`ext` locals and `handle1`/`handle2`. Screen for the
next case: a cross-unit sibling with the same call skeleton (here
`BuildFileName` -> `New_TimImage` -> `slot78` -> `New_ScreenSprite`).

Builds, all through `./build-and-verify.sh`:

| # | variant | funcdiff |
| --- | --- | --- |
| 1 | preserved body verbatim | 75/95, ins/del 3/3, skel 20 |
| 2 | sibling shape: `char path[0x20]`, `dir`/`ext` locals, string externs typed `const char []`, `handle1`/`handle2` | **95/95, 0/0, whole image OK** |
| 3 | variant 2 with the handles merged back into one variable | 75/95, 3/3, skel 20 (the old stall exactly) |
| 4 | preserved body (`s32 local[8]`, addresses inline, no `dir`/`ext`) with ONLY the handle split | **95/95, whole image OK** |

So the split is the whole lever. The array-vs-struct question in the brief,
the `dir`/`ext` naming and the string typing are all inert (variant 4).
Rounds 18 and 19 tried only reorderings and statement splits of the
ADDRESS values, never the handle's variable count. Arity was checked and is
not relevant: every call writes exactly the argument registers its callee's
declared parameter list names, and `BuildFileName`'s matched definition
(`code_171e0.c`) takes four.

The names/types are carried into the committed C because they are the
sibling's established reading. The rodata/sdata strings are referenced as
symbols (`extern const char D_...[]`), not retyped as literals.

REVISITED, round 73: MATCHED 95/95 (two disjoint handle variables instead of one reused `h`); names/types used

### Proposed learning

**One C variable reused for two disjoint values is a live-range lever on a
pure callee-saved rotation, and it is the INVERSE of round 59/64's
"delete the named local".** A value reassigned in two unrelated halves of a
function gets one pseudo whose live range spans both, which lowers its
global-alloc priority. Tell: the rotating value is re-produced by a call in
each half and never read across the boundary. Split it (ItemList__LoadResources,
75/95 -> 95/95 on the first build, after ten inert ordering attempts
across rounds 18-19). Discriminator against the dead-parameter-reuse
lever: that one REMOVES a pseudo, and this one ADDS one. Check a matched
cross-unit sibling with the same call skeleton first.

## Naming

- `ItemList__LoadResources` -- tier B. The slot44 occupant (classtable.py gItemListMethods +0x044): builds two "CARD\\<name>.TIM" paths, loads them through New_TimImage/New_D8006ED4C, and dispatches the second through self->methods->slot8C. Mechanics (load a pair of card-icon-shaped resources) are clear from the BuildFileName/CARD path evidence; what the two resources are FOR is not.

## Track 4

2026-09-25, round 84 (charlie): The class `New_D8006ED4C` constructs is unified as ScreenSprite in `include/ScreenSprite.h`; the unit includes it and its local extern is gone. The call reads `self->unk50 = (ItemListHandle_3bb8c_j *)New_ScreenSprite(handle1, (SpriteRect *)&gItemListPanelRect, 0)`: gItemListPanelRect is the rect (words 0, 256, 160), and unk50's +0x04C call passes the screen position gItemListPanelPos = (-100, -60). Image byte-identical.

## Track 4 (2026-09-26, round 88)

TimImage is unified (`include/TimImage.h`); this unit's local `extern
ItemListHandle_3bb8c_j *New_TimImage(char *)` is deleted. `handle1`/
`handle2` are `TimImage *`: +0x078 through `TimImageUploadFn` (occupant
TimImage__Upload), `slot4` -> the inherited `release`; `handle2` is cast to
`ItemListHandle_3bb8c_j *` for slot8C, whose parameter is this class's
own view (not TimImage's to retype). Image byte-identical.
