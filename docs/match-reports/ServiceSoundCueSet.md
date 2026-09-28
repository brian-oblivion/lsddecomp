# ServiceSoundCueSet -- MATCHED (round 73, 132/132, whole-image SHA1 green)

> Renamed from `func_8002CD08` on 2026-09-24 (tools/rename.py). Address 0x8002cd08.

REVISITED, round 73: MATCHED 132/132 in 13 builds, no permuter; names/types used (slot84's second parameter added; `divisor` local dropped)

## Round 73 (delta): matched

**Preserved round-45 body rebuilt first, unchanged:** 110/132, length exact,
funcdiff `insertions 5 / deletions 5`, positional skeleton diffs 17.

The round-45 verdict ("pure register-allocation identity, unreachable from
C") was wrong. Four independent source-shape causes, each measured:

1. **One counter, not two.** Retail's init loop keeps its counter in `$s3`,
   the SAME register as the main loop's counter. A loop with no calls only
   gets a callee-saved register if its pseudo is live across calls elsewhere,
   so both loops share one `i`. (110 -> 90 raw, but ins/del 5/5 -> 0/0: the
   drop is a pure rotation that items 3-4 then resolved.)
2. **Wrong call arity (this round's lever).** Retail loads `e->result` into
   `$a1` before `bltz $a1` in BOTH arms, and nothing else writes `$a1`
   before the `slot84` call. `slot84` takes `(self, e->result)`. 90 -> 94.
3. **Giv anchor (+0x1C vs +0x28) is the LAST address giv in the loop body.**
   `cc1 -dL` shows GCC 2.6.3's loop pass combines every address giv into the
   last one in insn order. The body's reload of `e->word0` for `slot80`'s
   second argument was the last giv, so the pointer anchored at word0.
   Hoisting it into a local (`note = e->word0 * 16;`) ahead of the two
   divisions makes the `word3` load last and gives retail's `s2+0x28` with
   `-0xC/-8/-4/0` offsets. The scheduler still sinks the load after the
   divides, because maspsx (not cc1) expands the `div` trap checks, so cc1
   sees one basic block. Same step: `e = &a1->entries[0];` before `i = 0;`,
   to match retail's `addiu s4 / move s3,zero / addiu s1` init order.
4. **The rotation was `i`'s global-alloc priority, and the fix was ONE
   increment.** From the `-dl` dump, n_refs is loop-depth weighted, and
   priority = floor_log2(n)*n/live_length: `i` 18 refs/64 insns = 1.125 >
   obj 0.906 > wptr 0.90 > a1 0.82 > rptr 0.63, so `i` took `$s0`. Retail's
   order (obj s0, wptr s1, a1 s2, i s3, rptr s4) needs `i` under 0.82. With
   a single `i++` at the loop bottom `i` has 14 refs (0.66), and reorg's
   delay-slot filler copies that one increment into BOTH arms' branch delay
   slots. That copying is why earlier rounds read two per-arm increments in
   the asm. Result 132/132, first build of that variant.

The round-44 permuter's `do { } while (0)` around the else arm is not
needed in the final body (removing it changed nothing at step 3).

Builds: rebuilt seed, single counter, index form (6/132: retail really
does walk pointers), local `w0` (inert), init reorder, slot84 arity,
inline-args (inert), `note` hoist, no `divisor` (inert), no do-while(0)
(inert), **one bottom `i++` (MATCH)**, top-of-body `i++` (length off),
tidy (else-if, unused local dropped: still matches).

### Matched body

```c
typedef struct Obj179D8CD08 Obj179D8CD08;

typedef struct {
    u8 pad0[0x80];
    s32 (*slot80)(Obj179D8CD08 *self, s32 arg1, s32 arg2, s32 arg3);
    void (*slot84)(Obj179D8CD08 *self, s32 handle);
    u8 pad88[0x9C - 0x88];
    void (*slot9C)(Obj179D8CD08 *self, s32 arg1);
} Obj179D8CD08Methods;

struct Obj179D8CD08 {
    Obj179D8CD08Methods *methods;
};

typedef struct {
    s32 result;
    s32 word0;
    s32 word1;
    s32 word2;
    s32 word3;
} Entry179D8CD08;

typedef struct S179D8CD08 S179D8CD08;

struct S179D8CD08 {
    s32 unk0;
    s32 unk4;
    s32 unk8;
    void (*callback)(s32 arg0, S179D8CD08 *self);
    s32 unk10;
    s32 unk14;
    Entry179D8CD08 entries[3];
};

void ServiceSoundCueSet(Obj179D8CD08 *a0, S179D8CD08 *a1) {
    s32 i;
    Entry179D8CD08 *e;
    s32 rem1;
    s32 rem2;
    s32 note;

    if (a1->unk0 > 0) {
        i = 0;
        e = &a1->entries[0];
        do {
            i++;
            e->word0 = -1;
            e->word1 = 0;
            e->word2 = 0x7F;
            e->word3 = 0x40;
            e++;
        } while (i < 3);

        a1->unk10 = 0;
        if (a1->callback != NULL) {
            a1->callback(a1->unk8, a1);
        }

        if (a1->unk10 >= 0) {
            e = &a1->entries[0];
            i = 0;
            do {
                if (e->word0 >= 0) {
                    if (e->result >= 0) {
                        a0->methods->slot84(a0, e->result);
                    }
                    a0->methods->slot9C(a0, e->word1);
                    note = e->word0 * 16;
                    rem1 = e->word2 - (e->word2 / a1->unk14) * a1->unk10;
                    rem2 = e->word3 - (e->word3 / a1->unk14) * a1->unk10;
                    e->result = a0->methods->slot80(a0, note, rem1, rem2);
                } else if (e->word0 == -2 && e->result >= 0) {
                    a0->methods->slot84(a0, e->result);
                }
                i++;
                e++;
            } while (i < 3);
        }
        a1->unk4++;
    }
}
```

### Proposed learning

**Two identical increments sitting in the delay slots of both arms of an
if/else can come from ONE source statement after the join.** reorg fills
each arm's last branch delay slot from the join block's thread. Writing
`i++` in each arm adds loop-weighted refs (2 x 2 x loop depth), which raises
the counter's global-alloc priority and rotates every callee-saved register.
Screen: the same `addiu sN,sN,1` in the delay slot of the last branch of
each arm means try it once after the join first. Companion: to see
global-alloc order, compute floor_log2(n)*n/len from the `-dl` dump's
"Register N used X times across Y insns". The weighting makes it exact, so
you do not have to guess.

---

## History (rounds 44-45, superseded by the match above)


> **REOPENED by round 44, AND SINCE WORKED -- marker spent (head, round 45).**
> This is now a DOCUMENTED STALL with a real, measured residue (110/132, length
> exact), not fresh ground, so `progress.py` counts it as a stall and no cold
> runner is staffed onto it to re-derive what is below. The round-44 reopening
> text is kept for history: this report's sole cited cause was `nop_mflo_mfhi`,
> RESOLVED in round 42 by the maspsx flag `--no-nop-mflo-mfhi`, and round 44
> attempted the function for the first time ever. Everything below the divider
> is round 44's work AFTER the fix.

Unit: `src/code_179d8_l.c` (carved round 24, 2026-09-08) · Size: 132 words
(0x210 bytes), file offset `0x1D508`, vram `0x8002CD08`.

**Length matches exactly at 132/132 words** (0x210 bytes) for the
best-derived body below -- confirmed via `objdump -t`
(`build/src/code_179d8_l.c.o`), not inferred from funcdiff (which is
drift-affected by other stalled functions elsewhere in the unit and
reports an unrelated "outside range" warning). **95/132 raw word-match**,
first real diff at the SECOND function-body instruction: retail's
`andi $a0,$a3,0xff` (masking `$a3` into `$a0`) has no counterpart; the
derived body computes the mask the other way around (masks `$a0` in place,
keeping the original in `$a3`) -- a genuine register-identity swap.
Confirmed with `tools/asm-differ/diff.py ServiceSoundCueSet`.

## What it computes

Per-channel slot-management callback. `a1` is a struct pointer (`S179D8CD08`
below); `a0` is an object with a small vtable (`Obj179D8CD08` below, three
call slots used: `0x80`, `0x84`, `0x9C`). If `a1->unk0 > 0`: initializes a
fixed 3-entry array (`a1->entries[0..2]`) with default field values
(`word0=-1, word1=0, word2=0x7F, word3=0x40`), unconditionally zeroes
`a1->unk10` then calls an optional callback (`a1->callback`) with
`(a1->unk8, a1)`. If `a1->unk10 >= 0` afterward, loops over the same 3
entries: for an entry whose `word0 >= 0`, optionally calls `slot84(a0)`
(gated on `entry->result >= 0`), always calls `slot9C(a0, entry->word1)`,
computes two remainders (`entry->word2 - (entry->word2/divisor)*scale` and
same for `word3`, where `divisor = a1->unk14` and `scale = a1->unk10`), and
calls `slot80(a0, entry->word0*16, rem1, rem2)`, storing its return into
`entry->result`. For an entry whose `word0 < 0`, only calls `slot84(a0)`
when `word0 == -2` AND `entry->result >= 0`. Loop runs until 3 ENTRIES have
been processed (a separate counter, incremented in both branches) --
`while (j < 3)`, not `while (i < 3)` over the array index directly, since
the array pointer walks in lockstep with `j` here (no early-exit
possibility that would desync them). Finally increments `a1->unk4`.

## Struct/global model (all local to this unit)

```c
typedef struct Obj179D8CD08 Obj179D8CD08;

typedef struct {
    u8 pad0[0x80];
    s32 (*slot80)(Obj179D8CD08 *self, s32 arg1, s32 arg2, s32 arg3);
    void (*slot84)(Obj179D8CD08 *self);
    u8 pad88[0x9C - 0x88];
    void (*slot9C)(Obj179D8CD08 *self, s32 arg1);
} Obj179D8CD08Methods;

struct Obj179D8CD08 {
    Obj179D8CD08Methods *methods;
};

typedef struct {
    s32 result;
    s32 word0;
    s32 word1;
    s32 word2;
    s32 word3;
} Entry179D8CD08;

typedef struct S179D8CD08 S179D8CD08;

struct S179D8CD08 {
    s32 unk0;
    s32 unk4;
    s32 unk8;
    void (*callback)(s32 arg0, S179D8CD08 *self);
    s32 unk10;
    s32 unk14;
    Entry179D8CD08 entries[3];
};
```

**The `Entry179D8CD08` layout was derived from the raw pointer values, not
guessed.** The init loop's pointer (`v1`) starts at `s2+0x28` and stores at
`v1-0xC, v1-0x8, v1-0x4, v1+0` each iteration (stride `0x14`); the main
loop's TWO pointers (`s1` for word0..word3, `s4` for `result`) start at the
SAME `s2+0x28` and `s2+0x18` respectively, also stride `0x14`. Since
`s2+0x28 - s2+0x18 == 0x10` and `s1`'s own `-0xC` offset from its base lands
at `s2+0x1C`, the only layout consistent with BOTH pointers walking the SAME
underlying array is a single 5-word (`0x14`-byte) struct with `result` at
`+0`, `word0` at `+4`, `word1` at `+8`, `word2` at `+0xC`, `word3` at `+0x10`
-- not two separate arrays that happen to be adjacent (they would overlap
if so, which is what first suggested they must be one struct).

## Round 44 update: a bounded permuter search found a real, verified fix -- 95/132 -> 110/132

A bounded (900s, `-j 4`) permuter search was launched against the 95/132 body
below as its seed (never searched before this round). Base score = 460.
Zero `score = 0` hits in the search, but the best candidate found
(`output-160-1`, score 160) has exactly ONE structural change from the
seed: the `else` arm's body (the `word0 < 0` case) wrapped in a
`do { ... } while (0);` block. **Spliced directly into the real unit and
rebuilt: verified, not assumed** -- length unchanged at 132/132, but raw
word-match jumped from **95/132 to 110/132**, and `asm-differ`'s realigned
diff confirms every remaining difference is either a register-identity swap
(same class as residue 1 below) or a constant anchor-point offset (`s1`
based at `s2+0x1c` in the derived body vs `s2+0x28` in retail -- the same
"anchor at last positive-offset field vs. negative-offset-from-last-field"
choice already documented as a codegen-only artifact in
`vmNoiseOn2`'s report). **No instruction is missing or extra anywhere in
the realigned diff** -- this function is now closed on STRUCTURE, open only
on register/anchor IDENTITY, the same wall as this unit's three other
stalls.

## Best-derived body (110/132 words, LENGTH EXACT at 132/132, preserved for the next attempt)

```c
typedef struct Obj179D8CD08 Obj179D8CD08;

typedef struct {
    u8 pad0[0x80];
    s32 (*slot80)(Obj179D8CD08 *self, s32 arg1, s32 arg2, s32 arg3);
    void (*slot84)(Obj179D8CD08 *self);
    u8 pad88[0x9C - 0x88];
    void (*slot9C)(Obj179D8CD08 *self, s32 arg1);
} Obj179D8CD08Methods;

struct Obj179D8CD08 {
    Obj179D8CD08Methods *methods;
};

typedef struct {
    s32 result;
    s32 word0;
    s32 word1;
    s32 word2;
    s32 word3;
} Entry179D8CD08;

typedef struct S179D8CD08 S179D8CD08;

struct S179D8CD08 {
    s32 unk0;
    s32 unk4;
    s32 unk8;
    void (*callback)(s32 arg0, S179D8CD08 *self);
    s32 unk10;
    s32 unk14;
    Entry179D8CD08 entries[3];
};

void ServiceSoundCueSet(Obj179D8CD08 *a0, S179D8CD08 *a1) {
    s32 i;
    s32 j;
    Entry179D8CD08 *e;
    s32 divisor;
    s32 rem1;
    s32 rem2;

    if (a1->unk0 > 0) {
        i = 0;
        e = &a1->entries[0];
        do {
            i++;
            e->word0 = -1;
            e->word1 = 0;
            e->word2 = 0x7F;
            e->word3 = 0x40;
            e++;
        } while (i < 3);

        a1->unk10 = 0;
        if (a1->callback != NULL) {
            a1->callback(a1->unk8, a1);
        }

        if (a1->unk10 >= 0) {
            j = 0;
            e = &a1->entries[0];
            do {
                if (e->word0 >= 0) {
                    j++;
                    if (e->result >= 0) {
                        a0->methods->slot84(a0);
                    }
                    a0->methods->slot9C(a0, e->word1);
                    divisor = a1->unk14;
                    rem1 = e->word2 - (e->word2 / divisor) * a1->unk10;
                    rem2 = e->word3 - (e->word3 / divisor) * a1->unk10;
                    e->result = a0->methods->slot80(a0, e->word0 * 16, rem1, rem2);
                } else {
                    do {
                        j++;
                        if (e->word0 == -2 && e->result >= 0) {
                            a0->methods->slot84(a0);
                        }
                    } while (0);
                }
                e++;
            } while (j < 3);
        }
        a1->unk4++;
    }
}
```

## Residues

1. **The `a3`/`a0` register-identity swap this unit's OTHER stalled
   functions (`vmNoiseOn2`, `SePitchBend`) already documented and
   confirmed unfixable-from-C.** Retail keeps the original unmasked param
   in one register and computes the masked value into a DIFFERENT register;
   this derivation (like the others) ends up with the roles swapped
   regardless of C-level variable naming or declaration order (not
   independently re-tried here; the finding is already established three
   times over in this unit).
2. **Downstream register-choice differences** cascading from (1) through
   the rest of the function -- same class, not independent.

## Attempts

Within the 30-attempt cap (4 real builds used):
1. First transcription (the body above): 132/132 length, 95/132
   raw-match -- landed on the FIRST try at the exact retail length.
2. **Tried forcing retail's apparent "two separate pointers" (`$s1` for
   word0..word3, `$s4` for `result`) by introducing two explicit raw
   pointer locals (`u8 *w`, `s32 *r`) instead of one `Entry179D8CD08 *e`.**
   Reasoning: the raw `.s` shows TWO independently-incrementing base
   registers walking the SAME array, which looked like it might need two
   source-level pointer variables to reproduce. Result: **regressed badly,
   to 139/132 (7 words LONG)** -- introduced EXTRA registers (`s5`, `s6`)
   and an extra register-clearing instruction the single-pointer version
   didn't need; `asm-differ` showed the codegen diverging further, not
   closer. **Reverted in full.**
3. **Re-applied the single-`Entry179D8CD08*`-pointer version to the MAIN
   loop only (keeping the init loop's already-correct single-pointer
   form)**: back to 95/132, 132/132 length -- confirms the ORIGINAL
   single-pointer phrasing (attempt 1) is the better starting point, not
   the "match retail's two registers with two C pointers" instinct.
4. **Bounded permuter search (900s, `-j 4`, base score 460) against the
   95/132 seed, then the found candidate spliced in and rebuilt against
   the real oracle**: found the `do { ... } while(0);`-wrapped `else` arm,
   verified real (not the isolated-scaffold trap CLAUDE.md warns about) --
   95/132 -> 110/132, length unchanged. This is the fifth instance in this
   unit of the "do-while(0) wrapper is a genuinely different C construct
   from a bare `__asm__("")` barrier" idiom (`SpuVmAlloc`,
   `SePitchBend` round 37) actually helping, after several instances
   elsewhere where the identical construct did NOT translate -- always
   verify by direct rebuild, per that established caution.

### Proposed learning

**When retail's disassembly shows what LOOKS like two independently-walking
base-pointer registers over one struct array, do not assume the C source
needs two pointer variables to reproduce it.** Here, a SINGLE
`Entry179D8CD08 *e` with ordinary `e->field` accesses and `e++` already
compiled to something close to retail's shape (95/132, exact length,
residues confined to one register-identity swap); explicitly modeling two
separate raw pointers to force the "two registers" appearance made the
function 44 words WORSE and introduced two entirely new callee-saved
registers. GCC's own register allocator, not the number of C-level pointer
variables, decides how many live base-pointer registers a loop gets --
adding more pointer variables just gives it more to allocate, which can
easily go worse rather than better. This generalizes the existing
"variable identifier choice does not control register assignment"
learning (`vmNoiseOn2`'s report) to the POINTER-COUNT axis, not just
naming.

## Round 45 update: second permuter round, seeded from the 110/132 body -- NEGATIVE

Round 44's disposition proposed a second permuter round seeded from the
110/132 body as one of two concrete next levers. It was run this round:

**Before searching, `--debug --stack-diffs` was checked** (per this round's
runner instructions) against the 110/132 seed:

```
Stack Differences:             0  (1)
Branch Differences:            0  (1)
Register Differences:          25  (5)
Reorderings:                   1  (60)
Insertions:                    0  (100)
Deletions:                     0  (100)
```

Base score 185. **Zero insertions/deletions is a strong predictor of a
cheap, structurally-clean search** (per this round's own guidance) -- the
residue really is confined to register choice, not missing/extra
instructions, confirming this report's own "closed on STRUCTURE, open only
on register/anchor IDENTITY" framing from round 44.

A bounded search (600s, `-j 4`, `--stop-on-zero`) was run against that seed.
**Result: ~63,000 iterations, and the score NEVER beat 185 -- not once, not
even a tie beyond the seed's own starting value.** `permuter rc=124` (the
600s bound fired). This is a clean, unambiguous negative: unlike
`vmNoiseOn`'s round-45 permuter run (which crawled slowly downward
without reaching zero, consistent with a large but real search space), this
search found LITERALLY NOTHING better than its own starting point across
tens of thousands of random mutations.

**This is itself informative, not just "no result."** A residue class that
a randomized structural-mutation search cannot move AT ALL, even
incrementally, is consistent evidence that the gap is a pure
REGISTER-ALLOCATION-IDENTITY difference (residue 1: the `a3`/`a0`
register-identity swap already documented here and in `vmNoiseOn2`/
`SePitchBend`) rather than a reachable structural rewrite the permuter's
mutation set (which edits C source, not register assignment directly) could
ever stumble onto. It reinforces HARD RULE 6's framing: this class of
residue is not a code-shape problem a source-level search tool can solve,
because the permuter mutates STATEMENTS and this gap is not a statement
difference.

## Disposition

**Restored to `INCLUDE_ASM`.** First-ever attempt on this function (round
44): 0 -> 95/132 (first transcription, exact length) -> 110/132
(permuter-found `do-while(0)` fix, verified against the real oracle).
Round 45 ran the remaining proposed lever (a second permuter round seeded
from 110/132) and got a clean, total negative -- 63k iterations without
ever beating the seed's own score, which is itself useful confirmation
that residue 1 is a pure register-identity gap outside what a
source-mutation search can reach. The realigned diff remains clean of any
missing/extra instruction anywhere; every remaining difference is the
already-documented register-identity swap or constant anchor-point offset.
The one lever from round 44 not yet tried is a targeted look at why `s1`
anchors at `+0x1c` instead of `+0x28` (matching `vmNoiseOn2`'s
already-documented anchor-point finding) -- not a permuter question, a
by-hand structural one.

## Naming (round 75, runner alpha, FINISHING-PLAN track 3)

**`ServiceSoundCueSet`** -- tier A. Renamed from `func_8002CD08` via
`tools/rename.py`. Evidence: `DreamSys__TickDrift` calls it only when
`this->cueServiceActive != 0` (`if (this->cueServiceActive != 0)
func_8002CD08(...)`), and `DreamSys__StopDrift` calls the sibling
`FlushSoundCueSet` (a DIFFERENT, already-matched function in
`PlacementGridVabSound.c`) to tear the same cue set down. A field literally named
`cueServiceActive` gating the call is about as direct as tier-A evidence
gets: this is the "service" (per-tick) half of the cue-set's start/stop
pair, `FlushSoundCueSet` the "flush"/stop half. `include/dream_sys.h` and
`include/entity.h` both already carried a stale `func_8002CD08/
FlushSoundCueSet` cross-reference from an earlier round's guess that this
was the SAME function as `FlushSoundCueSet` -- it is not (confirmed: they
are two distinct symbols at two distinct addresses, 0x8002CD08 vs
0x8002CC84, with opposite roles). `rename.py` rewrote every such comment.

Object/field identity (`self`/`set`): confirmed against `PlacementGridVabSound.c`'s
own `VabStreamObj`/`SoundCueSet`/`SoundCueSlot` and `gVabStreamObjMethods`
(`tools/classtable.py gVabStreamObjMethods`) -- see the struct comment in
`src/psyq/libsnd_vmanager.c` above the type definitions, and `FlushSoundCueSet.md`/
`DreamSys__SetSoundObj.md` for the cross-unit trail. Renamed fields, tier A
unless noted:

| old | new | tier | evidence |
| --- | --- | --- | --- |
| `Obj179D8CD08` / `Obj179D8CD08Methods` | `VabStreamObj` / `VabStreamObjMethods` | A | same object `PlacementGridVabSound.c` already names; slot offsets match exactly |
| `slot80` | `playTone` | A | offset +0x80 == `gVabStreamObjMethods`'s `VabStreamObj__PlayTone` |
| `slot84` | `stopVoice` | A | offset +0x84 == `VabStreamObj__StopVoice`; same `(self, index)` call shape as `FlushSoundCueSet`'s own call through the identical slot |
| `slot9C` | `setPitchOffset` | A | offset +0x9C == `VabStreamObj__SetPitchOffset` |
| `Entry179D8CD08` | `SoundCueSlot` | A | same 0x14-byte-stride struct `PlacementGridVabSound.c` declares |
| `result` | `index` | A | matches `SoundCueSlot.index` (`PlacementGridVabSound.c`): -1 sentinel, `>= 0` forwarded to `stopVoice` -- identical pattern to `FlushSoundCueSet`'s `if (slot->index >= 0) slot->index = self->methods->slot84(self, slot->index);` |
| `word0` | `note` | A | `note = e->note * 16` packs directly into `playTone`'s `index` argument (hi/lo split, see `VabStreamObj__PlayTone.md`) |
| `word1` | `pitchOffset` | A | passed unchanged to `setPitchOffset` |
| `S179D8CD08` | `SoundCueSet` | A | same object `PlacementGridVabSound.c` names |
| `unk0` | `tag` | A | matches `SoundCueSet.tag`; `> 0` guard here parallels that unit's `!= 0` guard |
| `unk8` | `owner` | A | matches `SoundCueSet.owner`; passed as `callback`'s first argument unchanged |
| `entries` | `slots` | A | matches `SoundCueSet.slots` |

## Proposed field names (not renamed -- weaker evidence, posted to broadcast)

- `SoundCueSlot.word2` / `word3` -- default `0x7F` (127) / `0x40` (64),
  feed `playTone`'s `arg2`/`arg3` via an `x - (x/unk14)*unk10` remainder.
  `VabStreamObj__PlayTone.md` shows `arg2`/`arg3` reach `SsUtAutoVol(result,
  arg2, arg3, 2)` and `SsUtKeyOn(..., arg2, arg2)` (arg2 passed twice) --
  consistent with a volume/pan pair (127 = full scale, 64 = center) but not
  confirmed against a real Sony signature. Proposed: `volume` / `pan`,
  tier B if adopted.
- `SoundCueSet.unk4` -- incremented once per `ServiceSoundCueSet` call;
  `PlacementGridVabSound.c`'s own view of the same struct never reads it. Proposed:
  `tickCount`, tier B (mechanics-only, purpose in the game not established).
- `SoundCueSet.unk10` -- zeroed before the callback runs; the callback may
  set it negative to skip processing this tick's slots entirely. No further
  evidence of what a non-negative value beyond 0 means. Proposed:
  `serviceGate` or similar, tier C -- too thin to commit to this pass.
- `SoundCueSet.unk14` -- matches `PlacementGridVabSound.c`'s own `unk14` (set to the
  constant 10 by `InitSoundCueSet`); used here as a divisor
  (`e->word2 - (e->word2/unk14)*unk10`). Purpose beyond "some kind of
  scaling period" not established in either unit.

