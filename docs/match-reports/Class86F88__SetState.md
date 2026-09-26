# Class86F88__SetState -- MATCH

> Renamed from `func_800521D4` on 2026-09-24 (tools/rename.py). Address 0x800521d4.

Unit `class_3bb8c_k`, round 15. `./build-and-verify.sh` exit 0 (checked once
the whole unit's batch of matches was in place); whole-image SHA1 matches
retail. `funcdiff.py Class86F88__SetState`: 42/42 words match.

This is vtable slot `+0x054` of `gClass86F88Methods` (`Class86F88Methods::slot54`),
dispatched by `Class86F88__TickClosing` (this unit) as `self->methods->slot54(self, 4)`.

## Source

```c
void Class86F88__SetState(Class86F88 *self, s32 state)
{
    self->unk30 = 0;
    if (state < 2) {
        goto end;
    }
    if (state < 4) {
        goto case_lt4;
    }
    if (state == 4) {
        goto case_eq4;
    }
    goto end;
case_lt4:
    self->methods->slot14(self, self->unk34);
    self->methods->slot48(self);
    self->unk2C = state;
    goto end;
case_eq4:
    self->methods->slot30(self, self->unk2C);
end:
    return;
}
```

## Notes

A 3-way dispatch on `state` (`<2` no-op, `2..3` one path, `==4` a second
path, `>4` no-op), with `self->unk30 = 0` unconditional at the top (it sits
in the delay slot of the first branch in the disassembly, so it executes
on every call regardless of `state`).

**Neither a nested `if`/`else if` nor an early-return-per-case reproduced
retail's physical block layout or its branch instructions**, even though
both are semantically identical to the final form. Both alternate shapes
compiled 2 words short (a whole `beq`+`nop`+`j`+`nop` 4-instruction group
collapsed into a 2-instruction `bne`), and when the `state==4` code was
written textually before the `state<4` case, GCC swapped which body was
placed first in the physical layout (matching the *source* order, not
retail's ROM order). Only writing the three-way test as explicit
`goto`/label pairs -- with `case_lt4` appearing textually (and therefore
physically) **before** `case_eq4`, replicating retail's own branch
polarities exactly (`bnez` into `case_lt4`, `beq` into `case_eq4`, implicit
fallthrough to `end`) -- reproduced retail's instructions exactly.

### Proposed learning

A three-way dispatch (`if (a) ... else if (b) ... else ...`) where two
branches jump into non-adjacent blocks is not reliably reproduced by
nested `if`/`else if`, even when every value and every condition is
correct -- GCC 2.6.3 chooses its own branch polarity and block order from
the *source's* textual/control-flow shape, not from some canonical
lowering of the semantics. When a residue is "off by a `beq`+`j` pair" or
"the two case bodies are swapped in physical order", try explicit
`goto`/label pairs whose textual order and branch polarity mirror the
disassembly's own `bnez`/`beq`/fallthrough sequence one-for-one, rather
than continuing to reshape nested conditionals.

## Naming

Round 75 (bravo, track 3). `func_800521D4` -> `Class86F88__SetState`, **tier B**.

Slot +0x054 of gClass86F88Methods (`tools/classtable.py gClass86F88Methods`). Clears `closeTicks`; for state 2 or 3 it removes the cached `inputSource` child, releases resources (slot +0x048, Class86F88__ReleaseResources) and stores the state in `result`; for state 4 it calls notifyParents(self, result). Callers: Class86F88__HandleInputCode (2 after input code 25, 3 after code 23) and Class86F88__TickClosing (4). The one parent-side reader, TaskObjF__OnItemListResult (class_3bb8c_g), takes code 2 as "read the selected item" and 3 as the other outcome. Tier B: `SetState` names the mechanics; the states' game meaning (confirm/cancel) is only suggested by that one caller.

Class86F88, per the round-75 pass, is a scrolling list selector: up to 4 visible rows of 26-character item text, a highlighted cursor row, a horizontal column offset (see the unit header comment of `src/class_3bb8c_k.c`).

### Globals and fields named in this pass

Globals (`tools/rename.py`):

| old | new | tier | evidence |
| --- | --- | --- | --- |
| `D_80086F88` | `gClass86F88Methods` | A | the class's method table (39 slots, header word 0x20), returned by GetClass86F88Methods |
| `D_8008AB00` | `gClass86F88RowOriginX` | B | sdata word -0x5C, row 0's x in Class86F88__CreateRows's `pos` |
| `D_8008AB04` | `gClass86F88RowOriginY` | B | sdata word -0xF, row 0's y; each further row +0xA |
| `D_8008AB0C` | `gClass86F88RowColor` | A | bytes 50 50 50 00, passed to the rows' +0x0B8 (TextRow__SetColor in gTextRowMethods) for every non-cursor row |
| `D_8008AB10` | `gClass86F88CursorColor` | A | bytes 80 80 00 00, the same colour slot for the cursor row |

Fields and slots (`include/class_3bb8c.h`, renamed definition-first; every
accessor the compiler listed, in both the build and
`tools/check-nonmatching.sh`, was in class_3bb8c_k, so none are proposed):

- Class86F88: `itemCount` (+0x10), `maxTextLen` (+0x14), `texts` (+0x18),
  `topIndex` (+0x20), `column` (+0x24), `cursorIndex` (+0x28), `result`
  (+0x2C), `closeTicks` (+0x30), `inputSource` (+0x34), `target` (+0x3C),
  `rows[4]` (+0x40), `resource` (+0x50). itemCount/maxTextLen/texts/
  inputSource/resource are confirmed by the same offsets in class_3bb8c_j's
  view (Class86F88__Class86F88, __AddChild, __LoadResources).
- Class86F88Methods: `removeChild`, `notifyParents`, `releaseResources`,
  `setState`, `forwardToTarget`, `scrollRight`/`scrollLeft`/`cursorUp`/
  `cursorDown` (new, +0x07C..+0x088), `refreshRows`, `stepCursorInView`, each
  the method gClass86F88Methods holds at that offset.
- Class86F88ElemMethods: `layout` (+0x04C), `setColor` (+0x0B8), `setText`
  (+0x0CC), from gTextRowMethods, the derived Obj6EAC0 table New_TextRow builds.

## Proposed field names

None: every field and slot renamed this round had accessors only in
class_3bb8c_k. For the head: the TYPE name `Class86F88` is still an address
name; `ListSelector` or similar would fit the reading above, but renaming it
touches class_3bb8c_j's local `Class86F88_3bb8c_j` and 20+ symbols, so it is
left for a pass that owns both units.
