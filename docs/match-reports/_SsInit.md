# _SsInit -- MATCH (83/83, ins 0 / del 0, round 63)

> Renamed from `func_8003221C` on 2026-09-24 (tools/rename.py). Address 0x8003221c.

> **Round 97 (track 6, runner charlie): `VoiceState80090368` is gone.** It was
> never a per-voice state struct: `D_80090368` is Sony's mark-callback table
> (libsnd `_SsMarkCallback`), `SsMarkCallbackProc [32][16]` from
> `include/psyq/libsnd.h` -- the same table `ContNrpn1` (code_179d8_k) calls
> through as `D_80090368[ch][sl]`. A row of 16 four-byte function pointers is
> the 0x40-byte stride this report measured. The unit now declares it
> `extern SsMarkCallbackProc D_80090368[0x20][16];` and loop 3 stores
> `D_80090368[j][i] = NULL;` -- byte-exact, whole image green. The unit also
> takes `<libetc.h>`, `<libsnd.h>`, `<libspu.h>` now, so its local
> `GetVideoMode`, `ResetCallback` and `SpuInit` prototypes are gone (Sony's
> `long GetVideoMode(void)` replaces a local `s32` one). Bodies quoted below
> are history and keep the old spelling.

**MATCHED round 63 (runner bravo).** The round-16..20 STALL title was
"register identity across three loop regions, not fixable by reshaping
tried"; the residue really WAS register identity, and it was fixable from C
in two statements. See "ROUND 63" at the bottom for the levers. Everything
between here and there is the stall history, kept verbatim because the
negatives in it are what made the right shape findable.

Unit `libsnd_ssinit`, carved round 16 (2026-09-04). **Attempted, restored
to `INCLUDE_ASM`.** Callee-saved register screen: 1 (`$s0`) -- well under
the deprioritisation band; register count was not why this stalled (see
"Answering the head's question" below).

## What it does

The sound-system init routine `SsInit`/`SsInitHot` tail-call
into (see those reports). Calls `func_80024D10(arg0)`, then
`SpuInit()` if `arg0 == 0` else `SpuInitHot()`. Then:

1. Writes a fixed 8-halfword template (`D_8006DC5C`) into each of 24
   PSX SPU voice register blocks (`0x1F801C00`, stride `0x10` -- matches
   real hardware voice register spacing).
2. Copies 16 consecutive halfwords from `D_8006DC6C` straight to
   `0x1F801D80` onward (no repetition, unlike step 1).
3. Calls `SpuVmInit(0x18)`.
4. Zeroes the first `0x40` bytes of each of 32 `D_80090368` entries
   (stride `0x40`, confirmed by the pointer increment).
5. Initializes the whole sound-system global block this unit has been
   working all round: `VBLANK_MINUS=0x3C`, `_snd_openflag=0`, `_snd_use_vsync_cb=0`,
   `_snd_use_interrupt_id=-1`, `_snd_1per2=0`, `_snd_vsync_cb=NULL`,
   `_snd_video_mode=func_8002551C()`, `_snd_ev_flag=0`. Every one of these
   globals is already established from `_SsSeqCalledTbyT_1per2`, `_SsTrapIntrVSync`
   and `SsEnd`'s reports this round -- this function is their
   init.

## Best C reached (100% structurally correct; register allocation differs)

