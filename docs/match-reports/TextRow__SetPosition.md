# TextRow__SetPosition — MATCHED (50/50), round 19

> Renamed from `Obj6EAC0__LayoutChildren` on 2026-09-26 (tools/rename.py). Address 0x80040e14.

> Renamed from `func_80040E14` on 2026-09-18 (tools/rename.py). Address 0x80040e14.

Unit: `src/code_2cc8c_f.c`. Blocker screen clean. Callee-saved count 4 --
below saturation.

## Final body (byte-exact, full oracle green)

```c
void TextRow__SetPosition(Obj6EAC0 *self, Pair32E99C *a1) {
    if (self->unkC != 0) {
        Pair32E99C buf;
        s32 i;
        s32 bound;
        Obj6EAC0 **elemp;

        GetCharSpriteMethods()->setPosition((CharSprite *)self, (ScreenSpritePos *)a1);
        buf = *a1;
        i = 0;
        elemp = self->unkB4;
        if (i < self->unkA9) {
            do {
                (*elemp)->methods->slotBC(*elemp, &buf);
                buf.a += self->unkB0;
                bound = self->unkA9;
                elemp++;
                i++;
            } while (i < bound);
        }
    }
}
```

`Pair32E99C` is the existing two-`s32` record from `Class6E99C__PushPosition`; this
is the same type `BoxFill__SetPosition` (this unit's `slotBC` base occupant, also
matched this round) uses for its own `a1`. `Obj6EAC0Methods::slotBC` was
already retyped to `Pair32E99C *a1` for that function, so this derived
occupant picks up the same prototype for free.

`build-and-verify.sh` exits 0 (`OK: build matches retail SLPS_015.56`) --
genuinely whole-image byte-exact, not a per-function read.

## Round-2 history (superseded, kept for the record)

The original report reached 15/50 at the WRONG total length (52 words vs
retail's 50) with a `s32 buf[2]` stack array copied field-by-field
(`buf[0]=a1[0]; buf[1]=a1[1];`). Two things closed the gap:

1. **The whole-struct-assignment axis** (same lever as `Class6E99C__PushPosition`/
   `BoxFill__SetPosition`/`TaskCore__SetColors`/`BoxFill__ApplyColor` this round and last):
   replacing the two scalar array-element copies with one `buf = *a1;`
   struct assignment (after retyping the buffer from a raw `s32[2]` to
   the existing `Pair32E99C` type) closed the length gap outright,
   48/50 immediately -- the two-scalar-copy shape was adding real
   instructions retail doesn't have, not just picking different
   registers.
2. **A pure delay-slot-filler scheduling residue, closed by statement
   order alone.** With the length fixed, the only remaining difference
   was which of two independent instructions (`i++` and a `nop`) filled
   which of two adjacent load-delay slots in the loop body -- both
   instructions are used, both slots exist, only their PAIRING differs.
   Confirmed NOT a register-identity issue (same register, `$s0`,
   throughout both versions -- CLAUDE.md rule 6's test for a banned fix
   does not even apply here, since nothing about WHICH register holds a
   value ever differed). Swapping the source order of `elemp++;` and
   `i++;` (previously `i++` before `elemp++`; retail-matching order is
   `elemp++` before `i++`) closed it exactly. A bare
   `__asm__("" ::: "memory")` barrier between the accumulate and the
   increment, tried first, had NO effect (expected -- the barrier
   anchors memory operations, and this residue is a register-only
   scheduling choice with no memory access involved).

### Proposed learning

A second confirmation (after `Class6E99C__PushPosition`'s "statement order flips
register allocation elsewhere in the function") that **plain statement
ORDER inside a loop body can decide which of two adjacent, otherwise
symmetric delay slots gets which independent instruction**, even when
every other aspect of the codegen (registers used, total instruction
count) is already correct. Where a residue is "instruction A and
instruction B are both present but placed in each other's slot," try
swapping the SOURCE order of the two corresponding statements before
reaching for a barrier -- a barrier only helps when the residue is a
memory-ordering question, and this one wasn't.

## Provenance

Round 2 (2026-09-0x), unspecified runner: original 2 attempts, restored to
`INCLUDE_ASM` at 15/50 (wrong length). Round 19, runner alpha: closed in
4 attempts (struct-copy retype; barrier, no effect; statement-order swap,
closed it).

## Naming

Round 54 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_80040E14` | `TextRow__SetPosition` | B |

**Evidence.** Derived occupant of `slotBC` (same slot
`BoxFill__SetPosition` fills for the base/leaf case): when
`self->hasChildren != 0`, walks `self->children[0..self->totalChildCount)`
dispatching `(*elemp)->methods->slotBC(*elemp, &buf)` on each, with
`buf.a` (posX) incremented by `self->childPitch` every iteration -- a
recursive "place each child along a line, `childPitch` apart" layout
pass. Mechanics well-established from the loop shape itself; the
in-game purpose (text layout, per this unit's own header-comment
hypothesis) is not independently confirmed, hence tier B. Sibling of
`TextRow__AttachToParent` (same shape, plus one extra offset at
`gapIndex`).

## Track 4

2026-09-26, round 86 (bravo): the parent class 0x1144 is unified as CharSprite (`include/CharSprite.h`, formerly D_8006EC74). The base call is `GetCharSpriteMethods()->setPosition((CharSprite *)self, (ScreenSpritePos *)a1)` (was `->slotBC(self, a1)`): an upcast and a cast of the view's Pair32E99C to ScreenSprite's pair, no code. Image byte-identical.

## Track 4 (2026-09-26, round 88, charlie)

2026-09-26, round 88 (charlie): class 0x11144 unified as TextRow in `include/TextRow.h` (a row of CharSprite cells: the ctor makes `count` New_CharSprite cells, setText hands each the next byte of a string, the layout slots step `cellPitch` along x). The view `Obj6EAC0` (named after BoxFill's old table address) is gone; its +0x00C `hasChildren` is SceneNode's `parent` (--merge CONFLICT s32 vs pointer: only tested against 0, bytes unchanged), `children` (+0x0B4) is `CharSprite **cells`, and the per-cell calls go through CharSprite's slots by name. Renamed from `Obj6EAC0__LayoutChildren`: the +0x0BC setPosition occupant, `(TextRow *self, ScreenSpritePos *pos)`. While attached it sets its own position through CharSprite's slot, then all `cellCount` cells at x + i*cellPitch. Image byte-identical; the current source is src/code_2cc8c_f.c.
