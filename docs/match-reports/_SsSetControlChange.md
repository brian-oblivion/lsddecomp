# _SsSetControlChange -- MATCHED (round 69, runner bravo): 200/200, byte-exact, whole-image SHA1 green

> Renamed from `func_80034690` on 2026-09-24 (tools/rename.py). Address 0x80034690.

**REVISITED, round 69: MATCHED; names/types used** (a callee parameter
type and a case-local's width -- no field or function renamed).

Preserved body rebuilt first, exactly as the `#ifdef NON_MATCHING` block
gave it: compiled length 195 words (`nm` distance 0x558 -> 0x864 in
`build/src/code_179d8_k.c.o`), 5 SHORT, as the title said. funcdiff's
first line on that build: `insertions 12 / deletions 12`, positional
skeleton diffs 184, 13/200 raw (window drifted, 270851 bytes outside).

### The lead checked first: return types in the merge chain

Every call in the shared tail is `rec->unk88 = ReadDeltaValue(a0, a1)`
(`s32`) in every arm; the `j .L80034984` return-arm calls (`ContDataEntry`,
`ContPortamento`, `ContNrpn1`, `ContNrpn2`, `ContRpn1`,
`ContRpn2`, `ContResetAll`) are all `void` and all merge into the
same epilogue in both builds; `SpuVmDamperOn/Off` and `SsUtSetReverbDepth`
are `void` and merge into the same `.L8003496C` entry in both builds. So
the chains were type-uniform, and the return-type lead DID NOT APPLY.
Reading the diff again showed the gap was not a tail merge at all.

### What the gap really was: a caller-side PARAMETER width (5 words)

Retail's cases 7/10/11 build `packed` from the sign-extended a0/a1
(`sll/sra` into `$s1`/`$s2`), `or` them, and pass the result straight
to `func_80030980` with NO narrowing. After the call they hand the
same widened registers to `ReadDeltaValue` with two `move`s. The
preserved body's local prototype said `func_80030980(s16 packed, ...)`
(and `s16 packed` in the body), so every one of those cases narrowed
the `or` result and then re-widened a0/a1 from scratch for the tail
(4 words), and case 11 could jump into the default arm's widening.
The callee's own report (`func_80030980.md`) and its asm read `a0` as a
full word (`andi 0xFF` / `andi 0xFF00`), so the first parameter is `s32`.
Retyping it (local prototype only; the callee is still `INCLUDE_ASM`, no
other caller in the tree) and writing `s32 packed`: **195 -> 200 words,
length exact, 190/200, ins/del 2/2.** The "missing 7th callee-saved
register" was the widened a0 kept across the call; it appeared on its own.

### The last 10 words: where `offset` lives

Retail loads `rec->unk12` into `$t0` before the dispatch and each case
copies it into its own register (`addu $s0,$t0,$zero`, `addu $v0,...`).
With `u8 offset` used directly, the byte got one callee-saved register for
the whole switch and each case re-zero-extended it (`andi s0,s0,0xff`).
A case-local `u16 o = offset;` (`s16` also works; `s32` and `u8` do not --
those still coalesce or still `andi`) gives the retail shape, and case 10
also needs `packed` computed BEFORE its `o`/`blk` (retail widens first).
Mixed sweep: case-10 copy + reorder alone 198/200; `s32` copies in 7/11
197/200; `u16` in all three **200/200**.

Builds this session on this function: 16. No permuter search.

### Proposed learning

A caller's LOCAL prototype that narrows a parameter the callee reads as a
full word (`s16` where the callee masks with `0xFF00`) costs a
`sll/sra` pair on the argument AND stops CSE from reusing the widened
inputs for later calls, which shows up as "N words short, one fewer
callee-saved register" -- exactly the look of a tail-merge or
register-identity stall. Before accepting either verdict, check each
parameter width of every prototype the function calls against the
callee's own reading, not only its return type (3f). Separately: a `u8`
local read in several switch arms keeps one callee-saved register
switch-wide and costs an `andi` per arm; a per-arm `u16` copy is
retail's `lbu $t0` + per-arm `move` shape.

---

## History (superseded -- the stall verdict below is retired by the match above)

Old title: _SsSetControlChange -- STALL: length 5 words SHORT (compiled 195/200); raw word-match DELIBERATELY NOT QUOTED (see below -- a length gap voids it); first real diff is the length gap itself

**5 words SHORT: compiled length 195/200 words.** Measured directly from
`build/src/code_179d8_k.c.o` (`objdump -d`, symbol-to-symbol distance),
not from `tools/funcdiff.py` -- once compiled length differs from
retail's, funcdiff's window comparison drifts (269348 bytes outside the
per-function range) and its word-match figure stops meaning anything for
this specific function. Length was cross-checked case-by-case against the
hand-disassembled retail body (below) rather than trusted from a single
tool reading.

## What it does

The unit's Control-Change dispatcher (`jtbl_80010CF0`, one of the three
switch tables `code_179d8_k` owns). Reads one data byte from the event
stream (the CC *value*) via `rec->unk4`, then routes on `a2` -- the CC
*number* -- through a dense `switch (0..121)` that GCC lowers to that
jump table. The controller numbers with dedicated handling are exactly
the standard MIDI CC assignments:

