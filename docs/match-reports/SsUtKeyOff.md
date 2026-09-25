# SsUtKeyOff -- MATCHED (135/135 words)

> Renamed from `func_80031280` on 2026-09-24 (tools/rename.py). Address 0x80031280.

Unit `code_179d8_j`, round 26 (2026-09-09). Not a class method. Sibling of
`SsUtGetDetVVol`/`SsUtSetDetVVol`/`SsUtSetVVol` (same 0x18-entry bounds check
idiom, same busy-lock reentrancy guard as `SsUtKeyOn`/`SsUtKeyOffV`'s
class) -- this one is a "stop a channel" operation: it validates the
caller's (p1,p2,p3,p4) against the four cached config records for `idx`,
then either clears the channel's active-mask bits or (if it was already in
the "0xFF" idle state) clears a fixed slot-25 pair of fields on the shared
`D_8006DAD4` table instead.

## What it is

```c
s32 SsUtKeyOff(s16 idx, s16 p1, s16 p2, s16 p3, s16 p4)
{
    u16 chan;
    u32 mask0;
    u16 mask1;

    if (_snd_ev_flag == 1) {
        goto fail_nolock;
    }
    _snd_ev_flag = 1;
    if ((u16) idx >= 0x18) {
        goto fail;
    }
    if (D_8008D99E[idx].unk0 != p1
     || D_8008D99A[idx].unk0 != p2
     || D_8008D99C[idx].unk0 != p3
     || D_8008D994[idx].unk0 != p4) {
        goto fail;
    }
    if (_svm_voice[idx].unk0 == 0xFF) {
        D_8008D9A3[(u8) idx].unk0 = 0;
        D_8008D98C[(u8) idx].unk0 = 0;
        D_8006DAD4[25].unk4 = 0;
        D_8006DAD4[25].unk6 = 0;
    } else {
        D_8008EA26 = idx;
        chan = D_8008EA26;
        if (chan < 0x10) {
            mask0 = 1 << chan;
            mask1 = 0;
        } else {
            mask0 = 0;
            mask1 = 1 << (chan - 0x10);
        }
        D_8008D9A3[chan].unk0 = 0;
        D_8008D98C[chan].unk0 = 0;
        _svm_voice[chan].unk0 = 0;
        D_80090C60 = mask0 | D_80090C60;
        D_80090C64 |= mask1;
        D_8008E228 &= ~D_80090C60;
        D_8008E22C &= ~D_80090C64;
    }
    _snd_ev_flag = 0;
    return 0;

fail:
    _snd_ev_flag = 0;
fail_nolock:
    return -1;
}
```

## Struct/global additions (this unit's local view only)

- `EntryDAD4` (shared with `SsUtGetDetVVol`/`SsUtGetVVol`) gained two new
  named fields at +0x4/+0x6 (`unk4`, `unk6`) in place of part of its pad --
  read/written **only** at the fixed literal index 25, never through the
  0..0x17 loop index the other accessors use. Total struct size (0x10) and
  the existing `unk0`/`unk2` offsets are unchanged, so this is safe for the
  already-matched sibling per CLAUDE.md's "struct edits are non-local" rule
  (confirmed: whole-image build stays green).
- `Rec34Half` (`_svm_voice`, `D_8008D98C`) retyped from `u16 unk0` to
  `s16 unk0` -- neither array was read by any already-matched function in
  this unit yet (only written, via `sh`, which doesn't care about
  signedness), so this was a free retype. It matters here because
  `SsUtKeyOff` compares `_svm_voice[idx].unk0` against `0xFF` with a
  genuinely *signed* `lh`, not `lhu` -- confirmed from the `.s` opcode
  encoding (top 6 bits `100001` = `LH`), not inferred.
- New sibling array `D_8008D98A[]`, same `Rec34Half` shape, 2 bytes after
  `_svm_voice` and 2 bytes before `D_8008D98C` -- a third member of the
  "several unrelated top-level symbols 2 bytes apart" convention already
  documented for the `D_8008D994` group. Declared but not used by this
  function (found via `SsUtKeyOn`'s disassembly while scoping the
  slice; only declared here for completeness, not yet exercised by C).