None of the four above cross a header boundary (both units keep independent
local views of this struct, per the project's convention), so these are
recorded here rather than applied -- posted to the broadcast for visibility,
not because another unit's file needs editing.

## Track 4 (2026-09-26, round 87)

The unit-local `VabStreamObj`/`VabStreamObjMethods` view is gone. The
function now includes `include/VabStreamObj.h`, and the whole image stays
byte-identical. Its three calls use the header's own slot names, which the
local view already had: `playTone` (+0x080), `stopVoice` (+0x084) and
`setPitchOffset` (+0x09C). The header names playTone's arguments
`(index, vol, endVol)`: `vol` is SsUtKeyOn's left and right volume and
SsUtAutoVol's start volume, and `endVol` is SsUtAutoVol's end volume. So
`word2` is the start volume and `word3` the end volume. The slot fields are
left unrenamed here because `SoundCueSet` was not part of the track 4 job.
setPitchOffset's argument is an octave: `pitchOffset = octave * 12 - 24`.

## Track 6 (2026-09-26, round 92, alpha): one SoundCueSet

`include/SoundCueSet.h` now holds the one definition of `SoundCueSet` and
`SoundCueSlot`. It replaced three views: PlacementGridVabSound.c's (named
`tag`/`owner`/`callback`/`slots[].index` only), libsnd_vmanager.c's (named
`note`/`pitchOffset`/`word2`/`word3`, `unk4`/`unk10`/`unk14`) and
include/entity.h's `EntityMoodHandlerArg` (all `unkNN`). Zero bytes; the
whole-image SHA1 is unchanged.

