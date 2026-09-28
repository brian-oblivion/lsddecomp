# CdDriver__RunRequestQueue -- MATCHED (round 67, bravo): length exact (0x25C/151 words), 151/151 words match, no diff

> Renamed from `Class6D4E8__RunRequestQueue` on 2026-09-26 (tools/rename.py). Address 0x80027a24.

> Renamed from `func_80027A24` on 2026-09-25 (tools/rename.py). Address 0x80027a24.

REVISITED, round 67: MATCHED 151/151, whole-image SHA1 green; names/types not relevant
(the round-47 structs and locals were already correct -- the gap was one statement's
PLACEMENT, and the round-47 report's stated CAUSE was wrong).

Round 47 (alpha) left this at 150/151 with a single wrong branch target, attributed to
a whole-translation-unit GCC cross-jump artifact "not reachable by rewriting this
function's own source". That attribution is refuted below. The fix is one statement
moved out of five `case` bodies to after the `switch`.

## Revisit measurements, taken BEFORE changing anything

Round 47's preserved body was rebuilt verbatim (its `slot48` is the in-tree
`onError`, the same 0x48 slot; no other edit) and measured:

```
build exit=2
 17 off=0x018268 vram=0x80027A68 DIFF retail=35004010 built=13004010
CdDriver__RunRequestQueue: 150/151 words match (file 0x18224-0x18480)
CdDriver__RunRequestQueue: insertions 0 / deletions 0 (opcode-level; positional skeleton diffs 0)
WARNING: the build differs OUTSIDE this range too (2 bytes)
```

So: **insertions 0 / deletions 0, positional skeleton diffs 0.** Round 47's reproduction
is exact.

**The `len-off` tag was wrong and the round-47 title was right.** Measured three ways:
the splat header says `nonmatching CdDriver__RunRequestQueue, 0x25C` (= 151 words); the rebuilt body
assembled to 151 words; and `cmp -l build/SLPS_015.56 disk/SLPS_015.56` returned exactly
**three bytes** across the whole image, so there was no size change and no address drift
anywhere. `len-off` is an artifact of `plan.py`'s `EXACT_RE`
(`length[: ]*EXACT|exact length|\b(\d+)/\1\b`), which is matched against the TITLE ONLY:
the round-47 title said "length matches (0x25C/151 words)", which is none of those three
spellings -- "matches" is not "exact", and `0x25C/151` is not an `N/N` pair because the
character before the slash is `C`. This title is spelled `length exact` so the tag is
right from here on. See *Proposed learning* 3.

Those three bytes are worth writing down, because they are the whole residue:

| file off | vram | retail | built | what |
| --- | --- | --- | --- | --- |
| 0x18268 | 0x80027A68 | `35004010` | `13004010` | `beqz $v0, .L80027B40` vs `beqz $v0, 0x80027AB8` |
| 0x1020 (2 bytes) | 0x80010820 | `407B0280` | `B87A0280` | `jtbl_80010810[4]` (switch value 6) -- same two targets |

