# Snd_setVabAttr -- MATCHED (round 49): 179/179, byte-exact

## Round 94 (runner echo, track 6): Sony types

`Scratch_800357B0` is gone: it was already Sony's `VagAtr` field-for-field at the fields it named, so the parameter is now `VagAtr scratch`. Sony's `SsUtGetVagAtr`/`SsUtSetVagAtr`/`SsUtReverbOn`/`SsUtReverbOff`/`SsUtSetReverb*` prototypes come from `<libsnd.h>`. Bytes unchanged; the bodies below keep their original declarations as history.

`AdsrRaw_800357B0` is now `AdsrFields`, and its fields are named for the SPU ADSR1/ADSR2 bit fields that `_SsUtResolveADSR` unpacks and `_SsUtBuildADSR` repacks (`func_80035F3C.md`, `func_80035F98.md`). The mapping: unk0 `attackRate` (ADSR1 14-8), unk2 `decayRate` (7-4), unk4 `sustainLevel` (3-0), unk6 `sustainRate` (ADSR2 12-6), unk8 `releaseRate` (4-0), unkA `attackMode` (ADSR1 bit 15), unkC `sustainMode` (ADSR2 bit 15), unkE `releaseMode` (bit 5), unk10 `sustainDir` (bit 14). This function's arms agree with it: 4/5 set attack rate with linear/exponential mode, 6 decay, 7 sustain level, 8/9 sustain rate+mode, 10/11 release rate+mode, and 12 the sustain direction.

> Renamed from `func_800357B0` on 2026-09-23 (tools/rename.py). Address 0x800357b0.

**ROUND 49: MATCHED.** See the update at the end of this report. Everything
below this point is the historical derivation that got the function to
171/179 (rounds 27-46); kept for the record.

Original title: STALL: length EXACT (179/179 words); raw word-match 171/179 (round 39, up from round 35's 163/179); first real diff a register-identity swap, `channel` in $s2/`arg5` in $s3 vs retail's $s3/$s2 (vram `0x8003580c` onward, e.g. word 24)

**ROUND 35 UPDATE.** This function was previously blocked on two things: the
exact type/role of its 4th argument and scratch struct, and total absence of
independent corroboration for its nine cross-unit calls (all local guesses,
`func_80036230`/`func_80036044`/etc, per the round-27 report below). **Both
are now resolved.** Round 34's SDK-object conversion linked `code_179d8_f.c`'s
tail against Sony's `libsnd/adsr.o` and placed six more `libsnd` objects
elsewhere in this game's build, which retyped every one of this function's
callees from a local guess into a real, named Psy-Q symbol:

| old local-guess name | real symbol | source |
| --- | --- | --- |
| `func_80033260` (entry fill) | `SsUtGetVagAtr` | `include/psyq/libsnd.h` |
| `func_80036230` (all three "set" calls) | `SsUtSetVagAtr` | `include/psyq/libsnd.h` |
| `func_80036044` | `SsUtReverbOff` | `include/psyq/libsnd.h` |
| `func_80036024` | `SsUtReverbOn` | `include/psyq/libsnd.h` |
| `func_80035F3C` | `_SsUtResolveADSR` | matched, `docs/match-reports/func_80035F3C.md` |
| `func_80035F98` | `_SsUtBuildADSR` | matched, `docs/match-reports/func_80035F98.md` |
| `func_80036064` | `SsUtSetReverbType` | `include/psyq/libsnd.h` |
| `func_80036118` | `SsUtSetReverbDepth` | `include/psyq/libsnd.h` |
| `func_800361B0` | `SsUtSetReverbFeedback` | `include/psyq/libsnd.h` |
| `func_800361F0` | `SsUtSetReverbDelay` | `include/psyq/libsnd.h` |

With every callee real, the function is now a **163/179-word near-miss,
length EXACT, zero out-of-range drift** -- a completely different situation
from the round-27 "no C written" stall below, which is kept for its (still
valid) control-flow decode but is superseded on every other point.

## What settled the 4th-argument / scratch-struct question

`ContDataEntry.md`'s "`Blk1`/`Blk2` by-value pair" reading turned out to be
one struct too many. Reading this function's OWN disassembly directly
(`asm/nonmatchings/code_179d8_k/Snd_setVabAttr.s`) rather than inferring from
the caller:

- The incoming 4th parameter's first word lands at `sp+0x3C` (register `$a3`'s
  standard home slot for a function with a `-0x30` frame) and
  `SsUtGetVagAtr`'s own `out` argument is `sp+0x3C` too -- i.e. **the 4th
  parameter IS the `VagAtr` `SsUtGetVagAtr`/`SsUtSetVagAtr` fill, passed BY
  VALUE**, not a `u32` header plus a separate scratch struct. Its incoming
  value is provably irrelevant: `SsUtGetVagAtr` unconditionally overwrites the
  whole 32 bytes before any dispatch arm reads it.
