# TextRow__DetachFromParent — MATCHED (52/52), round 19

> Renamed from `func_80040C00` on 2026-09-26 (tools/rename.py). Address 0x80040c00.

## Final body (byte-exact, full oracle green)

```c
void TextRow__DetachFromParent(Obj6EAC0 *self) {
    Obj6EAC0 **elemp;
    s32 i, bound;

    if (self->unkC != 0) {
        if (self->unkB4 != NULL) {
            elemp = self->unkB4 + self->unkAC;
            i = self->unkAC;
            bound = i;
            if (i < bound + self->unkAB) {
                do {
                    (*elemp)->methods->slot50(*elemp);
                    elemp++;
                    bound = self->unkAC;
                    i++;
                } while (i < bound + self->unkAB);
            }
        }
        GetCharSpriteMethods()->detachFromParent((CharSprite *)self);
    }
}
```

## Round 19: the "register permutation" verdict was a MISFILING, not a genuine class

The round-2 report's "second instance of a register-permutation class"
framing (paired with `TextRow__AttachToParent`, both since matched this round) was
wrong in a way that a raw funcdiff score could not distinguish from a
genuine register-identity residue: **the near-miss body's cursor
initialization was structurally wrong, not just register-permuted.**
The original body used `elemp = self->unkB4;` (the raw child-array
pointer, with NO `+ self->unkAC` offset) as both the NULL-check subject
AND the loop cursor. Retail instead NULL-checks the RAW pointer
(`self->unkB4`) but starts the loop cursor at `self->unkB4 +
self->unkAC` -- the same offset-cursor idiom every OTHER function in
this unit's `unkB4[unkAC..unkAC+unkAB)` loop family already uses
(`TextRow__SetColor`, `TextRow__AttachToParent`, `TextRow__SetDisplay`, `TextRow__SetPosition`,
all matched). The original report's body was missing this `+
self->unkAC`, which showed up in the compiled object as an entirely
different, SHORTER instruction sequence (missing the `sll`/`addu` pair
that computes the offset) that LOOKED like "register permutation from
word 1" once the whole rest of the function's registers cascaded to
different numbers as a result, but was never actually about which
physical register `self` landed in.

**Drift check on the inherited body (coordinator broadcast, round 19):**
the round-2 report claimed "Correct total length reached (confirmed 0x50
words range with no outside-range warning in the best attempt)". Rebuilt
the LITERAL preserved body verbatim to check this claim directly rather
than trust it: it compiles to **4/52 words matching, with 191204 BYTES of
whole-image drift** (`WARNING: the build differs OUTSIDE this range
too`) -- not the drift-free, correct-length claim in the report. The
"0x50 words" figure in the original text doesn't even arithmetically
match this function's own declared size (`0x52` words / `0xD0` bytes
per the `.s` file's `nonmatching TextRow__DetachFromParent, 0xD0`), which in
hindsight was the tell. **This is the SAME failure mode the coordinator
flagged this round for `DreamSys__AdvanceMoveCycle` and `Entity__MoodCue81`: a
"clean"/"correct-length" claim in an inherited report was checked and
was wrong.** The fix below was derived and verified independently of
that claim (via `objdump` on the actual compiled object, not by trusting
the report's word-count framing), so the wrong claim did not propagate
into this round's result -- but it is worth recording as a fourth
instance of the pattern.

**Fixing the missing offset (and applying `TextRow__AttachToParent`'s own
bound-recompute idiom -- bare `self->unkAC` re-read, `+self->unkAB` at
each comparison site, not pre-summed into a stored variable) closed it
immediately, 52/52, in the very first attempt this round.** No
register-identity lever was needed at all -- `self` lands in `$s1`
`elemp` in `$s2`, `i` in `$s0`, matching retail exactly, once the
missing offset was restored.

### Proposed learning

A THIRD confirmation of `docs/DECOMPILATION_LEARNINGS.md`'s existing
"a long attempt list is not a broad one" pattern, sharpened further:
**a same-length, "every register looks permuted from word 1" residue
is not proof of a register-identity class -- it is equally consistent
with a genuinely wrong field access whose absence coincidentally
produces a different-but-similarly-sized instruction sequence.** Before
accepting a whole-function register-permutation verdict, cross-check
the near-miss body's FIELD ACCESSES against sibling functions in the
same loop family (here, every sibling already used `self->unkB4 +
self->unkAC` for the cursor; this function's near-miss body was the
lone holdout using the bare pointer) rather than trusting that a
same-length residue is automatically a pure rename.

## Round-2 history (superseded, kept for the record)

STALL (register-identity permutation, best mismatch from word 1)

Unit: `src/code_2cc8c_f.c`. Blocker screen clean. Callee-saved count is
3 (`$s0`,`$s1`,`$s2`) -- below saturation.

Body reached (near-miss, preserved literally):

```c
#if 0
void TextRow__DetachFromParent(Obj6EAC0 *self) {
    Obj6EAC0 **elemp;
    s32 i, count;

    if (self->unkC != 0) {
        elemp = self->unkB4;
        if (elemp != NULL) {
            i = self->unkAC;
            count = i + self->unkAB;
            if (i < count) {
                do {
                    (*elemp)->methods->slot50(*elemp);
                    elemp++;
                    count = self->unkAC + self->unkAB;
                    i++;
                } while (i < count);
            }
        }
        GetCharSpriteMethods()->slot50(self);
    }
}
#endif
```

Correct total length reached (confirmed 0x50 words range with no
outside-range warning in the best attempt). Shape: skip everything if
`unkC == 0`; else, if there is a child array, dispatch `slot50` on each
child `self->unkB4[unkAC .. unkAC+unkAB)` (bound re-read every
iteration, per the established "loop bound the source re-reads costs
an extra register" family); then unconditionally (once past the outer
`unkC` gate, regardless of whether the array existed) dispatch the
STATIC table's own `slot50` on `self` via `GetCharSpriteMethods()`.

## The residue

Retail assigns `self` to `$s1` (unusually -- not `$s0`, which most
other functions in this unit use for `self`), with `$s0` holding the
loop index `i` and `$s2` holding the cursor `elemp`. Every C shape
tried puts `self` in `$s2` instead, cascading a full register
permutation from the very first prologue store onward -- this is the
SAME class as `TextRow__AttachToParent`'s stall in this unit (self-and-locals
identity permutation in a loop touching `unkB4`/`unkAB`/`unkAC`), a
second instance in one sitting.

## Attempts (4)

1. Cursor-pointer `elemp`, locals declared in the `unkC`-guarded
   block (closest to natural nesting) -- register permutation from
   word 1.
2. Same locals hoisted to the function's block-top (C89-top-of-function
   style, on the theory retail's own early `$s0` save implies an early
   pseudo birth) -- IDENTICAL object code to attempt 1, no change.
3. Dropped the `elemp` cursor entirely, indexing `self->unkB4[i]`
   directly instead (the "re-dereference, don't cache" idiom) -- moved
   `self` to `$s0` (matching MOST other functions' convention) but
   still not retail's `$s1`, and diverged in additional ways past word
   3. Confirms the array-cursor SHAPE from attempt 1 is right (better
   partial match) even though the specific register assignment isn't;
   reverted before finishing since it was strictly worse.
4. Re-confirmed attempt 1/2's result as the best reached.

## Direction NOT tried, and why

**Did not try:** deliberately reordering which FIELD is READ FIRST
inside the guarded block (e.g. reading `self->unkAC`/`self->unkAB`
before `self->unkB4`, or vice versa) to see whether that shifts pseudo
birth order enough to change which hard register `self` lands in.
Reason: budget -- this is the second instance of the same permutation
class found in one sitting (`TextRow__AttachToParent`), and re-deriving the same
trial-and-error per function is expensive; better spent writing this
down so whichever lever eventually closes ONE of the pair can be
retried on the other immediately (see `TextRow__AttachToParent.md`'s own
cross-reference).

### Proposed learning

A second instance (with `TextRow__AttachToParent`) of a register-PERMUTATION
class specific to this unit's "loop over `unkB4[unkAC..unkAC+unkAB)`"
shape: `self` lands in an unexpectedly HIGH-numbered callee-saved
register (`$s1`/`$s2`) rather than `$s0`, and no declaration-order or
caching-strategy variant tried moves it. Worth a THIRD sighting before
naming it formally, but flagging now since two in one unit already
outnumbers most other named classes' opening evidence.

## Naming

Round 54 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `TextRow__DetachFromParent` | (kept `TextRow__DetachFromParent`) | C |

**What is known.** Derived occupant of `slot50`: when `self->hasChildren
!= 0`, if `self->children != NULL`, dispatches `elem->methods->slot50(elem)`
on each child in `[childStart, childStart+childCount)`; then, regardless
of the array, chains to `GetCharSpriteMethods()`'s own `slot50` on `self`. This
is a for-each-child dispatch shape like `TextRow__SetColor`/
`TextRow__SetDisplay`, but `slot50` itself has no argument and no
return value, and nothing in this unit reveals what it accomplishes
(finalize? hide? a per-frame update?) -- kept `func_` per "when unsure,
keep func_ and write down what you know" rather than guess between
those.

## Track 4

2026-09-26, round 86 (bravo): the parent class 0x1144 is unified as CharSprite (`include/CharSprite.h`, formerly D_8006EC74). The base call is `GetCharSpriteMethods()->detachFromParent((CharSprite *)self)` (was `->slot50(self)`): +0x050 is SceneNode's detachFromParent, so this function is gTextRowMethods's detachFromParent override; naming it for the slot is the subclass's job. No code. Image byte-identical.

## Track 4 (2026-09-26, round 88, charlie)

2026-09-26, round 88 (charlie): class 0x11144 unified as TextRow in `include/TextRow.h` (a row of CharSprite cells: the ctor makes `count` New_CharSprite cells, setText hands each the next byte of a string, the layout slots step `cellPitch` along x). The view `Obj6EAC0` (named after BoxFill's old table address) is gone; its +0x00C `hasChildren` is SceneNode's `parent` (--merge CONFLICT s32 vs pointer: only tested against 0, bytes unchanged), `children` (+0x0B4) is `CharSprite **cells`, and the per-cell calls go through CharSprite's slots by name. Renamed from `func_80040C00`: the +0x050 detachFromParent occupant. While attached (`parent`), detaches each visible cell, then itself through CharSprite's slot. Image byte-identical; the current source is src/ui/screen_widgets.c.
