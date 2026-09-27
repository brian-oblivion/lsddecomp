# SetupPrimCode — MATCHED (29/29 words)

> Renamed from `func_8001934C` on 2026-09-17 (tools/rename.py). Address 0x8001934c.

Unit: `src/TmdRenderer.c`. Not previously declared in `include/BMemPMgr.h`;
called only from the still-`INCLUDE_ASM` giant `SortTmdObject` (13 call
sites), so its two arguments' real struct types are unknown outside this
function's own body. Treated both as raw byte-offset accesses rather than
guessing an unverified struct layout.

`arg0`: byte flags at `+0x3` and `+0x7`. `arg1`: `s32` at `+0x1C`, byte
outputs at `+0x14`/`+0x15`.

## Final source

```c
void SetupPrimCode(void *arg0, void *arg1)
{
    u8 *a = (u8 *)arg0;

    if (*(s32 *)((u8 *)arg1 + 0x1C) != 0) {
        a[7] = a[7] | 0x2;
    } else {
        a[7] = a[7] & 0xFD;
    }

    if (D_8008E248 != 0) {
        a[7] = a[7] | 0x1;
    } else {
        a[7] = a[7] & 0xFE;
    }

    *((u8 *)arg1 + 0x14) = a[3];
    *((u8 *)arg1 + 0x15) = a[7];
}
```

`D_8008E248` (a global `s32` flag, read-only from this function) is now
declared `extern` in `include/BMemPMgr.h`; no other symbol name evidence
was available for it.

## Notes

First attempt cached the two bit-set/clear results in a shared local `u8
flags` variable reused between the read-modify-write for each of the two
independent flag bits (12/29, address-drifted). Retail treats the two
`a[7] = a[7] | BIT` / `a[7] = a[7] & ~BIT` updates as two fully independent
statements, each reloading `a[7]` fresh from memory rather than reading it
from a variable holding the previous statement's result — dropping the
shared temp and writing each update as its own direct
`a[7] = a[7] | BIT;`/`a[7] = a[7] & ~BIT;` matched byte-exact on the second
attempt.

### Proposed learning

Two back-to-back "set-or-clear one bit of the same byte field, based on an
independent condition" updates are two independent statements in the
source, each of the form `x->flags = x->flags | BIT;` /
`x->flags = x->flags & ~BIT;` — do not factor them through a shared local
that holds "the flags byte's new value" across both updates. Retail reloads
the field from memory at the start of each one rather than carrying the
previous statement's result forward in a register/variable, even though the
two updates are adjacent and touch the same byte. (`SetupPrimCode`, 12/29 ->
29/29.)

## Naming (round 51, bravo)

`func_8001934C` -> `SetupPrimCode`, `arg0` -> `prim`, `arg1` -> `ctx`.
**Tier B** -- what the code does is now exact, but the name carries only
the dominant half (configuring the GPU command byte) and not the caching
half, and nothing establishes what the original called it.

**What changed this round is not the name but the identification of
`arg0`.** The round-13 report treated both arguments as opaque byte
offsets because "their real struct types are unknown outside this
function's own body". They are not unknown any more. In
`asm/nonmatchings/TmdRenderer/SortTmdObject.s`, every one of the 13
dispatch cases writes a constant into `arg0`'s byte `+0x3` and another into
byte `+0x7` in the two instructions immediately before `jal
func_8001934C`, and the eight distinct pairs are Sony's Psy-Q primitive
definitions exactly:

| len (+0x3) | code (+0x7) | Psy-Q primitive |
| --- | --- | --- |
| 4 | 0x20 | POLY_F3 |
| 5 | 0x28 | POLY_F4 |
| 6 | 0x30 | POLY_G3 |
| 7 | 0x24 | POLY_FT3 |
| 8 | 0x38 | POLY_G4 |
| 9 | 0x2C | POLY_FT4 |
| 9 | 0x34 | POLY_GT3 |
| 12 | 0x3C | POLY_GT4 |

Eight for eight. So `arg0` is a Psy-Q GPU primitive: `+0x3` is `P_TAG`'s
length field (little-endian, so the tag word's high byte) and `+0x7` is the
GPU command byte of the following colour word. That in turn identifies the
two bits this function sets and clears: `0x2` is the GPU's ABE
(semi-transparency) bit, taken from the draw context's flag at `ctx+0x1C`,
and `0x1` is the shade-texture bit that Psy-Q's `SetShadeTex()` sets, taken
from the global `D_8008E248`.

The function then caches the length byte and the finished command byte at
`ctx+0x14`/`ctx+0x15`; `TransformAndCullPoly` re-stamps the length byte
onto the primitive from `ctx+0x14` on every face.

### Proposed global name for the head

`D_8008E248` -> `gShadeTex`. **Tier B.** `SortTmdObject` writes it from bit
6 of the drawn object's flags word (`srl $a0,$a0,6; andi $a0,$a0,0x1; sw
$a0, %lo(D_8008E248)` at 0x800186D8) and this function is its only reader,
ORing it into the shade-texture bit; `gShadeTex` is Sony's own vocabulary
for that bit. **`tools/rename.py` cannot do this one** -- `FATAL:
'D_8008E248' resolves to 0x8008e248, outside the image` -- and I did not
rename it by hand.

### Proposed learning

**`tools/rename.py` cannot rename any bss global that lives past the end of
the image, and track 3 will hit this constantly.** The tool resolves a
`D_800XXXXX` placeholder by its address and rejects anything outside the
loaded image; a `.bss` symbol beyond the image end (here 0x8008E248, with
the image ending around 0x8008B800) has no in-image address to resolve, so
every such global is unnameable through the sanctioned path. These are not
rare -- they are the game's plain mutable globals. Two options exist and
the choice is the operator's: teach the tool to resolve a name from the
symbols file when the address route fails, or accept that this class of
global stays `D_`-named. Worth settling before track 3 gets much further,
because the workaround (renaming by hand) is exactly what `rename.py`
exists to prevent.

## Round 91 polish (delta, track 7)

The body is now libgpu's own macros -- `setSemiTrans(prim, ctx->semiTrans)`,
`setShadeTex(prim, D_8008E248)`, `ctx->primLen = getlen(prim)`,
`ctx->primCode = getcode(prim)` -- byte-identical (29/29). The two ternary
macro statements stay separate, which is the shape this report's
"independent statements" finding requires. `ctx` is the unit's
`PolyDrawCtx`. The function comment lost the POLY_xx (len, code) table (it
is in SortTmdObject's report) and the history.
