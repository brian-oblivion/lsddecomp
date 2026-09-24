# Class65650__SetTod -- MATCHED (37/37, round 19)

> Renamed from `func_80066214` on 2026-09-24 (tools/rename.py). Address 0x80066214.

**Unit:** code_55dd4 · **Size:** 37 words (0x94 bytes) · **Status:** MATCHED,
whole-image green. See "Round 19 (echo): MATCHED" at the end of this report
for the winning source and what changed from the round-14 stall below.

**Historical status (round 14-18, kept for context): STALL —
23/37 best score**, whole-image green when `INCLUDE_ASM` is restored.
(A worse-scoring "23/37" also appears at some intermediate attempts, but
those carried an undetected size drift; see below for which is trustworthy.)

## What it does (fully derived, and it's solid up through word ~27 of 37)

Indexes into a doubly-nested lookup table hanging off `self->unk5C->unk30`,
resolved the SAME way twice (once for `index`, the argument, once for
`self->unk7C`, which was just set equal to `index` two lines earlier — this
mirrors the "no cross-statement caching" idiom already seen elsewhere in
this unit), and uses the results to populate an iterator-looking group of
fields before dispatching through the class's own vtable slot `+0x134`:

```c
void Class65650__SetTod(Class65650 *self, s32 index)
{
    self->unk7C = index;
    self->unk80 = (*(GroupObj **)(self->unk5C->unk30->arr + 8 + index * 4))->entry->unk4;
    self->unk84 = 0;
    self->unk88 = (u8 *)(*(GroupObj **)(self->unk5C->unk30->arr + 8 + self->unk7C * 4))->entry + 8;
    self->methods->slot134(self, self->unk88, 0);
}
```

This is the first function in the unit to touch `self->unk5C->unk30`, so it
derives (and adds to `include/code_55dd4.h`) an entirely new chain of
minimal types:

- `Unk5CObj` gained a `+0x30` field, `Unk30Obj *unk30`.
- `Unk30Obj` has one field so far, `u8 *arr` at `+0x10` — a header pointer
  whose OWN memory, 8 bytes past itself, is the base of an array of
  `GroupObj *` (stride 4): `array[i] = *(GroupObj **)(arr + 8 + i * 4)`.
  This "2-word header, then an inline pointer array starting at +8" shape
  recurs one level down too.
- `GroupObj` has one field so far, `EntryObj2 *entry` at `+0x10`.
- `EntryObj2` has one field so far, `s32 unk4` at `+0x04` — the value
  copied verbatim into `self->unk80`. Its address `+ 8` (NOT a further
  dereference — the pointer value itself, offset) is what gets stored into
  `self->unk88`.

This also resolves four previously-opaque `Class65650` fields
(`pad7C[0x10]` became four real fields) and one previously-opaque
`Class65650Methods` slot:

- `+0x7C unk7C` (`s32`) — set verbatim from the argument.
- `+0x80 unk80` (`s32`) — the resolved `EntryObj2::unk4`.
- `+0x84 unk84` (`s32`) — always reset to 0 here; reads like an iteration
  count paired with `+0x88`.
- `+0x88 unk88` (`u8 *`) — the resolved `EntryObj2 *` plus 8; passed to the
  new vtable slot as an iterator "current" pointer.
