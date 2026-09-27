# SsUtChangeADSR -- MATCHED 61/61 (head adjudication: a lever-stacking stall)

> Renamed from `func_80031BA4` on 2026-09-24 (tools/rename.py). Address 0x80031ba4.

Unit `libsnd_vmanager`. Round 22. Runner alpha reached 57/61 and filed this as a
scheduling stall, concluding that **"stack-passed argument loads sit outside
ordinary scheduling-barrier reach"** -- i.e. that the four-instruction
block-reorder at the top of the function was not source-reachable. That
conclusion is wrong, and the way it is wrong is worth more than the function.

`s32 SsUtChangeADSR(s16 idx, s16 p1, s16 p2, s16 p3, u16 p4, u16 p5)` -- a leaf
that bounds-checks `idx` against 0x18, compares three parallel 52-byte-stride
tables against `p1`/`p2`/`p3`, and on a full match writes `p4`/`p5` into two
16-byte-stride tables and ORs `0x30` into a flag byte.

## What alpha reported as unreachable

Retail's first five instructions, with the two stack-passed arguments loaded
*between* the bounds test and the branch that consumes it:

```
addiu $sp, $sp, -0x8
andi  $v0, $a0, 0xFFFF
sltiu $v0, $v0, 0x18
lhu   $t1, 0x18($sp)      <- p4
lhu   $t2, 0x1C($sp)      <- p5
beqz  $v0, .L80031C88
 sll  $v0, $a0, 16
```

## The reproducer, which took one try

Written as plain C -- no barriers, no named intermediates, no frame trick --
through the pinned pipeline:

```c
s32 probe(s16 idx, s16 p1, s16 p2, s16 p3, u16 p4, u16 p5) {
    if ((u16)idx < 0x18) {
        if (D_8008D99E[idx].unk0 != p1) return -1;
        if (D_8008D99A[idx].unk0 != p2) return -1;
        if (D_8008D994[idx].unk0 != p3) return -1;
        D_8008D7F8[idx].unk0 = p4;
        D_8008D7FA[idx].unk0 = p5;
        _svm_sreg_dirty[idx] |= 0x30;
        return 0;
    }
    return -1;
}
```

```
andi  v0,a0,0xffff
sltiu v0,v0,24
lhu   t1,16(sp)
lhu   t2,20(sp)
beqz  v0,e0
```

**The ordering alpha spent its budget on was already correct in the most naive
possible C.** The only difference from retail is the missing empty 8-byte
frame, which is exactly why the two `lhu` offsets read 0x10/0x14 instead of
0x18/0x1C -- one displacement, not a reordering.

## The fix

Three levers, all of them alpha's own, applied WITHOUT the extras it stacked on
top:

1. `s32 dead[2];` plus an unreachable `if (0) { dead[0] = 1; }` -- alpha's own
   documented idiom for forcing an empty 8-byte frame (as opposed to a dead
   *call*, which forces 0x10). This shifts both stack-argument displacements by
   8 and reproduces the `addiu $sp, $sp, -0x8` prologue.
2. A bare `__asm__("")` between the `D_8008D7F8` and `D_8008D7FA` stores.
3. A second one between `D_8008D7FA` and the `_svm_sreg_dirty |= 0x30`
   read-modify-write.

Barriers 2 and 3 are order-only and permitted (removing them changes
instruction ORDER, not which register holds anything). With the frame idiom and
those two barriers on the naive body: **61/61, whole-image SHA1 green.**

## Adjudication: levers compose destructively

Alpha had every lever it needed. What it also had, in the 57/61 body, was:

```c
    __asm__("");            /* a THIRD barrier, at the top */
    cond = (u16) idx;       /* named intermediates for the guard */
    v4 = p4;                /* and for both stack arguments */
    v5 = p5;
```

Those four lines are what moved the two `lhu`s. Naming a stack argument in a
local gives the value a pseudo with a different live range, and the top barrier
pins a scheduling boundary in front of the guard -- between them they
reconstructed the prologue alpha then concluded was unreachable. Its final
diagnosis was a description of damage its own earlier fixes had done.

**The generalisable rule, and the reason this is filed as an adjudication
rather than a match: when a residue MOVES after you add a lever, the next step
is to REMOVE the earlier levers and re-measure, not to add another.** Each
lever here was individually sound and individually documented; the failure was
purely additive. A body carrying three barriers and three named intermediates
is no longer testing a hypothesis about the function, it is testing the
interaction of six edits, and the residue it reports is attributable to none of
them.