- Retail's frame is `-0x30` (48 bytes) yet this "scratch" struct's fields sit
  at `sp+0x3C..0x5C` -- ABOVE the local frame entirely. That is only possible
  if it is incoming argument space, not a local; the ordinary MIPS o32 rule
  for a large by-value struct (first word in a register, the rest already on
  the caller's stack, contiguous) explains the exact byte range with no local
  frame space spent on it at all.
- The SAME reasoning applies to what the round-27 report called `Blk2`: a
  second by-value struct, `sp+0x5C..0x6E`, matching `code_179d8_f.c`'s
  independently-matched `UnkStruct80035F3C` (9 halfwords, offsets `0`, `2`,
  `4`, `6`, `8`, `0xA`, `0xC`, `0xE`, `0x10`) EXACTLY -- this function's
  fifth parameter. `_SsUtResolveADSR(scratch.adsr1, scratch.adsr2, &resolved)`
  fills it from the VagAtr's own ADSR fields at entry to the arg5-in-[4,14]
  block; every case in that block mutates it (or, for two cases, the
  VagAtr's `vibT`/`porW` bytes instead); `_SsUtBuildADSR` writes it back into
  `scratch.adsr1`/`adsr2` before the shared `SsUtSetVagAtr` at the block's
  tail. Passing it by value costs nothing extra, for the same "already
  discarded before use" reason as the VagAtr parameter, and its stack
  position (immediately following the VagAtr, zero gap) is exactly what o32
  argument layout predicts for a 6th declared parameter.
- `arg5` (outer parameter selector, `sp+0x70`) and `arg6` (value byte,
  `sp+0x74`) are ordinary trailing scalar parameters, unchanged from the
  round-27 reading.

```c
typedef struct {
    u8 prior;    /* +0x0 */
    u8 mode;     /* +0x1 */
    u8 pad2[0x6 - 0x2];
    u8 min;      /* +0x6 */
    u8 max;      /* +0x7 */
    u8 pad8[0x9 - 0x8];
    u8 vibT;     /* +0x9 */
    u8 porW;     /* +0xA */
    u8 padB[0x10 - 0xB];
    u16 adsr1;   /* +0x10 */
    u16 adsr2;   /* +0x12 */
    u8 pad14[0x20 - 0x14];
} Scratch_800357B0;   /* this unit's reduced view of include/psyq/libsnd.h's VagAtr */

typedef struct {
    s16 unk0; s16 unk2; s16 unk4; s16 unk6; s16 unk8;
    s16 unkA; s16 unkC; s16 unkE; s16 unk10;
} AdsrFields;   /* same shape as code_179d8_f.c's UnkStruct80035F3C, renamed per unit */

void Snd_setVabAttr(s16 channel, s16 slot, s16 kind, Scratch_800357B0 scratch,
                    AdsrFields resolved, s16 arg5, u8 arg6);
```

**Prototype-typing correction needed for the by-value calls to land right**:
this unit's existing `SsUtGetVagAtr`/`SsUtSetVagAtr` extern declared their
2nd parameter `u8` (fine for this unit's OTHER call sites, which always pass
a genuine byte read like `p[0x2C]`); this call site passes the function's
own `s16 slot` parameter directly, already widened by the standard
per-parameter `sll`/`sra` re-widen every s16 local gets in this codebase.
Widening the shared declaration to `s16` (matching `include/psyq/libsnd.h`'s
real `short SsUtGetVagAtr(short, short, short, VagAtr*)` exactly) fixed this
call site with **zero effect on any already-matched call site in the same
unit** (confirmed by rebuilding after the prototype change alone, before
writing a single line of this function's body: `build-and-verify.sh` still
green). Same story for `SsUtSetReverbDepth`, previously declared
`(s32, s32)` in this unit for `_SsSetControlChange`'s own call: widening to
`(s16, s16)` (again matching the real header) removed a spurious `andi
...,0xff` mask this function's `SsUtSetReverbDepth(arg6, arg6)` picked up
under the wider declaration, again with no effect on `_SsSetControlChange`'s own
already-matched bytes.

## Two real levers that closed a 7-words-short gap to 163/179

1. **Do not hand-roll a bounds check ahead of the `switch`.** An explicit
   `if ((u16)arg5 >= 23) return;` before `switch (arg5) { ... }` makes GCC
   emit that check AND its own internal bounds check for the switch's jump
   table (computed off the highest explicit `case` label used) --
   **two range checks**, when retail has exactly one. Declaring `case 20:`,
   `case 21:`, `case 22:` explicitly (falling into `default: return;`,
   matching the jump table's own trailing duplicate/zero entries) lets the
   switch's own lowering produce the SAME SINGLE `sltiu ...,0x17` (23) check
   retail has, with no separate guard needed.
2. **The inner "case 4..14" dispatch must stay `switch (arg5) { case 4: ...
   case 14: ... }`, not `switch (arg5 - 4) { case 0: ... case 10: ... }`.**
   Both are semantically identical C, but retail's own machine code computes
   `arg5 - 4` on the RAW (not-yet-widened) parameter register and only
   THEN widens the difference (`addiu v0,$rawArg5,-4` / `sll` / `sra`);
   writing the subtraction explicitly in source, on an ALREADY-widened local
   `arg5`, makes GCC widen first and subtract second (`sll`/`sra` THEN
   `addiu v0,v0,-4`) -- same value, different instruction order, one extra
   reload of a stale idea about what "the raw value" is. Keeping the case
   labels at their real MIDI-parameter numbers (4..14) lets GCC's own
   switch-lowering do the `-4` normalization internally, on the register it
   already holds unwidened, matching retail exactly.

## The one residue that would not move: a dead-value computation, same
   family as `ContDataEntry`'s

Case 12 (arg5, i.e. the 9th arm of the inner switch, a 3-way range check on
`arg6` against `0x40`/`0x80`) is the one place retail computes a value it
does not always use: on the "arg6 in [1,0x3F]" path, retail computes
`v0 = arg6 - 0x40` (`addiu v0,s1,-0x40`) immediately before jumping away
WITHOUT reading that computed value at all -- the identical shape is
recomputed independently on the OTHER two paths (`arg6 == 0` and
`arg6 >= 0x40`) that actually consume it via a shared `sltiu ...,0x40`
check. A straightforward `if/else` (compute the subtraction only where
needed) came out 2 words short of this because GCC eliminated the dead
copy entirely on the LT-0x40 path. Hoisting the subtraction into an
unconditional `s32 t = arg6 - 0x40;` BEFORE the `if (arg6 == 0) {} else if
(arg6 < 0x40) { ...; break; }` chain reproduced retail's exact
redundant-computation shape byte for byte (confirmed: this specific block
now matches 100%, no diff reported anywhere in `case 12`'s bytes). This is
the same "GCC 2.6.3 keeps a register-only dead value alive across a branch"
family `ContDataEntry.md` documents, just resolved successfully here by
hoisting rather than defeated by it.

## The one residue that DID NOT resolve: `channel`/`arg5` register swap

After both fixes above, **every remaining word-level diff (16 of 179) is the
identical register-identity swap**: this build's compiled object keeps the
widened `channel` parameter live in `$s2` and the cached (post-`SsUtGetVagAtr`)
copy of `arg5` in `$s3`; retail has them the other way around (`channel` in
`$s3`, `arg5` in `$s2`). Every flagged word is either the callee-save
prologue pair (`sw $s2,.../sw $s3,...` swapped with its offset) or one of the
four places `channel` gets re-widened before a call (`sll a0,s2,0x10` vs
`sll a0,s3,0x10`). `slot` (`$s4`) and `kind` (`$s5`) match retail exactly,
both builds use the identical SIX-register set (`s0`-`s5`), and the frame
size, every stack offset, every branch target, and every call argument
matches byte for byte -- this is a pure allocation-order swap between two
already-correct registers, not a wrong value or a wrong control-flow shape.

**One reshape tried, no effect on this swap:** hoisting `channel` into an
explicit `s16 ch = channel;` local at function entry and using `ch`
throughout (matching the pattern that helped `ContDataEntry`'s own report)
produced byte-IDENTICAL output -- same 163/179, same swap. Per
CLAUDE.md's explicit register-identity test ("if removing it changes WHICH
REGISTER holds a value, it is banned; if it only changes instruction order,
it is allowed"), this is the STALL class the rule names, not a banned-fix
target: no `register T v asm("$N")` or extended-asm operand constraint was
used or considered.

## Verification

`./build-and-verify.sh` exit 0 confirms the whole-image SHA1 is unaffected
(function restored to `INCLUDE_ASM`). `tools/funcdiff.py Snd_setVabAttr`
reports **163/179 words match**, length exact (no out-of-range drift
warning). Compiled length cross-checked directly via
`mipsel-linux-gnu-objdump` symbol-to-symbol distance
(`build/src/code_179d8_k.c.o`, `Snd_setVabAttr`..`SetPitchBend` = `0x2CC` =
179 words, matching retail's own `nonmatching Snd_setVabAttr, 0x2CC` header).

## Body as reached (163/179, length exact, register-identity residue only)

```c
#if 0
typedef struct {
    u8 prior;    /* +0x0 */
    u8 mode;     /* +0x1 */
    u8 pad2[0x6 - 0x2];
    u8 min;      /* +0x6 */
    u8 max;      /* +0x7 */
    u8 pad8[0x9 - 0x8];
    u8 vibT;     /* +0x9 */
    u8 porW;     /* +0xA */
    u8 padB[0x10 - 0xB];
    u16 adsr1;   /* +0x10 */
    u16 adsr2;   /* +0x12 */
    u8 pad14[0x20 - 0x14];
} Scratch_800357B0;

typedef struct {
    s16 unk0;
    s16 unk2;
    s16 unk4;
    s16 unk6;
    s16 unk8;
    s16 unkA;
    s16 unkC;
    s16 unkE;
    s16 unk10;
} AdsrFields;

extern void _SsUtResolveADSR(s32 a0, s32 a1, AdsrFields *out);
extern void _SsUtBuildADSR(AdsrFields *in, u16 *adsr1, u16 *adsr2);
extern void SsUtReverbOn(void);
extern s16 SsUtSetReverbType(s16 a0);
extern void SsUtSetReverbFeedback(s16 a0);
extern void SsUtSetReverbDelay(s16 a0);
/* Also requires, in this unit: SsUtGetVagAtr/SsUtSetVagAtr's 2nd param
 * widened to s16 (was u8), and SsUtSetReverbDepth widened to (s16, s16)
 * (was (s32, s32)) -- see "Prototype-typing correction" above. Both were
 * verified to leave every OTHER already-matched call site in this unit
 * byte-identical. */

void Snd_setVabAttr(s16 channel, s16 slot, s16 kind, Scratch_800357B0 scratch,
                    AdsrFields resolved, s16 arg5, u8 arg6)
{
    SsUtGetVagAtr(channel, slot, kind, &scratch);

    switch (arg5) {
    case 0:
        scratch.prior = arg6;
        goto tailA;
    case 1:
        scratch.mode = arg6;
        SsUtSetVagAtr(channel, slot, kind, &scratch);
        if (arg6 == 0) {
            SsUtReverbOff();
            return;
        }
        if (arg6 == 1) {
            return;
        }
        if (arg6 == 2) {
            return;
        }
        if (arg6 == 3) {
            return;
        }
        if (arg6 != 4) {
            return;
        }
        SsUtReverbOn();
        return;
    case 2:
        scratch.min = arg6;
        goto tailA;
    case 3:
        scratch.max = arg6;
tailA:
        SsUtSetVagAtr(channel, slot, kind, &scratch);
        return;
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    {
        _SsUtResolveADSR(scratch.adsr1, scratch.adsr2, &resolved);
        switch (arg5) {
        case 4:
            resolved.unkA = 0;
            resolved.unk0 = arg6;
            break;
        case 5:
            resolved.unkA = 1;
            resolved.unk0 = arg6;
            break;
        case 6:
            resolved.unk2 = arg6;
            break;
        case 7:
            resolved.unk4 = arg6;
            break;
        case 8:
            resolved.unkC = 0;
            resolved.unk6 = arg6;
            break;
        case 9:
            resolved.unkC = 1;
            resolved.unk6 = arg6;
            break;
        case 10:
            resolved.unkE = 0;
            resolved.unk8 = arg6;
            break;
        case 11:
            resolved.unkE = 1;
            resolved.unk8 = arg6;
            break;
        case 12: {
            s32 t = arg6 - 0x40;

            if (arg6 == 0) {
                /* nothing -- falls to the shared check below */
            } else if (arg6 < 0x40) {
                resolved.unk10 = 0;
                break;
            }
            if ((u32)t < 0x40) {
                resolved.unk10 = 1;
            }
            break;
        }
        case 13:
            scratch.vibT = arg6;
            break;
        case 14:
            scratch.porW = arg6;
            break;
        }
        _SsUtBuildADSR(&resolved, &scratch.adsr1, &scratch.adsr2);
        SsUtSetVagAtr(channel, slot, kind, &scratch);
        return;
    }
    case 15:
        SsUtSetReverbType(arg6);
        return;
    case 16:
        SsUtSetReverbDepth(arg6, arg6);
        return;
    case 17:
        SsUtSetReverbFeedback(arg6);
        return;
    case 18:
    case 19:
        SsUtSetReverbDelay(arg6);
        return;
    case 20:
    case 21:
    case 22:
    default:
        return;
    }
}
#endif
```

## Proposed learning

1. **A round's "no independent corroboration" blocker can retire on its own,
   for free, when a later SDK-object conversion round lands** -- this
   function's nine "local guess" callees all turned into real Psy-Q symbols
   without anyone touching this file, purely as a side effect of round 34's
   `libsnd` object placements elsewhere. Worth re-reading a stalled function's
   disassembly against current symbol names before re-deriving its call
   graph from scratch.
2. **A manual bounds check ahead of a `switch` is not free** -- if the
   `switch` itself provides the same guard via its `default` arm, writing
   BOTH produces two redundant range checks. Declare the switch's OWN full
   case range (even trailing no-op cases matching a dense jump table's
   duplicate slots) and let its lowering produce the single check.
3. **Whether an inner dispatch's controlling expression is written as
   `arg - N` or as `case N: ... case N+k:` with the raw variable changes
   instruction ORDER (subtract-then-widen vs widen-then-subtract) for a
   value GCC 2.6.3 would otherwise widen once and cache** -- prefer letting
   switch-lowering do the normalization when the case values permit it.
4. Extends `ContDataEntry.md`'s "dead-value kept live across a branch"
   finding with a case where HOISTING the computation (rather than avoiding
   it) is the fix: a value dead on exactly one of three converging paths,
   computed unconditionally before the branch, reproduces retail exactly;
   computing it only where used lets GCC's dead-code elimination remove it
   on the one path that doesn't need it, going 2 words short.

## ROUND 39 update (runner alpha): permuter-found simplification closes 8 of round 35's 16 words -- 163/179 -> 171/179

**This function's title was explicitly flagged in the round-39 work order as
"NEVER permuter-searched" — it now has been, and it found something real.**

Rebuilt round 35's preserved body first to reconfirm: clean build, 163/179,
zero out-of-range drift, matching the title exactly.

Set up a fresh `tools/setup-permuter.sh` scaffold from a minimal seed (only
the types/externs this function's own body needs -- `Scratch_800357B0`,
`AdsrFields`, the ten Psy-Q externs). Sanity check (`--debug
--stack-diffs`): **base score 453**, no stack differences reported as
anomalous, consistent with a clean small-residue scaffold. Searched
`-j 6 --stack-diffs --stop-on-zero --best-only` for the full 240s budget
(`timeout 240`, confirmed exit via the timeout rather than an external
kill): **no zero found in 33480 iterations**, but the search found and held
a new best of **score 48** (down from 453) within the first ~30 iterations
and never improved on it further for the rest of the run.

**The score-48 candidate, translated and verified through the real oracle
(not trusted at face value, per `docs/MATCHING-GUIDE.md`):** the only
structural change versus the round-35 seed is in case 12 -- the named local
`s32 t = arg6 - 0x40;` is dropped entirely, and `arg6 - 0x40` is recomputed
inline at its one real use (`if ((u32)(arg6 - 0x40) < 0x40) { resolved.unk10
= 1; }`) instead of being cached in `t` right after the dead-on-one-path
computation round 35's own lever hoisted. Applied to the real function and
rebuilt: **163/179 -> 171/179, length still EXACT (179/179), zero
out-of-range drift** -- a genuine 8-word gain, not a permuter-local
artifact. `asm-differ` confirms the 8 newly-closed words are the ENTIRE
case-12 block realigning to retail byte-for-byte; nothing else in the
function changed shape.

Why this works where round 35's own hoist did not fully match: round 35's
`s32 t = arg6 - 0x40;` reproduced retail's REDUNDANT COMPUTATION (the value
is computed even on the path that never reads it) but pinned it to a NAMED
register-resident local kept live across the branch -- which is one
register-allocation decision retail did not actually need, since retail's
own dead computation on that path is discarded immediately (never spilled
or kept live for the later use; each of the three converging paths gets its
OWN fresh computation of the same expression rather than one shared named
value). Removing the name and letting the one real use recompute the
expression inline reproduces that "no dead value is ever named" shape
directly.

**The remaining 8-word residue is unchanged from round 35's own diagnosis:**
the same `channel`($s2 retail / $s3 built)-vs-cached-raw-`arg5`($s3 retail /
$s2 built) two-way register-color swap, confirmed via `asm-differ` --
every flagged word is either the callee-save prologue pair or a `sll
a0,sN,0x10` re-widen before a call, exactly as round 35 described. **Tried
the mirror-image reshape of round 35's own negative result**: round 35
hoisted `channel` into an explicit `s16 ch = channel;` local (no effect);
this round hoisted the OTHER side instead -- `s16 sel = arg5;` used in place
of `arg5` for the INNER switch only (the raw-value re-read retail caches
into $s3) -- and rebuilt. **Also byte-identical, 171/179 unchanged, same 8
diff words.** Both directions of "reproduce the swap by hoisting one side
into a fresh named local" are now confirmed inert; this is CLAUDE.md's
register-identity STALL class on both counts, not a banned-fix target (no
`register T v asm("$N")` or extended-asm operand constraint attempted or
considered).

The permuter search's own floor (score 48, held for the remaining ~33450
iterations after the initial find) is consistent with this: 48 is what the
scorer assigns to exactly this 8-word register-rotation residue on its own,
with no further structural mutation in the permuter's own mutation set
(reordering, decl motion, cast/paren insertion, dead-arm duplication)
capable of reaching it. Not marking permuter-exhausted -- one search at one
seed shape is evidence about that search, not a certificate against a
future one with a different scaffold framing.

### Round 39 lever checklist (this function)

- **Hoist-both-before-either (this round's headline lever)**: **checked,
  does not apply.** This function's residues are a dead-value-naming choice
  (case 12, now fixed) and a whole-value register-color swap (channel/arg5,
  unresolved) -- neither is a case of two adjacent loads/multiplies whose
  consumers come later; there is no adjacent-load-pair shape here to hoist.
- **Permuter, explicitly flagged as never-run**: **run, and it found a real
  8-word gain** (case 12's dead-value-naming fix) plus a floor (score 48)
  matching the remaining register-color swap it could not reach.
- **Fresh-local-for-a-reused-value (round 38's lever 2)**: tried on `arg5`
  (the side round 35 had NOT yet tried), mirroring round 35's own `channel`
  attempt -- **also inert.** Both sides of this specific swap are now
  confirmed unreachable by this lever.

## Body as reached (171/179, length exact, register-identity residue only) -- supersedes round 35's 163/179 body above

Identical to round 35's body except case 12, which drops the named `t`
local entirely:

```c
        case 12: {
            if (arg6 == 0) {
                /* nothing -- falls to the shared check below */
            } else if (arg6 < 0x40) {
                resolved.unk10 = 0;
                break;
            }
            if ((u32)(arg6 - 0x40) < 0x40) {
                resolved.unk10 = 1;
            }
            break;
        }
```

Every other case, the outer/inner switch structure, and all ten cross-unit
calls are unchanged from round 35's body above (still current in
`src/code_179d8_k.c` as the preserved `#if 0` near-miss).

## Verification

`./build-and-verify.sh` build exit=0 with `Snd_setVabAttr` restored to
`INCLUDE_ASM` (near-miss body preserved in `src/` as `#if 0`, updated to
the round-39 171/179 version). `funcdiff.py Snd_setVabAttr` against the
near-miss build (prior to reverting): 171/179 words match, length exact,
zero out-of-range drift.

### Proposed learning (round 39)

5. **A hoisted "reproduce retail's redundant dead-value computation"
   fix (round 35's own lever, `ContDataEntry.md`'s family) can itself be
   one register-allocation decision too many.** Naming the hoisted value
   (`s32 t = ...;`) pins it to a register kept live across the branch;
   if retail's OWN redundant computation is never named or kept live (each
   converging path recomputes the same dead expression fresh, discarded
   immediately), dropping the name and inlining the expression at its one
   real use can close the remaining gap. Check whether a "hoist to
   reproduce the redundant computation" fix's own local is itself read more
   than once before assuming the hoist is finished.

## Round 46 update (runner charlie): declare-then-assign split lever, tried and NEGATIVE (regressive)

Rebuilt round 39's preserved body: reproduces exactly, 171/179, length
exact, the same `channel`/`arg5` two-way register-color swap.

This session found a genuinely new lever on a different function this same
round (`GetCdFileEntry.md`): a value declared with a combined
`T x = expr;` initializer can land in a different register CLASS than the
identical value declared, then assigned in a separate statement
(`T x; x = expr;`) -- confirmed there by permuter search, closing a
19-word function outright. Round 35/39 had already tried hoisting
`channel` into a fresh named local as a **combined** declare+init
(`s16 ch = channel;`, inert) but never as a **split** declare-then-assign,
so this looked like a legitimately untried axis on this function's own
register-color swap.

**Tried: `s16 ch; ch = channel;` at function entry, with every other use
of the `channel` parameter in the body replaced by `ch` (signature
unchanged).** Rebuilt and measured against the real oracle:
**161/179 -- ten words WORSE than the 171/179 baseline**, not neutral.
Reverted immediately (`git checkout -- src/code_179d8_k.c`,
`build-and-verify.sh` reconfirmed byte-exact with `INCLUDE_ASM` in place).

**Why this negative is informative rather than just a miss:** it bounds
the new lever's scope. `GetCdFileEntry`'s win was a value crossing two
call boundaries (lock/unlock) where the source's *timing* was already
confirmed correct and only the register *class* was wrong -- splitting
the declaration changed nothing about liveness, only how the allocator
saw the value's birth. This function's residue is a **whole-function
two-way color swap between two values that are BOTH already parameters**
(`channel` and the cached `arg5`), not a hoisted-before-a-call local vs.
its own late computation. Introducing a fresh local for one side of an
already-established parameter pair adds a new live range rather than
reshaping an existing one, which is presumably why it made the
allocator's job harder rather than nudging a class choice -- consistent
with round 35's and round 39's already-negative "fresh local, combined
form" results on both `channel` and `arg5` respectively. The declare-split
lever is now confirmed NOT to generalize to whole-function
register-color-swap residues; it is scoped to values whose combined
declare+init form was already independently suspected correct in every
respect except register class (as in `GetCdFileEntry`).

No further axis attempted this round; `INCLUDE_ASM` unchanged,
whole-image build re-verified byte-exact.

## Round 49 update (runner charlie): MATCHED, 179/179, byte-exact

Rebuilt round 39's preserved 171/179 body in isolation first, per this
round's "rebuild before trust" discipline: reproduces exactly, same
`channel`($s2 built)/`arg5`($s3 built) whole-function register-color swap
described above.

**Check 3, run fresh**, since this function's own title was flagged this
round as "never permuter-searched" (the round-39 search that found the
case-12 lever having predated the discriminator's naming). Built a fresh
scaffold from the current 171/179 body; base compiles. `--debug
--stack-diffs`: **base score 48** -- `Stack Differences: 8`, `Register
Differences: 8`, everything else 0. Exact match to round 39's own
scaffold figures at the point its search plateaued (48, held for
~33450 iterations after an early find). AGREE case; searched with
confidence.

**Search: `timeout 900 ... -j 8 --stop-on-zero --best-only`.** Round 39's
own search only ran 240s/33480 iterations against this same base; this
round's search was the first LONGER one against it. **Found a ZERO at
iteration 149** (rc=0, `Found zero score! Exiting.` -- not a timeout).

**The winning candidate:** cache `channel` into a fresh local (`s16
new_var;`) declared INSIDE the `case 4: ... case 14:` compound block,
assigned right after the `_SsUtResolveADSR` call (`new_var = channel;`),
and read back in place of `channel` at that same block's closing
`SsUtSetVagAtr(channel, slot, kind, &scratch);` call (now `SsUtSetVagAtr
(new_var, slot, kind, &scratch);`). Translated into `src/code_179d8_k.c`
verbatim and rebuilt through the full pipeline:

```c
    case 4:
    /* ... case 5..14 ... */
    {
        s16 new_var;

        _SsUtResolveADSR(scratch.adsr1, scratch.adsr2, &resolved);
        new_var = channel;
        switch (arg5) {
        /* ... unchanged ... */
        }
        _SsUtBuildADSR(&resolved, &scratch.adsr1, &scratch.adsr2);
        SsUtSetVagAtr(new_var, slot, kind, &scratch);
        return;
    }
```

**`./build-and-verify.sh`: whole-image SHA1 matches retail. `funcdiff.py
Snd_setVabAttr`: 179/179, no drift warning.** Byte-exact.

**Why this succeeds where rounds 35/39/46's "hoist `channel` (or `arg5`)
into a fresh named local" attempts all failed.** Every prior attempt
declared its fresh local at FUNCTION ENTRY, holding it live across the
*entire* function body (all 23 `case` arms) -- competing for a
callee-saved register against every other value live that broadly. This
candidate declares its local at the narrowest possible scope: inside the
one compound block (`case 4`..`case 14`) where `channel` needs to survive
exactly one call (`_SsUtResolveADSR`) before its next use. That is a much
smaller, block-local liveness question, and it is the one register
decision retail's own compiler actually had to make differently from
ours -- not "which register holds `channel` for the whole function" (a
question this residue's own diagnosis had correctly identified, but which
no whole-function-scope local had ever been able to answer the way
retail did) but "which register holds `channel` across this one call,
in this one arm". Scoping the fix to the exact liveness range the
residue lives in, rather than to the widest scope available, is what
closed it.

**A latent same-translation-unit prototype conflict, exposed by this
match, not caused by it.** `ContDataEntry` (elsewhere in this same file,
still a stall) carries its own still-preserved `#if 0` body with a local
guess at `Snd_setVabAttr`'s signature from the CALLER's side: a
three-way decomposition of the by-value argument blob (`u32 a3,
Blk1_800351D0 blk1, Blk2_800351D0 blk2`) that sums to the same 50 bytes
as this function's own real two-struct signature (`Scratch_800357B0
scratch` (0x20) + `AdsrFields resolved` (0x12)), just sliced at a
different byte boundary -- both are "correct" in the sense of matching
the retail stack layout, but they are TWO DIFFERENT extern declarations
of the SAME identifier in ONE translation unit, which C does not permit
regardless of ABI compatibility. This never surfaced before because
`Snd_setVabAttr` was always `INCLUDE_ASM` (no real definition to
conflict against). Making it live triggered `` conflicting types for
`Snd_setVabAttr' `` against `ContDataEntry`'s forward declaration.

This is NOT the same situation `CLAUDE.md`'s "independent local views"
convention covers (that convention is explicitly about views split
across DIFFERENT files/units, e.g. a shared struct read differently by
each caller's own `.c`) -- two conflicting prototypes for the same
identifier in ONE file are a hard C constraint violation, not a stylistic
choice. Resolved by REMOVING the stale forward declaration (dead code:
`ContDataEntry`'s own body that used it is itself inert, wrapped in `#if
0`, so nothing live referenced it) rather than trying to reconcile two
by-value struct decompositions that would require moving
`Scratch_800357B0`/`AdsrFields`'s typedefs earlier in the file.
Left an inline comment at the removal site for whoever next attempts
`ContDataEntry`: its own call site will need reshaping to the two-struct
signature, and a fresh forward declaration (declared after the two
struct typedefs, or with them duplicated/forward-declared earlier) added
back before that function can build again.

### Proposed learning

1. **Scope a "hoist into a fresh local" fix to the NARROWEST liveness
   range the residue actually needs, not to function-entry.** Three prior
   rounds (35, 39, 46) tried hoisting `channel` or `arg5` at the top of
   the function and all three failed identically; the winning shape
   hoists the exact same value but scopes the fresh local to one
   compound block and one call, matching the ACTUAL cross-call liveness
   window the register allocator has to solve. A function-entry hoist and
   a block-local hoist of the "same" value are different asks to the
   allocator, and a residue's own diagnosis (correctly naming WHICH value
   is misassigned) does not by itself tell you at what scope to
   reintroduce it.
2. **A stale same-file forward declaration for a still-`INCLUDE_ASM`
   sibling function can silently disagree with reality for as long as
   the real function stays unmatched, and the compiler only catches it
   the moment a real definition appears.** Any unit carrying more than
   one still-stalled function that calls another still-stalled function
   in the SAME file is carrying this risk; when matching either one,
   check for a forward declaration of it elsewhere in the same `.c`
   (`grep -n 'extern.*<func>' src/<unit>.c`) before assuming a clean
   build means no such declaration exists to conflict with.
