# SceneNode__DispatchLinkCommand -- MATCHED (24/24 words)

> Renamed from `Class6B5CC__DispatchLinkCommand` on 2026-09-26 (tools/rename.py). Address 0x8001d6b4.

> Renamed from `func_8001D6B4` on 2026-09-18 (tools/rename.py). Address 0x8001d6b4.

Round 12, runner delta. `code_d294`.

## Summary

`a2` selects one of three behaviors: 2 or 3 dispatches through the vtable
(`self->methods->slotA0(self)`), exactly 4 stores `a1` into `self->unk28`,
and anything else (`< 2` or `> 4`) is a no-op.

```c
void SceneNode__DispatchLinkCommand(SceneNodeObj *self, s32 a1, s32 a2) {
    switch (a2) {
    case 2:
    case 3:
        self->methods->slotA0(self);
        break;
    case 4:
        self->unk28 = a1;
        break;
    }
}
```

## The `if`/`else if` forms do NOT match -- only `switch` does

Tried three logically-equivalent `if` shapes before this one, all producing
a wrong-size function (10-13/24, with the outside-range warning firing):

1. Nested `if (a2 >= 2) { if (a2 >= 4) { if (a2 != 4) return; eq4; } else { lt4; } }`
2. `if (a2 < 2) return; if (a2 < 4) { lt4; } else if (a2 == 4) { eq4; }`
3. Same as 2 but with an explicit `return;` after `lt4;` instead of `else`

All three compiled to the SAME wrong shape: cc1 inlined the `a2 < 4` body
immediately after a `beqz`-skip, with the `a2 == 4` test reached by falling
through and testing via `bne ... skip`. Retail does the opposite: the `a2 < 4`
body (6 words: `lw`/`nop`/`lw`/`nop`/`jalr`/`nop`) is placed OUT OF LINE,
reached by a forward `bnez` past the `a2 == 4` test, which is inlined
directly (1 word: `sw`) into the fallthrough path. Read as a compiler
size-based layout choice (put the bigger body out of line, thread the
smaller one straight through) that only the `switch` form happens to
trigger -- not something distinguishable by rewriting the `if` shape.

The retail control flow (`a2 < 2` -> early exit; `a2 < 4` -> case; `a2 == 4`
-> case; else -> nothing) doesn't actually need a range check or jump table
for a `switch` over `{2, 3, 4}` -- GCC 2.6.3 at `-O2` compiled it as the same
two sequential `slti` comparisons retail has, not a table (confirmed: no
`jtbl_*`/`%lo(jtbl` in the built object for this function, and the full
image SHA1 is green).

## Struct/header additions

- `SceneNodeMethods::slotA0` (+0x0A0, new field): dispatched with only
  `self` (the `jalr` leaves `$a0` untouched from function entry).
  `tools/classtable.py gSceneNodeMethods` confirms this slot's occupant is
  `SceneNode__TryAttachNearby` (still queued, this unit).
- `SceneNodeObj::unk28` (new field, `s32`): m2c inferred `s32` from `a1`'s
  own usage; never dereferenced by this unit's chosen functions, so nothing
  narrows it further.

## Evidence

Disassembly (`asm/nonmatchings/code_d294/SceneNode__DispatchLinkCommand.s`).

### Proposed learning

**A logically-equivalent `if`/`else`/early-return rewrite is not always
enough to match GCC 2.6.3 -O2's block-placement choice -- when a `switch`
is a natural fit for the case values (even a small, non-table one), try it
before iterating further on `if` shapes.** Here `switch` matched on the
first try after three failed `if` variants that were CFG-identical to each
other and to the `switch` at the C level, but compiled to a different
physical body-out-of-line-vs-inline layout.

## Naming (round 54, bravo, track 3)

**Not renamed -- PROPOSED only.** Proposed name: `SceneNode__DispatchLinkCommand`
(tier B). `a2` selects one of three behaviors: `2`/`3` dispatch through
`self->methods->slotA0` (occupant `SceneNode__TryAttachNearby`, proposed
`SceneNode__TryAttachNearby` below); exactly `4` stores `a1` directly
into `self->unk28` (the same field `SceneNode__TryAttachNearby` itself sets on a
successful attach); anything else is a no-op. Reads like a small
command dispatcher over the same "attach a nearby object" state
`SceneNode__TryAttachNearby` manages, with `a2==4` as a direct/forced-attach
shortcut that skips the proximity/bounds checks. Tier B: mechanics
(the three-way dispatch) are certain from the disassembly-verified
`switch`; which caller uses which `a2` value and why is not established
from this function alone. Held back from an actual rename because this
symbol is referenced (in a comment) from `include/Task.h:752` --
a different unit's own header, discussing the same-shaped `switch`
residue as cross-unit precedent. Posted to the broadcast.

## Track 4 (2026-09-25, round 81, charlie)

`(self, s32 a1, s32 a2)` is now `(self, void *sender, s32 event)`. It is
slot +0x09C, which SceneNode__OnNotify calls with its own `(sender, event)`
when the sender's class nibble is 4. Event 4 stores the sender in
`linkTarget` (+0x28, was `unk28`). SceneNode__TryAttachNearby stores its
`other` in the same field and then sends `other` event 4 with itself as the
sender, so the link is recorded on both objects. The int-to-pointer store
warning went away with the retype, and the bytes are identical.

## Round 100 (delta): track 7

No names changed. The case values 2, 3, 4 stay literals: 4 is the answer
TryAttachNearby sends, but 2 and 3 have no established meaning, and naming
one of the three alone would read as if the others were unknown for a
different reason.

### History: the comments in src/code_d294_b.c before this pass, verbatim

```c
/* a2 selects one of three behaviors: 2 or 3 dispatches through the vtable
 * (self->methods->slotA0), exactly 4 stores a1 into self->linkTarget, and
 * anything else (< 2 or > 4) is a no-op. */
```