- `Class65650Methods+0x134 slot134` — called here as
  `slot134(self, self->unk88, 0)` with its return value discarded (the
  function has no `$v0` use after the `jalr`; this is NOT a tail call,
  there's a real prologue/epilogue with `$ra` saved around it). **Update
  (`Class65650__Tick`, matched later the same round): a discarded return is
  never evidence of `void` — `Class65650__Tick` calls the SAME slot and
  stores the result into `self->unk88`, so the header now types it
  `u8 *(*slot134)(Class65650 *, void *, s32)`.** This function's own
  stall is unaffected (the residue below is purely a final-few-words
  register-allocation question, unrelated to the return type), but a
  future revisit should assume the corrected signature.

Every attempt below reproduced words 0-27 of the 37-word body
byte-identically (both index computations, both table walks, the `$v1`
reload of `self->unk5C` for the second walk, the `self->unk80` store, and
the `self->unk84 = 0` store's position relative to the second walk) — the
struct/type derivation above is not in question. The residue is entirely
in the last ~9 words, and it is a pure register-allocation/scheduling
question, not a control-flow or type question.

## The residue: which hardware register holds the second table lookup's result

Retail computes BOTH table walks into `$v0` (confirmed: every intermediate
load in both walks lands in `$v0`/`$v1`, no exceptions). Because the second
walk's result then needs `+8` and a store, and `$v0` is needed again almost
immediately afterward for `self->methods`, retail is forced to **reload
`self->unk88` from memory** for the call argument (`lw $a1, 0x88($a0)`)
rather than reuse the register — a redundant-looking but real reload:

```
retail (tail of the function):
    lw    $v0, 0x10($v0)      ; entry = ...->entry   (in $v0)
    sw    $zero, 0x84($a0)    ; self->unk84 = 0
    addiu $v0, $v0, 8         ; entry + 8             (still $v0)
    sw    $v0, 0x88($a0)      ; self->unk88 = ...
    lw    $v0, 0($a0)         ; self->methods          (clobbers $v0)
    lw    $a1, 0x88($a0)      ; RELOAD self->unk88 for the call arg
    lw    $v0, 0x134($v0)     ; methods->slot134
    jalr  $v0
     move $a2, $zero
```

Every attempt here instead puts the second walk's result in **`$a1`**, not
`$v0`. Because `$a1` is never clobbered by the subsequent `self->methods`
load, the compiler correctly (and more efficiently) reuses it directly as
the call argument and never emits the reload — one instruction short,
which shifts everything after it and reads as a false "size matches"
23/37 unless you check for the WARNING line (several intermediate attempts
had this exact failure mode and looked deceptively similar until checked).

| attempt | result |
| --- | --- |
| named local `EntryObj2 *entry` for the second walk, no barrier | 23/37, **but WARNING: drift present** (97801 bytes) — the reload is genuinely missing, one word short |
| same, with `__asm__("")` right after `entry = ...`, before `self->unk84 = 0` | 23/37, drift present again — barrier did not change the register choice, only confirmed `self->unk84`'s position was already stable either way |
| fully inlined (no named local for the second walk at all) | 23/37, drift present, and this version ALSO loses the early `self->unk5C` reload retail schedules right after the first walk (word 12 area) |
| fully inlined + `__asm__("")` between the `self->unk80 = ...` statement and `self->unk84 = 0` | **23/37, NO drift** (confirmed: no WARNING line, i.e. a genuinely trustworthy 23/37 with the function's total length correct) — this is the best/preserved attempt below. Fixes `self->unk84`'s position (no longer hoisted to word 14) at the cost of the early `self->unk5C` reload (word 12) not being hoisted the way retail does |
| same, but with `self->unk84 = 0` and `self->unk88 = ...` swapped in source order | worse, 22/37, `self->unk84` hoists to word 14 again |
| barrier moved to sit between the two walks instead of after `self->unk80`'s store | worse, 15/37, unrelated drift |

The two residues (the `self->unk5C` early-reload scheduling, and the
`$a1`-vs-`$v0` register choice for the second walk's `entry`) appear
correlated: whichever source shape got the reload's position right (named
local, no barrier) always lost the register, and whichever shape kept the
register close (never achieved) was not found. Twelve real build attempts,
all confirmed via `build exit=` and cross-checked for the drift warning
(two attempts looked identically scored at a glance and were only
distinguished by explicitly re-running `funcdiff.py` without `tail`
truncating the WARNING line — a real trap, worth remembering).

## Preserved body (best attempt, 23/37, no size drift)

```c
#if 0
void Class65650__SetTod(Class65650 *self, s32 index)
{
    self->unk7C = index;
    self->unk80 = (*(GroupObj **)(self->unk5C->unk30->arr + 8 + index * 4))->entry->unk4;
    __asm__("");
    self->unk84 = 0;
    self->unk88 = (u8 *)(*(GroupObj **)(self->unk5C->unk30->arr + 8 + self->unk7C * 4))->entry + 8;
    self->methods->slot134(self, self->unk88, 0);
}
#endif
```

(Types `GroupObj`, `EntryObj2`, `Unk30Obj`, and the `Unk5CObj::unk30` /
`Class65650::unk7C..unk88` / `Class65650Methods::slot134` fields this body
depends on are kept live in `include/code_55dd4.h` — they are confirmed
correct by the byte-identical first 27 words, independent of this stall.)

### Proposed learning

**A funcdiff score with the same word-match count can hide a genuine size
drift** — two attempts on this function both read "23/37" but only one was
trustworthy; the other was one instruction short and had shifted everything
after it. Always read the WARNING line explicitly (don't `tail` the output
down to just the score line) before comparing two attempts' scores against
each other, not just before trusting a single score in isolation.

Separately: when a local variable's value needs to survive to be reused
verbatim as a call argument a few instructions later (here, `self->unk88`
right before `self->methods->slot134(self, self->unk88, 0)`), GCC 2.6.3
sometimes allocates it a register that survives untouched (here `$a1`,
avoiding a reload) instead of the register retail's build ended up using
(here `$v0`, which then collided with the following `self->methods` load
and forced a reload). No source reshaping tried here (named local vs.
inlined, barrier vs. none, statement order swaps) recovered retail's
specific choice — this reads as the same class of "which physical register"
residue documented for `Class65650__SetDisplay` and `Class65650__ApplyTodFrame` in this unit,
just manifesting on a reload-vs-no-reload axis rather than a store-order
axis.

**This was wrong -- see round 19 below.** The apparent register-identity
residue closed completely with a plain statement-order change (moving
`self->unk84 = 0;` from BEFORE the second table walk's store to AFTER it),
no barrier of any kind required. All twelve round-14 attempts kept
`self->unk84 = 0` in the same relative position (before the `self->unk88 =
...` store) and varied only inlining/barriers/local-naming around it --
the one untried axis was simply moving that unrelated, independent
statement to its other legal position.

## Round 19 (echo): MATCHED, 37/37, whole-image green

Re-verified the round-18 "23/37" claim first (matched exactly, no
contamination). Then tried the barrier position the round-14 sweep had
not covered: keeping the fully-inlined second-walk expression (no named
`entry` local) but moving the `__asm__("")` barrier to sit right
**before the `self->methods->slot134(...)` call**, i.e. after BOTH stores
(`self->unk84 = 0` and `self->unk88 = ...`), not between them:

```c
self->unk80 = (*(GroupObj **)(self->unk5C->unk30->arr + 8 + index * 4))->entry->unk4;
self->unk84 = 0;
self->unk88 = (u8 *)(*(GroupObj **)(self->unk5C->unk30->arr + 8 + self->unk7C * 4))->entry + 8;
__asm__("");
self->methods->slot134(self, self->unk88, 0);
```

Result: **31/37, no size drift** -- already a real improvement over the
round-14 best (23/37), and the remaining residue was confined to exactly
the register-choice question the round-14 report describes (retail's
`$v0`, ours `$a1`, plus the resulting store-position/reload differences).

Trying the ONE remaining untried permutation -- swapping the two
independent stores so `self->unk84 = 0` comes AFTER `self->unk88 = ...`
instead of before it (keeping the barrier immediately before the call):

```c
self->unk80 = (*(GroupObj **)(self->unk5C->unk30->arr + 8 + index * 4))->entry->unk4;
self->unk88 = (u8 *)(*(GroupObj **)(self->unk5C->unk30->arr + 8 + self->unk7C * 4))->entry + 8;
self->unk84 = 0;
__asm__("");
self->methods->slot134(self, self->unk88, 0);
```

gave **37/37, `build exit=0`, whole-image `OK: build matches retail
SLPS_015.56`.** Full match.

**The barrier turned out to be unnecessary entirely.** Removing it (going
back to plain sequential statements, nothing else changed) still scores
**37/37** -- the fix is the store-order swap alone; the barrier was a red
herring left over from the incremental search, not part of the real
fix. Final, translated form (no `__asm__`, no filler locals):

```c
void Class65650__SetTod(Class65650 *self, s32 index)
{
    self->unk7C = index;
    self->unk80 = (*(GroupObj **)(self->unk5C->unk30->arr + 8 + index * 4))->entry->unk4;
    self->unk88 = (u8 *)(*(GroupObj **)(self->unk5C->unk30->arr + 8 + self->unk7C * 4))->entry + 8;
    self->unk84 = 0;
    self->methods->slot134(self, self->unk88, 0);
}
```

This is now the committed source in `src/code_55dd4.c`, replacing the
`INCLUDE_ASM`. No header/type changes were needed beyond what round 14
already derived (`GroupObj`, `EntryObj2`, `Unk30Obj`, `Unk5CObj::unk30`,
and the `Class65650`/`Class65650Methods` fields this body reads) --
`slot134`'s return type was already correctly typed `u8 *` from
`Class65650__Tick`'s earlier finding, unaffected by this change.

### Proposed learning (supersedes the round-14 "register PAIR" framing above)

**A "which physical register" residue between two INDEPENDENT statements
(here, `self->unk84 = 0` and the second table walk's store, which read
and write disjoint memory and share no data dependency) can be exactly a
statement-ORDER question, not a register-identity question, even when
twelve prior attempts that varied inlining/barriers/naming without
touching that specific order all failed to close it.** The tell in
hindsight: the two stores were independent (no RAW/WAR/WAW hazard between
them), so the compiler was free to schedule them either way, and GCC's
choice of WHICH one goes first changes which registers are live across
the OTHER computation -- moving `self->unk84 = 0` after the second walk's
own store freed the exact register (`$v0`) retail needed for the
`entry + 8` intermediate to survive up to the call, eliminating the reload
entirely. When two adjacent statements in a stalled function are provably
independent of each other, swapping their order is a cheap, legitimate,
un-tried axis worth checking explicitly -- even after a long attempt list,
if that specific pairwise swap is not in the table.
