# New_TodActor

> Renamed from `New_Class65650` on 2026-09-26 (tools/rename.py). Address 0x800655d4.

> Renamed from `New_class_65650` on 2026-09-24 (tools/rename.py). Address 0x800655d4.

**Unit:** code_55dd4 · **Size:** 31 words (0x7C bytes) · **Status:** MATCHED
(31/31 words, whole-image `./build-and-verify.sh` green)

## What it does

The `New_X` allocator for the class whose method table is `gTodActorMethods`
(resolved with `tools/classtable.py gTodActorMethods --vs 0x800878D4`; see
`include/code_55dd4.h` for the full inheritance chain). Allocates a 0x98-byte
instance through the game allocator `BMemPMgrAlloc`, fetches the class's own
vtable via `GetTodActorMethods()` (a `Get_vtable`-style accessor, matched
separately, see its own report), and calls the constructor slot (`+0x008`)
with `(self, arg1, arg2)`. Unlike the simpler `New_X` shapes documented
elsewhere in this project (`New_Pad`, `New_GameApplication`), **this one
also checks the constructor's own return value**: on constructor failure it
frees the object (`BMemPMgrFree`) and returns `NULL` instead of leaving it
allocated.

```c
void *New_TodActor(void *arg1, void *arg2)
{
    TodActor *self;
    TodActorMethods *vt;

    self = (TodActor *)BMemPMgrAlloc(0x98);
    if (self == NULL) {
        return NULL;
    }
    vt = GetTodActorMethods();
    if (vt->ctor(self, arg1, arg2) != NULL) {
        return self;
    }
    BMemPMgrFree(self);
    return NULL;
}
```

## A third `New_X` sub-shape, and the `goto` lever did not apply here

The head's broadcast (round 2026-08-30-a, `New_Pad`) established that for
a two-way early exit sharing one epilogue with *different* return values,
`goto fail; ...; fail: return OTHER;` is needed to match byte-for-byte — a
plain `if (...) return OTHER;` costs an extra `j`+`nop`.

This function is a **third sub-shape**, distinct from both `New_Pad`
(ignores the constructor's return, one early exit) and `New_GameApplication`
(returns the allocation unconditionally, no early exit at all): it has **two**
early-exit-shaped branches (the allocation null check, and the constructor
failure check) plus a genuine three-way return (`NULL` on alloc failure,
`self` on success, `NULL` again — via a different path, after freeing — on
constructor failure).

Written with the straightforward, non-`goto` C above, it matched **31/31 on
the first successful build**, with no residue at all. So for this sub-shape,
**the `goto` lever was not needed.** A plausible reason: retail's own control
flow already converges through a *shared* epilogue block reached from three
places (not two), and with that many convergent paths GCC 2.6.3's block
layout puts the plain `if`/`return` bodies exactly where retail has them
without requiring the delay-slot trick that a strict two-way branch needs.
This is not a full explanation, just the empirical result — worth recording
so the next runner does not spend attempts reflexively applying the `goto`
rewrite to every `New_X` residue.

## Notes on the header

`include/code_55dd4.h` types `BMemPMgrAlloc` as taking a single `s32 size`
parameter (matching the correction the head made to `Pad.h`'s
`BMemPMgrAlloc` prototype in the same round) — the call site here sets only
`$a0` before `jal`.

### Proposed learning

Not every `New_X` allocator is the same sub-shape. At least three exist in
this codebase: (1) ignore the constructor's return, single early exit
(`New_Pad`, needs `goto`); (2) unconditional return, no early exit
(`New_GameApplication`, open stall); (3) check *both* the allocation and the
constructor's return, two early exits converging on one epilogue
(`New_TodActor`, matched with plain `if`/`return` — no `goto` needed).
Identify which shape a given `New_X` is (does the retail asm test the
constructor's own `$v0` after the `jalr`?) before assuming a `goto` rewrite is
required.

## Naming

Round 75 (charlie), track 3.

- `New_TodActor` (was `New_class_65650`), tier A. Allocates 0x98 bytes with BMemPMgrAlloc, runs `GetTodActorMethods()->ctor`, frees on failure: the project's `New_Class` allocator shape. Replaces FirecatFG's `New_class_65650` (same meaning, convention spelling).

## Track 4 (2026-09-25, round 85, alpha)

The class (id 0x234, table `gTodActorMethods`) is unified as `TodActor` in `include/TodActor.h`: an Actor subclass (its ctor chains to Actor's first) and Entity's base. Any source block above is the pre-unification spelling (the local `TodActorMethods` of `include/code_55dd4.h`, `linkCompanion`/`unlinkCompanion`, `companion2`, `Unk5CObj`/`Unk70ElemObj`); the live body in `src/code_55dd4.c` takes the unified types and the inherited slot and field names (`addChild`/`removeChild`, Actor's `ticker`, `Actor *` parts, `ModelData *` modelData), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 7 (round 99, bravo)

Parameters `desc`, `sound` (TodActor.h's banner: the descriptor and the
VabStreamObj); the allocation is `sizeof(TodActor)` (0x98). Byte-identical.
