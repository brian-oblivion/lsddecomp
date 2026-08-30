# Entity__GetEventVideo

**Unit:** Entity · **Size:** 9 instructions · **Status:** STALLED, class TOOLCHAIN (maspsx `addiu_at` macro-expansion mismatch — NEW, distinct from the gp-relative blocker)

## What it does

`return (s8)D_80089EAC[this->moodIndex * 0x10] - 1;` — a signed-byte table
lookup indexed by `this->moodIndex` (a 16-byte stride, the same index used
by `Entity__GetMoodEffect`/`Entity__GetUnlockEffect`/`Entity__GetLinkStage`,
all four keyed off the same field, each with its own table).

## The residue

```
retail:  lui   $at,%hi(D_80089EAC)
         addiu $at,$at,%lo(D_80089EAC)
         addu  $at,$at,$v0
         lb    $v0,0x0($at)
built:   lui   $at,%hi(D_80089EAC)
         addu  $at,$at,$v0
         lb    $v0,%lo(D_80089EAC)($at)
```

Both compute the identical final address; retail fully resolves the symbol
into `$at` (4 instructions total for the load), the build folds the `%lo`
relocation into the `lb`'s own displacement field (3 instructions). This is
1 word short, every time, regardless of how the C is written.

**This is not a compiler codegen choice — it happens below cc1.** Isolated
with the pinned pipeline per CLAUDE.md's reproducer recipe:

```c
typedef signed char s8;
typedef signed int s32;
extern s8 D_80089EAC[];
s32 test(s32 mood) {
    return D_80089EAC[mood * 0x10] - 1;
}
```

`cc1`'s own output (before maspsx ever runs) is a single symbolic
pseudo-instruction:

```
sll  $4,$4,4
lb   $2,D_80089EAC($4)
j    $31
addu $2,$2,-1
```

`cc1` never chooses between the two forms above at all — it emits one
generic `lb $reg,sym($reg)` pseudo-op and leaves the hi/lo split entirely to
the assembler. So **no C-level reshaping can affect this**, which matches
what was actually observed: four different rewrites (a local `s8 *p =
&arr[idx]; return *p;` intermediate, a separate `s32 idx = ...;` statement, a
separate `s32 mood = this->moodIndex;` copy, and indexing through a 16-byte
`struct { s8 value; u8 pad[15]; }` array instead of raw byte arithmetic) all
produced byte-identical output for this instruction sequence.

**The real decision is `maspsx`'s `addiu_at` flag**
(`tools/maspsx/maspsx/__init__.py`, the `elif is_addend and r_source:` branch
around line 970). Piping the same `cc1` output through `maspsx.py` with two
different `--aspsx-version` values, changing nothing else:

| `--aspsx-version` | expansion | matches retail? |
| --- | --- | --- |
| `2.34` (**the project's pin**) | `lui / addu / lb %lo(sym)($at)` | **no** |
| `2.29` | `lui / addiu / addu / lb 0x0($at)` | **yes** |

`tools/maspsx/maspsx.py`'s own `config_for_aspsx_version()` sets
`addiu_at = True` only when `aspsx_version < (2, 30)`, and the project pins
`2.34`. So the maspsx version-behavior model, at exactly the pinned version,
disagrees with retail for this specific pseudo-instruction shape
(register-indexed byte/word load from a non-`$gp` symbol — `lb $reg,
sym($reg)`), even though `2.34` is presumably correct for whatever evidence
originally justified the pin (probably the `div`/`li` macro shapes documented
elsewhere, or the gp-relative behavior, which is a *different* code path in
maspsx entirely — see `docs/research/gp-relative-blocker.md`; that finding is
about `$gp`-relative addressing being unreachable at `-G0`, this one is about
a *non-`$gp`* indexed load choosing the wrong hi/lo split at the pinned
version).

## Scope

At minimum, this blocks all three of `Entity__GetUnlockEffect`,
`Entity__GetLinkStage`, and `Entity__GetEventVideo` in this unit — every
function here that *loads through* a table symbol with a runtime register
index. `Entity__GetMoodEffect` (matched, see its own report) is unaffected
because it only ever forms the address (`&arr[idx]`), never loads through
it, which routes through a different maspsx code path (`is_addend and
r_source is None` above, or the plain address-formation macro, not the
load-expansion branch). Any function anywhere in the game that reads a
global/rodata table using a non-constant index is a candidate for this same
residue — this is likely comparable in scope to the gp-relative blocker,
possibly larger, since indexed table lookups are a very common shape.

## What is NOT yet established

- Whether `--aspsx-version=2.29` (or `addiu_at=True` some other way) would
  regress any of the 46 already-matched functions elsewhere in the project.
  **Untested against the real build** — the comparison above used only the
  isolated `/tmp` reproducer pipeline, never `build-and-verify.sh`, per
  CLAUDE.md's "escalate, do not experiment" rule. Given the gp-relative
  blocker's own experience (a global `-G` flip looked clean in isolation but
  broke 19148 bytes elsewhere once actually rebuilt), a global version flip
  here should be assumed equally risky until an operator authorizes and runs
  the real experiment with a full `rm -rf build` rebuild.
- Whether the "right" fix is a different global `--aspsx-version`, or
  whether `maspsx` needs a per-construct override (its `addiu_at` config is
  a single project-wide bool, not conditional on instruction shape) — i.e.
  whether the *true* Sony ASPSX build of this game mixed both hi/lo
  expansion styles depending on context in a way no single maspsx version
  flag currently models.
- What other constructs, if any, gate on `addiu_at` inside maspsx besides
  this indexed-load path (grep hit at least one more call site,
  `tools/maspsx/maspsx/__init__.py:1059`, `if self.addiu_at and op != "la"`,
  not yet investigated).

## Preserved bodies

```c
#if 0
s32 Entity__GetUnlockEffect(Entity *this) {
    return D_80089EA6[this->moodIndex * 0x10] * 1000;
}
#endif
```

```c
#if 0
s32 Entity__GetLinkStage(Entity *this) {
    s32 v;

    v = D_80089EAB[this->moodIndex * 0x10];
    if (v < 0) {
        return ~v;
    }
    return v - 1;
}
#endif
```

```c
#if 0
s32 Entity__GetEventVideo(Entity *this) {
    return D_80089EAC[this->moodIndex * 0x10] - 1;
}
#endif
```

All three preserve the correct control flow and field/table derivation
(`Entity__GetLinkStage`'s branch, in particular, is verified structurally
correct — same branch targets as retail, confirmed with `asm-differ`, which
is exactly the check the head's round-2026-08-30-a broadcast on `strcat`
said to make before calling anything compiler-internal; here the CFG matches
and the *only* residue is the one-instruction addressing-mode gap above).
Any of the four table declarations (`s8 D_80089EA6/AB/AC[]`) and
`Entity.h`'s `moodIndex` field are correct regardless of this stall.

## Proposed learning

**New toolchain finding, sibling to the gp-relative blocker but a different
mechanism and a different maspsx flag (`addiu_at`, not `-G`).** Any function
whose retail disassembly shows a *fully resolved* symbol+register address
(`lui`+`addiu`+`addu`+`load 0x0(...)`, four instructions) where the pinned
toolchain produces a *folded* form (`lui`+`addu`+`load %lo(sym)(...)`, three
instructions) is this class, not a reshaping target — check with
`tools/gcc263/cc1` + `tools/maspsx/maspsx.py` directly (varying only
`--aspsx-version` in isolation) before spending attempts. Escalating per
CLAUDE.md's "report with a reproducer, never experiment mid-round" — the
reproducer above is self-contained and takes under a second to re-run.
