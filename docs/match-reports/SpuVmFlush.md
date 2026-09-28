# SpuVmFlush -- STALL: 5 words short (236/241 built length), 93/241 raw word-match (drift-affected, not fully trustworthy), first real diff at file 0x1FF58 vram 0x8002F758 (retail's unconditional `move a2,v0`/`li t0,1`/`move a3,a0` setup inside the `count>0` block, which this C's `for` does not reproduce) -- see "Round 48 update" below

> Renamed from `UpdateVoiceEnvelopes` on 2026-09-23 (tools/rename.py). Address 0x8002f700.

> Renamed from `func_8002F700` on 2026-09-20 (tools/rename.py). Address 0x8002f700.

Unit: `src/libsnd_vmanager.c`. Round 26, runner bravo.

## Screens (clean)

```
grep -n 'gp_rel' asm/nonmatchings/libsnd_vmanager/SpuVmFlush.s            -> no hits
grep -A2 -nE '\b(mflo|mfhi)\b' ... | grep -E '\b(mult|multu|div|divu)\b'  -> no hits
```

## Result

`./build-and-verify.sh` GREEN with `INCLUDE_ASM` restored. Best attempt
compiled clean and reached **237 words against retail's 241** (measured via
`objdump -t build/src/libsnd_vmanager.c.o`, the reliable way once any drift
appears). The body below got the overwhelming majority of the function's
CONTENT and CONTROL FLOW byte-correct — five separate structural fixes each
moved the length substantially closer to retail (see "Axes tried" below) —
but a residual 4-word gap, most likely concentrated in the stack frame size,
was not resolved within this round's budget.

**Do not trust the raw funcdiff word-match number (49/241) as a measure of
how close this is.** Once ANY function's length is off, `asm-differ`'s
window comparison walks past the true end of the retail function into the
NEXT ones (confirmed here: past `~0x1ff00+0xf20` the "diff" output shows
content belonging to entirely different, unrelated functions, complete with
their own `jr $ra` returns). The only trustworthy way to assess this
specific function is to bound the comparison to its own true range
(`0x1ff00`-`0x202c4`, 241 words) and read within that window only — done for
every fix below.

## Round 30 (charlie) update: rebuilt, confirmed accurate

Re-spliced this exact preserved body and rebuilt from scratch. **All title
figures reconfirmed:** built length **237 words** (`objdump -t
build/src/libsnd_vmanager.c.o` shows `SpuVmFlush` at `0x3b4` bytes = 237
words, retail is 241, so 4 short exactly as titled), `funcdiff.py`'s
in-range figure **49/241** with its own drift warning firing (matching the
report's own caution not to trust that number as a distance measure).

**One correction found during the rebuild, cosmetic only (does not affect
the score): the preserved body's `func_800375E8(0, 0xFFFFFF)` call used a
STALE placeholder name.** `asm/nonmatchings/libsnd_vmanager/SpuVmFlush.s`
now names this call `SpuSetNoiseVoice` (`config/symbols.slps01556.lsdde.txt`
line 263, Psy-Q `libspu`, from the SDK-object-linking work in later
rounds) — same staleness class found in `SpuVmInit`'s two SPU calls
this round. The preserved body below has been updated to the current name;
nothing about the residue or the score changes. No new structural axis was
attempted this round given the time budget and the six-function work list;
the 2-word frame gap plus 2 unidentified words remain as described above.

## Signature and shape (believed fully correct in content, order and control flow)

`void SpuVmFlush(void)` — no arguments (confirmed: `SpuVmInit`'s own
report, and this function's own prologue, never touch `$a0` before first
overwriting it). Five phases, each independently verified against the
disassembly instruction-by-instruction within the true `0x1ff00`-`0x202c4`
window:

1. **Ring-buffer bookkeeping.** `_svm_envx_ptr` is a rotating index (`(x+1)
   & 0xF`), stored back immediately; `_svm_envx_hist[ringIdx]` (an `s32[16]`
   array) is the new ring slot, zeroed.
2. **Per-channel "ready" snapshot**, guarded by `spuVmMaxVoice > 0`: for each
   channel `i`, copy `_svm_sreg[i].unkC` (a NEW field on the already
   `+0x194`/`+0x196`-known `_svm_sreg` object, read here as part of an
   independent 0x10-byte-stride array view — see struct notes below) into
   `D_8008D98E[i]` (unsigned 16-bit; re-read via `lhu` after the store, so
   needs an UNSIGNED view, not the existing signed `Rec34Half`), and if
   that snapshot is zero, OR the channel's bit into the new ring slot.
   Written with EXPLICIT WALKING POINTERS (`p98E`, `pDad`, each `++`
   advancing one whole record), not `array[i]` indexing — see "Axes tried"
   #1, the single highest-value fix this round.
3. **Starved-channel force-release**, guarded by `_svm_auto_kof_mode == 0`: AND all
   FIFTEEN of the OTHER ring slots together (a plain `for (j=0;j<0xF;j++)
   mask &= _svm_envx_hist[j];` loop — note this reads only 15 of the 16 slots,
   confirmed against the raw instruction count), then for each channel
   whose bit is set in that combined mask, force-release it
   (`func_800375E8(0, 0xFFFFFF)` if `D_8008D9A3[i] == 2`, then zero it
   regardless).
4. Two unconditional bitmask updates:
   `_svm_okon1 &= ~_svm_okof1; _svm_okon2 &= ~_svm_okof2;`
5. **Per-channel interpolation dispatch**, unconditional 0..0x17 loop:
   `SetAutoVol(i)` if `D_8008D9A4[i] != 0`, `SetAutoPan(i)` if
   `D_8008D9B0[i] != 0` (both still `INCLUDE_ASM` themselves — see their own
   match reports).
6. **Flag-driven per-channel field copy**, another unconditional 0..0x17
   loop, testing four independent bits of `_svm_sreg_dirty[i]` (1, 4, 8, 0x10)
   and copying the corresponding `_svm_sreg_buf`/`D_8008D7F4`/`D_8008D7F6`
   field into the matching `_svm_sreg[i]` field (`+0`/`+2` for bit 1,
   `+4` for bit 4, `+6` for bit 8, `+8`/`+0xA` for bit 0x10), then zeroing
   `_svm_sreg_dirty[i]`. **`_svm_sreg_dirty[i]` must be re-read fresh for EACH of the
   four `if` conditions, not cached in a local** — caching it into one
   `u8 flags` local changed nothing measurably here (this function had a
   different problem at the time), but is the established idiom project-
   wide and was kept removed on principle.
7. Unconditional tail: read `_svm_okof1`/`_svm_okof2`/`_svm_okon1`/
   `_svm_okon2`/`_svm_orev1`/`_svm_orev2` into locals, zero the first four
   globals, then store all six into fixed offsets of the SAME `_svm_sreg`
   object phase 2 read as an array (`+0x18C`/`+0x18E`/`+0x188`/`+0x18A`/
   `+0x198`/`+0x19A` — a THIRD independent view of `_svm_sreg`, alongside
   the `+0x194`/`+0x196` single-struct view and the 0x10-stride array
   view).

## Struct/global knowledge derived this round

- `_svm_envx_ptr` (`s32`, ring index 0-15) and `_svm_envx_hist[]` (`s32[16]`, ring
  buffer of per-call "channel ready" bitmasks).
- `D_8008D98E[]`: needs an UNSIGNED 16-bit view (`Rec34HalfU2`, not the
  existing signed `Rec34Half`) — confirmed by the `lhu` re-read after the
  store, same idiom already established for other members of this 0x34-
  stride family elsewhere in this unit.
- `_svm_sreg` gains a THIRD independent local view in this function
  (`Rec16DAD4C`, 0x10-byte stride, fields at `+0`,`+2`,`+4`,`+6`,`+8`,`+0xA`,
  and now also `+0xC` [`u16`, read via `lhu`] beyond the six fields
  `SpuVmInit`'s stalled report already established) — on top of the
  existing single-struct view (`+0x194`/`+0x196`) and this function's own
  final-tail single-struct-again view (`+0x188` through `+0x19A`). Three
  independent readings of the SAME base pointer in different parts of ONE
  function, all confirmed structurally correct against the disassembly.
- `Rec16D7F0Wide`: `_svm_sreg_buf`'s existing `Rec16D7F0` type (declared
  earlier in this file, field `unk0` only) needs widening for this
  function, which ALSO reads `+0x2`, `+0x8` and `+0xA` — reached via a cast
  to a wider local struct type, not a redeclaration (same rule as every
  other multi-width symbol in this file).
- `D_8008D7F6[]`: a NEW 0x10-byte-stride array, same shape as the already-
  established `Rec16D7F4`/`D_8008D7F4`.
- `_svm_auto_kof_mode` (`u8` flag), `_svm_orev1`/`_svm_orev2` (`s16`).

## Axes tried, in order, with effect on built length (retail is 241 words)

1. **Baseline: `array[i]` indexing throughout, including for
   `D_8008D98E`/`_svm_sreg` in phase 2**: **258/241 (17 words too LONG).**
   Recomputing the 0x34-stride and 0x10-stride offsets via multiply-chains
   each iteration, instead of walking pointers, produced substantially MORE
   code than retail for phase 2's loop. Retail visibly hoists a `_svm_sreg`
   base LOAD and TWO derived-pointer sets (`Rec34Half`-typed and
   `Rec16DAD4`-typed) OUTSIDE the loop and increments them by one record
   each iteration — this is the standard MIPS-o32-at-`-O2` "dual induction
   variable" pattern (index kept alive for a bit-shift, PLUS a pointer
   walked in lockstep for the memory accesses), not something `array[i]`
   indexing reliably reproduces from this compiler.
