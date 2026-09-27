# SsUtAllKeyOff — STALL (length EXACT; raw word-match 103/131; first real diff at vram 0x80032064 / file 0x22864)

> Renamed from `func_80031F3C` on 2026-09-24 (tools/rename.py). Address 0x80031f3c.

`libsnd_ut_ako`, vram `0x80031F3C`, file offset `0x2273C`, 131 instructions
(0x20C bytes). Frameless. Only function in the unit.

**Round 66 supersedes the round 26/31 title.** That title read *"best compiled
122/131, 9 words SHORT; raw word-match 42/131; first real diff at vram
0x80031F4C"*. The length deficit is closed and the word-match is 103/131.

## Round 66 REVISIT — figures before and after

Rebuilt the preserved round-31 body verbatim before touching anything, per the
revisit rule:

| | baseline (round 31 body, rebuilt) | round 66 best |
| --- | --- | --- |
| length | 122/131, **9 words SHORT** | **EXACT** |
| raw word-match | 42/131 | **103/131** |
| `funcdiff` insertions / deletions | 29 / 29 | **10 / 10** |
| positional skeleton diffs | 82 | **26** |
| first real diff | vram `0x80031F4C` | vram `0x80032064` |

Baseline reproduced the recorded title exactly; no stale-title correction was
needed on the figures themselves. The recorded **cause** was wrong, though —
see lever 1.

`REVISITED, round 66: improved 42/131 -> 103/131, length 9-short -> EXACT, ins/del 29/29 -> 10/10; still a STALL on a register-identity residue; names/types used`

## The four levers

### 1. Eight of the nine missing words were CODE THAT WAS NEVER WRITTEN

The round-31 report spent its whole residue section on the split-shift index
and treated the 9-word deficit as scheduling. It is not. Retail's
post-bit-branch block does **three** stores off the reloaded `D_8008EA26`
index, sharing one `*0x34` multiply:

```
lui at,%hi(D_8008D9A3) ; addiu ; addu at,at,v0 ; sb zero,0(at)     /* 22a3c */
lui at,%hi(D_8008D98C) ; addiu ; addu at,at,v0 ; sh zero,0(at)     /* 228c0 */
lui at,%hi(_svm_voice) ; addiu ; addu at,at,v0 ; sh zero,0(at)     /* 228d0 */
```

The preserved body had only the `D_8008D9A3` one. Two missing stores × 4 words
= **8 words**; the ninth is the `sll a0,a0,0x13`. That accounts for the deficit
exactly, with nothing left over.

Note these three fields are each written **twice per iteration** with different
values — `_svm_voice[i] = 0xFF` and `D_8008D98C[i] = 0` at the top of the loop,
then both `= 0` again here. Genuinely duplicated, present in retail's own
instructions, and not to be "simplified" away. (Round 31 had already made this
observation about `D_8008D9A3` alone and still missed the other two.)

**Generalisable:** a residue *classification* in an inherited report is a
hypothesis about the machine, and this one survived two rounds because both
attempts started from the recorded cause instead of from the disassembly. Count
the missing words and try to itemise them to named instructions BEFORE accepting
any scheduling explanation — 9 = 1 + 4 + 4 fell out in one pass.

### 2. The "split scaled index" idiom is SIGNED here, and that is the whole difference

`docs/DECOMPILATION_LEARNINGS.md` writes the idiom as `u16 woff = (u16)i * 8;`.
Round 31 tried that spelling (and two others) and got `andi`+`sll 3`, never
retail's split `sll a0,a0,0x13` … `sra a0,a0,0xf`. The signed spelling produces
it on the first build:

```c
s16 woff = i * 8;
... D_8006DAD4[woff + 3] = 0x200;    /* u16 *, so byte offset = woff*2 + 6 */
```

