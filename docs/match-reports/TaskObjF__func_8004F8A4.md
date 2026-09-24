# TaskObjF__func_8004F8A4 -- MATCHED 77/77, round 75 (delta). Levers: return type `void` (no return value), and three ordinary leaf `slot7C` calls instead of the shared `dispatch` function-pointer local + `goto call_it`.

REVISITED, round 75: MATCHED 77/77 in 3 builds, whole image `OK: build matches retail`; names/types used (the return TYPE was the first lever).

## Round 75 (delta): revisit

**Baseline first.** The round-73 `#ifdef NON_MATCHING` body, made live
exactly as it stood, measured **63/77 at 0x140/0x134 (80 words, 3 too
long), `insertions 3 / deletions 3`**, 12 positional skeleton diffs.

**What retail says.** The recorded residue was "retail reuses Validate's
0 as the return value; the build re-materialises `move $v0,zero` plus a
skip-jump". Read the other way round, retail's `beqz $v0,<epilogue>`
lands on `lw $ra` with nothing setting `$v0` and the `jalr` path also
falls into the epilogue without touching `$v0` -- which is exactly what a
`void` function compiles to. The function is reached only through a
class table (`asm/data/76DC8.data.s`, `.word TaskObjF__func_8004F8A4`),
so no caller constrains its return type.

| build | change | score | ins/del |
| --- | --- | --- | --- |
| 1 | NON_MATCHING body as-is (`s32`, `return dispatch(...)` / `return 0`) | 63/77 (80 words) | 3 / 3 |
| 2 | `void`, `dispatch(self, code);` with no returns | 73/77 (77 words, length exact) | 0 / 0; 4 words: `move $a0,$s0` in each leaf's delay slot in retail, once in the shared `jalr` slot in the build |
| 3 | drop `dispatch`/labels: `if/else if/else` with a `slot7C(self, code)` call at each leaf (the `m = self->methods` cache kept in the third) | **77/77**, whole image OK | 0 / 0 |

Build 2's residue told the rest: retail sets `$a0 = self` separately in
each predecessor of the shared `jalr`, so the argument setup belongs to
three separate calls that GCC cross-jumped down to their common tail
(`nop; jalr`), not to one call reached through a function-pointer local.
The round-20/-27 finding that "inlining every call is 4 words too long"
was measured with the `s32` return, whose per-leaf `return` stopped the
cross-jump; with `void` the three calls merge exactly as retail does.

### Proposed learning

- **A return value retail never sets is a `void` function.** If an early
  exit branches straight to the epilogue with whatever `$v0` a call left,
  and the main path falls from the last `jalr` into the epilogue too,
  type the function `void` before reading the residue as "GCC re-
  materialises a known-zero return". Check callers first (here: only a
  class table).
- **Per-predecessor `$a0` setup before a shared `jalr` = separate calls
  cross-jumped**, not one call through a shared function-pointer local.
  The shared-local shape puts the argument setup once, in the `jalr`
  delay slot.

## Final C (matched)

```c
void TaskObjF__func_8004F8A4(TaskObjF *self, s32 a1, s32 a2, s32 a3, u8 a5, s32 a6, s32 a7, s32 a8) {
    s32 code;
    TaskObjFMethods *m;

    self->unk40 = a1;
    self->unk44 = a2;
    self->unk48 = a3;
    self->opMode = 2;
    self->unk4C = a5;
    self->unk50 = a6;
    self->unk54 = a7;
    self->unk58 = a8;
    if (TaskObjF__Validate(self)) {
        if (self->methods->slot54(self, 0, a1) != 0) {
            code = 0xA;
            if (self->statusCode == code) {
                code = 0x11;
            } else if (self->statusCode == 0x11) {
                code = 0xB;
            }
            self->methods->slot7C(self, code);
        } else if (!self->methods->slot60(self, a5, a8)) {
            self->methods->slot7C(self, 9);
        } else {
            m = self->methods;
            code = 0x11;
            if (self->statusCode == code) {
                code = 0xB;
            }
            m->slot7C(self, code);
        }
    }
}
```

(History below is the pre-round-75 stall record, kept unchanged.)

## Previous title (superseded)

TaskObjF__func_8004F8A4 -- STALL. Length: 3 words TOO LONG (80/77, 0x140/0x134). Word-match: 63/77. First real diff: file 0x40158 / vram 0x8004F958 (delay-slot fill: retail `move $a0,$s0`, built `nop`).