| CC# | meaning | callee |
| --- | --- | --- |
| 0 | Bank Select MSB | (stores into `rec->unk4C` directly) |
| 6 | Data Entry MSB | `ContDataEntry` |
| 7 | Volume | `func_80030980` (+ writes a 16-bit field) |
| 10 | Pan | `func_80030980` (+ writes a byte field) |
| 11 | Expression | `func_800307F0` then `func_80030980` |
| 64 | Sustain | `func_80036518` / `func_800363FC` (threshold 0x40) |
| 65 | Portamento | `ContPortamento` |
| 91 | Reverb Depth | `func_80036118` |
| 98/99 | NRPN LSB/MSB | `ContNrpn1` / `ContNrpn2` |
| 100/101 | RPN LSB/MSB | `ContRpn1` / `ContRpn2` |
| 121 | Reset All Controllers | `ContResetAll` |

This cross-check against real MIDI semantics is strong independent
confirmation the struct/dispatch reading is right, not just a numerology
coincidence.

**New parameter/field usage, no struct changes**: this function is the
first in the unit to read `(u8 *)rec + rec->unk12 + 0x2C` / `+0x17` /
`(u8*)rec + rec->unk12*2 + 0x4E` as *write* targets rather than only
reads -- same convention already established by `NoteOn`'s and
`ContPortamento`'s own comments (raw pointer arithmetic, not named fields,
since the base address is only known at runtime).

Cross-unit prototypes added (local guesses, kept in this .c only, per
project convention): `func_800363FC(void)` and `func_80036118(s32, s32)`
(both matched in `code_179d8_f.c`), `func_800307F0(s16, s16, s32) -> s32`
(matched in `code_179d8_j.c`), and `func_80030980(s16 packed, s16 note,
u8 vol, s32 arg3, s32 arg4)` -- still `INCLUDE_ASM` in `code_179d8_j.c`,
so this is this call site's own reading: a 5th argument spills to
`0x10($sp)`, alongside the "packed = (slot<<8)|channel" first-argument
idiom this file's siblings already use (`NoteOn`, `SetPitchBend`).

## Block-order / case-order lever (same class as GetSeqData, and it worked)

Exactly like `GetSeqData`, GCC 2.6.3 lays out this switch's non-default
case BODIES in ASCENDING case-value order (0, 6, 7, 10, 11, 64, 65, 91,
98, 99, 100, 101, 121) even though the compiled *dispatch* is a computed
jump through the table, not a compare chain -- writing the `switch`
statement's cases in that same ascending order (matching, unsurprisingly,
the MIDI CC numbers' natural order) was necessary to get the jump table
itself right and reproduced retail's per-case body layout.