Mechanism: `(i<<19)>>15` (arithmetic) is `sext16(i*8) * 2`. A `u16` truncation
is a **mask** (`andi`), which cannot fuse with the scale shift; an `s16`
truncation is `sll 16`/`sra 16`, which cc1 **fuses** with the `*8` and the
index `*2` into a single `sll 19` / `sra 15` pair — and then schedules the two
halves far apart, the early `sll` before the nine 0x34-stride stores and the
late `sra` right before first use. The "two independently-scheduled halves"
observation in round 31's *Proposed learning* was correct; what it was missing
is that only the signed spelling produces the pair at all.

### 3. Direct global read-modify-write for a run of mirror updates, not four locals

Retail interleaves load / compute / store tightly for the SPU key-on pair:

```
lhu v1,D_80090C60 ; lhu a0,D_80090C64 ; ... ; lhu v0,D_8008E228
or v1,a3,v1 ; sh v1,D_80090C60 ; nor v1,zero,v1 ; and v0,v0,v1 ; sh v0,D_8008E228
lhu v0,_svm_okon2 ; or a0,a2,a0 ; sh a0,D_80090C64 ; nor a0 ; and ; sh
```

Caching all four globals in four locals (`hw0`/`hw1`/`mask0`/`mask1`, the
round-31 shape) lets cc1 hoist all four loads to the top and sink all four
stores to the bottom — 8 words of misordering. Direct RMW on the globals
(`D_8008E228 = D_8008E228 & ~hw0;`) reproduces the interleave. The best body
keeps locals only for the two *hardware* words, which are read up front in
retail, and uses direct RMW for the two software masks: 98 -> 103.

### 4. `volatile` ON THE POINTEE pins cc1's scheduler — worth 44 words here

`D_8006DAD4` holds `0x1F801C00`, the SPU voice register block (established when
`code_179d8_m` was named as the 24-voice sound driver). So its pointee is
hardware:

```c
extern volatile u16 *D_8006DAD4;   /* NOT `u16 *` */
```

Measured, with everything else held fixed: **54/131 -> 98/131**, and it is what
closed the last word of the length gap.

Mechanism, and this is the transferable half: **cc1 2.6.3 WILL hoist a volatile
access across non-volatile stores, but NOT across another volatile store.**
Directly observed in cc1's own output — with `D_8006DAD4` non-volatile, cc1
moved the `D_8008EA26` volatile store/reload pair *six stores earlier*, to fill
the load-delay slots of the `lw` and the `lhu`:

```
	lw	$2,D_8006DAD4
	sra	$4,$4,15
	#.set	volatile
	sh	$8,D_8008EA26      <- hoisted past the six stores below
	#.set	volatile
	lhu	$3,D_8008EA26
	addu	$4,$4,$2
	sh	$2,6($4)  ...      <- the six SPU stores
```

Typing the pointee `volatile` makes those six stores volatile too, and the pair
stays put. So "is this global volatile?" is not only a question about whether a
reload is elided — it is a **scheduling barrier question**, and the answer can
be worth tens of words in a function that touches memory-mapped I/O.

### 5 (minor). `for (i = 0; i < N; i++)` vs a guard plus `for (;;)` is not cosmetic

The guard form leaves cc1's delay-slot pass unable to fill the entry `beqz`, so
maspsx inserts a defensive nop and the function is a word long:

```
beqz v0,<end>          beqz v0,<end>
 move t0,zero            nop            <- guard form
                        move t0,zero
```

Verified this is cc1's doing, not maspsx's: cc1 wraps the *loop* branches in
`.set noreorder`/`.set nomacro` (it filled those slots itself) and leaves the
entry branch bare, so maspsx is correct to insert the nop. The `for` form makes
cc1 fill it. `while (i < N) { ... i++; }` is byte-identical to the `for` form.

## Best body (length EXACT, 103/131, ins/del 10/10)

