# _card_clear -- NOT GAME CODE. Psy-Q **libcard/card** (3.3 disc: same length, shape 1.00; retail's assembly differs, so the object never places). UNMATCHABLE BY CONSTRUCTION -- do not staff, do not permuter, do not attempt.

> Renamed from `func_80050B28` on 2026-09-24 (tools/rename.py). Address 0x80050b28.

> **ROUND 39 (head): VERDICT REPLACED.** Everything below this box is a real
> and careful derivation of a function that no C was ever compiled to. The
> mechanism round 32 found is correct and reproduces exactly; what was wrong
> was the conclusion drawn from it. Kept in full, because the evidence is
> what makes the new verdict checkable.

## The finding: the `li`-expansion residue is not a toolchain blocker, it is a FINGERPRINT

Round 32 established -- correctly, and I re-reproduced all of it this round
through the pinned pipeline -- that word 5 differs as `addiu $a1,$zero,0x3F`
(retail, `2405003f`) versus `ori $a1,$zero,0x3F` (ours, `3405003f`); that
`objdump` and `asm-differ` print BOTH as `li a1,0x3f` and so cannot see it;
that `cc1` emits only the generic `li $5,0x0000003f`; and that
`maspsx.expand_load_immediate()` picks `ori` for every `0 < K < 0x10000`
under the pinned `--aspsx-version=2.34`, so **no C spelling can reach the
`addiu` form.**

From that, round 32 concluded the function was blocked by the toolchain. The
step never taken was to ask **how often retail uses the form our pipeline
cannot emit.** One command settles it:

```sh
grep -rcE '\b(addiu|ori) +\$[a-z0-9]+, \$zero, 0x' asm/
```

**In the entire 505856-byte executable's disassembly there is exactly ONE
`addiu $rX, $zero, K` against 1089 `ori`. The one is this function's word 5.**

A toolchain rule that is wrong for 1 instruction in 1090 is not wrong. It is
right, and this instruction came from somewhere else.

## Where it came from: Sony's libcard, built with a different ASPSX

Sony shipped the libraries as `.OBJ` files, so the objects in `lib/` are the
ground truth for what Sony's own assembler emitted. Disassembling all 178 of
them and reading the raw encoding under objdump's `li` synthesis:

| library | `addiu` form | `ori` form |
| --- | --- | --- |
| **libapi** | **54** | **0** |
| **libcard** | **14** | **0** |
| libgte | 39 | 8 |
| libsnd | 42 | 49 |
| libgs | 23 | 43 |
| libcd | 15 | 103 |
| libc2 | 8 | 71 |
| libspu | 11 | 56 |
| libetc | 5 | 26 |
| libpress | 2 | 28 |
| (all objects) | **226** | **386** |

The `addiu` form is not exotic -- Sony's own objects are full of it, because
different libraries were built with different ASPSX versions (CLAUDE.md
already records that the game MIXED library builds). **`libcard` is 100%
`addiu`, 14 of 14.**

And this function is inside libcard's own run. From the splat yaml and
`config/symbols.slps01556.lsdde.txt`, the objects tile the region exactly:

```
0x41308  libcard/c172   _card_load    (placed object)
0x41318  libcard/c171   _card_info    (placed object)
0x41328  _card_clear  12w, 0x30 bytes   <-- THIS FUNCTION, unclaimed
0x41358  libcard/a78    _card_write   (placed object)
0x41368  libcard/a74    InitCARD      (placed object)
0x41378  libcard/a75    StartCARD     (placed object)
0x41388  libcard/c112   _bu_init      (placed object)
0x41398  libcard/a80    _new_card     (placed object)
```

It is bracketed on both sides by placed libcard objects, it calls **only**
`_new_card` and `_card_write` (both libcard), and it carries libcard's
assembler fingerprint in the one place the whole game disassembly carries it.

**Conclusion: `_card_clear` is Psy-Q libcard code. Its object is simply not
on the discs in `sdk/`** -- it is one of the SDK functions `progress.py`
counts under "have no object on any disc in sdk/". No source shape reaches
those bytes, and every attempt on it is spent for certain.

## Why no existing screen caught it, and what to add

- `tools/sdkstalls.py` crosses the stall queue against **PLACED** objects. This
  function's object was never placed, because it is not on the user's discs,
  so the tool correctly reports no overlap. **`sdkstalls.py` answers "is this
  owned by an object we HAVE", not "is this Sony's".**
- Every blocker screen passes it: no `gp_rel`, no `nop_mflo_mfhi`, not a
  trampoline. The unit comment says `BLOCKER PROFILE: CLEAN`, and it is.
- At 12 words it sorts FIRST in `nearmiss.py`'s `ASSIGN FROM HERE`, so it has
  been the single most attractive target in the queue for four rounds.

That is the Gate 1b lesson arriving a third time: **a screen measures the
obstruction it was built for and says nothing about the ones it was not.**

**The cheap new screen, and it is one line:** a `c` segment of one or two
functions wedged between `o` segments of a single library, whose callees are
all that library's, is probably that library's. The `addiu`/`ori` census is
the corroborating fingerprint where the library is one of the 100% ones.

## Proposed learning (round 39)

