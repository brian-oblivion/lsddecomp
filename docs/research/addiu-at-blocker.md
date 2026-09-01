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