Declarations, as they must appear (note the `volatile` on `D_8006DAD4`'s
pointee and the `u16 *` retype — this replaces round 31's `Rec16DAD4` struct):

```c
typedef struct {
    u16 unk0; /* +0x0 */
    u8 pad2[0x34 - 0x2];
} Rec34U16;
extern Rec34U16 _svm_voice[];
extern Rec34U16 D_8008D98A[];
extern Rec34U16 D_8008D98C[];
extern Rec34U16 D_8008D98E[];
extern Rec34U16 D_8008D996[];
extern Rec34U16 D_8008D998[];
extern Rec34U16 D_8008D99A[];
extern Rec34U16 D_8008D99C[];

typedef struct {
    u8 unk0; /* +0x0 */
    u8 pad1[0x34 - 0x1];
} Rec34U8;
extern Rec34U8 D_8008D9A3[];

/* code_179d8_m.c's own comment on this exact symbol: "written as a side
 * effect, then re-read from the global (not a cached register) a few
 * instructions later... Genuinely needs volatile: without it, this
 * compiler proves... the re-read is redundant and elides it entirely."
 * Independently re-confirmed here: the same store-then-reload shape shows
 * up in this function's own disassembly. */
extern volatile u16 D_8008EA26;

/* "Loop bound / threshold" -- code_179d8_m.c's own comment on this symbol. */
extern u8 D_8008E9D0;

/*
 * This function's OWN reading of D_8006DAD4: a POINTER VARIABLE (loaded with
 * `lw`, not an array base) into the PS1 SPU voice register block -- the value
 * is 0x1F801C00, established when code_179d8_m was named as the 24-voice
 * sound driver.  Indexed as HALFWORDS: `D_8006DAD4[woff + N]` with
 * `s16 woff = i * 8`, i.e. 8 halfwords (0x10 bytes) per voice, which is the
 * SPU's own per-voice register stride.  code_179d8_m.c reads the SAME symbol
 * as a fixed-offset object pointer (its own SpuRegs, offsets 0x194/0x196) --
 * a different, valid reading per the project's convention.
 *
 * ROUND 66: the POINTEE MUST BE `volatile`.  These are hardware registers, and
 * the qualifier is load-bearing for the MATCH, not just for correctness: cc1
 * 2.6.3 orders volatile accesses against other volatile accesses only, so
 * without it cc1 hoists the `D_8008EA26` volatile store/reload pair across
 * these six stores.  Worth 54/131 -> 98/131.  Round 31's `Rec16DAD4` struct
 * spelling (stride 0x10, fields f0..fA) is RETRACTED: it compiles the index as
 * a plain late `sll 4` instead of retail's split `sll 19` / `sra 15`.
 */
extern volatile u16 *D_8006DAD4;

/* PS1 SPU voice key-on/off pair, split low/high across two 16-bit halves
 * (voices 0-15 / 16-31) -- D_80090C60/64 are the hardware-mirrored "just
 * keyed on" mask, D_8008E228/22C a software mask this function clears the
 * same bit from (a "no longer fading out" bookkeeping flag). */
extern u16 D_80090C60;
extern u16 D_80090C64;
extern u16 D_8008E228;
extern u16 _svm_okon2;
```

```c
#if 0
void SsUtAllKeyOff(void)
{
    s16 i;
    s16 woff;
    u16 bitpos;
    u32 bitLo;
    u32 bitHi;
    u16 hw0;
    u16 hw1;

    for (i = 0; i < D_8008E9D0; i++) {
        woff = i * 8;
        D_8008D98A[i].unk0 = 0x18;
        _svm_voice[i].unk0 = 0xFF;
        D_8008D9A3[i].unk0 = 0;
        D_8008D98C[i].unk0 = 0;
        D_8008D98E[i].unk0 = 0;
        D_8008D996[i].unk0 = 0xFF;
        D_8008D998[i].unk0 = 0;
        D_8008D99A[i].unk0 = 0;
        D_8008D99C[i].unk0 = 0xFF;

        D_8006DAD4[woff + 3] = 0x200;
        D_8006DAD4[woff + 2] = 0x1000;
        D_8006DAD4[woff + 4] = 0x80FF;
        D_8006DAD4[woff + 0] = 0;
        D_8006DAD4[woff + 1] = 0;
        D_8006DAD4[woff + 5] = 0x4000;

        D_8008EA26 = i;
        bitpos = D_8008EA26 & 0xFFFF;
        if (bitpos < 0x10) {
            bitLo = 1u << bitpos;
            bitHi = 0;
        } else {
            bitLo = 0;
            bitHi = 1u << (bitpos - 0x10);
        }

        D_8008D9A3[bitpos & 0xFFFF].unk0 = 0;
        D_8008D98C[bitpos & 0xFFFF].unk0 = 0;
        _svm_voice[bitpos & 0xFFFF].unk0 = 0;

        hw0 = D_80090C60;
        hw1 = D_80090C64;
        hw0 = bitLo | hw0;
        D_80090C60 = hw0;
        D_8008E228 = D_8008E228 & ~hw0;
        hw1 = bitHi | hw1;
        D_80090C64 = hw1;
        _svm_okon2 = _svm_okon2 & ~hw1;
    }
}
#endif
```

## Residues, all in the last third, none source-reachable

Itemised from `tools/asm-differ/diff.py`. The first 100 words are
instruction-for-instruction identical to retail.

1. **Register identity on `bitLo`/`bitHi` — first real diff, vram
   `0x80032064`.** Retail allocates `bitLo` to `$a3` and `bitHi` to `$a2`; the
   build does the reverse. Everything downstream follows from it, including the
   flipped commutative operand order (`or v1,a3,v1` vs `or v1,v1,a2`): cc1
   canonicalises commutative operands by register, so the operand order is a
   *symptom* of the allocation, not a second residue. Per CLAUDE.md HARD RULE 6
   this is a STALL — no `register T v asm("$N")`, no operand constraint.
   Swapping the declaration order of the two variables is byte-identical.
2. **The `i++` pair (`addiu a1,t0,1` / `move t0,a1`) sits 5 words later than
   retail**, after the `*0x34` multiply instead of before the `andi`. **Four
   different source placements produce byte-identical output** — `for` with the
   increment in the clause, `for (i=0; i<N; )` with an explicit `i++` after the
   if/else, the same with `i++` at the body end, and a bare `__asm__("")`
   barrier after the if/else. Not source-reachable.
3. **Hoisted-load choice swapped.** Retail hoists `lhu D_80090C64` into the
   early slot and keeps `lhu D_8008E228` next to its use; the build does the
   opposite, and correspondingly sinks the `D_8008E228` store to the end. Four
   spellings of the tail were measured (below); none moves it.

## Attempts — 25 scored builds, well under the 30 cap; 6 improved

| change | result |
| --- | --- |
| baseline, round-31 body rebuilt | 42/131, 9 short, 29/29 |
| + two missing stores, + `s16 woff = i * 8` | 21/131, **length exact**, 21/21 |
| + direct global RMW in the SPU tail | 30/131, 19/19 |
| index `D_8006DAD4[...]` directly vs a `u16 *spu` local | **byte-identical** — cc1 CSEs the pointer load either way |
| + `for (i = 0; i < N; i++)` instead of guard + `for(;;)` | 54/131 |
| + `volatile u16 *D_8006DAD4` | **98/131** |
| + `hw0`/`hw1` locals, masks still direct RMW | **103/131**, 10/10 |
| swap `bitLo`/`bitHi` declaration order | byte-identical |
| split `D_80090C60 = hw0 = ...` into two statements | byte-identical |
| `while (i < N) { ... i++; }` | byte-identical to the `for` form |
| `for (;cond;)` + explicit `i++`, two positions | byte-identical |
| `__asm__("")` after the if/else | byte-identical |
| `do { ... } while (i < N)` with a guard | **4/131**, skeleton 125 — far worse |
| hw locals + `~D_80090C60` (global) in the `nor` | 98/131 |
| `hw1` local only, `hw0` direct | 100/131 |
| mask locals read at point of use | byte-identical to 103 |
| `__asm__("")` before the hw reads | 84/131 |

## Permuter: scaffold provisioned, Gate 3 run, search DECLINED on budget

Gate 3 (`docs/PARALLEL-RUNS.md` §3.5), all three checks run:

1. **Scaffold compiles and scores** — base score 1055.
2. **Scaffold `--debug --stack-diffs`** — insertions 4 / deletions 4,
   registers 27, reorderings 2.
3. **Agreement with the real build** — funcdiff on the identical body reports
   **10 / 10**. Numerically that is a disagreement, which reads as "decline".

**It is a false disagreement, and there is a cheap decisive test.** I dumped
both objects with relocations masked:

```sh
sh permuter-work/SsUtAllKeyOff/compile.sh permuter-work/SsUtAllKeyOff/base.c -o /tmp/scaf.o
tools/binutils/bin/mipsel-linux-gnu-objdump -d /tmp/scaf.o
tools/binutils/bin/mipsel-linux-gnu-objdump -d build/src/libsnd_ut_ako.c.o
```

The two disassemblies are **identical, 132 lines each**. The scaffold compiles
exactly what the tree compiles; the 4/4-vs-10/10 gap is two tools aligning the
same instruction stream differently. Neither "decline" row of Gate 3's table
applies either (the scaffold is *cleaner* than the real build, and the real
build is not zero-drift). So check 3 is satisfied in substance and a search
would be meaningful.

The scaffold is left in place at `permuter-work/SsUtAllKeyOff` (gitignored) for
whoever picks this up. The search itself was not spent: the one remaining
in-function residue that a permuter could plausibly move is residue 1, a
register-identity swap between two adjacent temporaries, and residues 2 and 3
were each shown source-inert across four and four spellings respectively.

### Proposed learning

**`volatile` on a pointee is a scheduling barrier, not just a reload
guarantee — and cc1 2.6.3 orders volatile accesses only against OTHER volatile
accesses.** Measured both directions in this function: with `D_8006DAD4` typed
`u16 *`, cc1 hoisted an unrelated `volatile u16` global's store/reload pair
across six stores through that pointer; typing it `volatile u16 *` pinned the
pair back and moved the function 54/131 -> 98/131. So when a near-miss looks
like "cc1 hoisted something retail left in place", ask which of the memory
operands is memory-mapped I/O before reaching for a barrier or calling it a
scheduling stall. The discriminator is cheap: dump cc1's own output and look
for a `#.set volatile` marker that has migrated past non-volatile stores.

**Corollary for `tools/uncarved.py`-style screens:** this unit's carve comment
correctly flagged the "split scaled index" idiom as likely to apply, and it
did — but in its *signed* form, which the learnings entry does not mention.
The entry is worth amending: the sign of the truncation decides whether the
scale shift fuses (`sll 19`/`sra 15`) or stays a mask plus a shift
(`andi`/`sll 3`).

**Gate 3 check 3 needs an object-level tiebreak.** When the scaffold's and
funcdiff's insertion/deletion counts differ but neither "decline" row applies,
compare the two objects' disassemblies directly (two `objdump` calls). Equal
objects means the numeric gap is a diff-alignment artifact and the search is
meaningful; unequal objects is the real scaffold artifact the check exists to
catch.

## Track 2 (round 86, 2026-09-26, alpha)

The per-field symbols this report names (`D_8008D988`..`D_8008D9BA` at a 0x34 stride, and the twelve game names over +0x1C..+0x33 that earlier rounds gave `D_8008D9A4`..`D_8008D9BA`) are ONE Sony table: libsnd/vmanager.o (disc 3.5) bss puts `_svm_voice` at +0x198 of the block anchored at 0x8008D7F0, so `_svm_voice` = 0x8008D988, 24 voices x 0x34 = 0x4E0 bytes, ending exactly at `_svm_envx_ptr`. The symbols file now carries `_svm_voice` (size:0x4E0); the record type is `include/SvmData.h` (fields by offset only, Sony's rule). Field map: +0x00 `unk00` (was `D_8008D988`), +0x02 `unk02` (`D_8008D98A`), +0x04 `unk04` (`D_8008D98C`), +0x06 `unk06` (`D_8008D98E`), +0x08 `unk08` (`D_8008D990`), +0x0A `unk0A` (`D_8008D992`), +0x0C `unk0C` (`D_8008D994`), +0x0E `unk0E` (`D_8008D996`), +0x10 `unk10` (`D_8008D998`), +0x12 `unk12` (`D_8008D99A`), +0x14 `unk14` (`D_8008D99C`), +0x16 `unk16` (`D_8008D99E`), +0x18 `unk18` (`D_8008D9A0`), +0x1B `unk1B` (`D_8008D9A3`), +0x1C..+0x26 `unk1C`..`unk26` (the SeAutoVol/SetAutoVol ramp: active, step, interval, countdown, accum, limit; `D_8008D9A4`..`D_8008D9AE`), +0x28..+0x32 `unk28`..`unk32` (the SeAutoPan/SetAutoPan ramp, same order; `D_8008D9B0`..`D_8008D9BA`). Preserved bodies in this report keep the per-address `D_` spellings, which still link (`D_8008D988` itself now reads `_svm_voice` above) (splat keeps them as auto-symbols, since the table lies past the global segment's vram range and splat does not fold them into `_svm_voice`).

The preserved `#if 0` body in `src/` now stores `_svm_voice[i].unkNN`. Compiled live as a probe it scores 103/131, ins/del 10/10 -- exactly the round-66 best: the struct spelling neither helps nor hurts this stall.

## History: the unit banner (moved round 97, track 8)

When the unit became `src/libsnd_ut_ako.c` its banner was rewritten as
documentation. The history it carried, verbatim (it was then
`code_179d8_p`):

```c
/*
 * ROUND 42 CORRECTION (2026-09-15) -- READ BEFORE ANY "BLOCKED" LINE BELOW:
 * every claim in this comment that a function is BLOCKED by `gp_rel`,
 * `nop_mflo_mfhi` or `addiu_at` is STALE.  All three constructs are RESOLVED
 * by pinned maspsx flags (CLAUDE.md, "Open toolchain blockers");
 * `tools/nearmiss.py` reports them tagged (RESOLVED-not-a-blocker) and counts
 * none of them.  Any "do NOT spend attempts on these" directive below is
 * therefore RETRACTED: those functions are ordinary matching work, and most
 * carry a mechanism-correct partial derivation already.  The rest of this
 * comment still stands -- only the blocker verdicts are withdrawn.
 * Screen: `python3 tools/nearmiss.py`, round 43 (2026-09-15).
 *
 * code_179d8_p -- SsUtAllKeyOff, 0x2273C..0x22948 (vram 0x80031F3C..
 * 0x80032148).  A single 131-word function.  Carved round 26 (2026-09-09)
 * out of what had been the `code_179d8_mid_d` remainder; renamed on carve
 * because "mid_d" named a leftover and the leftover is now fully consumed.
 *
 * Blocker census at carve time, four screens: BLOCKER-CLEAN -- zero gp_rel,
 * zero forward nop_mflo_mfhi, zero `jr $t2` trampolines, zero `jtbl_`, zero
 * `alabel`.  The body is FRAMELESS (no `addiu $sp, $sp, -N` anywhere), which
 * is worth knowing before you write C for it.
 *
 * It had been parked for several rounds as "addiu-$at blocked"; `addiu_at`
 * was resolved in round 21 and was its only obstruction.
 *
 * Owns NO jump table, so no rodata sub-slot is attached.  It does reference
 * plain rodata/data SYMBOLS -- reference them as symbols, never re-type a
 * string literal (splat has already emitted those bytes; a literal emits a
 * second copy and shifts the whole image).
 *
 * READ THIS BEFORE STARTING: SsUtAllKeyOff touches the same global family as
 * `code_179d8_m` -- libsnd's _svm_voice table (include/SvmData.h) and
 * D_8006DAD4.  The "split scaled index" entry in
 * docs/DECOMPILATION_LEARNINGS.md (a mask on the PRODUCT means a halfword
 * array indexed by a truncated `idx*8`, NOT a struct array indexed by a cast
 * index) was derived on exactly those globals, together with its
 * loop-versus-non-loop refinement.  It is very likely to apply here.
 *
 * This unit's extern declarations stay LOCAL to this file, except Sony's
 * _svm_voice, whose one type is include/SvmData.h (round 86, track 2).
 */
```