NON_MATCHING body promoted, round 73

> Renamed from `func_8004F8A4` on 2026-09-20 (tools/rename.py). Address 0x8004f8a4.

**Unit:** class_3bb8c_f · **Size:** 77 words (0x134) · **Status:** STALL —
tail-merge / early-return scheduling residue. Previously 36/77 (0x130, 1 word
short); ROUND 27 (delta) raised this to 63/77 (0x140, 3 words long -- the
opposite direction) with a genuine structural fix, then hit a SECOND,
narrower residue class. Title rebuilt this round per `tools/nearmiss.py`'s
`[UNRANKABLE-TITLE]` flag -- the previous title carried no length verdict,
word-match or first-diff location.

> **ROUND 37 (delta): re-verified by rebuilding this EXACT preserved body
> before trusting its recorded score, per this round's own directive.**
> Splicing it back in and rebuilding reproduces **63/77 words at
> 0x140/0x134 (3 words too long)** exactly, first real diff at file
> 0x40158 / vram 0x8004F958 (delay-slot fill: retail `move $a0,$s0`, built
> `nop`) -- matching this report's own title precisely, no drift in the
> claim. This function had never been permuter-searched before this
> round (per the round's own corpus census). Scaffolded with
> `tools/setup-permuter.sh`; `--debug --stack-diffs` base score was
> **760** (`Stack Differences: 0`, `Branch Differences: 0`, `Register
> Differences: 0 (5)`, `Reorderings: 1 (60)`, `Insertions: 5 (100)`,
> `Deletions: 2 (100)`) -- net 3 extra words (5 insertions, 2 deletions),
> matching this function's own "3 words too long" residue exactly and
> confirming the scaffold measures the right thing.
>
> Ran `timeout 900 permuter.py -j 6 --stop-on-zero --best-only
> --stack-diffs` in the background (after the TaskObjF__TryWriteMemcardSaveFile search
> released the machine -- one search at a time, per this round's rule).
> **68,682 iterations**, floor reached was **490** (seen once; 520 was
> the recurring local floor, seen 74 times), **no zero found**. Compile
> errors climbed steadily through the run (up to 1730 per generation by
> the end). Same detached-launch caveat as TaskObjF__TryWriteMemcardSaveFile's search: `rc`
> could not be read back directly (not this shell's own child); the log's
> mid-line cutoff timing is consistent with the 900s bound firing, not
> reported as a captured `rc=124`. Not closed in this budget. Not
> upgraded to "permuter-exhausted." Restored to `INCLUDE_ASM`, not left
> live.

> **ROUND 20 (charlie): real attempt given per the coordinator's staffing
> note, using a fast isolated reproducer (per this report's own suggestion
> below).** Set up a standalone `cpp|cc1|maspsx|as` pipeline over a minimal
> stub (`TaskObjF`/`TaskObjFMethods` cut down to only the fields/slots this
> function touches) -- confirmed the reproducer is FAITHFUL first (it
> reproduces the exact same 0x130 compiled length as the real project
> build for the previously-preserved body), then iterated in well under a
> second per variant instead of a full project rebuild.
>
> **Found and fixed one genuine, independent bug in the preserved body's
> OWN description, not just a prose slip this time.** Rebuilding the
> EXACT preserved body verbatim in the real project (byte-for-byte
> diffed against this file's own code block first, to rule out a
> transcription error -- there was none) showed `funcdiff.py` reporting
> only **33/77**, not simply "0x130, 1 word short" as this report's
> header implied -- i.e. the header's LENGTH claim was accurate but the
> WORD-MATCH count was never actually re-measured against a real build
> after the preserved body was written down (or the measurement did not
> survive into this file). Direct instruction-level inspection (both via
> the isolated reproducer and a real build) showed the nested-`if`/`goto`
> form of the "top" 3-way `unk28` check compiles to `beq` at the FIRST
> comparison, the OPPOSITE of retail's `bne` at the same spot --
> contradicting attempt 4's own claim ("branch-polarity fixes ... were
> necessary to get variant 3's word count as close as it is"). Rewriting
> just that one block as a flat `if (self->unk28 == 0xA) { code = 0x11; }
> else if (self->unk28 == 0x11) { code = 0xB; }` (still feeding the SAME
> shared `dispatch` function-pointer variable + `goto call_it;` the rest
> of the preserved body already uses) reproduces retail's exact `bne`
> polarity and target for BOTH comparisons in that block, and raises the
> real score from 33/77 to **36/77** -- still 0x130 total length, so this
> is a genuine 3-word improvement within the same byte-count residue,
> not a rediscovery of the same number. **The preserved body below has
> been updated to this corrected form**; the OLD nested-`if`/`tmp`-variable
> form is not preserved separately since it is strictly worse on every
> axis the new form is measured on.
>
> **The remaining 1-word gap did NOT close, and three additional
> structural variants (beyond the original report's 5) confirm it is the
> same "block-layout choice" class already named below, not something
> new.** Tried in the fast reproducer: (a) the corrected flat `if`/`else
> if` in place of the old nested form -- 0x130, no length change,
> polarity now correct (this is the form kept); (b) the SAME logic
> rewritten as an outer `if (slot54(...) != 0) { <top case, own dispatch
> assignment> } else { <slot60_path, own dispatch assignment> }`
> (no `goto slot60_path;`, matching a natural if/else instead of an early
> goto) -- BYTE-IDENTICAL output to (a), confirming GCC 2.6.3's choice of
> which block (`top_dispatch`'s fetch vs `slot60_path`'s code) gets
> physically placed first is invariant to this phrasing; (c) a fully
> flat, direct transcription with `return self->methods->slot7C(self,
> code);` written out separately at each of the 3 remaining leaves (no
> shared `dispatch` variable, no `goto` beyond the necessary control
> flow) -- 0x144 in isolation (confirms attempt 1's "no sharing at all"
> finding independently, scaled to the reproducer's own baseline). No
> variant found a source shape that makes GCC place the `top_dispatch`
> fetch block physically ADJACENT to the 3-way check (where retail has
> it) rather than after `slot60_path`'s code (where every one of my
> variants puts it) while STILL sharing the final call. **This is
> confirmed, independently, as the class this report already named**:
> GCC 2.6.3's tail-merge block-layout choice for a "shared call, 3
> independent fetches" shape is not steerable by if/else-vs-goto
> phrasing or by source block order, distinct from both the
> register-identity and commutative-operand-order classes documented
> elsewhere this round. Not the same class as those two -- reported as
> its own thing, per the coordinator's framing, rather than folded into
> either.

## What it does

`s32 TaskObjF__func_8004F8A4(TaskObjF *self, s32 a1, s32 a2, s32 a3, u8 a5, s32 a6,
s32 a7, s32 a8)`. A constructor-ish setup: stores the first three
parameters plus four stack args into `self->unk40..unk58` (one of them a
byte field, `unk4C`), sets the state tag `self->unk24 = 2`, then — only if
`TaskObjF__Validate(self)` (this unit's own validation gate, matched
separately) succeeds — dispatches `self->methods->slot54(self, 0, a1)`.
On success, picks one of three error codes (0xA/0xB/0x11-ish logic against
`self->unk28`) and dispatches `self->methods->slot7C(self, code)`. On
`slot54` failure, dispatches `self->methods->slot60(self, a5, a8)`
instead, similarly picking a code (9, or 0xB/0x11) before the same
`slot7C` dispatch. Returns 0 if `TaskObjF__Validate` itself failed, otherwise
whatever `slot7C` returns.

**Every branch target, every field write, and every arithmetic/comparison
op were derived directly from the disassembly and are settled** — this is
not a logic or CFG miss. The entire residue is which of several
equivalent ways to reach the shared final `self->methods->slot7C(self,
code)` call retail's own compiler chose.

## The residue

Retail's tail dispatch is **not** one single shared fetch+call (contrast
`TaskObjF__Validate` and `TaskObjF__Notify`, both matched, where the whole
multi-predecessor tail collapses to one `lw;lw;jalr` triple). Instead
retail has **three independent `self->methods->slot7C` FETCHES** (one for
the `unk28`-check "top" merge point reached by all 3 of its own
sub-cases, one for the `slot60`-returned-0 case, one for the
`slot60`-returned-nonzero case), and only the LAST TWO INSTRUCTIONS
(`nop; jalr $v0`) are a single physical, shared site reached by two
explicit jumps plus one fallthrough. This partial sharing (share the CALL,
not the FETCH) is a materially different shape from every other
multi-predecessor tail in this unit, and none of the reshaping levers that
worked elsewhere reproduced it exactly.

## What was tried (order matters; each is a distinct lever)

1. **Direct transcription**, each `return self->methods->slot7C(self,
   code);` written out separately at each of the 5 leaves — GCC did NOT
   cross-jump-merge these into anything: 0x194 (100 bytes too long, no
   sharing at all).
2. **One shared `goto dispatch; ... dispatch: self->methods->slot7C(self,
   code); return 0;`** (the `TaskObjF__Validate`/`TaskObjF__Notify` lever) — fully
   shares the fetch AND the call across all 5 predecessors. 0x12C (8 bytes
   / 2 words short) — undershoots because retail does NOT share this much.
3. **A local function-pointer variable** (`s32 (*dispatch)(TaskObjF*,
   s32); ... dispatch = self->methods->slot7C; goto call_it; ... call_it:
   return dispatch(self, code);`), assigned separately by the "top" merge
   point and by each of the two `slot60`-branch leaves (3 assignment
   sites, 1 shared call) — 0x130 (4 bytes / 1 word short). This is the
   BEST result reached, and structurally the closest to retail's own
   shape (3 fetches, 1 shared call).
4. **Branch-polarity fixes within the "top" 3-way `unk28` check**, to
   match retail's own `bne`/`bne` direction (checking `!= 0xA` first, not
   `== 0xA` first) exactly — this was necessary to get variant 3's word
   count as close as it is, but did not close the remaining 1-word gap on
   its own; without it, variant 3's own structure sat at correct SIZE but
   with several branch-encoding words differing outside the top section.
5. **Removing the intermediate `top_dispatch` label** so all three "top"
   sub-cases assign `dispatch` and `goto call_it` directly (no shared
   fetch even among themselves) — 0x138 (4 bytes / 1 word TOO LONG this
   time, since now 5 independent fetches feed 1 call, one too many
   relative to retail's 3). Confirms retail's fetch count is exactly 3,
   not 5, not 1.

## What was not tried, and why

Splitting the "top" 3-way check so its OWN natural GCC-computed fallthrough
predecessor (rather than the label `top_dispatch`) is the one physically
adjacent to the shared call site, instead of letting GCC's own block
scheduler decide where to place a label with 3 incoming `goto`s. Attempt 4
established that GCC 2.6.3, when a label has multiple `goto` predecessors,
schedules that label's block adjacent to its OWN SUCCESSOR (here,
`call_it`) rather than adjacent to whichever predecessor is textually
closest — this looks like a real, general scheduling behavior of this
compiler version rather than something a further C reshape inside THIS
function can override, since attempts 3-5 already explored the full range
from "0 extra fetches" (variant 2) to "2 extra fetches" (variant 5) without
finding a 3-fetch shape that lands the LAST word correctly. The next thing
worth trying, if this function is revisited: an isolated `cpp|cc1|maspsx`
reproducer (per `docs/DECOMPILATION_LEARNINGS.md`'s `Class866E8__BuildFootprintSlots`
precedent) to iterate on this ONE tail-merge shape in under a second per
variant, rather than a full project rebuild per attempt — this report's 5
variants each needed a full `build-and-verify.sh` cycle.

## ROUND 27 (delta): new lever found, best raised 36/77 -> 63/77, one residue class isolated

Read per the head's own finding this round on `Obj86ED0__HandleCommand` (a shared
function-pointer local can let GCC cross-jump-merge blocks retail keeps
separate) -- but THIS function's residue turned out to be the OPPOSITE
problem: the `dispatch` local is not causing an unwanted merge, it is
this function's ONLY way to reproduce retail's genuine partial sharing
(3 independent fetches, 1 shared call). Confirmed by testing the "inline
every call site" lever directly: dropping `dispatch` and writing
`return self->methods->slot7C(self, code);` at all three leaves compiled
to **0x144 (4 words too long)** -- worse than the `dispatch`-variable
form in the OPPOSITE direction from `Obj86ED0__HandleCommand`. **The two functions
are the same LEVER (named-local-vs-inline) with opposite correct
answers**, which is exactly what the project's "measure per function,
never assume" convention predicts -- worth stating plainly since it would
be easy to over-generalize this round's other finding.

**What DID move the score**: the report's own preserved body resolves
`self->methods->slot7C` directly inside the `slot60`-returned-nonzero
tail's 2-way `unk28` check, forcing a FRESH `self->methods` reload right
at the point of use. Retail instead caches `self->methods` into a local
(`v1`, via `lw v1,0(s0)`) ONE level earlier -- right after loading
`self->unk28` for that block's own comparison, BEFORE either branch of
the 2-way check -- and reuses that cached value for the shared fetch
regardless of which branch is taken. Reproduced by scoping a
`TaskObjFMethods *m = self->methods;` local at exactly that point and
reading `m->slot7C` instead of `self->methods->slot7C` for the fetch:

```c
    slot60_path:
        if (!self->methods->slot60(self, a5, a8)) {
            code = 9;
            dispatch = self->methods->slot7C;
            goto call_it;
        }
        {
            TaskObjFMethods *m = self->methods;
            code = 0x11;
            if (self->unk28 == code) {
                code = 0xB;
            }
            dispatch = m->slot7C;
        }
```

This alone took the function from 36/77 to **63/77**, and the compiled
body now matches retail byte-for-byte all the way through the final
shared `jalr` instruction -- the ENTIRE 3-fetch/1-call dispatch structure
this report originally flagged as "not steerable" is now fully
reproduced. The remaining residue is a completely different, narrower
thing (below), isolated to the function's own early-exit tail.

**Two further attempts on the remaining 3-word-long residue, both
negative:**

1. **Unwrap the guard** (`if (!TaskObjF__Validate(self)) { return 0; }` followed
   by the rest of the body UNINDENTED, instead of `if (TaskObjF__Validate(self))
   { <body> } return 0;`) -- tried on the theory that retail's early exit
   needs to be textually adjacent to the call whose result it reuses.
   **Regressed hard, to 23/77 at 0x13C (2 words long)** -- this also
   flipped the guard's own branch polarity wrong and disturbed the
   already-matching body. Reverted immediately; the WRAPPING `if` form is
   required, not merely stylistic, for this function (opposite of what the
   "unwrap the guard" lever suggests in isolation -- another case where
   two candidate fixes can't be evaluated independently of each other).
2. **Capture `TaskObjF__Validate`'s own return value in a local and `return`
   THAT instead of a literal `0`** (`s32 ok = TaskObjF__Validate(self); ...
   return ok;`), on the theory that GCC might recognize the value is
   already sitting in the right register and skip re-materializing it.
   **Zero effect** -- byte-identical output to the literal-`0` form. GCC
   2.6.3 does not carry a value's register binding live across the entire
   intervening call-heavy body just because the C source names it.

**The isolated residue, precisely**: retail's early-exit path (`beqz
$v0,<target>` on `TaskObjF__Validate`'s own false/0 return) branches DIRECTLY
into the function's epilogue, reusing the already-0 `$v0` as the return
value with ZERO extra instructions. This build always re-materializes an
explicit `move $v0,zero` for that path, and -- because the "real" return
(`dispatch(self, code)`'s result) needs to skip PAST that materialization
to avoid clobbering its own return value -- also emits an extra `j
<epilogue>` plus a wasted delay-slot `move $a0,$s0`. Net: 3 extra words
(`move $a0,$s0` [wasted delay-slot filler] + `j` + the `move $v0,zero`
itself, positioned after -- not merged with -- the final `jalr`). Neither
attempt above found a source shape that lets GCC skip this
re-materialization. Not the same class as the (now-closed) 3-fetch/1-call
dispatch residue; flagging as its own narrower thing for whoever revisits
this function next -- an isolated `cpp|cc1|maspsx` reproducer (as ROUND 20
already used successfully for the OTHER residue) is the fast way to keep
iterating on just this tail without a full project rebuild per attempt.

## Preserved near-miss body (`#if 0`, best variant this round, 63/77 words at
0x140/0x134 — 3 words TOO LONG, address drift beyond that)

**This is ROUND 27's `self->methods` local-caching form** (see the section
above) -- superior on every axis to the previous 36/77 form: matches
retail byte-for-byte through the entire dispatch structure, with the only
remaining residue being the isolated early-exit tail described above.