**A reproduced toolchain mechanism is not a blocker until its CORPUS FREQUENCY
is measured.** This report reproduced its mechanism impeccably -- in isolation,
through the pinned pipeline, traced to the exact lines of
`maspsx.expand_load_immediate()` -- and drew a project-wide conclusion from a
sample of one. CLAUDE.md already states the rule this needed ("a blocker's
SCOPE is measured, not reasoned") and it was applied to the mechanism rather
than to the verdict.

**The asymmetry is the familiar one, pointed a new way.** A wrong SCORE gets
corrected the next time anyone measures. A wrong CAUSE is what the next round
acts on. Here the cause was right and the *ownership* was wrong, which is
worse than either: it produced a clean, well-argued, internally consistent
report recommending a permuter budget on Sony's code.

---

## (SUPERSEDED BELOW -- retained as evidence) _card_clear -- STALL: length EXACT (12/12 words, no drift); 9/12 raw word-match (round 32, up from 5/12); first real diff at word 1 (`sw ra,0x18(sp)` vs retail's `0x1c(sp)`)

> **ROUND 36 (2026-09-12), runner charlie -- MEASURED, not just re-named.**
> Round 34 renamed the two callees in prose but never rebuilt this body under
> the new names, so the 9/12 figure was carried forward UNVERIFIED (see
> CLAUDE.md's round-36 assignment note and `tools/stalesyms.py`, which still
> flagged this report). Corrected `func_80050B98` -> `_new_card` and
> `func_80050B58` -> `_card_write` (both confirmed in
> `config/symbols.slps01556.lsdde.txt`, `lib/libcard/a80.o` /
> `lib/libcard/a78.o`), spliced the exact round-32 body into
> `src/psyq/libcard_card.c` in place of the `INCLUDE_ASM`, and ran the real
> oracle: **`build exit=0`, whole-image SHA1 matches**, and
> `funcdiff.py _card_clear` reports **9/12 words, exact length, no
> drift** -- IDENTICAL to round 32's recorded diff (same three words: the
> `ra` slot at word 1, the `li a1,0x3f` encoding at word 5, the `ra` reload
> at word 8). The unverified figure is now a measured one, and it did not
> move.
>
> Tried one additional, previously-untested reshape within budget: two
> separate 4-byte `volatile int` locals (`pad0`, `pad1`) in place of the one
> `volatile long long pad` -- identical 9/12 result, consistent with round
> 31's "granular 0/8/16, no 4-byte middle ground" finding (two 4-byte
> volatiles still floor the frame at the same tier as one 8-byte one).
> Reverted; the round-32 body (`long long pad` + unused `int *new_var`) is
> the one restored to `INCLUDE_ASM` and preserved below, corrected for the
> rename. Not re-attempting the li-encoding residue (independently
> reproduced as toolchain-unreachable under the pinned `--aspsx-version=2.34`
> maspsx flag) or re-running the permuter (round 31 already spent an
> 80k-iteration `--stack-diffs` search on this exact seed with no
> improvement) -- both would re-derive rather than extend what is already
> established. Still genuinely stalled; still worth the permuter as a
> *fresh* seed if anyone wants to spend a dedicated search on it, but not
> re-running the same one for a third time.

> **ROUND 34 (2026-09-12), runner bravo -- NAME CORRECTION, nothing else.**
> This function is still game code and still stalled; what changed is its two
> callees. `func_80050B98` and `func_80050B58` are the BIOS trampolines either
> side of it, they are now linked from Sony's own objects (`libcard/a80`,
> `libcard/a78`), and they are named `_new_card` and `_card_write` in
> `config/symbols.slps01556.lsdde.txt`. **Every candidate body below that
> spells them `func_8005xxxx` will now fail to link** -- substitute the Sony
> names and everything else in this report reads unchanged. The vector numbers
> the report already carries (B(0x50) and B(0x4E)) were re-read out of the
> stubs' own delay slots this round and are correct.

