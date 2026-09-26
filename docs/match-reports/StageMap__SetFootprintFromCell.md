# StageMap__SetFootprintFromCell — MATCH

> Renamed from `Class866E8__SetFootprintFromCell` on 2026-09-26 (tools/rename.py). Address 0x8004afe0.

> Renamed from `func_8004AFE0` on 2026-09-22 (tools/rename.py). Address 0x8004afe0.

**Unit:** class_3ac78 · **Size:** 20 instructions · **Result:** 20/20 words

## What it does

Sets `self->unk7C`/`unk7E` from two signed bytes read off an opaque
descriptor buffer (`arg1`, offsets 2 and 3, each decremented by one), and
stashes `arg2` verbatim into both `self->unk80` and `self->unk84`. Finishes
with a plain (non-virtual) tail call to `StageMap__BuildFootprintSlots(self)`.

`arg1`'s buffer is populated elsewhere (out of this round's scope, by a
call through `StageMapMethods` slot `+0x110`, itself not decompiled) so
only the two bytes this function actually reads are typed — added as
`UnkArgObj_3ac78` in `include/class_3ac78.h`, following the
`Unk*Obj_<unit>` naming convention already used in `code_171e0.h`.

## Final source

```c
void StageMap__SetFootprintFromCell(StageMap *self, UnkArgObj_3ac78 *arg1, s32 arg2)
{
    s16 t;

    self->unk7C = arg1->unk2 - 1;
    t = arg1->unk3 - 1;
    self->unk80 = arg2;
    self->unk84 = arg2;
    self->unk7E = t;
    StageMap__BuildFootprintSlots(self);
}
```

## Residue and the fix that closed it

First attempt (natural source order — compute+store `unk7C`, store
`unk80`/`unk84`, compute+store `unk7E`, call) reached only 12/20: retail
leaves the FIRST `lbu`'s (arg1->unk2) load-delay slot as a genuine `nop`,
but this body's scheduler eagerly hoisted the very next statement's store
(`self->unk80 = arg2`) into that slot instead — four instructions earlier
than where retail places it (which is the SECOND `lbu`'s delay slot,
`arg1->unk3`).

**Fix: compute `arg1->unk3 - 1` into a named local (`t`) placed
immediately after the `unk7C` store, but defer the ASSIGNMENT to
`self->unk7E` until after both `unk80`/`unk84` stores.** This reproduces
retail's actual instruction order exactly: the byte-3 load happens early
(right after `unk7C`'s store, its own delay slot filled by the `unk80`
store that was sitting ready), and the final `sh` into `unk7E` becomes a
cheap register-only store that the scheduler slides into the `jal`'s own
delay slot at the very end — retail does precisely this (`sh v0,0x7e(a0)`
is the tail call's delay-slot instruction).

Two other orderings were tried and both scored WORSE (11/20 and 12/20 with
different word patterns): plain reordering of the four statements without
splitting the *compute* of `unk7E` from its *store* never reproduced
retail's exact delay-slot assignment, no matter which of the four
statements came first.

### Proposed learning

**When a value is computed early but its store is delay-slot bait for a
LATER instruction (a call, here), split the computation from the store
with a named local, and place the local's assignment to the field at the
point in source order where the STORE (not the load) should land.**
Textual adjacency of two independent statements is not enough to predict
which delay slot the scheduler fills — GCC 2.6.3's scheduler here bypassed
a `nop`-bound load-delay slot four instructions early and instead used the
LAST instruction of the function (a tail call) as a landing spot for a
value computed much earlier. Splitting compute-from-store made that
placement reachable from C.

## Provenance

round 2026-09-02, runner ALPHA, unit class_3ac78.

## Naming

Round 67 (track 3, naming pass).

| symbol | name | tier | evidence |
| --- | --- | --- | --- |
| `func_8004AFE0` | `StageMap__SetFootprintFromCell` | B | Not a vtable slot -- a plain helper, called only from `StageMap__ApplyToSenderFootprint`, on the `config->unk4 == 0` branch. It writes a single cell coordinate (`footprintCol`/`footprintRow`, each a descriptor byte minus one) and a square span (`footprintW` and `footprintH` both set to the same argument), then tail-calls `StageMap__BuildFootprintSlots`, which `class_3bb8c` documents as the function that turns `footprintCol`/`footprintW` into grid rectangles. So: set up a footprint from one cell plus a span. Tier B. |

| field | name | tier | evidence |
| --- | --- | --- | --- |
| `StageMap+0x07C` | `footprintCol` | B | Written here from a descriptor byte; `class_3bb8c`'s `StageMap__BuildFootprintSlots` clamps it into `[0, 0x14)` -- i.e. into `[0, 20)`, the grid's column range -- and uses it with `footprintW` to decide whether the footprint spans one or two cells. |
| `StageMap+0x07E` | `footprintRow` | B | Same treatment, vertical. |
| `StageMap+0x080` | `footprintW` | B | Written here; forwarded by `StageMap__BuildFootprintSlots` as the horizontal extent. |
| `StageMap+0x084` | `footprintH` | B | Same, vertical. Both receive the SAME value from this function, which is why they were previously read as one duplicated field. |

Parameters renamed: `arg1` -> `desc`, `arg2` -> `span`.