2. **Rewrote phase 2 with explicit walking pointers** (`Rec34HalfU2 *p98E`,
   `Rec16DAD4C *pDad`, both incremented once per iteration): **231/241 (10
   words too SHORT)** — overshot in the other direction. This was still the
   right lever (phase 2's own instructions became near-exact matches, only
   register-renamed), but two things elsewhere were ALSO wrong and had been
   masking each other:
   - the `for (j ...)` / `for (i ...)` loop counters were declared `s16`,
     which this compiler re-narrows (`sll`/`sra` pair) after every
     increment when the variable is ALSO used in a context requiring the
     narrow value (the bit-shift `1 << i`); retail's counters are plain,
     un-narrowed `addiu` increments throughout, meaning the SOURCE type is
     a wider int (`s32`), not `s16`. **Declaring all of this function's
     loop counters `s32` instead of `s16`** dropped the extraneous
     `sll`/`sra` triplet after every `i++`/`j++` project-wide in this
     function.
   - `if (mask & (1 << i))` compiled to `(mask >> i) & 1` (an `srav` +
     `andi 0x1`) rather than retail's `(1 << i) & mask` (an `sllv` then
     `and`) — cc1 apparently picks the shift-then-mask-1 form for a bare
     bit-test used only in a boolean context, and only the sllv-then-and
     form when the shifted value is materialized as its own expression.
     **Extracting `s32 bit = 1 << i;` into its own statement before the
     `if`** forced the `sllv` form, matching retail.
3. **Applied both fixes together**: **236/241 (5 words short)** — the two
   fixes combined moved it further from 231, i.e. in the right direction
   but with 5 words still missing.
4. **Rewrote phase 6 (the `_svm_sreg_dirty`-flag loop) with a SINGLE walking
   pointer for `_svm_sreg_buf` (all four field offsets via one struct
   pointer), but KEPT a second walking pointer (`pDad`) for `D_6006DAD4`
   as well**: **233/241 (8 words short)** — WORSE than #3. Reading the
   disassembly closely at this point showed retail RE-LOADS `_svm_sreg`
   (a fresh `lui`/`lw` pair) inside EACH of the four `if` blocks
   individually, rather than hoisting it once — the opposite of what
   phase 2 does for the SAME symbol. Introducing a shared `pDad` pointer
   here was the wrong lever for this specific loop.
5. **Kept the `_svm_sreg_buf` walking pointer from #4, but switched
   `D_6006DAD4` back to a fresh `((Rec16DAD4C *) _svm_sreg)[i]` cast in
   EACH of the four conditional blocks** (matching the fresh-reload
   pattern observed): **237/241 (4 words short)** — the best result
   reached, and the one preserved below. Within the true `0x1ff00`-
   `0x202c4` window, phases 2 through 7 are now either EXACT matches or
   pure register renames; the residual gap did not localize to any single
   readable instruction difference inside that window.