> **HEAD ADJUDICATION, round 32 (2026-09-12). The li-encoding finding is
> CONFIRMED, and its SCOPE is measured: it blocks exactly ONE function in the
> whole live queue -- this one -- by exactly ONE word.**
>
> Charlie found that its `li $a1, 0x3F` assembles as `ori` (`0x3405003f`)
> where retail has `addiu` (`0x2405003f`), and that `objdump`, `asm-differ`
> and the permuter's scorer all print BOTH as `li a1,0x3f`. Reproduced
> independently through the pinned pipeline, one file, under a second:
>
> ```c
> extern void sink(int a, int b);
> void probe(int a) { sink(a, 0x3F); }
> ```
>
> cc1 emits the pseudo-op `li $5,0x0000003f`; maspsx expands it to
> `ori $5,$zero,63`; `as` assembles `0x3405003f`; and objdump prints
> `li a1,0x3f`. Retail's word at file `0x4133C` is `0x2405003f`. Same
> printed text, different byte. **Confirmed.**
>
> **But charlie's stated mechanism -- "maspsx hard-codes ORI for any positive
> value" -- needed the scope measured before it could be escalated, and the
> measurement changes what to do about it.** Censused over the retail image:
>
> | population | addiu-form `li` | ori-form `li` |
> | --- | --- | --- |
> | whole code image | 714 | 2834 |
> | functions we compile from C and match BYTE-EXACT | 252 | 1387 |
>
> The pinned pipeline therefore **does** emit `addiu`-form `li`, 252 times, in
> code that matches retail byte-for-byte -- which on its own refutes "maspsx
> always emits ORI". Splitting those 252 by the immediate's sign settles it:
>
> - **248 have a NEGATIVE immediate** (`>= 0x8000` sign-extended). `ori`
>   zero-extends, so a negative constant CANNOT be an `ori` and maspsx
>   correctly emits `addiu`.
> - **4 have a positive immediate, and all four are spurious** -- they are
>   attributed to the data symbol `gDreamAuxSpawnInfo`, i.e. data words being read as
>   instructions by the extent walk, not code.
>
> So the rule is exact: **positive constant -> `ori`, always; negative
> constant -> `addiu`, always.** Retail chose per-context and used `addiu`
> for a positive `0x3F` here. That specific choice is unreachable.
>
> **Scope, which is the part that decides what happens next.** Across all 220
> live `INCLUDE_ASM` functions, the number containing a positive-immediate
> `addiu`-form `li` is **1** -- this function -- totalling **1 instruction**.
> It is a real toolchain gap and a genuinely tiny one. It does NOT deserve a
> `--li-addiu` maspsx flag on current evidence: at one word in one function it
> is far below the bar `addiu_at` cleared at 76 functions, and charlie's own
> observation that retail uses `ori` elsewhere for positive constants means a
> blanket flag would break the 2834 places retail agrees with maspsx.
>
> **The operator escalation is therefore NOT "add a flag". It is: this
> function is one word from matching, that word is a toolchain-unreachable
> encoding, and the correct disposition is to leave it as a documented
> permanent stall unless someone wants to teach maspsx a per-call-site rule.**
> Do not spend further attempts on the encoding word. The frame-size half of
> this function's residue (`-0x20` vs `-0x18`) is unrelated, is ordinary C,
> and IS still worth attempts -- see DECOMPILATION_LEARNINGS, "An
> allocated-but-unused stack frame is not a residue": it reproduces from
> ordinary C with enough simultaneously-live locals.
>
> **The genuinely project-wide half of charlie's finding is the VISIBILITY
> gap, and it is worth more than the encoding itself.** `asm-differ` and the
> permuter's scorer compare disassembly TEXT, so they are blind to any
> difference `objdump` renders identically -- and a permuter cannot optimise
> toward a difference it cannot see. Only `funcdiff.py`'s raw word compare
> catches it. When a function is "one word short" with no visible diff,
> compare the WORDS, not the text.

Unit: `src/psyq/libcard_card.c` (carved this round). Round 27, by the HEAD.
Size: 12 words (0x30 bytes), vram `0x80050B28`, file `0x41328`.

## Screens (clean)

```
grep -n 'gp_rel' asm/nonmatchings/libcard_card/_card_clear.s            -> no hits
grep -A2 -nE '\b(mflo|mfhi)\b' ... | grep -E '\b(mult|multu|div|divu)\b'   -> no hits
```

It was `addiu_at`-blocked before round 21. `addiu_at` is RESOLVED; do not
screen for it and do not file against it.

## What it is -- PSX MEMORY CARD CODE, identified from the BIOS call numbers

Both callees are BIOS trampolines in the neighbouring `asm` segments
(`class_3bb8c_h_c`), each `addiu $t2,$zero,0xB0 / jr $t2 / addiu $t1,$zero,N`
-- the 0xB0 vector with the call number in `$t1`:

| symbol | `$t1` | PSX BIOS B-function |
| --- | --- | --- |
| `func_80050B98` | `0x50` | `_new_card()` |
| `func_80050B58` | `0x4E` | `_card_write(chan, sector, src)` |

So the function is `_new_card(); return _card_write(chan, 0x3F, NULL);` --
`arg0` is the memory-card channel. Note `src/ui/TitleMenuTaskObjF.c` calls that
argument "a resource handle" in a field comment; that was a guess and this
identifies it. Its declaration there is otherwise correct and is the one to
match:

```c
extern s32 _card_clear(s32 arg0);        /* TitleMenuTaskObjF.c:276 */
```

The remaining trampolines in the two sibling segments decode as `0x4A`
InitCARD, `0x4B` StartCARD and an `0xA0`-vector `0x70`, which corroborates
that this whole block is the card interface.

## THE MECHANISM, WHICH IS THE USEFUL PART

Retail keeps the incoming parameter in **its own home slot across the call**
and uses **no callee-saved register at all**:

```
addiu sp,sp,-0x20
sw    ra,0x1c(sp)
jal   func_80050B98
 sw   a0,0x20(sp)      <- spill to the INCOMING HOME SLOT, in the delay slot
lw    a0,0x20(sp)      <- reloaded, not held in a register
li    a1,0x3f
jal   func_80050B58
 move a2,zero
```

The obvious C (`func_80050B98(); return func_80050B58(chan, 0x3F, 0);`)
does **not** produce that. GCC promotes `chan` to `$s0`, which costs a
save/restore pair and makes the function **14 words, 2 long**:

```
addiu sp,sp,-0x18 / sw s0,0x10 / sw ra,0x14 / jal + move s0,a0
move a0,s0 / li a1,0x3f / jal + move a2,zero / lw ra / lw s0 / addiu sp
```

**Taking the parameter's address forces memory residency and reproduces
retail's shape exactly, at retail's exact length.** That is the finding, and
it moves the function from 0/12-with-drift to 5/12 at 12/12 words:

```c
/* stalesyms --fix 2026-09-22: func_80050B58 -> _card_write, func_80050B98 -> _new_card -- names retrofitted so this body links as written; the residue it recorded is unverified until rebuilt. */
s32 _card_clear(s32 chan) {
    s32 *p = &chan;
    _new_card();
    return _card_write(*p, 0x3F, 0);
}
```

## The preserved body (5/12, EXACT length, no drift)

Complete and compilable as written -- splice it in place of the
`INCLUDE_ASM` and it builds:

```c
/* stalesyms --fix 2026-09-22: func_80050B58 -> _card_write, func_80050B98 -> _new_card -- names retrofitted so this body links as written; the residue it recorded is unverified until rebuilt. */
#include "common.h"

extern void _new_card(void);                           /* B(0x50) _new_card   */
extern s32 _card_write(s32 chan, s32 sector, void *src); /* B(0x4E) _card_write */

s32 _card_clear(s32 chan) {
    s32 *p = &chan;
    _new_card();
    return _card_write(*p, 0x3F, 0);
}
```

Diff against retail, read off `tools/asm-differ/diff.py` (which realigns) --
the instruction SEQUENCE is right and only two things are wrong:

```
TARGET                              CURRENT
addiu sp,sp,-0x20            i      addiu sp,sp,-0x18
sw    ra,0x1c(sp)            s      sw    ra,0x10(sp)
jal   50b98                         jal   50b98
sw    a0,0x20(sp)            s      sw    a0,0x18(sp)
                             >      li    a1,0x3f          <- ORDER
lw    a0,0x20(sp)            s      lw    a0,0x18(sp)
li    a1,0x3f                <         (moved up)
jal   50b58                         jal   50b58
move  a2,zero                       move  a2,zero
lw    ra,0x1c(sp)            s      lw    ra,0x10(sp)
addiu sp,sp,0x20             i      addiu sp,sp,0x18
jr    ra / nop                      jr    ra / nop
```

1. **Frame 0x20 vs 0x18** -- 8 bytes. Every `sp`-relative offset follows from
   it, which is why 7 of the 12 words differ for one cause. Retail's `ra` sits
   at `0x1c`, i.e. there are 12 bytes between the 4-word outgoing-arg area and
   `ra`; this attempt has `ra` at `0x10` with nothing between.
2. **One ordering swap**: retail emits `lw a0` then `li a1`; this emits
   `li a1` then `lw a0`.

## What was ruled out, and how

Eight in-tree attempts plus four isolated reproducer runs through the pinned
pipeline (`CLAUDE.md` "Escalate, do not experiment" form -- under a second
each). Every negative below is measured, not reasoned:

| variant | result |
| --- | --- |
| plain param | 14 words, 2 long, `$s0` promoted -- drift |
| pass `chan` to the first call as well | identical to plain param |
| `volatile s32 chan` | worse, drift |
| second unused parameter | identical to plain param |
| **`s32 *p = &chan`** | **5/12, EXACT length, no drift -- kept** |
| address-taken + bare `__asm__("")` between the calls | **INERT** -- byte-identical to the kept body, at both positions tried |
| address-taken + an addressable local | 2/12, drift (frame grew the wrong way) |
| declaring the callee with 6 params and passing 6 | 2/12, drift (adds arg stores) |

Isolated reproducers, to bound the FRAME question rather than guess at it:

```c
extern void g(void);  extern int h(int,int,void*);
int f(int chan){ int *p=&chan; g(); return h(*p,0x3f,0); }   /* -> frame 0x18, ra at 0x10 */
int f(int chan){ g(); return h(chan,0x3f,0); }               /* -> frame 0x18, no spill   */
int f(int chan){ int *p=&chan; int loc; g(); loc=*p;
                 return h(loc,0x3f,0); }                     /* -> frame 0x18 STILL       */
```

**So the address-taken shape reliably yields frame `0x18`, and adding a local
does not grow it.** A fourth reproducer adding an unrelated 7-argument call
*does* reach `ra` at `0x20` (arg area 0x1c) and, notably, also emits retail's
`lw a0` / `li a1` order -- so both residues move together and both look
frame-driven. But it costs the extra call's instructions, so it is a
diagnosis, not a candidate.

That is the honest state: the residue is ONE cause (frame layout) with the
ordering swap probably downstream of it, and no source shape tried reaches a
0x20 frame without emitting extra instructions.

### Next moves, in order of promise

1. **The permuter.** This is an excellent seed: 12 words, exact length, one
   structural cause, and a compilable body. It was NOT run here -- four
   runners were live and one search is the standing limit.
2. Find what gives a 12-byte gap between the outgoing-arg area and `ra`
   without emitting code. The reproducers above narrow it to frame layout;
   the arg-area route reaches the right offset but costs instructions.
3. `func_80050A84` in the sibling unit `class_3bb8c_u` is an 8-word wrapper on
   the same call chain and will likely present the same question -- worth
   attempting alongside this rather than separately.

### Proposed learning