Both encode ONE fact: where the `default:` label sits. Retail puts it at 0x80027B40
(case 7's fall-through tail); round 47's build put it at 0x80027AB8 (case 2's tail), a
byte-identical `j .L80027C60` + `sh $zero, 0x28($s0)` pair 0x88 earlier.

## Did the round-47 cause survive the re-read? NO.

Round 47's load-bearing claim was Gate 3 check 3 run through the permuter: a scaffold
built from the same body reported **base score 0**, and its `base.o` was said to
disassemble with the CORRECT branch target -- from which the report concluded the residue
was a whole-file artifact, that "a permuter search here would search a program that
already matches", and that nothing in this function's own source controls it.

I compiled **that exact body, alone**, through the pinned pipeline -- `cpp | cc1 |
maspsx | as`, maspsx flags `sed`-ed out of the Makefile per CLAUDE.md, plus `-Iinclude`
on `as` for `labels.inc` -- with only the struct/extern declarations it needs:

```
  44:	10400013 	beqz	v0,94 <CdDriver__RunRequestQueue+0x94>
```

151 words, and `beqz` to +0x94 = 0x80027AB8: **the isolated compile reproduces the wrong
target byte-for-byte.** The residue is entirely local to this function's source, there is
no whole-translation-unit dependence, and neighbouring functions never had anything to do
with it. (`0x94` is case 2's tail; retail wants `+0x11C` = 0x80027B40.)

That isolation also turned the loop from a ~1s whole-image build into a ~1s single-file
build with the answer visible as one `objdump` line, which is how the lever was found on
the second variant.

## The lever

**Retail wrote ONE `self->unk28 = 0;` AFTER the `switch`, with a plain `break` in every
case. Round 47 wrote it inside all five cases plus an explicit `case 6: default:`.**

Retail's asm has four identical `j .L80027C60` + `sh $zero, 0x28($s0)` tails (at
0x80027AB8, 0x80027AD8, 0x80027B14, 0x80027B40), which reads like four source stores. It
is not. There is one store, and the other three are `dbr_schedule` filling delay slots
*from the target block*: `j after_switch` + `nop`, where `after_switch:` is
`sh $zero,0x28($s0)` followed by `j exit`, becomes `j exit` + `sh $zero,0x28($s0)` --
the target's first insn is copied into the delay slot and the jump is redirected past it.
Case 7 reaches the same block by falling through, and `case 6` / out-of-range reach it as
the switch's end, which is why `default:` is that block.

Writing the store literally five times is what broke it: with five real store blocks in
the RTL, GCC 2.6.3 cross-jumps the `default:` copy onto case 2's identical tail and moves
the label 0x88 earlier. The block at 0x80027B40 survives regardless (case 7 falls into
it), so **nothing is deleted and nothing is inserted** -- which is exactly why this
presented as `insertions 0 / deletions 0`, skeleton diffs 0, length exact, and three
stray bytes.

Diff against round 47's body, in full:

```diff
             case 2:
                 self->methods->slot44(self, GetCdFileEntry(node->unk10),
                                       node->unk14, node->unk18);
-                self->unk28 = 0;
                 break;
             case 3:
-                self->methods->slot48(self);
-                self->unk28 = 0;
+                self->methods->onError(self);
                 break;
             case 4:
                 self->methods->slot4C(self, node->unk14, node->unk18);
-                self->unk28 = 0;
                 break;
             case 5:
                 self->methods->slot54(self, node->unk14, node->unk18);
-                self->unk28 = 0;
                 break;
             case 7:
                 self->methods->slot58(self, GetCdFileEntry(node->unk10));
-                /* fallthrough */
-            case 6:
-            default:
-                self->unk28 = 0;
                 break;
             }
+            self->unk28 = 0;
```

Round 47 tried four switch-restructurings ("explicit `case 6:`, a non-fallthrough
`case 7` with its own literal `self->unk28 = 0; break;`, and reordering") and reports the
merge target unchanged in all four. Every one of those keeps the per-case stores; the
variant that removes them was not among them.

## The two round-47 levers that DID survive, re-measured on the new shape

Both were re-tested against the matched body, because a shape change can retire a lever:

| round-47 lever | variant tried now | result |
| --- | --- | --- |
| #4: `self->unk24 = *(volatile s32 *)&self->unk24 \| N;` for the `\|= 2` / `\|= 4` pair | plain `self->unk24 \|= 2;` / `\|= 4;` | **148 words, 3 short** -- still load-bearing, keep it |
| #1: read `node->unk8` once into a local `code` shared by both switches | `switch (node->unk8)` inlined at both sites, local dropped | **154 words, 3 long**, and the bounds check no longer even lands at +0x44 -- still load-bearing |

Round 47's levers #2 and #3 are descriptions of residues that #4 resolved, not separate
source constructs, and are unaffected.

## Final

```
build exit=0
CdDriver__RunRequestQueue: 151/151 words match (file 0x18224-0x18480)
CdDriver__RunRequestQueue: insertions 0 / deletions 0 (positional skeleton diffs 0)
OK: build matches retail SLPS_015.56
```

No permuter search was run and none was needed. `Obj80027480`, `Methods80027480`,
`Node8008A894`, `sCdIdle`, `GetCdFileEntry` and `FreeCdRequestNode` are unchanged from
round 47; no struct was edited, so the shared-struct oracle re-run is the same green
whole-image build above.

## Proposed learning

1. **N identical 2-instruction tails ending in `j <exit>` with a store in the delay slot
   are usually ONE source statement after the block, not N statements inside it.**
   `dbr_schedule` fills a `j L` + `nop` delay slot by COPYING `L`'s first instruction and
   redirecting the jump past it, so one post-`switch` (or post-`if`) statement is
   replicated into every `break` path and looks exactly like a hand-written copy in each
   case. Discriminator: write the statement once after the block and once inside every
   case, and compare lengths -- they are often EQUAL, so length will not tell you which
   is right. What tells you is where the `default:` label lands: with the copies real,
   GCC 2.6.3 cross-jumps `default:`'s copy onto the first identical tail, moving the
   switch's bounds-check branch target and the jump-table slot for the unhandled in-range
   value, and nothing else. Round 67, `CdDriver__RunRequestQueue`.

2. **A "both directions" reading of Gate 3 check 3 is only as good as the scaffold, and a
   direct pinned-pipeline isolation is cheaper and more trustworthy than a permuter base
   score.** Round 47's proposed learning was that a scaffold scoring base 0 while the real
   build has a residue is itself the finding -- "the gap is a whole-file artifact, and the
   fix is not in this function's own text" -- and it declined a search on that basis. The
   isolated compile above refutes the premise for this function: the same body compiled
   alone reproduces the residue exactly. Whatever the scaffold measured, it was not
   "this body through the pinned pipeline". Before drawing any conclusion from a
   scaffold/real disagreement -- in EITHER direction -- reproduce it with the escalation
   recipe already in CLAUDE.md ("Escalate, do not experiment"), which is ~1s and puts the
   instruction in front of you instead of a score. A scaffold score that disagrees with a
   direct isolation is a fact about the scaffold, not about the compiler. Round 67.

3. **`plan.py`'s `len-exact`/`len-off` tag reads the report TITLE with a fixed regex, so a
   title that states the length correctly in other words is tagged `len-off`.** Round 47's
   "length matches (0x25C/151 words)" is accurate and still scored `len-off`, because
   `EXACT_RE` wants the literal `length exact` / `exact length` / an `N/N` pair, and
   `0x25C/151` is not an `N/N` pair (the character before the slash is `C`). The tag ranks
   revisit jobs, so a mis-tag costs ranking, not correctness. Cheapest fix is a convention,
   not a code change: **spell it `length exact (0x25C/151 words)` in stall titles**, which
   the regex does match. Flagged rather than fixed here because `tools/plan.py` is a shared
   file and this is a runner branch. Round 67.

## Naming

Round 79 (charlie), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_80027A24` | `CdDriver__RunRequestQueue` | A |

**Evidence.** Called every service tick through the class table by
`ServiceCdDriver` (CdDriver), whose local view already names the slot
`runRequestQueue`. It takes the head of `gCdRequestQueue`: if the node is
not yet active it raises `owner->inQueueDispatch` and calls the owner's
slot for `node->op` (open/close/seek/read/loadFile, the CD_OP_* values the
five methods above enqueue); once `sCdIdle` says the drive finished, it
decrements `owner->pendingRequests`, ORs `CD_FLAG_DONE`, `CD_FLAG_NONE_PENDING`
(when the count hits 0) and the op's own completion bit into `owner->flags`,
calls `setFlag`, frees the node, and calls `stopCdService` when the queue
is empty. "Run" rather than "drain": one call advances only the head node.
It takes no `self`; the object it works on is the node's owner.

**Class prefix.** `Class6D4E8` is the placeholder token for the method
table `gCdDriverMethods` (the convention `CdDriver__RequestLoadFile` and its two
siblings already use); `tools/classtable.py gCdDriverMethods` lists this function
at slot `+0x068`. The prefix names the table, not the developers' class.

## Proposed field names

For the head to apply by type scope (out of unit):

| unit | type | field | proposed | tier | evidence |
| --- | --- | --- | --- | --- | --- |
| CdDriver | `Obj6D4E8` | `unk28` | `inQueueDispatch` | A | the constructor's clear of the same field this function raises around each dispatch (note that view types it `s16`, this unit `u16`) |
| include/GameApplicationFileResource.h | `FileResource` | `unk28` | `inQueueDispatch` | B | the base ctor zeroes it; only this derived class reads it |
| include/GameApplicationFileResource.h | `FileResource` | `unk22` | `pendingRequests` | B | the base ctor zeroes it; EnqueueCdRequest/this function count it |


Track 4, 2026-09-26 (round 88). The class of gCdDriverMethods (was D_8006D4E8, id 0x13 = DATASOURCE_CD) is CdDriver, in include/CdDriver.h: its ctor calls InitCdDrive, its slots enqueue CD_OP_* requests and drive the CD read state machine, and it is NullDriver's sibling. The object views this function was typed against are replaced by CdDriver, whose fields are all FileResource's (the driver runs on its clients' objects; FileResource's +0x018/+0x01C were named pos/size for it). Byte-identical. `Class6D4E8__RunRequestQueue` -> `CdDriver__RunRequestQueue` by rename.py. The +0x070 call passes `self` through the StopServiceSelfFn cast (FileResource's stopService slot takes no arguments; retail loads $a0): without it 150/151 words.

## History (moved from code_179d8_s.c, round 100)

The unit banner ended: "RunRequestQueue's two switches own this unit's only
jump tables, jtbl_80010810/jtbl_80010828 (carved round 47)." The
`volatile` re-reads (the table above, lever #4) now carry a `MATCHING:` line
in the source. The CD_FLAG_* bits it ORs moved from code_179d8_s.c to
include/CdDriver.h in round 100, beside the clients that poll them.
