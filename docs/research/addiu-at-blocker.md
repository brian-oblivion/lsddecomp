# The `addiu_at` indexed-addressing blocker

**Status: ESCALATED, NOT TESTED. Diagnosis confirmed by the head with an
isolated reproducer and a corpus census. The obvious remedy is NOT surgical and
has the same shape as the `-G` trap — read "Why this is not a version bump"
before proposing anything.**

Found independently by two runners in two unrelated units during round
2026-08-30-a (`runner/alpha` in `DreamSys`, `runner/delta` in `Entity`), and
reproduced from scratch by the head. This is the second stall class on this
project to survive head scrutiny as a genuine toolchain issue, after
`gp-relative-blocker.md`.

## The claim

Retail reaches a **runtime-indexed global** — `sym[reg]` — by fully resolving
the symbol into `$at` and then loading at offset zero:

```
lui   $at, %hi(D_80089EAC)
addiu $at, $at, %lo(D_80089EAC)
addu  $at, $at, $v0
lb    $v0, 0x0($at)
```

The project's pinned pipeline emits the *folded* three-instruction form
instead, where `%lo` rides in the load's own displacement:

```
lui  $at, %hi(D_80089EAC)
addu $at, $at, $v0
lbu  $v0, %lo(D_80089EAC)($at)
```

One instruction shorter. As with the gp-relative blocker, the mismatch is not
contained: every function after it in the same translation unit shifts by a
word, which is why the affected functions score as low as 8/53 rather than
"one word off".

## It is not cc1's choice

cc1 emits a single generic pseudo-op and expresses no opinion about addressing:

```
sll  $4,$4,4
lbu  $2,D_80089EAC($4)
```

The expansion happens below cc1. Note `$at` is the assembler's reserved
temporary — cc1 never allocates it — which is by itself proof that this shape
is macro expansion, not code generation.

## The reproducer

```c
typedef signed char s8;
extern s8 D_80089EAC[];
s8 probe(int i) { return D_80089EAC[i * 16] - 1; }
```

```sh
tools/gcc263/cpp -Iinclude -Iinclude/psyq -undef -lang-c -nostdinc -Dmips -D__GNUC__=2 /tmp/t.c \
  | tools/gcc263/cc1 -mips1 -mcpu=3000 -quiet -G0 -O2 \
  | .venv/bin/python3 tools/maspsx/maspsx.py --aspsx-version=VERSION --dont-force-G0 --expand-div
```

| `--aspsx-version` | expansion |
| --- | --- |
| **2.34** (the project's pin) | `lui` / `addu` / `lbu %lo(sym)($at)` — folded, 3 instructions |
| 2.29 | `lui` / `addiu %lo(sym)` / `addu` / `lbu 0x0($at)` — **retail's shape**, 4 instructions |

The 2.29 output is byte-for-byte the shape in
`asm/nonmatchings/Entity/Entity__GetEventVideo.s`.

The switch is maspsx's `addiu_at` flag (`tools/maspsx/maspsx/__init__.py`
around line 974), set by `config_for_aspsx_version` in `tools/maspsx/maspsx.py`.

## The census — retail never uses the folded form

This is the measurement that makes the diagnosis solid rather than suggestive.
Counting *indexed* accesses only (an `addu $at, $at, $rN` followed immediately
by a load or store through `$at`), across every `.s` in `asm/`:

| form | occurrences |
| --- | --- |
| unfolded — `addu $at` then `0x0($at)` (the 2.29 form) | **502** |
| folded — `addu $at` then `%lo(sym)($at)` (the 2.34 form) | **0** |

Not a majority. **Every single indexed access in the executable is the
unfolded form, and there are no counterexamples.**

A caveat on an earlier, wrong count: a naive `grep` for `%lo(sym)($at)` returns
706 hits, which looks like a contradiction. Those are ordinary *non-indexed*
absolute accesses (`lui $at,%hi(sym)` + `sw $reg,%lo(sym)($at)`, two
instructions, no `addu`). `addiu_at` does not touch that form. Filter on the
`addu $at` prerequisite or the census is meaningless.

## Why this went unnoticed until now

The same reason as the gp-relative blocker: all 502 sites are still
`INCLUDE_ASM`. No currently-matched function performs a runtime-indexed global
load, so the pin has never been exercised against this construct. The first
five functions to exercise it — two in `DreamSys`, three in `Entity` — all
failed, in two units, found by two runners who had not spoken to each other.