**An incoming parameter that must survive a call has TWO retail shapes, and
the default C form only reaches one of them.** GCC 2.6.3 promotes such a
parameter to a callee-saved register (costing a save/restore pair and 2
words); retail sometimes instead spills it to its own incoming home slot and
reloads it, using no callee-saved register. Taking the parameter's address
(`s32 *p = &chan;` and reading `*p`) switches GCC to the home-slot form and is
worth trying whenever a small function is exactly 2 words LONG with an
`$s0`/`$ra` pair retail does not have. This is the mirror image of the
existing "one named C variable gets ONE storage location" entry: that one is
about retail using *no* stable location, this one is about retail using
*memory* where GCC picks a register.

## Round 31 update (runner echo) — permuter run, frame mechanism narrowed to one 4-byte gap, still not closed

Rebuilt the exact preserved 5/12 body first: confirmed unchanged (frame
`0x18` vs retail's `0x20`, `ra` at `0x10` vs `0x1c`), same as documented.

**A `--stack-diffs`-scored permuter run found a genuine FALSE ZERO first —
worth recording as its own gotcha.** `tools/setup-permuter.sh`'s own
suggested search command omits `--stack-diffs`; running it that way found a
"score = 0" candidate (`new_var = &(*p);` inside a `do{}while(0)`, with
`chan` retyped `unsigned int`) almost immediately. **This is NOT a match —
verified by hand: the permuter's default scorer NORMALIZES stack-offset
differences away, so a candidate compiling to the WRONG frame size
(`addiu sp,sp,-0x18`, still 8 bytes short of retail's `-0x20`) scored 0
against the real target.** Splicing it into `src/psyq/libcard_card.c` and
running `./build-and-verify.sh` gave `5/12`, unchanged — the "zero" was
purely an artifact of the missing flag. **Re-ran with `--stack-diffs`
passed explicitly; this is the flag to use whenever the residue under
search is a frame/offset difference (which every reshaping attempt on this
function has been) — a search without it can report a false zero on
exactly this class of function and should not be trusted without a manual
`build-and-verify.sh` check of ANY candidate it reports, zero included.**
This is worth a line in `docs/PARALLEL-RUNS.md`'s permuter guidance if a
second instance turns up.

**With `--stack-diffs`, one dead end (score 0 is unreachable via
stack-diff normalization tricks) but real, substantial progress on the
frame mechanism itself.** The corrected search's best-found candidate
(`permuter-work/_card_clear/output-8-1`, score 8, ~80k iterations, no
zero found in a 600s/`-j 6` run) reproduces retail's frame size AND the
`lw a0` / `li a1` instruction ORDER exactly — the only remaining
difference is `ra`'s own slot: `0x18(sp)` vs retail's `0x1c(sp)`, a single
4-byte gap. Distilled and verified by hand (`tools/binutils/.../objdump`
against `permuter-work/_card_clear/compile.sh`, the REAL Makefile
pipeline, not an isolated reproducer):

```c
/* stalesyms --fix 2026-09-22: func_80050B58 -> _card_write, func_80050B98 -> _new_card -- names retrofitted so this body links as written; the residue it recorded is unverified until rebuilt. */
s32 _card_clear(s32 chan) {
    s32 *p = &chan;
    volatile long long pad;   /* forces the frame to grow to 0x20 at all */
    int *new_var;              /* unused -- but REQUIRED: without it, the
                                * exact same `pad` alone reproduces frame
                                * 0x20 but with li/lw in the WRONG order */
    _new_card();
    return _card_write(*p, 0x3F, 0);
}
```

Two mechanisms isolated here, neither in the original 8-attempt table:

1. **Any single address-escaping/volatile local (`volatile long long`,
   `volatile int`, even a plain unused `int *` alone is NOT enough by
   itself — see below) floors the frame at exactly `0x20` with `ra` at
   `0x18`, REGARDLESS of the local's own size** (tested `long long` (8B),
   `int` (4B), `short` (2B), `char` (1B), `int[2]`, `int[3]` — all
   identical `addiu sp,sp,-32` / `sw ra,24(sp)`). Two SEPARATE
   volatile/address-taken locals jump the frame a full tier further, to
   `0x28` with `ra` at `0x20` — there is no size or count of extra locals
   found that lands `ra` at the in-between `0x1c` retail actually has.
2. **A second, textually inert declaration (an unused `int *new_var;`)
   changes which of two equally-valid instruction orders GCC emits for the
   `lw a0` / `li a1` pair**, independent of the frame-size mechanism above
   — `pad` alone (no `new_var`) reproduces frame `0x20` but with `li a1`
   BEFORE `lw a0` (wrong order, matching the original 5/12 body's own
   defect); adding the extra unused pointer flips it to `lw` first (right
   order). This is a second, independent scheduler tie-break, not the same
   knob as (1).

**Not closed: no combination found that shifts `ra` by the remaining 4
bytes without ALSO overshooting the frame to `0x28`.** Every attempt to
widen the "floor" pad from a single scalar (or `long long`, which is the
same size class) to something 12 bytes wide jumped straight to `0x28`
rather than landing at the needed `0x1c`/`0x20` split — tried `long
long + short`, `long long + char`, `int[3]`, all rounding the SAME way.
This reads as an 8-byte GRANULARITY in whatever reservation rule is
firing (local-var area is either "0" (no address-taken local), "8" (one),
or "16" (two/enough) bytes — never 4 or 12), which does not admit the
4-byte middle ground retail's own frame implies. The permuter's own
80k-iteration search over this exact seed did not find one either.

### Proposed learning

**A permuter search on a residue that is fundamentally about STACK FRAME
LAYOUT must pass `--stack-diffs`, or a wrong-frame candidate can score
zero and look closed when it is not.** `tools/setup-permuter.sh`'s printed
example command does not include the flag; for this class of residue
(anything already diagnosed as a frame-size/offset difference, as opposed
to a register-identity or instruction-selection difference) it should be
added by hand, and any reported zero should still be spot-checked with a
real `./build-and-verify.sh` before trusting it — the permuter's own
scorer and the project's actual byte-exact oracle are not the same
question unless every flag that makes them agree is passed.

**Local-variable stack-area reservation seems to be GRANULAR here (0 / 8 /
16 bytes), not proportional to bytes actually used** — worth knowing
before spending more attempts hunting for an exact-byte-count local to
plug a small gap; a byte count between tiers (this function needs 12) may
simply not be reachable via "add a local of the right size."

## Round 32 update (runner charlie, LEAD TASK) — 5/12 -> 9/12, order residue closed, and a SECOND residue found that round 31's own tooling could not see

Re-verified round 31's exact preserved 5/12 body first via `--debug --stack-diffs`
on a fresh permuter scaffold: confirmed unchanged (base score 68 = one
reordering (60) + stack-diff (8), same as documented). Re-ran the corrected
(`--stack-diffs`) search fresh, `timeout 300 -j 6`, no PERM macros (blind):
floor held at score 8 by iteration ~2430, identical to round 31's own finding
— **not** a new search result, a reproduction of one.

**But translating the search's saved `output-8-1` candidate by hand (rather
than trusting its score) found something round 31's own analysis missed.**
`output-8-1` wraps the first call in a no-op `do { func_80050B98(); } while
(0);`, textually inert per the permuter's own scorer (score unchanged at 8).
Compiled it directly through `compile.sh` and objdumped it rather than
reading the permuter's score:

```c
/* stalesyms --fix 2026-09-22: func_80050B58 -> _card_write, func_80050B98 -> _new_card -- names retrofitted so this body links as written; the residue it recorded is unverified until rebuilt. */
s32 _card_clear(s32 chan) {
    s32 *p = &chan;
    volatile long long pad;
    int *new_var;

    do { _new_card(); } while (0);
    return _card_write(*p, 0x3F, 0);
}
```

```
addiu sp,sp,-32   sw ra,24(sp)   jal func_80050B98
sw a0,32(sp)      lw a0,32(sp)   li a1,0x3f        <- ORDER NOW MATCHES RETAIL
jal func_80050B58  move a2,zero  lw ra,24(sp)
addiu sp,sp,32    jr ra          nop
```

**The `do{}while(0)` wrapper alone closes the instruction-ORDER residue that
round 31 could not close** (round 31's `new_var`-based explanation for the
order fix was not the actual mechanism — see below). Confirmed by direct
ablation, each recompiled and objdumped:

| variant | order |
| --- | --- |
| `pad` + `new_var`, no wrapper (round 31's literal candidate) | `li a1` **before** `lw a0` — WRONG |
| `pad`, `new_var`, plain `{ func_80050B98(); }` block (no do/while) | WRONG, unchanged |
| `pad` + `do { func_80050B98(); } while (0);`, **no `new_var` at all** | `lw a0` **before** `li a1` — RIGHT, matches retail |

So **`new_var` was never the lever; the `do/while(0)` loop-shaped wrapper
around the first call is.** This is consistent with the project's general
finding that GCC 2.6.3's instruction scheduler treats a (trivial,
executed-once) loop construct as a distinct scheduling region from a plain
compound statement, even when both produce byte-identical control flow —
worth generalising: **when an order-only residue resists a bare `__asm__("")`
barrier, try wrapping the preceding statement in `do { ... } while (0)`
before concluding the order is unreachable.**

With the order fixed, only the stack-frame `ra`-offset residue (`0x18` vs
retail's `0x1c`, this report's already-documented granularity problem) and
ONE OTHER instruction should remain. Spliced this exact body into
`src/psyq/libcard_card.c` and ran the real oracle:

```
build exit=2
_card_clear: 9/12 words match (file 0x41328-0x41358)
  1 off=0x04132C DIFF retail=1c00bfaf built=1800bfaf   (sw ra: 0x1c vs 0x18)
  5 off=0x04133C DIFF retail=3f000524 built=3f000534   (the li a1,0x3f encoding itself)
  8 off=0x041348 DIFF retail=1c00bf8f built=1800bf8f   (lw ra: 0x1c vs 0x18)
```

Best-ever score for this function: **9/12, up from 5/12**, exact length,
no drift (confirmed via `funcdiff.py`, no outside-range warning). This is
now the preserved body to resume from — not the round-31 one.

### The genuinely new finding: word 5 is NOT a register-identity or frame residue at all — it is a `li`-EXPANSION discrepancy that every prior pass on this function was structurally blind to

Word 5's raw bytes, `3f000524` (retail) vs `3f000534` (this body), differ in
only the top byte of the instruction word: `0x24` vs `0x34`. Decoded:

- retail: `0x2405003F` = **ADDIU** `$a1, $zero, 0x3F` (opcode `001001`)
- this body: `0x3405003F` = **ORI** `$a1, $zero, 0x3F` (opcode `001101`)