## What's missing: most likely 2 words of stack frame, plus 2 more not isolated

The clearest structural signal: **retail's frame is `-0x38`; the best
attempt's is `-0x30`**, an 8-byte (2-word) gap. Both use the identical set
of five callee-saved registers (`grep -oE '\$(s[0-7]|fp)' … | sort -u` on
the retail `.s` gives exactly `$s0`-`$s4`, matching this attempt's own
register usage one-for-one), and grepping the retail `.s` for any
`($sp)`-relative access OUTSIDE the six register-save/restore instructions
finds NONE — so the extra 0x20 bytes of retail's frame (`0x38` total minus
`0x18` for `ra`+5 saved registers) is not a directly-addressed local
variable or array; it reads as reserved-but-unused stack space, the kind
o32 ABI frames carry for an outgoing-argument area sized larger than this
function's own calls need. Neither adjusting local variable count/order
nor any of the axes above moved this specific 8 bytes. The remaining 2
words (4 bytes each, retail 241 vs this attempt's 237 minus the 2-word
frame gap = 2 words unaccounted) were not isolated within budget.

### Proposed learning

**Two loop-counter lessons, both generalizable to any function in this
project with a `for` loop over a byte-or-word count that also needs the
index for a bit-shift or an array subscript check:**

1. **Declare loop counters `s32`/plain `int`-width, not `s16`, unless the
   retail disassembly shows a genuine truncation.** An `s16` counter used
   in ANY expression requiring its narrow value (a variable shift amount,
   a comparison against a masked value) gets an extra `sll`/`sra`
   re-narrowing pair emitted after EVERY increment by this compiler, which
   a plain-`int`-width counter never needs. This cost 3 extra instructions
   per loop in this function alone, times three affected loops.
2. **`if (mask & (1 << i))` and `if ((1 << i) & mask)` are NOT
   interchangeable at the instruction level, even though they are
   value-identical.** This compiler lowers a bit-test used only in a
   boolean CONTEXT (no other use of the shifted value) to `(mask >> i) &
   1`, but lowers the same test to a literal `1 << i` THEN `and` if the
   shifted value is pulled into its own named expression first. When
   retail's disassembly shows an `sllv`/`and` pair rather than an
   `srav`/`andi 0x1` pair for what reads as a single-bit test, extract the
   shift into its own statement before the `if`.

**A third, non-loop lesson: re-reading a global pointer fresh inside EACH
of several independent `if` blocks, rather than hoisting it once outside
them, is not automatically "wrong" or "worse" — it can be the CORRECT
match even when a walking-pointer hoist is exactly right for a superficially
similar loop earlier in the SAME function.** Phase 2 (this function) and
phase 6 both index through the SAME `_svm_sreg` global pointer inside a
loop, and the correct C shape for one (a hoisted, incremented pointer) was
measurably WRONG for the other (which wants a fresh cast-and-index at each
use site). The discriminator was not obvious from either loop's shape in
isolation — it only showed up as a length regression when guessed wrong,
confirmed by reading whether retail's disassembly re-emits the `lui`/`lw`
pair for the global pointer inside each conditional block (fresh reload) or
hoists it once outside the loop (persistent pointer). Check this
per-loop, never assume consistency across two loops touching the same
symbol in the same function.

## Round 37 (bravo) update: rebuilt (confirmed accurate), permuter searched for the first time

Round 37's designated permuter-priority item 2 (this unit's five stalls are
the largest never-permuter-searched block in the corpus this round -- 73/111
near-misses project-wide already have a search logged, none of this unit's
did before this round). Per the round's "build the inherited body before
you trust its score" instruction, this exact preserved body was
re-spliced into the live unit (reusing this file's already-declared
`Rec34Half`/`Rec16D7F0`/`SpuRegs` types via casts rather than the isolated
splice's own flat externs, since this function is not first in the file
and everything it needs was already declared upstream) and rebuilt from
scratch.

**All title figures reconfirmed exactly:** `objdump -t
build/src/libsnd_vmanager.c.o` shows `SpuVmFlush` at `0x3b4` bytes = **237
words** (retail 241, 4 short, exactly as titled), `funcdiff.py` reports
**49/241** in-range with its drift warning firing, matching this report's
own caution not to trust that figure as a distance measure.

### Permuter search

`tools/setup-permuter.sh SpuVmFlush <seed>` -- seed built from this
report's preserved body. Since the built length itself is short by 4
words, the permuter's own scorer (which diffs against retail's actual
bytes, not a realigned window) is expected to report a nonzero baseline
dominated by insertions/deletions from the length gap itself, on top of
any register residue. See the Permuter result subsection for the base
`--debug --stack-diffs` score and the real search's outcome (iteration
count and `rc`).

#### Permuter result

`--debug --stack-diffs` base score: **2276** (Stack Differences 96 x
weight 1 = 96 -- confirming the report's own 2-word/8-byte frame-gap
finding, since `--stack-diffs` is what makes that visible at all;
Register Differences 92 x 5 = 460; Reorderings 2 x 60 = 120; Insertions 6
x 100 = 600; Deletions 10 x 100 = 1000; zero Branch differences).

Real search: `timeout 900 permuter.py -j 6 --stop-on-zero --best-only
--stack-diffs`, launched via the harness's own `run_in_background` (not
the hand-rolled `cmd & ; echo rc=$?` pattern -- see `SpuVmPBVoice.md`'s
round 37 update for why that pattern lost its exit-code marker on the
previous search this round). **Completed cleanly with `rc=124`** (the
search's own 900s bound, not an external kill) after **63,192
iterations**. Best score reached: **1578** (from base 2276), saved at
`permuter-work/SpuVmFlush/output-1578-1/`; no candidate reached zero.

Diffing the 1578 candidate against the scaffold's `base.c` (both
re-extracted to just the function body and compared) shows the
improvement comes from materializing `count > 0` into an explicit boolean
temporary (`new_var = count > 0; if (new_var) {...}`) rather than any
translatable structural change -- a permuter-internal boolean-hoisting
mutation with no natural idiomatic C phrasing, not unlike `SpuVmPBVoice`'s
own best candidate this round. **Not closed; the frame-size gap and the
associated register-class residue this report already diagnoses did not
move.** Given this function's own report already identifies the extra
0x20 bytes of retail's frame as unaddressed reserved outgoing-argument
space (not a variable this C fails to declare), and the permuter's own
best result never touched that gap, this is consistent with the report's
existing "two words unaccounted for" conclusion rather than a new lead.

