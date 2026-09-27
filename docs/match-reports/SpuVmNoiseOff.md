# SpuVmNoiseOff -- MATCHED (49/49 words)

> Renamed from `ClearNoiseVoices` on 2026-09-24 (tools/rename.py). Address 0x8002f2a4.

> Renamed from `func_8002F2A4` on 2026-09-20 (tools/rename.py). Address 0x8002f2a4.

Unit: `src/code_179d8_m.c`. Round 24, runner bravo.

## Result

Byte-exact. `./build-and-verify.sh` green (`build exit=0`, whole-image SHA1
matches). `funcdiff.py`: `49/49 words match (file 0x1FAA4-0x1FB68)`, no
out-of-range drift.

## Final C

```c
typedef struct {
    u8 unk0; /* +0x0 */
    u8 pad1[0x34 - 0x1];
} Rec34Byte;
extern Rec34Byte D_8008D9A3[];

typedef struct {
    u16 unk0; /* +0x0 */
    u8 pad2[0x34 - 0x2];
} Rec34Half;
extern Rec34Half D_8008D98C[];

typedef struct {
    u8 pad[0x194];
    u16 unk194; /* +0x194 */
    u16 unk196; /* +0x196 */
} SpuRegs;
extern SpuRegs *D_8006DAD4;

void SpuVmNoiseOff(void) {
    s16 i;

    for (i = 0; i < D_8008E9D0; i++) {
        if (D_8008D9A3[i].unk0 == 2) {
            D_8008D9A3[(u8) i].unk0 = 0;
            D_8008D98C[(u8) i].unk0 = 0;
            D_8006DAD4->unk194 = 0;
            D_8006DAD4->unk196 = 0;
        }
    }
}
```

## Shape

Frameless leaf, no arguments. `for (i = 0; i < D_8008E9D0; i++)` sweeps a
table of `D_8008E9D0` "channel" slots; any slot in state `2` gets reset:
its own state byte and a companion halfword cleared, plus two halfword
fields on a separate "current object" cleared unconditionally inside the
same `if`.

`D_8008D9A3` and `D_8008D98C` are the same 0x34 (52)-byte-stride
channel-configuration record family `code_179d8_j.c` already documents
(`Rec34Byte`/`Rec34Half` there) -- redeclared here as this unit's own local
view per the project's multiple-independent-local-views convention (no
shared header). `D_8006DAD4` is the same symbol `code_179d8_j.c` reads as
`EntryDAD4 *` (an array of 0x10-byte records indexed by channel); this
function instead reads two FIXED offsets (`+0x194`, `+0x196`) off the same
pointer's value with no index scaling at all -- a different reading of the
same base pointer, so it gets its own local type (`SpuRegs`, padded out to
0x194 bytes) rather than reusing `code_179d8_j.c`'s `EntryDAD4`.

## The one snag: two width truncations of the same loop variable

Retail computes the `D_8008D9A3`/`D_8008D98C` byte offset (`i * 0x34`)
**twice** inside the `if` body -- once implicitly reused from the READ
(`i` sign-extended via `sll 16`/`sra 16`, matching a plain `s16 i`), and
once **recomputed from scratch** for the two WRITEs using a completely
different narrowing: `andi $v0, $a0, 0xff` (zero-extend as a byte), not
another sign-extending shift pair. The first attempt (plain
`D_8008D9A3[i].unk0 = 0;` with no cast) let GCC 2.6.3 reuse the
already-computed offset register from the read -- CSE across the branch,
6 fewer instructions than retail, everything after it shifted (237500
bytes of drift, in-range score 20/49).

**What worked: cast the loop variable to `(u8)` at the two WRITE sites
only, leaving the READ site (the `if` condition) uncast.** That single
change reproduced retail's exact two-different-narrowings shape and closed
the function outright, first try. The two sites are reading the same `i`
with two different declared-width VIEWS in source (`s16` at the
comparison, `u8` at the assignment), and GCC 2.6.3 does not CSE the
address computation across that width change -- it regenerates it with the
narrower cast's own extension code.