```c
#if 0
s32 TaskObjF__func_8004F8A4(TaskObjF *self, s32 a1, s32 a2, s32 a3, u8 a5, s32 a6, s32 a7, s32 a8) {
    s32 code;
    s32 (*dispatch)(TaskObjF *, s32);

    self->unk40 = a1;
    self->unk44 = a2;
    self->unk48 = a3;
    self->unk24 = 2;
    self->unk4C = a5;
    self->unk50 = a6;
    self->unk54 = a7;
    self->unk58 = a8;
    if (TaskObjF__Validate(self)) {
        if (self->methods->slot54(self, 0, a1) == 0) {
            goto slot60_path;
        }
        code = 0xA;
        if (self->unk28 == code) {
            code = 0x11;
        } else if (self->unk28 == 0x11) {
            code = 0xB;
        }

    top_dispatch:
        dispatch = self->methods->slot7C;
        goto call_it;

    slot60_path:
        if (!self->methods->slot60(self, a5, a8)) {
            code = 9;
            dispatch = self->methods->slot7C;
            goto call_it;
        }
        {
            TaskObjFMethods *m = self->methods;
            code = 0x11;
            if (self->unk28 == code) {
                code = 0xB;
            }
            dispatch = m->slot7C;
        }

    call_it:
        return dispatch(self, code);
    }
    return 0;
}
#endif
```