```c
#if 0
extern void func_80024D10(s32 arg0);
extern void SpuInit(void);
extern void SpuInitHot(void);
extern void SpuVmInit(s32 arg0);
extern s32 func_8002551C(void);
extern u16 D_8006DC5C[8];
extern u16 D_8006DC6C[0x10];
extern s32 VBLANK_MINUS;
extern s32 _snd_openflag;
extern s32 _snd_use_vsync_cb;
extern s32 _snd_use_interrupt_id;
extern s32 _snd_1per2;
extern void (*_snd_vsync_cb)(void);
extern s32 _snd_video_mode;
extern s32 _snd_ev_flag;

typedef struct {
    s32 pad0[0x10];
} VoiceState80090368;

extern VoiceState80090368 D_80090368[0x20];

void _SsInit(s32 arg0)
{
    s32 i, j;
    u16 *base;
    u16 *src;
    u16 *reg;

    func_80024D10(arg0);

    if (arg0 == 0) {
        SpuInit();
    } else {
        SpuInitHot();
    }

    reg = (u16 *)0x1F801C00;
    base = D_8006DC5C;
    for (i = 0; i < 0x18; i++) {
        j = 0;
        src = base;
        for (; j < 8; j++) {
            *reg = *src;
            src++;
            reg++;
        }
    }

    reg = (u16 *)0x1F801D80;
    i = 0;
    src = D_8006DC6C;
    for (; i < 0x10; i++) {
        *reg = *src;
        src++;
        reg++;
    }

    SpuVmInit(0x18);

    for (i = 0; i < 0x20; i++) {
        for (j = 15; j >= 0; j--) {
            D_80090368[i].pad0[j] = 0;
        }
    }

    VBLANK_MINUS = 0x3C;
    _snd_openflag = 0;
    _snd_use_vsync_cb = 0;
    _snd_use_interrupt_id = -1;
    _snd_1per2 = 0;
    _snd_vsync_cb = NULL;
    _snd_video_mode = func_8002551C();
    _snd_ev_flag = 0;
}
#endif
```