### Proposed learning

**A recomputed-vs-reused array-offset residue (same index, address
recomputed from scratch with a DIFFERENT narrowing than the first
occurrence) is diagnostic of two different WIDTH VIEWS of the same
variable at the two source sites, not two different variables.** Casting
the loop/index variable to a narrower type at only the SECOND (or later)
occurrence -- while leaving earlier occurrences uncast -- reproduces this
directly: GCC 2.6.3 does not CSE an array address across a change in the
index expression's declared/cast width, so the recompute appears for free
and with the right instruction shape. This is a distinct axis from
"declare the local's own width" (DECOMPILATION_LEARNINGS' existing lever)
-- here the VARIABLE's declaration stays `s16` (matching the first,
sign-extending use); it's a per-SITE cast that forces the second
occurrence's own narrower codegen.

## Naming

**SpuVmNoiseOff** (was `func_8002F2A4`) -- Tier B. Releases every
voice whose state byte (`D_8008D9A3`) reads exactly `2`. The value `2` is
the same one `SpuVmAlloc` (code_179d8_l) and SpuVmFlush both
react to by calling their respective "silence the SPU noise generator"
Sony/library function (`func_800375E8` / `SpuSetNoiseVoice`) before
clearing state -- three independent sites agreeing on what state `2`
means is why "Noise" is used here rather than a bare "state-2" name, even
though this function itself never calls a noise-silencing routine (it
only clears the bookkeeping fields, which is consistent with being called
AFTER the noise generator has already been silenced elsewhere, or for a
voice that never needed it).

## Track 2 (round 86, 2026-09-26, alpha)

The per-field symbols this report names (`D_8008D988`..`D_8008D9BA` at a 0x34 stride, and the twelve game names over +0x1C..+0x33 that earlier rounds gave `D_8008D9A4`..`D_8008D9BA`) are ONE Sony table: libsnd/vmanager.o (disc 3.5) bss puts `_svm_voice` at +0x198 of the block anchored at 0x8008D7F0, so `_svm_voice` = 0x8008D988, 24 voices x 0x34 = 0x4E0 bytes, ending exactly at `_svm_envx_ptr`. The symbols file now carries `_svm_voice` (size:0x4E0); the record type is `include/SvmData.h` (fields by offset only, Sony's rule). Field map: +0x00 `unk00` (was `D_8008D988`), +0x02 `unk02` (`D_8008D98A`), +0x04 `unk04` (`D_8008D98C`), +0x06 `unk06` (`D_8008D98E`), +0x08 `unk08` (`D_8008D990`), +0x0A `unk0A` (`D_8008D992`), +0x0C `unk0C` (`D_8008D994`), +0x0E `unk0E` (`D_8008D996`), +0x10 `unk10` (`D_8008D998`), +0x12 `unk12` (`D_8008D99A`), +0x14 `unk14` (`D_8008D99C`), +0x16 `unk16` (`D_8008D99E`), +0x18 `unk18` (`D_8008D9A0`), +0x1B `unk1B` (`D_8008D9A3`), +0x1C..+0x26 `unk1C`..`unk26` (the SeAutoVol/SetAutoVol ramp: active, step, interval, countdown, accum, limit; `D_8008D9A4`..`D_8008D9AE`), +0x28..+0x32 `unk28`..`unk32` (the SeAutoPan/SetAutoPan ramp, same order; `D_8008D9B0`..`D_8008D9BA`). Preserved bodies in this report keep the per-address `D_` spellings, which still link (except `D_8008D988`, which is now `_svm_voice` itself) (splat keeps them as auto-symbols, since the table lies past the global segment's vram range and splat does not fold them into `_svm_voice`).

`src/` now reads/writes `_svm_voice[i].unk1B`/`unk04`. Byte-exact.
