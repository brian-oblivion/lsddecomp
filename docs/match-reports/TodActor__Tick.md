# TodActor__Tick

> Renamed from `Class65650__Tick` on 2026-09-26 (tools/rename.py). Address 0x80065fd8.

> Renamed from `func_80065FD8` on 2026-09-24 (tools/rename.py). Address 0x80065fd8.

**Unit:** code_55dd4 · **Size:** 57 words (0xE4 bytes) · **Status:** MATCHED
(57/57 words, whole-image `./build-and-verify.sh` green)

## What it does

`TodActorMethods` slot `+0x108` (already known from the header comment:
"TodActor__Tick -- used by TodActor__Update when its `val` == 2"). A per-tick
bookkeeping function: bumps a counter, optionally fires a no-argument
callback, and optionally advances an iterator that wraps back to the start
of the same array `TodActor__SetTod` sets up — then unconditionally zeroes
`*self->unk14`:

```c
void TodActor__Tick(TodActor *self)
{
    self->unk24 = self->unk24 + 1;
    if (self->unk8C != 0) {
        ((void (*)(void))self->unk78)();
    }
    if (self->unk90 != 0 && self->unk80 >= 2) {
        self->unk88 = self->methods->slot134(self, self->unk88, 0);
        self->unk84 = self->unk84 + 1;
        if (self->unk84 >= self->unk80) {
            self->unk84 = 0;
            self->unk88 = (u8 *)(*(GroupObj **)(self->unk5C->unk30->arr + 8 + self->unk7C * 4))->entry + 8;
        }
    }
    *self->unk14 = 0;
}
```

## New knowledge

- **Two new `TodActor` fields**, both carved out of the previously
  fully-opaque `pad10[0x40]`: `+0x14 s32 *unk14` (dereferenced and zeroed
  unconditionally on every call — the pointed-at object isn't identified
  beyond that its first word is meaningful) and `+0x24 s32 unk24` (a plain
  incrementing counter).
- **`self->unk78` is genuinely called with NO arguments.** Confirmed by
  the disassembly: no `$a0` setup precedes the `jalr` at all. Its field
  type stays `void *` (it's still assigned from the untyped-as-function
  `slot118`/`slot11C`/`slot120` in `TodActor__SelectTickCallback`, already matched —
  retyping the field itself would risk that function's bytes for no
  reason); the call site casts it explicitly instead:
  `((void (*)(void))self->unk78)();`.
- **`TodActorMethods::slot134`'s return value IS used here, correcting
  its type.** `TodActor__SetTod` (STALLED, see its report) called this same
  slot and discarded the return — which is legal for ANY return type in C
  and was never evidence the slot is `void`. This function stores the
  result straight into `self->unk88` (a `u8 *`), so the slot is retyped
  `u8 *(*slot134)(TodActor *, void *, s32)`. This doesn't reopen
  `TodActor__SetTod`'s stall — that function's own residue was purely a
  register-allocation question in its final few words, unrelated to the
  return type — but it's a correction worth recording for whoever revisits
  it.
- **The iterator-wraparound reset reuses the EXACT SAME expression**
  `TodActor__SetTod` writes for its own iterator setup:
  `(u8 *)(*(GroupObj **)(self->unk5C->unk30->arr + 8 + self->unk7C * 4))->entry + 8`.
  This strongly confirms `self->unk7C`/`unk80`/`unk84`/`unk88` really are
  a single coherent "current index / limit / count / current pointer"
  iterator group, with this function being the "advance, and rewrap at
  the limit" half and `TodActor__SetTod` the "(re)seek to an explicit index"
  half.

No residue at all — matched on the first attempt once the two new fields
and the `slot134` return type were declared correctly.

### Proposed learning

**A discarded return value is never evidence a vtable slot is `void`** —
confirmed concretely here: `TodActor__SetTod` discarded `slot134`'s return
and this function consumes it. When a slot's first-observed call site
discards the result, leave a note (as `TodActor__SetTod`'s report and the
header comment both now do) rather than committing to `void`, and prefer
whichever caller DOES consume the value to fix the return type once one is
found.

## Naming

Round 75 (charlie), track 3.

- `TodActor__Tick` (was `func_80065FD8`), tier B. Occupies +0x108; reached from TodActor__Update (formerly OnClass6EF50Notify) code 2. Increments SceneNode's tick (+0x24), calls tickCallback while tickCallbackEnabled, and while todPlaying with todFrameCount >= 2 applies the next frame (applyTodFrame), advances todFrame and wraps to the TOD's first frame at todFrameCount; finally clears coord2->flg. Mechanics are all in the body; B because the per-call driver is known only as 'code 2 from the tag-5 companion'.

## Track 4 (2026-09-25, round 85, alpha)

The class (id 0x234, table `gTodActorMethods`) is unified as `TodActor` in `include/TodActor.h`: an Actor subclass (its ctor chains to Actor's first) and Entity's base. Any source block above is the pre-unification spelling (the local `TodActorMethods` of `include/code_55dd4.h`, `linkCompanion`/`unlinkCompanion`, `companion2`, `Unk5CObj`/`Unk70ElemObj`); the live body in `src/code_55dd4.c` takes the unified types and the inherited slot and field names (`addChild`/`removeChild`, Actor's `ticker`, `Actor *` parts, `ModelData *` modelData), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