`funcdiff`: 46/83 words. **The entire epilogue tail (steps 3-5, roughly
half the function's bytes, offsets `0x22b04`-`0x22b64`) is byte-identical
to retail with zero diffs** -- confirmed by re-running `asm-differ`
scoped to just that address range. All 37 mismatched words are confined
to the three loop regions in steps 1-2, and every mismatch there is a
register rename, not a content or ordering difference: same opcodes,
same immediates, same branch targets, same instruction COUNT
(`0x14C`/83 words on both sides, no size drift once the branch-direction
fix below was applied).

## Residue: three separate loop regions, same swap pattern, resistant to seven distinct reshapes

The if/else at the top needed the `_SsSeqCalledTbyT_1per2` block-order lever
(raw `arg0` feeding `bnez` directly -- writing `if (arg0 != 0) {
SpuInitHot(); } else { SpuInit(); }` produced a `beqz` where
retail has `bnez`, plus a wrong-length inline early block, cascading
~280 bytes of address drift through the rest of the function). Flipping
to `if (arg0 == 0) { SpuInit(); } else { SpuInitHot(); }`
fixed the branch AND collapsed the drift back to zero everywhere except
the loop regions -- another confirmation this lever generalises to raw
truthy tests (see `SsEnd.md` for the sibling finding).

What's left is pure register identity. Retail's per-loop choices:

- Loop 1 (nested, 24x8): outer counter -> `$a0`, inner counter -> `$a1`,
  src pointer -> `$v1`, SPU write pointer -> `$a2`, template base
  (persists) -> `$a3`.
- Loop 2 (flat, 16x): counter -> `$a0`, src pointer -> `$v1`, SPU write
  pointer -> `$a2` (continues the role, freshly reloaded via constant,
  not incremented from loop 1's final value).
- Loop 3 (nested, 32x16, descending inner): outer counter -> `$a1`,
  inner counter -> `$a0`, element pointer -> `$v1`.

Built output permutes these -- consistently `$a1`<->`$a2` and
`$a0`<->`$v1` in loops 1-2, and correspondingly in loop 3 -- but the
CONTENT is identical throughout. Seven variations tried, all reproducing
the identical permutation or worse:

1. Baseline: reused `i`/`j`/`reg`/`src`/`base` across all three sections
   (the version shown above) -- 46/83, the best result.
2. Declaration order reversed (`i, j` before the pointers, vs. after) --
   no change at all.
3. Separate, non-reused variable names per loop section (`voice`/`slot`/
   `vsrc`/`vreg` for loop 1, `cnt2`/`src2`/`reg2` for loop 2, `entry`/
   `word` for loop 3) -- regressed badly (14/83) and changed the
   function's overall SIZE, i.e. register pressure from the extra live
   pseudos forced different codegen entirely, not just different
   allocation.
4. `do { ... } while` instead of `for` for loop 1 -- byte-identical
   output to the baseline, same 46/83, same exact diff lines. Confirms
   the loop TEST placement (top vs. bottom) isn't the lever here.
5. Explicit `j = 0; src = base;` as two separate statements at the top
   of the outer loop body (vs. relying on the inner `for`'s own init
   clause) -- this one was actually load-bearing for the STRUCTURE (see
   below), not for the register swap.
6. Hoisting `src = base` before the outer loop entirely (treating it as
   loop-invariant) -- wrong: this changes loop 1's TARGET address (the
   back-edge landing 4 bytes earlier than retail's), because retail
   genuinely re-executes the pointer reset every outer iteration. Not a
   register fix, a structural regression; superseded by #5.
7. `(u32)` vs `s32` was not separately retried here since the loop
   bounds are all small positive constants already producing `slti` on
   both sides -- no live discriminator to test.

Attempt 5's fix (explicit statement order matching retail's own
"counter reset, then pointer reset" instruction order) IS what took this
function from a wrong CFG (0/83, alternate outer-loop entry point 4
bytes into the block) to the current 46/83 with correct CFG and pure
register swap -- worth recording separately from the register-identity
question, since it's a real, reusable structural fix and not a dead end.

Per CLAUDE.md, this is squarely the banned-to-force-fix register-identity
class (a plain C form exists, this is not a GTE/COP2 case). Restored to
`INCLUDE_ASM`.

## Answering the head's question: did the register-count screen stop this?

No. The screen (`grep -oE 'sw +\$(s[0-7]|fp),' ... | wc -l`) reported 1
for this function, deep inside the "should be tractable" band, and it
was never a factor in the decision to stop. What stopped this was
running out of DISTINCT ideas for the register permutation after seven
tries that all landed on the identical swap (or regressed) -- the same
resistance-to-reshaping pattern already documented for `GetRCnt`
this round, just spread across three loop bodies instead of one. If
anything, this function is evidence the register-identity class doesn't
correlate with function size or `s`-register count at all: this one
uses only `$s0` and stalls the same way a leaf function with zero
callee-saved registers does.

### Proposed learning

1. Confirms `SsEnd`'s finding: the `_SsSeqCalledTbyT_1per2` block-order
   lever applies to ANY raw-value `if`/`else` feeding `beqz`/`bnez`
   directly, including as the very first statement of a much larger
   function, and a wrong choice there can manifest as severe whole-
   function address drift that looks unrelated to the actual cause --
   check the FIRST branch's polarity before chasing anything downstream.
2. For a loop whose body needs to reset an inner counter AND an inner
   pointer at the top of each outer iteration, writing both resets as
   explicit separate statements (matching retail's own instruction
   order: counter first, pointer second) can be structurally
   load-bearing, distinct from and prior to any register-identity
   question -- try this before concluding a loop's residue is
   register-only.
3. Splitting loop variables into separate, non-reused names per section
   is not a safe default move against a register-identity residue: here
   it changed the function's SIZE (extra register pressure), which is a
   strictly worse failure mode than a same-size permutation. Prefer
   minimal reshapes (declaration order, statement order) before
   introducing new locals.

## Round 19 verification (runner charlie)

Per the head's mid-round drift-check broadcast: rebuilt this report's
exact body and confirmed BOTH the score and the absence of drift
directly, not just by trusting the report's own "no size drift" claim.
`funcdiff.py` showed 46/83 with **no outside-range warning**, and
`objdump` on the built object confirms the function compiles to exactly
83 instructions, matching retail's length precisely. **Drift absent;
this report's recorded score survives the check.**

Also tried one round-19 axis not in the original seven: retyping the two
loop counters `i`/`j` from `s32` to `s16` (axis 1, retype-to-real-width,
since both are small bounded counters -- max value 31). Regressed badly:
14/83 with 285380 bytes of outside-range drift (a real size/shape
change, not just a register swap). Reverted. No axis from this round's
brief closes this function; confirmed stall, restored to `INCLUDE_ASM`.

## Provenance

round 16 (2026-09-04), runner delta, unit libsnd_ssinit (fresh carve,
second pass, head-directed follow-up on the big-three queue). ~8
attempts; restored to `INCLUDE_ASM` per the hard rule against register
pinning.

## ROUND 20 note (runner echo)

This unit's `GetRCnt` (this round) independently found the same
"commutative-add operand/destination-register choice, cc1-canonicalised,
not reachable from C" residue this report already documents (or, for
`_SsInit`, the same class of register-identity resistance) --
tried on a completely different pair of operands (a table-index address
computation, not a field-offset one) and confirmed via five further
reshapes (subscript-commutativity rewrite, pointer-arithmetic spelling,
operand-order reversal, declaration order, pointer width) plus a
64631-iteration permuter search with the floor unmoved. Full detail in
`GetRCnt.md`. This is now a THIRD confirmed instance of the class
in this single unit (`func_800323A8`'s original finding, plus
`GetRCnt` this round) -- worth treating as a settled project-wide
"not reachable from C" residue category, not a per-function curiosity.