## Round 48 update (runner echo): tested charlie's frame-padding lever -- realigns the frame exactly, does NOT close the length gap

Round 48's designated test of charlie's `ContDataEntry` discovery (a
`u8 dead[N];` local under `if (0) { dead[0] = 0; }`, sized to the gap
between the CURRENT BUILD's frame and retail's -- not retail's raw
unaddressed-byte count). Rebuilt the round-37 preserved body first, in
isolation, per this round's "rebuild before trust" rule: **reconfirmed
exactly** -- 237/241 built words, `funcdiff.py` 49/241 in-range with its
drift warning firing, matching this report's own prior figures.

### The frame gap, measured directly (before the fix)

This build's own frame was `addiu sp,sp,-0x30` (48 bytes); retail's is
`addiu sp,sp,-0x38` (56 bytes) -- an 8-byte/2-word gap, exactly as this
report's own pre-round-48 "What's missing" section already named it.
Grepping the retail `.s` for `($sp)`-relative operands confirms (same
method as `ContDataEntry`'s own derivation) that **nothing in retail
addresses anything below `0x20($sp)`** -- the register-save block occupies
`0x20`-`0x34`, and the remaining `0x20` bytes below that (`0x0`-`0x20`) are
never touched by any instruction in the whole function. Per charlie's own
refinement, sized the padding to THIS BUILD's gap (8 bytes), not retail's
full unaddressed span.

### Fix 1: `u8 dead[8];` under `if (0) { dead[0] = 0; }`

Rebuilt: `addiu sp,sp,-0x38` -- **frame now byte-IDENTICAL to retail**,
confirmed via `objdump -d`. **Built length: UNCHANGED at 237/241.** This is
the headline finding: unlike `ContDataEntry`, where the frame fix combined
with a SEPARATE tail-duplication fix that added real instructions, here the
frame-size correction is purely a change to the `addiu`/`sw` immediate
OPERANDS of already-existing prologue/epilogue instructions -- it costs (and
recovers) exactly the same INSTRUCTION COUNT regardless of the immediate
value, so it cannot by itself close a missing-CONTENT gap. Raw word-match
moved only marginally (49/241 -> 54/241) from incidental register-offset
realignment lower in the function, not from any new match.

**Read literally: charlie's lever, when the extra frame bytes are pure
unaddressed register-save-area padding with no companion missing
instructions elsewhere (unlike `ContDataEntry`, which ALSO had a
duplicated-tail gap), fixes frame ALIGNMENT exactly but leaves the built
LENGTH untouched.** It is still worth applying (it makes the rest of the
disassembly comparison meaningful instead of running through a
26-byte-misaligned lens), but it is not by itself a length-closing move.

### Fix 2: `s32 count` instead of `u8 count` -- found BECAUSE the frame fix made the diff readable

With the frame aligned, `tools/asm-differ/diff.py SpuVmFlush` showed a
real, localized residue right after the frame/prologue: this build's
`if (count > 0)` (with `u8 count`) compiled to an extra `andi a0,a0,0xff`
before the `beqz`, where retail uses a single `blez a0,...` directly on the
freshly-`lbu`-loaded (already zero-extended) value, with no re-mask.
Changing the local's declared type from `u8 count` to `s32 count` (still
correctly holding a `spuVmMaxVoice` value, 0-255) drops the spurious mask and
reproduces retail's `blez` exactly, at the exact same address (`0x1ff50`
both sides). **Raw word-match jumped 54/241 -> 93/241.** Built length moved
to **236/241 (5 words short, one word SHORTER** than before this fix,
because the removed `andi` was a real instruction retail's `blez` path
does not pay for -- so the total gap grew by one word even as the local
match improved, confirming this report's own standing caution that length
and raw-match do not move together once drift is present).

### One further lever tried and NEGATIVE: do-while conversion for the now-visible loop-guard residue

With fixes 1+2 applied, the diff shows one more concrete, LOCAL residue:
retail's `count>0` block unconditionally sets up `a2=slot`, `t0=1`,
`a3=count` right after the `blez`, then runs the loop with a single
tail `bnez` (no separate upfront bound check) -- the classic
do-while-after-a-guard-if shape this project documents elsewhere. This
build's compiled `for (i = 0; i < count; i++) { ... }` inside the same
`if (count > 0)` instead emits an EXTRA `slt`/`beqz` pair before the first
iteration (GCC 2.6.3 does not infer, from the enclosing `if`, that the
`for`'s own bound check is redundant).

**Tried: converting the loop to `i = 0; do { ... i++; } while (i < count);`
matching the shape retail's control flow implies.** **Hard NEGATIVE**:
regressed to 233/241 built length and 16/241 raw match (from 236/241 and
93/241) -- markedly worse in both dimensions. Reverted immediately. Unlike
the `ContDataEntry`/other documented do-while conversions, this one made
the compiler choose a substantially different (and worse) instruction
sequence rather than the one retail shows; something about this specific
loop's surrounding pointer/register pressure (`p98E`/`pDad` walking
pointers alongside the `slot`/`count` values) makes the naive do-while
transcription the wrong lever here. Not a lever to retry without a new idea
about why it regresses instead of matching.

### Net result and disposition

**49/241 -> 93/241 raw word-match, oracle-verified** (both fixes rebuilt
through `./build-and-verify.sh`, non-`INCLUDE_ASM` build confirmed to
compile clean each time, reverted to `INCLUDE_ASM` and whole-image SHA1
reconfirmed green after). Built length is now 236/241 (5 words short, was
4). **Not byte-exact; restored to `INCLUDE_ASM` per project rule.** The
frame fix and the `s32 count` fix are both worth keeping in the next
attempt's starting point; the do-while conversion is a confirmed dead end
for this specific loop. The remaining gap is concentrated in: (a) the
`move a2,v0`/`li t0,1`/`move a3,a0` unconditional-setup-before-loop shape
retail uses (do-while-like control flow that a literal do-while rewrite did
NOT reproduce -- open), and (b) pure register-color swaps (`s0`<->`s1`
throughout phases 2-6) that are cosmetically harmless but still count
against raw match.

### Proposed learning

