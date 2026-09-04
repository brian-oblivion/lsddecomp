# func_8002CA3C -- STALL (best: 5/55 words match, but see caveat below)

Unit: `code_179d8_e`. Runner: echo, round 17. Restored to `INCLUDE_ASM`.

## Class: compiler-behavior residue (NOT gp_rel, NOT addiu_at) -- reproduced in isolation

Screened clean on both documented blockers (no `gp_rel`, no `addiu $at,$at,%lo`
hits in the `.s`). This is a THIRD, previously undocumented residue class,
confirmed with a from-scratch minimal reproducer through the pinned pipeline
(see below) -- not merely suspected.

## What the function does (high confidence, established via m2c + manual read)

`D_8006DA34`'s vtable slot +0x080. Signature:
`s32 func_8002CA3C(ObjDA34 *self, s32 index, s32 arg2, s32 arg3)`.

```c
if (index >= 0) {
    hi = index >> 4;                      /* array-of-pointers index */
    lo = index - hi * 16;                 /* sub-index within that chunk */
    entry = &self->unk50[hi][lo];         /* Chunk179D8E *, 0x20-byte stride */
    result = func_80030E90(self->unk54, (s16)hi, (s16)lo,
                            (s16)(entry->unk4 + self->unk60), entry->unk5,
                            (s16)arg2, (s16)arg2);   /* NOTE: arg2 passed TWICE */
    if (result >= 0) {
        func_80031E94(result, (s16)arg2, (s16)arg3, 2);
        return result;
    }
}
return -1;
```

This structure (control flow, argument counts -- including the double-`arg2`
7th argument to `func_80030E90`, which m2c independently confirmed -- field
offsets, and the `self->unk50[hi][lo]` addressing) is corroborated by
`.venv/bin/python3 tools/m2ctx.py code_179d8_e --sig 's32
func_8002CA3C(ObjDA34 *self, s32 index, s32 arg2, s32 arg3)' --run`, which
produces the same shape independently from the raw instructions. I am
confident this part is right.

## The residue: retail does NOT fold `index - (index>>4)*16` into `index&0xF`

Retail's `hi`/`lo` split:

```
sra   v0, a0, 0x4      ; hi = index >> 4  (bare shift, no bias correction)
sll   a1, v0, 0x10
sra   a1, a1, 0x10     ; hi16 = (s16)hi
sll   v1, v0, 0x4      ; v1 = hi << 4
subu  v1, a0, v1       ; lo = index - hi*16   <-- UNFOLDED subtraction
sll   a2, v1, 0x10
sra   a2, a2, 0x10     ; lo16 = (s16)lo
sll   v0, v0, 0x2      ; hi*4  (array-of-pointers stride)
sll   v1, v1, 0x5      ; lo*32 (Chunk179D8E stride)
```

Every C form tried compiles the `lo` computation to a single `andi $t,$t,0xf`
instead, using the ORIGINAL `index` register directly (bypassing `hi`
entirely) -- and correspondingly loses the two `sll`/`sra` "widen hi/lo back
to a full register" pairs and the `subu`, an instruction-count deficit large
enough to shift every following instruction and drag the whole function's
score down to noise (5/55, plus an out-of-range byte-count warning).

**This is not a source-shape guess -- it reproduces in isolation, unconditionally,
across seven independent minimal probes through the exact pinned pipeline**
(`tools/gcc263/cpp` -> `cc1 -O2` -> `maspsx --aspsx-version=2.34
--dont-force-G0 --expand-div` -> `mipsel-linux-gnu-as`):

1. `hi = x>>4; lo = x - hi*16;` (plain, no guard) -> folds to `andi`.
2. Same, with an s16-typed "hi16" copy computed immediately after hi (mimicking
   retail's instruction order) -> still folds.
3. Same, wrapped in `if (x>=0) { ... }` (matching CA3C's own negative-index
   guard) -> hi's bias-check DOES disappear (matches retail's bare `sra`!) but
   `lo` STILL folds to `andi`.
4. `hi = x/16; lo = x - hi*16;` with NO guard -> hi gets bias-corrected
   (`bgez`/`addiu` before the `sra`, which retail does NOT have) but `lo`
   stays UNFOLDED (`subu`) -- the only variant that preserved the `subu`.
