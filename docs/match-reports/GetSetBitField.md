# GetSetBitField -- MATCHED (22/22, round 18 permuter pass)

> Renamed from `func_8001EDAC` on 2026-09-17 (tools/rename.py). Address 0x8001edac.

Unit: `SceneNode` (round 14). The generic packed-bitfield accessor
already MEASURED and documented (before this carve existed) in
`include/SceneNode.h`'s standing comment: clears `width` bits at bit
offset `shift` in `*word`, ORs in `value << shift`, and returns the
PREVIOUS contents of that bitfield shifted back to bit 0. Five (now more)
sibling functions in this class of unit are thin wrappers around it, all
over `&self->unk10`. `u32 GetSetBitField(u32 *word, s32 shift, s32 width,
u32 value)`.

Blocker screen clean: no `gp_rel`, no `addiu $at,$at,%lo`, no
`mfhi`/`mflo`-adjacent-`mult`/`div` hit.

## Best-reached source (compiles, builds green, scores 16/22 in-range;
## restored to `INCLUDE_ASM` per project rule)

```c
u32 GetSetBitField(u32 *word, s32 shift, s32 width, u32 value) {
    u32 mask;
    s32 i;
    u32 old;

    mask = 1;
    for (i = 0; i < width; i++) {
        mask <<= 1;
    }
    mask -= 1;
    mask <<= shift;
    old = (*word & mask) >> shift;
    *word = (*word & ~mask) | (value << shift);
    return old;
}
```

Preserved inline per project convention (`#if 0`, positioned where it
would compile back into `src/SceneNode.c` in place of the current
`INCLUDE_ASM`):

```c
#if 0
u32 GetSetBitField(u32 *word, s32 shift, s32 width, u32 value) {
    u32 mask;
    s32 i;
    u32 old;

    mask = 1;
    for (i = 0; i < width; i++) {
        mask <<= 1;
    }
    mask -= 1;
    mask <<= shift;
    old = (*word & mask) >> shift;
    *word = (*word & ~mask) | (value << shift);
    return old;
}
#endif
```

## The residue, precisely

