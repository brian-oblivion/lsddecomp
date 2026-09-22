# DreamSys__SaveLinkSnapshot / DreamSys__RestoreLinkSnapshot

> Renamed from `func_8005B904` on 2026-09-22 (tools/rename.py). Address 0x8005b904.

**Unit:** DreamSys · **Sizes:** 35 words / 36 words · **Status:** BOTH MATCHED (byte-exact, full build verified)
**Vtable slots:** `DREAMSYS_METHODS +0x220` / `+0x224`

Documented together: they are a save/restore pair for the same struct, and
neither makes sense read in isolation.

## Context

Both functions were previously named-only placeholders
(`void *DreamSys__SaveLinkSnapshot; void *DreamSys__RestoreLinkSnapshot;`) with a comment guessing they
were "driven by a length read from `this->unk_0x14`". That undersold it:
`unk_0x14` is a POINTER (already documented, just untyped beyond `void *`),
and these two functions are a save/restore pair for the WHOLE struct it
points to, plus a second struct reached through one of its fields.

Both bodies are the confirmed "whole-struct assignment" block-move idiom
(`DECOMPILATION_LEARNINGS.md`: "batches 4 words per iteration cycling four
temp registers... 2.6.3's inlined block-move for a struct assignment").
`DreamSys__SaveLinkSnapshot` copies `*this->unk_0x14` (0x50 bytes) then
`*this->unk_0x14->unk_0x44` (0x28 bytes) into a scratch area at
`this+0x890` (inside the struct's already-documented, still partly-unclaimed
allocator tail -- see `DreamSys::unknown_values_0x890`'s history in the
header). `DreamSys__RestoreLinkSnapshot` copies the SAME two regions back, then clears
`unk_0x14->unk_0x0` to 0.

This resolves concrete new fields of the struct `unk_0x14` points to,
previously only known to hold "a 3-word vector at +0x38" (read by
`Class6B5CC__LocalOffsetToWorldPos`/`func_8005942C`, both still `INCLUDE_ASM`):

- `+0x0`: a word, cleared to 0 by `DreamSys__RestoreLinkSnapshot`'s restore, after the rest
  of the struct has already been overwritten.
- `+0x38`: the pre-existing 3-word vector (unchanged, just now embedded in
  a named struct instead of floating as a comment).
- `+0x44`: a pointer to a SECOND struct (0x28 bytes), block-copied (not
  just followed) by both functions.
- `0x4..0x38` and `0x48..0x50`: unconfirmed padding, never read by either
  function.

## The C

```c
typedef struct DreamSysUnk14Tail {
	s32 raw[0x28 / 4];
} DreamSysUnk14Tail;

typedef struct DreamSysUnk14 {
	s32 unk_0x0;
	s32 unknown_values_0x4[0x34 / 4];
	s32 vec[3];
	DreamSysUnk14Tail *unk_0x44;
	s32 unknown_values_0x48[0x8 / 4];
} DreamSysUnk14;
```

`DreamSys::unk_0x14` retyped from `void *` to `DreamSysUnk14 *`.
`DreamSys::unknown_values_0x890[0x78]` split into the two fields these
functions actually copy into/out of:

```c
DreamSysUnk14 unk14Snapshot;
DreamSysUnk14Tail unk14TailSnapshot;
```

```c
void DreamSys__SaveLinkSnapshot(DreamSys *this)
{
	DreamSysUnk14 *p = this->unk_0x14;

	this->unk14Snapshot = *p;
	this->unk14TailSnapshot = *p->unk_0x44;
}

void DreamSys__RestoreLinkSnapshot(DreamSys *this)
{
	DreamSysUnk14 *p = this->unk_0x14;

	*p = this->unk14Snapshot;
	*p->unk_0x44 = this->unk14TailSnapshot;
	p->unk_0x0 = 0;
}
```

## Derivation notes -- the one real residue, and its fix

**First attempt (wrong):** writing the two statements against
`this->unk_0x14` directly each time --

```c
this->unk14Snapshot = *this->unk_0x14;
this->unk14TailSnapshot = *this->unk_0x14->unk_0x44;
```

-- compiled clean but drifted the WHOLE FILE (`build-and-verify.sh` failed
the SHA1 check; `funcdiff.py` reported 0/35 and 0/36 with an explicit
"differs OUTSIDE this range" warning). Disassembling the built object
showed why: GCC reloaded `this->unk_0x14` a SECOND time (an extra `lw` +
`nop`) before dereferencing `->unk_0x44`, instead of reusing the register
from the first statement's load. This project's assumption that repeated
reads of an unchanging field get trivially CSE'd across separate C
statements at `-O2` does NOT hold here -- the intervening whole-struct
assignment apparently confuses (or resets) that local value tracking.

**Fix:** cache the pointer in an explicit local exactly once --

```c
DreamSysUnk14 *p = this->unk_0x14;
```

-- and dereference through `p` for every subsequent access in the same
function. This is safe for `DreamSys__RestoreLinkSnapshot` specifically because caching
the POINTER (the address stored in `this->unk_0x14`) is not the same as
caching the STRUCT CONTENT it points to: the restore's second statement
(`*p->unk_0x44 = ...`) still issues a fresh memory read of `p->unk_0x44`
AFTER the first statement overwrites `*p`, which is exactly what retail
does (it re-reads offset `0x44` from the just-restored struct, not a
stale cached value) -- only the ADDRESS `p` itself needed to stop being
reloaded, and a local variable is the natural way to say that in C.

With this one change both functions matched immediately, byte-exact,
confirmed by a full `./build-and-verify.sh` pass (not just the
per-function `funcdiff.py` window).

### Proposed learning

**A repeated read of an unchanging pointer field is NOT reliably CSE'd
across separate statements in this compiler when a whole-struct assignment
sits between the reads** -- `DreamSys__SaveLinkSnapshot`/`DreamSys__RestoreLinkSnapshot` both reloaded
`this->unk_0x14` a second time (extra `lw`+`nop`, full-file address drift)
when written as two independent `this->unk_0x14`-dereferencing statements.
Caching the pointer into an explicit local (`DreamSysUnk14 *p =
this->unk_0x14;`) and dereferencing through `p` closed it immediately, in
both the read direction (`DreamSys__SaveLinkSnapshot`, no intervening writes to `*p`)
and the write direction (`DreamSys__RestoreLinkSnapshot`, where the SECOND access
deliberately still needs a fresh read of `p->unk_0x44` after the first
statement overwrites `*p` -- caching the pointer, not the pointee, is what
matters). Worth checking any other multi-step access through the same
`this->fieldPtr` before assuming CSE will handle it for free -- this
project had at least two prior struct-assignment idioms confirmed
(`func_80025E1C`) but none combined with a repeated pointer-field read like
this.

## Attempt log (abbreviated)

1. Direct `this->unk_0x14` dereference in both statements of both
   functions -- compiled, but 0/35 and 0/36 with full-file drift (extra
   reload of `this->unk_0x14` before the `->unk_0x44` access).
2. Introduced `DreamSysUnk14 *p = this->unk_0x14;` in both functions,
   dereferencing through `p` thereafter -- full match, both functions,
   confirmed by a clean `./build-and-verify.sh`.

## Provenance

round 2026-09-02, runner ALPHA, unit DreamSys.