The 502 sites span **39 files**, so this is not localised to one subsystem.

## Why this is NOT a version bump — read before proposing anything

`addiu_at` cannot be enabled on its own.

- **maspsx exposes no `--addiu-at` flag.** The CLI accepts `--aspsx-version`,
  `--dont-force-G0`, `--expand-div`, `--macro-inc`, `--dont-expand-li`,
  `--use-comm-section`, `--use-comm-for-lcomm` and some debug switches. There is
  no per-flag override for this behaviour.
- **Dropping below 2.30 flips FOUR flags at once**, from
  `config_for_aspsx_version`:

  ```python
  if aspsx_version < (2, 30):
      config.nop_at_expansion = True     # was False
      config.nop_mflo_mfhi   = False     # was True
      config.addiu_at        = True      # was False  <- the one we want
      config.nop_lw_lw       = True      # was False
  ```

  Three of those four are **nop-insertion rules**. `nop_mflo_mfhi` and
  `nop_lw_lw` affect multiply/divide result reads and consecutive loads
  respectively — constructs that appear throughout the image, including inside
  the 57 functions that currently match. Changing them is not surgical; it is a
  global codegen change wearing a version number.

This is the same shape as the `-G` experiment in `gp-relative-blocker.md`: a
correct diagnosis whose obvious remedy costs far more than it buys. That one
looked surgical too, and a clean rebuild damaged 19148 bytes.

## Options, in rough order of promise

Per CLAUDE.md rule 5 the head does not choose among these. This section is the
evidence for whoever does.

1. **Patch `addiu_at` alone**, decoupled from the version — either a local
   maspsx change or an upstream flag. This is the only option that matches the
   evidence: the census says `addiu_at=True` is right for all 502 indexed sites,
   and says nothing at all about the three nop rules, which the 2.34 defaults
   currently get right across 57 matched functions.
2. **Repin to 2.29 and measure.** Cheap to try, but expect nop-rule fallout, and
   it conflates four changes so a red result will not say which one did it.
3. **Establish what ASPSX version the game actually used.** 2.34 is inherited,
   not derived. If the real answer is below 2.30 then the three nop rules are
   also currently wrong and simply have not been exercised — which would be a
   much larger finding than this one.

## MANDATORY procedure for any experiment here

**`rm -rf build` first.** The Makefile makes every object depend on every source
and header, but **not on the Makefile itself**. Changing `MASPSX_FLAGS`
therefore rebuilds nothing and the build stays green because it is still the
*previous* build. This exact trap produced a fictional clean green during the
`-G` experiment and is documented in `gp-relative-blocker.md`. There is no safe
incremental path, because the flags are invisible to the dependency graph.

## Scope

At minimum 5 functions, confirmed blocked and stub-reported:

- `DreamSys`: `func_80059814`, `func_800598E8`
- `Entity`: `Entity__GetUnlockEffect`, `Entity__GetLinkStage`,
  `Entity__GetEventVideo`

502 unmatched sites across 39 files is the ceiling, though a single function may
contain several. Any function that loads through a runtime-indexed global is
exposed, and table lookups are pervasive in this game.

## Routing rule for runners, effective immediately

Before attempting a function, check for the construct:

```sh
grep -n 'addiu *\$at, *\$at, *%lo' asm/nonmatchings/<unit>/<func>.s
```

A hit means the function is blocked. File a stub report citing this document and
move on — do not spend attempts, and do not classify it as a scheduling or
delay-slot residue. The tell in a diff is a function that is one instruction
short with everything after it shifted.

**A useful control case:** `Entity__GetMoodEffect` matched cleanly at 6/6 and it
touches the same table. It only *forms* the address (`&arr[i]`) and never loads
through it, so no macro expansion is involved. Address-only table arithmetic is
safe; loading through a runtime-indexed global is not.

## Addendum, round 2026-09-01: `nop_mflo_mfhi` is implicated directly, with a corpus census

**Nothing was changed. This section is evidence for the operator's decision, per
CLAUDE.md rule 5.**

A third function, `func_8005950C` in `DreamSys`, stalled at 17/33 on a residue
that is NOT the `addiu_at` construct but IS one of the other three flags this
document already identifies as moving together with it.

### The stall, and its isolated reproducer

