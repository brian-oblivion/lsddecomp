# Entity__IsTargetInRange

> Renamed from `func_8005E02C` on 2026-09-23 (tools/rename.py). Address 0x8005e02c.

**Unit:** Entity_b · **Size:** 33 words · **Status:** MATCHED (33/33 words,
whole-image build verified byte-exact). Superseded a prior STALL
classification -- see "Round 8: the stall was a missing argument" below for
what actually closed it and why fourteen prior attempts (two authors) missed
it.

## What it does

`(Entity *this, s32 arg1) -> s32`. Compares `this`'s own Y position
(`this->unk14->y`) against `this->unk94->unk14->y` +/- `0x200`; returns `0` if
outside that band. Otherwise calls `this->methods->slot144(this,
this->unk94)` and returns `1` if the result is `< arg1`, else `0`.

This function is the primary evidence that `Entity::unk94` is a POINTER TYPE
with its own `+0x14` field readable as `EntityPos *` (same convention as
`Entity::unk14` itself) — see `Entity__MoodCue01.md` (matched the round before
this one) for why it is nonetheless NOT another `Entity`, and for the
`Unk94Obj`/`Unk94Methods` struct this function's own evidence fed into.

`EntityMethods::slot144` is `s32 (*)(Entity *self, Unk94Obj *arg1)` —
**two** arguments, not one (see below for how the second was found).

## Final C

```c
s32 Entity__IsTargetInRange(Entity *this, s32 arg1) {
    Unk94Obj *other;
    s32 oy, ty;

    __asm__("");
    other = this->unk94;
    oy = other->unk14->y;
    ty = this->unk14->y;
    if (oy + 0x200 < ty) {
        goto fail;
    }
    if (ty < oy - 0x200) {
        goto fail;
    }
    if (this->methods->slot144(this, other) < arg1) {
        return 1;
    }
fail:
    return 0;
}
```

The `__asm__("")` at the top is a bare scheduling barrier (permitted per
CLAUDE.md rule 6 — verified by removing it: register ALLOCATION is
unchanged, only the load-delay-slot filler changes, which instruction fills
the slot after `this->unk94`'s load). Without it, the compiler fills that
slot with the independent `move $s0, $a1` instead of `nop`, which is valid
scheduling but not retail's instruction ORDER (retail does the `move`
earlier, right after `sw $s0`, with an explicit `nop` filling the delay
slot). This part of the fix was already known from the prior round; see
below for what was NEW.

## Round 8: the stall was a missing argument, not an unreachable register choice

The prior round (two authors, fourteen attempts total, corpus census
included below for the record) filed this as a register-identity stall:
`this->unk94`'s value landed in `$v0` instead of retail's `$a1`, and no
reshape or barrier placement moved it. **That diagnosis was incomplete.**

The actual cause: `slot144` takes a SECOND argument, `this->unk94` itself.
Retail keeps `this->unk94`'s value in `$a1` for its entire lifetime — from
the moment it's loaded (`lw $a1, 0x94($a0)`) all the way to the `slot144`
call at the end of the function — specifically BECAUSE `$a1` is exactly the
register the call needs it in. Retail isn't making an arbitrary allocator
choice; it's avoiding a reload by keeping the value parked in its eventual
argument register the whole time. Passing `other` (the C name for that same
value) as `slot144`'s second argument reproduces this immediately: with the
2-argument prototype, GCC allocates `this->unk94` to `$a1` throughout the
function unprompted — no barrier, no reshape, first try once the signature
was right.

**How the missing argument was found.** While working `Entity__MoodCue12` (a
DIFFERENT, LARGER function in this unit, same round) its own call to the
SAME vtable offset —

```
lw   $v0, 0x0($s0)
lw   $a1, 0x94($s0)     ; this->unk94, loaded with NOTHING between here and the jalr
lw   $v0, 0x144($v0)
jalr $v0
 move $a0, $s0
```