**A second lever, specific to this function: which cases `return`
immediately vs. `break` into the shared tail is NOT just about which ones
skip the final `ReadDeltaValue` call -- it changes whether each case does
its OWN full a0/a1 re-widening before jumping to the shared call, or
whether it can only partially reuse the shared tail.** Cases 0, 7, 10 and
11 all reach the exact same final `rec->unk88 = ReadDeltaValue(a0, a1);
return;`, and retail duplicates that line's compiled effect (four words
of `sll`/`sra` widening plus a direct jump to the shared `jal`) in EVERY
one of them rather than routing them through the `default` case's own
copy of the same widening. Writing these as `break;` (relying on the
switch's implicit shared post-switch code) produced a function that was
18 words short, because GCC then unified ALL non-return arms (0, 7, 10,
11, 64, 91, default) onto ONE shared widening+call+store block instead of
duplicating it for 0/7/10/11. Rewriting cases 0, 7 and 10 with the
tail-call inlined explicitly (`rec->unk88 = ReadDeltaValue(a0, a1); return;`
written out in each case, matching what 65/98/99/100/101/121/6 already do
structurally) recovered 13 of those 18 words -- cases 0, 7 and 10 now
compile to a byte-exact match against retail (case-by-case counts
verified: preamble 43w, case0 6w, case6 8w, case7 20w, case10 19w, case64
11w, case65 8w, case91 5w, case98/99/100/101 8w each, case121 7w, shared
default/combine/epilogue tail 18w -- every one of those equals retail's
own count exactly).

## The residue that would not move: case 11

Case 11 (Expression) is the ONE case that, even written the same way as 0/7/10
(explicit `rec->unk88 = ReadDeltaValue(a0, a1); return;`), still does not
reproduce retail's full duplicate widening. It compiles 3 words short: it
jumps into the MIDDLE of the shared `default` tail (reusing that block's
`sra a0`/`sll a1`/`sra a1` instructions after supplying only the initial
`sll a0` itself in its own branch-delay slot) rather than doing all four
widening instructions itself and jumping straight to the shared `jal`, the
way retail -- and this function's OWN cases 7 and 10 -- do. Case 11 is
the one arm with TWO calls before the tail (`func_800307F0` then
`func_80030980`, vs. one call in cases 7/10), which is the only structural
difference that stands out, but it did not turn out to be steerable:

**Reshapes tried, none of which changed the compiled size or the tail
target by even one word** (all confirmed via
`objdump -d build/src/code_179d8_k.c.o | grep -n '<func_'` distance, not
funcdiff, since the function-level drift makes funcdiff's number
meaningless mid-experiment):

1. Moving the `packed = (a1 << 8) | a0` computation to AFTER the
   `func_800307F0` call (matching retail's instruction order exactly,
   since GCC's own scheduler is free to reorder pure-arithmetic
   expressions with no side effects regardless of source position).
2. Introducing explicit named locals `s16 wch = a0, wsl = a1;` reused
   both for building `packed` and for the final `ReadDeltaValue(wch,
   wsl)` call, to encourage the allocator to keep one canonical widened
   copy alive across both calls (matching what retail's `s1`/`s2` reuse
   looks like at the assembly level).
3. Removing all the intermediate named locals entirely (`packed`, `wide`,
   `wch`, `wsl`), inlining every expression directly into the two call
   sites, to reduce artificial register pressure from block-scoped C89
   locals that this old compiler may pool over the whole function rather
   than per nested block.

All three produced byte-IDENTICAL object code (same address for the next
symbol, same total size). This is the CLAUDE.md-documented
register-identity/tail-merge-choice STALL pattern: reshaping did not move
it, so it is not banned-fix territory to keep pulling on, it is a stall.

**The 5-word gap therefore has two independently-measured components**:
2 words from one fewer callee-saved register overall (this build uses
`$s0`-`$s5`, six; retail additionally keeps the CC *value* byte in a
persistent `$s6` across the whole function, never reloading it -- this
build's allocator apparently didn't need a 7th register at all, given
case 11's cheaper tail), and 3 words from case 11 specifically not
duplicating its widening. The two are plausibly the SAME underlying cause
(lower overall register pressure lets the allocator get away with
sharing case 11's tail, which in turn needs one fewer permanently-live
register) rather than two unrelated residues, but that connection is a
hypothesis, not a measurement.

## Body as reached (195/200 words -- 5 short, all in case 11 / one register)

**Round 49 note (runner charlie): rewrapped in literal `#if 0`/`#endif`,
and the two callee names below CORRECTED to match `src/code_179d8_k.c`'s
actual current preserved body**, per `tools/stalesyms.py`'s finding
(relayed by the head) that this report's body sat in a plain fenced code
block, unwrapped, and had drifted from the real source: round 34's SDK
renaming retyped case 64's `func_80036518`/`func_800363FC` local guesses
into the real `SpuVmDamperOff`/`SpuVmDamperOn` Psy-Q symbols, case 91's
`func_80036118` into `SsUtSetReverbDepth`, and case 11's `func_800307F0`
into `SpuVmSetProgVol` -- `src/code_179d8_k.c` itself already carries the
corrected names (it was never broken there), only this report's copy was
stale. No behavioral change; this is a documentation fix so the next
resume starts from the real names instead of pre-round-34 guesses.

```c
#if 0
/* Forward declarations for siblings defined later in this unit's ROM order: */
extern void ContDataEntry(s16 a0, s16 a1, u8 a2);
extern void ContPortamento(s16 a0, s16 a1, s32 a2);
extern void ContNrpn1(s16 a0, s16 a1, u8 a2);
extern void ContNrpn2(s16 a0, s16 a1, u8 a2);
extern void ContRpn1(s16 a0, s16 a1, u8 a2);
extern void ContRpn2(s16 a0, s16 a1, u8 a2);
extern void ContResetAll(s16 a0, s16 a1);

void _SsSetControlChange(s16 a0, s16 a1, u8 a2)
{
    Entry90902E8 *rec = &D_800902E8[a0][a1];
    u8 *p = rec->unk4;
    u8 offset = rec->unk12;
    u8 val;

    rec->unk4 = p + 1;
    val = *p;
    switch (a2) {
    case 0:
        rec->unk4C = val;
        rec->unk88 = ReadDeltaValue(a0, a1);
        return;
    case 6:
        ContDataEntry(a0, a1, val);
        return;
    case 7: {
        u8 *blk = (u8 *)rec + offset;
        s16 packed = (a1 << 8) | a0;

        func_80030980(packed, rec->unk4C, blk[0x2C], val, blk[0x17]);
        *(s16 *)((u8 *)rec + offset * 2 + 0x4E) = val;
        rec->unk88 = ReadDeltaValue(a0, a1);
        return;
    }
    case 10: {
        u8 *blk = (u8 *)rec + offset;
        s16 packed = (a1 << 8) | a0;
        s16 wide = *(s16 *)((u8 *)rec + offset * 2 + 0x4E);

        func_80030980(packed, rec->unk4C, blk[0x2C], wide, val);
        blk[0x17] = val;
        rec->unk88 = ReadDeltaValue(a0, a1);
        return;
    }
    case 11: {
        u8 *blk = (u8 *)rec + offset;

        SpuVmSetProgVol(rec->unk4C, blk[0x2C], val);
        func_80030980((a1 << 8) | a0, rec->unk4C, blk[0x2C],
                      *(s16 *)((u8 *)rec + offset * 2 + 0x4E), blk[0x17]);
        rec->unk88 = ReadDeltaValue(a0, a1);
        return;
    }
    case 64:
        if (val < 0x40) {
            SpuVmDamperOff();
        } else {
            SpuVmDamperOn();
        }
        break;
    case 65:
        ContPortamento(a0, a1, val);
        return;
    case 91:
        SsUtSetReverbDepth(val, val);
        break;
    case 98:
        ContNrpn1(a0, a1, val);
        return;
    case 99:
        ContNrpn2(a0, a1, val);
        return;
    case 100:
        ContRpn1(a0, a1, val);
        return;
    case 101:
        ContRpn2(a0, a1, val);
        return;
    case 121:
        ContResetAll(a0, a1);
        return;
    default:
        break;
    }
    rec->unk88 = ReadDeltaValue(a0, a1);
}
#endif
```

## Round 32 update (runner bravo): re-verified; volatile negative; permuter scaffold rejected as insane

Re-verified: 195/200 compiled length (5 short), reproducing this report's own
case-by-case breakdown.

**Lever 1 (narrow `volatile`), tried on `val` (the CC value byte retail keeps
permanently live in its 7th callee-saved register, `$s6`) and NEGATIVE.**
Declaring `volatile u8 val;` was meant to test whether forcing every access
through memory would push the allocator toward giving it its own persistent
register the way retail does. It did the opposite: `volatile` forces an
actual memory access at every use rather than a register-resident value, so
the compiled function *lost* a callee-saved register (down to `$s0`-`$s4`,
5 registers, from the baseline's 6) while growing to 202 words (now 2 LONGER
than retail instead of 5 shorter) from the extra reload traffic. Reverted
immediately -- this confirms `volatile` is the wrong instrument for "keep a
value pinned in one register for a whole function"; that is a register-
pressure/lifetime question a qualifier cannot answer, unlike the load/fold
ordering questions it is documented to fix elsewhere in this project.

**`tools/decomp-permuter` scaffold REJECTED at the sanity-check step, not
searched.** Built a scaffold from this report's own preserved body and ran
`--debug --stack-diffs` before committing any search time, per
`docs/MATCHING-GUIDE.md`'s explicit instruction to sanity-check first. The
scaffold reported **base score 4982** (52 stack differences, 86 register
differences, 20 insertions, 25 deletions) against a function this report
documents as 5 words short with two independent, narrowly-scoped causes --
an order of magnitude larger and qualitatively different (nonzero stack
differences in particular, which should be at or near zero for a function
whose frame size this report never flagged as suspect) than what the
documented residue predicts. Per `docs/MATCHING-GUIDE.md`'s own worked
example of this exact check ("a scaffold that scores a different residue
than the real build is the second such instance recorded"), this scaffold is
scoring something other than this function's real residue -- most likely an
artifact of the hand-reconstructed local struct/prototype set used to make
the seed self-contained rather than a fact about the function itself. Not
searched; a bad scaffold's zero would be meaningless and its absence of a
zero would prove nothing. Left as an open task for whoever next has this
function: get the scaffold's `--debug` base score down to something
consistent with "5 words short, two independent causes" before spending any
search budget.

### Round 32 lever checklist (this function)

- **Lever 1 (narrow `volatile`)**: tried, **NEGATIVE** (actively worse --
  turns a register-resident value into a memory-resident one, the opposite
  of the desired effect, and both loses a register and gains 7 words).
- **Lever 2 (register-identity verdict is a hypothesis)**: not directly
  applicable -- this report's own verdict is "register-identity/tail-merge-
  choice", already flagged as a hypothesis rather than asserted register
  saturation, and this round found no new scratch-value axis to test it
  against.
- **Lever 3 (emission order != source order)**: not tested this round.
- **Lever 4 (permuter negative is evidence about one search)**: SHARPENED --
  this round's finding is that a permuter run can fail before the search
  even starts, at the scaffold-sanity step, and that failure mode is
  distinct from "search ran and found nothing."
- **Lever 5 (asm-differ/permuter compare text)**: not applicable; no search
  was run to compare against.

### Proposed learning

Two, both generalizing the GetSeqData block-order finding:

1. Switch-statement case-body layout following ASCENDING case-value
   order (not declaration/dispatch order) holds for jump-table-lowered
   switches too, not just compare-chain ones.
2. Whether a case duplicates a shared tail's instructions or jumps into
   the MIDDLE of another case's copy of them is a real, measured source
   of word-count drift distinct from ordinary register identity -- and,
   per this function's case 11, it can resist the same reshaping levers
   (statement reordering, named vs. inlined temporaries) that fixed the
   sibling cases 0/7/10 for the identical `return ReadDeltaValue(...)`
   tail shape. Worth a name if a second instance turns up:
   "tail-merge-choice residue."

## ROUND 35 (runner alpha): re-verified, SKIPPED after re-confirming the round-32 permuter rejection with a clean seed

Rebuilt this report's preserved body (using the current SDK-renamed callees
already reflected in `src/code_179d8_k.c` -- `SpuVmSetProgVol`,
`SpuVmDamperOn`/`Off`, `SsUtSetReverbDepth`) to reconfirm the recorded score
before spending any budget: clean build, **195 words** compiled
(`mipsel-linux-gnu-objdump` symbol-to-symbol distance on
`build/src/code_179d8_k.c.o`), matching this report's own "5 words short"
figure against retail's 200.

Round 32 rejected a permuter scaffold for this function at the sanity-check
step (base score 4982, called it "insane") but built that scaffold from a
**hand-reconstructed copy of the whole preceding source file's struct/
prototype set**, which round 35's own `ContDataEntry`/`GetMetaEvent`
scaffolding (this same round) showed is a real trap distinct from the
function's actual residue: a seed that pulls in OTHER functions' own
still-`INCLUDE_ASM` bodies (or, here, simply carries more struct fields /
prototypes than this function's own body reads) can inflate a scaffold's
score for reasons that have nothing to do with the target function.

**Built a fresh, MINIMAL seed this round** (only the fields of
`Entry90902E8` load-bearing to this function's own body, not the full
struct; only the externs this function itself calls) to rule that out.
Result: **base score 4787, `Stack Differences: 52` (nonzero)** -- close to
round 32's 4982 and, critically, **still nonzero on stack**, confirming
this is NOT a scaffold-construction artifact. The nonzero stack difference
is consistent with, not contradictory to, this report's own documented
residue: retail keeps a 7th value (`val`) permanently live in `$s6` across
the whole function, this build's allocator does not, and one fewer
callee-saved register changes which stack slots are used throughout --
exactly the kind of frame-shape difference `docs/MATCHING-GUIDE.md` says
the permuter's own scorer cannot measure meaningfully without
`--stack-diffs`, and which, once measured, disqualifies the scaffold from
being searched (a search against a scaffold whose STACK LAYOUT already
disagrees with retail cannot distinguish "found the real residue" from
"found a different way to be wrong about the frame").

**Not searched, for the same reason round 32 gave**: per
`docs/MATCHING-GUIDE.md`'s explicit instruction, a scaffold that does not
sanity-check clean is not spent against a search budget. This round adds
independent confirmation (a properly minimal seed, not merely a smaller
one) that the rejection is about THIS function's residue class (a
register-count/allocation difference, which shows up as a stack-shape
difference no source-level AST permutation is likely to fix), not about
how round 32 happened to build its scaffold.

**SKIPPED this round beyond the re-verification above.** The residue is
deeply worked (rounds 24/31/32/33) and this round's own contribution is
negative-but-informative: the permuter route is closed for this specific
residue class regardless of scaffold quality, which future rounds can take
as settled rather than re-litigating. `src/code_179d8_k.c` unchanged
(still `INCLUDE_ASM`); no commit needed for this function beyond this
report addendum.

### Round 35 lever checklist

- **Rebuild-before-trust**: done, reconfirmed 195/200 unchanged.
- **Permuter**: re-scaffolded with a minimal (not whole-file) seed,
  sanity-checked, still REJECTED (nonzero stack diff, comparable base
  score to round 32) -- not searched. Confirms round 32's rejection was
  about the residue, not the scaffold.

### Proposed learning

A permuter scaffold's rejection at the sanity-check step is worth
re-verifying with a DELIBERATELY minimal seed (only the target function's
own types/externs) before accepting it as a verdict about the function --
but when a minimal seed reproduces the same nonzero-stack-diff rejection a
larger, hand-reconstructed seed already got, that convergence is itself
evidence the rejection is about the function's real residue (a genuine
frame/register-count difference) rather than an artifact of scaffold
construction. Two independently-built scaffolds agreeing to reject is a
stronger signal than either alone.

## Round 39 update (runner alpha): re-verified; hoist-both-before-either checked, not applicable

Rebuilt the preserved near-miss body and cross-checked compiled length
directly via `objdump` symbol-to-symbol distance: `_SsSetControlChange` is
`0x30c` bytes (195 words), reproducing the title's "5 words SHORT
(195/200)" exactly. Confirmed clean against both blocker screens
(`gp_rel`, `nop_mflo_mfhi`) -- no hits, as the unit header already states.

**Hoist-both-before-either: not applicable.** The residue has two
independently-measured components, neither a swapped-load-pair shape: (1)
this build's allocator uses one fewer callee-saved register overall (6 vs
retail's 7, costing 2 words in prologue/epilogue) -- a whole-function
register-BUDGET decision, not an instruction-order one; and (2) case 11
specifically jumps into the middle of the shared `default` tail instead of
duplicating its own widening the way retail (and this function's own cases
7/10) do -- a tail-merge/duplication choice, not a load-scheduling one.
Neither component has an adjacent-load-then-later-consumer shape for the
lever to act on. Consistent with round 32/35's already-on-file permuter
scaffold REJECTION (frame-size sanity-check failure) -- no new search
attempted this round given the scaffold route is already closed for this
function and no new manual axis was found. `INCLUDE_ASM` unchanged,
`build-and-verify.sh` confirmed byte-exact.

**NON_MATCHING body promoted, round 66** (runner charlie).
