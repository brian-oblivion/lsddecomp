# StageMap__SetFootprintRect — MATCHED round 71 (charlie): 52/52, byte-exact, whole-image SHA1 green

> Renamed from `Class866E8__SetFootprintRect` on 2026-09-26 (tools/rename.py). Address 0x8004b030.

REVISITED, round 71: MATCHED (4 builds); names/types used (parameters renamed
`desc`/`span`; the fix was local-variable SHAPE, not a type change).

## Round 71 (charlie) — the match

**Rebuilt as given first.** The round-47 preserved body (the `#if 0` block
that sat in `src/class_39e08.c`) rebuilt at **22/52, insertions 5 /
deletions 5, 18 positional skeleton diffs** — the recorded score was
current, but the recorded CAUSE ("the oversized frame") was a symptom: the
old body carried four `s16` locals, two `s8` locals and a dead trailing
`col = b2;`, and its frame (0x38) was oversized because of those, not
because of anything the permuter could reach.

**What retail actually says**, read straight from the asm:

- `lb s2,2(a1)` / `lb s3,3(a1)` then `move v1,s2` / `move a0,s3`: each byte
  is loaded once into the variable that gets decremented AND copied into a
  temp that the `== 0x13` edge test reads. So two locals per byte: `col` and
  `origCol` (build 3 fixed the length: re-reading `desc->unk2` for the edge
  test cost a reload).
- `bnez s2, dec; j; addiu s4,s1,-1`: the ZERO case is the then-arm, so the
  source is `if (col == 0) width = span - 1; else col--;` (build 4 fixed the
  branch sense and the register assignment in one go).
- `addiu s1,s1,-1` twice for the height: the height companion is the `span`
  PARAMETER itself (`span--`), not a separate local initialised from it.
  `width` is the separate copy (`move s4,s1`).
- The second byte's zero test reads the COPY (`bnez a0`), the first byte's
  reads the variable (`bnez s2`) — asymmetric, measured: testing `row`
  instead of `origRow` gave 51/52 with the single diff at word 22.

Build history: 22/52 (as given) -> 11/52 length-off (reload) -> 4/4
ins/del (copies added) -> 51/52 (branch sense + span as height) -> 52/52.

The matched C is the live definition in `src/class_39e08.c`; not repeated
here.

### Proposed learning

A stall whose first diff is "the frame is too big" is usually telling you
the body has too many locals, not that the permuter should search harder.
Count retail's saved registers against the values that are live across the
call before anything else; here 5 saved regs = self, col, row, width, span,
and the old body had seven named locals competing for them.

---

## Historical record (rounds up to 47; superseded by the match above)

### (old title) StageMap__SetFootprintRect — STALL: length EXACT 52/52; 22/52 raw word-match (round 47, up from 19/52); first real diff at word 5 (vram 0x8004B044, the oversized frame)

> Renamed from `func_8004B030` on 2026-09-22 (tools/rename.py). Address 0x8004b030.

**Unit:** class_3ac78 · **Size:** 52 instructions · **Best reached:** 22/52
words (correct size, no address drift)

## What it does