**Charlie's `dead[N]`/`if(0)` frame-padding idiom recovers frame BYTE
ALIGNMENT exactly (sized to the CURRENT BUILD's gap vs retail, confirmed
again here), but it does not recover missing INSTRUCTION WORDS by itself
when the extra frame bytes are pure unaddressed register-save-area padding
with no separate content gap (e.g. no duplicated tail) elsewhere in the
function.** `ContDataEntry`'s own length recovery came from a SEPARATE
fix (tail duplication) applied alongside the frame padding, not from the
padding itself -- worth stating explicitly since the padding fix's real
value here turned out to be diagnostic (it makes the REST of the function's
disassembly comparison meaningful) rather than word-count-closing. Screen:
after applying the padding and confirming the frame's `addiu` immediate
matches retail exactly, re-read the diff for genuinely NEW localized
residues (like the `andi 0xff` mask found here) rather than assuming the
length gap itself has closed.

## Preserved body (best attempt this round, 236/241 built words -- 5 short, 93/241 raw word-match; phases 2-7 individually confirmed correct in content/order within the true function window; do-while conversion of phase 2's loop CONFIRMED NEGATIVE, do not retry without a new idea)

```c
#if 0
/* Round 48 (echo): frame padded to match retail exactly (dead[8], see
 * "Round 48 update" above); count changed from u8 to s32 to drop a
 * spurious andi mask GCC inserted for the byte-typed local (matches
 * retail's blez-without-remask exactly). Do NOT convert phase 2's loop to
 * do-while -- tried, confirmed hard regression (236/241,93/241 ->
 * 233/241,16/241). */

/* Ring buffer of "channel activity" bitmasks, one slot appended per
 * call, most-recent index tracked by _svm_envx_ptr (mod 16). */
extern s32 _svm_envx_ptr;
extern s32 _svm_envx_hist[];

/* 0x34-stride record family, UNSIGNED 16-bit view -- this function
 * writes it via `lhu`-driven re-reads (store, then re-check the SAME
 * field unsigned) rather than the `Rec34Half` signed view. D_8008D98E
 * already declared elsewhere in the unit as `Rec34Half`; reinterpreted
 * here via cast, not redeclared. */
typedef struct {
    u16 unk0; /* +0x0 */
    u8 pad2[0x34 - 0x2];
} Rec34HalfU2;

/* Flag byte: when set, skip the "channel starved for N frames -> force
 * release" scan below. */

extern void SpuSetNoiseVoice(s32 a0, s32 a1);
extern void SetAutoVol(s16 a0);
extern void SetAutoPan(s16 a0);
extern Rec16D7F4 D_8008D7F6[];

/* Same 0x10-byte-stride record family as `Rec16D7F0`/_svm_sreg_buf's other
 * field (declared above, `unk0` only) -- this function ALSO reads this
 * array's `+0x2`, `+0x8` and `+0xA` sub-fields, so it needs a wider
 * local view of the same base symbol, reached via a cast per this
 * project's multiple-independent-local-views convention. */
typedef struct {
    s16 unk0; /* +0x0 */
    s16 unk2; /* +0x2 */
    u8 pad4[0x8 - 0x4];
    s16 unk8; /* +0x8 */
    s16 unkA; /* +0xA */
    u8 padC[0x10 - 0xC];
} Rec16D7F0Wide;

/* _svm_sreg, already declared above as `SpuRegs *` (one struct, fields
 * at +0x194/+0x196), is ALSO the base of an array of 0x10-byte
 * per-channel records here -- another independent local view of the
 * same pointed-to object (see also libsnd_vmanager.c's own array-of-0x10
 * reading of a sibling symbol). */
typedef struct {
    s16 unk0; /* +0x0 */
    s16 unk2; /* +0x2 */
    s16 unk4; /* +0x4 */
    s16 unk6; /* +0x6 */
    s16 unk8; /* +0x8 */
    s16 unkA; /* +0xA */
    u16 unkC; /* +0xC */
    u8 padE[0x10 - 0xE];
} Rec16DAD4C;

void SpuVmFlush(void) {
    s32 i = 0;
    s32 ringIdx;
    s32 *slot;
    s32 count;
    u8 dead[8];

    if (0) {
        dead[0] = 0;
    }

    ringIdx = (_svm_envx_ptr + 1) & 0xF;
    _svm_envx_ptr = ringIdx;
    slot = &_svm_envx_hist[ringIdx];
    count = spuVmMaxVoice;
    *slot = 0;

    if (count > 0) {
        Rec34HalfU2 *p98E = (Rec34HalfU2 *) D_8008D98E;
        Rec16DAD4C *pDad = (Rec16DAD4C *) _svm_sreg;

        for (i = 0; i < count; i++) {
            p98E->unk0 = pDad->unkC;
            if (p98E->unk0 == 0) {
                *slot |= 1 << i;
            }
            p98E++;
            pDad++;
        }
    }

    if (_svm_auto_kof_mode == 0) {
        s32 mask;
        s32 j;

        mask = -1;
        for (j = 0; j < 0xF; j++) {
            mask &= _svm_envx_hist[j];
        }

        for (i = 0; i < spuVmMaxVoice; i++) {
            s32 bit = 1 << i;

            if (mask & bit) {
                if (D_8008D9A3[i].unk0 == 2) {
                    SpuSetNoiseVoice(0, 0xFFFFFF);
                }
                D_8008D9A3[i].unk0 = 0;
            }
        }
    }

    _svm_okon1 &= ~_svm_okof1;
    _svm_okon2 &= ~_svm_okof2;

    for (i = 0; i < 0x18; i++) {
        if (D_8008D9A4[i].unk0 != 0) {
            SetAutoVol(i);
        }
        if (D_8008D9B0[i].unk0 != 0) {
            SetAutoPan(i);
        }
    }

    {
    Rec16D7F0Wide *p7F0 = _svm_sreg_buf;

    for (i = 0; i < 0x18; i++) {
        if (_svm_sreg_dirty[i] & 1) {
            ((Rec16DAD4C *) _svm_sreg)[i].unk0 = p7F0->unk0;
            ((Rec16DAD4C *) _svm_sreg)[i].unk2 = p7F0->unk2;
        }
        if (_svm_sreg_dirty[i] & 4) {
            ((Rec16DAD4C *) _svm_sreg)[i].unk4 = D_8008D7F4[i].unk0;
        }
        if (_svm_sreg_dirty[i] & 8) {
            ((Rec16DAD4C *) _svm_sreg)[i].unk6 = D_8008D7F6[i].unk0;
        }
        if (_svm_sreg_dirty[i] & 0x10) {
            ((Rec16DAD4C *) _svm_sreg)[i].unk8 = p7F0->unk8;
            ((Rec16DAD4C *) _svm_sreg)[i].unkA = p7F0->unkA;
        }

        _svm_sreg_dirty[i] = 0;
        p7F0++;
    }
    }

    {
        SpuRegs *rec = _svm_sreg;
        u16 lowMask = _svm_okof1;
        u16 highMask = _svm_okof2;
        u16 lowActive = _svm_okon1;
        u16 highActive = _svm_okon2;
        s16 v230 = _svm_orev1;
        s16 v234 = _svm_orev2;

        _svm_okof1 = 0;
        _svm_okof2 = 0;
        _svm_okon1 = 0;
        _svm_okon2 = 0;

        *(u16 *) ((u8 *) rec + 0x18C) = lowMask;
        *(u16 *) ((u8 *) rec + 0x18E) = highMask;
        *(u16 *) ((u8 *) rec + 0x188) = lowActive;
        *(u16 *) ((u8 *) rec + 0x18A) = highActive;
        *(s16 *) ((u8 *) rec + 0x198) = v230;
        *(s16 *) ((u8 *) rec + 0x19A) = v234;
    }
}
#endif
```