Needs `TaskObjF`/`TaskObjFMethods` (see `TaskObjF__ForEachEvent`'s report) and a
forward declaration of `TaskObjF__Validate` (defined later in this unit's ROM
order; already present near the top of `src/class_3bb8c_f.c`).

## Proposed learning

**A multi-predecessor shared tail has (at least) three distinct
sub-shapes, not one "tail merge" lever**: (a) fully shared fetch+call
(`TaskObjF__Validate`, `TaskObjF__Notify`, this unit, both matched with a plain
`goto`), (b) fully independent, no sharing at all, and (c) shared CALL
only, with each predecessor doing its own fetch (`TaskObjF__func_8004F8A4`, this
report). GCC 2.6.3 chooses which of these three a given call site gets
based on something not yet identified from source reshaping alone.

**ROUND 27 update: for sub-shape (c), the predecessor-to-label adjacency
IS reproducible -- the missing piece was WHERE each fetch caches its own
`self->methods` pointer, not the dispatch/goto skeleton itself.** One of
this function's three fetch predecessors needed `self->methods` cached
into a local ONE level earlier than the naive per-branch reload (see the
round-27 section above) -- once that matched retail's own caching point,
the entire 3-fetch/1-call structure this note previously called
unreproduced now matches byte-for-byte. Register-saturation census is
still NOT the discriminator here (4 callee-saved registers, well under
the round-13 stall threshold of 5+).

**ROUND 27's second finding: `Obj86ED0__HandleCommand` and this function are the
SAME lever (named function-pointer local vs. inline call) with OPPOSITE
correct answers, confirmed by testing both directions on both
functions.** `Obj86ED0__HandleCommand` needed the local REMOVED (retail keeps two
guard blocks separate that a shared local caused GCC to merge).
`TaskObjF__func_8004F8A4` needs the local KEPT (retail's own partial-sharing shape
is UNREACHABLE without it -- the fully-inlined form is 4 words too long
here). Never apply either direction of this lever without testing the
specific function; "does retail merge here" is measured per call site,
not inherited from a sibling finding in the same round.

**The now-isolated early-exit residue is a separate, third finding**: an
early `return` whose value is provably already sitting in the tested
register (`TaskObjF__Validate`'s own false/0 result) still gets a fresh `move
$v0,zero` from this compiler, whether the C spells it as a literal `0` or
a captured variable holding the identical value -- GCC 2.6.3 does not
track a value's register binding live across an intervening large,
call-heavy body just because the source names it. Unlike the dispatch
residue, unwrapping the guard to place the early return textually
adjacent to the tested call made things WORSE here (regressed to 23/77),
so this one still needs a genuinely new lever, not a variant of either
lever already tried.

## Naming (round 60, track 3)

`func_8004F8A4` -> `TaskObjF__func_8004F8A4`. **Tier C placeholder**, same
reasoning as `TaskObjF__func_8004F638`: class established, concrete
operation not. This one sets `self->opMode = 2` and dispatches through
`slot54`/`slot60`/`slot7C`, all implemented by a subclass outside this
unit. STALL; preserved body unchanged by this naming pass beyond the
field renames (`unk24`/`unk28` -> `opMode`/`statusCode`) already applied
project-wide to `TaskObjF`.