## Key findings (all measured, not inferred)

### 1. `D_8006DAD4[25]` is a fixed literal index, not `idx`

The tail of the `==0xFF` branch does `sh $zero, 0x194($v0)` / `sh $zero,
0x196($v0)` where `$v0` holds the bare `D_8006DAD4` pointer value with NO
index-register addition. `0x194 = 25*0x10 + 0x4` and `0x196 = 25*0x10 +
0x6` are compile-time constants, i.e. `D_8006DAD4[25].unk4/unk6` -- entry
25 is a fixed "shared/global" slot in the same table the 0..0x17 loop
index uses elsewhere, addressed directly because 25 is a literal, not
`idx`. This explains an otherwise-mysterious "struct pointer dereferenced
with no index" pattern in the `.s`.

### 2. Guard clauses need an explicit shared-tail `goto`, not four separate `return -1;`

The busy-lock check, the bounds check, and the 4-way field-mismatch OR-chain
all target ONE shared fail block in retail (`L80031488`/`L80031490`, the
former falling into the latter). Writing four independent
`{ _snd_ev_flag = 0; return -1; }` bodies let GCC 2.6.3 tail-merge only SOME
of them (3 of 4 OR-chain arms merged into a shared far block; the 4th,
being immediately followed by the large `if/else` body, got its consequent
inlined instead, and the very first busy-lock check similarly got its
`return -1` treated as the "close" fallthrough with the real body pushed
behind an explicit `j`). Writing the four failure sites as `goto fail;` /
`goto fail_nolock;` into one literal shared tail at the bottom of the
function reproduced retail's exact branch polarity and shared target on
the first attempt -- this is the general fix for the "GCC only tail-merges
some but not all textually-identical early returns" symptom, not just a
one-off for this function.

### 3. The `==0xFF` branch re-derives the array offset from a BYTE mask, not the sign-extended index

Outside the `==0xFF` branch, every prior array access indexes with `idx`
sign-extended 16-to-32 (`sll`+`sra` by 16). Inside the `==0xFF` branch,
retail recomputes the same `idx*0x34` stride offset from `(u8) idx`
(`andi $v0, $t0, 0xff`, no sign extension) instead of reusing the
already-computed offset register. Writing `D_8008D9A3[(u8) idx]` /
`D_8008D98C[(u8) idx]` explicitly (rather than plain `D_8008D9A3[idx]`,
which the compiler CSE'd into the earlier offset and dropped 6
instructions) reproduced this exactly. Likely explanation: since `idx` is
already bounds-checked `< 0x18` by this point, the source author (or GCC,
prompted by some now-invisible declaration) used a narrower-width access
here; we don't know why, only that the byte-mask recompute is what retail
does.

### 4. The `else` branch's three table writes use the RE-READ global (`chan`), not the parameter (`idx`)

Confirmed via a length mismatch that pointed straight at it: writing
`D_8008D9A3[idx]` (the parameter) in the `else` branch compiled the offset
via a fresh 16-to-32 sign-extend of the ORIGINAL `idx` register, one
instruction longer than retail's version, which reuses the `u16`
(zero-extended, `andi ...,0xffff`) value already sitting in the register
that held the just-written-then-reread `D_8008EA26` (`chan` in this C).
This is the same "written as a side effect and then re-read from the
global, not from the parameter" idiom this file's own header comment
already documents for `D_8008EA22`/`D_8008EA26` -- it applies to
`SsUtKeyOff`'s array-index use too, not just to the scratch value
itself.

### 5. `mask1` needed to be `u16`, not `u32` -- found by the permuter, not by reasoning