## Old preserved body (round 26-37, 237/241 built words -- 4 short, kept for reference; superseded above)

```c
#if 0
/* Ring buffer of "channel activity" bitmasks, one slot appended per
 * call, most-recent index tracked by _svm_envx_ptr (mod 16). */
extern s32 _svm_envx_ptr;
extern s32 _svm_envx_hist[];

/* 0x34-stride record family, UNSIGNED 16-bit view -- this function
 * writes it via `lhu`-driven re-reads (store, then re-check the SAME
 * field unsigned) rather than the `Rec34Half` signed view. */
typedef struct {
    u16 unk0; /* +0x0 */
    u8 pad2[0x34 - 0x2];
} Rec34HalfU2;
extern Rec34HalfU2 D_8008D98E[];
extern Rec34Half D_8008D9A4[];

/* Flag byte: when set, skip the "channel starved for N frames -> force
 * release" scan below. */
extern u8 _svm_auto_kof_mode;

extern u16 _svm_okof1;
extern u16 _svm_okof2;
extern u16 _svm_okon1;
extern u16 _svm_okon2;
extern s16 _svm_orev1;
extern s16 _svm_orev2;
extern s32 D_8008E258;
extern s32 D_8008E25C;

extern void SpuSetNoiseVoice(s32 a0, s32 a1);
extern void SetAutoVol(s16 a0);
extern void SetAutoPan(s16 a0);
extern Rec16D7F4 D_8008D7F6[];

/* Same 0x10-byte-stride record family as `Rec16D7F0`/_svm_sreg_buf's other
 * field (declared above, `unk0` only) -- this function ALSO reads this
 * array's `+0x2`, `+0x8` and `+0xA` sub-fields, so it needs a wider
 * local view of the same base symbol, reached via a cast per this
 * project's multiple-independent-local-views convention. */
typedef struct {
    s16 unk0; /* +0x0 */
    s16 unk2; /* +0x2 */
    u8 pad4[0x8 - 0x4];
    s16 unk8; /* +0x8 */
    s16 unkA; /* +0xA */
    u8 padC[0x10 - 0xC];
} Rec16D7F0Wide;

/* _svm_sreg, already declared above as `SpuRegs *` (one struct, fields
 * at +0x194/+0x196), is ALSO the base of an array of 0x10-byte
 * per-channel records here -- another independent local view of the
 * same pointed-to object (see also libsnd_vmanager.c's own array-of-0x10
 * reading of a sibling symbol). */
typedef struct {
    s16 unk0; /* +0x0 */
    s16 unk2; /* +0x2 */
    s16 unk4; /* +0x4 */
    s16 unk6; /* +0x6 */
    s16 unk8; /* +0x8 */
    s16 unkA; /* +0xA */
    u16 unkC; /* +0xC */
    u8 padE[0x10 - 0xE];
} Rec16DAD4C;

void SpuVmFlush(void) {
    s32 i = 0;
    s32 ringIdx;
    s32 *slot;
    u8 count;

    ringIdx = (_svm_envx_ptr + 1) & 0xF;
    _svm_envx_ptr = ringIdx;
    slot = &_svm_envx_hist[ringIdx];
    count = spuVmMaxVoice;
    *slot = 0;

    if (count > 0) {
        Rec34HalfU2 *p98E = D_8008D98E;
        Rec16DAD4C *pDad = (Rec16DAD4C *) _svm_sreg;

        for (i = 0; i < count; i++) {
            p98E->unk0 = pDad->unkC;
            if (p98E->unk0 == 0) {
                *slot |= 1 << i;
            }
            p98E++;
            pDad++;
        }
    }

    if (_svm_auto_kof_mode == 0) {
        s32 mask;
        s32 j;

        mask = -1;
        for (j = 0; j < 0xF; j++) {
            mask &= _svm_envx_hist[j];
        }

        for (i = 0; i < spuVmMaxVoice; i++) {
            s32 bit = 1 << i;

            if (mask & bit) {
                if (D_8008D9A3[i].unk0 == 2) {
                    SpuSetNoiseVoice(0, 0xFFFFFF);
                }
                D_8008D9A3[i].unk0 = 0;
            }
        }
    }

    _svm_okon1 &= ~_svm_okof1;
    _svm_okon2 &= ~_svm_okof2;

    for (i = 0; i < 0x18; i++) {
        if (D_8008D9A4[i].unk0 != 0) {
            SetAutoVol(i);
        }
        if (D_8008D9B0[i].unk0 != 0) {
            SetAutoPan(i);
        }
    }

    {
    Rec16D7F0Wide *p7F0 = _svm_sreg_buf;

    for (i = 0; i < 0x18; i++) {
        if (_svm_sreg_dirty[i] & 1) {
            ((Rec16DAD4C *) _svm_sreg)[i].unk0 = p7F0->unk0;
            ((Rec16DAD4C *) _svm_sreg)[i].unk2 = p7F0->unk2;
        }
        if (_svm_sreg_dirty[i] & 4) {
            ((Rec16DAD4C *) _svm_sreg)[i].unk4 = D_8008D7F4[i].unk0;
        }
        if (_svm_sreg_dirty[i] & 8) {
            ((Rec16DAD4C *) _svm_sreg)[i].unk6 = D_8008D7F6[i].unk0;
        }
        if (_svm_sreg_dirty[i] & 0x10) {
            ((Rec16DAD4C *) _svm_sreg)[i].unk8 = p7F0->unk8;
            ((Rec16DAD4C *) _svm_sreg)[i].unkA = p7F0->unkA;
        }

        _svm_sreg_dirty[i] = 0;
        p7F0++;
    }
    }

    {
        SpuRegs *rec = _svm_sreg;
        u16 lowMask = _svm_okof1;
        u16 highMask = _svm_okof2;
        u16 lowActive = _svm_okon1;
        u16 highActive = _svm_okon2;
        s16 v230 = _svm_orev1;
        s16 v234 = _svm_orev2;

        _svm_okof1 = 0;
        _svm_okof2 = 0;
        _svm_okon1 = 0;
        _svm_okon2 = 0;

        *(u16 *) ((u8 *) rec + 0x18C) = lowMask;
        *(u16 *) ((u8 *) rec + 0x18E) = highMask;
        *(u16 *) ((u8 *) rec + 0x188) = lowActive;
        *(u16 *) ((u8 *) rec + 0x18A) = highActive;
        *(s16 *) ((u8 *) rec + 0x198) = v230;
        *(s16 *) ((u8 *) rec + 0x19A) = v234;
    }
}
#endif
```