Retail computes `(dv * scaledArg2) / dt` as `mult; mflo v0; div zero,v0,a3`.
The pinned pipeline inserts **two `nop`s between the `mflo` and the `div`**.
Six source reshapes never moved that gap by a single word.

Reproduced in complete isolation, no project types involved — a five-line file
through the exact pinned pipeline:

```c
int f(int a, int b, int c) {
	int p = a * b;
	return p / c;
}
```

This is the `nop_mflo_mfhi` flag named in the table above, which a version bump
below 2.30 would set to `False`.

### The census, over the byte-exact build (read-only)

Counting `mflo`/`mfhi` sites across the whole image via `objdump -d` on
`build/lsdde.elf`:

| | count | share |
| --- | --- | --- |
| `mflo`/`mfhi` immediately followed by `nop` | 46 | 7.6% |
| `mflo`/`mfhi` NOT followed by `nop` | 557 | 92.4% |
| **total** | **603** | |

Narrowing to the specific hazard construct — an `mflo`/`mfhi` with a
`mult`/`div` within the next four instructions, which is exactly what
`func_8005950C` hits:

| | count |
| --- | --- |
| no intervening `nop` | 180 |
| one or more intervening `nop` | 65 |
| **total occurrences** | **245** |

(Some of the 65 are attributable to an intervening load's own delay slot rather
than to the `mflo` — e.g. `between=['lw','nop']` — so 65 is an upper bound on
genuine `mflo`-hazard nops.)

### What this changes, and what it does not

**It changes the framing of `nop_mflo_mfhi` in the section above.** That section
lists `nop_mflo_mfhi = False` among the *collateral damage* of a version bump —
"a global codegen change wearing a version number". For the large majority of
the image that characterisation is backwards: retail overwhelmingly does NOT
have the nop, so `False` is closer to retail's behaviour than the current
setting is.

**It does not make the bump the remedy.** Retail is not uniform: 180 sites want
no nop and up to 65 want one. **A global boolean cannot be right for both**, so
flipping the flag trades one set of failures for another, and the count of what
it would break is not knowable from this census alone.

The leading hypothesis, untested and deliberately so: the 46 immediate nops are
emitted by **cc1** as part of its own scheduling, and the correct assembler
behaviour is to insert none at all. That would reconcile every number here.
Testing it means running the pinned pipeline with a changed flag, which is a
toolchain change and therefore not something a head or runner does.

### Bearing on the blocker

The four flags move together, so this is not an independent lead — it is a
second, independently-discovered symptom of the same version question, arriving
from a different unit and a different construct. It raises the value of
resolving the version question and lowers the credibility of "the bump is purely
destructive", without making the bump safe.

It also adds one function to the blocked population that the `gp_rel` and
`addiu_at` greps do NOT catch. **Screening cannot currently see this class.** A
candidate screen for it, if the operator wants blocked functions flagged before
they are staffed:

```sh
grep -nE 'mflo|mfhi' asm/nonmatchings/<unit>/<func>.s
```

That over-reports heavily (603 sites, most harmless), so it is a triage hint and
not a blocker test like the other two.

## Addendum, round 2026-09-02 (carve round): the blocker also covers every
## jump-table `switch`

**Nothing was changed. Evidence only, per CLAUDE.md rule 5.**

The document above frames `addiu_at` as an *indexed global* problem —
`sym[reg]`. That framing is too narrow, and it matters for routing, because it
lets a runner look at a plain C `switch` with no array in sight and conclude the
blocker cannot apply.

A dense `switch` compiles to a jump table, and the dispatch is the same indexed
load through `$at`. Retail, at `func_80049EB4` in the newly carved
`class_39e08`:

```
lui   $at, %hi(jtbl_8001140C)
addiu $at, $at, %lo(jtbl_8001140C)
addu  $at, $at, $v0
lw    $v0, 0x0($at)
jr    $v0
```

### The reproducer

Self-contained, no project headers needed beyond the standard invocation:

```c
extern int sink(int);
int probe(int i) {
    switch (i) {
    case 0: return sink(11);
    case 1: return sink(22);
    case 2: return sink(33);
    case 3: return sink(44);
    case 4: return sink(55);
    case 5: return sink(66);
    case 6: return sink(77);
    case 7: return sink(88);
    case 8: return sink(99);
    }
    return -1;
}
```

Through the pinned pipeline, varying only `--aspsx-version`:

| version | dispatch expansion |
| --- | --- |
| **2.34** (the pin) | `lui $at,%hi($L12)` / `addu $at,$at,$2` / `lw $2,%lo($L12)($at)` — folded, 3 instructions |
| 2.29 | `lui` / `addiu $at,$at,%lo($L12)` / `addu` / `lw $2,0x0($at)` — **retail's shape** |
| 2.21 | same as 2.29 |

Identical behaviour to the indexed-array case, same flag, same remedy question.
This does not add a new blocker; it widens the reach of the one already
escalated, and it strengthens option 1 (patch `addiu_at` alone) by adding a
second independent construct that the flag gets right and the three nop rules
have nothing to say about.

### Corpus census

Across every `.s` in `asm/`:

| | count |
| --- | --- |
| distinct `jtbl_*` symbols in rodata | **27** |
| dispatch sites using the unfolded (2.29) form | **27** |
| dispatch sites using the folded (2.34) form | **0** |

The 27 tables are spread over 11 files. They are a subset of the 502 indexed
sites already counted above, so the ceiling does not move — what moves is the
*description* of which C constructs are exposed.

### Routing rule, extended

The existing grep still finds these, because the jump table's dispatch contains
the same instruction:

```sh
grep -n 'addiu *\$at, *\$at, *%lo' asm/nonmatchings/<unit>/<func>.s
```

What changes is what a runner should conclude when it hits. Add: **a `switch`
dense enough to become a jump table is blocked, even with no array in the C.**
Sparse switches that compile to compare-and-branch chains are fine and several
already match (`func_8005966C` and `func_800596E8` in `DreamSys`, four cases
each).

At carve time this is also a boundary question, not only a routing one: a
carved unit that takes ownership of a function containing a jump table takes
ownership of that table's rodata slot too. See the `class_39e08` carve commit.

## Addendum, round 2026-09-02b: cc1 does NOT emit the nops — maspsx does. Confirmed by reading cc1's output.

**Nothing was changed. Evidence only, per CLAUDE.md rule 5.**

The 2026-09-01 addendum above closes with a hypothesis, explicitly marked
untested and deliberately so:

> The leading hypothesis, untested and deliberately so: the 46 immediate nops
> are emitted by **cc1** as part of its own scheduling, and the correct
> assembler behaviour is to insert none at all.

**Half of that is now settled, and it did not need a flag change to settle —
only reading cc1's output instead of maspsx's.** cc1 emits no nops at all
here. It emits `#nop`, commented out, as a *hint*:

```
	mult	$2,$17
	mfhi	$4
	#nop
	#nop
	mult	$16,$17
```

That is the raw `cpp | cc1` stream. The `mult` sits immediately after the
`mfhi`, which is retail's shape, and the two `#nop` lines are markers saying
"a hazard nop may be required here". Psy-Q-patched cc1 leaves the decision to
the assembler.

maspsx is what decides. At 2.34 (`nop_mflo_mfhi = True`) it converts the hints
into real instructions and displaces the `mult` behind them; its own debug
output shows the substitution:

```
mult	$2,$3
mfhi	$4
nop
nop
mult	$16,$3
# #nop  # DEBUG: skipped
# #nop  # DEBUG: skipped
# mult	$16,$3  # DEBUG: skipped
```

At 2.29 and 2.21 it leaves them commented and the stream keeps cc1's order.

### Consequence for the framing

The hypothesis said cc1 emitted the nops and the assembler should insert none.
**cc1 emits none, so the second half — that the correct assembler behaviour is
to insert none — is the only remaining question, and the census above already
answers it for the majority of the image**: 180 sites want no nop against up to
65 that want one.

This does not change the conclusion that a global boolean cannot be right for
both, and it does not make a version bump safe — the other three flags still
move with it. What it does change is where the remaining uncertainty lives. It
is no longer "who emits these"; it is only "what should the 65 sites be".

### A full-function reproducer, not a construct

Every prior entry in this document rests on a minimal construct. This one has
a whole retail function behind it.

`IsDaySpecial` (DreamSys, 52 instructions) was stalled at 18/52 by a runner as
a scheduling residue with "no C-level lever". Both of its residues are now
closed. One was a missing cast (`(u32)i < 42` rather than `u32 i`, so the
comparison goes unsigned while `i % 12` keeps its signed magic-multiply). The
other is this flag.

