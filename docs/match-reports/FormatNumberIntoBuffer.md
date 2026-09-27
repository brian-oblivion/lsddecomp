# FormatNumberIntoBuffer — MATCHED (round 45, 22/22 words)

> Renamed from `func_8004D6AC` on 2026-09-22 (tools/rename.py). Address 0x8004d6ac.

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

void FormatNumberIntoBuffer(s32 arg0)
{
    FormatFullWidthNumber(D_8008AA24, arg0, 3, 0);
    *(Buf6_3bb8c_c *)((s8 *)gSaveTitle + 0x12) = *(Buf6_3bb8c_c *)D_8008AA24;
}
```

`FormatFullWidthNumber` (matched round 38, `src/code_2cc8c_f.c`) formats `arg0` as a
zero-padded 3-digit decimal string into the buffer pointed to by
`D_8008AA24`. This unit keeps `FormatFullWidthNumber`'s `self` parameter opaque
(`void *`) rather than pulling in `Obj6EAC0` from `Task.h`, since
nothing here touches its fields — just a pass-through pointer, per the
project's independent-local-view convention.

`D_8008AA24` is a new declaration in `include/class_3bb8c.h`
(`extern void *D_8008AA24;`), added additively next to the existing
`sSaveFileName`/`gSaveTitle` VALUE-of `%gp_rel` globals — same pattern: the ROM
image initializes it to a rodata placeholder (`D_8008AA1C`, the "7654321"
string in `asm/data/7B12C.sdata.s`) but the runtime value is a writable
buffer that `FormatFullWidthNumber` formats into.

The 6-byte copy afterward is the interesting part, and needed three
structurally distinct attempts to land:

1. **First attempt** — a struct of `s16 a, b; s8 c, d;` (alignment 2, size
   6), copied in one whole-struct assignment. This produced the right
   leading `lwl`/`lwr` unaligned word copy for the first 4 bytes (same idiom
   as `Vec2s16` in `FlagLargePolyForDivide`), but the DECLARED alignment of 2 let GCC
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

## Naming

**FormatNumberIntoBuffer** -- tier B. Free function (called directly by
`TitleMenu__TitleMenu`, not through any vtable), `VerbNoun`. Mechanics are
fully evident: formats `arg0` via `FormatFullWidthNumber` into
`D_8008AA24`'s buffer, then copies 6 raw bytes of that buffer into
`gSaveTitle`'s buffer at `+0x12`. Purpose is explicitly NOT established --
the header's own comments on `gSaveTitle`/`D_8008AA24` document both as
"writable-buffer placeholders" whose real runtime role is outside this
unit's own carved ground (a nearby string, "CARD\FILEICN1.TIM", and the
disc's own product-code string sit in the same rodata block, which is
*suggestive* of a memory-card save-icon label under construction, but
that is exactly the kind of purpose-guess the naming rule forbids without
a function that actually establishes it). Named for the one certain
mechanic -- format a number, copy it into another buffer -- and nothing
more. `D_8008AA24`/`gSaveTitle` themselves are left unrenamed for the same
reason.

## Track 6 (2026-09-27, round 96)

`Buf6_3bb8c_c` (`s8 a, b, c, d, e, f`) retired for `FullWidthChars3`
(`FullWidthChar chars[3]`, `FullWidthChar` = `s8 lead, trail`), tier A:
the six bytes are three full-width Shift-JIS digits, which
`FormatFullWidthNumber(..., 3, 0)` writes and the copy puts at the save
title's +0x12, characters 9..11 (the "001" of "LSD   Day001"; the title's
layout was measured in round 95, StampSaveTitleFileLetter.md). It is the
same record class_3bb8c_g's StampSaveTitleFileLetter copies, so the three
types (`FullWidthChar`, `FullWidthChars3`, `FullWidthChars6`) moved from
class_3bb8c_g.c into `include/TitleMenu.h`: the save title is TitleMenu's
buffer (its banner: createSaveTitle builds `saveTitle` from the SJIS title
in gSaveTitle's buffer; both writers serve it), and `TaskObjF.h` only sees
a `char *title` passed in. Alignment is still 1 (all-`s8` leaves), so the
copy is still one `lwl`/`lwr` word plus two `lb`/`sb` pairs, as derived
above; the header keeps the one `MATCHING:` line. Image byte-identical.

Left for track 7 (this unit's polish): the raw `(s8 *)gSaveTitle + 0x12`
(could read `&((FullWidthChar *)gSaveTitle)[9]`) and the
`gSaveTitle`/`D_8008AA24` names.

Comment moved out of the source (verbatim), on the old local type:
"The 6-byte value formatted into D_8008AA24's buffer by FormatFullWidthNumber
above, copied whole into gSaveTitle's buffer at +0x12 as ONE struct
assignment. All-`s8` fields (alignment 1, not 2 or 4) is what makes
retail's block-move split this way: the leading 4 bytes go via the
unaligned lwl/lwr word copy regardless of declared alignment (same
idiom as Vec2s16, FlagLargePolyForDivide), but the trailing 2 bytes can no
longer be proven 2-byte aligned, so there is no safe halfword move for
them and the compiler falls back to two individual signed-byte
loads/stores."
