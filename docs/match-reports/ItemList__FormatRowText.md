# ItemList__FormatRowText

> Renamed from `Class86F88__FormatRowText` on 2026-09-26 (tools/rename.py). Address 0x8005292c.

> Renamed from `func_8005292C` on 2026-09-24 (tools/rename.py). Address 0x8005292c.

**Unit:** ObjMStyleActor · **Size:** 52 instructions (0xD0 bytes) ·
**Status: MATCHED 52/52**, whole-image SHA1 green.

## Role

Formats a text-table entry into a fixed-width (0x1A = 26 char) buffer:
looks up a string via `self->unk18[idx]` (a table of BYTE OFFSETS added to
the caller-supplied `base` pointer), copies up to 0x1A bytes of it into
`dest` via a Psy-Q strncpy-style helper, space-pads the remainder, and
null-terminates at a fixed offset. Called by `ItemList__RefreshRows` (this unit,
also matched this round) once per loop iteration with a local stack buffer
as `dest`.

## New struct field

`ItemList::unk18` (`s32 *`, offset 0x018, was opaque padding) -- a table
of byte offsets, added additively to `include/class_3bb8c.h` (splitting the
existing `pad018[0x020-0x018]` into `unk18` + a 4-byte `pad01C`, so no
other field's offset moves). `ItemList`/`gItemListMethods` is this unit's own
class, not shared with any other live runner this round, so this edit
carries no header-contention risk from a sibling unit.

## Local prototypes

`func_80013348` (Psy-Q strlen, already matched elsewhere, called here with
a byte-pointer argument) and `func_800238A8` (Psy-Q `psyq_GsLinkObject4.s`:
`if (dest == NULL) return NULL; else copy n bytes src->dest, return dest`
-- a strncpy-without-null-pad) are declared LOCAL to this unit
(`ObjMStyleActor.c`), not in a shared header, per the project's
cross-unit-prototype rule. `func_80013348` already has a differently-typed
local declaration elsewhere (`TextEntryItemList.c`: `s32 func_80013348(void
*arg0)`); this unit's own call site reads a byte pointer, so it is typed
`char *s` here instead -- per-call-site typing of an already-established
function, same convention documented for `DecodeFullWidthSjis` in
`include/class_3bb8c.h`.

## Final source

```c
char *ItemList__FormatRowText(ItemList *self, char *dest, s32 arg3, s32 arg4, char *base)
{
    s32 idx = arg4 + arg3;
    s32 len;
    s32 i;

    len = func_80013348(base + self->unk18[idx]);
    if (len >= 0x1B) {
        len = 0x1A;
    }
    func_800238A8(dest, base + self->unk18[idx], len);
    i = len;
    if (i < 0x1A) {
        for (; i < 0x1A; i++) {
            dest[i] = ' ';
        }
    }
    dest[0x1A] = 0;
    return dest;
}
```

## Two residues, two different fixes

1. **A one-instruction-short "redundant move" residue (51/52 -> fixed).**
   Retail materializes the post-call value of `len` into `$a2` TWICE: once
   as `func_800238A8`'s own 3rd-argument setup (unavoidable -- it's the
   call), and AGAIN immediately after the call, purely to feed the
   `slti $v0, $a2, 0x1a` comparison, even though `$a2` had just been
   clobbered by the call and `len`'s real value still lives safely in the
   callee-saved `$s0`. A plain `if (len < 0x1A) { for (i = len; ...) }`
   compiles the comparison straight off `$s0` (functionally identical,
   0 extra instructions, and 1 word SHORT of retail -- funcdiff's
   "outside range" warning fired, confirming a length/shift problem, not a
   register-identity one). Splitting the read into its own statement --
   `i = len; if (i < 0x1A) { for (; i < 0x1A; i++) ... }` -- was enough to
   make GCC 2.6.3 re-materialize `i` into a fresh register (`$a2`) for the
   comparison instead of reusing `$s0` directly, matching retail exactly.
   Two things that did NOT work, worth knowing before re-deriving this
   class of residue: assigning to a distinctly-NAMED second variable
   (`count = len;`) had NO effect (compiler still elided the copy), and
   marking `len` `volatile` overcorrected drastically (forced reloads
   throughout the whole function, dropping the score to 1/52) -- consistent
   with this project's documented "redundant move... resists temp
   placement... and volatile" residue class, except here plain statement
   splitting (no new variable name, no qualifier) was the lever that
   worked.
