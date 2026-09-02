# func_8004ABD0 — STALL

**Unit:** class_3ac78 · **Size:** 74 instructions · **Best reached:** 9/74
words, correct size, no address drift

## What it does

Walks a 7-element array of 0x1C-byte "slot" records starting at
`self+0xEC` (new struct knowledge: `Class866E8::unkEC[7]`, typed
`UnkSlotEntry_3ac78`). For each slot entry: calls the "child" object's
(`entry->unk4`) own slot `+0x74`; zeroes the entry's `unk0` (u16); calls
`self->methods->slot108(self, entry)`; if the "list" object's
(`entry->unk8`) field `+0x2C` is non-NULL, refreshes it through its own
base-class `unk04` slot (same `GenericObject`/`unk04` pattern already
established by `func_8004AA6C`); calls `self->methods->slot88(self, 6,
entry, i)` (dispatches to `func_8004AA6C`, which per its own already-
matched signature only reads 2 of these 4 args — the extra ones are dead
at the callee, consistent with the "argument register carries no meaning
if unused by callee" case in DECOMPILATION_LEARNINGS); calls
`entry->unk4->methods->slot84(entry->unk4)` again. After the loop:
zeroes `self->unk1B8`/`unk1B4` and calls `self->methods->slot140(self)`.

New struct/vtable knowledge added regardless of the stall (all verified
straight from the disassembly): `Class866E8Methods::slot88` (declared with
a 4th `s32 arg3` parameter that the occupant, `func_8004AA6C`, doesn't
read — same "field type need not match every occupant's real signature"
precedent as `func_8004A984`/`func_8004AA6C`'s `func_8001E57C`), `slot108`
(`func_8004C0AC`, not decompiled), `slot140` (`func_8004D088`, not
decompiled — this promotes what was previously just end-of-struct
padding into a real slot), `Class866E8::unkEC[7]` (`UnkSlotEntry_3ac78`,
0x1C bytes each), `unk1B4`/`unk1B8`, and three new opaque types
(`UnkSlotEntry_3ac78`, `UnkSlotChildObj_3ac78`/`Methods`,
`UnkSlotListObj_3ac78`).

## Best-reached body (does NOT compile to retail bytes)

```c
#if 0
void func_8004ABD0(Class866E8 *self)
{
    s32 i;
    s32 offset;
    UnkSlotEntry_3ac78 *entry;

    offset = 0xEC;
    for (i = 0; i < 7; i++) {
        GenericObject *check;

        entry = (UnkSlotEntry_3ac78 *)((u8 *)self + offset);
        offset += 0x1C;
        entry->unk4->methods->slot74(entry->unk4);
        entry->unk0 = 0;
        self->methods->slot108(self, entry);
        check = entry->unk8->unk2C;
        if (check != NULL) {
            entry->unk8->unk2C = check->methods->unk04(check);
        }
        self->methods->slot88(self, 6, entry, i);
        entry->unk4->methods->slot84(entry->unk4);
    }

    self->unk1B8 = 0;
    self->unk1B4 = 0;
    self->methods->slot140(self);
}
#endif
```

## The residue: the loop-offset increment keeps landing in the WRONG
delay slot

Retail recomputes the slot pointer fresh each iteration from two
registers added together (`self` + a running byte offset, `addu
$s0,$s2,$s4`) rather than carrying an incrementing pointer across
iterations, and the offset's own increment (`addiu $s4,$s4,0x1C`) sits in
the delay slot of the `beqz $a0,...` branch that guards the `unk2C`
refresh — i.e. textually in the MIDDLE of the loop body, right after
reading `entry->unk8->unk2C`.

**Every reconstruction tried puts the increment in the FIRST available
call's delay slot (`slot74`'s `jalr`) instead, regardless of where the
increment statement sits in the C source:**

1. `entry = &self->unkEC[i];` (array indexing, fresh index each
   iteration): 8/74. Compiler additionally split the walk into TWO
   separate persisted induction-variable registers (one for
   `entry`/`unk0`, one specifically for `&entry->unk4`), which retail does
   not do — retail re-reads `entry->unk4` twice through the SAME pointer.
2. `entry++` (persisted incrementing pointer, C89-idiomatic "increment
   pointer" per the project's own `func_80066340` lesson): 8/74. Same
   general shape as (1) but with a single induction variable; still not
   retail's "fresh add each iteration" shape.
3. `offset` accumulator (plain `s32`), `entry` recomputed from
   `self+offset` each iteration, increment written at the END of the loop
   body: regressed HARD — 0/74 with a 196KB whole-image drift. GCC
   strength-reduced this into the exact same single-incrementing-pointer
   shape as (2), i.e. the accumulator didn't survive as a literal
   register-pair recomputation.
4. Same accumulator, increment moved to right after reading `check =
   entry->unk8->unk2C;` (retail's approximate textual position): 9/74, no
   drift. This is the first attempt where `addu $s0,$s1,$s3` (recompute,
   not increment-in-place) appears at the top of the loop, matching
   retail's overall SHAPE — but the increment itself still schedules into
   `slot74`'s `jalr` delay slot, not the `beqz`'s.
5. Same, with a bare `__asm__("");` scheduling barrier inserted between
   the `slot74` call and `entry->unk0 = 0;` (attempting to block the
   later-appearing increment from floating backward across it): no
   change — 9/74, identical instruction layout. The barrier did not
   prevent the delay-slot fill from reaching backward past it, which is
   itself worth recording (see below).
6. Same, increment moved to immediately after computing `entry` (the very
   TOP of the loop, before `slot74` at all): 9/74, unchanged — the
   increment still ends up in `slot74`'s delay slot either way, meaning
   its FINAL position is apparently independent of where between "entry
   computed" and "check read" it's written in source.

**None of the four listed word-counts should be read as size-verified
except (4) and (6)** — (1)/(2) showed no `WARNING: differs OUTSIDE this
range` at 8/74 either, so those are also drift-safe; only (3) drifted.

### What's actually going on (best guess, unconfirmed)

The delay-slot filler for `slot74`'s `jalr` is choosing the offset
increment because it is the FIRST independent, side-effect-only
instruction reachable in the block, and GCC 2.6.3's `dbr_schedule` pass
here appears to search past several intervening independent instructions
(not just the immediately-next one — contrast with `func_8004AFE0`'s
residue, which stayed local) to find and hoist it, regardless of textual
distance from where the C source puts it. A bare `__asm__("")` scheduling
barrier did NOT block this hoist, which contradicts the working
assumption from `func_80065E1C`/`func_80065AE0` that the barrier is a
reliable local lever — worth flagging for whoever revisits the barrier's
actual scope in this compiler. The remaining structural difference (two
separate registers recomputed by addition vs. one incrementing pointer)
was successfully reproduced (attempt 4/6); only the SCHEDULING of the one
increment instruction resists.

### Proposed learning

**A bare `__asm__("")` barrier does not reliably block a later
instruction from being hoisted BACKWARD into an earlier delay slot** —
at least not past multiple intervening independent statements. This
narrows the barrier's documented scope (CLAUDE.md's "if removing it
changes ordering only, it's allowed" test already implies it's a
reordering tool, not an ordering GUARANTEE) and should be checked before
reaching for it as a fix for a "value hoisted too early" residue class,
which up to this point had only been solved by splitting COMPUTE from
STORE (`func_8004AFE0`), not by barriers.

## Provenance

round 2026-09-02 (head-requested extension), runner ALPHA, unit
class_3ac78. Six attempts across two structural strategies (array
indexing / incrementing pointer / offset accumulator), best 9/74 with the
loop's overall two-register shape reproduced but one instruction's
delay-slot placement unmoved. Moved on to stay within budget for the
remaining assigned functions. Restored to `INCLUDE_ASM`.