5. Same division form WITH the `if (x>=0)` guard (i.e., CA3C's actual shape)
   -> the guard eliminates the bias-check (bare `sra`, matching retail) but
   ALSO re-enables the `andi` fold for `lo` -- the two "range now known"
   simplifications are evidently the SAME optimizer decision, not two
   independent ones, so they cannot be pried apart by choosing `/` vs `>>`.
6. A bare `__asm__("")` scheduling barrier between computing `hi` and `lo`
   does not block the fold (it happens at a tree/RTL level before instruction
   scheduling, which the barrier constrains).
7. A full-shape probe (struct self, real field offsets, two helper calls,
   matching register pressure) with `/`-based `hi` inside the guard -- gets
   EVERY OTHER INSTRUCTION IN THE FUNCTION BYTE-IDENTICAL to retail (bare
   `sra` for hi, matching `lh`/`lw`/`lbu`/`sw` sequence, matching stack
   layout) except this one `andi` vs `sll`+`subu` substitution. This is the
   strongest evidence the rest of my reconstruction is correct and this is
   an isolated, load-bearing residue.

Reproducer (probe 5/7, the closest, saved for anyone re-verifying):

```c
extern int helper(short hi, short lo);
int probe(int x) {
    int hi, lo;
    if (x >= 0) {
        hi = x / 16;
        lo = x - hi * 16;
        return helper((short)hi, (short)lo) + hi*4 + lo*32;
    }
    return -1;
}
```

```sh
tools/gcc263/cpp -Iinclude -Iinclude/psyq -undef -lang-c -nostdinc -Dmips -D__GNUC__=2 /tmp/probe5.c \
  | tools/gcc263/cc1 -mips1 -mcpu=3000 -quiet -G0 -O2 \
  | .venv/bin/python3 tools/maspsx/maspsx.py --aspsx-version=2.34 --dont-force-G0 --expand-div \
  | tools/binutils/bin/mipsel-linux-gnu-as -march=r3000 -EL -no-pad-sections -G0 -o /tmp/probe5.o
tools/binutils/bin/mipsel-linux-gnu-objdump -d /tmp/probe5.o
```
Result: `andi s1,a0,0xf` where retail-equivalent code would need `sll`+`subu`.

## What this means

GCC 2.6.3 `-O2`, once it can prove (via an earlier `>= 0` / `< 0` guard) that
the dividend is non-negative, appears to ALWAYS simplify
`x - (x/16 or x>>4)*16` into `x & 0xF` -- there is no plain-C spelling of
"divide by 16 and take the remainder, guarded by a prior non-negativity
check" that keeps the unfolded subtraction. Every reshaping only moves WHERE
this happens, never whether it happens. This is a genuinely new residue class,
distinct from `gp_rel` and `addiu_at` -- worth a name if it recurs
(tentatively: the "shift-remainder blocker" or "div-mod CSE fold").

**Scope not yet measured.** I have not run a corpus census (unlike the two
documented blockers, which have `docs/research/*.md` write-ups with censuses).
This report is the first sighting; per CLAUDE.md's standard ("a minimal
reproducer that FAILS to reproduce is itself the result" -- here it DOES
reproduce, repeatedly, which is the mirror image finding), it should be
escalated for review rather than re-attempted by the next runner without new
information. **Do not re-stage this function until someone either (a) finds a
source form none of the seven probes tried, or (b) confirms via corpus census
that this is systemic and worth a `docs/research/*.md` writeup of its own.**

## Attempts

7 build-verified attempts against the real function (all logged above as
probes 1-3 and 5 applied directly, plus the two-var/`hi*16` explicit-multiply
variants not separately listed), all on the SAME axis (how to spell the
hi/lo split): plain shift, explicit multiply instead of shift, `%` operator,
division operator, with and without an intermediate s16 copy, with and
without a scheduling barrier. **The untested axis is whether the object
layout or argument count is subtly wrong in a way that would produce a
DIFFERENT arithmetic relationship between hi and lo than "index/16,
index%16" -- I did not find any instruction evidence for that, but it is
the one axis all 7 attempts share.**

## Preserved best-effort body

```c
#if 0
s32 func_8002CA3C(ObjDA34 *self, s32 index, s32 arg2, s32 arg3) {
    s32 hi;
    s32 lo;
    Chunk179D8E *entry;
    s16 result;

    if (index >= 0) {
        hi = index / 16;
        lo = index - hi * 16;
        entry = &self->unk50[hi][lo];
        result = func_80030E90(self->unk54, (s16)hi, (s16)lo, (s16)(entry->unk4 + self->unk60),
                                entry->unk5, (s16)arg2, (s16)arg2);
        if (result >= 0) {
            func_80031E94(result, (s16)arg2, (s16)arg3, 2);
            return result;
        }
    }
    return -1;
}
#endif
```

Needs, from this unit's top-of-file scaffolding: `ObjDA34`, `Chunk179D8E`,
`extern s16 func_80030E90(s16, s16, s16, s16, s32, s32, s32);`,
`extern void func_80031E94(s16, s16, s16, s32);` -- all already present in
`src/code_179d8_e.c`.

### Proposed learning

GCC 2.6.3 `-O2` in this pinned toolchain unconditionally folds
`x - (x/N or x>>k)*N` (N a power of two) into `x & (N-1)` whenever it can
independently prove `x >= 0` -- and it can prove that from an ordinary
preceding `if (x < 0) return ...;` guard, the single most natural way to
write this kind of bounded index-splitting code. This fold could not be
avoided by choosing `/` vs `>>`, by an intermediate narrower type, or by a
scheduling barrier in 7 tried variants. If retail code elsewhere splits an
index into `hi = idx>>k; lo = idx - hi*(1<<k)` (or equivalent) *guarded by a
non-negativity check just like this one*, screen for this residue shape
(`andi` in your build where retail has `sll`+`subu`, with hi's bias-check
also missing on both sides) before spending attempts reshaping the C -- it
reproduces in isolation and is very unlikely to be source-shape-fixable.
