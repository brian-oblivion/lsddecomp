# StyleEffect__BuildRandomSprites -- MATCHED 87/87 (head adjudication of a mis-classified stall)

> Renamed from `Class876FC__BuildRandomSprites` on 2026-09-26 (tools/rename.py). Address 0x80056bbc.

> Renamed from `func_80056BBC` on 2026-09-23 (tools/rename.py). Address 0x80056bbc.

Unit `dream_scene`. Round 22. Runner delta reached 86/87 and filed this as a
**stall of the documented "redundant move" class** -- "a genuine dead store in
retail's own compiled output", one instruction the source cannot produce. The
head re-read the residue and it is **not a dead store at all**. The function is
now byte-exact; the fix was one character of type information.

## What the residue actually was

The unmatched instruction is the delay-slot filler of the `self->unk70 < 2`
branch:

```
/* 47418 80056C18 17004014 */  bnez  $v0, .L80056C78
/* 4741C 80056C1C 01000534 */   ori  $a1, $zero, 0x1
```

The stall report argued `$a1` is never read on either path. That is right for
the fallthrough (`$a1` is overwritten by `%hi(sSpriteShiftScratch)` in the very next
instruction) and **wrong for the branch-taken path**, which is the half that
decides. Read `.L80056C78` forward to its first call:

```
.L80056C78:
    lw    $s0, 0x88($s1)
    nop
    lw    $v0, 0x0($s0)
    nop
    lw    $v0, 0x64($v0)
    nop
    jalr  $v0                 <- slot64
     addu $a0, $s0, $zero
```

**Nothing between the label and that `jalr` writes `$a1`.** The `ori` is not
dead: it is the SECOND ARGUMENT of the `slot64` call, hoisted by GCC into the
branch's delay slot in the ordinary way. `$a1` is live across seven
instructions of branch, and a residue that survives an argument register into a
call is an argument, not a leftover.

## The fix

The unit's local `LinkNodeMethods` view typed the slot with one parameter:

```c
void (*slot64)(LinkNode *self);           /* what stalled at 86/87 */
void (*slot64)(LinkNode *self, s32 arg1); /* what matches 87/87 */
```

and the call site becomes `child->methods->slot64(child, 1);`. With no second
parameter in the type there is no `li $a1, 1` anywhere in the C, so there was
nothing for the delay-slot filler to hoist and the slot came out as a `nop`.

**This was cross-checkable without touching the assembler, and that is the
transferable part.** Every other unit in the project that names this slot
already passes it a second argument:

```
src/world/dream_scene.c:72   self->unk18->methods->slot64(self->unk18, v);
src/world/entity.c:244       this->unk94->unk5C->methods->slot64(..., sMoodCue74ClearColor);
src/ui/screen_widgets.c:319   self->methods->slot64(self, 0);
src/ui/screen_widgets.c:419   methods->slot64(self, 1);
src/ui/screen_widgets.c:437   methods->slot64(self, 0);
src/ui/screen_widgets.c:445   methods->slot64(self, 0);
src/world/dream_scene.c:207  unk18->methods->slot64(unk18, unk50->unkC);
```

(`vab_sound.c`'s `s32 (*slot64)(void *)` is a different class's table and
not evidence either way.) Six matched call sites in four units agree on the
arity; this unit's one-argument view was the outlier, and the project's
multiple-independent-local-views convention is exactly what makes checking the
siblings cheap and non-binding.

Delta's other two findings were correct and are kept: the `child->methods`
hoist that fills an earlier load-delay slot, and the `(parity != 0) ?
sSpriteScaleLarge : sSpriteScaleSmall` branch sense. Without those this is 84/87, not 86/87 --
the head's contribution here is the last word, not the body.

## Adjudication: why the class was wrong, and the discriminator that catches it

"Redundant move / dead store" is a real class in this project
(`strcat`, `TextEntry__ResetAllChars`, and the round-16 instance), and it is the class you
reach for when an instruction has no visible consumer. **The check that
separates it from an ordinary hoisted argument is one grep, and it is a
liveness question, not a similarity question:**

> Walk FORWARD from the residue along the branch-TAKEN path to the first
> `jal`/`jalr`. If nothing writes the register in between and the register is
> `$a0`-`$a3`, it is a call argument. It is only a dead store if BOTH successors
> redefine it before any call.

The stall report checked the fallthrough path (correctly) and asserted the
taken path "sets `$a1` itself before its own uses" -- which the listing
contradicts four instructions in. A single-path liveness check is not a
liveness check.

A second-order note for whoever reads a report like this next: the near-miss
score was accurate, the two fixes in it were real and load-bearing, and the
permuter numbers were honestly reported (delta explicitly declined to claim
permuter-exhaustion). **The classification was the only thing wrong, and it was
the only thing that would have kept this function stalled** -- an
`INCLUDE_ASM` with a "genuine dead store in retail's own output" verdict is
one nobody re-opens. This is what PARALLEL-RUNS means by a wrong CAUSE costing
more than a wrong score.

