# func_8003ECD0 — STALL (near-miss, 71/73, instruction-order residue)

Unit: `code_2cc8c_d`. Round 14, runner delta. Best score: 71/73 words
in-range, build clean at that score. ~14 real attempts, all on the SAME
2-word residue. Restored to `INCLUDE_ASM` per project rule.

## Signature (as attempted)

```c
void func_8003ECD0(Unk18Obj *self);
```

`Unk18ObjMethods`'s own `+0x08C`... no — not a vtable slot found in
`tools/classtable.py D_8006E8E4`'s output; called directly by symbol
(caller not yet found in this unit's own queue).

## What it does

One-time allocation/init, guarded by `self->unk70` (the same latch
`func_8003EA2C`/`func_8003EA48` check, this round): allocates one buffer
sized to fit two internal records plus a `4 << self->unk3C`-sized payload
each, carves it into `unk78`/`unk80`/`unk88` (bases) and
`unk7C`/`unk84`/`unk8C` (bases + size), writes a 2-word header into each of
`unk78`/`unk7C`, then hands both off to `func_8003FC18` (next slice,
uncarved).

```c
void func_8003ECD0(Unk18Obj *self) {
    s32 size;
    s32 buf;

    if (self->unk70 != 0) {
        return;
    }

    size = self->unk48 * self->unk44 + (4 << self->unk3C) + 0x14;

    buf = (s32)func_80017B34(size * 2);
    if (buf == 0) {
        return;
    }

    self->unk78 = buf;
    self->unk80 = buf + 0x14;
    self->unk88 = (4 << self->unk3C) + self->unk80;

    self->unk7C = size + self->unk78;
    self->unk84 = size + self->unk80;
    self->unk8C = size + self->unk88;

    *(s32 *)self->unk78 = self->unk3C;
    *(s32 *)(self->unk78 + 4) = self->unk80;

    *(s32 *)self->unk7C = self->unk3C;
    *(s32 *)(self->unk7C + 4) = self->unk84;

    func_8003FC18(0, 0, self->unk78);
    func_8003FC18(0, 0, self->unk7C);

    self->unk70 = 1;
    self->unk74 = 0;
}
```

Every field write, every branch, the allocation call, both `func_8003FC18`
calls, and the whole tail all match byte-for-byte. **The only residue is
2 words (out of 73) in the very first arithmetic expression** — the
initial `size` computation.

## The residue, exactly

Retail's own instruction sequence for `size`:

```
mult  $v1, $v0        ; self->unk48 * self->unk44  (into HI:LO)
lw    $v1, 0x3C($s0)  ; v1 = self->unk3C  (independent, fills mult latency)
sllv  $v1, $s2, $v1   ; v1 = 4 << self->unk3C
mflo  $v0              ; v0 = product (mult now resolved)
addiu $v0, $v0, 0x14   ; v0 = product + 0x14
addu  $s1, $v1, $v0    ; size = shiftVal + (product + 0x14)
```

Every attempt that reproduces the first 4 of those 6 instructions
byte-for-byte (confirmed — `mult`, the independent `unk3C` reload, the
`sllv`, and `mflo` all land exactly where retail has them) ends the
combination as `(product + shiftVal) + 0x14` instead of retail's
`shiftVal + (product + 0x14)` — same VALUE, different pairing, 2
instructions swapped.

## What was tried (all real attempts, all reverted)

1. `self->unk48 * self->unk44 + (4 << self->unk3C) + 0x14;` (plain
   left-to-right) — **71/73, the best reached.** Gets the first 4
   instructions exactly right; only the final pairing differs.
2. `(4 << self->unk3C) + (self->unk48 * self->unk44 + 0x14);` (retail's
   own final grouping, written explicitly) — 68/73, WORSE, and the
   regression starts at instruction 1 (the `mult`/reload/`sllv` ordering
   itself changes), not just the tail. This is the most informative
   negative result: naming retail's own target grouping does not
   reproduce it, because the earlier instructions are sensitive to the
   SAME expression shape in a way that isn't obvious from the final value.
3. `self->unk48 * self->unk44 + 0x14 + (4 << self->unk3C);` — 69/73,
   same regression pattern.
4. `(self->unk48 * self->unk44 + 0x14) + (4 << self->unk3C);` (explicit
   parens forcing retail's inner grouping) — 69/73.
5. Two-statement splits (`size = mult; size = shift + size;`, `size =
   mult + 0x14; size = shift + size;`, `size = mult; size += 0x14;`) —
   67–68/73 each, all regressing the early instructions too.
6. A named `s32 shiftVal = 4 << self->unk3C;` local, reused at both call
   sites (this and `unk88`'s own computation, which ALSO computes `4 <<
   self->unk3C`) — no change from attempt 1's score either way; ruled out
   as the lever.
7. A bare `__asm__("");` scheduling barrier, tried both mid-expression
   (via a statement split) and as the function's literal first statement
   — no improvfement in either position; the "prologue callee-save order"
   precedent's lever does not transfer to this residue.

**Every attempt other than #1 disturbs instructions BEFORE the residue
even starts**, which is the real finding here: this isn't a case where the
tail can be reshaped independently of the head. The `mult`/`mflo` pair's
own scheduling (which independent work the compiler interleaves to hide
multiply latency) is apparently entangled with the FULL expression's shape
in a way where only the naive left-to-right parse reproduces retail's own
early scheduling, and that same naive parse is what gets the tail's pairing
wrong.

## Header changes kept

`include/code_2cc8c.h` — all MEASURED from the disassembly, independent of
the stall:
- `Unk18Obj::unk70`'s own comment updated: this function is its set site
  (previously "not itself written by any function this unit attempted").
- `Unk18Obj` gains `unk74` and `unk78`/`unk7C`/`unk80`/`unk84`/`unk88`/
  `unk8C` (all `s32`, deliberately not pointer-typed — see the struct's own
  comment on why: retail computes every one of them via plain word
  arithmetic, and `unk78`/`unk7C` are ALSO dereferenced directly as raw
  2-word records via explicit casts, which a pointer-typed field would not
  reproduce cleanly).
- New extern `func_8003FC18(s32 a0, s32 a1, s32 a2)` (next slice,
  uncarved).

## Proposed learning

**A `mult`/`mflo` pair's instruction scheduling can be entangled with the
FULL shape of the expression it feeds, not just the sub-expression
adjacent to the multiply.** Every reshaping that targeted only the TAIL of
this expression (the final `+0x14`/`+shiftVal` pairing) changed the HEAD
too (the `mult`→reload→`sllv`→`mflo` ordering), even though those
instructions look, by inspection, independent of how the result is later
combined. This contradicts the intuitive model of "reshape locally, get a
local effect" and is worth flagging for any future multiply-latency-
adjacent residue: verify the WHOLE instruction range after every attempt,
not just the words immediately around the change, because a fix attempt at
the tail can regress the head silently.
