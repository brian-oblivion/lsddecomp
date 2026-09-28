# TodActor__FindPartIndex

> Renamed from `Class65650__FindPartIndex` on 2026-09-26 (tools/rename.py). Address 0x80065d64.

> Renamed from `func_80065D64` on 2026-09-24 (tools/rename.py). Address 0x80065d64.

**Unit:** TodActor · **Size:** 22 words (0x58 bytes) · **Status:** MATCHED
(22/22 words, whole-image `./build-and-verify.sh` green)

## What it does

`TodActorMethods` slot `+0x0FC`. A linear byte search: scans
`self->unk74` (a `u8` array, `self->unk6C` bytes long) for the low byte of
`value`, returning the index of the first match, or `-1` if the array is
NULL, the count is `<= 0`, or nothing matches.

```c
s32 TodActor__FindPartIndex(TodActor *self, s32 value)
{
    u8 *arr;
    s32 count;
    s32 i;
    u8 target;
    u8 unused[8];

    if (self->unk74 == NULL) {
        return -1;
    }
    arr = self->unk74;
    __asm__("");
    count = self->unk6C;
    if (count <= 0) {
        return -1;
    }
    i = 0;
    target = (u8)value;
    do {
        if (*arr == target) {
            return i;
        }
        i++;
        arr++;
    } while (i < count);
    return -1;
}
```

Confirms `self->unk74` is genuinely a `u8 *` (indexed with `lbu`), not the
generic `void *` it was typed as after the first pass — updated in
`src/world/TodActor.c`.

## Two residues, two different fixes

### 1. An 8-byte stack frame with nothing stored in it

Retail's frame is `$sp,8` even though this function makes no calls (no
`$ra` save) and stores nothing to the stack. A straight translation with
only the four meaningful locals (`arr`, `count`, `i`, `target`) compiles
to a **zero-byte** frame — confirmed both in isolation and in the full
build (`build/`'s output showed `size change... shifted linked addresses`,
i.e. the function came out 2 words short). Adding a fifth, genuinely
**unused** local (`u8 unused[8];`) reproduces the 8-byte frame exactly with
`vars=8`.

This is an open question, not a confirmed derivation: nothing in the
disassembly says *what* the 8 bytes were for, only that 8 bytes' worth of
some local declaration existed and never got the compiler to load or store
through it. Whoever revisits struct/field naming for this function's
neighbours should keep this in mind — it may turn out to be a real,
differently-shaped local (e.g. two `s32`s) once more of this class is
understood, not literally a `u8[8]`.

### 2. Two independent instructions, scheduled in a different order

With the frame fixed, the function was **18/22** — four words differing,
and per the head's diagnostic broadcast (round 2026-08-30-a, second pass),
the first thing to check was whether this was a *branch target* difference
(a different CFG, meaning the source is wrong) or a genuine scheduling-only
difference. Lining the words up:

```
retail:  addu $a2,$v0,$zero  ;  lw $a0,0x6C($a0)  ; nop ; blez $a0,FAIL(offset 0xA)
mine:    lw $a0,0x6C($a0)    ;  nop               ; blez $a0,FAIL(offset 0xB) ; addu $a2,$v0,$zero (delay slot)
```

The `blez` targets the **same** `FAIL` label in both — the encoded offset
differs only because the branch's own position shifted by one word when
the scheduler moved `addu $a2,$v0,$zero` into its delay slot instead of
placing it earlier. Same CFG, confirmed before treating this as residue
rather than a wrong source shape.

`arr = self->unk74;` (independent of the count load: only needs `$v0`,
already available) and `count = self->unk6C;` (independent of the copy:
only needs `$a0`) have no data dependency on each other, so GCC 2.6.3's
list scheduler is free to interleave them either way, and it consistently
chose to defer the copy into the `blez` delay slot regardless of which
order the two statements were written in the source (six variations tried:
reordering the two assignments, moving the null check before/after the
assignment, initializing `i` at declaration vs. separately — all produced
the *same* wrong schedule).

A bare `__asm__("")` between `arr = self->unk74;` and `count = self->unk6C;`
closes it: **22/22, image green.** Verified this is the permitted form under
CLAUDE.md rule 6 — removing the barrier reproduces the exact same register
allocation (`$v0`/`$a2` for the array pointer, `$a0` for count, `$a1` for
the target byte, `$v1` for the index) with only the four words' *order*
different, never which register holds which value.

### Proposed learning

Extends the project's existing "instruction order only" residue category
with a concrete recipe: when two adjacent, data-independent statements
scheduled correctly everywhere else land in the wrong relative order (one
gets pulled into a nearby branch's delay slot instead of running as a
free-standing instruction before it), a bare `__asm__("")` placed between
them in source order is the lever — cheaper to reach for than six
permutations of statement order, which this case tried first and which
uniformly failed to move the schedule.

Also: **before concluding a residue is scheduler-internal, line up the
actual branch targets, not just the delay-slot contents** — a differing
encoded branch offset can be nothing more than the branch's own position
shifting by the same reorder that scrambled its delay slot, if it still
targets the identical label. Confirming that first is what justified
reaching for `__asm__("")` here instead of re-deriving the control flow.

## Naming

Round 75 (charlie), track 3.

- `TodActor__FindPartIndex` (was `func_80065D64`), tier A. Occupies +0x0FC. Linear search of partIds (one byte per part, partCount entries) for the low byte of its argument; returns the index or -1. ApplyTodPacket uses it to map a TOD packet's object id to a part.

## Track 4 (2026-09-25, round 85, alpha)

The class (id 0x234, table `gTodActorMethods`) is unified as `TodActor` in `include/TodActor.h`: an Actor subclass (its ctor chains to Actor's first) and Entity's base. Any source block above is the pre-unification spelling (the local `TodActorMethods` of `include/code_55dd4.h`, `linkCompanion`/`unlinkCompanion`, `companion2`, `Unk5CObj`/`Unk70ElemObj`); the live body in `src/code_55dd4.c` takes the unified types and the inherited slot and field names (`addChild`/`removeChild`, Actor's `ticker`, `Actor *` parts, `ModelData *` modelData), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## asm sites

Round 89 (runner delta, track 5 `asm-sites`): the bare `__asm__("")` after
`arr = self->partIds;` is **justified** and now commented at the site.
Measured by deleting it alone: the image went red (14 bytes, same length),
`funcdiff` 18/22, and asm-differ shows `move a2,v0` (the copy of `arr`) moved
from before `lw a0,0x6c(a0)` (the `self->partCount` load) into the delay slot
of `blez a0` (the `count <= 0` test). Instruction order.

## Track 7 (round 99, bravo): readable spelling, byte-identical

`arr`/`value`/`target` are `ids`/`id`/`wanted`. `unused[8]` and the
`__asm__("")` each carry a one-line `MATCHING:` comment; the measurements are
"Two residues, two different fixes" and "asm sites" above.