Layout verified against every reader: InitSoundCueSet (+0x00 tag, +0x04,
+0x08 owner, +0x0C callback, +0x14 = 10, three 0x14-byte slots from +0x18
whose +0x0 gets -1), ServiceSoundCueSet (per-slot +0x4/+0x8/+0xC/+0x10
reset to -1/0/0x7F/0x40, +0x10 zeroed, callback(owner, set), +0x04
incremented), FlushSoundCueSet (slot +0x0 through stopVoice, +0x00
cleared), Entity__GetProximityRatio (+0x14 divisor), the Entity__MoodCueNN
handlers (+0x04, +0x10, slot 0 +0x4..+0x10, slot 1/2 +0x4/+0x8),
dream_scene's StyleCueNN `self` (the same offsets) and dream_sys.h's
`SoundCueCallbackArg` (+0x00 == tag 1, +0x04 % 20, slot 0/1 +0x4/+0x8).

Names, tier A, each from what its readers do:

| old (e / l / entity.h) | new | evidence |
| --- | --- | --- |
| slot `index` / `index` / - | `voice` | ServiceSoundCueSet stores playTone's result there (the voice, or -1) and passes it to stopVoice(voice); Flush stops it |
| - / `note` / `unk1C` `unk30` `unk44` | `program` | ServiceSoundCueSet passes `program * 16` as playTone's `index`, which PlayTone splits into program `index >> 4` and tone `index & 0xF` (so tone 0); -1 none, -2 stops the voice |
| - / `pitchOffset` / `unk20` `unk34` `unk48` | `octave` | forwarded unchanged to setPitchOffset, whose parameter is `octave` (pitchOffset = octave * 12 - 24) |
| - / `word2` / `unk24` | `vol` | playTone's `vol` argument after attenuation; default 0x7F |
| - / `word3` / `unk28` | `endVol` | playTone's `endVol` argument after attenuation; default 0x40 |
| `unk4` / `unk4` / `unk4` | `tick` | zeroed by Init, incremented once per service pass; handlers time requests on `tick % N` and `tick == 0`, and reset it with -1 |
| - / `unk10` / `unk10` | `attenuation` | zeroed per tick, then each volume loses (vol / attenuationSteps) per unit; < 0 skips keying; handlers store a proximity ratio in 0..10 |
| `unk14` / `unk14` / `unk14` | `attenuationSteps` | set to 10 by Init; the divisor above, and the scale both proximity helpers map a distance onto |

