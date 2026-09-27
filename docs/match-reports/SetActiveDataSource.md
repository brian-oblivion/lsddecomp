# SetActiveDataSource -- MATCHED round 75 (35/35) -- lever: table entries take NO argument, plus a label+goto loop

REVISITED, round 75: MATCHED 35/35, whole image `OK: build matches retail`, 3 builds after the baseline; names/types used (the table's element type was the first lever).

## Round 75 revisit (runner delta)

**Game code check.** 0x80026CFC is not in `config/sdk-in-game.txt`, has no
`config/psyq-objects.ld` pin and no `identified` comment; it sits among this
unit's own FileResource methods, calls only game functions, and reads no strings.
Game code.

**Baseline.** The preserved body below, rebuilt live (with the
`CopyDataSourceSlots` prototype it needs, since the definition follows it):
`insertions 15 / deletions 15`, 1/35 raw, 38 words (3 long; out-of-range
drift 316350 bytes). asm-differ showed the three extra words were `$s2` for
`val` and `move a0,s0` before the `jalr` -- i.e. the built code PASSED `val`
to each table entry.

**Lever 1 -- the table entries take no argument.** Retail's `jalr $v0` has no
`$a0` set up before it (`$a0` is whatever `CopyDataSourceSlots` left). The
14 entries of `gDataSourceClientGetters` are `Get*Methods` getters (`GetVabStreamObjMethods`
is one of them by name). Typing the table `void *(*gDataSourceClientGetters[])(void)` and
calling `fn()` removed `$s2` and both moves: every register then matched and
the length became 35 (24/35 raw, insertions 4 / deletions 4). So the
round-43 REGISTER-ALLOCATION class was an arity error: `val` needed its own
callee-saved register only because the C kept it live as the next call's
argument.

**Lever 2 -- loop layout.** The remaining 4/4 was loop rotation: retail jumps
from the entry into a bottom test (`CopyFields; lw; bnez` back to `jalr`),
the build tested at the top and `j`-ed back. `while (CopyFields(val, ret),
(fn = *entry) != NULL)`, `for (;;) { ...; if (fn == NULL) break; ... }` and
`for (val = ...; ; val = fn())` all gave the same 24/35 top-tested layout. A
label+goto loop (`goto copy; next: entry++; methods = getMethods(); copy:
CopyFields(...); getMethods = *entry; if (getMethods != NULL) goto next;`)
matched 35/35 on the first build (LEARNINGS 3a, round 74's label+goto loops).

Builds: baseline (with missing prototype, discarded) + baseline + 4 variants
+ 1 renaming rebuild + 1 final. No permuter.

### Proposed learning

- **A `jalr` with no `$aN` set up before it is a zero-argument call.** When a
  report blames a "loop-carried argument" needing an extra s-register, check
  whether retail actually loads `$a0` before the indirect call. Here the
  "argument" was the previous call's return value, and the C passed it along
  only because the table was typed `void *(*)(void *)`.
- **Retail jumps into a bottom test and the test contains a call:** every
  `while`/`for` spelling came out top-tested; label+goto reproduced it
  (a second data point for 3a).

---

# (historical) SetActiveDataSource -- 3 words long (38 vs 35) -- 1/35 raw word match -- first real diff at vram 0x80026D00

> Renamed from `SetActiveDataSource` on 2026-09-18 (tools/rename.py). Address 0x80026cfc.

> **HEAD CORRECTION, round 43 (2026-09-15): THE PERMUTER NEGATIVE IN THIS
> REPORT IS AN ARTIFACT AND MUST NOT BE READ AS EVIDENCE.** The
> "150582 iterations, best score 1643 -> 900, never zero" result was produced
> by `tools/setup-permuter.sh`'s generated `compile.sh`, which hardcodes
> `MASPSX_FLAGS` independently of the Makefile and **omits round 42's
> `--gp-symbols=config/gp-symbols.txt` and `--no-nop-mflo-mfhi`**.
>
> That matters for THIS function specifically: its body reads `gActiveDataSource`,
> which **is** in `config/gp-symbols.txt`, and its retail asm carries one
> `%gp_rel` reference. Without `--gp-symbols` the permuter's baseline emits an
> absolute access where retail has a gp-relative one, so **at least one word
> could never match no matter what C the permuter produced.** The search could
> not have reached zero; it was not testing the register-allocation
> hypothesis at all. Base score, candidate scores and the iteration count are
> all measured against an unreachable target and none of them is informative.
>
> **The REGISTER-ALLOCATION class itself is NOT withdrawn** -- it rests on the
> hand-built variants and the 3-words-long / 1-of-35 funcdiff figures, which
> came from the real pinned build through `build-and-verify.sh` and are
> unaffected. What is withdrawn is only the claim that a permuter search has
> been tried and failed. **This function has, in effect, never been
> permuter-searched**, so it belongs in Gate 1b's sixth screen as
> never-searched ground rather than as exhausted ground.
>
> Independently confirmed: runner delta hit the same gap the same round on
> `TitleMenu__CycleSaveTitleColor`, diagnosed it, and hand-patched its own gitignored
> `compile.sh` -- so delta's 26500-iteration negative IS valid while this one
> is not. Two searches in one round, one trustworthy and one not, separated
> only by whether the runner happened to notice.
>
> The tool is NOT fixed here: changing `MASPSX_FLAGS` is a flag change and
> therefore an operator escalation (CLAUDE.md, "Escalate, do not experiment").
> Escalated in round 43's write-up.


**Unit:** code_171e0 · **Size:** 35 words (retail) · **Status:** MATCHED round 75 (see top); was STALLED,
class REGISTER-ALLOCATION (round 43, 2026-09-15, runner bravo) -- that class is withdrawn: the extra register came from a wrong callee arity.

## History

Never attempted before round 43 (round-2026-08-29-a/30-a filed only a
`gp_rel`-blocked stub, no derivation). Round 42 resolved `gp_rel`
project-wide. Round 43 derived the control flow correctly on the first try
(confirmed by the `gDataSourceClientGetters` array in `asm/data/5DB70.data.s` ending in a
`0x00000000` sentinel, exactly matching a "call through the table until a
NULL entry" loop) but could not reproduce retail's REGISTER ALLOCATION
across roughly a dozen structurally-equivalent rewrites plus a 150582-iteration
permuter search (bounded at 600s, `-j 8 --stack-diffs --stop-on-zero
--best-only`) that reduced the permuter's own score from 1643 to 900 but
**did not close it, and did not reach zero in that time under load** --
every candidate below 1000 relied on reading `fn` before its first
assignment (undefined behavior), not a legitimate reshape.

## What it does

Dispatches on `gActiveDataSource` to pick one of two "ret" objects
(`GetCdDriverMethods()` if `arg0 == 0x13`, else `GetVabDriverMethods()`), stores `arg0`
into `gActiveDataSource`, then walks the function-pointer table `gDataSourceClientGetters`
(14 entries + a NULL sentinel, confirmed in `asm/data/5DB70.data.s`),
calling `CopyDataSourceSlots(val, ret)` before EVERY table read (including the
first, before any table entry is even inspected), and — for every NON-NULL
entry — calling that entry as `val = entry(val)` before advancing to the
next slot and repeating. All 4 callees are cross-unit: `GetCdDriverMethods` /
`GetVabDriverMethods` are shared with `GetActiveDataSourceMethods` (see that report), and the
14 table entries are ordinary game functions elsewhere in the image.

## Residue -- this is a genuine register-allocation puzzle, not a control-flow miss

Retail's compiled loop uses exactly **2** callee-saved registers (`$s0` =
the `gDataSourceClientGetters` walk pointer, `$s1` = `ret`) plus `$ra`. The loop-carried
"call argument" value (`val`, first produced by `GetFileResourceMethods()`, then by
each table-entry call) flows **directly through `$a0`/`$v0`**, with no
callee-saved register of its own, EVEN THOUGH it is read and written at a
control-flow JOIN with two predecessors (the initial path and the loop
back-edge):

```
.L80026D3C:
  jal   GetFileResourceMethods
   addu $s1, $v0, $zero      ; s1 = ret (from the EARLIER branch's return,
                              ;   in the delay slot -- BEFORE this call runs)
  j     .L80026D58
   addu $a0, $v0, $zero      ; a0 = GetFileResourceMethods()'s return (val, initial)
.L80026D4C:                  ; loop back-edge target
  jalr  $v0
   addiu $s0, $s0, 0x4       ; entry++ (delay slot)
  addu  $a0, $v0, $zero      ; a0 = the just-called entry's return (val, new)
.L80026D58:                  ; JOIN -- CopyDataSourceSlots call site, 2 predecessors
  jal   CopyDataSourceSlots
   addu $a1, $s1, $zero
  lw    $v0, 0x0($s0)
  bnez  $v0, .L80026D4C
```

Every structurally-equivalent C rewrite tried in round 43 (a `for(;;)` with
`break`, an explicit `goto`-based rewrite matching this exact CFG 1:1, the
same with the formal parameter `arg0` reused as the `val` slot to try to
pin it to `$a0`'s "home" register, dropping the named `fn` variable and
calling through `*entry` directly, and typing `val`/`ret` as
`FileResource *` instead of `void *`) makes this project's pinned GCC
2.6.3 allocate a THIRD callee-saved register (`$s2`) for `val` and round-trip
it through TWO extra `move` instructions per production/consumption instead
of flowing straight through `$a0`/`$v0` -- consistently **38 words (3 over)**
regardless of which of these was tried. One rewrite (producing `val` in the
`for(...)` loop's own increment clause, i.e.
`for (val = GetFileResourceMethods(); ; val = fn(val)) { ... }`) got closest, at
**38 words** with the extra register shuffle moved to a slightly different
point in the schedule -- still not zero.

**The permuter search (150582 iterations, 600s wall-clock, 8 workers) did not
close this and did not find any candidate near zero.** Its best-scoring
candidates (900 and below) all shared the same defect: they moved
`entry = gDataSourceClientGetters;` inside the loop body and called `fn(val)` as the
ARGUMENT expression to `CopyDataSourceSlots` before `fn` is ever assigned on the
first iteration -- reading an uninitialized function pointer. That is
undefined behavior, not a legitimate source reshape, and it does not
qualify as a permuter zero (it never reached zero at all). Per
CLAUDE.md/MATCHING-GUIDE.md: **not closed in 150582 iterations under load**,
not "permuter-exhausted" -- the search space is much larger than what 600s
covers and a longer or differently-seeded run might still find something.

## What this is not

- **Not a `gp_rel` residue.** `gActiveDataSource`'s own load/store already matches
  (`sw $a0, %gp_rel(gActiveDataSource)($gp)` reproduces correctly); the divergence
  is entirely in which registers get allocated in the walk loop.
- **Not a control-flow miss.** The branch targets, the loop shape (fall
  straight into the shared call site before ever testing the first table
  entry), and the table-walk/NULL-sentinel termination all reproduce
  exactly; only the register-to-value binding around the loop's carried
  argument differs.
- **Not fixable by `register T v asm("$N")` or an operand constraint** —
  CLAUDE.md bans exactly this kind of register-identity fix, and this is
  squarely that case (register IDENTITY differs, not instruction order), so
  it is not attempted here.

## Preserved best-effort body (38/35 words, 3 words long)

```c
extern void *gDataSourceClientGetters[];

void CopyDataSourceSlots(FileResource *dst, FileResource *src);

void SetActiveDataSource(s32 arg0) {
    void *ret;
    void *val;
    void *(*fn)(void *);
    void **entry;

    gActiveDataSource = arg0;
    if (arg0 == 0x13) {
        ret = GetCdDriverMethods();
    } else {
        ret = GetVabDriverMethods();
    }
    entry = gDataSourceClientGetters;
    for (val = GetFileResourceMethods(); ; val = fn(val)) {
        CopyDataSourceSlots(val, ret);
        fn = *entry;
        if (fn == NULL) {
            break;
        }
        entry++;
    }
}
```

(`extern s32 gActiveDataSource;`, `extern void *GetVabDriverMethods(void);` and
`extern void *GetCdDriverMethods(void);` are declared once earlier in this file,
above `GetActiveDataSourceMethods`.)

## Proposed learning

**A loop-carried call ARGUMENT (not accumulator, not index) that changes
every iteration and is produced/consumed exactly once per iteration, at a
control-flow join with two predecessors, is a harder class for this pinned
GCC 2.6.3 than the "constant-argument do-while" idiom that closed
`SetActiveDataSourceDriverMode` in this same unit on the first try.** The difference: when
the loop's call arguments never change across iterations (`SetActiveDataSourceDriverMode`),
each argument gets its own stable callee-saved register once and is done.
When the argument itself IS the previous call's return value
(`SetActiveDataSource`), this compiler's register allocator — at least across
every C shape tried here — insists on giving it a permanent register too
(3 total: walk pointer, loop-invariant second argument, AND the changing
first argument) rather than retail's 2 (walk pointer, loop-invariant second
argument only, with the changing first argument flowing through
`$a0`/`$v0` directly across the join). If another runner or a future round
hits the identical shape (a `D_XXXXXXXX[]`-table walk that threads a
call's own return value back in as the next call's argument), this report's
permuter seed (`permuter-work/SetActiveDataSource`, gitignored but reproducible
from this report's preserved body via `tools/setup-permuter.sh`) and its
150582-iteration ceiling are the starting point, not a re-derivation from
scratch.

## Naming

Round 52 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `SetActiveDataSource` | `SetActiveDataSource` | B (STALL -- named without matching) |

**Evidence.** Stores the new mode into `gActiveDataSource`, resolves the
newly-active source's vtable (the same `GetCdDriverMethods`/`GetVabDriverMethods`
pair `GetActiveDataSourceMethods` uses), then walks a table of 14 callbacks,
calling `CopyDataSourceSlots(val, ret)` before each and threading each
non-NULL entry's return value into the next call. The control flow and every
callee are fully confirmed (only the register allocation in the walk loop is
unmatched, per the report's own REGISTER-ALLOCATION class) -- solid enough
ground to name the operation ("install a new active data source, and refresh
every registered client with it") even though the function itself is not
byte-exact. Per the brief, named without attempting to match it.

**Global renamed alongside it.** `gActiveDataSource` -> `gActiveDataSource` (tier B):
this function's own `gActiveDataSource = arg0;` write is the clearest evidence for
what the global holds -- a mode tag whose two observed values (`0x13`,
`0x23`) are exactly the header words of the two sibling classes it selects
between (`gCdDriverMethods` and `gVabDriverMethods`; confirmed by `tools/classtable.py`).
Every other function in this unit that reads it forwards to one sibling's
real implementation or the other's fallback, which is where the whole
`ActiveDataSource` naming family comes from.

## History moved from src/code_171e0.c (round 99, charlie, track 7)

The function comment's last sentence was matching history. It is now a
`MATCHING:` line. Moved here unchanged:

> The label+goto loop is retail's layout (jump into a bottom test); every
> while/for spelling tried came out top-tested.

Round 99 also renamed the parameter `arg0` to `source`.
