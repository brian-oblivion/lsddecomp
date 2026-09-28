# ItemList__AttachTarget -- MATCHED (27/27 words)

> Renamed from `Class86F88__AttachTarget` on 2026-09-26 (tools/rename.py). Address 0x80052110.

> Renamed from `ItemList__AddChildAndSetState` on 2026-09-26 (tools/rename.py). Address 0x80052110.

> Renamed from `ItemList_3bb8c_j__AddChildAndSetState` on 2026-09-24 (tools/rename.py). Address 0x80052110.

> Renamed from `func_80052110` on 2026-09-24 (tools/rename.py). Address 0x80052110.

Unit: `src/ui/input_dialogs.c`. `self` is `ItemList_3bb8c_j`.

## Body

```c
void ItemList__AttachTarget(ItemList_3bb8c_j *self, void *arg1, s32 arg2, s32 arg3)
{
    typedef void (*Slot10NarrowFn)(ItemList_3bb8c_j *self, s32 arg1);
    void (*fn)(ItemList_3bb8c_j *self, void *arg1, s32 arg2, s32 arg3);
    s32 zero;

    zero = 0;
    fn = self->methods->slot10;
    do {
        fn(self, arg1, arg2, arg3);
        ((Slot10NarrowFn)self->methods->slot10)(self, arg2);
        self->unk3C = arg3;
        self->unk2C = zero;
    } while (0);
}
```

Calls `ItemListMethods_3bb8c_j::slot10` (established this round) TWICE at
different arities from the SAME function -- the first call is a straight
passthrough of this function's own `(self, arg1, arg2, arg3)` (nothing
overwrites `$a1`-`$a3` between function entry and the first `jalr`), the
second passes only `(self, arg2)` (confirmed by the disassembly: no `$a2`/
`$a3` setup before the second `jalr`, matching DECOMPILATION_LEARNINGS'
"per-call-site arity" convention). The narrower 2-arg call needs an
explicit function-pointer cast (`Slot10NarrowFn`) since C cannot call a
4-parameter function pointer with 2 arguments.

Also establishes `ItemList_3bb8c_j::unk2C` (cleared to 0) and `unk3C` (set to
`arg3`).

## Residue and how it closed

Straightforward direct translation (`self->methods->slot10(self, arg1,
arg2, arg3); ((Slot10NarrowFn)self->methods->slot10)(self, arg2);
self->unk3C = arg3; self->unk2C = 0;`) scored 3/27 with a ONE-WORD
size overshoot: retail schedules the preservation of `arg2` (into what
becomes a callee-saved register) into the LOAD-DELAY SLOT between
`self->methods` and `self->methods->slot10`'s own load (the two `lw`s of
the first call), for free; my direct form left that delay slot a plain
`nop` and instead materialized the same preservation as an extra,
separate instruction right at function entry.

Ran the permuter (`tools/setup-permuter.sh`, found in ~5000 iterations).
The zero-score candidate's structural difference from my code: caching
`self->methods->slot10` into a local BEFORE the first call, caching the
literal `0` into a local BEFORE either call, and wrapping the whole body
in a `do { ... } while (0)`. Translating the cache-into-locals part alone
did NOT reproduce the zero; only adding the `do { } while (0)` wrapper
around the body closed it. Removing the wrapper (keeping everything else
identical) reproduces the original 3/27 score exactly -- confirmed by
direct A/B test.

### Proposed learning

**A `do { ... } while (0)` wrapping an otherwise-unconditional function
body can be load-bearing for GCC 2.6.3's instruction scheduling, not just
a stylistic/macro-hygiene artifact.** Here it was the deciding factor
between a delay-slot-filling `move` (retail's shape) and a wasted `nop`
plus a separate early `move` (one word longer) -- with IDENTICAL
statement content inside vs. outside the wrapper. Suspect this lever when
a residue is a single extra/missing instruction tied to delay-slot
filling around a `self->methods->slotN` call and reshaping the statements
alone doesn't move it. Not yet understood WHY the wrapper changes
scheduling (a basic-block boundary artifact of `-O2` in this compiler is
the working guess); flagging as a confirmed-but-unexplained mechanism for
whoever investigates next.

## Naming

- `ItemList__AttachTarget` -- tier B. The slot4C occupant (classtable.py gItemListMethods +0x04C): dispatches self->methods->slot10 (this class's own addChild override) twice, once at full arity and once narrowed to (self, arg2) via a local function-pointer typedef, then sets unk3C=arg3 and clears unk2C. Mechanics established across rounds 18-19's residue hunt; the double-arity dispatch's in-game reason is not.

## Track 4 (2026-09-26, round 89)

Renamed from `ItemList__AddChildAndSetState`. The body is
TextEntry__AttachTarget's, at the same slot (+0x04C) of a sibling class with
the same one caller: addChild(child1), addChild(child2), `target = arg3`,
`result = 0` (TextEntry also clears altCommands). Its only caller,
TaskObjF__AttachItemList (title_menu), passes `(unk60, unk64, childC)`
exactly as TaskObjF__AttachTextEntry passes them to TextEntry's attachTarget, and
`target` is what ForwardToTarget calls. Nothing in it sets a state; the
cleared word is `result`.

## Round 94 (track 6, charlie): the target is a VabStreamObj

`TargetObj86ED0`/`TargetMethods86ED0` (include/class_3bb8c.h) are deleted.
Both attachTarget callers (TaskObjF, src/ui/title_menu.c) pass TaskObjF's
`sound`, already typed `struct VabStreamObj *`, and the one slot the view
named, +0x080, is VabStreamObj's `playTone(self, index, vol, endVol)`
(include/vab_stream_obj.h): the `(code, 0x60, 0x60)` call plays tone `code`
at volume 0x60. TextEntry::target and ItemList::target (+0x03C) and both
attachTarget prototypes are `struct VabStreamObj *`, the casts at the call
sites are gone, and `slot80` is `playTone`. Zero bytes changed.

## History (moved from src/class_3bb8c_j.c, round 100)

The comment above this function read: "The first addChild passes all four
words through (ItemListAddChildWideFn, no code): see this function's report
for the do/while." It now says what the function does, and the do/while (0)
with the cached slot and zero keep a MATCHING line.

## Track 7 (round 100, charlie)

Parameters `child1`/`child2` renamed `inputSource`/`tickSource` in the
definition (the header prototype keeps child1/child2: see the unit's
proposals), local `fn` renamed `addChildWide`. Zero bytes changed.