Reads two signed bytes off the shared `UnkArgObj_3ac78` descriptor
(`arg1->unk2`, `arg1->unk3` — same buffer type as `StageMap__SetFootprintFromCell`/
`StageMap__SetFootprintRect`/`StageMap__ApplyToSenderFootprint` per the header's existing comment).
For EACH byte independently: if nonzero, store `byte - 1` into one field
and leave the `count`-derived companion field alone; if zero, store the
byte as-is (0) and additionally decrement the companion `count`-derived
field. THEN, independently of that branch, if the byte's ORIGINAL value
was exactly `0x13` (19), decrement the companion field AGAIN. Both byte
groups follow this identical shape, just against different field pairs
(`unk90`/`unk94` for `arg1->unk2`, `unk92`/`unk96` for `arg1->unk3`).
Finally: `self->unk88 = 1;` (always, unconditionally — the register that
computes it, `$v1`, is reused from an earlier unrelated comparison and
just happens to always end at `1` by the time it's stored); calls
`self->methods->slot124(self, arg1->unk28)` and stores the return into
`self->unk8C`; stores the four derived `s16` values into `unk90/92/94/96`.

New struct knowledge added regardless of the function itself stalling
(verified straight from the disassembly, independent of getting a byte
match): `StageMapMethods::slot124` (`StageMap__FindSlotIndexByChunk`, not decompiled),
`UnkArgObj_3ac78::unk28` (a `void *`, reread and forwarded to `slot124`),
and `StageMap`'s `unk88` (`s32`), `unk8C` (`void *`), `unk90`/`unk92`/
`unk94`/`unk96` (`s16` each).

**Update (corrected by `StageMap__ApplyToSenderFootprint`, matched later the same round):**
`self->unk8C` is NOT a standalone 4-byte pointer field. `StageMap__ApplyToSenderFootprint`
copies the WHOLE region `self+0x8C..self+0xBC` (0x30 bytes) with retail's
batched 4-word-per-iteration block-move codegen on both a save and a
restore — the "whole-struct assignment, not an indexed loop" signature
already established by `Pad__LoadButtonTable`. The header now models this as
`HistoryBlock_3ac78 unk8C` (3 x 0x10-byte `HistoryEntry_3ac78` elements).
This function's own write (`self->unk8C = self->methods->slot124(...)`)
is still consistent with the new layout — it's writing element `[0]`'s
first field (`unk8C.e[0].unk0`) — so the best-reached body below is not
wrong, just stated in terms of field names (`unk8C` as a bare pointer,
`unk90`/`unk92`/`unk94`/`unk96` as top-level `StageMap` fields) that no
longer exist in the header. Relative to element `[0]`'s base (`self+0x8C`),
the four `s16` stores land at `+4`/`+6`/`+8`/`+0xA` — exactly
`HistoryEntry_3ac78`'s `unk4`/`unk6`/`unk8`/`unkA` fields, in that same
order (`unk90`->`unk8C.e[0].unk4`, `unk92`->`unk8C.e[0].unk6`,
`unk94`->`unk8C.e[0].unk8`, `unk96`->`unk8C.e[0].unkA`). A future attempt
should rewrite the best-reached body's four stores through
`self->unk8C.e[0]` accordingly.
`self->unk88`, `StageMapMethods::slot124`, and `UnkArgObj_3ac78::unk28`
are unaffected by this correction and still stand as originally reported.

## Best-reached body (does NOT compile to retail bytes)

```c
#if 0
void StageMap__SetFootprintRect(StageMap *self, UnkArgObj_3ac78 *arg1, s32 count)
{
    s8 b2;
    s8 b3;
    s32 raw3;
    s16 v90;
    s16 v92;
    s16 v94;
    s16 v96;

    b2 = arg1->unk2;
    b3 = arg1->unk3;
    raw3 = b3;
    v94 = count;

    if (b2 != 0) {
        v90 = b2 - 1;
    } else {
        v90 = b2;
        v94 = count - 1;
    }
    if (b2 == 0x13) {
        v94 -= 1;
    }

    v96 = count;
    if (raw3 != 0) {
        v92 = raw3 - 1;
    } else {
        v92 = b3;
        v96 = count - 1;
    }
    if (raw3 == 0x13) {
        v96 -= 1;
    }

    self->unk88 = 1;
    self->unk8C = self->methods->slot124(self, arg1->unk28);
    self->unk90 = v90;
    self->unk92 = v92;
    self->unk94 = v94;
    self->unk96 = v96;
}
#endif
```

## The residue: an asymmetric compiler choice between two structurally
identical code blocks

Retail loads BOTH `arg1->unk2` and `arg1->unk3` with a plain `lb`
(single-instruction signed byte load) and never re-derives the sign
extension — each byte's raw (`$v1` for byte 2, `$a0` for byte 3) and
decremented (`$s2`/`$s3`) values live in two SEPARATE registers for the
whole function, set once, read as needed, no rework.

A straightforward translation, using ONE local per byte for the "raw"
comparisons (`b2`, `b3`, structurally identical to each other — each read
once, never reassigned, and used in exactly two comparisons) reproduces
this EXACTLY for the first byte (`b2`: single `lb`, reused cleanly) but
NOT for the second (`b3`: loads as `lbu` — the UNSIGNED byte — and
re-derives the sign extension with an explicit `sll`/`sra` pair at EACH of
its two use sites, 4 extra instructions total). Both blocks are
byte-for-byte identical in C source shape (only the field names and
literal differ); the compiler treats the first and second occurrences of
this pattern differently.

**Attempts and results, in order:**

1. Symmetric `b2`/`b3`, no extra locals: 12/20 → this was actually run on
   `StageMap__SetFootprintFromCell` earlier in the round for a similar-shaped residue, not
   this function — disregard, listed for pattern-recognition only.
2. Symmetric `b2`/`b3`, no extra locals, on THIS function: redundant
   `lbu`+`sll`+`sra` on `b3` only, function grows to 56 words (4 too many),
   whole-image drift warning.
3. Reorder `self->unk88 = 1;` to just before the call instead of between
   the two `0x13` checks (matching where the STORE actually lands in
   retail, as opposed to where the register load `$v1=1` sits): no change
   to the `b3` residue.
4. Add `s32 raw3 = b3;` and use `raw3` for BOTH of `b3`'s comparisons,
   keeping `b3` only for the one `else`-branch store: fixes the redundant
   extension (`b3` now also gets a single clean load) but grows the frame
   by 16 bytes (`-0x38` instead of retail's `-0x28`) — 19/52, no drift,
   this round's best score.
5. Same, but replace `raw3 = b3;` with `raw3 = arg1->unk3;` directly (drop
   `b3` as an intermediate) and use `raw3` everywhere including the
   `else`-branch: regresses hard, back to a 4-word-oversized function and
   drift.
6. Add a matching `s32 raw2 = b2;` for symmetry (now BOTH bytes have a
   `bN`/`rawN` pair): regresses further — frame grows even more (both
   extra locals get charged), 4/52, drift.
7. Insert a bare `__asm__("");` scheduling barrier between reading `b2`
   and `b3` (attempt 4's variant): changes prologue register-SAVE ORDER
   (matches CLAUDE.md's documented "barrier perturbs the whole function's
   allocation" warning) but does not fix the `b3` vs `b2` asymmetry or the
   frame size; net no improvement over attempt 4.

**None of the four/16-byte-oversized attempts (2, 5, 6) were address-drift
safe** — each was confirmed via the `WARNING: the build differs OUTSIDE
this range too` guard before being discarded; none of their word-match
counts should be read as partial credit.

### What's actually going on (best guess, unconfirmed)

Two structurally-identical source blocks receiving different codegen from
the SAME compiler pass is the kind of thing CLAUDE.md's residue list
attributes to `__asm__("")` barrier scope or declaration-order effects,
but neither lever closed it here. The strongest working hypothesis: retail
genuinely has FOUR independent byte-sized locals (not two byte locals
sharing their "final" value with two `s16` result locals) — i.e. the real
source keeps `unk2`'s raw AND final value in visibly separate declarations
for BOTH fields symmetrically, but ALSO something else keeps the frame at
exactly `-0x28` (40 bytes: 5 saved regs + 16-byte arg-build area, no
padding) that a naive 4-raw-local reconstruction (attempt 6) does not
reproduce. This function needs either a real permuter pass (see
`docs/PARALLEL-RUNS.md` Gate 3) or a fresh derivation from someone who
spots the missing shape; it is NOT a toolchain blocker (no `gp_rel`/
`addiu_at` hits) and NOT the epilogue-merge class (single exit, no early
return).

### Proposed learning

**Two textually-identical `if`/`if` blocks against sibling fields do not
necessarily compile identically, even with matching source shape** — GCC
2.6.3 gave the FIRST such block a clean single-`lb` register lifetime and
the SECOND a raw-byte-plus-repeated-sign-extend lifetime, with no
observable difference in the C driving them. Padding the second block's
local width back to `s32` fixes the extra instructions but changes the
frame size, so the two symptoms (extra sign-extend instructions vs. extra
frame bytes) trade off against each other rather than being independently
fixable — suggesting the real fix is a genuinely different STRUCTURAL
shape for this pattern, not a per-variable width or barrier tweak. Worth
testing against future "repeated field-extraction pattern, second instance
differs from first" residues before re-deriving from scratch.

## Provenance

round 2026-09-02 (head-requested extension, second pass after the initial
6-function batch merged to main), runner ALPHA, unit class_39e08. Seven
attempts, hard stop not reached but diminishing returns — moved on to stay
within budget for the remaining assigned functions. Restored to
`INCLUDE_ASM`.

## Update (corrected AGAIN by StageMap__DispatchToRectCells, same round, runner delta)

`HistoryEntry_3ac78` (and the field this function writes,
`unk8C.e[0].unk0`) moved again. The 0x10-byte/3-element model this
report's own "Update" section fixed to is now ALSO superseded: it is
0xC bytes/4-elements, and the field this function writes is renamed
`elemIdx` and RETYPED `void *` -> `s32` (an index into `self->unkEC[]`,
not a pointer — confirmed by two independent readers, `StageMap__FindSlotIndexByChunk`'s
own retyped return and this function's own read-back in `StageMap__DispatchToRectCells`).
See `docs/match-reports/StageMap__DispatchToRectCells.md` for the full evidence. This
function's own write, `self->unk8C = self->methods->slot124(...)`
(`self->methods->slot124` already returns `s32`), is if anything a
BETTER fit for the corrected type than the old `void *` was — no `#if 0`
body change needed, just note that `unk8C.e[0].unk0` in the preserved
body above now reads `unk8C.e[0].elemIdx` in the current header, same
memory, same value, s32 not pointer.

## Round 19 (echo): re-verified, field names updated to current header,
## one more axis tried (negative)

Re-verified the 19/52 claim by rewriting the preserved body using the
CURRENT `HistoryEntry_3ac78` field names (`elemIdx`/`col`/`row`/`width`/
`height`, per the corrections layered on by `StageMap__DispatchToRectCells` and
`StageMap__ApplyToSenderFootprint` since this report's original pass) and dropping it in:
confirmed **19/52, correct size, no address drift** -- matches exactly,
no contamination from either struct-shape correction.

Studied the disassembly closely to understand exactly WHY `b3`'s raw
value needs a callee-saved register while `b2`'s does not: retail
reuses `$a0` (the `self` parameter register, freed the instant `self` is
copied to `$s0`) to hold `b3`'s raw value for the rest of the function,
never allocating it a NEW callee-saved slot -- `b2`'s raw value instead
goes into `$v1` (an ordinary scratch register, later reused for the
literal `1`). This is asymmetric by REGISTER ROLE (a freed argument
register vs. a scratch temp), not by anything visibly different in the
C source between the two byte-handling blocks.

Tried reordering the two raw-value reads/assignments (`raw3 = arg1->
unk3; b3 = raw3;` computed and assigned BEFORE `b2`'s own read, instead
of after) on the theory that request ORDER might determine which free
register (a0 vs. a fresh scratch) each value lands in. Result: **still
19/52 in raw word count, but the FRAME GREW by 8 bytes** (`-0x30` instead
of retail's `-0x28`, confirmed via the diff's own word 0) -- i.e. an
apparent tie in score that is actually a regression once the frame-size
signal is read, not a genuine improvement. Reverted immediately.

This is now 8 real attempts (7 from the original pass + this round's
reordering try) without closing either the frame-size/raw-value tradeoff
or the `b2`/`b3` asymmetry. Filing unchanged as STALL at 19/52,
`INCLUDE_ASM` restored; the preserved body's field names updated to the
current header (`col`/`row`/`width`/`height` via `self->unk8C.e[0]`,
matching `HistoryEntry_3ac78`'s now-settled shape) so a future attempt
does not have to re-translate old field names first.

### Proposed learning

**A tied word-count across two attempts is not evidence of a tied
result -- always check whether the FRAME SIZE (word 0, the `addiu
$sp,$sp,-N` prologue instruction) matches before treating two same-score
attempts as equally good.** Here, reordering the raw-value reads kept
the raw in-range word-match count at 19/52 but grew the frame by 8
bytes, which is strictly worse (address drift outside this specific
comparison would occur in the real oracle, since a differently-sized
function shifts everything after it) even though `funcdiff.py`'s
top-line number looked unchanged. This is a variant of the project's
existing "read the WARNING line, not just the score" caution, narrowed
to the specific case where the score itself doesn't change but the
frame does.

## ROUND 20 (runner echo): lever not applicable; one fresh structural idea tried, negative

**The `GetRCnt` two-independently-live-locals lever does not apply
to this function at all** -- there is no table/struct-base address
computation anywhere in its body (it is pure byte-field reads, sign
extension, and conditional decrements; the only pointer dereference is
`arg1->unk2`/`arg1->unk3`/`arg1->unk28`, none of which involve a
base-plus-runtime-index pattern). Not tested, since there is nothing to
apply it to; recorded as a scope finding rather than a silent skip.

**One fresh idea tried instead, re-deriving from the raw disassembly
rather than resuming the preserved body:** retail's `s2`/`s3` registers
hold `b2`/`b3` for the ENTIRE function and are decremented IN PLACE
(`addiu s2,s2,-1`) inside the nonzero branch, then stored DIRECTLY to
`unk90`/`unk92` at the end -- there is no separate "result" register at
all, unlike this report's preserved body (which keeps `b2`/`b3` and a
separate `col`/`row` local, assigning `col = b2 - 1` rather than mutating
`b2` itself). Retail also uses a genuinely SYMMETRIC raw-copy shape for
BOTH bytes (`v1` = raw `b2` copy, `a0` = raw `b3` copy, same register
ROLE for both, just different hardware registers) -- not the asymmetric
"only `b3` needs a raw copy" shape the existing best body uses.

Tried both changes together: `b2`/`b3` mutated in place (no separate
`col`/`row` locals, direct `self->unk8C.e[0].col = b2;` at the end) plus
a symmetric `rawB2`/`rawB3` pair (was `raw3`-only asymmetric). **Result:
5/52 words, with a genuine 148890-byte outside-range drift** -- a real
size regression, not a neutral rephrasing; this shape compiles
noticeably LARGER than retail. Reverted immediately; `git diff --stat`
confirmed clean.

This substantially reproduces the report's own already-documented
attempt 6 (symmetric raw locals regress hard, 4/52 with drift) under a
different variable-naming scheme, rather than finding new ground -- the
in-place-mutation half of the idea did not rescue the symmetric-raw-copy
half. Filed as a confirming re-derivation, not a new lever: this
function's residue (frame-size/register-role tradeoff between `b2` and
`b3`) remains open, `INCLUDE_ASM` restored, still 19/52.

## ROUND 47 (runner echo): first permuter search ever run on this function -- real, oracle-confirmed +3 words (19/52 -> 22/52), no zero

This function was flagged this round as "never permuter-searched," the
cheapest search ground in the queue. Ran all three of this round's
permuter pre-checks before searching, in order:

**(a) scaffold compiles and scores.** Built via `tools/setup-permuter.sh
StageMap__SetFootprintRect <seed = round-19/20's preserved attempt-4 body,
field names updated to the current `col`/`row`/`width`/`height`/`elemIdx`
header>`. Base compiles; `--debug --stack-diffs` gives base score 1512
(via the standalone `--debug` invocation) / 1330 (via the actual search
harness -- the two invocations use slightly different internal scoring
paths, both consistent within themselves).

**(b) insertion/deletion penalties near 0/0 -- NOT satisfied.** The debug
penalty list: `Insertions: 6 (100)`, `Deletions: 6 (100)`, `Register
Differences: 28 (5)`, `Stack Differences: 172 (1)`. This is FAR from
0/0 -- on its own, this would usually mean decline the search (see
`NoteOn`'s round-47 entry for the contrasting clean case).

**(c) does the scaffold's signature agree with the real build's own
residue -- checked instead of declining outright, and this is the
discriminator that justified searching anyway.** Dropped the exact same
seed body into `src/class_39e08.c` in place of the `INCLUDE_ASM`,
rebuilt through the real oracle, and compared: the in-tree build ALSO
scores 19/52 with the IDENTICAL 6 insertions/6 deletions (`asm-differ`
shows the same extra `move s4,zero` + `sll/sra` sign-extend pair
inserted, and the same `move s4,s1` deleted, as the scaffold's own
`--debug` diff). Scaffold and real build AGREE -- the harness is not
lying, the nonzero ins/del is a GENUINE property of this exact source
shape (the extra `raw3` local costs 16 stack bytes, frame grows from
`-0x28` to `-0x38`), not a scaffold artifact. This is different from a
MISMATCH (scaffold nonzero, real build zero) and different from the
clean AGREE case (both zero) -- it is "both agree, and both are
nonzero," which the round's guidance didn't name explicitly. Treated it
as still worth searching because the residue looked like a
declaration-shape/frame-layout question (which locals exist, in what
order) rather than a genuinely uncloseable control-flow gap (contrast
with `code_179d8_h`'s address-caching residues, which delta declined on
the same round for exactly that reason).

**Search: `timeout 600 ... permuter.py -j 8 --stop-on-zero --best-only`,
confirmed exit via the timeout (not an external kill).** ~120000
iterations in 600s. Read every `output-*/score.txt` this produced (6
directories, per this round's standing instruction): best local score
710 (down from base 1330), found twice (`output-710-1`, `output-710-2`);
four more at 810. No zero.

**The score-710 candidate is a real, oracle-verified lead, translated and
kept as this function's new best (22/52, up from 19/52):**

```c
b2 = arg1->unk2;
b3 = arg1->unk3;
raw3 = b3;
width = (height = count);        /* was: width = count; ... height = count; (two statements) */

if (b2 != 0) {
    col = b2 - 1;
} else {
    width = count - 1;           /* was: col = b2; width = count - 1; -- the
                                   * redundant `col = b2;` here is GONE */
}
if (b2 == 0x13) {
    width -= 1;
}

if (raw3 != 0) {
    row = raw3 - 1;
} else {
    row = b3;
    height = count - 1;
}
if (raw3 == 0x13) {
    height -= 1;
}

self->unk88 = 1;
self->unk8C.e[0].elemIdx = self->methods->slot124(self, arg1->unk28);
self->unk8C.e[0].col = col;
self->unk8C.e[0].row = row;
self->unk8C.e[0].width = width;
self->unk8C.e[0].height = height;
col = b2;                        /* NEW: a trailing DEAD store, after every
                                   * real use of both `col` and `b2` -- has
                                   * no observable effect, but forces `b2`'s
                                   * register to stay live one instruction
                                   * longer than it otherwise would */
```

Two changes, both kept (removing either one in isolation was not
separately tested this round -- time went to verifying the combination
through the real oracle instead):
1. Chaining `width = (height = count);` instead of two separate
   statements, and dropping the `col = b2;` in the `else` branch that the
   19/52 body had. `col` IS still read on that path (at the final
   `self->unk8C.e[0].col = col;` store), so this leaves `col`
   C89-uninitialized on the `b2 == 0` branch -- looks like a correctness
   regression at first glance. It is not, because this matches what
   retail's OWN disassembly does on that path: `s2` (the register
   holding `b2`) is never re-assigned when `b2 == 0`, so retail reads
   the same never-mutated `b2` value back out at the store. The C's
   "uninitialized `col`" and retail's "never-reassigned `s2`" name the
   same runtime value via different means -- safe here, but fragile
   enough (depends on `col` and `b2` being provably equal on this one
   path) that it is worth flagging for whoever next edits this body.
2. A trailing dead `col = b2;` statement after the function's real work
   is done, matching the permuter's own found shape exactly.

**Verified via the real oracle** (`./build-and-verify.sh` + `funcdiff.py`
+ `asm-differ`): 22/52, length still exact (52/52), frame still
oversized at `-0x38` (retail: `-0x28`) -- so the fundamental frame-size
gap this function has carried since round 2 is UNCHANGED, but 3 more
words now agree that did not before, with zero address drift outside the
function's own range. `asm-differ` score dropped from ~1330 (base) to
1107 for this candidate specifically.

**Still not byte-exact; `INCLUDE_ASM` restored,** this new 22/52 body is
what `src/class_39e08.c` now preserves in `#if 0`. The frame-size gap
(`raw3`'s extra stack slot) and the remaining `b3`-side sign-extend
residue are unchanged from every prior round's analysis -- this is an
incremental win on the SAME open residue, not a new mechanism.

### Proposed learning

A permuter scaffold whose ins/del penalty is nonzero is not automatically
a function to decline searching -- check whether the REAL BUILD's own
residue (from dropping the same seed in-tree) shows the IDENTICAL nonzero
signature. If it does, the harness is representative even though it
fails the "near 0/0" necessary condition, and a search can still surface
a genuine (if partial) improvement -- as happened here, +3 real words,
zero drift, first zero-free search this function has ever had.

## ROUND 48 (runner delta): re-verified the 22/52 claim, then a long permuter
## search — negative, and it surfaced a scorer-exploitation pattern worth
## generalising

**Check 3, run in full before searching.** Rebuilt the preserved round-47
body (verbatim, current header field names) in-tree in place of
`INCLUDE_ASM`: `build exit=2` (no compile-error grep hits — clean compile,
SHA1 mismatch as expected of any non-byte-exact function), `funcdiff.py`
reports **22/52 words, correct size (52/52), no address drift** — matches
the report exactly. Then built the permuter scaffold from the same body via
`tools/setup-permuter.sh` and ran `--debug --stack-diffs`: base score 912,
`Insertions: 3 (100)` / `Deletions: 3 (100)` — both DOWN from round 47's
6/6 (consistent with the 19->22 improvement), and the scaffold's printed
diff shows the identical residue as the real build (the same `s2`/`s3`/`s4`
register-role swap across the four `sh` stores, the same `-0x38` vs
retail's `-0x28` frame). **AGREE confirmed** — nonzero on both sides, same
signature — so the search is meaningful, matching round 47's own finding
for this exact function. `INCLUDE_ASM` restored before searching.

**Search 1: `timeout 1800 permuter.py -j 6 --stop-on-zero --best-only`,
seeded from the 22/52 body.** Ran the full bound — **193,644 iterations**,
confirmed by the last `iteration N` line printed before the process exited
and by wall-clock (script launched ~19:20:51, log's last write ~19:51:23,
~1830s including scaffold overhead). No `permuter rc=` marker survived in
the log to read the literal exit code from — see the note below, a real
process-management finding, not a search result — but the evidence is
unambiguous that this is a **timeout, not a `--stop-on-zero` early exit**:
no candidate ever scored 0 (`grep -c "score = 0"` is zero across the whole
11MB log) and the iteration counter kept climbing at a steady rate all the
way to the process's last line, which is exactly the shape a bound firing
produces and not the shape an early stop produces. Phrasing per this
round's standing rule: **not closed in 193,644 iterations under load
(rc=124, inferred — see below)**.

**Four local-best improvements found, permuter-score 710 -> 690 -> 660 ->
595 -> 495 (permuter's own units, not funcdiff words).** All four were
pulled and read as C, not just trusted as numbers — and this is the
important part of this round's result: **two of the four are not
translatable candidates at all, because the permuter's scorer never
executes the code, only diffs compiled bytes against `target.o`, so a
candidate that is flatly WRONG C can still score an "improvement" if the
wrongness happens to compile smaller.**

- **`output-495-1` (score 495, the "best"): a genuine uninitialized-read.**
  The mutation moves `row = b3;` out of the `if (raw3 != 0) {...} else {
  row = b3; height = count - 1; }` block and into the UNRELATED `if (b2 !=
  0) { row = b3; col = b2 - 1; }` block instead, then deletes it from the
  original site. Trace every one of the four `(b2, raw3)` sign
  combinations: the only place this changes behaviour is `b2 == 0 &&
  raw3 == 0` (both source bytes zero) — on that path NEITHER surviving
  block ever assigns `row`, and it is read moments later at
  `self->unk8C.e[0].row = row;`, fully uninitialized. This is exactly the
  class CLAUDE.md's permuter section already names ("reject anything that
  branches on a value read before its first assignment") extended by one
  step: here the uninitialized value isn't branched on, it's stored, but
  the mechanism — the scorer rewarding a mutation because GCC 2.6.3 left a
  register holding a leftover value that happens to match retail's actual
  bytes for this one compiled instantiation — is identical. Rejected.
- **`output-690-1` (score 690): not UB, but a plain logic bug the UB
  screen would MISS.** The mutation inserts `b3 = 0x13;` immediately before
  `if (b2 == b3)` (turning a literal comparison into a variable one) —
  which looks harmless because `b3`'s ORIGINAL value has already been
  copied into `raw3` by that point. But `b3` is read AGAIN, unmutated,
  several lines later at `row = b3;` inside the `raw3 == 0` branch. Since
  `raw3 == 0` implies the original `b3` was `0`, retail's real semantics
  need `row = 0` on that path; this candidate delivers `row = 0x13`
  instead. Not uninitialized-read UB — `b3` has a perfectly well-defined
  value, just the WRONG one — so a screen that only checks for
  read-before-first-assignment does not catch it. Caught here only by
  tracing every use of the mutated variable forward through the rest of
  the function, not by a syntactic check. Rejected.
- **`output-660-1` (score 660): a type change (`s8 b2` -> `unsigned int
  b2`), not incorrect but not adopted.** Well-defined by C89's conversion
  rules for every value, so not a bug — but it silently changes how a
  negative byte in `arg1->unk2` would compare against `0`/`0x13`, and nothing
  in this function's own evidence rules out that field holding a negative
  sentinel. No zero reached either way, so there was nothing to gain by
  resolving the question; noted rather than adopted.
- **`output-595-1` (score 595): the one candidate with NO correctness
  question at all.** Purely caches `b2 != 0` into a new `int new_var` and
  branches on that instead of the raw comparison — provably equivalent,
  no UB, no clobbered value. **Translated and run through the real oracle
  anyway, since a clean permuter improvement is exactly the case this
  round's instructions say to measure rather than trust:** regressed
  hard, **7/52 words with 187427 bytes of drift outside the function's own
  range.** Reverted immediately, `git diff --stat` confirmed clean before
  moving on.

**So all four of this search's improvements are dispositive negatives, and
together they make the project's "a permuter score is not a funcdiff proxy
in either direction" rule about as concrete as it can get**: the ONE
semantically clean candidate this search produced made the real score
dramatically WORSE, and the two candidates that scored BEST in permuter
units are outright incorrect C that the real toolchain would never be
asked to accept as a genuine translation. Worth carrying forward: **a
"reject UB" screen that only checks for reads-before-first-assignment is
too narrow** — `output-690-1` shows a live-but-wrong-value clobber that
produces the identical failure mode (scorer rewards a mutation for reasons
that have nothing to do with C-level correctness) without tripping that
specific check. The generalizable screen is "trace every read of every
variable the mutation touches to its next use," not "grep for use before
assignment."

**Process note, not a search result:** the `rc=$?; echo "permuter rc=$rc"
>> logfile` idiom used to capture the timeout's exit code lost a race with
Python's `multiprocessing.resource_tracker`, which stayed alive after the
main permuter process exited and wrote its leaked-semaphore warning to the
same (non-append) file descriptor it inherited, landing after the parent
shell's independently-opened append write and leaving no `rc=` line
recoverable from the log. Worked around on the SECOND search below by
writing a distinguishable `PERMUTER_RC_MARKER=` line plus an explicit
`sync`; whether that resolves the race or just narrows the window is
unconfirmed. **This is a logging-capture artifact, not a permuter-behaviour
finding** — the actual outcome (timeout fired, no zero, 193,644 iterations)
is independently nailed down by the iteration count and the absence of any
`score = 0` line, both content-level facts unaffected by which process won
the race to the log's tail.

Still not byte-exact; best reached remains **22/52** (round 47's figure,
re-confirmed, unchanged by this search). `INCLUDE_ASM` in place,
`src/class_39e08.c` untouched beyond the verification rebuilds (all
reverted, `git diff --stat` clean throughout).

### Proposed learning

**The permuter's scorer never executes a candidate — it only diffs
compiled bytes against retail — so it can reward a candidate that is
flatly semantically WRONG, and "reject UB" as currently scoped (reads
before first assignment) does not catch every such case.** A value that is
reassigned to something ELSE first, then read again later under its OLD
meaning, produces the identical failure mode (a byte-coincidental score
improvement on a broken candidate) without any uninitialized read at all.
Before adopting or even reporting a nonzero-but-improved permuter
candidate as a lead, trace every variable the mutation touches forward to
its next use in the function, not just check whether it introduces a
read-before-assignment. Separately: a candidate with NO correctness
question (`output-595-1` here) still regressed hard through the real
oracle (7/52, 187KB drift) — reinforcing that a permuter-unit score
improvement carries no directional information about the real funcdiff
score, even for provably-safe mutations, and every candidate this round's
instructions call a "lead" still needs the full real-oracle round trip
before it means anything.

**Search 2 (same round): a second independent 1800s bound, fresh RNG, same
seed.** Confirmed the same way as search 1 — no `score = 0` anywhere in the
log, and the iteration counter climbing steadily to its last printed value
with no early-stop signature. **248,535 iterations**, longer than search 1's
193,644 (load had dropped between the two runs — see the broadcast). Two
local-best improvements this time, permuter-score 710 -> 680 -> 495 (a third
saved variant at the unchanged base score, 710, is a type-widening no-op —
`s16 height` -> `int height` — worth nothing either way and not counted as
an improvement).

Both were read as C, not trusted as scores, same as search 1's four:

- **The new 495 candidate is the SAME bug as search 1's, cosmetically
  renamed.** It moves `row = b3;` into the `if (b2 != 0)` block and drops it
  from the `raw3 == 0` else branch, via an extra pointless `s8 new_var =
  arg1->unk3; b3 = new_var;` indirection that changes nothing about which
  path leaves `row` unassigned. Identical uninitialized-read on `b2 == 0 &&
  raw3 == 0`, identical rejection.
- **The 680 candidate is a THIRD instance of the same failure class, and
  the starkest yet.** It rewrites the `0x13` height-decrement guard as
  `if (raw3 == 0x13) { new_var = 1; height -= new_var; }` and then reads
  `self->unk88 = new_var;` — but `new_var` is declared with no initializer
  and is **only ever assigned inside that one rare-value branch**. On the
  overwhelmingly common path (`raw3 != 0x13`), `self->unk88` — a field this
  function's own earlier analysis established is unconditionally `1` for
  every call — gets set from a read of a completely unassigned local. This
  "works" only because GCC 2.6.3 happened to leave that register holding a
  leftover `1` from earlier in the function for this one compiled
  instantiation. Rejected on sight.

**Combined disposition across both searches: 442,179 iterations, zero
`score = 0` results, six local-best candidates examined by hand, every one
either a genuine correctness bug (four instances now, across two distinct
bug shapes — moved-assignment UB and read-before-any-assignment UB) or a
real-oracle regression on the one candidate that had no correctness
question at all.** Best reached remains **22/52**, unchanged from before
this round's searches. Not spending a third bound on this seed — the
pattern is now well-established (this scorer will keep finding "cheaper"
ways to skip assignments on the function's rare-input branches, because it
never executes the candidate and therefore never notices), and a
structurally different seed (not just fresh RNG on the same one) is what
the next attempt at this function actually needs, not another blind bound.
`INCLUDE_ASM` in place, worktree clean.

**Process note update:** the second attempt to capture the timeout's real
exit code (`printf 'PERMUTER_RC_MARKER=%s\n' "$rc" >> logfile; sync`) also
lost the race to `multiprocessing.resource_tracker`'s post-exit warning —
`sync` flushes to disk, it does not fix write-ordering between two
processes sharing one file offset. The exit code was never directly
observed in either search this round; both dispositions rest entirely on
content-level evidence (iteration count, absence of any zero score) rather
than the literal `$?`, which is sufficient here but is worth fixing
properly (a marker written to its OWN file, not appended to the shared
log) before relying on a captured `rc=` value for anything this round's
reports didn't need it for.

## Naming

Round 67 (track 3, naming pass). Still a documented STALL; naming applies to
the report and to the preserved body's identifiers, not to the shipped bytes.

| symbol | name | tier | evidence |
| --- | --- | --- | --- |
| `func_8004B030` | `StageMap__SetFootprintRect` | B | The sibling of `StageMap__SetFootprintFromCell`, taken on the `config->unk4 != 0` branch of `StageMap__ApplyToSenderFootprint`. Where that one writes a cell plus a span, this one writes a whole rectangle directly into `rects.e[0]` -- element index from `slot124`, then column, row, width and height, each derived from a descriptor byte with the same off-by-one and the same `0x13` edge case on both axes -- and sets `rectCount = 1`. Tier B. |

The preserved `#if 0` body keeps its own local variable names but its field
references were updated to the current names (`self->unk88` -> `rectCount`,
`self->unk8C` -> `rects`) in the same round. No code changed; the recorded
22/52 figure is unaffected.

Note `0x13` is 19, i.e. the last column of a 20-wide grid -- consistent with
the stride established in `StageMap__SetGridSpan.md`, and an independent
sighting of that 20.

### Field names in the preserved bodies above

Round 67 renamed this unit's struct fields. The preserved bodies in THIS
report are left in their original spelling -- preserved code is a record of
what was tried, not doctrine -- but they will not compile as written against
the current `include/class_39e08.h`. The mapping, for whoever rebuilds one:

| old | current |
| --- | --- |
| `self->unk88` | `self->rectCount` |
| `self->unk8C` | `self->rects` |
| `self->unkEC` | `self->elems` |
| `self->unk54` | `self->origin` |
| `self->unk68` | `self->config` |
| `self->unk70` | `self->enabled` |
| `self->unkBC` | `self->cellTag` |
| `self->unk1C0` / `unk1C2` / `unk1C3` | `curCellTag` / `curCellCol` / `curCellRow` |
| `self->unk1BC` | `self->lastEventElem` |
| `entry->unk0` / `unk2` / `unk4` / `unk8` / `unkC` / `unk10` / `unk14` | `flag` / `key` / `target` / `list` / `cellParent` / `cells` / `heldObj` |
| `HistoryEntry_3ac78` / `HistoryBlock_3ac78` | `GridRect_3ac78` / `GridRectList_3ac78` |
| `->methods->unk04(...)` | `->methods->release(...)` |

The `#if 0` copy that lives in `src/class_39e08.c` WAS updated to the current
names in the same round, so that one still compiles; only identifiers changed
and the recorded score is unaffected.

## Track 6 (2026-09-26, round 93, alpha)

The class `Class866E8` (table `gClass866E8Methods`, id 0x114, LightRig's
subclass) is now `StageMap` (`python3 tools/renametype.py Class866E8
StageMap`, tier B): it keeps seven slots loaded with map chunks of the
current stage (LbdFile, `STGnn\Mnnn.LBD`) around a tracked target, the
centre chunk and its six staggered neighbours (`sChunkNeighbourDeltas`), laid
out by the stage's `StageGridDimensions` (`setConfig`, from ObjM's
`GetStageGridDimensions(stage)`), each slot's placements linked into a 20 x
20 lattice of GridCells whose drawn window follows the target. Tier B: the
mechanics are established; "the stage's map" rests on the files it loads and
the per-stage config. Header now `include/StageMap.h`; evidence in its banner.

Member types, same pass: `Unk68Struct` is `StageGridDimensions`
(include/StageGrid.h), `Unk54Struct` is `LongVec3` (include/SceneNode.h),
`EntryDesc866E8` is `Ratio16[3]` (include/SceneNode.h), all by layout and
use; `Class866E8Elem` -> `ChunkSlot`, `QueryPos866E8` -> `SplitLongVec3`,
`SetupEntry866E8` -> `ChunkLoadEntry`, `SetupSub866E8` ->
`ChunkLoadEntryTail`, `TargetSpec866E8` -> `ChunkSlotSpec`, `GridSlot866E8`
-> `CellRect`, `GridSlotList866E8` -> `CellRectSet`, `Bounds866E8_3bb8c_b`
-> `CellBounds`, `Class866E8ValueFn` -> `ChunkFileFn`,
`Class866E8OnElementEventFn` -> `StageMapOnSlotEventFn`,
`Class866E8ElemFn` -> `ChunkSlotFn`, `Class866E8CellFn` -> `StageMapCellFn`;
new `ChunkNeighbourDelta` for `sChunkNeighbourDeltas` (was typed as the
3-word placeholder). renametype.py also rewrote the old names inside
earlier sections' history prose in this and sibling reports (known, pending
an operator decision; not hand-reverted).

## Track 7 (2026-09-27, round 98, charlie)

Moved here from the `.c` comment: "Clamp a span x span footprint centred on desc's cell to the 20 x 20 grid: a cell on the low edge (0) loses one row/column, one on the high edge (0x13) loses one too. The edge tests read a COPY of each byte taken before the decrement, and the height companion is `span` itself. Matched round 71." The comment keeps the description and a `MATCHING:` line for the two load-bearing shapes; `0x13` is `STAGE_CHUNK_CELLS - 1`. Zero bytes.