With that cast, compiled standalone through the pinned pipeline varying
**only** `--aspsx-version`:

| version | `.text` | result |
| --- | --- | --- |
| **2.34** (the pin) | 0xD8 | two nops between `mfhi` and `mult`; 2 instructions long |
| 2.29 | 0xD0 | **all 52 instructions match retail one-for-one, register for register** |

The full side-by-side is in `docs/match-reports/IsDaySpecial.md`, along with
the body, which is complete and needs no further work if the flag question is
ever resolved.

This is the strongest form of evidence this document has: a real function,
correct C, and a single flag standing between it and a byte match. It is worth
weighing against the 65 sites on the other side of the census — but it does not
by itself decide them, and the pin stays where it is until the operator says
otherwise.

### Screening

Unchanged from the previous addendum, and still the weak point: neither the
`gp_rel` nor the `addiu_at` grep catches this class, and `mflo|mfhi` over-reports
badly (603 sites). A tighter screen, now that the mechanism is known — a `mflo`
or `mfhi` with a `mult`, `multu`, `div` or `divu` within the next two
instructions in RETAIL:

```sh
grep -A2 -nE '\b(mflo|mfhi)\b' asm/nonmatchings/<unit>/<func>.s | grep -E '\b(mult|multu|div|divu)\b'
```

A hit means retail did not take the nops and the pin will insert them.
## Addendum, round 2026-09-02b: cc1 does NOT emit the nops — maspsx does. Confirmed by reading cc1's output.

**Nothing was changed. Evidence only, per CLAUDE.md rule 5.**

The 2026-09-01 addendum above closes with a hypothesis, explicitly marked
untested and deliberately so:

> The leading hypothesis, untested and deliberately so: the 46 immediate nops
> are emitted by **cc1** as part of its own scheduling, and the correct
> assembler behaviour is to insert none at all.

**Half of that is now settled, and it did not need a flag change to settle —
only reading cc1's output instead of maspsx's.** cc1 emits no nops at all
here. It emits `#nop`, commented out, as a *hint*:

```
	mult	$2,$17
	mfhi	$4
	#nop
	#nop
	mult	$16,$17
```

That is the raw `cpp | cc1` stream. The `mult` sits immediately after the
`mfhi`, which is retail's shape, and the two `#nop` lines are markers saying
"a hazard nop may be required here". Psy-Q-patched cc1 leaves the decision to
the assembler.

maspsx is what decides. At 2.34 (`nop_mflo_mfhi = True`) it converts the hints
into real instructions and displaces the `mult` behind them; its own debug
output shows the substitution:

```
mult	$2,$3
mfhi	$4
nop
nop
mult	$16,$3
# #nop  # DEBUG: skipped
# #nop  # DEBUG: skipped
# mult	$16,$3  # DEBUG: skipped
```

At 2.29 and 2.21 it leaves them commented and the stream keeps cc1's order.

### Consequence for the framing

The hypothesis said cc1 emitted the nops and the assembler should insert none.
**cc1 emits none, so the second half — that the correct assembler behaviour is
to insert none — is the only remaining question, and the census above already
answers it for the majority of the image**: 180 sites want no nop against up to
65 that want one.

This does not change the conclusion that a global boolean cannot be right for
both, and it does not make a version bump safe — the other three flags still
move with it. What it does change is where the remaining uncertainty lives. It
is no longer "who emits these"; it is only "what should the 65 sites be".

### A full-function reproducer, not a construct

Every prior entry in this document rests on a minimal construct. This one has
a whole retail function behind it.

`IsDaySpecial` (DreamSys, 52 instructions) was stalled at 18/52 by a runner as
a scheduling residue with "no C-level lever". Both of its residues are now
closed. One was a missing cast (`(u32)i < 42` rather than `u32 i`, so the
comparison goes unsigned while `i % 12` keeps its signed magic-multiply). The
other is this flag.

With that cast, compiled standalone through the pinned pipeline varying
**only** `--aspsx-version`:

| version | `.text` | result |
| --- | --- | --- |
| **2.34** (the pin) | 0xD8 | two nops between `mfhi` and `mult`; 2 instructions long |
| 2.29 | 0xD0 | **all 52 instructions match retail one-for-one, register for register** |

The full side-by-side is in `docs/match-reports/IsDaySpecial.md`, along with
the body, which is complete and needs no further work if the flag question is
ever resolved.