— sets `$a1` explicitly right before the `jalr`, with nothing overwriting it
in between (the very next instruction that touches `$a1` is in the delay
slot of a LATER, unrelated branch, i.e. after the call already returned).
Per this unit's own `docs/DECOMPILATION_LEARNINGS.md` rule ("a value in an
argument register live at the next call IS an argument"), that is direct
proof `slot144` takes a second argument at `Entity__MoodCue12`'s call site.
`Entity__IsTargetInRange`'s OWN call site (same slot, same table) shows a plain `nop`
in the delay slot with no fresh `$a1` load — which looked like a
CONTRADICTION (the same function pointer can't have two arities) until
re-reading `Entity__IsTargetInRange`'s own preceding code: `this->unk94` is loaded
into `$a1` at the TOP of the function and NOTHING overwrites it before the
`slot144` call — so `$a1` already holds the right value, and no fresh load
instruction is needed. The "plain nop" wasn't evidence of a 1-argument call;
it was evidence the argument was already positioned from earlier in the
SAME function. This is the same class of trap
`docs/DECOMPILATION_LEARNINGS.md` already names ("the converse does NOT
hold... check the callee's own body, or ANOTHER caller, for its true
arity"), but a new variant of it: the tell came from realizing why a value
would ALREADY be resident in an argument register across an entire function
body, not just from a second caller's more obvious fresh load.

**Why fourteen prior attempts missed it.** All of them varied the SHAPE of
the `this->unk94->unk14->y` chain (named vs. anonymous temps, barrier
placement, decomposition) while keeping the 1-argument `slot144(this)` call
fixed throughout — because the 1-argument signature had already been
"confirmed" by that call site's own bytes looking unremarkable (a plain
`nop`). None of the attempts cross-checked a DIFFERENT caller of the same
slot. The register-identity test in CLAUDE.md rule 6 ("if reshaping does not
move it, it is a stall") is still correct as written — reshaping genuinely
could not move it, because the reshapes never touched the actual variable
(the missing call argument). The lesson isn't that the rule is wrong; it's
that "reshaping" needs to include the call's OWN argument list, not just the
statements that feed it.

## Superseded material (prior round's stall write-up, kept for the record)

<details>
<summary>Original attempt log and corpus census, now explained by the missing argument above</summary>

Roughly a dozen attempts across two authors, all varying the shape of the
`this->unk94->unk14->y` chain (anonymous temps, named locals with
overlapping live ranges, `__asm__("")` barrier placement) while calling
`slot144(this)` with one argument throughout. Every variant landed on the
identical 5-word residue: `this->unk94`'s value in `$v0`/`$a1` one register
off from retail's `$a1`/`$a2` chain. A corpus census of the shape "an
argument register vacated into a callee-saved register, then reused within 3
instructions as a fresh load's destination" found 6 hits in 4 files
(`Entity_c.s`, `class_3bb8c.s`, `code_2cc8c.s`, and this function), three of
them in still-uncarved segments — noted at the time as a poor permuter
target on rarity grounds. That count was measuring the wrong shape (the
census pattern doesn't describe "value kept live in place because it's a
future call argument"), so it should not be trusted as a lead on remaining
work; it's left here only as a record of what was tried.

</details>

## Proposed learning

**When a register-identity residue survives every reshape of the code that
COMPUTES a value, check whether a DIFFERENT caller of the same vtable slot
passes an extra argument — the residue may be that this function is a
parameter short, and the "unreachable" register is just the value sitting in
its future argument register the whole time.** This sharpens
`docs/DECOMPILATION_LEARNINGS.md`'s existing "value in an argument register
live at the next call IS an argument" rule with the harder case: the
call site under test can look completely unremarkable (a plain `nop` delay
slot, no fresh load) BECAUSE the argument was already resident from earlier
in the SAME function, and only a sibling caller's fresh load exposes the
true arity. Cross-check every caller of a vtable slot before accepting a
register-identity classification, not just the one function currently being
matched — this cost fourteen attempts and one full round before a second,
unrelated function in the same unit surfaced the missing parameter.

## Naming

`Entity__IsTargetInRange` -- tier A (round 71, runner echo, FINISHING-PLAN track 3). Renamed from `func_8005E02C`.

A pure predicate leaf: returns 1 when `this->target`'s y is within +/-0x200 of this entity's y AND slot +0x144 (`distanceToRegion`, occupant `Entity__DistanceToRegion`) from this entity to the target is below `range`, else 0. Callers agree: `Entity__NotifyIfTargetInRange` (range = eventVideo << 9) and `func_80061778` (Entity_d, range 0x800, then faces the target). The parameter `arg1` is renamed `range` at the definition.

Observation for the head, outside this unit: `Entity__DistanceToRegion` (Entity.c) is called with `this->target` as its `EntityRegionRef *` at every call site this pass found (here, Entity__GetProximityRatio, Entity__MoodCue11/12, Entity_c..g). With that reading, `region->slots` at +0x14 is the target's GsCOORDINATE2 (the same +0x14 pointer `EntityPos` models), and `slots[1].x0`/`z0` at +0x38/+0x40 are `workm.t[0]`/`t[2]`. So the function returns |dx| + |dz| between this entity's local x/z and the other object's world x/z, and `flag` (+0xC) is the other object's +0xC word. `Entity__DistanceToRegion` may deserve a sharper name and type. I left it alone because Entity.c owns it.
