# ReleaseDreamAuxEntities -- MATCHED

> Renamed from `TickDreamAuxSlots2` on 2026-09-27 (tools/rename.py). Address 0x8005c76c.

> Renamed from `func_8005C76C` on 2026-09-21 (tools/rename.py). Address 0x8005c76c.

Unit `DreamAux` (was `code_4cd08`). 26/26 words, `0x4CF6C`-`0x4CFD4`. Whole-image
`build-and-verify.sh` green. Same shape as `ReleaseDreamAuxModels`, over `gDreamAuxSlots2`
instead of `gDreamAuxSlots` (see that report for the object/vtable/loop-shape
derivation -- not repeated here).

```c
void ReleaseDreamAuxEntities(void)
{
    u32 done;
    DreamAuxSlot *slot;

    done = 0;
    slot = gDreamAuxSlots2;

    for (; done < 1; done++) {
        DreamAuxObj *obj = slot->obj;

        if (obj != NULL) {
            DreamAuxTickFn tick = (DreamAuxTickFn)obj->vtable[1];
            slot->obj = tick(obj);
        }
        slot++;
    }
}
```

## Residue: prologue spill/init INTERLEAVING, not just store order

The first, direct port of `ReleaseDreamAuxModels`'s shape (`DreamAuxSlot *slot =
gDreamAuxSlots2; u32 done;`, initializers at declaration) built clean but only
21/26 -- the two callee-save spills and their register inits were emitted in
the wrong relative order:

```
retail                              built (direct port)
sw   s1, 0x14(sp)                   sw   s0, 0x10(sp)
move s1, zero                       lui  s0, ...            ; slot's addr
sw   s0, 0x10(sp)                   addiu s0, s0, ...
lui  s0, ...                        sw   s1, 0x14(sp)
addiu s0, s0, ...                   move s1, zero
sw   ra, 0x18(sp)                   sw   ra, 0x18(sp)
```

Retail interleaves per-variable (spill `s1`, init `s1`, spill `s0`, init
`s0`), in the OPPOSITE variable order from `ReleaseDreamAuxModels`'s retail (which is
slot/`s0` first, done/`s1` second) even though the two functions are
otherwise structurally identical. Per the head's broadcast on
`Pad__DispatchEvents` (Lever 1: prologue callee-save store order is not reachable
from C; use a bare `__asm__("")` as the first statement), I checked whether
this is the same class of residue before reshaping further:

- Swapping the *declaration* order alone (`u32 done;` before
  `DreamAuxSlot *slot = gDreamAuxSlots2;`, both still initialized in their
  declarators) made **no difference** -- still 21/26, byte-identical to the
  unswapped version. Confirms declaration order alone does not reach
  whatever pass decides this, same finding as the broadcast.
- A bare `__asm__("");` as the first statement (with declaration order both
  ways) made it **worse** (20/26) -- it moved the `s1` spill+init to a
  different wrong position, not retail's. So Lever 1's specific fix does not
  generalize to this exact shape; this is prologue-adjacent but not the same
  residue.
- What worked: splitting the declarations from their initialization into
  separate STATEMENTS, in the order retail wants (`done = 0;` before
  `slot = gDreamAuxSlots2;`), with the declarations themselves left in either
  order. This is a genuinely different C shape from an initializer at the
  declarator (not just cosmetically -- GCC 2.6.3 apparently schedules
  spill/init pairs for straight assignment statements as a unit, in
  STATEMENT order, whereas declarator initializers get scheduled by some
  other heuristic that produced `ReleaseDreamAuxModels`'s order regardless of which
  declarator came first).

## Proposed learning

- **When two structurally-identical functions in the same binary want
  OPPOSITE prologue spill/init orders for the same pair of registers, check
  whether the source used declarator initializers (`T x = expr;`) vs.
  separate assignment statements (`T x; ...; x = expr;`) before reaching for
  a scheduling barrier.** Here, declarator-initializer order was NOT
  reachable from C (matches the `Pad__DispatchEvents` broadcast) but converting to
  explicit assignment STATEMENTS, in the desired order, was -- and needed no
  barrier at all. This is a cheaper, more targeted lever than
  `__asm__("")` for this specific residue shape (adjacent prologue
  spill+init pairs out of order) and should be tried first.

## Naming

**ReleaseDreamAuxEntities** — tier A. Identical mechanics to `ReleaseDreamAuxModels`,
over `gDreamAuxSlots2` instead of `gDreamAuxSlots` (see that report/entry for
the shared derivation). Tier A for the same reason: the tick pass over the
slot family IS the function's purpose. Called from `ObjM__TeardownStyle`
(`ObjMStyleActor.c`) alongside other per-frame-looking calls, consistent with
"tick", though nothing in this unit distinguishes what makes the "2" family
different in KIND from the first (it is never populated by any function in
this unit's own queue).

## Round 100 (alpha): track 7, moved from src/world/DreamAux.c and include/DreamAux.h

## Naming (round 100)

**ReleaseDreamAuxEntities** (was TickDreamAuxSlots2) -- tier A.
gDreamAuxSlots2 (0x80088D2C) is gDreamAuxSlots one word in, so the word each
element's first field reads is the slot's `entity`, the Entity
SetDreamAuxWorld made; slot +0x004 is `release`. Its only caller is
ObjM__TeardownStyle (onDeinit, src/world/ObjMStyleActor.c), mirroring ObjM's scene
setup calling SetDreamAuxWorld. The body reads it as `(Entity *)slot->model`
with a comment. `done` -> `i`; the split `i = 0; slot = ...;` keeps a
one-line MATCHING comment (the derivation is above). Byte-identical.