Reviewed this function's own report fresh this round per the
coordinator's "re-derive from scratch" standard applied to
`FillRVectors4`/`GetRCnt`/`ResetRCnt`. No new lever found:
the extra-independently-live-temp technique that helped `GetRCnt`
was checked conceptually against this function's existing best body and
already-tried variants (see the report above) and does not present a
new, untried shape here -- the residue here is the operand-order class
itself, not a register-pressure-slack question. Not re-attempted this
round to avoid burning attempts on a lever with no new angle; still
restored to `INCLUDE_ASM`.

## ROUND 63 (runner bravo) -- REVISIT

### Step (a): the inherited body rebuilt EXACTLY, measured before any change

Rebuilt the `#if 0` body above verbatim, with **only the two stale symbol
names corrected** (`tools/stalesyms.py` flags exactly these two and nothing
else): `func_80024D10` -> `ResetCallback`, `func_8002551C` -> `GetVideoMode`.
No other character changed.

```
build exit=2, no compile-error grep hit
_SsInit: 46/83 words match (file 0x22A1C-0x22B68)
_SsInit: insertions 14 / deletions 14
```

**The score reproduces exactly (46/83, length exact, no outside-range drift).
The recorded CAUSE does not.** A pure register-identity residue is `0/0` by
definition -- `funcdiff.py`'s skeleton keeps the register fields and masks
only the immediate, so a rename is a *replacement* and contributes nothing to
ins/del. `14/14` therefore says the two instruction SEQUENCES do not align:
there is real ordering work in here, reachable from C.

Also settles the title-tag contradiction the head flagged: the `len-off` tag
is WRONG and the report body is right. Both sides are 83 words / 0x14C; there
is no size drift, in range or out.

### Step (b): what the 14/14 actually was, and the false-positive it exposes

`asm-differ` on the step-(a) build showed exactly ONE real misalignment --
three instructions rotated -- and 34 register-only replacements. Retail:

```
22a58:  move    a0,zero          <- loop-1 outer counter = 0
22a5c:  lui     a3,0x8007        <- base = D_8006DC5C
22a60:  addiu   a3,a3,%lo
```

The inherited body's `for (i = 0; i < 0x18; i++)` emits the base load first
and the counter reset third. Writing the reset as its own statement --