2. **A commutative-operand-order residue (51/52 -> 52/52, unrelated to
   #1).** `idx = arg3 + arg4;` (the two function parameters added in
   left-to-right declaration order) compiled to `addu $a2,$a2,$a3`; retail
   has `addu $a2,$a3,$a2` -- the operands swapped. Both computed the
   correct VALUE; only the register order in the encoding differed. Fixed
   by writing `arg4 + arg3` instead. A reminder that for a commutative
   binary op, GCC 2.6.3 is NOT operand-order-invariant at the instruction
   level, and matching it can require writing the addition in the "wrong"
   (non-declaration) order.

### Proposed learning

Confirms and extends the existing "redundant move" entry in
`docs/MATCHING-GUIDE.md`: where introducing a differently-NAMED temp
variable for the second use of a value did nothing, splitting the SAME
read into its own separate statement (`i = len;` before `if (i < 0x1A)`,
rather than folding the read into the `if`/`for` header directly) was
enough to force GCC 2.6.3 to re-materialize the value into a fresh
register across an intervening call, rather than reusing the
already-live callee-saved register. Worth trying as a cheaper first step
before reaching for a differently-named variable on this residue class.

## Naming

Round 75 (bravo, track 3). `func_8005292C` -> `ItemList__FormatRowText`, **tier A**.

Not a table slot (non-virtual helper). Copies item (top + row)'s text, starting `column` characters in, into `dest`, truncated to 26 characters, pads to 26 with spaces, NUL-terminates, returns dest. Callers: CreateRows, RefreshRows. The matched body types `column` as `char *` and `texts` as `s32 *` (their sum is the source pointer); retyping them the natural way is left alone because it touches a matched body.

ItemList, per the round-75 pass, is a scrolling list selector: up to 4 visible rows of 26-character item text, a highlighted cursor row, a horizontal column offset (see the unit header comment of `src/world/ObjMStyleActor.c`).

## Track 4 (2026-09-26, round 89)

`texts` (+0x018) is `char **` in include/ItemList.h and `column` is an
`s32` parameter: `strlen(self->texts[idx] + column)` compiles byte-identical
to the `s32 *texts` / `char *column` spelling above (whole-image SHA1
green), so the pointer is on the side the ctor allocates and strcpys into,
and CreateRows/RefreshRows pass `column` without a cast.

## Round 99 (delta, track 7)

Local `idx` -> `item`. `0x1B`/`0x1A` -> `ITEMLIST_ROW_CHARS` (new, include/ItemList.h: 26, the row width; `len >= 0x1B` is written `len > ITEMLIST_ROW_CHARS`), and the terminator `dest[ITEMLIST_ROW_CHARS] = '\0'`. The padding loop is now plain `for (i = len; i < ITEMLIST_ROW_CHARS; i++)`: measured byte-identical, so the `i = len; if (i < 0x1A) { for (; ...) }` split recorded under "Two residues" no longer carries residue 1 in the current source. Residue 2 (operand order `top + row`) still holds, measured: `row + top` breaks the image; it keeps a `MATCHING:` line.

## History: the strlen/memcpy comment (moved from src/class_3bb8c_k.c, round 99)

The unit now carries a two-line comment; the full text it replaced was:

```c
/* Psy-Q strlen and memcpy (libc2/strlen and libc2/memcpy, linked from
 * Sony's own SDK objects). libc2's memcpy guards a NULL dest, copies `n`
 * bytes a byte at a time and returns dest -- which is why this call site
 * was read as strncpy-like before the object gave it its name.
 * Both are declared LOCAL to this unit, not in a shared header, since
 * these are cross-unit prototypes for functions this unit does not define
 * (see CLAUDE.md's header-contention rule). strlen already has a
 * differently-typed local declaration in class_3bb8c_j.c
 * (`s32 strlen(void *arg0)`); this unit's own call site reads its
 * argument as a byte pointer, so it is typed `char *` here instead --
 * per-call-site typing of an undefined function's argument, same
 * convention as DecodeFullWidthSjis (see include/class_3bb8c.h HEAD NOTE).
 * memcpy's return type is `void *` to agree with include/psyq/memory.h's
 * unprototyped declaration should this unit ever include it; the result
 * is discarded at the one call site either way. */
```