The cheap discipline that catches it, and which cost under a second here:
**before concluding any residue is source-unreachable, compile the NAIVE body
in isolation through the pinned pipeline and look at what it actually
produces.** CLAUDE.md already requires this of a toolchain escalation ("never
escalate a lead you have not tried and failed to reproduce in isolation"). This
report is the argument for extending it to a *scheduling-class stall*, which is
the other verdict that closes a function to future rounds: alpha's isolation
test would have shown the ordering correct on line one, and the whole
"outside barrier reach" hypothesis would never have been formed.

Alpha's other three learnings are unaffected and stand -- they came from
functions it matched.

## Final body

Lives in `src/libsnd_vmanager.c` in ROM order.

### Proposed learning

**Levers do not commute, and a moved residue is a signal to subtract.** Bare
`__asm__("")` barriers, named intermediates and dead-local frame tricks each
change scheduling; stacked, they interact, and the resulting diff is not
evidence about the function. Add one lever, measure, and if the residue moves
rather than shrinks, revert before trying the next. **Corollary: a
"source-unreachable scheduling residue" verdict requires an isolation
reproducer of the NAIVE body**, on the same standard CLAUDE.md already sets for
a toolchain escalation -- here the naive body reproduced the supposedly
unreachable ordering on the first attempt, and only the frame displacement was
ever really missing.

## Track 2 (round 86, 2026-09-26, alpha)

The per-field symbols this report names (`D_8008D988`..`D_8008D9BA` at a 0x34 stride, and the twelve game names over +0x1C..+0x33 that earlier rounds gave `D_8008D9A4`..`D_8008D9BA`) are ONE Sony table: libsnd/vmanager.o (disc 3.5) bss puts `_svm_voice` at +0x198 of the block anchored at 0x8008D7F0, so `_svm_voice` = 0x8008D988, 24 voices x 0x34 = 0x4E0 bytes, ending exactly at `_svm_envx_ptr`. The symbols file now carries `_svm_voice` (size:0x4E0); the record type is `include/SvmData.h` (fields by offset only, Sony's rule). Field map: +0x00 `unk00` (was `D_8008D988`), +0x02 `unk02` (`D_8008D98A`), +0x04 `unk04` (`D_8008D98C`), +0x06 `unk06` (`D_8008D98E`), +0x08 `unk08` (`D_8008D990`), +0x0A `unk0A` (`D_8008D992`), +0x0C `unk0C` (`D_8008D994`), +0x0E `unk0E` (`D_8008D996`), +0x10 `unk10` (`D_8008D998`), +0x12 `unk12` (`D_8008D99A`), +0x14 `unk14` (`D_8008D99C`), +0x16 `unk16` (`D_8008D99E`), +0x18 `unk18` (`D_8008D9A0`), +0x1B `unk1B` (`D_8008D9A3`), +0x1C..+0x26 `unk1C`..`unk26` (the SeAutoVol/SetAutoVol ramp: active, step, interval, countdown, accum, limit; `D_8008D9A4`..`D_8008D9AE`), +0x28..+0x32 `unk28`..`unk32` (the SeAutoPan/SetAutoPan ramp, same order; `D_8008D9B0`..`D_8008D9BA`). Preserved bodies in this report keep the per-address `D_` spellings, which still link (except `D_8008D988`, which is now `_svm_voice` itself) (splat keeps them as auto-symbols, since the table lies past the global segment's vram range and splat does not fold them into `_svm_voice`).

`src/` now compares `_svm_voice[idx].unk16/unk12/unk0C`. Byte-exact.

**_svm_sreg_buf / _svm_sreg_dirty (same round).** `D_8008D7F0` (0x180 bytes, 24 voices x 0x10, halfwords at +0x0..+0xA spelled `D_8008D7F0`..`D_8008D7FA` by splat) is Sony's `_svm_sreg_buf` and `D_8008D970` (24 bytes) is `_svm_sreg_dirty`: libsnd/vmanager.o bss +0x000 and +0x180, anchored at 0x8008D7F0. Both are in the symbols file; the record type is `SvmSreg` in `include/SvmData.h` (fields by offset). `src/` now stores `_svm_sreg_buf[idx].unk8`/`.unkA` in place of the two "independent arrays" `D_8008D7F8`/`D_8008D7FA`: byte-exact. The earlier reason for modeling them apart (each access computes its own address) is reproduced by the one struct as well, with this function's existing `__asm__("")` order barriers left in place.

## asm sites

Round 89 (runner delta, track 5 `asm-sites`): both bare `__asm__("")`
barriers (levers 2 and 3 above, now between the `_svm_sreg_buf[idx].unk8` and
`.unkA` stores and before the `_svm_sreg_dirty[idx] |= 0x30`) are **retired**.
Measured one at a time: deleting the first alone, then the second as well, each
rebuild left `build/src/code_179d8_j_c.c.o` byte-identical to the object built
with both (`cmp`), and `./build-and-verify.sh` stayed green both times. In the
current source the order no longer depends on them (what changed since they
were needed was not measured; the `SvmSreg` retype above is a candidate); lever 1 (the `dead[2]` frame idiom) was not touched. `SsUtChangeADSR`
now carries no `__asm__`.

## Round 97 (bravo, track 6)

The definition now takes <libsnd.h>'s prototype: return type `s32` became `s16` (Sony's `short`). Zero bytes.
The in-source comment on `dead` shrank to a MATCHING line; its content was:
`dead` is never read and the write is unreachable; it exists to make GCC
allocate retail's empty 8-byte frame, which puts the two stack-passed
arguments at 0x18/0x1C($sp) instead of 0x10/0x14 -- the frame is the ONLY
thing the idiom is for, and adding anything else on top of it breaks the
scheduling.
