# Class86F88__StepCursorInView

> Renamed from `func_80052A58` on 2026-09-24 (tools/rename.py). Address 0x80052a58.

**Unit:** class_3bb8c_k · **Size:** 63 instructions (0xFC bytes) ·
**Status: MATCHED 63/63**, whole-image SHA1 green. Matched on the first
attempt.

## Role

Advances/retreats one end of `self`'s active element window
(`self->unk40[]`, the same 4-slot `Class86F88Elem *` array `Class86F88__ReleaseRows`/
`Class86F88__RefreshRows` use), releasing the element that falls out of the window
and dispatching to the element that enters it, then optionally notifies
`self` itself via its own `slot60`.

```c
void Class86F88__StepCursorInView(Class86F88 *self, s32 dir, s32 flag)
{
    Class86F88Elem **p;
    s32 idx;

    if (!self->unk50) {
        return;
    }
    idx = self->unk28 - self->unk20;
    p = &self->unk40[idx];
    (*p)->methods->slotB8(*p, &gClass86F88RowColor);
    if (dir) {
        self->unk28++;
        p++;
    } else {
        self->unk28--;
        p--;
    }
    (*p)->methods->slotB8(*p, &gClass86F88CursorColor);
    if (flag) {
        self->methods->slot60(self, 0);
    }
}
```

`self->unk40[idx]` is addressed via a genuine pointer walk (`p`), not
re-indexed each time -- matches retail computing the element address ONCE
(`&self->unk40[unk28-unk20]`) and then simply `p++`/`p--` for the second
dispatch, rather than recomputing `self->unk28 - self->unk20` again with
the updated `unk28`.

## New struct members

- `Class86F88Methods::slot60` (`void (*)(Class86F88 *, s32)`, offset
  0x060, was opaque padding) -- shared with `Class86F88__RefreshRows`'s own tail
  dispatch (still `INCLUDE_ASM` as of this function's match; both call
  sites pass `(self, 0)`).
- `gClass86F88RowColor` (`extern s32`) -- 4 bytes of rodata immediately before the
  already-declared `gClass86F88CursorColor` (itself `Class86F88__SetView`'s fixed 2nd
  `slotB8` argument); this function's FIRST dispatch uses the new one, its
  SECOND dispatch reuses `gClass86F88CursorColor`.

Both additions are additive splits of existing padding/adjacent rodata, no
existing declaration changed.

## Naming

Round 75 (bravo, track 3). `func_80052A58` -> `Class86F88__StepCursorInView`, **tier A**.

Slot +0x098 (`tools/classtable.py gClass86F88Methods`). Re-colours the current cursor row gClass86F88RowColor, moves `cursorIndex` +1 (dir != 0) or -1, colours the new row gClass86F88CursorColor, and if `notify` calls forwardToTarget(0). Callers: CursorUp (dir 0), CursorDown (dir 1).

Class86F88, per the round-75 pass, is a scrolling list selector: up to 4 visible rows of 26-character item text, a highlighted cursor row, a horizontal column offset (see the unit header comment of `src/class_3bb8c_k.c`).