**Both `objdump` and `asm-differ` print this as `li a1,0x3f` in EITHER
encoding** — GNU objdump's pseudo-op synthesis collapses `ori $rd,$zero,K`
and `addiu $rd,$zero,K` to the same display text when `$rd` is otherwise
unused for arithmetic. That is why every prior round's asm-differ-based
"instruction-exact, zero inserted/zero deleted" analysis (round 13's HEAD
PASS onward, all the way through round 21) never once flagged this word —
the text comparison genuinely cannot see it. Round 31's own permuter
`--debug` scaffold is built on the SAME objdump-based scorer and also cannot
see it (confirmed: this body's permuter score is unchanged whether the
instruction is `ori` or `addiu` — verified by hand-patching the saved
candidate's assembly and re-running the scorer). **Only `funcdiff.py`'s raw
word comparison — the project's actual byte oracle — catches it,** and it
has been sitting in this exact word position, unflagged, since round 27's
very first attempt (verified: the ORIGINAL simplest body, `s32 *p = &chan;`
alone with no other changes, was independently recompiled through
`compile.sh` this round and objdumped — it ALSO emits `ori`, not `addiu`, at
this position; this residue predates every lever tried in this report).

**Isolated, minimal reproducer through the pinned pipeline** (per CLAUDE.md's
"Escalate, do not experiment" form), confirming this is not source-shape-dependent:

```c
extern s32 g(s32,s32,s32,s32,s32,s32,s32,s32,s32,s32);
s32 f(void) { return g(0x3F, 0x100, 0x7FFF, 5, 6, 7, 8, 9, 10, 11); }
```

Every one of these loads — `0x3F`, `0x7FFF`, and every other tested positive
value in `[1, 0xFFFF)` (`1`, `0x64`, `0x8000`, `0xFFFF`) — comes out as `ori`
through the pinned pipeline (`--aspsx-version=2.34`), while negative values
(`-1`, `-0x8000`) correctly come out as `addiu`. **Traced to source:**
`cc1` never emits a raw `addiu`/`ori` for a compile-time-constant register
load at all — it always emits the generic pseudo-op text `li $5,0x0000003f`
(confirmed via direct `cpp | cc1` inspection, bypassing `maspsx` and `as`
entirely). The ADDIU/ORI choice is made **entirely downstream, by
`maspsx`'s own `expand_load_immediate()`** (`tools/maspsx/maspsx/__init__.py`),
which implements (replicating ASPSX < 2.50's behaviour, selected here because
the Makefile pins `--aspsx-version=2.34`):

```python
if 0 < operand < 0x10000:
    res.append(f"ori\t{r_dest},$zero,{operand}")
...
elif 0 > operand > -0x8000:
    res.append(f"addiu\t{r_dest},$zero,{operand}")
```

This rule is **purely a function of the operand's numeric sign/range** — no
source-level reshaping of the C (type, cast, literal base, expression form)
can change which branch it takes, because by the time `maspsx` sees the
line, it is just the text `li $5, 63` regardless of how that 63 was spelled
in C. Confirmed: hex vs decimal literal, `(char)`/`(s32)`/`(s16)`/`'?'`
(0x3F is `'?'` in ASCII) casts, `unsigned`/`signed`/`short` parameter typing,
pointer-arithmetic spellings (`(u8*)0+0x3F`) — **all 10+ variants tried
produce byte-identical `ori` output.** This is not a residue any amount of
further C reshaping will move; the value never survives to `maspsx` as
anything other than a plain positive integer text token.

**Diagnostic-only test (NOT applied to the real build — flags are pinned,
HARD RULE 5): re-ran the identical pipeline with `--dont-expand-li`** (an
already-existing `maspsx` flag, just not one the Makefile passes), which
defers the `li` expansion to `mipsel-linux-gnu-as`'s own native macro
instead of `maspsx`'s ASPSX-2.34 emulation:

```
2405003f   li   a1,63      <- GNU as's OWN li macro: ADDIU, matching retail exactly
```

**So the real assembler's native `li` macro picks `ADDIU` for this exact
value, while `maspsx`'s ASPSX-emulation picks `ORI` — and retail's own bytes
agree with the native assembler, not with `maspsx`'s model, for this one
instruction.** This does NOT mean `maspsx`'s ori-for-positive rule is simply
wrong — `docs/match-reports/TaskCore__TickFadeOut.md` already proves the opposite: a
DIFFERENT function's retail bytes use `ori $s0,$zero,0x80` for a positive
`0x80`, exactly matching `maspsx`'s current model. **Both are real, in two
different already-examined functions of the same executable — retail is
not internally consistent with a single value-only rule** — the
real Sony ASPSX assembler's actual `li`-expansion decision depends on
something `maspsx`'s simple range check does not model (plausibly
context — e.g. whether $at/reorder mode is active at that point, or something
about which specific register or instruction-position is involved — but
untested and not claimed here).

