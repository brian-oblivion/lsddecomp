# FileResource__FreeBuffer

> Renamed from `Class6D430__FreeBuffer` on 2026-09-26 (tools/rename.py). Address 0x80026c20.

> Renamed from `func_80026C20` on 2026-09-18 (tools/rename.py). Address 0x80026c20.

**Unit:** GameApplicationFileResource · **Size:** 24 instructions · **Status:** MATCHED (24/24 words, whole-image build verified byte-exact)

## What it does

`gFileResourceMethods`'s vtable slot `+0x05C` (also reachable indirectly through
`FileResource__Finalize`, the class's own dtor). Frees `this->unk10` via
`BMemPMgrFree` and clears it, but only when three conditions all hold:
the pointer is non-NULL, `this->unk14` (its recorded size) is non-zero, and
`this->unk20` (a flag cleared in the constructor) is zero.

## Derivation

```
lw    $a0, 0x10($s0)
beqz  $a0, END                  ; if (this->unk10 == NULL) return;
lw    $v0, 0x14($s0)
beqz  $v0, END                  ; if (this->unk14 == 0) return;
lhu   $v0, 0x20($s0)
bnez  $v0, END                  ; if (this->unk20 != 0) return;
jal   BMemPMgrFree             ; BMemPMgrFree(this->unk10) -- a0 unchanged since first load
sw    $zero, 0x10($s0)          ; this->unk10 = NULL
END: epilogue
```

`$a0` is loaded once (`this->unk10`) and never reloaded before the call —
GCC kept it live across the two intervening unrelated loads/branches rather
than re-fetching it, which is exactly what plain sequential C produces here
(nothing writes `this->unk10` between the first read and the call).

## Final C

```c
void FileResource__FreeBuffer(FileResource *this) {
    if (this->unk10 == NULL) {
        return;
    }
    if (this->unk14 == 0) {
        return;
    }
    if (this->unk20 != 0) {
        return;
    }
    BMemPMgrFree(this->unk10);
    this->unk10 = NULL;
}
```

## Attempt log

Matched on the first real attempt (once written against the corrected
`FileResource` struct). An earlier diff run against this function showed
0/24 and a pure 1-word shift for its entire body — that was **not** a bug in
this function; it was downstream drift from `FileResource__LoadFile`'s wrong-sized
allocator call (see that report) shifting every address after it in the
unit. Re-diffed clean after the sibling fix, with zero changes to this
function's own source.

## Head broadcast levers — applicability

- **goto-vs-return:** not applicable — all three early exits are bare
  `return;` with no value (void function), so there is no return-value
  divergence for the lever to act on. Plain `if (cond) { return; }` sequences
  matched with no residue.
- **loop-invariant hoisting:** not applicable, no loop.
- **prologue store-order barrier:** not applicable, no store-order residue.

## Proposed learning

Reinforces `FileResource__FileResource.md`'s note: when a function's `funcdiff` shows a
uniform shift (every word wrong, but the SAME word appearing one slot over)
with zero words matching, check sibling functions in the same translation
unit for a genuine size bug before touching this function's own source at
all — this one needed no changes once the real culprit was fixed.

## Naming

Round 52 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_80026C20` | `FileResource__FreeBuffer` | B |

**Evidence.** `+0x05C` slot: frees `this->unk10` via `BMemPMgrFree` and
clears it, guarded by three conditions (non-NULL, sized, not busy per
`unk20`). Also reachable indirectly through `FileResource__Finalize`. Mirrors
`FileResource__LoadFile`'s naming; mechanics known, why the buffer needs
this specific guard is not.

## Proposed field names

**APPLIED by the head at merge, round 52** -- all four fields, both types
and all five vtable slots below are now in the tree, each one applied
separately with `./build-and-verify.sh` green and byte-exact after it. One
mis-hit had to be resolved by receiver type: `src/cd/CdDriver.c:143`
accesses `pendingGeneration` on a `FileResource *self`, while the same file's
lines 97/174/175/182 are its OWN `ObjA34_179D8H::unk0C` and were left alone.
The compiler named that mis-hit (`structure has no member named 'unk0C'`),
which is the procedure working in the direction where it can work.

Same cross-unit exposure as `FileResource__LoadFile.md` (`FileResource`
is shared with `code_179d8_h.c`/`CdDriver.c`), so PROPOSED, not renamed:

| field | proposed name | tier | evidence |
| --- | --- | --- | --- |
| `unk20` | `freeGuard` | B | Nonzero blocks the `unk10` free in this function; zeroed in the constructor. Mechanics (a guard flag) are clear; what sets it nonzero was not found in this unit -- likely a subclass concern. |

Posted to the broadcast.