```c
    reg = (u16 *)0x1F801C00;
    i = 0;
    base = D_8006DC5C;
    for (; i < 0x18; i++) {
```

-- which is **the idiom the same report already used for loop 2, one loop
down** -- took 46/83 -> 48/83 and ins/del 14/14 -> 13/13.

**And 13/13 was the whole remaining figure, with `asm-differ` showing not a
single `<` or `>` marker.** Measured directly on the two 83-word skeleton
sequences: equal length, 35 elementwise differences, perfect positional
alignment, and `difflib.SequenceMatcher` nonetheless reporting one `insert`
of 3, two `delete`s of 3 and 6, and six unequal `replace` blocks.

> **NEGATIVE, and it bounds the round-62 step-(a) rule.** "Nonzero ins/del
> falsifies a register-identity title" has a FALSE-POSITIVE mode, and this
> function is a measured instance of it: a pure register permutation over a
> loop nest reads 13/13, not 0/0. The cause is `SequenceMatcher` re-anchoring
> on the instruction skeletons that recur identically in every loop (`bnez
> v0,..`, `addiu`, `lui`, `slti`), which splits one honest `replace` into
> insert/delete pairs. The rule is still worth running -- it is what made this
> round look at the instruction ORDER at all, and the look was right -- but
> **the confirmation is `asm-differ`'s `<`/`>` markers, not funcdiff's
> number.** Treat nonzero ins/del as "go read the diff", never as a verdict.
> A function with no loops and few repeated skeletons will not show this.

### Step (c): the lever that closed it -- retail SWAPS the two counters' roles

With ordering exact, all 35 remaining words were register renames, and they
were **consistent per C variable**, which is what made them readable:

| role | retail | built (step b) |
| --- | --- | --- |
| loop-1 outer counter | `$a0` | `$a2` |
| loop-1 inner counter | `$a1` | `$v1` |
| loop-2 counter | `$a0` | `$a2` |
| loop-3 OUTER counter | **`$a1`** | `$a2` |
| loop-3 INNER counter | **`$a0`** | `$v1` |
| source pointer | `$v1` | `$a0` |
| SPU write pointer | `$a2` | `$a1` |
| template base | `$a3` | `$a3` |

Read down the retail column. `$a0` and `$a1` are **the same two pseudos all
the way through** -- but in loop 3 they have traded outer for inner. The
build, which wrote loop 3 as `for (i ...) { for (j ...) }` like loops 1-2,
keeps `i` outer everywhere and so can never reproduce that. Retail's source
wrote loop 3 the other way round:

```c
    for (j = 0; j < 0x20; j++) {
        for (i = 15; i >= 0; i--) {
            D_80090368[j].pad0[i] = 0;
        }
    }
```

One build: **48/83 -> 83/83, insertions 0 / deletions 0, `build exit=0`,
whole-image SHA1 green.** No other change; no permuter search was spent.

Two further checks after the match, both still byte-exact:

- `ResetCallback` really is called with **no argument** (`int
  ResetCallback(void)`, LIBETC.H). Retail never sets `$a0` before the `jal`;
  the `move s0,a0` in the delay slot is the incoming `arg0` being saved across
  the call, which the round-16 report misread as `func_80024D10(arg0)`. The
  honest `ResetCallback();` is what is committed.
- Frame check (lever 2 from round 62), recorded because it corroborates:
  `addiu $sp,$sp,-0x18` = 0x10 argument area + `$s0` + `$ra` = **zero spill
  budget**. Every source shape here must live entirely in registers, which is
  exactly why the round-16 attempt #3 (fresh locals per loop section) could
  not work: it added pseudos to a function with no room for them, and paid in
  size (14/83) rather than in allocation. The fix was FEWER variables
  re-paired, not more.

### Where the old report was right and where it was wrong

- Right: length exact (83 words / 0x14C both sides, no drift) -- the
  `len-off` tag on the ready-jobs list is **wrong** and should be cleared.