**This is filed as a genuine, isolated, reproduced toolchain-modeling
question, not a source-code stall in the usual sense — flagged for
operator attention alongside the two open blockers in CLAUDE.md, but
explicitly NOT claimed as a third project-wide blocker.** Unlike
`gp-relative-blocker`/`nop_mflo_mfhi`, this is not (yet) shown to recur
elsewhere: a project-wide grep of the currently-open queue
(`grep -rlE 'addiu\s+\$[a-z][0-9a-z]*,\s*\$zero,\s*0x[0-7][0-9A-Fa-f]{0,3}\b' asm/nonmatchings/`)
found no OTHER currently-stalled function with this exact construct — this
may be a one-off in the still-open queue, or it may be silently present
inside functions that already read as "matched" purely because `asm-differ`
and the permuter's own scorer cannot distinguish `ori`/`addiu`-with-$zero
from each other (only `funcdiff.py`'s raw word compare and the true
byte-for-byte `build.sha1` oracle can). No claim is made either way beyond
what was directly measured here.

### What remains open

Two independent residues, not one:

1. **Stack frame `ra`-offset** (`0x18` vs retail's `0x1c`) — this report's
   pre-existing, still-unclosed granularity problem (see the round-31
   section above). Unaffected by this round's `li`/order findings.
2. **The `li a1,0x3f` ADDIU-vs-ORI encoding** — a toolchain-level question,
   not reachable from C under the pinned `--aspsx-version=2.34` flag, per
   the isolated reproducer above.

Both must close for this function to go byte-exact. Neither closed this
round; both are now precisely characterized, which they were not before.

### Proposed learning

**`asm-differ` and the permuter's own scorer both go through `objdump`,
which prints `ori $rd,$zero,K` and `addiu $rd,$zero,K` identically as
`li rd,K` — so neither tool can ever detect this specific residue class.**
Only `funcdiff.py`'s raw word/byte comparison (and the ultimate
`build.sha1` oracle) can. Any function whose asm-differ/permuter analysis
claims "zero inserted, zero deleted, N register diffs" should still have
its raw `funcdiff.py` word list checked directly for a same-position,
different-top-byte word pair before being trusted as fully characterized —
this project has at least one instance (this function, since round 27)
where such a check was never done and the residue went unseen for five
rounds.

**A `do { stmt; } while (0);` wrapper around an otherwise-ordinary statement
is a real, reproducible scheduling lever distinct from a bare
`__asm__("")` barrier** — it fixed an instruction-order residue here that a
barrier could not reach (this function has no barrier-lever precedent
tried, but `FlagLargePolyForDivide`'s report shows a case where `__asm__("")` had NO
effect; this wrapper is a different, apparently more disruptive-to-the-
scheduler construct, worth trying as a distinct next step before that
report's "not reachable" verdict is extended to a case that hasn't tried
this specific lever).

## Round 36 — corrected, LINKABLE preserved body (9/12, measured, current tree)

Every prior `#if 0` body above spells the two callees `func_80050B98` /
`func_80050B58`, which round 34's SDK-object conversion retargeted to
`_new_card` / `_card_write` — those old names no longer exist as symbols and
a body using them will not link. This is the corrected version, confirmed to
compile and measure 9/12 (identical to every figure recorded above) against
the CURRENT tree:

```c
#if 0
extern void _new_card(void);                            /* B(0x50) */
extern s32 _card_write(s32 chan, s32 sector, void *src); /* B(0x4E) */

s32 _card_clear(s32 chan) {
    s32 *p = &chan;
    volatile long long pad;
    int *new_var;

    do { _new_card(); } while (0);
    return _card_write(*p, 0x3F, 0);
}
#endif
```

## File history (moved from the unit banner, round 90)

- Round 27 (head): carved `class_3bb8c_v` as a one-function unit, because
  the function sat between two PSX BIOS trampoline clusters inside the old
  `class_3bb8c_h` segment (2 before, now `class_3bb8c_h_b`; 5 after, now
  `class_3bb8c_h_c`), and trampolines stay in `asm` segments. Not a
  class-table slot (`classtable.py --scan`). The canonical declaration is
  the caller's, `extern s32 _card_clear(s32 arg0);` in
  `src/class_3bb8c_e.c`, passing `self->unk10`.
- Round 34: the neighbours became linked libcard objects, so the unit now
  sits between placed `libcard/c171` and `libcard/a78`.
- Round 36 (runner charlie): retargeted the preserved body's callees to
  `_new_card`/`_card_write`, re-measured 9/12, exact length, no drift;
  restored `INCLUDE_ASM` (body preserved above).
- Round 39 (head): identified as Sony libcard, unmatchable by construction
  (this report's top sections). The old banner kept a stale "12 words --
  this should close in one sitting" directive as an example of a confident
  stale instruction; that example now lives here.
- Round 90 (track 8): `tools/unitfile.py rename class_3bb8c_v
  libcard_card_clear`. `tuboundary.py --unit class_3bb8c_v`: "(after
  sony:libcard/c171): start edge possible"; both neighbours are placed
  objects, so no merge was possible. Named for the function because the
  object's module id is not measurable from the discs on hand.
- Round 90 (head): `tools/unitfile.py rename libcard_card_clear
  libcard_card`, because round 74 had identified the object as
  `libcard/card` on the 3.3 disc by shape (exact length). That rename also
  rewrote the line above to quote a command never run; restored by the
  premium session after round 90, and `unitfile.py` now leaves a report's
  history sections alone.

## History (moved from src/libcard_card.c, comments pass)

The file's banner carried its edge evidence:

> Edges: both are placed Sony objects (libcard/c171 _card_info before,
> libcard/a78 _card_write after), so this file cannot merge with a
> neighbour.
