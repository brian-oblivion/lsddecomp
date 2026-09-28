# The Psy-Q inline macro layer is silently inert (CRLF line endings)

**Status: RESOLVED round 91, 2026-09-26 (premium head, plan setup item
`sdk-headers`).** Per the operator's decision of the same day, every header
under `include/psyq/` is now LF and lowercase (as Sony's own `#include`s spell
them, and as the pe2 and sotn decomps keep them), `include/types.h` defines
Sony's `u_*` names under `<sys/types.h>`'s guards, and `src/graphics/DrawSystem.c`
includes `<libgte.h>`, `<libgpu.h>`, `<libgs.h>` and `<libetc.h>` with the image
byte-identical. `setPolyFT4()` and friends expand again. The include route is
FINISHING-PLAN track 6; `tools/sonyheaders.py` lists what still collides. The
paths below were rewritten to the new names. `inline.h` stays unused: its
`gte_*` macros are ASPSX-flavoured (`include/gte.h`, and the section at the end).

**Found: round 12, 2026-09-03, by the head while adjudicating a runner's
proposed learning about GTE stores.**

## The finding

Eight headers under `include/psyq/` have **CRLF** line terminators. In a
multi-line macro that means each continuation line ends `\` `CR` `LF` — and
GCC 2.6.3's `cpp` splices `\` only when it is immediately followed by `LF`.
With a `CR` in between, **the splice does not happen and the macro body is
discarded.**

**1134 multi-line macros are affected:**

| header | broken multi-line macros | CR lines |
| --- | --- | --- |
| `include/psyq/inline.h` | 1043 | 1229 |
| `include/psyq/libgpu.h` | 87 | 805 |
| `include/psyq/libgs.h` | 4 | 1436 |
| `LIBSND.H`, `LIBETC.H`, `LIBPRESS.H`, `LIBMATH.H`, `STDLIB.H` | 0 | 29–275 |

That is the entire Psy-Q **inline GTE layer** (`gte_*`) and most of the
**LIBGPU primitive-setup layer** (`setPolyFT4`, `setRGB0`, …).

## Why it is dangerous rather than merely broken

**The damaged expansion is syntactically valid C.** `#define gte_stsxy3(...) {\`
expands to:

```c
void f(void *dst) {
    {\ ;
}
```

`{ }` followed by `;` — an empty block and a null statement. It compiles
**clean**, with no warning, and does **nothing**. There is no diagnostic
anywhere in the pipeline.

So the failure mode is not a build break. It is: a runner writes
`gte_stsxy3(&p->x0, &p->x1, &p->x2)`, the build goes green, and the function
scores a mismatch with the GTE stores simply *absent* from the output. In a
five-word leaf that is obvious. In a 200-word render function it reads as an
inexplicable codegen divergence, and the header is the last place anyone would
look — because it is a *vendored SDK header that appears correct when read*.

## Nothing is broken today

```sh
grep -rlnE 'gte_[a-z0-9_]+\(' src/     # -> no hits
```

Zero call sites in `src/` today, so this has cost nothing so far and no
matched function depends on it. It is a landmine, not an active bug — which is
exactly why it should be dealt with deliberately rather than discovered by
whoever first carves the renderer.

## Reproducer

Under a second. `gte.c`:

```c
#include "psyq/inline.h"

/* What retail actually contains, hand-rolled: */
void charlie_version(void *dst) {
    __asm__ volatile (
        "swc2 $12, 0x8(%0)\n\t"
        "swc2 $13, 0xc(%0)\n\t"
        "swc2 $14, 0x10(%0)"
        : : "r" (dst) : "memory");
}

/* The SDK macro that ought to express the same thing: */
void sdk_stsxy3(void *dst) {
    gte_stsxy3((char*)dst+0x8, (char*)dst+0xc, (char*)dst+0x10);
}
```

```sh
tools/gcc263/cpp -Iinclude -Iinclude/psyq -undef -lang-c -nostdinc -Dmips -D__GNUC__=2 /tmp/gte.c \
  | tools/gcc263/cc1 -mips1 -mcpu=3000 -quiet -G0 -O2 \
  | .venv/bin/python3 tools/maspsx/maspsx.py --aspsx-version=2.34 --dont-force-G0 --expand-div \
  | tools/binutils/bin/mipsel-linux-gnu-as -march=r3000 -EL -no-pad-sections -G0 -o /tmp/gte.o
tools/binutils/bin/mipsel-linux-gnu-objdump -d /tmp/gte.o
```

Result:

```
00000000 <charlie_version>:
   0:	e88c0008 	swc2	$12,8(a0)
   4:	e88d000c 	swc2	$13,12(a0)
   8:	e88e0010 	swc2	$14,16(a0)
   c:	03e00008 	jr	ra
  10:	00000000 	nop

00000014 <sdk_stsxy3>:
  14:	03e00008 	jr	ra          <-- the macro emitted NOTHING
  18:	00000000 	nop
```

Stop at the preprocessed output to see the mechanism directly:

```sh
tools/gcc263/cpp -Iinclude -Iinclude/psyq -undef -lang-c -nostdinc -Dmips -D__GNUC__=2 /tmp/gte.c \
  | sed -n '/sdk_stsxy3/,/^}/p'
```

```c
void sdk_stsxy3(void *dst) {
    {\ ;
}
```

And the cause, byte-exact:

```sh
sed -n '780p' include/psyq/inline.h | od -c
#   d   e   f   i   n   e       g   t   e   _   s   t   s   x
y   3   (   r   1   ,   r   2   ,   r   3   )       {   \  \r
\n
```

## What this does NOT mean

**It does not mean the escalated remedy is "convert the headers to LF".** That
is the obvious move and it is exactly the kind of thing rule 5 exists to stop a
round from doing on its own: these are pinned vendored SDK assets, un-breaking
1134 macros changes what is available to every future translation unit, and the
right disposition may well be different per header (INLINE.H's GTE layer and
LIBGPU.H's primitive-setup layer are used very differently). Any fix must be
followed by a full `./build-and-verify.sh`, and the interesting question —
whether the *retail* build used these macros at all — is answerable and
unanswered. See below.

**It also does not mean `gte_*` was unavailable to the original programmers.**
The CRLF is an artifact of how these headers reached *this repo*, not of the
1998 build. The original source may well have used `gte_stsxy3` freely.

## The open question this raises, which is the valuable one

Retail's `StoreSxyPolyF3` is:

```
swc2 $12, 0x8($a0)
swc2 $13, 0xC($a0)
swc2 $14, 0x10($a0)
jr   $ra
nop
```

Three `swc2` at immediate offsets off the **incoming argument register**, with
no `move`. Every `gte_stsxy*` macro in this `INLINE.H` begins
`move $12,%0` and then emits `.word` constants — which cannot produce that
sequence even once the CRLF is fixed.

So the six GTE leaves matched in round 12 (`StoreSxyPolyF3`, `StoreSxyPolyG3`,
`StoreSxyPolyFT3`, `StoreSxyPolyGT3`, `StoreSxyPolyF4`, `StoreSxyPolyG4`) are
**not** `gte_stsxy*` call sites. They are either game-local inline asm or a
game-local macro. That is a statement about the *game's* source, and it stands
independently of the CRLF bug — but it could only be established after ruling
the CRLF bug out as the explanation, which is why the two are written up
together.

Whoever takes this: the census above is one command per header and the
reproducer is under a second. Re-measure rather than trusting this table.
