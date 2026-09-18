# func_8004D6AC — MATCHED (round 45, 22/22 words)

**Unit:** class_3bb8c_c · **Size:** 22 words (0x58 bytes)

Filed as a `gp_rel`-blocked stub in round 9. That blocker was RESOLVED in
round 42 (`--gp-symbols`, pinned in the Makefile). Rebuilding this stub
report's evidence showed the function is ordinary matching work; it matched
this round after a few structurally distinct attempts on the 6-byte struct
copy at the end (below).

## Derivation

```c
extern void FormatFullWidthNumber(void *self, s32 a1, s32 width, s32 unpadded);

typedef struct {
    s8 a, b, c, d, e, f;
} Buf6_3bb8c_c;

void func_8004D6AC(s32 arg0)
{
    FormatFullWidthNumber(D_8008AA24, arg0, 3, 0);
    *(Buf6_3bb8c_c *)((s8 *)D_8008AA18 + 0x12) = *(Buf6_3bb8c_c *)D_8008AA24;
}
```

`FormatFullWidthNumber` (matched round 38, `src/code_2cc8c_f.c`) formats `arg0` as a
zero-padded 3-digit decimal string into the buffer pointed to by
`D_8008AA24`. This unit keeps `FormatFullWidthNumber`'s `self` parameter opaque
(`void *`) rather than pulling in `Obj6EAC0` from `code_2cc8c.h`, since
nothing here touches its fields — just a pass-through pointer, per the
project's independent-local-view convention.

`D_8008AA24` is a new declaration in `include/class_3bb8c.h`
(`extern void *D_8008AA24;`), added additively next to the existing
`D_8008AA10`/`D_8008AA18` VALUE-of `%gp_rel` globals — same pattern: the ROM
image initializes it to a rodata placeholder (`D_8008AA1C`, the "7654321"
string in `asm/data/7B12C.sdata.s`) but the runtime value is a writable
buffer that `FormatFullWidthNumber` formats into.

The 6-byte copy afterward is the interesting part, and needed three
structurally distinct attempts to land:

1. **First attempt** — a struct of `s16 a, b; s8 c, d;` (alignment 2, size
   6), copied in one whole-struct assignment. This produced the right
   leading `lwl`/`lwr` unaligned word copy for the first 4 bytes (same idiom
   as `Vec2s16` in `func_8001A268`), but the DECLARED alignment of 2 let GCC
   trust a safe halfword move for the trailing 2 bytes, emitting a single
   `lh`/`sh` where retail has two individual `lb`/`sb` pairs. Wrong: retail
   never merges those two bytes.
2. **Second attempt** — split the trailing two bytes out of the struct into
   two separate scalar statements (`dst[4] = src[4]; dst[5] = src[5];`)
   computed via local `s8 *src, *dst;` pointer variables assigned BEFORE the
   `FormatFullWidthNumber` call. This forced `src`/`dst` to be kept alive ACROSS the
   call, spilling them into callee-saved `s0`/`s1` with a much larger
   prologue/epilogue that retail does not have — retail computes both
   pointers fresh (via `%gp_rel`) AFTER the call returns, using only
   caller-saved temporaries.
3. **Third attempt** — same split-statement approach, but with the pointer
   assignments moved to AFTER the call. This fixed the register-saving
   problem and CSE'd the two globals' loads correctly across all three
   statements, but the individual byte moves came out as `lbu` (zero-extend)
   instead of retail's `lb` (sign-extend), and retail's actual instruction
   order interleaves differently — it loads ALL THREE pieces (word + both
   tail bytes) before storing any of them, which is `move_by_pieces` batching
   behavior for a SINGLE block copy, not three independently-scheduled
   statements.

**What actually matched:** declaring the WHOLE 6-byte struct as all `s8`
fields (alignment 1, not 2) and keeping it as ONE struct assignment. With
alignment 1, GCC's MIPS block-move still uses the unaligned `lwl`/`lwr` word
move for the first 4 bytes (that instruction pair works for any alignment
and the backend prefers it over four separate byte moves), but for the
2-byte tail it can no longer assume 2-byte alignment (no unaligned halfword
instruction exists on this ISA), so it falls back to two individual `lb`
(signed — the block-mover's own canonical QImode load, unrelated to
`-funsigned-char`) / `sb` pairs, batched load-load-store-store-store-store
in exactly retail's order. This is the SAME `lwl`/`lwr` idiom as `Vec2s16`
extended one step further: the DECLARED struct alignment is what selects
the tail chunk size, not the runtime alignment of the actual buffers.

### Proposed learning

For a struct whose size is not a multiple of 4 and whose retail tail is
copied byte-by-byte (not merged into a halfword), the struct's DECLARED
alignment must be 1 — not 2 — even when every field pair "looks like" it
could be `s16`. Declaring it `s16`+`s8` tail (alignment 2) is enough to make
GCC trust a halfword move for a 2-byte remainder that happens to sit at a
2-aligned offset, silently merging two retail instructions into one and
costing 2 words plus a whole-image address shift. The tell is retail using
two separate `lb`/`sb` for what looks like an adjacent pair.

Also: computing pointers into globals used both as a call argument AND
after a following call must happen AFTER the call if retail's own version
does — doing it before turns a two-caller-saved-register need into a
callee-saved spill (`s0`/`s1` prologue/epilogue) that does not exist in
retail and is a strong signal of a mis-ordered access, not a matching
struct-copy question.