Every instruction's OPCODE, IMMEDIATE, and POSITION match retail exactly
(confirmed via `tools/asm-differ/diff.py`: 22 words in, 22 words out, same
count both sides -- no address drift, unlike most residues in this
project's history). The only difference is that TWO registers are
swapped throughout the second half of the function:

- The loop counter (retail: `$a0`, reusing the now-dead `word` parameter
  register once its value has been copied to `$t1`; mine: `$v1`).
- The `~mask` intermediate and the `value << shift` intermediate (retail:
  `$a0`/`$v1` respectively; mine: `$v1`/`$a0` -- literally swapped from
  retail's assignment).

No instruction is missing, extra, or reordered; this is the "same
register, same value... resists reshaping" class from
`docs/MATCHING-GUIDE.md`, just for a register PAIR rather than a single
redundant move.

## What was tried (7 real full builds, all scoring identically at 16/22 --
## the score never moved even by one word across any of these)

1. **Baseline** (source above).
2. **Explicit pointer walk / statement reorder**: moved `new_word`
   (renamed from the inline expression) to be computed BEFORE `old`,
   matching retail's own `v1`-before-`v0` instruction order. No change.
3. **Operand order swap** on the final OR: `(value << shift) | (*word &
   ~mask)` instead of `(*word & ~mask) | (value << shift)` -- matching
   retail's literal `v1 = v1 | a0` (value-shifted first, cleared-word
   second). Made it WORSE (14/22): this is NOT the axis retail's operand
   order reflects; reverted immediately.
4. **Named `orig` local caching `*word` once**, used in both the "clear"
   and "extract" expressions instead of dereferencing `*word` twice
   (which the compiler CSEs identically either way). No change.
5. **`for` loop rewritten as `while`** with the increment split out as its
   own statement (`i = 0; while (i < width) { i++; mask <<= 1; }`). No
   change -- both compile to the identical rotated-loop shape retail's
   own `blez`-guarded post-test loop has.
6. **Fewer/more locals** (collapsing `old` into the `return` expression
   directly, vs. splitting `new_word`/`old` into two named locals). No
   change in either direction.

None of these are the banned `register T v asm("$N")` class -- nothing
here pins WHICH register a value lives in; every attempt is a pure
source-level reshape (statement order, loop form, local variable count),
exactly what `docs/MATCHING-GUIDE.md`'s residue guide prescribes trying
before calling a register-identity mismatch a stall.

### Proposed learning

**A register PAIR can be swapped between two otherwise-independent
intermediate values with zero effect from statement reordering, loop-form
changes, or intermediate-naming -- confirmed here across 6 source-level
axes with the score frozen at the same 16/22 every time.** This
generalizes the existing single-register "redundant move" entry in
`docs/MATCHING-GUIDE.md` to the PAIRED case: when two scratch values
(here, a loop counter freed up from a dead parameter register, and two
unrelated bitwise intermediates) get consistently cross-allocated versus
retail, treat it as the same permuter-target class rather than continuing
to vary source shape -- the six variants tried here span every reasonable
axis (order, form, naming) without moving either register.

## Round 18 (permuter pass, charlie): MATCHED

`permuter.py --debug` first confirmed the scaffold reproduces this exact
residue (`Register Differences: 8`, base score 40, consistent with the
16/22 recorded above -- permuter's scorer counts the two swapped registers
across all their occurrences). Ran the bounded search: `timeout 480
permuter.py -j 6 --stop-on-zero --best-only`. **Zero found at iteration
975** (`permuter exit=0`, the search stopped itself, not a timeout kill).

**The winning form is a plain, idiomatic source change**, not UB or a
duplicate-arm trick -- splitting the final combined expression into two
sequential statements:

```c
/* was (16/22): */
*word = (*word & ~mask) | (value << shift);

/* zero-scoring form: */
*word = *word & ~mask;
*word = *word | (value << shift);
```

Nothing else changed from the preserved 16/22 body. This is the same
"register PAIR consistently cross-allocated" residue described above
(`~mask` and `value << shift` swapping `$a0`/`$v1` against retail) --
apparently GCC 2.6.3's register allocator makes a different choice for
which pseudo gets which hardware register when the AND and OR are two
statements each assigning to `*word` versus one expression combining both
before a single store, even though the two forms are semantically
identical and (per the six prior manual attempts) statement order,
operand order, and intermediate-naming alone hadn't found this specific
split.

**Translated to `src/SceneNode.c` verbatim and reverified with the real
oracle** (not just the permuter's own scorer):

```
build exit=0
GetSetBitField: 22/22 words match (file 0xF5AC-0xF604)
OK: build matches retail SLPS_015.56
```

Full match, whole-image green. `include/SceneNode.h`'s comment on this
function's role (generic packed-bitfield accessor) is unaffected -- no
struct or signature changes, so no other unit is affected.

### Proposed learning (supersedes the single-source "register PAIR" entry above)

**A register-pair swap that resists 6 statement-order/operand-order/
naming reshapes can still be a single-expression-vs-two-statements
question** -- collapsing two independent stores to the same lvalue into
one expression (`a = (a & m) | b;`) versus writing them as two plain
statements (`a = a & m; a = a | b;`) is a NINTH axis this residue class
had not been tried on, and it is exactly the kind of small, semantically
inert rewrite the permuter's random statement-level mutation finds fast
(975 iterations here) that manual guessing had not reached in 7 real
attempts. Worth trying this specific split early on any future
"register pair swapped, both semantically-transparent reshapes exhausted"
residue, before spending a permuter budget on it.

## Naming (round 50, charlie -- FINISHING-PLAN track 3)

- **`func_8001EDAC` -> `GetSetBitField`. Tier A.** Free function, complete
  semantics: it replaces the `width` bits at bit offset `shift` of `*word`
  with `value` and returns that field's PREVIOUS contents shifted down to
  bit 0. The `GetSet` prefix is this project's own established spelling for
  the swap shape (`DreamSys__GetSetScreenShake`,
  `DreamSys__GetSetDreamTimeLimit` in the symbols file) and is load-bearing
  here: several callers use the return value
  (`return GetSetBitField(&self->unk10, 0x1F, 1, a1 == 0) == 0;`).
- Corroborated at scale: thirteen one-line wrappers across `SceneNode.c`,
  `SceneNode.c` and `ScreenWidgets.c` call it at fixed, non-overlapping
  (shift, width) pairs over one word -- the per-field setters of a packed
  register. That word is `SceneNodeObj::unk10`, which the PSY-Q
  IDENTIFICATION note in include/SceneNode.h pins as `GsDOBJ2.attribute`.
- The mask is built by a loop rather than `(1 << width) - 1`; that is
  retail's own source shape and the name does not assert otherwise.


## Round 95 (bravo): moved from include/SceneNode.h

The header's banner was rewritten as documentation in round 95; the comment it carried about this function, verbatim:

```c
/* GetSetBitField (round 54 correction: this banner was STALE -- it is
 * now carved and MATCHED in src/code_d294_c.c, not SceneNode): a
 * generic packed-bitfield accessor. Given a word pointer, a bit SHIFT, a
 * bit WIDTH and a VALUE, it clears WIDTH bits at bit-offset SHIFT in *word,
 * ORs in (value << shift), and returns the PREVIOUS contents of that
 * bitfield (shifted back down to bit 0). MEASURED from its own disassembly
 * (asm/SceneNode.s @ GetSetBitField): a `while` loop builds `(1 << width)
 * - 1` one bit at a time (i.e. computes a WIDTH-bit mask, not a
 * `(1<<width)-1` closed form -- retail's own source apparently spelled it
 * as the loop), then shifts that mask into position, clears/sets, and
 * shifts the old value back down. Five of this unit's own functions
 * (SceneNode__SetDisplay/D374/D3A0/D3CC/D3F8) are thin wrappers around this,
 * always over `&self->unk10`, at five non-overlapping bit positions
 * (shift 3 width 3, shift 6 width 1, shift 28 width 2, shift 30 width 1,
 * shift 31 width 1) -- i.e. self->unk10 is a packed flags/small-fields
 * register and these five functions are its per-field setters. */
```

## Round 98 (echo): track 7, moved from src/SceneNode.c

The mask loop keeps a one-line `MATCHING:` note in the source.

The source comment was rewritten as documentation; the one it replaced, verbatim (field names as they were then):

```c
/* Packed-bitfield accessor: replaces the `width` bits at bit `shift` of
 * `*word` with `value` and RETURNS the previous contents of that field,
 * shifted down to bit 0. The mask is built one bit at a time by a loop
 * rather than as `(1 << width) - 1`; that loop is retail's own shape, not
 * an artefact. Thirteen thin per-field setters across SceneNode.c,
 * SceneNode.c and ScreenWidgets.c are wrappers around this. */
```