After points 1-4 the function reached 134/135 words, with the single
remaining residue a commutative-operand-order swap on one `or` instruction
(`or a0,a1,a0` retail vs `or a0,a0,a1` built) -- textbook "redundant
commutative operand order" residue per MATCHING-GUIDE. Manually swapping
the C expression to `D_80090C64 = mask1 | D_80090C64;` (matching the sibling
fix that DID work for the other mask/global pair, `D_80090C60`) instead
**regressed the WHOLE function** to garbage (register renaming from the
very first instruction, `t1` at the top shifting to a different register)
-- a strong signal this was the wrong lever entirely, not a partial fix,
matching the project's documented pattern that a register-identity residue
resists source reordering. `tools/setup-permuter.sh` + a 90-second, ~2800-
iteration `-j 6 --stack-diffs` search found a genuine ZERO
(`output-0-1/score.txt` = `0`) whose diff was a SINGLE type change:
`mask1` declared `unsigned short` (`u16`) instead of `u32`. Applying that
(keeping the expression `D_80090C64 |= mask1;`, NOT swapping operand
order) reached 135/135 and the whole-image SHA1 passed on the first
verify. `mask0` stays `u32` -- only `mask1` needed the narrower type; this
was almost certainly the actual source width (both fields are ultimately
stored into 16-bit globals, and only one of the two local temporaries
happened to matter for the codegen path taken here).

## Attempts

7 manual reshapes (guard-clause goto restructure; `(u8)` index cast inside
the `==0xFF` arm; `chan`-vs-`idx` indexing fix in the `else` arm; several
failed manual attempts at the mask0/mask1 operand-order and declaration-
order axis) plus one ~90-second / ~2800-iteration permuter run that found
the type-width fix. Well under the 30-attempt cap.

### Proposed learning

**A permuter-found "zero" is not always an expression reshape -- it can be
a plain local-variable TYPE change** (here, `u32` -> `u16` on one of two
otherwise-symmetric mask locals). Manual attempts fixated on operand ORDER
(matching the sibling fix that worked for `mask0`/`D_80090C60`) and every
one of those regressed the whole function; the actual fix left the
expression form (`|=`) untouched and narrowed the type instead. Worth
widening the axes considered for a stubborn single-`or`-operand residue:
declared width of the two operands, not just their written order, especially
when the two operands are NOT symmetric in the source (one is a bare global
already known to be `u16`, the other a local the decompiler guessed as
`u32` by default).

## Track 2 (round 86, 2026-09-26, alpha)

The per-field symbols this report names (`D_8008D988`..`D_8008D9BA` at a 0x34 stride, and the twelve game names over +0x1C..+0x33 that earlier rounds gave `D_8008D9A4`..`D_8008D9BA`) are ONE Sony table: libsnd/vmanager.o (disc 3.5) bss puts `_svm_voice` at +0x198 of the block anchored at 0x8008D7F0, so `_svm_voice` = 0x8008D988, 24 voices x 0x34 = 0x4E0 bytes, ending exactly at `_svm_envx_ptr`. The symbols file now carries `_svm_voice` (size:0x4E0); the record type is `include/SvmData.h` (fields by offset only, Sony's rule). Field map: +0x00 `unk00` (was `D_8008D988`), +0x02 `unk02` (`D_8008D98A`), +0x04 `unk04` (`D_8008D98C`), +0x06 `unk06` (`D_8008D98E`), +0x08 `unk08` (`D_8008D990`), +0x0A `unk0A` (`D_8008D992`), +0x0C `unk0C` (`D_8008D994`), +0x0E `unk0E` (`D_8008D996`), +0x10 `unk10` (`D_8008D998`), +0x12 `unk12` (`D_8008D99A`), +0x14 `unk14` (`D_8008D99C`), +0x16 `unk16` (`D_8008D99E`), +0x18 `unk18` (`D_8008D9A0`), +0x1B `unk1B` (`D_8008D9A3`), +0x1C..+0x26 `unk1C`..`unk26` (the SeAutoVol/SetAutoVol ramp: active, step, interval, countdown, accum, limit; `D_8008D9A4`..`D_8008D9AE`), +0x28..+0x32 `unk28`..`unk32` (the SeAutoPan/SetAutoPan ramp, same order; `D_8008D9B0`..`D_8008D9BA`). Preserved bodies in this report keep the per-address `D_` spellings, which still link (`D_8008D988` itself now reads `_svm_voice` above) (splat keeps them as auto-symbols, since the table lies past the global segment's vram range and splat does not fold them into `_svm_voice`).

`src/` now reads `_svm_voice[idx].unk16/unk12/unk14/unk0C/unk00` (all `s16`, as the old views) and clears `unk1B/unk04/unk00`. Byte-exact.