This is the strongest form of evidence this document has: a real function,
correct C, and a single flag standing between it and a byte match. It is worth
weighing against the 65 sites on the other side of the census — but it does not
by itself decide them, and the pin stays where it is until the operator says
otherwise.

### Screening

Unchanged from the previous addendum, and still the weak point: neither the
`gp_rel` nor the `addiu_at` grep catches this class, and `mflo|mfhi` over-reports
badly (603 sites). A tighter screen, now that the mechanism is known — a `mflo`
or `mfhi` with a `mult`, `multu`, `div` or `divu` within the next two
instructions in RETAIL:

```sh
grep -A2 -nE '\b(mflo|mfhi)\b' asm/nonmatchings/<unit>/<func>.s | grep -E '\b(mult|multu|div|divu)\b'
```

A hit means retail did not take the nops and the pin will insert them.

## Addendum (round 10): the blocker's SCOPE, measured

**Dense `switch` jump tables are inside this blocker, not beside it.** This was
already assumed in a couple of stub reports, but it had never been measured,
and it is the assumption a careful reader is most likely to overturn — so here
is the measurement, cheap enough to repeat.

The reason it invites doubt: the two constructs share nothing that is visible
in the disassembly. A jump-table dispatch loads a **code** address and jumps to
it; an indexed global load produces a **data** value the code then uses. The
symbol names differ too — `jtbl_*` against `D_*`. Reading the `.s`, they look
like separate phenomena, and the natural conclusion is that the screening grep
over-reports on the `jtbl_*` ones.

It does not. cc1 emits the **same generic pseudo-op** for both and expresses no
opinion about addressing:

```
indexed global:      lbu $2,D_80089EAC($4)
switch jump table:   lw  $2,$L13($2)
```

The expansion happens in maspsx, *below* cc1, which cannot distinguish them and
does not try. Through the pinned pipeline a dense ten-case switch comes out in
the folded three-instruction form where retail has the unfolded four — the same
one-instruction deficit, with the same non-local consequence for everything
after it in the translation unit.

Reproducer:

```c
extern int sink(int);
int probe(int sel)
{
    switch (sel) {
    case 0: return sink(10);
    case 1: return sink(11);
    case 2: return sink(12);
    case 3: return sink(13);
    case 4: return sink(14);
    case 5: return sink(15);
    case 6: return sink(16);
    case 7: return sink(17);
    case 8: return sink(18);
    case 9: return sink(19);
    }
    return -1;
}
```

Through the pipeline in CLAUDE.md's "Escalate, do not experiment" section:

```
lui   at, %hi(.rodata)
addu  at, at, v0
lw    v0, %lo(.rodata)(at)     <- folded; retail resolves the base first
nop
jr    v0
```

### What this changes

- **Nothing about the escalation.** Same mechanism, same remedy question, same
  operator call. The version bump is still not the remedy.
- **The footprint is bigger than a `D_*`-only reading suggests**, and the
  corpus census should be read with the raw grep: of 505 `addiu $at, $at, %lo`
  sites, 30 are `jtbl_*`. All 505 count.
- **Corroborating negative:** no matched C function in the project owns a dense
  `switch` jump table. The matched `switch` statements that do exist are sparse
  (e.g. `func_8003BC14`'s cases 5/7/8/0x12), which GCC 2.6.3 compiles to a
  compare chain and which matches fine. That is consistent with dense switches
  being unreachable under the pin, and it is the reason the gap went unnoticed.

### How this was found, and the cheaper route

Round 10's head found two `jtbl_*` hits while carving `code_2cc8c`, reasoned
they were false positives, edited CLAUDE.md's screening grep to exclude them,
audited the corpus and "recovered" three functions, and broadcast the
correction to five live runners — one of which had already been handed two of
the recovered functions as work. Then it ran the reproducer above, got the
opposite result, and retracted all of it.

The reproducer took under a second and was available from the first minute. The
project rule it violated is already written down in CLAUDE.md: *never escalate
a toolchain lead you have not tried and failed to reproduce in isolation.* The
same rule applies in reverse — **never DE-escalate one either.** Narrowing a
blocker's scope is a toolchain claim and needs the same reproducer as widening
it, and it is the more dangerous direction: widening one costs attempts,
narrowing one sends runners at functions that cannot match and reads, in the
match reports left behind, exactly like ordinary stalls.
