# SortTmdObject — MATCHED 954/954 (round 76)

> Renamed from `func_80018464` on 2026-09-26 (tools/rename.py). Address 0x80018464.

REVISITED, round 76: MATCHED 954/954, whole image OK, 0 permuter iterations (about 45 cc1/whole-image builds); names/types not relevant

The plain C is live in `src/TmdRenderer.c` (no `#ifdef`, no `INCLUDE_ASM`).
`./build-and-verify.sh` says `OK: build matches retail SLPS_015.56`, and
`tools/check-nonmatching.sh` is green.

## Round 76 revisit-2 (2026-09-24, runner echo): six levers, in the order applied

Baseline, the preserved round-56 body rebuilt exactly as it stood: **982
words against 954**, raw 50/954, funcdiff `insertions 49 / deletions 49`. That
in-range figure means nothing: the length is off, so everything after the
prologue is misaligned (319772 bytes differ outside the range).

| # | lever (family) | build after it |
| --- | --- | --- |
| 1 | all 13 inner loops as `loopX: ...; if (--count != 0) goto loopX;` (LOOP KIND) | **954 words, length exact**, 626/954, ins/del 46/46 |
| 2 | `elem = list + K` after `SetupPrimCode`; `elem +=` before `list +=`; `switch` on `u32` (sltiu); lws reloaded, not cached; dpShift as one `(a && b) \|\| c` test; `gte_ldrgb(ctx + 0x34)` before `elem =` | 953-956 words, ins/del 23/23 down to 9/9 |
| 3 | the flags word read through a struct field; the `ctx + 0x8 = 0xA` store after the four global stores, also a field (scheduler alias rule) | the 4-`lw` block exact |
| 4 | the tint as a 3-byte `s8` struct copy (BLOCK MOVE) | `la` + `lb`x3 + `sb`x3 exact |
| 5 | `ctx = ctxIn` assigned AFTER the early `return` (PARAMETER WALKING) | **954 words**, 940/954, ins/del 2/2 |
| 6 | the CLUT update wrapped in `do { } while (0)` (loop depth, see below) | **954/954, 0/0, OK** |

### Lever 1: round 56's "second induction variable" is the do/while itself

Round 56's question was: *what C shape makes GCC 2.6.3 keep `elem` as a bare
biv with five constant-offset uses, when one of those uses is an inline-asm
`"r"` operand?* The answer is that there is no loop for the optimiser to see.
2.6.3's loop pass (strength reduction included) runs only between
`NOTE_INSN_LOOP_BEG`/`END`, which only `for`/`while`/`do` emit. With a goto
loop, `elem` and `list` stay as two plain pointers, each bumped once per
iteration, which is exactly retail's `s1`/`s3`. That is 13 x 2 words plus
`$s7`'s save and restore, 28 words, and the frame drops to `-0x40`, all in
one build. Round 56's twelve reproducer variants were all syntactic loops,
which is why none of them got there.

### Lever 3: the scheduler's struct-versus-scalar alias rule

Retail loads the flags word four times, back to back, then stores `0xA` to
`ctx + 0x8`, then does the four shift/mask pairs, then the four global
stores. Written as `*(u32 *)obj`, each load sat behind the previous global
store. The scheduler DOES reorder here (`-O2` turns on both of 2.6.3's
scheduling passes). It will not move a scalar load at a varying address past a
store to a scalar global, but it will move a `MEM_IN_STRUCT_P` reference past
one. So the fix is `((RenderObjHead *)obj)->flags`. CSE still keeps four loads.
The `ctx + 0x8` store also has to be a struct field, written after the global
stores, so that it waits behind the obj loads (struct against struct may alias)
and nothing else.

### Lever 5: two moves at entry mean an assignment after the return

Retail has `move a1,a3` (in the `bltz` delay slot) and then `move s2,a1`.
Mine had one `move s2,a3`. Declaring the parameter as `u8 *ctx` did nothing.
Declaring `u8 *ctx;` and assigning `ctx = (u8 *)ctxIn;` after the
`if (... < 0) return;` gives the pair and the missing word.

### Lever 6: a goto loop costs a loop-depth level, and local-alloc notices

The last residue was 14 words, and all of them were one pure `$v0`/`$v1` swap
repeated at the six CLUT updates (`*(u16 *)(prim + 0xE) += (A >> B) << 6`).
Retail gives the shift chain `$v0` and the loaded CLUT plus the sum `$v1`.
Every one of about 20 source spellings, checked in under a second each by
comparing cc1 output (`X + sh`, `sh + X`, casts, temporaries, struct-typed
`prim`/`ctx`), gave one of two wrong answers. Either the colours were swapped,
or the colours were right but the sum was tied to the shift.

Local-alloc gives the lowest free register to the highest-priority quantity,
`floor_log2(refs) * refs / span`, and flow weights refs by LOOP DEPTH. Lever 1
took one depth level away from every insn in the per-element loops. At the
lower weight the CLUT quantity (fewer refs, shorter span) outranks the shift
chain. At the higher weight the `floor_log2` step reverses them. A
`do { } while (0)` is a real loop to 2.6.3 (LEARNINGS already says so), so
wrapping just the update in one restores the depth for those insns and nothing
else. It is spelled as a statement macro, `ADD_CLUT_ROWS`, which is also a
plausible reading of the original source.

### Proposed learning

- **A goto loop is the lever for an unwanted strength reduction.** Screen for
  it: the built loop has one extra `addiu sN, biv, K` before the loop, one
  extra per-iteration increment, and one more callee-saved register than
  retail, while retail addresses every use off a single pointer. Every
  syntactic loop form splits the same way (round 56 measured twelve).
  `SortTmdObject` went from 982 to 954 words in one edit.
- **Its side effect is a loop-depth level, and that can flip local-alloc.**
  Once a function's loops are goto loops, a residue that is a pure register
  swap between two short-lived quantities can be the missing depth weighting.
  Wrapping the statement in `do { } while (0)` restores it. Diagnose it with
  cc1 alone on the unit's cpp output, in under a second per variant.
- **Retail's load order can come from the scheduler's alias rule.** Several
  loads grouped above stores to globals, where the C interleaves them, means
  the loads are struct-field references (`MEM_IN_STRUCT_P` passes a
  fixed-address scalar store). Rewrite the access as a field before trying
  temporaries.
- **Two entry moves (`move aX,aN; move sY,aX`) where the build has one mean
  the parameter is copied to its local after an early return, not at the
  declaration.**

### Gate 3

No search was spent, so none of the three checks was run. The function
matched by hand with 0 permuter iterations.

---

## Round 56 revisit (2026-09-19, runner charlie): the length gap is now FULLY
## ACCOUNTED FOR, and it is +28 long rather than -30 short. Three levers, one
## of which overturns a round-50 structural conclusion. Restored to
## `INCLUDE_ASM`; only `include/gte.h` changed in the tree.

REVISITED, round 56: STALL, but the length gap is now fully accounted for -- the body came out +28 words LONG, not -30 short, and all 28 words are one measured compiler transformation rather than an unexplained residue; names/types not relevant

### Why "names/types not relevant", stated before the findings so it is not read as a conclusion drawn to fit them

The revisit rule's hypothesis is that round 50's score was evidence about what
was known then rather than about the function, because `code_8220_b` had not
yet passed track 3. Measured here, that is **not** what happened. Taking the
three levers below one at a time:

- The Psy-Q RGB macro operand form came from **reading the disassembly** (the
  same `addiu` / `swc2 ... 0x0($v0)` pair fifteen times over) and was then
  confirmed against Sony's `include/psyq/inline.h`, which has been in the tree
  the whole time. No track-3 name was involved.
- The switch finding came from **reading the disassembly** — specifically from
  comparing case-body ADDRESSES against case-body TAGS, which needs neither
  names nor types.
- The induction-variable finding came from **building and diffing**, and is a
  property of the loop body shape.

What the naming pass did give was the callee signatures
(`SetupPrimCode(void *prim, void *ctx)`, `ProjectTriFace`, `ProjectQuadFace`,
the six `StoreSxyPoly**` leaves) as live declarations rather than guesses — but
round 50 had already guessed every one of them **correctly**, so they confirmed
work rather than changing it. `PolyDrawCtx` was deliberately not used: this
function touches roughly a dozen context offsets that struct does not model,
and CLAUDE.md's rule that every struct edit is potentially non-local makes
extending a struct an already-matched function reads the expensive option, not
the cheap one.