- Right: the epilogue (steps 3-5) was already byte-identical, and the
  `if (arg0 == 0) {...} else {...}` branch-polarity lever from
  `_SsSeqCalledTbyT_1per2` was and is required.
- Right: the residue class -- it was register identity.
- Wrong: "not fixable by reshaping tried" was read as "not fixable". Seven
  reshapes were tried; none of them was the loop-3 role swap, and none of them
  looked at WHICH ROLE each hard register played per loop. The round-20 note
  then generalised this to a settled project-wide "not reachable from C"
  category on the strength of a sibling function's separate residue and a
  **permuter search that was run on `GetRCnt`, not on this function** --
  this one was never searched. A category claim inherited across four rounds,
  resting on a negative measured somewhere else.
- The two stale symbol names (`func_80024D10`, `func_8002551C`) meant the
  preserved body would not have linked as written; `tools/stalesyms.py` named
  both in one call.

### Proposed learning

1. **When a residue is pure register identity, tabulate hard register against
   ROLE per region before concluding anything.** If the same registers recur
   across separate loops in a DIFFERENT pairing, that is retail reusing a
   small set of locals with the outer/inner (or any other) roles SWAPPED in
   one region, and the fix is to permute the C loop variables to agree. It is
   a one-token edit and it moved this function 48 -> 83 in a single build,
   after four rounds of treating the same residue as unreachable. The
   discriminator against the usual advice is direction: this is FEWER
   variables re-paired. Splitting into per-region names (round 16's attempt
   #3) is the opposite move and regressed to 14/83 -- and the frame-size
   screen says why, since a zero-spill frame has no room for extra pseudos.
2. **`funcdiff.py`'s ins/del is a POINTER, not a verdict** -- see the boxed
   negative in step (b). Over a loop nest, a pure register permutation can
   read 13/13. Confirm with `asm-differ`'s `<`/`>` markers before calling a
   recorded cause falsified. (This round's step (a) still earned its keep:
   14/14 sent me to the diff, and the diff held a real 3-instruction rotation
   worth 2 words. The number was right to be suspicious of and wrong as a
   measurement of what was wrong.)
3. **A "not reachable from C" category claim must cite a negative measured on
   the function it is applied to.** The round-20 note extended
   `GetRCnt`'s 64631-iteration search to this function by analogy; this
   function had never been searched, and it did not need to be.

REVISITED, round 63: MATCHED 83/83 (ins 0 / del 0), whole-image SHA1 green;
names/types not relevant (the two stale names had to be corrected to link, but
no naming or retyping work closed it -- two statement-level source-order
changes did).

## Naming (round 78, runner alpha)

`_SsInit` is already Sony's own identified name (matched round 63) --
untouched this round. Confirmed the unit as a whole has no class
(`tools/classtable.py --scan` has no `libsnd_ssinit` entry): it is plain
Sony sound-init C sandwiched between the placed `libsnd/vm_vsu` and
`libsnd/sstable` objects.

Data this function touches: `_snd_openflag`/`_snd_ev_flag` are Sony-pinned in
`config/psyq-objects.ld` as `_snd_openflag`/`_snd_ev_flag` (same
addresses) -- proposed to the head for `rename.py` rather than applied
directly, since `_snd_ev_flag` also appears in `src/libsnd_vm_vol_ut_key_ut_keyv.c`,
outside this unit (collision rules, PARALLEL-RUNS §2.1/§2.3). Tier: not
applicable (identification, not a game name).

`D_8006DC5C`/`D_8006DC6C` (SPU voice/control register init templates) and
`D_80090368` (per-voice state array, already typed `VoiceState80090368`)
are read only by `_SsInit`, a Sony function -- left unnamed per CLAUDE.md's
"never write C for a function a Sony object owns" / "a field of a struct
only Sony functions read" rule (FINISHING-PLAN track 3, round 75
precedent: 24 libsnd variables). Not proposing game names for these.