Also corrected from the prior draft: the residue's address was transcribed as
`8005671C`, a digit transposition of `80056C1C`, which is before the function's
own entry point and would not have resolved for the next reader.

## Final body

Lives in `src/world/dream_scene.c` in ROM order between `StyleEffect__ReleaseModelChildren` and
`StyleEffect__SpawnSprites`.

### Proposed learning

**A one-instruction residue in a branch delay slot that sets `$a0`-`$a3` is an
argument to a call in the taken arm until proven otherwise -- check liveness
along the TAKEN path, not just the fallthrough.** A function-pointer slot typed
with too few parameters produces exactly this and nothing else: the call itself
is `jalr`, identical either way, so the arity is invisible at the call site and
shows up only as a missing argument-register setup that the scheduler had
hoisted somewhere non-obvious. Cross-check a slot's arity against every other
unit that calls it (`grep -rn 'slotNN' src/`) before classifying such a residue
as unfixable -- the project's per-unit local views make disagreement cheap to
find and are themselves the evidence.

## Extern arity (round 59)

**Verdict: arity-ok idiom.** `src/world/dream_scene.c`'s unprototyped declaration
stays.

**Callee evidence** (`0x80056BBC`, and the matched definition *in this same
file*, ROM-later at line 379): entry is `move s1,a0` and `$a1` is never read —
one real argument, as `void StyleEffect__BuildRandomSprites(LinkNode *self)` says.

**Why the extern must stay unprototyped.** `StyleEffect__InitByKind`'s dispatch passes a
second argument, and retail emits it:

```
8005660c:  move  a0,s1
80056610:  jal   80056bbc <StyleEffect__BuildRandomSprites>
80056614:  move  a1,zero          <- the dead 2nd argument, in retail
```

This is the same uniform `(self, 0)` switch as the `StyleEffect__SpawnPlainSprites`
arm two cases down (`move a1,zero` at `0x80056624`).

**What makes this one different from the rest of the round.** The declaration
and the definition are in the SAME translation unit — the extern at line 146
exists only because `StyleEffect__InitByKind` (ROM-earlier) calls a function defined
ROM-later in the file, and CLAUDE.md requires strict ROM-address order. So
this is not two units holding different views; it is one unit that must
declare its own function with an argument list its own definition contradicts.
The unspecified parameter list is what lets both coexist: a full prototype at
line 146 would make the `StyleEffect__BuildRandomSprites(self, 0)` call at line 192 a
`too many arguments` error against the definition 187 lines further down.

**Declaration sites changed:** none (arity unchanged). `/* arity-ok: ... */`
added to `src/world/dream_scene.c:146`. Oracle green.

## Naming

Round 70 (alpha). `func_80056BBC` -> `StyleEffect__BuildRandomSprites`, **tier B**.

Only caller StyleEffect__InitByKind, kind 2. Body: parity = rand() % 2;
StyleEffect__SpawnSprites with sSpriteScaleHalf on parity 0, NULL otherwise;
then tableIndex >= 2: sprites[1] gets Actor__AddTranslation by
(sSpriteShiftX[tableIndex], 0, 0) and slotB8 with altColor or color; else
sprites[1] gets setSemiTrans(1), setSemiTransRate(0) and updateScale(set,
parity ? sSpriteScaleLarge : sSpriteScaleSmall); finally sprites[2]
setDisplay(0). "Sprites" rests on the D800879C4 reading (see
StyleEffect__SpawnSprites); B.

Globals named in this pass (only this unit references them, tier B, named by
their ratio-triple values): `sSpriteShiftX` (was D_80087844, s32[6]),
`sSpriteScaleLarge` (was D_8008785C, {6/5, 6/5, 1/1}), `sSpriteScaleHalf`
(was D_80087868, {3/6, 3/6, 1/1}), `sSpriteScaleSmall` (was D_80087874,
{4/6, 4/6, 1/1}), `sSpriteShiftScratch` (was D_80087880, a zero Vec3S whose
.x is overwritten before each use).

## Track 4 (2026-09-26, round 88, charlie)

dream_scene.c's `LinkNode` view (owner and children under one type) is gone: the owner is `StyleEffect` (include/style_effect.h), `modelChildren` are `Actor *`, `sprites` are `VariantSprite *`, and the local `Vec3S` is `LongVec3`. Accessor renames: `kind` is Actor's `pendingExtra` (+0x054, where the ctor stores it); `offset`/`rotation`/`scale`/`modelChildLayout`/`tableIndex`/`color`/`altColor` are `params.*`; slot `slotB8` is Actor's `setTranslation` on the owner and model children and Sprite's `setColor` on sprites; `*coord2 = 0` is `coord2->flg = 0`. Image byte-identical.

## Naming (track 7, round 101)

- Locals: `tblOrNull` -> `scale` (SpawnSprites' scale argument), `child` ->
  `sprite`, `arg` -> `color`, `m` -> `methods`.
- sSpriteScaleLarge/Half/Small are declared `Ratio16[3]`: their data is
  three {num, den} halfword pairs (x and y both 6/5, 3/6 or 4/6, then z
  1/1),
  and they only reach updateScale / SpawnSprites.