Why not `note`/`pitchOffset` (libsnd_vmanager.c) or the earlier proposal's
`voiceNTone`/`voiceNPitch` (Entity__MoodCue07.md): the value is neither a
note nor a tone. VabStreamObj__PlayTone's `index` is program << 4 | tone,
and ServiceSoundCueSet always sends tone 0, so what the callback writes is a
VAB program number. The pitch word is the octave setPitchOffset takes, not
a pitch offset (that is what setPitchOffset computes from it). `tick` and
`attenuation` are the earlier proposal's names, kept.

`callback` is typed `SoundCueCallbackFn`, `void (*)(void *owner,
SoundCueSet *set)`; InitSoundCueSet's parameter takes that type and its
first parameter is `sound` (it is unused). The three functions have no
shared prototype: entity.h, dream_sys.c and dream_scene.c declare them
with their own type for the sound object (TodActor's `arg2` is a
`struct UnkArg2Obj *`), and a header prototype taking `VabStreamObj *`
would warn in each.

The comment that stood above libsnd_vmanager.c's local view, moved here:
round 75 (naming) confirmed `self`/`set` are the objects PlacementGridVabSound.c names
`VabStreamObj`/`SoundCueSet`: the +0x80/+0x84/+0x9C slots this function
dispatches are `tools/classtable.py gVabStreamObjMethods`'
`VabStreamObj__PlayTone`/`StopVoice`/`SetPitchOffset`, and the slot `index`,
`tag`, `owner` and `slots` usage (the `>= 0`-gated stop-voice call, the
`> 0` tag guard, callback's first argument) matches. VabStreamObj comes from
include/VabStreamObj.h since round 87 (track 4). The parameters and locals
were renamed with the fields (a0/a1 to sound/set, note/rem1/rem2 to
toneIndex/vol/endVol).

## Round 99 (echo, track 7): constants

`-1`/`-2` are SoundCueSet.h's `SOUND_CUE_NONE`/`SOUND_CUE_STOP`; the reset
volumes are the new `SOUND_CUE_DEFAULT_VOL` (127, libsnd's full volume) and
`SOUND_CUE_DEFAULT_END_VOL` (64), whose meaning is `VabStreamObj__PlayTone`'s:
it keys the tone at `vol` and hands `endVol` to `SsUtAutoVol` as the ramp's
end. The loop bound is `ARRAY_COUNT(set->slots)`, the tone index `program *
VAB_TONES_PER_PROG`, defined token-identically to `PlacementGridVabSound.c`'s (the
move of both `VAB_*` defines into `VabStreamObj.h` is proposed to the head).
`e` is `slot`. Byte-identical.

## History (moved from src/libsnd_vmanager.c, comments pass)

The file's banner carried its edge evidence and the reason it is parked:

> What decided its edges (python3 tools/tuboundary.py): every edge from
> PlacementGridVabSound.c to the placed object libsnd/vm_prog, which follows
> this file, is "boundary possible"; the binary is silent. Content decided:
> the carve edges code_179d8_l|m and m|j were staffing cuts, and each fell
> inside one object on every disc (vm_autov between SeAutoVol and
> SetAutoVol, vm_key between SpuVmKeyOff and SpuVmSeKeyOn), so the three
> units were merged. PARKED: the content puts a file boundary after
> ServiceSoundCueSet (game code cannot sit in Sony's object) and, on the 3.6
> reading, at each module edge above; a split is a new carve, so the file
> keeps them and is named for the Sony object, as FINISHING-PLAN track 8
> names Sony code carried as C.