## Naming

**Superseded, round 71 (track 2):** the track-3 game name
`UpdateVoiceEnvelopes` is replaced by Sony's own name -- `psyq-objects.ld`
pins this address to `SpuVmFlush` (a linked Sony object calls it by that
name), and it also fingerprint-matches libsnd/vmanager `SpuVmFlush` at
masked 0.99 (disc 3.3). This is Sony's SDK code, not decompiled game logic;
track 2 names those functions and moves them out of tracks 1/1b/3.

**SpuVmFlush** (was `func_8002F700`) -- Tier B. Body mostly
evident (preserved below, 4 words short): appends this tick's voice-
activity bitmask to a 16-slot ring buffer (`_svm_envx_ptr`/
`_svm_envx_hist`), and once 16 consecutive ticks show a voice as
inactive, force-releases it (silencing the SPU noise generator first if
its state was the `2`/noise value SpuVmNoiseOff also reacts to); then
clears the active-voice mask and calls SetAutoVol/SetAutoPan
for every voice whose respective flag is set. Named for the dispatch
role, which is unambiguous; the exact tick cadence (every video frame?
every audio-driver callback?) is not established from this function's
body alone -- it is simply called once at the end of SpuVmInit in
this unit, with its own logic implying a recurring caller elsewhere.

## Proposed field names

This function is the best-evidenced site for several of this cluster's
shared globals, but per the ownership rule (and this round's
call-graph-contention note on the broadcast: `libsnd_vm_vol_ut_key_ut_keyv.c`, bravo's
live unit this round, references nearly all of them) none are applied
here -- proposing for the head to apply once no runner is live on
`libsnd_vm_vol_ut_key_ut_keyv`/`_l`/`_j`/`_j_c`/`_k`/`_p`:

- `D_8008EA26` -> `gSelectedVoice` ("currently selected channel" scratch,
  already documented `volatile`, read back via a plain `u8 *` cast --
  see SpuVmKeyOff's own report for why that specific cast matters).
- `spuVmMaxVoice` -> `gVoiceCount` ("loop bound for a small table of active
  objects", consistently the upper bound of every per-voice loop in this
  unit and its siblings).
- `_svm_okof1`/`_svm_okof2` -> `gVoiceEnableMaskLo`/`gVoiceEnableMaskHi`
  (OR'd with a per-voice bit when releasing a voice, split low/high 16
  across the 0..0x1F channel space).
- `_svm_okon1`/`_svm_okon2` -> `gVoiceActiveMaskLo`/`gVoiceActiveMaskHi`
  (AND-NOT'd with the enable mask above -- the actual SPU key bitmask
  pair, per `vmNoiseOn2`'s report).
- `_svm_sreg_dirty` -> `gVoiceFlags` (per-voice byte OR'd with 3 or 4 by
  several functions in this cluster; never fully decoded here).
- `D_8008D9A3` -> `gVoiceState` (the byte SpuVmKeyOff/SpuVmNoiseOff/
  `SpuVmAlloc` all compare against `2` for "noise voice").
- `_svm_sreg` (and this unit's two local views `SpuRegs`/`SpuRegsEdd4`)
  -> `gSpuRegs`: confirmed to be the PS1 SPU's own hardware base address
  `0x1F801C00` by `vmNoiseOn2`'s report in `libsnd_vmanager`.

Posted to the broadcast this round; see also SpuVmInit.md's own
`## Proposed field names`.

## NON_MATCHING body promoted, round 67

Placed in `src/libsnd_vmanager.c` under `#ifdef NON_MATCHING`, `INCLUDE_ASM`
kept in `#else`. Used the CURRENT best preserved body (round 48 echo,
236/241 words, 5 short) rather than the older superseded 237/241 one kept
at the end of this report for reference. All of its supporting
declarations (the `_svm_envx_hist*` pair, `Rec34HalfU2`,
`Rec16D7F0Wide`, `Rec16DAD4C`, `SpuSetNoiseVoice`, the `SetAutoVol`/
`SetAutoPan` externs, `D_8008D7F6`) are new to the unit and were kept
local to this function's `#ifdef` block, per CLAUDE.md's rule against
adding to a shared header; everything else it touches (`D_8008D9A3`,
`_svm_sreg`, `_svm_sreg_dirty`, `_svm_okof1`/`64`, `_svm_okon1`/`22C`,
`_svm_orev1`/`234`, `_svm_sreg_buf`, `D_8008D7F4`, `spuVmMaxVoice`,
`_svm_auto_kof_mode`, `D_8008D9A4`, `D_8008D9B0`) was
already declared earlier in the unit and needed no change.
`./build-and-verify.sh` green (zero bytes changed) and
`tools/check-nonmatching.sh libsnd_vmanager` green.

## Track 2 (round 86, 2026-09-26, alpha)

The per-field symbols this report names (`D_8008D988`..`D_8008D9BA` at a 0x34 stride, and the twelve game names over +0x1C..+0x33 that earlier rounds gave `D_8008D9A4`..`D_8008D9BA`) are ONE Sony table: libsnd/vmanager.o (disc 3.5) bss puts `_svm_voice` at +0x198 of the block anchored at 0x8008D7F0, so `_svm_voice` = 0x8008D988, 24 voices x 0x34 = 0x4E0 bytes, ending exactly at `_svm_envx_ptr`. The symbols file now carries `_svm_voice` (size:0x4E0); the record type is `include/SvmData.h` (fields by offset only, Sony's rule). Field map: +0x00 `unk00` (was `D_8008D988`), +0x02 `unk02` (`D_8008D98A`), +0x04 `unk04` (`D_8008D98C`), +0x06 `unk06` (`D_8008D98E`), +0x08 `unk08` (`D_8008D990`), +0x0A `unk0A` (`D_8008D992`), +0x0C `unk0C` (`D_8008D994`), +0x0E `unk0E` (`D_8008D996`), +0x10 `unk10` (`D_8008D998`), +0x12 `unk12` (`D_8008D99A`), +0x14 `unk14` (`D_8008D99C`), +0x16 `unk16` (`D_8008D99E`), +0x18 `unk18` (`D_8008D9A0`), +0x1B `unk1B` (`D_8008D9A3`), +0x1C..+0x26 `unk1C`..`unk26` (the SeAutoVol/SetAutoVol ramp: active, step, interval, countdown, accum, limit; `D_8008D9A4`..`D_8008D9AE`), +0x28..+0x32 `unk28`..`unk32` (the SeAutoPan/SetAutoPan ramp, same order; `D_8008D9B0`..`D_8008D9BA`). Preserved bodies in this report keep the per-address `D_` spellings, which still link (except `D_8008D988`, which is now `_svm_voice` itself) (splat keeps them as auto-symbols, since the table lies past the global segment's vram range and splat does not fold them into `_svm_voice`).

The NON_MATCHING body reads `_svm_voice[i].unk1C/unk28/unk1B` and walks +0x06 with its u16 0x34-stride pointer, now started at `&_svm_voice[0].unk06`; normalized disassembly identical. Tried and rejected: walking a `SvmVoice *` and reading `->unk06` (one instruction longer, mnemonic ratio vs retail 0.876 -> 0.874).

**_svm_sreg_buf / _svm_sreg_dirty (same round).** `D_8008D7F0` (0x180 bytes, 24 voices x 0x10, halfwords at +0x0..+0xA spelled `D_8008D7F0`..`D_8008D7FA` by splat) is Sony's `_svm_sreg_buf` and `D_8008D970` (24 bytes) is `_svm_sreg_dirty`: libsnd/vmanager.o bss +0x000 and +0x180, anchored at 0x8008D7F0. Both are in the symbols file; the record type is `SvmSreg` in `include/SvmData.h` (fields by offset). The NON_MATCHING body now walks `SvmSreg *p7F0 = _svm_sreg_buf` and reads `_svm_sreg_buf[i].unk4/unk6` (the `Rec16D7F0Wide`/`Rec16D7F4` views of `D_8008D7F0`/`D_8008D7F4`/`D_8008D7F6` are gone); normalized disassembly identical.

## Track 2 (round 86, 2026-09-26, head)

Game names off Sony data (`tools/sonydata.py`; symbols-file `identified`
comments hold the evidence): `gDisableVoiceStarveScan` is libsnd/vm_g.o's
`_svm_auto_kof_mode` (every aligned access, discs 3.0 to 3.6). In SpuVmInit,
`gMasterVolL/R` and `D_8008E258/5C` were four loose words of the pinned
`_svm_rattr`, libspu's `SpuReverbAttr` (LIBSPU.H): `.mask`, `.mode`,
`.depth.left/right` = 0x3FFF, i.e. reverb depth, not a master volume; the C now
writes the fields through a local mirror of the SDK struct. `gSpuMallocArea`
is `_ss_spu_vm_rec + 8` (vmanager.o bss, 3.5 layout), spelled `D_8008DEB0`
because splat names only addresses some asm references. Byte-identical.

## Track 6 (round 96, charlie)

Round 96 (charlie, track 6) moved `src/libsnd_vmanager.c` onto Sony's headers (`<libsnd.h>`, `<libspu.h>`) and Sony's types; zero bytes changed, whole-image SHA1 green, NON_MATCHING bodies compile. The preserved body's `Rec16DAD4C` (0x10-byte records over `_svm_sreg`) is `SpuVoiceRegs`, one element of `SpuRegs.voice[24]` (the SPU register block at 0x1F801C00; `ObjDAD4` renamed `SpuRegs`). The activity walk reads `envx` (+0xC, the current envelope level); the dirty copy-out writes `voice[i].volL/volR/pitch/addr/adsr1/adsr2`; the tail's byte-offset stores are `keyOff[0..1]` (+0x18C, from _svm_okof1/64), `keyOn[0..1]` (+0x188, from _svm_okon1/22C) and `reverbOn[0..1]` (+0x198, from _svm_orev1/234). `SpuSetNoiseVoice` now comes from `<libspu.h>`. `Rec34HalfU2` (the 0x34-stride walk over `_svm_voice[].unk06`) is kept: it is the walk's own element type.

## History (moved from src/libsnd_vmanager.c, comments pass)

The comment above this function's NON_MATCHING body in src/libsnd_vmanager.c read:

> NON_MATCHING: 236/241 words, 5 words short. Residue: retail's
> unconditional `move a2,v0`/`li t0,1`/`move a3,a0` do-while-style setup
> before the count>0 loop, which this C's `for` does not reproduce (a
> literal do-while conversion was tried and regressed hard, 236/241,
> 93/241 raw match -> 233/241, 16/241 -- reverted), plus cosmetic
> s0/s1 register-color swaps in phases 2-6. Round 48's frame-padding
> lever and an `s32 count` fix (drop a spurious andi mask) already
> applied; a permuter search (round 37) plateaued at 1578/2276 with no
> candidate reaching zero (docs/match-reports/SpuVmFlush.md).
> Hand-derived.
