# Class65650__SetupModelData

> Renamed from `func_80065BFC` on 2026-09-24 (tools/rename.py). Address 0x80065bfc.

**Unit:** code_55dd4 · **Size:** 12 words (0x30 bytes) · **Status:** MATCHED
(12/12 words, whole-image `./build-and-verify.sh` green)

## What it does

`Class65650Methods` slot `+0x0F4` (`slot_setup5C`, already typed in
`include/code_55dd4.h` from the constructor's call site). Guards
`self->unk5C`: if it is already set, returns 0 (success, nothing to do);
otherwise defers to `Class65650__AcquireModelData(self, arg1)` and returns its result
directly. `arg1` is never touched in this function's own body — retail
leaves `$a1` untouched from entry and lets `Class65650__AcquireModelData`'s call inherit it,
which is why the signature carries a second parameter purely to forward it.

```c
s32 Class65650__SetupModelData(Class65650 *self, void *arg1)
{
    if (self->unk5C != NULL) {
        return 0;
    }
    return Class65650__AcquireModelData(self, arg1);
}
```

Matched with the plain, non-`goto` translation above — no reshaping needed.

## On the broadcast's `goto` lever (`New_Pad.md`)

This function has the *same shape* the broadcast flagged as needing `goto`:
a guard that early-returns one constant, falling through to a `return` of a
different value on the main path. Here the plain `if (...) return 0; return
callee(...);` matched **first try, zero residue** — no `goto` required.

The likely discriminator: in `New_Pad`'s failing case, the main path's
return value is a *separately named local* (`self`, computed earlier and
reused) — GCC has to materialize it into `$v0` right before the shared
epilogue, and that materialization is what needs the `goto`-shaped CFG to
land in the delay slot correctly. Here, the main path's return value **is
the callee's own `$v0`** — nothing to materialize, so there is no
instruction whose placement depends on which CFG shape you spell in C.

### Proposed learning

Refines `New_Pad.md`'s finding rather than contradicting it: the
`goto`-vs-`return` sensitivity for a two-arm shared-epilogue early exit
shows up when the **fall-through arm's return value is a stored local**
that must be copied into `$v0` right before the epilogue. When the
fall-through arm's return value **is already `$v0`** (a bare `return
callee(...);` with no intervening use of the result), plain `if (...)
return CONST; return callee(...);` matches directly — there is nothing left
for `goto` to reposition. Check which shape you have before reaching for
`goto`.

## Naming

Round 75 (charlie), track 3.

- `Class65650__SetupModelData` (was `func_80065BFC`), tier A. Occupies +0x0F4 (called by the ctor as setupModelData(self, arg1)). Returns 0 if modelData is already set, otherwise AcquireModelData.

## Track 4 (2026-09-25, round 85, alpha)

The class (id 0x234, table `gClass65650Methods`) is unified as `Class65650` in `include/Class65650.h`: an Actor subclass (its ctor chains to Actor's first) and Entity's base. Any source block above is the pre-unification spelling (the local `Class65650Methods` of `include/code_55dd4.h`, `linkCompanion`/`unlinkCompanion`, `companion2`, `Unk5CObj`/`Unk70ElemObj`); the live body in `src/code_55dd4.c` takes the unified types and the inherited slot and field names (`addChild`/`removeChild`, Actor's `ticker`, `Actor *` parts, `ModelData *` modelData), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