So this is the same shape round 55's revisit reported: **a post-naming revisit
closed real ground, and the names were not the reason.** Two rounds is a
pattern worth the rule's attention — if the trigger for a revisit is "the unit
has since been named", it is selecting the right functions for the wrong
reason. What actually paid here was that a second reader re-derived the
structure from the assembly with a specific question in hand ("where do the 30
words go?"), which is a property of the REVISIT, not of the NAMING.

### The three figures, all of which moved

| figure | round 50 | round 56 |
| --- | --- | --- |
| length | 924 words, **30 short** | 982 words, **28 long** |
| raw word match | 44/954 | 50/954 |
| first real diff | word 0, 0x8C64, prologue | word 0, 0x8C64, prologue (frame `-0x48` vs retail `-0x40`) |

The in-range word count is still not a meaningful number while the length is
wrong — every address after the prologue drifts — and it is quoted only because
the brief asks for the three figures. The length is the figure that matters and
it is the one that is now explained.

### Retail's prologue, and what the frame is made of

```
    /* 8C64 80018464 C0FFBD27 */  addiu      $sp, $sp, -0x40
    /* 8C68 80018468 2000B0AF */  sw         $s0, 0x20($sp)
    /* 8C6C 8001846C 21808000 */  addu       $s0, $a0, $zero
    /* 8C70 80018470 2118A000 */  addu       $v1, $a1, $zero
    /* 8C74 80018474 2140C000 */  addu       $t0, $a2, $zero
    /* 8C78 80018478 3C00BFAF */  sw         $ra, 0x3C($sp)
    /* 8C7C 8001847C 3800B6AF */  sw         $s6, 0x38($sp)
    /* 8C80 80018480 3400B5AF */  sw         $s5, 0x34($sp)
    /* 8C84 80018484 3000B4AF */  sw         $s4, 0x30($sp)
    /* 8C88 80018488 2C00B3AF */  sw         $s3, 0x2C($sp)
    /* 8C8C 8001848C 2800B2AF */  sw         $s2, 0x28($sp)
    /* 8C90 80018490 2400B1AF */  sw         $s1, 0x24($sp)
```

`0x40` = `0x20` of outgoing-argument area + `0x20` of saved registers.
The argument area is `0x20` and not the o32 minimum `0x10` because
`ProjectQuadFace` takes **seven** arguments (prim, ctx, four indices, the
callback), which is 28 bytes rounded to 32. The saved set is `$s0`-`$s6`
plus `$ra` — **seven** callee-saved registers, and they are:

| reg | holds |
| --- | --- |
| `$s0` | `prim`, the packet-buffer write cursor (reloaded from `GsOUT_PACKET_P` per group, advanced by each submit wrapper's return) |
| `$s1` | `elem`, the per-element cursor |
| `$s2` | `ctx`, the draw context (the caller's scratchpad block) |
| `$s3` | `list`, the group cursor |
| `$s4` | `count`, the per-group element counter |
| `$s5` | `remaining`, the running total across all groups |
| `$s6` | `StoreSxyPolyG3` held in a register — it is the only callback passed from more than one case (B and J), and retail materialises it once before the outer loop (`lui`/`addiu` at 0x80018758) rather than at each of the two call sites |

Any C that needs an eighth callee-saved register produces `-0x48` and a
one-word-longer prologue and epilogue. Round 50's `-0x48` and this round's
`-0x48` have the SAME cause, identified below, and it is not the one round 50
guessed.

### Lever 1 — the Psy-Q RGB macros are one-pointer-at-offset-0 forms, and open-coding the offset loses a word per call site

Round 50 wrote every colour move as an open-coded block with the offset folded
into the instruction, e.g.

```c
__asm__ volatile ("swc2 $22, 0x4(%0)" : : "r" (self) : "memory");   /* 1 word */
```

Retail never does that. Every one of these is a pair:

```
    /* 90C8 800188C8 000040C8 */  lwc2       $0, 0x0($v0)
    ...
    /* 90D8 800188D8 1304E84A */  ncds
    /* 90DC 800188DC 04000226 */  addiu      $v0, $s0, 0x4
    /* 90E0 800188E0 000056E8 */  swc2       $22, 0x0($v0)
```

The tally over the whole function is unambiguous — `swc2 $22, 0x0($v0)`
occurs **15 times** and `swc2 $22, <nonzero>($s0)` occurs **zero** times
except in the two G3-layout triples. The `addiu` is not scheduling noise: it
is GCC materialising the sum because Sony's macro takes a POINTER and accesses
it at offset `0x0`.

Sony's own `include/psyq/inline.h` settles the operand shapes. Its macro-call
words say nothing about encodings (the file banner in `include/gte.h` explains
why), but the **operand count** and **op count** are readable there and they
are what decides the bytes:

| Sony macro | operands | ops | retail's form |
| --- | --- | --- | --- |
| `gte_ldrgb(p)` | 1 | 1 | `lwc2 $6, 0x0(p)` |
| `gte_ldrgb3(p0,p1,p2)` | 3 | 4 | three pointers at `0x0`, 4th reloads p2 into RGB |
| `gte_ldrgb3c(p)` | 1 | 4 | `0x0/0x4/0x8`, 4th is `lwc2 $6, 0x8(p)` |
| `gte_strgb(p)` | 1 | 1 | `swc2 $22, 0x0(p)` |
| `gte_strgb3(p0,p1,p2)` | 3 | 3 | three pointers at `0x0` |
| `gte_strgb3_g3(p)` | 1 | 3 | `0x4/0xC/0x14` from one pointer |

Every row was checked against retail instruction-for-instruction. The `c`
suffix on `gte_ldrgb3c` is Sony's "contiguous" form and it is exactly cases J
and L; the generic three-pointer `gte_ldrgb3`/`gte_strgb3` is exactly cases K
and M. Worked contrast, both from this function:

```
  case J (POLY_G3 layout, gte_ldrgb3c + gte_strgb3_g3 -- no addiu at all)
    lwc2 $20, 0x0($s1) / $21, 0x4($s1) / $22, 0x8($s1) / $6, 0x8($s1)
    dpct
    swc2 $20, 0x4($s0) / $21, 0xC($s0) / $22, 0x14($s0)

  case K (POLY_GT3 layout, gte_ldrgb3 + gte_strgb3 -- five addiu)
    addiu $v1, $s3, 0x10 ; addiu $v0, $s3, 0x14
    lwc2 $20, 0x0($v1) / $21, 0x0($v0) / $22, 0x0($s1) / $6, 0x0($s1)
    dpct
    addiu $a0, $s0, 0x4 ; addiu $v1, $s0, 0x10 ; addiu $v0, $s0, 0x1C
    swc2 $20, 0x0($a0) / $21, 0x0($v1) / $22, 0x0($v0)
```

The two are not interchangeable and the difference is worth 5 words in one
case body. POLY_G3/G4 space their three RGBs at `+0x4/+0xC/+0x14`, close
enough to index from one pointer; POLY_GT3/GT4 space theirs at
`+0x4/+0x10/+0x1C` and retail reaches those with the generic three-pointer
form rather than Sony's own `gte_strgb3_gt3`.

All six macros are now in `include/gte.h` with the retail-confirmed offsets,
which is where that header's own banner says they belong.

**Worth +19 words on the store side and +2 on the load side** (cases D and H
load the group's constant colour with `addiu $v0, $s2, 0x34; lwc2 $6, 0x0($v0)`
before the loop, where round 50 wrote `lwc2 $6, 0x34(ctx)`).

### Lever 1b — the same preamble ops are Sony macros too, and the op counts prove which

The 16 `cfc2`/`sw`, 10 `lw`/`ctc2` and 6+6 halfword/`mtc2`/`mfc2` blocks in the
preamble are not loose instructions to transcribe one by one. Counting the
volatile lines in `INLINE.H` gives the op counts directly, and retail's
preamble has blocks of exactly those sizes in exactly that order:

| Sony macro | ops | retail block |
| --- | --- | --- |
| `gte_ReadRotMatrix(p)` | 16 | `cfc2 $0..$7` + 8 `sw` at `p+0x0..0x1C` |
| `gte_SetRotMatrix(p)` | 10 | 5 `lw` + `ctc2 $0..$4` |
| `gte_ldclmv(p)` | 6 | 3 `lhu` at stride 6 + `mtc2 $9/$10/$11` |
| `gte_stclmv(p)` | 6 | 3 `mfc2` + 3 `sh` at stride 6 |

So the whole preamble is six macro calls, and the register discipline falls
out for free: all four use `$12`/`$13`/`$14` as scratch, which is exactly the
`$t4`/`$t5`/`$t6` retail shows, and is the same situation `gte_stflg()` was
already in. Note the asymmetry, which is retail's: `gte_ReadRotMatrix` saves
control `$0..$7` (a full Psy-Q `MATRIX`, 0x20 bytes) and `gte_SetRotMatrix`
restores only `$0..$4`. The translation vector is read out and never put back.

The memory clobbers then reproduce retail's reload pattern for free.
`gte_SetRotMatrix` reads memory and does not clobber it, so the
`*(u8 **)(obj + 0x4)` load feeding it stays live and retail keeps it in `$v1`
for the first `gte_ldclmv`; `gte_stclmv` DOES clobber memory, so the second and
third columns reload it — which is exactly retail's `lw $v0, 0x4($s0)` before
each of the second and third blocks. That is a correctness check on the clobber
lists, not a coincidence.

This block is not part of the 28-word gap (round 50's open-coded version was
already the right LENGTH) but it is the difference between a readable body and
thirty lines of transcribed assembly, and it is what HARD RULE 6 means by
"look it up in `include/gte.h` first".

### Lever 2 — it is a `switch`, and round 50's "definitely not a switch" is wrong

Round 50 read the `sltiu`/`beq` ladder as a hand-written nested range cascade
and wrote, in this file: *"writing it as a real C `switch` risks GCC 2.6.3
choosing a completely different (possibly jump-table) layout."* That is
backwards, and the evidence is in the report's own case table — it just was
never crossed against the addresses.

**The tell is body layout.** Retail's thirteen case bodies sit at ascending
addresses in ascending TAG order:

```
0x2000 .L80018868   0x2004 .L80018914   0x2101 .L800189F4   0x2400 .L80018A88
0x2501 .L80018B78   0x2800 .L80018C50   0x2901 .L80018D00   0x2C00 .L80018D9C
0x2D01 .L80018EA4   0x3101 .L80018F8C   0x3501 .L80019028   0x3901 .L80019120
0x3D01 .L800191EC
```

That is source order for a `switch` written in ascending case order. It is NOT
what an `if`/`else if` chain produces: the chain's first test is `== 0x2901`,
so an if-chain would lay case G's body FIRST. Retail lays it seventh, exactly
where its tag sorts.

The comparison ladder is then GCC 2.6.3's balanced binary search over sparse
case values, and it is a binary search over the SORTED list, not over source
order. Thirteen values; the median (7th) is `0x2901`, so the first test is
`== 0x2901` then `< 0x2902`; the lower half `{0x2000, 0x2004, 0x2101, 0x2400,
0x2501, 0x2800}` splits on `0x2101`, the upper half `{0x2C00, 0x2D01, 0x3101,
0x3501, 0x3901, 0x3D01}` on `0x3101`, and each quarter recurses the same way.
Every one of retail's `sltiu <value>+1` / `beq <value>` pairs falls out of that,
including the `default:` arms that `j` straight to the epilogue.

There is no jump table because the values are sparse (`0x2000`..`0x3D01` over
13 cases); GCC only emits one when the range is dense enough to be worth it.

Confirmed by building it: the switch form reproduces the ladder and the
ascending body layout. One residue left on this lever — retail's range tests
are `sltiu` (unsigned) and a `switch` on an `s32` gives `slti`, so the switch
expression wants to be read unsigned. That is a one-line change that was
measured as not affecting length and was not chased further this round.

**Generalisable screen, and it is cheap:** `sltiu` against *value + 1* paired
with `beq` against *value*, plus case bodies at ascending addresses in
ascending case-value order, is the signature of a C `switch`. An `if`/`else if`
chain transcribing the same comparisons gets the comparisons right and the
LAYOUT wrong, and the layout is most of the bytes.

### Lever 3 — the whole remaining +28 is ONE compiler transformation, and it is not what round 50 guessed

With levers 1 and 2 in, the body built clean and came out **982 words against
retail's 954**. Diffing the two instruction streams, the +28 is not scattered.
It is one mechanism repeated once per case:

**GCC splits `elem` into TWO induction variables in every one of the thirteen
per-element loops.** Case A, built:

```
    addiu s3,s4,4        <- s3 = list + 4  = elem      (for the offset-0 asm)
    addiu s1,s4,8        <- s1 = list + 8  = elem + 4  (biased base for the lhu's)
  loop:
    lhu   a2,2(s1)       ...   elem + 6
    lhu   a3,4(s1)             elem + 8
    lhu   v1,6(s1)             elem + 0xA
    ...
    lhu   v0,0(s1)             elem + 4
    lwc2  $6,0(s3)             elem + 0
    ...
    addiu s1,s1,16       <- extra increment
    addiu s5,s5,-1
    bnez  s5,loop
    addiu s3,s3,16
```

Retail uses a **single** base for all five offsets:

```
    addiu $s1, $s3, 0x4
  .L80018888:
    lhu   $a2, 0x6($s1)
    lhu   $a3, 0x8($s1)
    lhu   $v1, 0xA($s1)
    ...
    lhu   $v0, 0x4($s1)
    lwc2  $6,  0x0($s1)
    ...
    addiu $s4, $s4, -0x1
    bnez  $s4, .L80018888
     addiu $s1, $s1, 0x10
```

The arithmetic closes exactly:

```
  13 cases x 1 extra setup addiu      = 13
  13 cases x 1 extra tail increment   = 13
  the extra IV is the 8th callee-saved register:
    sw $s7 in the prologue + lw $s7 in the epilogue
                                      =  2
                                        ---
                                         28   <- the entire surplus
```

**This corrects round 50's diagnosis of its own frame gap.** That round
attributed `-0x48` to "the short-lived per-case helper pointers (`nBase`,
`tri0`/`tri1`/`tri2`) getting promoted to a saved register", and proposed
recomputing them inline as the fix. This round's build recomputes exactly
those inline — they are `gte_ldv0(...)` and `gte_ldrgb3(...)` arguments now,
named nowhere — and the frame is still `-0x48`. The eighth register is the
strength-reduced induction variable, which is a property of the LOOP BODY
SHAPE and not of any named local, so no amount of not-naming a temporary
reaches it.

#### Negative: no source form tried defeats the split

Isolated to a self-contained reproducer through the pinned pipeline (flags read
from the Makefile, never retyped) so the finding is about the toolchain and not
about this function:

```c
typedef unsigned short u16; typedef unsigned char u8; typedef int s32;
extern s32 P(void *a, u8 *b, u16 c, u16 d, u16 e, void *f);
extern void *Q(void *a, void *b);
void f(u8 *prim, u8 *ctx, u8 *list, s32 count)
{
    u8 *elem = list + 4;
    do {
        if (P(prim, ctx, *(u16 *)(elem + 6), *(u16 *)(elem + 8),
              *(u16 *)(elem + 0xA), 0) == 0) {
            __asm__ volatile ("lwc2 $6, 0x0(%0)" : : "r" (elem));
            prim = (u8 *)Q(prim, ctx);
        }
        list += 0x10;
        elem += 0x10;
    } while (--count != 0);
}
```

reproduces it in nine lines: `addiu s3,a2,4` and `addiu s0,a2,14`, two
increments, offsets `-4/-2/0` off the second. **Ten variants, all of which
still split** (iteration count: 10 reproducer variants plus 5 whole-image
builds):

| variant | result |
| --- | --- |
| `"memory"` clobber on the asm | splits |
| Sony's `$12/$13/$14` + `"memory"` clobbers | splits |
| `"m"` memory operand instead of `"r"` | splits |
| a C read at `elem + 0` before the asm | splits |
| a C read at `elem + 0` after the asm | splits |
| a `volatile` read inside the loop | splits |
| `elem` declared inside the loop (giv, not biv) | splits, and adds a third register |
| index reads taken off `list` instead of `elem` | splits |
| `while (count-- != 0)` | splits |
| `for (i = 0; i < count; i++)` | splits |
| typed `struct *elem` with `elem++` | splits |
| no asm at all | does NOT split — the biv is eliminated entirely |

The last row is the mechanism: the inline asm's `"r"` operand is a plain
REGISTER use of `elem`, so GCC cannot eliminate the biv; the four address uses
at `elem + 4..0xA` are then reduced into their own register. Remove the asm and
the biv goes away and there is one IV. Which base GCC picks for the giv is not
stable either — the reproducer picks the highest address use, the real function
picks the lowest — so it is not steerable by ordering the uses.

Retail has the asm AND a single IV, from the same compiler and the same flags.
Something about retail's source suppresses the reduction and none of the twelve
forms above is it. **That is the one open question**, and it is now a narrow,
well-posed one: *what C shape makes GCC 2.6.3 keep `elem` as a bare biv with
five constant-offset uses, when one of those uses is an inline-asm `"r"`
operand?*

Explicitly NOT a toolchain escalation. The reproducer proves the behaviour is
deterministic and contextual, not a bug, and CLAUDE.md's rule is that a
reproducer which behaves consistently is a result about the source shape, not a
flag to turn.

#### Why no permuter search was spent

Gate 3's three checks were not run and no search was started, deliberately. The
permuter mutates C source to shake out register and scheduling residue on a
body whose LENGTH is already right; here the length is wrong by 28 words from a
single identified transformation, so every candidate would be scored against a
fully drifted image and the base would carry no signal. The next attempt should
spend its search only after the IV question above is answered and the body
builds at 954 words. Recording this as a decision rather than an omission.

### Derived C, preserved (NOT matched, 982 words, restore point)

Compiles clean against the current tree and links; it needs the six new RGB
macros and the four preamble macros added to `include/gte.h` this round, which
ARE committed. Put it back in place of the `INCLUDE_ASM` at the same ROM-order
position.

```c
#if 0
/* Globals this renderer publishes for SetupPrimCode and the code_8220_c
 * submit wrappers to read back. D_8008E248 is already in code_8220.h; the
 * other four are this unit's own view and stay local per CLAUDE.md's
 * cross-unit-declaration rule. */
extern s32 D_80090C18;
extern s32 gSortUseGlobalLightMode;
extern s32 gSortLightMode;
extern s32 GsLIGHT_MODE;
extern s8 gTexturedFaceColor[3];
extern void *GsOUT_PACKET_P;

extern void InitDivPolygonPtrs(void *dst, void *table, s32 count);
extern void StoreSxyPolyFT4(void *dst, s32 storeFirst3);
extern void StoreSxyPolyGT4(void *dst, s32 storeFirst3);

/*
 * The eight submit wrappers in code_8220_c, which are still INCLUDE_ASM there
 * and declared `void`. That is provably a placeholder: every one of them is a
 * tail call whose last instruction before its epilogue is `jal RCpolyXX` with
 * no intervening store to $v0, so Sony's return value falls straight out --
 * the standard `p = RCpolyF3(p);` work-buffer-advance idiom. This function
 * consumes exactly that value, so this file declares its own view rather than
 * importing the stale one (round 50's finding; see the report).
 */
extern void *SubmitPolyF3(void *prim, void *ctx);
extern void *SubmitPolyG3(void *prim, void *ctx);
extern void *SubmitPolyFT3(void *prim, void *ctx);
extern void *SubmitPolyF4(void *prim, void *ctx);
extern void *SubmitPolyG4(void *prim, void *ctx);
extern void *SubmitPolyFT4(void *prim, void *ctx);
extern void *SubmitPolyGT3(void *prim, void *ctx);
extern void *SubmitPolyGT4(void *prim, void *ctx);

/* Defined below, in ROM order. Forward-declared because this function comes
 * first in the segment and calls all of them. */
void SetupPrimCode(void *prim, void *ctx);
s32 ProjectTriFace(void *prim, u8 *ctx, u16 idx0, u16 idx1, u16 idx2, void (*storeSxy)(void *));
s32 ProjectQuadFace(void *prim, u8 *ctx, u16 idx0, u16 idx1, u16 idx2, u16 idx3, void (*storeSxy)(void *, s32));
void StoreSxyPolyF3(void *dst);
void StoreSxyPolyG3(void *dst);
void StoreSxyPolyFT3(void *dst);
void StoreSxyPolyGT3(void *dst);
void StoreSxyPolyF4(void *dst, s32 storeFirst3);
void StoreSxyPolyG4(void *dst, s32 storeFirst3);

/*
 * Walk one model's face groups and emit a GPU primitive per surviving face.
 *
 * `obj` is the drawable, `arg1` supplies the ordering table, `arg2` its shift,
 * and `ctx` is the per-object scratch block the caller places in the PS1
 * scratchpad. Each group header is a 2-byte element count plus a 2-byte
 * primitive tag (and one bit of the same word, the semi-transparency flag);
 * the tag selects one of thirteen case bodies, each with its own element
 * stride, index offsets, GTE colour op and submit wrapper. `prim` is the
 * packet-buffer write cursor, reloaded from GsOUT_PACKET_P at the top of every
 * group and advanced by each submit wrapper's return value.
 *
 * Every GTE access goes through include/gte.h. The RGB store macros are the
 * one-pointer-at-offset-0 Psy-Q forms, which is why `gte_strgb(prim + 0x4)`
 * and not `swc2 $22, 0x4(prim)`: the addiu that materialises the sum is part
 * of retail.
 */
void SortTmdObject(void *objIn, void *otSrc, s32 otShift, void *ctxIn)
{
    u8 *obj = (u8 *)objIn;
    u8 *ctx = (u8 *)ctxIn;
    u8 *prim;
    u8 *list;
    s32 remaining;
    s32 dpShift;

    if (*(s32 *)obj < 0) {
        return;
    }

    *(void **)(ctx + 0x0) = *(void **)((u8 *)otSrc + 0x4);
    *(s32 *)(ctx + 0x4) = otShift;
    InitDivPolygonPtrs(ctx + 0x88, gDivPolygon3, 3);
    InitDivPolygonPtrs(ctx + 0x94, gDivPolygon4, 4);

    remaining = *(s32 *)(*(u8 **)(obj + 0x8) + 0x14);
    list = *(u8 **)(*(u8 **)(obj + 0x8) + 0x10);
    *(void **)(ctx + 0xC) = *(void **)(*(u8 **)(obj + 0x8) + 0x0);
    *(void **)(ctx + 0x10) = *(void **)(*(u8 **)(obj + 0x8) + 0x8);

    /* When the object carries a local light/world matrix, save the GTE's
     * current rotation matrix into the context, install the object's own,
     * run each of the three columns of the object's 3x3 through it, and put
     * the saved matrix back. */
    if (*(s32 *)(*(u8 **)(obj + 0x4) + 0x48) != 0) {
        u8 *lws = *(u8 **)(obj + 0x4);

        gte_ReadRotMatrix(ctx + 0x38);
        gte_SetRotMatrix(*(u8 **)(lws + 0x48) + 0x24);

        gte_ldclmv(lws + 0x24);
        gte_llir();
        gte_stclmv(lws + 0x24);

        gte_ldclmv(*(u8 **)(obj + 0x4) + 0x26);
        gte_llir();
        gte_stclmv(*(u8 **)(obj + 0x4) + 0x26);

        gte_ldclmv(*(u8 **)(obj + 0x4) + 0x28);
        gte_llir();
        gte_stclmv(*(u8 **)(obj + 0x4) + 0x28);

        gte_SetRotMatrix(ctx + 0x38);
    }

    /* Four independent reads of the flags word, not one cached copy: each
     * global store below kills the previous load for CSE, and retail shows
     * all four `lw`s. */
    *(s32 *)(ctx + 0x8) = 0xA;
    D_80090C18 = (*(u32 *)obj >> 9) & 0x7;
    D_8008E248 = (*(u32 *)obj >> 6) & 0x1;
    gSortUseGlobalLightMode = (*(u32 *)obj >> 5) & 0x1;
    gSortLightMode = (*(u32 *)obj >> 3) & 0x3;

    {
        s8 *tint = gTexturedFaceColor;

        *(s8 *)(ctx + 0x34) = tint[0];
        *(s8 *)(ctx + 0x35) = tint[1];
        *(s8 *)(ctx + 0x36) = tint[2];
    }

    if (gSortUseGlobalLightMode != 0 && GsLIGHT_MODE != 0) {
        dpShift = 9;
    } else if (gSortLightMode != 0) {
        dpShift = 9;
    } else {
        dpShift = 0x10;
    }
    *(s32 *)(ctx + 0x2C) = dpShift;

    if (remaining == 0) {
        return;
    }

    {
        void (*cbG3)(void *) = StoreSxyPolyG3;

        do {
            s32 count;

            prim = (u8 *)GsOUT_PACKET_P;
            *(s32 *)(ctx + 0x18) = *(u16 *)(list + 0x2) & 0xFD07;
            count = *(u16 *)(list + 0x0);
            *(s32 *)(ctx + 0x1C) = (*(u32 *)(list + 0x0) >> 25) & 0x1;
            remaining -= count;


            /* Thirteen Psy-Q primitive flavours, one case each, in ascending
             * tag order. GCC 2.6.3 expands a switch over sparse values as a
             * balanced binary search -- `== median`, then `< median + 1` and
             * recurse -- which is where retail's sltiu/beq ladder comes from;
             * the case bodies then follow in source order, which is why they
             * sit at ascending addresses in ascending tag order. */
            switch (*(s32 *)(ctx + 0x18)) {
            case 0x2000: {
                u8 *elem = list + 0x4;

                /* A: POLY_F3, opaque */
                prim[3] = 4;
                prim[7] = 0x20;
                SetupPrimCode(prim, ctx);
                do {
                    if (ProjectTriFace(prim, ctx, *(u16 *)(elem + 0x6), *(u16 *)(elem + 0x8),
                                       *(u16 *)(elem + 0xA), StoreSxyPolyF3) == 0) {
                        gte_ldv0(*(u8 **)(ctx + 0x10) + (s32)*(u16 *)(elem + 0x4) * 8);
                        gte_ldrgb(elem);
                        gte_ncds();
                        gte_strgb(prim + 0x4);
                        prim[7] = ctx[0x15];
                        prim = (u8 *)SubmitPolyF3(prim, ctx);
                    }
                    list += 0x10;
                    elem += 0x10;
                } while (--count != 0);
                break;
            }

            case 0x2004: {
                u8 *elem = list + 0xC;

                /* B: POLY_G3, opaque -- one ncds per vertex colour */
                prim[3] = 6;
                prim[7] = 0x30;
                SetupPrimCode(prim, ctx);
                do {
                    if (ProjectTriFace(prim, ctx, *(u16 *)(elem + 0x6), *(u16 *)(elem + 0x8),
                                       *(u16 *)(elem + 0xA), cbG3) == 0) {
                        gte_ldv0(*(u8 **)(ctx + 0x10) + (s32)*(u16 *)(elem + 0x4) * 8);
                        gte_ldrgb(list + 0x4);
                        gte_ncds();
                        gte_strgb(prim + 0x4);
                        prim[7] = ctx[0x15];
                        gte_ldrgb(list + 0x8);
                        gte_ncds();
                        gte_strgb(prim + 0xC);
                        gte_ldrgb(elem);
                        gte_ncds();
                        gte_strgb(prim + 0x14);
                        prim = (u8 *)SubmitPolyG3(prim, ctx);
                    }
                    list += 0x18;
                    elem += 0x18;
                } while (--count != 0);
                break;
            }

            case 0x2101: {
                u8 *elem = list + 0x4;

                /* C: POLY_F3, depth-cued */
                prim[3] = 4;
                prim[7] = 0x20;
                SetupPrimCode(prim, ctx);
                do {
                    if (ProjectTriFace(prim, ctx, *(u16 *)(elem + 0x4), *(u16 *)(elem + 0x6),
                                       *(u16 *)(elem + 0x8), StoreSxyPolyF3) == 0) {
                        gte_ldrgb(elem);
                        gte_dpcs();
                        gte_strgb(prim + 0x4);
                        prim[7] = ctx[0x15];
                        prim = (u8 *)SubmitPolyF3(prim, ctx);
                    }
                    list += 0x10;
                    elem += 0x10;
                } while (--count != 0);
                break;
            }

            case 0x2400: {
                u8 *elem = list + 0x10;

                /* D: POLY_FT3, opaque -- one constant colour for the whole
                 * group, loaded once before the loop. */
                prim[3] = 7;
                prim[7] = 0x24;
                SetupPrimCode(prim, ctx);
                gte_ldrgb(ctx + 0x34);
                do {
                    if (ProjectTriFace(prim, ctx, *(u16 *)(elem + 0x2), *(u16 *)(elem + 0x4),
                                       *(u16 *)(elem + 0x6), StoreSxyPolyFT3) == 0) {
                        *(u32 *)(prim + 0xC) = *(u32 *)(elem - 0xC);
                        *(u32 *)(prim + 0x14) = *(u32 *)(elem - 0x8);
                        *(u32 *)(prim + 0x1C) = *(u32 *)(elem - 0x4);
                        *(u16 *)(prim + 0xE) = *(u16 *)(prim + 0xE) +
                            (u16)((*(s32 *)(ctx + 0x24) >> *(s32 *)(ctx + 0x2C)) << 6);
                        gte_ldv0(*(u8 **)(ctx + 0x10) + (s32)*(u16 *)(elem + 0x0) * 8);
                        gte_ncds();
                        gte_strgb(prim + 0x4);
                        prim[7] = ctx[0x15];
                        prim = (u8 *)SubmitPolyFT3(prim, ctx);
                    }
                    list += 0x18;
                    elem += 0x18;
                } while (--count != 0);
                break;
            }

            case 0x2501: {
                u8 *elem = list + 0x10;

                /* E: POLY_FT3, depth-cued */
                prim[3] = 7;
                prim[7] = 0x24;
                SetupPrimCode(prim, ctx);
                do {
                    if (ProjectTriFace(prim, ctx, *(u16 *)(elem + 0x4), *(u16 *)(elem + 0x6),
                                       *(u16 *)(elem + 0x8), StoreSxyPolyFT3) == 0) {
                        *(u32 *)(prim + 0xC) = *(u32 *)(elem - 0xC);
                        *(u32 *)(prim + 0x14) = *(u32 *)(elem - 0x8);
                        *(u32 *)(prim + 0x1C) = *(u32 *)(elem - 0x4);
                        *(u16 *)(prim + 0xE) = *(u16 *)(prim + 0xE) +
                            (u16)((*(s32 *)(ctx + 0x24) >> *(s32 *)(ctx + 0x2C)) << 6);
                        gte_ldrgb(elem);
                        gte_dpcs();
                        gte_strgb(prim + 0x4);
                        prim[7] = ctx[0x15];
                        prim = (u8 *)SubmitPolyFT3(prim, ctx);
                    }
                    list += 0x1C;
                    elem += 0x1C;
                } while (--count != 0);
                break;
            }

            case 0x2800: {
                u8 *elem = list + 0x4;

                /* F: POLY_F4, opaque */
                prim[3] = 5;
                prim[7] = 0x28;
                SetupPrimCode(prim, ctx);
                do {
                    if (ProjectQuadFace(prim, ctx, *(u16 *)(elem + 0x6), *(u16 *)(elem + 0x8),
                                        *(u16 *)(elem + 0xA), *(u16 *)(elem + 0xC),
                                        StoreSxyPolyF4) == 0) {
                        gte_ldv0(*(u8 **)(ctx + 0x10) + (s32)*(u16 *)(elem + 0x4) * 8);
                        gte_ldrgb(elem);
                        gte_ncds();
                        gte_strgb(prim + 0x4);
                        prim[7] = ctx[0x15];
                        prim = (u8 *)SubmitPolyF4(prim, ctx);
                    }
                    list += 0x14;
                    elem += 0x14;
                } while (--count != 0);
                break;
            }

            case 0x2901: {
                u8 *elem = list + 0x4;

                /* G: POLY_F4, depth-cued */
                prim[3] = 5;
                prim[7] = 0x28;
                SetupPrimCode(prim, ctx);
                do {
                    if (ProjectQuadFace(prim, ctx, *(u16 *)(elem + 0x4), *(u16 *)(elem + 0x6),
                                        *(u16 *)(elem + 0x8), *(u16 *)(elem + 0xA),
                                        StoreSxyPolyF4) == 0) {
                        gte_ldrgb(elem);
                        gte_dpcs();
                        gte_strgb(prim + 0x4);
                        prim[7] = ctx[0x15];
                        prim = (u8 *)SubmitPolyF4(prim, ctx);
                    }
                    list += 0x10;
                    elem += 0x10;
                } while (--count != 0);
                break;
            }

            case 0x2C00: {
                u8 *elem = list + 0x14;

                /* H: POLY_FT4, opaque */
                prim[3] = 9;
                prim[7] = 0x2C;
                SetupPrimCode(prim, ctx);
                gte_ldrgb(ctx + 0x34);
                do {
                    if (ProjectQuadFace(prim, ctx, *(u16 *)(elem + 0x2), *(u16 *)(elem + 0x4),
                                        *(u16 *)(elem + 0x6), *(u16 *)(elem + 0x8),
                                        StoreSxyPolyFT4) == 0) {
                        *(u32 *)(prim + 0xC) = *(u32 *)(elem - 0x10);
                        *(u32 *)(prim + 0x14) = *(u32 *)(elem - 0xC);
                        *(u32 *)(prim + 0x1C) = *(u32 *)(elem - 0x8);
                        *(u32 *)(prim + 0x24) = *(u32 *)(elem - 0x4);
                        *(u16 *)(prim + 0xE) = *(u16 *)(prim + 0xE) +
                            (u16)((*(s32 *)(ctx + 0x24) >> *(s32 *)(ctx + 0x2C)) << 6);
                        gte_ldv0(*(u8 **)(ctx + 0x10) + (s32)*(u16 *)(elem + 0x0) * 8);
                        gte_ncds();
                        gte_strgb(prim + 0x4);
                        prim[7] = ctx[0x15];
                        prim = (u8 *)SubmitPolyFT4(prim, ctx);
                    }
                    list += 0x20;
                    elem += 0x20;
                } while (--count != 0);
                break;
            }

            case 0x2D01: {
                u8 *elem = list + 0x14;

                /* I: POLY_FT4, depth-cued */
                prim[3] = 9;
                prim[7] = 0x2C;
                SetupPrimCode(prim, ctx);
                do {
                    if (ProjectQuadFace(prim, ctx, *(u16 *)(elem + 0x4), *(u16 *)(elem + 0x6),
                                        *(u16 *)(elem + 0x8), *(u16 *)(elem + 0xA),
                                        StoreSxyPolyFT4) == 0) {
                        *(u32 *)(prim + 0xC) = *(u32 *)(elem - 0x10);
                        *(u32 *)(prim + 0x14) = *(u32 *)(elem - 0xC);
                        *(u32 *)(prim + 0x1C) = *(u32 *)(elem - 0x8);
                        *(u32 *)(prim + 0x24) = *(u32 *)(elem - 0x4);
                        *(u16 *)(prim + 0xE) = *(u16 *)(prim + 0xE) +
                            (u16)((*(s32 *)(ctx + 0x24) >> *(s32 *)(ctx + 0x2C)) << 6);
                        gte_ldrgb(elem);
                        gte_dpcs();
                        gte_strgb(prim + 0x4);
                        prim[7] = ctx[0x15];
                        prim = (u8 *)SubmitPolyFT4(prim, ctx);
                    }
                    list += 0x20;
                    elem += 0x20;
                } while (--count != 0);
                break;
            }

            case 0x3101: {
                u8 *elem = list + 0x4;

                /* J: POLY_G3, depth-cued -- three colours, one dpct */
                prim[3] = 6;
                prim[7] = 0x30;
                SetupPrimCode(prim, ctx);
                do {
                    if (ProjectTriFace(prim, ctx, *(u16 *)(elem + 0xC), *(u16 *)(elem + 0xE),
                                       *(u16 *)(elem + 0x10), cbG3) == 0) {
                        gte_ldrgb3c(elem);
                        gte_dpct();
                        gte_strgb3_g3(prim);
                        prim[7] = ctx[0x15];
                        prim = (u8 *)SubmitPolyG3(prim, ctx);
                    }
                    list += 0x18;
                    elem += 0x18;
                } while (--count != 0);
                break;
            }

            case 0x3501: {
                u8 *elem = list + 0x18;

                /* K: POLY_GT3, depth-cued */
                prim[3] = 9;
                prim[7] = 0x34;
                SetupPrimCode(prim, ctx);
                do {
                    if (ProjectTriFace(prim, ctx, *(u16 *)(elem + 0x4), *(u16 *)(elem + 0x6),
                                       *(u16 *)(elem + 0x8), StoreSxyPolyGT3) == 0) {
                        *(u32 *)(prim + 0xC) = *(u32 *)(elem - 0x14);
                        *(u32 *)(prim + 0x18) = *(u32 *)(elem - 0x10);
                        *(u32 *)(prim + 0x24) = *(u32 *)(elem - 0xC);
                        *(u16 *)(prim + 0xE) = *(u16 *)(prim + 0xE) +
                            (u16)((*(s32 *)(ctx + 0x24) >> *(s32 *)(ctx + 0x2C)) << 6);
                        gte_ldrgb3(list + 0x10, list + 0x14, elem);
                        gte_dpct();
                        gte_strgb3(prim + 0x4, prim + 0x10, prim + 0x1C);
                        prim[7] = ctx[0x15];
                        prim = (u8 *)SubmitPolyGT3(prim, ctx);
                    }
                    list += 0x24;
                    elem += 0x24;
                } while (--count != 0);
                break;
            }

            case 0x3901: {
                u8 *elem = list + 0x10;

                /* L: POLY_G4, depth-cued -- three colours by dpct, the
                 * fourth by a second dpcs. */
                prim[3] = 8;
                prim[7] = 0x38;
                SetupPrimCode(prim, ctx);
                do {
                    if (ProjectQuadFace(prim, ctx, *(u16 *)(elem + 0x4), *(u16 *)(elem + 0x6),
                                        *(u16 *)(elem + 0x8), *(u16 *)(elem + 0xA),
                                        StoreSxyPolyG4) == 0) {
                        gte_ldrgb3c(list + 0x4);
                        gte_dpct();
                        gte_strgb3_g3(prim);
                        prim[7] = ctx[0x15];
                        gte_ldrgb(elem);
                        gte_dpcs();
                        gte_strgb(prim + 0x1C);
                        prim = (u8 *)SubmitPolyG4(prim, ctx);
                    }
                    list += 0x1C;
                    elem += 0x1C;
                } while (--count != 0);
                break;
            }

            case 0x3D01: {
                u8 *elem = list + 0x20;

                /* M: POLY_GT4, depth-cued -- the widest element, 0x2C bytes,
                 * all four UVs and all four colours. */
                prim[3] = 0xC;
                prim[7] = 0x3C;
                SetupPrimCode(prim, ctx);
                do {
                    if (ProjectQuadFace(prim, ctx, *(u16 *)(elem + 0x4), *(u16 *)(elem + 0x6),
                                        *(u16 *)(elem + 0x8), *(u16 *)(elem + 0xA),
                                        StoreSxyPolyGT4) == 0) {
                        *(u32 *)(prim + 0xC) = *(u32 *)(elem - 0x1C);
                        *(u32 *)(prim + 0x18) = *(u32 *)(elem - 0x18);
                        *(u32 *)(prim + 0x24) = *(u32 *)(elem - 0x14);
                        *(u32 *)(prim + 0x30) = *(u32 *)(elem - 0x10);
                        *(u16 *)(prim + 0xE) = *(u16 *)(prim + 0xE) +
                            (u16)((*(s32 *)(ctx + 0x24) >> *(s32 *)(ctx + 0x2C)) << 6);
                        gte_ldrgb3(list + 0x14, list + 0x18, list + 0x1C);
                        gte_dpct();
                        gte_strgb3(prim + 0x4, prim + 0x10, prim + 0x1C);
                        prim[7] = ctx[0x15];
                        gte_ldrgb(elem);
                        gte_dpcs();
                        gte_strgb(prim + 0x28);
                        prim = (u8 *)SubmitPolyGT4(prim, ctx);
                    }
                    list += 0x2C;
                    elem += 0x2C;
                } while (--count != 0);
                break;
            }

            default:
                return;
            }

            GsOUT_PACKET_P = prim;
        } while (remaining != 0);
    }
}
#endif
```

### Proposed learning

**1. When retail computes an address into a temporary and then accesses it at
offset `0x0`, that is a macro's operand form, not a scheduling artifact.**
The discriminator is repetition: one `addiu`/`swc2 ... 0x0(reg)` pair could be
anything, fifteen of them with zero counter-examples is an operand convention.
Folding the offset into the instruction (`swc2 $22, 0x4(base)`) is a word
shorter per call site and looks harmless, which is why it survives review. The
inverse also holds and is the cheaper direction to check first: if a `gte_*`
macro already exists for the instruction, its operand count and op count are
readable in `include/psyq/inline.h` even though its macro-call words are the
ASPSX encoding and tell you nothing about the bytes. Count the `volatile` lines
in the SDK macro, subtract the `move $12,%0` setup lines, and compare against
the block length in retail — that identified all ten macros used here without
decoding a single word.

**2. `sltiu` against *value + 1*, plus case bodies at ascending addresses in
ascending case-value order, is the signature of a C `switch`.** GCC 2.6.3
expands a switch over sparse values as a balanced binary search over the SORTED
case list, so the first comparison is against the median, not against the first
case in source order — which is exactly why the ladder reads like a
hand-written nested range cascade. The distinguishing evidence is never in the
comparisons; it is in where the bodies sit. An if/else chain can reproduce
every comparison and still lay the bodies out in the wrong order, and on a
13-way dispatch the layout is most of the function. `docs/DECOMPILATION_LEARNINGS.md`
has no entry for this and round 50 was actively warned off `switch` by a
plausible-sounding argument that had the evidence backwards.

**3. Two `addiu` of the same stride in a loop tail, on pointers that differ by
a constant, means GCC strength-reduced a pointer into a second induction
variable that retail does not have.** The cost is exactly two words per loop
(one setup, one increment) plus, if it pushes the function over seven
callee-saved registers, two more in the prologue and epilogue. This is worth
screening for BEFORE reading any per-instruction diff on a function with many
loops, for the same reason the round-50 report gave for checking the stack
frame first: it is a whole-function length effect and it invalidates every
address after the first loop. The trigger is an inline-asm `"r"` operand on a
loop pointer — it is a plain register use, so GCC cannot eliminate the biv, and
the pointer's constant-offset memory uses get reduced into a separate register
alongside it.

**4. Attributing a frame-size gap to a named local is a guess worth
re-testing.** Round 50 named `nBase`/`tri0`/`tri1`/`tri2` as the likely eighth
saved register and proposed inlining them. This round inlined all four and the
frame did not move, because the real cause was a compiler-created temporary
with no source-level name at all. A register that appears in the built object
but not in the source is the signal to look at loop transformations rather than
at the variables you can see.

## Round 50 update (2026-09-17, runner bravo): both structural questions
## from round 45 are RESOLVED, first full-function C attempt written and
## built, confirmed not byte-exact. Restored to `INCLUDE_ASM`.

This round's assignment was exactly the two things round 45 named as
"settle before writing C for the bulk," plus a first cold attempt at the
whole function. Both are resolved below with hard evidence (not guesses),
and are the round's primary deliverable per the assignment — they were
posted to the broadcast as soon as each was nailed down.

### 1. Phase 2's `mvmva` spelling — RESOLVED, it is a raw `.word`, and it is now `gte_llir()`

Round 45 flagged `include/psyq/inline.h`'s `gte_llir()` as an untested guess
for `mvmva(sf=1, mx=0, v=3, cv=3, lm=0)` and said to check it against the
pinned pipeline before trusting it. Doing that:

```sh
$ cat > /tmp/t.c <<'EOF'
void test(void) { __asm__ volatile ("mvmva 1, 0, 3, 3, 0"); }
EOF
$ tools/gcc263/cpp ... | tools/gcc263/cc1 ... | .venv/bin/python3 tools/maspsx/maspsx.py $(MASPSX_FLAGS) \
  | tools/binutils/bin/mipsel-linux-gnu-as -march=r3000 -EL -no-pad-sections -G0 -o /tmp/t.o
{standard input}: Assembler messages:
{standard input}:18: Error: unrecognized opcode `mvmva 1,0,3,3,0'
```

The pinned `as` rejects the bare `mvmva` mnemonic outright — splat's own
disassembly text (`mvmva 1, 0, 3, 3, 0` in the `.s`) is a **display-only**
decode, not something the assembler accepts as input. So this is the raw-word
case, same as `gte_rtps`/`gte_rtpt`/`gte_nclip`/`gte_avsz3`.

The word itself is retail's own bytes: all three occurrences in this
function are `.word 0x4A49E012` (bytes `12 E0 49 4A`), 2 leading nops, same
convention as the other cofun macros. Verified this specific word round-trips
through the pinned pipeline correctly:

```sh
$ cat > /tmp/t2.c <<'EOF'
void test(void) { __asm__ volatile ("nop\n\tnop\n\t.word 0x4A49E012"); }
EOF
$ ... | tools/binutils/bin/mipsel-linux-gnu-objdump -d /tmp/t2.o
   8:	4a49e012 	c2	0x49e012
```

**Crucially, this is a DIFFERENT value from `INLINE.H`'s own `gte_llir()`**
(`.word 0x0000133f`/`0x133e`/`0x133e`), because that header's `.word`s are
Sony's **ASPSX macro-CALL** encoding — only Sony's own assembler expands
those into real instructions, exactly as `include/gte.h`'s file banner
already documents for why `INLINE.H` can't be used directly through this
pipeline. What DOES carry over from `INLINE.H` is the **name**: its own
naming scheme (`gte_ll`=local matrix, `gte_llv0/1/2`=matrix×vertex 0/1/2,
`gte_llir`=matrix×IR-vector, `*tr/*bk/*fc` suffixes for which translation
vector) matches `mx=0` ("local"), `v=3` ("IR"), `cv=3` ("none", no suffix)
field-for-field. So the fix is: add `gte_llir()` to `include/gte.h` spelled
with the REAL retail word, not the `INLINE.H` macro-call word, keeping
Sony's name. Done — see the diff to `include/gte.h` in this round.

**Bonus finding, same mechanism:** phase 3 (the per-face dispatch) uses three
MORE cofun ops with no macro yet: `ncds` (bytes `13 04 E8 4A` =
`0x4AE80413`), `dpcs` (`10 00 78 4A` = `0x4A780010`), `dpct`
(`2A 00 F8 4A` = `0x4AF8002A`). Command numbers 0x13/0x10/0x2A match Sony's
own NCDS/DPCS/DPCT GTE command numbers exactly. Same "as rejects the
mnemonic, confirmed with the reproducer" story for all three. Added as
`gte_ncds()`/`gte_dpcs()`/`gte_dpct()`, same 2-nop convention.

### 2. Phase 3's "unrolled slots vs. real loop" — RESOLVED: it is 13 real loops, not 9

Round 45's report said "roughly nine ~37-word blocks" and asked whether they
are unrolled named slots or loop iterations. Reading the full phase 3
disassembly line by line (`asm/nonmatchings/TmdRenderer/SortTmdObject.s`,
roughly lines 148–940) settles it completely:

- There are **13 distinct dispatch targets**, not 9 (one per PS1 GPU
  primitive flavor × opaque/depth-cued variant: F3×2, FT3×2, G3, GT3×2,
  F4×2 [one via the triangle-shaped submit path, see the case table], G4×2,
  GT4×2 — 13 total, matching the 13 distinct tag constants compared:
  `0x2000, 0x2004, 0x2101, 0x2400, 0x2501, 0x2800, 0x2901, 0x2C00, 0x2D01,
  0x3101, 0x3501, 0x3901, 0x3D01`).
- **Every single one of the 13 has its own `bnez $s4, .L...` back-edge**,
  its own element stride, and its own callback/submit-function pair. None of
  them are unrolled named slots — they are all real `for`/`do-while` loops
  over the group's own element count.
- The **outer structure is one `do`/`while` loop over "groups"**
  (`.L80018760`…`.L80019310`, `bnez $s5, .L80018760`): each outer iteration
  reads one group header (a 2-byte count + a 2-byte tag, plus one bit
  extracted from the same 32-bit word), dispatches via the 13-way cascade to
  the matching case body, and that case body's OWN inner loop consumes
  exactly that group's `count` elements before returning control to the
  outer loop for the next group. `$s5` (my `remaining`) is the running total
  across ALL groups; `$s4` (my `count`) is reused as the PER-GROUP loop
  counter inside whichever case fires.
- The specific "genuine back-edge loop at `.L80019300`" round 45 flagged
  (case M below, tag `0x3D01`, the largest element at 0x2C bytes) is
  structurally **identical** to the other 12 — it isn't special, it's just
  the last case in the cascade and the one round 45 happened to read far
  enough to see clearly.

So the phase-3 shape is: **outer `do { ... } while (remaining)` over face
groups, dispatching via a 13-way nested-if cascade (mirroring retail's exact
range-check nesting, not a plain `switch`) to one of 13 case bodies, each
itself `elem = list + K; selfFlags(); do { transform(); if (!culled)
{ color/UV; gteOp(); submit(); } elem += stride; } while (--count);`.**

### The full case table (tag, self[3]/self[7], stride, offsets, callback, submit)

| case | tag | self[3] | self[7] | elem init | stride | idx offsets | transform (cb) | color/GTE op | submit |
|---|---|---|---|---|---|---|---|---|---|
| A | 0x2000 | 4 | 0x20 | list+4 | 0x10 | +6,+8,+A | tri (StoreSxyPolyF3) | preload norm@+4, color@+0, `ncds` | SubmitPolyF3 |
| B | 0x2004 | 6 | 0x30 | list+0xC | 0x18 | +6,+8,+A | tri (cbG3=StoreSxyPolyG3) | 3× per-vertex `ncds` from list+4/list+8/elem+0, one norm preload | SubmitPolyG3 |
| C | 0x2101 | 4 | 0x20 | list+4 | 0x10 | +4,+6,+8 | tri (StoreSxyPolyF3) | color@+0, `dpcs`, no norm preload | SubmitPolyF3 |
| D | 0x2400 | 7 | 0x24 | list+0x10 | 0x18 | +2,+4,+6 | tri (StoreSxyPolyFT3) | 3 UV copies (elem-0xC/-8/-4), depth-cue shift, preload const color 0x34(prim) + norm@+0, `ncds` | SubmitPolyFT3 |
| E | 0x2501 | 7 | 0x24 | list+0x10 | 0x1C | +4,+6,+8 | tri (StoreSxyPolyFT3) | 3 UV copies (same offsets), depth-cue shift, color@+0, `dpcs` | SubmitPolyFT3 |
| F | 0x2800 | 5 | 0x28 | list+4 | 0x14 | +6,+8,+A,+C (quad) | quad (StoreSxyPolyF4) | norm@+4, color@+0, `ncds` | SubmitPolyF4 |
| G | 0x2901 | 5 | 0x28 | list+4 | 0x10 | +4,+6,+8,+A (quad) | quad (StoreSxyPolyF4) | color@+0, `dpcs`, no preload | SubmitPolyF4 |
| H | 0x2C00 | 9 | 0x2C | list+0x14 | 0x20 | +2,+4,+6,+8 (quad) | quad (StoreSxyPolyFT4) | 4 UV copies (elem-0x10/-C/-8/-4), depth-cue shift, preload const color + norm@+0, `ncds` | SubmitPolyFT4 |
| I | 0x2D01 | 9 | 0x2C | list+0x14 | 0x20 | +4,+6,+8,+A (quad) | quad (StoreSxyPolyFT4) | 4 UV copies (same), depth-cue shift, color@+0, `dpcs` | SubmitPolyFT4 |
| J | 0x3101 | 6 | 0x30 | list+4 | 0x18 | +C,+E,+10 | tri (cbG3=StoreSxyPolyG3) | 3 colors via regs $20/$21/$22 from elem+0/+4/+8, one `dpct` | SubmitPolyG3 |
| K | 0x3501 | 9 | 0x34 | list+0x18 | 0x24 | +4,+6,+8 | tri (StoreSxyPolyGT3) | 3 UV copies (elem-0x14/-0x10/-0xC), depth-cue shift, 3 colors via $20/$21/$22 from list+0x10/list+0x14/elem, `dpct` | SubmitPolyGT3 |
| L | 0x3901 | 8 | 0x38 | list+0x10 | 0x1C | +4,+6,+8,+A (quad) | quad (StoreSxyPolyG4) | 3 colors via $20/$21/$22 from list+4/+8/+C, `dpct`, PLUS a 4th color at elem+0 via separate `dpcs` | SubmitPolyG4 |
| M | 0x3D01 | 0xC | 0x3C | list+0x20 | 0x2C | +4,+6,+8,+A (quad) | quad (StoreSxyPolyGT4) | 4 UV copies (elem-0x1C/-18/-14/-10), depth-cue shift, 3 colors via $20/$21/$22 from list+0x14/+18/+1C, `dpct`, PLUS a 4th color at elem+0 via `dpcs` | SubmitPolyGT4 |

Cascade order exactly mirrors retail (checked first: `==0x2901`, then
`<0x2902` splits into `==0x2101`/`<0x2102`{`==0x2000`,`==0x2004`} vs.
`==0x2501`/`<0x2502`{`==0x2400`} vs. `==0x2800`; then the `>=0x2902` half
mirrors the same shape one tier up). This is a hand-written nested range
cascade, not a `switch` — writing it as a real C `switch` risks GCC 2.6.3
choosing a completely different (possibly jump-table) layout, so the
derived C below reproduces the cascade with literal nested `if`/`else if`.

The `SetupPrimCode(self, prim)` call (self[3]/self[7] set to the table
values above, immediately before it) recurs identically at the top of
every one of the 13 cases — this is the already-matched flag-byte packer
this same unit closed earlier.

### Secondary finding: the eight submit wrappers are NOT void — code_8220_c.c's stub is stale

`SubmitPolyF3`/`SubmitPolyG3`/`SubmitPolyFT3`/`SubmitPolyF4`/
`SubmitPolyG4`/`SubmitPolyFT4`/`SubmitPolyGT3`/`SubmitPolyGT4` are all
still `INCLUDE_ASM` in `src/code_8220_c.c`, and each one's own (unmatched,
`#if 0`-preserved) C stub currently declares itself `void`. That is provably
stale: every one of the eight falls straight from `jal RCpolyXX` into its
own epilogue with **no intervening store to `$v0`** (checked
`SubmitPolyF3.s`'s tail directly). That means whatever value Sony's own
`RCpolyXX` leaves in `$v0` — the well-known Psy-Q `p = RCpolyF3(p);`
work-buffer-advance idiom — IS this function's return value. `SortTmdObject`
relies on exactly that value (`self = SubmitPolyF3(self, prim);`, advancing
the packet-buffer write cursor every submit), so this file declares its own
`void *`-returning prototypes for all eight per CLAUDE.md's cross-unit
local-view exception, rather than importing the stale `void` ones. This is
worth fixing in `code_8220_c.c` itself when those functions are next worked,
but it is out of scope for this round (that file is untouched by this
session).

### First cold C attempt: written, built, confirmed NOT byte-exact

Wrote the full function (preamble, matrix save/transform/restore, all 13
per-face cases) using the two resolutions above, per the assignment's
"only then attempt C, phase by phase." It **compiles clean and links**
(`build exit=2`, zero hits on the compile-error grep, only the expected
whole-image SHA1 mismatch) but is **not close to byte-exact**:

```
SortTmdObject: 44/954 words match (file 0x8C64-0x9B4C)
WARNING: the build differs OUTSIDE this range too (323295 bytes) ...
```

Built length: **924 words vs retail's 954 — 30 words short.** First real
diff is at word 0 / file offset 0x8C64 (the prologue itself): retail's
frame is `-0x40` (8 callee-saved words + `$ra` = 9 words, 0x24, rounded to
0x40 with the outgoing-args area); mine is `-0x48`, one register too many
(`$s0`-`$s7`, 8 saved regs, vs retail's `$s0`-`$s6`, 7). Since the frame
size differs from word 0, **every address after it drifts**, which is why
the out-of-range byte count is so large and the 44/954 in-range number is
not a meaningful indicator of how close phases 1–2 individually are — per
CLAUDE.md's "address drift" entry, a per-function score is untrustworthy
once the length itself has changed.

**What was tried to close the 8th register, and what's still open:**
Confirmed via `objdump` on the built object that my source needs
`$s0..$s7` for: `self` (the packet-buffer cursor, reassigned throughout —
retail's own `$s0`), `prim` (`$s2`), `elem` (`$s3`), `list` (`$s4`), `count`
(`$s5`), `remaining` (`$s6`), `cbG3` (`$s7`), plus one more short-lived
per-case pointer that ends up needing a saved register too (visible as
`$s1` in the built object, used for a per-case helper computation like
`nBase`/`tri0` that in my source is a fresh local declared inside the `if`
body). Two things were tried and did NOT close the gap:
- Unifying the original `owner` argument and the per-iteration write
  cursor into one variable (`self`), matching retail's actual single-`$s0`
  reuse — this was necessary (retail really does reuse one register across
  both roles) but not sufficient on its own.
- Hoisting the 13 separate `u8 *elem = list + K;` case-local declarations
  into one `u8 *elem;` declared once at the top of the outer loop, assigned
  per case — same story: correct (retail really does reuse one `$s1` across
  every case), still not sufficient.

Not yet tried: whether the culprit is the short-lived per-case helper
pointers (`nBase`, `tri0`/`tri1`/`tri2`) getting promoted to a saved
register because their live range is judged (by this era's simple
allocator) to span the `gte_ncds()`/`gte_dpct()` inline-asm blocks that
follow them, even though at the C level they are dead by the time the
submit call happens. If so, the fix is likely to recompute those addresses
inline at each use (as retail's own asm does — e.g. `elem - 0xC` computed
directly in the `lw`/`sw` pair rather than materialized once into a named
pointer) rather than naming them at all. This is exactly the shape of
residue this project's own guide calls a live axis to vary, not a stall
condition — it just was not reached this round.

### Why this stops here as a STALL rather than a further iteration

This is, by a wide margin, the largest function in the corpus (954 words —
roughly 9× the previously-largest closed function in this unit). Getting
the STACK FRAME itself to match is a precondition for any of the 13 case
bodies' own internal register choices to mean anything, and that precondition
was not reached this round. Per the round's explicit budget guidance, an
untested, still-drifting whole-function translation is not something to keep
mutating blind — the responsible stop point is here, with the frame-size
gap named concretely (one extra callee-saved register, likely one of the
per-case helper pointers) rather than left as "doesn't match yet."

### Derived C, preserved (NOT matched, restore point for the next attempt)

This is the last state before reverting to `INCLUDE_ASM`. It compiles clean
against the current tree (with the `gte_llir`/`gte_ncds`/`gte_dpcs`/`gte_dpct`
macros added to `include/gte.h` this round) and needs the forward
declarations included below (they are NOT installed in the tree — only the
`include/gte.h` macro additions are committed this round). Building on this
means placing it back as the body of `SortTmdObject` in `src/TmdRenderer.c`,
in the same ROM-order position, and continuing from "one extra saved
register" above.

```c
#if 0
extern void *GsOUT_PACKET_P;
extern s32 D_80090C18;
extern s32 gSortUseGlobalLightMode;
extern s32 gSortLightMode;
extern s32 GsLIGHT_MODE;
extern s8 gTexturedFaceColor[3];

extern void InitDivPolygonPtrs(void *arg0, void *arg1, s32 kind);
extern s32 ProjectTriFace(void *arg0, u8 *prim, u16 idx0, u16 idx1, u16 idx2, void (*callback)(void *));
extern s32 ProjectQuadFace(void *arg0, u8 *prim, u16 idx0, u16 idx1, u16 idx2, u16 idx3, void (*callback)(void *, s32));
extern void SetupPrimCode(void *arg0, void *arg1);
extern void StoreSxyPolyF3(void *dst);
extern void StoreSxyPolyG3(void *dst);
extern void StoreSxyPolyFT3(void *dst);
extern void StoreSxyPolyGT3(void *dst);
extern void StoreSxyPolyF4(void *dst, s32 flag);
extern void StoreSxyPolyG4(void *dst, s32 flag);
extern void StoreSxyPolyFT4(void *dst, s32 flag);
extern void StoreSxyPolyGT4(void *dst, s32 flag);

/*
 * These eight are still INCLUDE_ASM in code_8220_c.c, declared void there.
 * That is an unverified placeholder, not a fact about retail: each one's
 * own tail falls straight into its epilogue immediately after `jal
 * RCpolyXX`, with no intervening store to $v0 -- see SubmitPolyF3.s --
 * so the value THIS function's call sites use (assigning the result back
 * into `self`) is exactly Sony's own RCpolyXX return, the standard
 * "p = RCpolyF3(p);" work-buffer-advance idiom. This file's own view,
 * per CLAUDE.md's cross-unit-prototype exception.
 */
extern void *SubmitPolyF3(void *arg0, void *arg1);
extern void *SubmitPolyG3(void *arg0, void *arg1);
extern void *SubmitPolyFT3(void *arg0, void *arg1);
extern void *SubmitPolyF4(void *arg0, void *arg1);
extern void *SubmitPolyG4(void *arg0, void *arg1);
extern void *SubmitPolyFT4(void *arg0, void *arg1);
extern void *SubmitPolyGT3(void *arg0, void *arg1);
extern void *SubmitPolyGT4(void *arg0, void *arg1);

void SortTmdObject(void *arg0, void *arg1, s32 arg2, void *arg3)
{
    u8 *self = (u8 *)arg0;
    u8 *prim = (u8 *)arg3;
    u8 *list;
    s32 remaining;
    s32 v0;

    if (*(s32 *)self < 0) {
        return;
    }

    *(void **)(prim + 0x0) = *(void **)((u8 *)arg1 + 0x4);
    *(s32 *)(prim + 0x4) = arg2;
    InitDivPolygonPtrs(prim + 0x88, gDivPolygon3, 3);
    InitDivPolygonPtrs(prim + 0x94, gDivPolygon4, 4);

    {
        u8 *mesh = *(u8 **)(self + 0x8);
        remaining = *(s32 *)(mesh + 0x14);
        list = *(u8 **)(mesh + 0x10);
        *(void **)(prim + 0xC) = *(void **)(mesh + 0x0);
    }
    *(void **)(prim + 0x10) = *(void **)(*(u8 **)(self + 0x8) + 0x8);

    if (*(s32 *)(*(u8 **)(self + 0x4) + 0x48) != 0) {
        u8 *ctx4 = *(u8 **)(self + 0x4);
        u8 *matSrc = *(u8 **)(ctx4 + 0x48) + 0x24;
        u32 c0, c1, c2, c3, c4, c5, c6, c7;
        u32 n0, n1, n2, n3, n4;
        s16 *vp;
        s32 vx, vy, vz;

        __asm__ volatile ("cfc2 %0, $0" : "=r" (c0));
        __asm__ volatile ("cfc2 %0, $1" : "=r" (c1));
        *(u32 *)(prim + 0x38) = c0;
        *(u32 *)(prim + 0x3C) = c1;
        __asm__ volatile ("cfc2 %0, $2" : "=r" (c2));
        __asm__ volatile ("cfc2 %0, $3" : "=r" (c3));
        __asm__ volatile ("cfc2 %0, $4" : "=r" (c4));
        *(u32 *)(prim + 0x40) = c2;
        *(u32 *)(prim + 0x44) = c3;
        *(u32 *)(prim + 0x48) = c4;
        __asm__ volatile ("cfc2 %0, $5" : "=r" (c5));
        __asm__ volatile ("cfc2 %0, $6" : "=r" (c6));
        __asm__ volatile ("cfc2 %0, $7" : "=r" (c7));
        *(u32 *)(prim + 0x4C) = c5;
        *(u32 *)(prim + 0x50) = c6;
        *(u32 *)(prim + 0x54) = c7;

        n0 = *(u32 *)(matSrc + 0x0);
        n1 = *(u32 *)(matSrc + 0x4);
        __asm__ volatile ("ctc2 %0, $0" : : "r" (n0));
        __asm__ volatile ("ctc2 %0, $1" : : "r" (n1));
        n2 = *(u32 *)(matSrc + 0x8);
        n3 = *(u32 *)(matSrc + 0xC);
        n4 = *(u32 *)(matSrc + 0x10);
        __asm__ volatile ("ctc2 %0, $2" : : "r" (n2));
        __asm__ volatile ("ctc2 %0, $3" : : "r" (n3));
        __asm__ volatile ("ctc2 %0, $4" : : "r" (n4));

        vp = (s16 *)(*(u8 **)(self + 0x4) + 0x24);
        vx = vp[0];
        vy = vp[3];
        vz = vp[6];
        __asm__ volatile ("mtc2 %0, $9"  : : "r" (vx));
        __asm__ volatile ("mtc2 %0, $10" : : "r" (vy));
        __asm__ volatile ("mtc2 %0, $11" : : "r" (vz));
        gte_llir();
        __asm__ volatile ("mfc2 %0, $9"  : "=r" (vx));
        __asm__ volatile ("mfc2 %0, $10" : "=r" (vy));
        __asm__ volatile ("mfc2 %0, $11" : "=r" (vz));
        vp[0] = (s16) vx;
        vp[3] = (s16) vy;
        vp[6] = (s16) vz;

        vp = (s16 *)(*(u8 **)(self + 0x4) + 0x26);
        vx = vp[0];
        vy = vp[3];
        vz = vp[6];
        __asm__ volatile ("mtc2 %0, $9"  : : "r" (vx));
        __asm__ volatile ("mtc2 %0, $10" : : "r" (vy));
        __asm__ volatile ("mtc2 %0, $11" : : "r" (vz));
        gte_llir();
        __asm__ volatile ("mfc2 %0, $9"  : "=r" (vx));
        __asm__ volatile ("mfc2 %0, $10" : "=r" (vy));
        __asm__ volatile ("mfc2 %0, $11" : "=r" (vz));
        vp[0] = (s16) vx;
        vp[3] = (s16) vy;
        vp[6] = (s16) vz;

        vp = (s16 *)(*(u8 **)(self + 0x4) + 0x28);
        vx = vp[0];
        vy = vp[3];
        vz = vp[6];
        __asm__ volatile ("mtc2 %0, $9"  : : "r" (vx));
        __asm__ volatile ("mtc2 %0, $10" : : "r" (vy));
        __asm__ volatile ("mtc2 %0, $11" : : "r" (vz));
        gte_llir();
        __asm__ volatile ("mfc2 %0, $9"  : "=r" (vx));
        __asm__ volatile ("mfc2 %0, $10" : "=r" (vy));
        __asm__ volatile ("mfc2 %0, $11" : "=r" (vz));
        vp[0] = (s16) vx;
        vp[3] = (s16) vy;
        vp[6] = (s16) vz;

        {
            u32 r0 = *(u32 *)(prim + 0x38), r1 = *(u32 *)(prim + 0x3C);
            __asm__ volatile ("ctc2 %0, $0" : : "r" (r0));
            __asm__ volatile ("ctc2 %0, $1" : : "r" (r1));
        }
        {
            u32 r2 = *(u32 *)(prim + 0x40), r3 = *(u32 *)(prim + 0x44), r4 = *(u32 *)(prim + 0x48);
            __asm__ volatile ("ctc2 %0, $2" : : "r" (r2));
            __asm__ volatile ("ctc2 %0, $3" : : "r" (r3));
            __asm__ volatile ("ctc2 %0, $4" : : "r" (r4));
        }
        {
            u32 r5 = *(u32 *)(prim + 0x4C), r6 = *(u32 *)(prim + 0x50), r7 = *(u32 *)(prim + 0x54);
            __asm__ volatile ("ctc2 %0, $5" : : "r" (r5));
            __asm__ volatile ("ctc2 %0, $6" : : "r" (r6));
            __asm__ volatile ("ctc2 %0, $7" : : "r" (r7));
        }
    }

    {
        u32 raw = *(u32 *)self;

        *(s32 *)(prim + 0x8) = 0xA;
        D_80090C18 = (raw >> 9) & 0x7;
        D_8008E248 = (raw >> 6) & 0x1;
        gSortUseGlobalLightMode = (raw >> 5) & 0x1;
        gSortLightMode = (raw >> 3) & 0x3;
    }
    prim[0x34] = gTexturedFaceColor[0];
    prim[0x35] = gTexturedFaceColor[1];
    prim[0x36] = gTexturedFaceColor[2];

    if (gSortUseGlobalLightMode != 0 && GsLIGHT_MODE != 0) {
        v0 = 9;
    } else if (gSortLightMode != 0) {
        v0 = 9;
    } else {
        v0 = 0x10;
    }
    *(s32 *)(prim + 0x2C) = v0;

    if (remaining == 0) {
        return;
    }

    {
        void (*cbG3)(void *) = StoreSxyPolyG3;

        do {
            u32 hdr = *(u32 *)list;
            s32 count = (s32) *(u16 *)(list + 0x0);
            u8 *elem;

            self = (u8 *) GsOUT_PACKET_P;
            *(s32 *)(prim + 0x18) = (s32)(u16)(*(u16 *)(list + 0x2) & 0xFD07);
            *(s32 *)(prim + 0x1C) = (hdr >> 25) & 1;
            remaining -= count;

            if (*(s32 *)(prim + 0x18) == 0x2901) {
                /* G: POLY_G4, opaque */
                elem = list + 0x4;
                self[3] = 5;
                self[7] = 0x28;
                SetupPrimCode(self, prim);
                do {
                    u16 idx0 = *(u16 *)(elem + 0x4);
                    u16 idx1 = *(u16 *)(elem + 0x6);
                    u16 idx2 = *(u16 *)(elem + 0x8);
                    u16 idx3 = *(u16 *)(elem + 0xA);

                    if (ProjectQuadFace(self, prim, idx0, idx1, idx2, idx3, StoreSxyPolyF4) == 0) {
                        __asm__ volatile ("lwc2 $6, 0x0(%0)" : : "r" (elem) : "memory");
                        gte_dpcs();
                        __asm__ volatile ("swc2 $22, 0x4(%0)" : : "r" (self) : "memory");
                        self[7] = *(u8 *)(prim + 0x15);
                        self = (u8 *) SubmitPolyF4(self, prim);
                    }
                    list += 0x10;
                    elem += 0x10;
                } while (--count != 0);
            } else if (*(s32 *)(prim + 0x18) < 0x2902) {
                if (*(s32 *)(prim + 0x18) == 0x2101) {
                    /* C: POLY_F3, depth-cued */
                    elem = list + 0x4;
                    self[3] = 4;
                    self[7] = 0x20;
                    SetupPrimCode(self, prim);
                    do {
                        u16 idx0 = *(u16 *)(elem + 0x4);
                        u16 idx1 = *(u16 *)(elem + 0x6);
                        u16 idx2 = *(u16 *)(elem + 0x8);

                        if (ProjectTriFace(self, prim, idx0, idx1, idx2, StoreSxyPolyF3) == 0) {
                            __asm__ volatile ("lwc2 $6, 0x0(%0)" : : "r" (elem) : "memory");
                            gte_dpcs();
                            __asm__ volatile ("swc2 $22, 0x4(%0)" : : "r" (self) : "memory");
                            self[7] = *(u8 *)(prim + 0x15);
                            self = (u8 *) SubmitPolyF3(self, prim);
                        }
                        list += 0x10;
                        elem += 0x10;
                    } while (--count != 0);
                } else if (*(s32 *)(prim + 0x18) < 0x2102) {
                    if (*(s32 *)(prim + 0x18) == 0x2000) {
                        /* A: POLY_F3, opaque */
                        elem = list + 0x4;
                        self[3] = 4;
                        self[7] = 0x20;
                        SetupPrimCode(self, prim);
                        do {
                            u16 idx0 = *(u16 *)(elem + 0x6);
                            u16 idx1 = *(u16 *)(elem + 0x8);
                            u16 idx2 = *(u16 *)(elem + 0xA);

                            if (ProjectTriFace(self, prim, idx0, idx1, idx2, StoreSxyPolyF3) == 0) {
                                u16 nIdx = *(u16 *)(elem + 0x4);
                                u8 *nBase = *(u8 **)(prim + 0x10) + (s32) nIdx * 8;

                                __asm__ volatile ("lwc2 $0, 0x0(%0)\n\tlwc2 $1, 0x4(%0)" : : "r" (nBase));
                                __asm__ volatile ("lwc2 $6, 0x0(%0)" : : "r" (elem) : "memory");
                                gte_ncds();
                                __asm__ volatile ("swc2 $22, 0x4(%0)" : : "r" (self) : "memory");
                                self[7] = *(u8 *)(prim + 0x15);
                                self = (u8 *) SubmitPolyF3(self, prim);
                            }
                            list += 0x10;
                            elem += 0x10;
                        } while (--count != 0);
                    } else if (*(s32 *)(prim + 0x18) == 0x2004) {
                        /* B: POLY_G3, opaque */
                        elem = list + 0xC;
                        self[3] = 6;
                        self[7] = 0x30;
                        SetupPrimCode(self, prim);
                        do {
                            u16 idx0 = *(u16 *)(elem + 0x6);
                            u16 idx1 = *(u16 *)(elem + 0x8);
                            u16 idx2 = *(u16 *)(elem + 0xA);

                            if (ProjectTriFace(self, prim, idx0, idx1, idx2, cbG3) == 0) {
                                u16 nIdx = *(u16 *)(elem + 0x4);
                                u8 *nBase = *(u8 **)(prim + 0x10) + (s32) nIdx * 8;

                                __asm__ volatile ("lwc2 $0, 0x0(%0)\n\tlwc2 $1, 0x4(%0)" : : "r" (nBase));
                                __asm__ volatile ("lwc2 $6, 0x0(%0)" : : "r" (list + 0x4) : "memory");
                                gte_ncds();
                                __asm__ volatile ("swc2 $22, 0x4(%0)" : : "r" (self) : "memory");
                                __asm__ volatile ("lwc2 $6, 0x0(%0)" : : "r" (list + 0x8) : "memory");
                                gte_ncds();
                                __asm__ volatile ("swc2 $22, 0xC(%0)" : : "r" (self) : "memory");
                                self[7] = *(u8 *)(prim + 0x15);
                                __asm__ volatile ("lwc2 $6, 0x0(%0)" : : "r" (elem) : "memory");
                                gte_ncds();
                                __asm__ volatile ("swc2 $22, 0x14(%0)" : : "r" (self) : "memory");
                                self = (u8 *) SubmitPolyG3(self, prim);
                            }
                            list += 0x18;
                            elem += 0x18;
                        } while (--count != 0);
                    } else {
                        return;
                    }
                } else {
                    if (*(s32 *)(prim + 0x18) == 0x2501) {
                        /* E: POLY_FT3, depth-cued */
                        elem = list + 0x10;
                        self[3] = 7;
                        self[7] = 0x24;
                        SetupPrimCode(self, prim);
                        do {
                            u16 idx0 = *(u16 *)(elem + 0x4);
                            u16 idx1 = *(u16 *)(elem + 0x6);
                            u16 idx2 = *(u16 *)(elem + 0x8);

                            if (ProjectTriFace(self, prim, idx0, idx1, idx2, StoreSxyPolyFT3) == 0) {
                                s32 shift;

                                *(u32 *)self = *(u32 *)(elem - 0xC);
                                *(u32 *)(self + 0xC) = *(u32 *)(elem - 0x8);
                                *(u32 *)(self + 0x14) = *(u32 *)(elem - 0x4);
                                shift = *(s32 *)(prim + 0x24) >> *(s32 *)(prim + 0x2C);
                                *(u16 *)(self + 0xE) = *(u16 *)(self + 0xE) + (u16)(shift << 6);
                                __asm__ volatile ("lwc2 $6, 0x0(%0)" : : "r" (elem) : "memory");
                                gte_dpcs();
                                __asm__ volatile ("swc2 $22, 0x4(%0)" : : "r" (self) : "memory");
                                self[7] = *(u8 *)(prim + 0x15);
                                self = (u8 *) SubmitPolyFT3(self, prim);
                            }
                            list += 0x1C;
                            elem += 0x1C;
                        } while (--count != 0);
                    } else if (*(s32 *)(prim + 0x18) < 0x2502) {
                        if (*(s32 *)(prim + 0x18) == 0x2400) {
                            /* D: POLY_FT3, opaque */
                            elem = list + 0x10;

                            self[3] = 7;
                            self[7] = 0x24;
                            SetupPrimCode(self, prim);
                            __asm__ volatile ("lwc2 $6, 0x34(%0)" : : "r" (prim) : "memory");
                            do {
                                u16 idx0 = *(u16 *)(elem + 0x2);
                                u16 idx1 = *(u16 *)(elem + 0x4);
                                u16 idx2 = *(u16 *)(elem + 0x6);

                                if (ProjectTriFace(self, prim, idx0, idx1, idx2, StoreSxyPolyFT3) == 0) {
                                    u16 nIdx = *(u16 *)(elem + 0x0);
                                    u8 *nBase = *(u8 **)(prim + 0x10) + (s32) nIdx * 8;
                                    s32 shift;

                                    *(u32 *)self = *(u32 *)(elem - 0xC);
                                    *(u32 *)(self + 0xC) = *(u32 *)(elem - 0x8);
                                    *(u32 *)(self + 0x14) = *(u32 *)(elem - 0x4);
                                    shift = *(s32 *)(prim + 0x24) >> *(s32 *)(prim + 0x2C);
                                    *(u16 *)(self + 0xE) = *(u16 *)(self + 0xE) + (u16)(shift << 6);
                                    __asm__ volatile ("lwc2 $0, 0x0(%0)\n\tlwc2 $1, 0x4(%0)" : : "r" (nBase));
                                    gte_ncds();
                                    __asm__ volatile ("swc2 $22, 0x4(%0)" : : "r" (self) : "memory");
                                    self[7] = *(u8 *)(prim + 0x15);
                                    self = (u8 *) SubmitPolyFT3(self, prim);
                                }
                                list += 0x18;
                                elem += 0x18;
                            } while (--count != 0);
                        } else {
                            return;
                        }
                    } else {
                        if (*(s32 *)(prim + 0x18) == 0x2800) {
                            /* F: POLY_G4, opaque (quad) */
                            elem = list + 0x4;
                            self[3] = 5;
                            self[7] = 0x28;
                            SetupPrimCode(self, prim);
                            do {
                                u16 idx0 = *(u16 *)(elem + 0x6);
                                u16 idx1 = *(u16 *)(elem + 0x8);
                                u16 idx2 = *(u16 *)(elem + 0xA);
                                u16 idx3 = *(u16 *)(elem + 0xC);

                                if (ProjectQuadFace(self, prim, idx0, idx1, idx2, idx3, StoreSxyPolyF4) == 0) {
                                    u16 nIdx = *(u16 *)(elem + 0x4);
                                    u8 *nBase = *(u8 **)(prim + 0x10) + (s32) nIdx * 8;

                                    __asm__ volatile ("lwc2 $0, 0x0(%0)\n\tlwc2 $1, 0x4(%0)" : : "r" (nBase));
                                    __asm__ volatile ("lwc2 $6, 0x0(%0)" : : "r" (elem) : "memory");
                                    gte_ncds();
                                    __asm__ volatile ("swc2 $22, 0x4(%0)" : : "r" (self) : "memory");
                                    self[7] = *(u8 *)(prim + 0x15);
                                    self = (u8 *) SubmitPolyF4(self, prim);
                                }
                                list += 0x14;
                                elem += 0x14;
                            } while (--count != 0);
                        } else {
                            return;
                        }
                    }
                }
            } else {
                if (*(s32 *)(prim + 0x18) == 0x3101) {
                    /* J: depth-cued gouraud, untextured triple */
                    elem = list + 0x4;
                    self[3] = 6;
                    self[7] = 0x30;
                    SetupPrimCode(self, prim);
                    do {
                        u16 idx0 = *(u16 *)(elem + 0xC);
                        u16 idx1 = *(u16 *)(elem + 0xE);
                        u16 idx2 = *(u16 *)(elem + 0x10);

                        if (ProjectTriFace(self, prim, idx0, idx1, idx2, cbG3) == 0) {
                            __asm__ volatile (
                                "lwc2 $20, 0x0(%0)\n\t"
                                "lwc2 $21, 0x4(%0)\n\t"
                                "lwc2 $22, 0x8(%0)\n\t"
                                "lwc2 $6, 0x8(%0)"
                                : : "r" (elem) : "memory");
                            gte_dpct();
                            __asm__ volatile (
                                "swc2 $20, 0x4(%0)\n\t"
                                "swc2 $21, 0xC(%0)\n\t"
                                "swc2 $22, 0x14(%0)"
                                : : "r" (self) : "memory");
                            self[7] = *(u8 *)(prim + 0x15);
                            self = (u8 *) SubmitPolyG3(self, prim);
                        }
                        list += 0x18;
                        elem += 0x18;
                    } while (--count != 0);
                } else if (*(s32 *)(prim + 0x18) < 0x3102) {
                    if (*(s32 *)(prim + 0x18) == 0x2C00) {
                        /* H: POLY_GT4, opaque */
                        elem = list + 0x14;

                        self[3] = 9;
                        self[7] = 0x2C;
                        SetupPrimCode(self, prim);
                        __asm__ volatile ("lwc2 $6, 0x34(%0)" : : "r" (prim) : "memory");
                        do {
                            u16 idx0 = *(u16 *)(elem + 0x2);
                            u16 idx1 = *(u16 *)(elem + 0x4);
                            u16 idx2 = *(u16 *)(elem + 0x6);
                            u16 idx3 = *(u16 *)(elem + 0x8);

                            if (ProjectQuadFace(self, prim, idx0, idx1, idx2, idx3, StoreSxyPolyFT4) == 0) {
                                u16 nIdx = *(u16 *)(elem + 0x0);
                                u8 *nBase = *(u8 **)(prim + 0x10) + (s32) nIdx * 8;
                                s32 shift;

                                *(u32 *)(self + 0xC) = *(u32 *)(elem - 0x10);
                                *(u32 *)(self + 0x14) = *(u32 *)(elem - 0xC);
                                *(u32 *)(self + 0x1C) = *(u32 *)(elem - 0x8);
                                *(u32 *)(self + 0x24) = *(u32 *)(elem - 0x4);
                                shift = *(s32 *)(prim + 0x24) >> *(s32 *)(prim + 0x2C);
                                *(u16 *)(self + 0xE) = *(u16 *)(self + 0xE) + (u16)(shift << 6);
                                __asm__ volatile ("lwc2 $0, 0x0(%0)\n\tlwc2 $1, 0x4(%0)" : : "r" (nBase));
                                gte_ncds();
                                __asm__ volatile ("swc2 $22, 0x4(%0)" : : "r" (self) : "memory");
                                self[7] = *(u8 *)(prim + 0x15);
                                self = (u8 *) SubmitPolyFT4(self, prim);
                            }
                            list += 0x20;
                            elem += 0x20;
                        } while (--count != 0);
                    } else if (*(s32 *)(prim + 0x18) == 0x2D01) {
                        /* I: POLY_GT4, depth-cued */
                        elem = list + 0x14;
                        self[3] = 9;
                        self[7] = 0x2C;
                        SetupPrimCode(self, prim);
                        do {
                            u16 idx0 = *(u16 *)(elem + 0x4);
                            u16 idx1 = *(u16 *)(elem + 0x6);
                            u16 idx2 = *(u16 *)(elem + 0x8);
                            u16 idx3 = *(u16 *)(elem + 0xA);

                            if (ProjectQuadFace(self, prim, idx0, idx1, idx2, idx3, StoreSxyPolyFT4) == 0) {
                                s32 shift;

                                *(u32 *)(self + 0xC) = *(u32 *)(elem - 0x10);
                                *(u32 *)(self + 0x14) = *(u32 *)(elem - 0xC);
                                *(u32 *)(self + 0x1C) = *(u32 *)(elem - 0x8);
                                *(u32 *)(self + 0x24) = *(u32 *)(elem - 0x4);
                                shift = *(s32 *)(prim + 0x24) >> *(s32 *)(prim + 0x2C);
                                *(u16 *)(self + 0xE) = *(u16 *)(self + 0xE) + (u16)(shift << 6);
                                __asm__ volatile ("lwc2 $6, 0x0(%0)" : : "r" (elem) : "memory");
                                gte_dpcs();
                                __asm__ volatile ("swc2 $22, 0x4(%0)" : : "r" (self) : "memory");
                                self[7] = *(u8 *)(prim + 0x15);
                                self = (u8 *) SubmitPolyFT4(self, prim);
                            }
                            list += 0x20;
                            elem += 0x20;
                        } while (--count != 0);
                    } else {
                        return;
                    }
                } else {
                    if (*(s32 *)(prim + 0x18) == 0x3901) {
                        /* L: POLY_G4, depth-cued */
                        elem = list + 0x10;
                        self[3] = 8;
                        self[7] = 0x38;
                        SetupPrimCode(self, prim);
                        do {
                            u16 idx0 = *(u16 *)(elem + 0x4);
                            u16 idx1 = *(u16 *)(elem + 0x6);
                            u16 idx2 = *(u16 *)(elem + 0x8);
                            u16 idx3 = *(u16 *)(elem + 0xA);

                            if (ProjectQuadFace(self, prim, idx0, idx1, idx2, idx3, StoreSxyPolyG4) == 0) {
                                u8 *tri = list + 0x4;

                                __asm__ volatile (
                                    "lwc2 $20, 0x0(%0)\n\t"
                                    "lwc2 $21, 0x4(%0)\n\t"
                                    "lwc2 $22, 0x8(%0)\n\t"
                                    "lwc2 $6, 0x8(%0)"
                                    : : "r" (tri) : "memory");
                                gte_dpct();
                                __asm__ volatile (
                                    "swc2 $20, 0x4(%0)\n\t"
                                    "swc2 $21, 0xC(%0)\n\t"
                                    "swc2 $22, 0x14(%0)"
                                    : : "r" (self) : "memory");
                                self[7] = *(u8 *)(prim + 0x15);
                                __asm__ volatile ("lwc2 $6, 0x0(%0)" : : "r" (elem) : "memory");
                                gte_dpcs();
                                __asm__ volatile ("swc2 $22, 0x1C(%0)" : : "r" (self) : "memory");
                                self = (u8 *) SubmitPolyG4(self, prim);
                            }
                            list += 0x1C;
                            elem += 0x1C;
                        } while (--count != 0);
                    } else if (*(s32 *)(prim + 0x18) < 0x3902) {
                        if (*(s32 *)(prim + 0x18) == 0x3501) {
                            /* K: depth-cued gouraud, untextured triple, UV-bearing */
                            elem = list + 0x18;
                            self[3] = 9;
                            self[7] = 0x34;
                            SetupPrimCode(self, prim);
                            do {
                                u16 idx0 = *(u16 *)(elem + 0x4);
                                u16 idx1 = *(u16 *)(elem + 0x6);
                                u16 idx2 = *(u16 *)(elem + 0x8);

                                if (ProjectTriFace(self, prim, idx0, idx1, idx2, StoreSxyPolyGT3) == 0) {
                                    u8 *tri0 = list + 0x10;
                                    u8 *tri1 = list + 0x14;
                                    s32 shift;

                                    *(u32 *)(self + 0xC) = *(u32 *)(elem - 0x14);
                                    *(u32 *)(self + 0x18) = *(u32 *)(elem - 0x10);
                                    *(u32 *)(self + 0x24) = *(u32 *)(elem - 0xC);
                                    shift = *(s32 *)(prim + 0x24) >> *(s32 *)(prim + 0x2C);
                                    *(u16 *)(self + 0xE) = *(u16 *)(self + 0xE) + (u16)(shift << 6);
                                    __asm__ volatile (
                                        "lwc2 $20, 0x0(%0)\n\t"
                                        "lwc2 $21, 0x0(%1)\n\t"
                                        "lwc2 $22, 0x0(%2)\n\t"
                                        "lwc2 $6, 0x0(%2)"
                                        : : "r" (tri0), "r" (tri1), "r" (elem) : "memory");
                                    gte_dpct();
                                    __asm__ volatile (
                                        "swc2 $20, 0x4(%0)\n\t"
                                        "swc2 $21, 0x10(%0)\n\t"
                                        "swc2 $22, 0x1C(%0)"
                                        : : "r" (self) : "memory");
                                    self[7] = *(u8 *)(prim + 0x15);
                                    self = (u8 *) SubmitPolyGT3(self, prim);
                                }
                                list += 0x24;
                                elem += 0x24;
                            } while (--count != 0);
                        } else {
                            return;
                        }
                    } else {
                        if (*(s32 *)(prim + 0x18) == 0x3D01) {
                            /* M: POLY_GT4, depth-cued gouraud (the round-45
                             * "genuine back-edge loop"; structurally identical
                             * to the other 12 case bodies, just the largest
                             * element -- 0x2C bytes, all four UVs). */
                            elem = list + 0x20;
                            self[3] = 0xC;
                            self[7] = 0x3C;
                            SetupPrimCode(self, prim);
                            do {
                                u16 idx0 = *(u16 *)(elem + 0x4);
                                u16 idx1 = *(u16 *)(elem + 0x6);
                                u16 idx2 = *(u16 *)(elem + 0x8);
                                u16 idx3 = *(u16 *)(elem + 0xA);

                                if (ProjectQuadFace(self, prim, idx0, idx1, idx2, idx3, StoreSxyPolyGT4) == 0) {
                                    u8 *tri0 = list + 0x14;
                                    u8 *tri1 = list + 0x18;
                                    u8 *tri2 = list + 0x1C;
                                    s32 shift;

                                    *(u32 *)(self + 0xC) = *(u32 *)(elem - 0x1C);
                                    *(u32 *)(self + 0x18) = *(u32 *)(elem - 0x18);
                                    *(u32 *)(self + 0x24) = *(u32 *)(elem - 0x14);
                                    *(u32 *)(self + 0x30) = *(u32 *)(elem - 0x10);
                                    shift = *(s32 *)(prim + 0x24) >> *(s32 *)(prim + 0x2C);
                                    *(u16 *)(self + 0xE) = *(u16 *)(self + 0xE) + (u16)(shift << 6);
                                    __asm__ volatile (
                                        "lwc2 $20, 0x0(%0)\n\t"
                                        "lwc2 $21, 0x0(%1)\n\t"
                                        "lwc2 $22, 0x0(%2)\n\t"
                                        "lwc2 $6, 0x0(%2)"
                                        : : "r" (tri0), "r" (tri1), "r" (tri2) : "memory");
                                    gte_dpct();
                                    __asm__ volatile (
                                        "swc2 $20, 0x4(%0)\n\t"
                                        "swc2 $21, 0x10(%0)\n\t"
                                        "swc2 $22, 0x1C(%0)"
                                        : : "r" (self) : "memory");
                                    self[7] = *(u8 *)(prim + 0x15);
                                    __asm__ volatile ("lwc2 $6, 0x0(%0)" : : "r" (elem) : "memory");
                                    gte_dpcs();
                                    __asm__ volatile ("swc2 $22, 0x28(%0)" : : "r" (self) : "memory");
                                    self = (u8 *) SubmitPolyGT4(self, prim);
                                }
                                list += 0x2C;
                                elem += 0x2C;
                            } while (--count != 0);
                        } else {
                            return;
                        }
                    }
                }
            }

            GsOUT_PACKET_P = self;
        } while (remaining != 0);
    }
}
#endif
```

### Proposed learning

**A function large enough that "does it compile" is itself uninformative
needs the STACK FRAME checked before anything else.** For a function this
size, `funcdiff`'s in-range word-match number is close to meaningless on a
first attempt — the very first word (the prologue's frame size) already
disagreed here, which invalidates every address after it. The cheap,
high-value check for a function in this size class, before reading ANY
per-instruction diff, is: `objdump -d` the built object, diff its saved-
register set against retail's own prologue/epilogue (`sw $sN, ...($sp)`
lines), and only then start reading the per-case bodies. This would have
saved the time spent producing the (currently untrustworthy) 44/954 number
above.

**The "eight submit wrappers return `void`" prototype in `code_8220_c.c`
is stale and should be corrected when those functions are next worked** —
see the secondary finding above. Flagging here rather than editing that
file, which is out of this round's scope.

## Naming (round 51, bravo) — NAME DELIBERATELY NOT CHANGED, and one of the two reasons is a head decision

The track-3 naming pass renamed all 19 other definitions in
`src/TmdRenderer.c`. This one kept `SortTmdObject`. Two reasons, in order
of weight.

### 1. Its only caller sits in an SDK segment, and that is a head call

`grep -rn 'SortTmdObject' src/ asm/` finds exactly one call site outside
this function's own `.s`: `asm/psyq_2864.s:225`, inside `Viewport__DrawNode`.
That caller calls `GsSortBoxFill`, `GsGetLws`, `GsSetLightMatrix` and
`GsSetLsMatrix` and nothing else identifiable, and it passes
`0x1F800000` — the PS1 scratchpad — as this function's fourth argument
(`lui $a3, 0x1F80` at 0x80012368). That is `GsSortObject4`-shaped code.

Evidence the other way, which is why this is a question and not a verdict:

- `python3 tools/sdkstalls.py` reports no placed Sony object overlapping
  this function (it prints "No live stalled function overlaps a placed
  Sony object" for the whole queue).
- The splat layout puts this function inside a contiguous game run:
  `0x8220` `code_8220` (the BasicClass framework, unambiguously game code)
  → `0x8A88` `code_8220_b` → `0x9F74` `code_8220_c` → `psyq_rcpolyf3`. The
  renderer sits directly between the game's own class framework and Sony's
  RCpoly primitives, which is what a game-written `GsSortObject4`
  replacement calling Sony's packers would look like.

`docs/PARALLEL-RUNS.md` §3.3 screen 3 says Sony ownership with no object on
any disc is decided by segment topology and the assembler fingerprint, by
the head, "never by smell". A runner attaching a game-style name to this
function would pre-empt that decision in the direction that is harder to
undo, since the name propagates to its report filename and to every
citation. Posted to the broadcast for the head.

### 2. The brief scoped this function out

The round-51 assignment said to leave the `INCLUDE_ASM` alone. A rename
changes zero bytes so it is not obviously covered, but combined with (1)
there was no reason to stretch it.

### Proposed name, if the head rules it game code

`SortModelFaces`. **Tier B.** The mechanics are not in doubt — they are
this report's own round-50 case table, independently re-derived this round
from the `(len, code)` pairs: it walks a model's face groups, dispatches on
each group's tag to one of 13 cases, and per face calls `SetupPrimCode`,
then `ProjectTriFace` or `ProjectQuadFace`, then one of the eight RCpoly*
wrappers in `TmdRenderer`. "Sort" is Psy-Q's verb for inserting into the
ordering table and matches the `GsSort*` caller; "Faces" is what the
per-group element lists are. Tier B rather than A because what `arg0` and
`arg1` are as classes is still unestablished.

### Round-51 corrections to this report's own content

- The case table's callbacks and submit wrappers were re-derived
  independently this round and agree with round 50 in every row. The
  `(len, code)` columns are not just tags: all eight distinct pairs are
  Sony's Psy-Q primitive definitions exactly — (4, 0x20) POLY_F3,
  (5, 0x28) POLY_F4, (6, 0x30) POLY_G3, (7, 0x24) POLY_FT3, (8, 0x38)
  POLY_G4, (9, 0x2C) POLY_FT4, (9, 0x34) POLY_GT3, (12, 0x3C) POLY_GT4.
  That is what named the six store leaves; see
  `docs/match-reports/SetupPrimCode.md`.
- This report describes `FlagLargePolyForDivide`'s second argument as a
  "primitive kind" code following `include/code_8220.h`'s old wording. It
  is a **vertex count**: that function walks `count` screen-XY pairs and
  computes their 2D bounding box (its own report derives the body). The
  numbers 3 and 4 are right; the reason given for them was not. The header
  comment is corrected; the preserved `#if 0` body below is left exactly as
  round 50 built it, since it is a restore point.
- The preserved `#if 0` body now spells the NEW names, because
  `tools/rename.py` rewrites report files along with the sources. Round 50
  wrote it against `func_8001934C`, `func_800193C0`, `func_800194A4`,
  `func_800196D4`, `func_800196E8`, `func_800196FC`, `func_80019710`,
  `func_80019724` and `func_8001974C`; those are now `SetupPrimCode`,
  `ProjectTriFace`, `ProjectQuadFace`, and
  `StoreSxyPolyF3`/`G3`/`FT3`/`GT3`/`F4`/`G4`. The old spellings are listed
  here so the block stays greppable under either name. `tools/stalesyms.py`
  reports nothing against this file, i.e. every symbol the preserved body
  calls is still live.

## Round 91 polish (delta, track 7)

Byte-identical after every step; the three oracles green at each commit.

### Naming

- `func_80018464` -> **`SortTmdObject`, tier A.** Round 51 held the name back
  pending a Sony-ownership ruling. That question is now measured: Sony's
  `GsSortObject4` exists (`libgs/objt2.o` on the 3.3, 3.5 and 3.6 discs,
  `objt.o` on 3.0) and is a different body -- it also stores GsTON from
  attribute bit 30, writes `ndiv`/`HWD0`/`VWD0` into the scratch block and
  tests GsLIOFF/GsLIGNR/GsLMODE against GsLIGHT_MODE with a different ladder;
  it has none of this function's InitDivPolygonPtrs calls, TMD-type switch or
  calls into TmdRenderer. `psyq_sdk.py match` never placed it here, and the
  caller (`Viewport__DrawNode`, formerly in `psyq_2864`) is carved game code
  now. What the body does is evident from it alone: same four arguments as
  `GsSortObject4` (`GsDOBJ2 *`, `GsOT *`, shift, scratch), it reads the
  GsDOBJ2's GsCOORDINATE2 and TMD object at Sony's offsets, walks the TMD's
  primitive list and writes one POLY_xx per surviving face into the
  GsOUT_PACKET_P buffer and the OT. "Sort" is libgs's verb for that.
- `D_800902E0` -> `GsLIGHT_MODE`: Sony's, pinned in `psyq-objects.ld` by eight
  libgs objects; `rename.py` names it as the only allowed rename.
- `D_8008E24C` -> `gSortLightMode`, tier B: attribute bits 3-4 (GsFOG|GsMATE).
- `D_8008E250` -> `gSortUseGlobalLightMode`, tier B: attribute bit 5
  (GsLLMOD); the object is depth-cued when it is set and GsLIGHT_MODE is
  non-zero.
- `D_8008A82C` -> `gTexturedFaceColor`, tier A: `.byte 0x80,0x80,0x80`,
  copied into the context once per object and loaded as the GTE colour of
  every lit textured face (the POLY_FT3 and POLY_FT4 cases). Only reader.
- Sony's `GsSortObject4` (disassembled from `objt2.o`) stores the same four
  attribute fields as this function -- `(a >> 3) & 3` GsLMODE, `(a >> 5) & 1`
  GsLIGNR, `(a >> 6) & 1` GsLIOFF, `(a >> 9) & 7` GsNDIV. No linked Sony
  object references those four names, so the game-style names above are
  used; whether the addresses ARE Sony's commons (3.3+ `libgs/global.o`
  defines all four) is left to the head (see the proposals in the round-91
  summary). `D_8008E248` (GsLOFF) and `D_80090C18` (GsDIV) cannot be renamed
  by `rename.py`: it reports them inside Sony's `PSDOFSY` and `dc_cb`, whose
  pinned sizes (8) are distance-to-next-pin estimates -- `PSDOFSY` is 4 bytes
  in `libgs/gs_010.o`.

### Types and constants

- Arguments are Sony's `GsDOBJ2 *` and `GsOT *`; the TMD object is
  `struct TMD_STRUCT` (libgs.h) through `OBJ_TMD()`; the rotation block reads
  `obj->coord2->super->workm` and the three columns of `obj->coord2->workm`.
- Every scratchpad offset is a field of the unit-local `PolyDrawCtx`
  (`+0x88`/`+0x94` are `RVECTOR *` into the DIVPOLYGON3/4 tables, `+0xA4`
  `SVECTOR *[4]`, `+0x78` the subdivide flag, agreeing with code_8220_c).
- Each case names its packet (`TMD_P_F3` ... `TMD_P_TNG4`) through a per-case
  `PKT` macro, `(type *)(elem - offsetof(type, member))`, because `elem` has
  to stay parked on the member retail parks it on; list-relative colour reads
  cast `packet`. The primitive is `POLY` (`POLY_F3` ... `POLY_GT4`). None of
  this moved a byte, including the struct-ness of every new field access.
- libgpu's `setPolyF3` ... `setPolyGT4` replace the `prim[3]`/`prim[7]` pairs
  and `setcode` the cached-code store; all compile to the same `sb`s.
- Case labels are `TMD_TYPE(GPU_COM_*, flag)` with libgs's `GPU_COM_*` and
  `GsTMDFlagGRD`, plus unit-local `TMD_FLAG_LGT`, `TMD_TYPE_MASK` (0xFD07),
  `TMD_WORD_ABE_SHIFT` (25), `DP_CLUT_SHIFT_CUED`/`_NONE` (9/16) and
  `GsDOFF` for the early-out (`bltz` unchanged).
- The old case comments called the lit cases "opaque"; they are the lit
  (ncds from the face normal) packets, the others unlit and depth-cued.
  Semi-transparency is the separate ABE bit.

### Moved out of the source

The function comment's derivation of the goto loops, the alias-rule
explanation of the four attribute reads, the do/while(0) loop-depth note,
the binary-search switch layout and the "submit wrappers declared void"
history (round 50) are all in the sections above; the source keeps one
MATCHING line for each.
