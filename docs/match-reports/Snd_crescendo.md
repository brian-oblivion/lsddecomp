# Snd_crescendo

> Renamed from `func_80036528` on 2026-09-23 (tools/rename.py). Address 0x80036528.

**Unit:** libsnd_cres · **Size:** 240 words (0x3C0 bytes) · **Status:**
**MATCHED, round 70 (bravo)** — 240/240, insertions 0 / deletions 0,
whole image `OK: build matches retail`.

REVISITED, round 70: MATCHED (from 13 words short); names/types used — the
two things that closed it were a parameter TYPE (`s16`, not `s32`) and a
callee-prototype TYPE (`u16` x/y), not an allocation lever.

## History

- **Round 25** (fresh): stalled 13 words SHORT (227/240), first real diff at
  the first instruction after frame setup (retail's `move $a3,$a0` argument
  register hop). Recorded two "levers" and an unresolved "a0/a1 hop".
- **Round 70** (bravo, revisit): matched. See below for what the round-25
  body got wrong; both of its recorded levers are withdrawn.

## Rebuilt-as-given figure (round 70, before any change)

The preserved round-25 body, rebuilt unchanged against the current pinned
toolchain (all four maspsx flags in): **still 13 words SHORT (227/240 built,
epilogue `jr` at 0x800368B0 against retail's 0x800368E0); funcdiff
`insertions 34 / deletions 34`, positional skeleton diffs 236, 3/240 raw
(window misaligned).** So the flags that landed after round 25 did not move
this body's length.

## What it does

Per-slot "beat" tick on `_ss_score[screen][slot]` (172-byte `SsScore`,
`Entry90902E8` before round 97). Decrements `unk98`; if the period `unk42 > 0` and
`unk98 % unk42 == 0`, decrements `unk40` and pushes an XY step (`+1`) through
`SpuVmGetSeqVol` (read) / `SpuVmSetSeqVol` (write), clamping to `(0x7F,0x7F)`
and clearing bit `0x10` of `unk90` when out of range or out of beats. If
`unk42 <= 0` the same happens with `unk40 += unk42` and a `- unk42` step.
Then, unless the modulo missed, clears bit `0x10` when `unk98 == 0` or
`unk40 == 0`. Always finishes by reading the XY into `entry->unk78/unk7A`.

## What closed it, in order of impact (each measured in-tree)

| step | change | result |
| --- | --- | --- |
| 0 | round-25 body as given | 227/240 (13 short), ins/del 34/34 |
| 1 | definition `(s32 a0, s32 a1)` -> `(s16 a0, s16 a1)`, drop the `(s16)` casts | 12 short; retail's `move a3,a0` ... `move s5,a3` hop reproduced exactly |
| 2 | local extern `SpuVmSetSeqVol(s16,s16,s16,s32)` -> `(s16,u16,u16,s32)`; `sp10/sp12` `s16` -> `u16`; drop round-25's `d1/d2` named locals | ins/del 17/17 |
| 3 | local struct view `unk40` `u16` -> `s16`, drop the `(s16)` casts on it | the `li v1,0xffff; addu` decrement became retail's `addiu -1` |
| 4 | control flow: the `unk98 == 0 || unk40 == 0` clear is OUTSIDE the `unk42 > 0` if/else, and the modulo miss is `goto end` (skips it) | **1 word short**, ins/del 2/2 |
| 5 | `s16 count` read AFTER the `unk98` store, guard and divisor on `entry->unk42` directly | exact length (236/240); retail's `lh a2` + `move v1,a2` pair appears |
| 6 | compute `entry = &_ss_score[a0][a1]` BEFORE `arr = &_ss_score[a0]` | 239/240; prologue `lui/addiu` now scheduled between `sll` and `sra 14`, as retail |
| 7 | delete `count`; the `<= 0` arm adds `entry->unk42` (permuter find, below) | last `addu v0,v0,v1` operand order fixed: **240/240, whole image green** |

Evidence for each type from the callee/caller bytes, not from guessing:

- **`s16` parameters**: retail sign-extends `a0`/`a1` at EVERY use
  (`sll 16; sra 14` for the index, `sll 16; sra 16` for the slot), holds the
  raw incoming `a0` in `a3` first and only moves it to callee-saved `s5`
  after the slot-offset arithmetic. That two-step is what GCC 2.6.3 emits
  for a HImode parameter (incoming SImode register copied to a pseudo, then
  promoted to a callee-saved home); an `s32` parameter goes straight to
  `s3`. Round 25 tried several restructurings of the index expressions and none
  could move it, because the cause was the declaration.
- **`SpuVmSetSeqVol(s16, u16, u16, s32)`**: retail masks both XY arguments
  `andi 0xFFFF` at the call; the callee (`code_179d8_l`, still
  `INCLUDE_ASM`) stores them with `sh` and re-reads with `lhu` +
  `sltiu 0x80`, i.e. unsigned. `libsnd_decre.c` already carried
  `(s16 a0, u16 a1, u16 a2, s32 a3)`.
- **`u16 sp10/sp12`**: retail reads them with `lhu` and compares
  `(x + 1) < 0x80` / `(x - unk42) < 0x80` with `slti` on the int-promoted
  value, never truncating to 16 bits before the compare.

## Both round-25 levers are WITHDRAWN

- **"Lever 1" (bind `sp10 - thresh` to named `s16 d1/d2` locals to fix a
  `vars=48` frame inflation)** was compensating for `s16` stack locals and
  an `s16`-parameter prototype. With `u16` locals and a `u16` prototype the
  plain inline `(sp10 - thresh) < 0x80 ... SpuVmSetSeqVol(..., sp10 - thresh,
  ...)` is the matching form and the frame comes out at retail's
  `vars=24` with no named temporaries.
- **"Lever 2" (an unsigned cast on a named `s16` local contaminates its
  load to `lhu`)** is real GCC behaviour but the wrong reading of THIS
  function: the `lhu` came from the `s16 count` local being used in a
  HImode add (`unk40 + count`), which only needs the low 16 bits. Retail has
  no such local at all; `unk42` is read directly in all three places.

The round-69 levers named in the brief were checked: the narrowed-callee
lever DID apply here, as a signedness rather than a width error (the
round-25 local extern said `s16` where the callee reads
`u16`, which cost a sign-extend pair per argument instead of an `andi`).
The dead-delay-slot lever did not apply: no delay-slot `move` in retail is
dead.

## Search

One bounded permuter search, run AFTER the in-tree body reached 239/240
(last residue: one `addu v0,v0,v1` operand order). Gate 3 checks run: (1)
scaffold compiled and scored, base score 10; (2) `--debug --stack-diffs`
insertions 0 / deletions 0; (3) real-build funcdiff `insertions 0 /
deletions 0` — AGREE. `timeout 2400`, `-j 6 --stop-on-zero`; exited rc 0 at
iteration 1326 with a zero whose only material change was deleting the
`count` local and reading `entry->unk42` in the `<= 0` arm. Translated and
verified in-tree: 240/240, whole image green. (Independently, an in-tree
variant keeping `count` and routing the add through a block-local
`s16 t = entry->unk40; entry->unk40 = t + count;` also matched byte-exact;
the permuter's form is adopted because it has fewer names, not because it
scores better.)

## Struct knowledge (the record, `Entry90902E8` until round 97, now Sony's `SsScore`)

`unk40` is `s16`, not `u16` (retail decrements with `addiu -1` and tests the
sign with `sll 16; bltz`/`lh`). Only this function reads it in this unit;
whole-image oracle green after the edit.

### Proposed learning

**A "parameter hops through an argument register before reaching its
callee-saved home" residue (`move $a3,$a0` ... later `move $s5,$a3`) is the
signature of a NARROW (`s16`/`s8`) PARAMETER DECLARATION, not an allocation
choice.** Discriminator: every use of the parameter re-sign-extends it
(`sll 16; sra N`) and the source you have writes `(s16)a0` casts on an
`s32` parameter. Retype the definition, drop the casts. Round 25 spent a
whole derivation on index-expression restructurings that could not touch
it; retyping took 13-short to 12-short and unlocked every later step.
Corollary: when a stall report's "lever" is a named temporary that fixes a
frame-size inflation, re-check the types of the values it names before
trusting it — here both round-25 levers were compensations for wrong types.

## Round 97 (echo, track 6): the record is Sony's `SsScore`

The unit's local 0xAC-byte view `Entry90902E8` is deleted; the function now
uses `SsScore` from `include/SsScore.h`, the record behind libsnd's
`_ss_score` (named from Sony's variable, as no libsnd internal header ships
on any disc). The type is Sony's because only Sony functions read it: this
function is library code by the `config/psyq-objects.ld` pin, and the other
readers are libsnd (`Snd_setvol_data`/`Snd_SetCres` in the linked
`libsnd/vol`, `SpuVmGetSeqVol`/`SpuVmSetSeqVol` in `code_179d8_l`).
`tools/sonydata.py` reports no game-named Sony data. The fields keep their
offset names, per that header's rule; the fields this function reads
(`unk3E`, `unk40`, `unk42`, `unk78`, `unk7A`, `unk90`, `unk98`) were added to
`SsScore` in place of padding with the same types the matched body needs,
their mechanics taken from `lib/libsnd/vol.o`:

- `Snd_setvol_data(access, seq, vol, v_time)` stores `vol` into `+0x3E` and
  `+0x40`, `v_time` into `+0x94` and `+0x98`, and `+0x42` = `v_time / |vol|`
  when `|vol| < v_time`, else `-(|vol| / v_time)`; it does nothing while
  `+0x90` has `0x4` or `0x100` set, or when `vol == 0`.
- `Snd_SetCres` does the same, then sets `+0x90 |= 0x10` and clears `0x20`.

So `unk42 > 0` is ticks per one-step rise and `unk42 < 0` is steps per tick,
which is exactly the two arms of this function. Zero bytes changed.

Also found: the libsnd `cres` object DOES ship, as `cres.o` defining
`Snd_crescendo` on the 3.0 and 3.3 discs (renamed `_SsSndCrescendo` on 3.5
and 3.6), but neither build is retail's (text 0x474 and 0x57C bytes against
retail's 0x3C0), so it stays carried as C.

## Unit history (moved from the code_179d8_f.c banner, round 97)

ROUND 42 CORRECTION (2026-09-15) -- READ BEFORE ANY "BLOCKED" LINE BELOW:
every claim in this comment that a function is BLOCKED by `gp_rel`,
`nop_mflo_mfhi` or `addiu_at` is STALE.  All three constructs are RESOLVED
by pinned maspsx flags (CLAUDE.md, "Open toolchain blockers");
`tools/nearmiss.py` reports them tagged (RESOLVED-not-a-blocker) and counts
none of them.  Any "do NOT spend attempts on these" directive below is
therefore RETRACTED: those functions are ordinary matching work, and most
carry a mechanism-correct partial derivation already.  The rest of this
comment still stands -- only the blocker verdicts are withdrawn.
Screen: `python3 tools/nearmiss.py`, round 43 (2026-09-15).

code_179d8_f -- what is LEFT of functions 256..273 of the original
274-function code_179d8 monolith after round 34 gave sixteen of its
eighteen functions back to Sony.  Now 0x26D28..0x270E8
(vram 0x80036528..0x800368E8), a ONE-function unit.

ROUND 34 (2026-09-12): the unit's whole PREFIX, 0x2673C..0x26D28, is six
linked `libsnd` objects -- `adsr` (3.3), `ut_rev` (3.3), `ut_sva` (3.3),
`vm_don` (3.3), `next` (3.5), `vm_doff` (3.3) -- covering thirteen
functions:
  0x80035F3C _SsUtResolveADSR       (was matched C)
  0x80035F98 _SsUtBuildADSR         (was a 35w INCLUDE_ASM stall)
  0x80036024 SsUtReverbOn           (was matched C)
  0x80036044 SsUtReverbOff          (was matched C)
  0x80036064 SsUtSetReverbType      (was matched C)
  0x80036108 SsUtGetReverbType      (was matched C)
  0x80036118 SsUtSetReverbDepth     (was matched C)
  0x800361B0 SsUtSetReverbFeedback  (was matched C)
  0x800361F0 SsUtSetReverbDelay     (was matched C)
  0x80036230 SsUtSetVagAtr          (was matched C, 115w)
  0x800363FC SpuVmDamperOn          (was matched C)
  0x80036410 _SsSndNextSep          (was matched C)
  0x80036518 SpuVmDamperOff         (was matched C)
Their C is DELETED, not commented out.  Twelve of them were matched as game
code and were Sony's the whole time; reclassifying them is the correction
CLAUDE.md asks for, not a regression.  Do not write C for any of them again
-- `python3 tools/sdkstalls.py` and
`.venv/bin/python3 tools/psyq_sdk.py coverage` are the evidence.

That was a pure PREFIX trim, so this unit kept its name and its `c` line
simply moved to 0x26D28.

A SECOND RUN in the same round then took the other end.  `libsnd/stop`
(3.3, 0x270E8..0x272A8) is `Snd_stop`, `SsSeqStop` and `SsSepStop` --
func_800368E8/A54/A7C, all three previously MATCHED as C and all three now
deleted from here.  It sat in the MIDDLE of what the prefix trim had left,
so the slice became [c][o][c] and the tail half became the one-function
unit `src/libspu_s_ih.c` (SpuInitHot).  Nothing moved with it: this
unit never owned a rodata attach.

WHAT IS LEFT OF THIS UNIT IS ONE FUNCTION, Snd_crescendo.

Owns NO switch jump table (zero `jtbl_` in its disassembly, and the splat
yaml's rodata slot list names no `.rodata, code_179d8_f` line), so no
rodata attach, before or after the split.

BLOCKER PROFILE: screen with `python3 tools/nearmiss.py`, never by
re-implementing the greps and never for `addiu_at` (resolved round 21).
Snd_crescendo (240w) is this unit's ONLY function and is MATCHED (round
70); docs/match-reports/Snd_crescendo.md has the derivation.

Expect low-level driver-shaped code rather than class-framework code, as
elsewhere in code_179d8; confirm with tools/classtable.py, do not assume.

## File name (round 97, charlie, track 8)

`code_179d8_f.c` became `src/libsnd_cres.c` (`tools/unitfile.py rename`,
image byte-identical). Evidence: `tuboundary.py --unit code_179d8_f` reports
`code_179d8_f (after sony:libsnd/vm_doff): start edge possible`, and the yaml
places `libsnd/stop` directly after it, so both edges are Sony objects and
the region is one file. `nm` over `sdk/work/<disc>/elf/libsnd/cres.o` gives
one text symbol on every disc (`Snd_crescendo` on 3.0/3.3,
`_SsSndCrescendo` on 3.5/3.6), text 0x474 / 0x57C / 0x2DC / 0x2DC against
retail's 0x3C0: the module is `cres`, and no disc holds retail's build.
