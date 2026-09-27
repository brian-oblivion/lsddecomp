# TaskObjF__AddChild — MATCH (41/41 words)

> Renamed from `func_8004E444` on 2026-09-24 (tools/rename.py). Address 0x8004e444.

**Unit:** class_3bb8c_e (round 14, `Node3bb8cE` class).

## What it does

`void TaskObjF__AddChild(Node3bb8cE *self, Res3bb8cE *res)`. If `res` is
non-NULL: chains to the base class's `addChild` (`Get_vtable_BasicClass()`'s
+0x010 slot), then reads `res->methods->header` (a type-tag word) and
stores `res` itself into one of self's four typed resource slots
(`unk60`/`unk64`/`unk78`/`unk7C`) depending on whether the tag is
2, 5, 0x10 or 0x20 respectively. `TaskObjF__RemoveChild` is the mirror
(remove/zero) of this function.

## Where it stood, and the fix

Reached 35/41 immediately with a naive if-chain — every VALUE and every
STORE TARGET was already right, but six words (0x494-0x4B0) were the
same six instructions in the wrong ORDER (a pure delay-slot-filler
transposition, zero address drift, confirmed via `asm-differ`).

**The cause: retail's comparisons for tag values 2 and 5 both use the
SAME 4-bit mask (`tag & 0xF`), not the full byte.** Only the 0x10/0x20
comparisons need the full 8-bit mask (0x10 and 0x20 don't fit in 4 bits).
My first attempt masked with `0xF` for case 2 only and immediately
widened to `0xFF` for case 5 — a semantically equivalent but structurally
different test, since `tag & 0xF == 5` and `tag & 0xFF == 5` agree
whenever the upper nibble matching is irrelevant to that particular
branch. GCC 2.6.3 evidently derives the NARROWEST mask that still
distinguishes each case from the ones decided so far, given the full
case-value set {2, 5, 0x10, 0x20} — reusing the 4-bit mask across the
first two comparisons and widening only where required. Writing the
source with that same narrow-then-wide split matched immediately:

```c
void TaskObjF__AddChild(Node3bb8cE *self, Res3bb8cE *res)
{
    s32 tag;

    if (res == NULL) {
        return;
    }
    Get_vtable_BasicClass()->addChild(self, res);
    tag = res->methods->header;
    if ((tag & 0xF) == 2) {
        self->unk60 = res;
        return;
    }
    if ((tag & 0xF) == 5) {
        self->unk64 = res;
        return;
    }
    if ((tag & 0xFF) == 0x10) {
        self->unk78 = res;
        return;
    }
    if ((tag & 0xFF) == 0x20) {
        self->unk7C = res;
    }
}
```

## What was tried and rejected

- `switch (tag & 0xFF) { case 2: ...; case 5: ...; case 0x10: ...;
  case 0x20: ...; }` — GCC 2.6.3 chose a completely different binary-search
  comparison tree for the switch form, producing large address drift
  (15/41, 185KB of outside-range diff). The genuine source shape here is a
  literal if-chain, not a switch, despite superficially looking
  switch-shaped.
- A bare `__asm__("")` scheduling barrier between the first and second
  `if` — inserted an extra `nop`, growing the frame and cascading drift
  (20/41, 177KB outside-range diff). This residue was never an
  order-only/barrier-fixable one; it needed the mask-width correction
  above.
- Precomputing `tag8 = tag & 0xFF` as its own statement before the first
  `if`, still testing case 2 against `tag & 0xF` — regressed to 27/41 with
  an extra instruction. Also wrong.
- Swapping comparison operand order (`5 == (tag & 0xFF)` vs
  `(tag & 0xFF) == 5`) — no effect either way (GCC 2.6.3 canonicalizes).

### Proposed learning

**When a case-dispatch chain tests several small integer constants against
a masked value, check whether EARLIER (smaller) case constants share a
narrower mask than LATER (larger) ones, rather than assuming one mask width
for the whole chain.** Here cases 2 and 5 (fit in 4 bits) reuse `tag & 0xF`
across two comparisons before the mask widens to `tag & 0xFF` for 0x10/0x20
(which don't fit in 4 bits). This is a new instance of the already-recorded
"GCC 2.6.3's switch/case-comparison strategy depends on the exact
case-value SET" family, but manifesting as an if-chain rather than an
actual `switch` — the superficially switch-shaped `andi`/`bne` sequence
here is NOT reproducible by a real `switch` statement (tried, regressed
badly); it wants the narrow-then-wide literal if-chain instead. See
`TaskObjF__RemoveChild`, which needed the identical narrow-then-wide split.

## Naming (round 78, track 3)

`func_8004E444` -> `TaskObjF__AddChild`. **Tier A.** Sits at `gTaskObjFMethods` +0x010 (asm/data/76DC8.data.s), the exact offset this unit's own `BaseMethods3bb8cE` view had already named `addChild`. Classifies the incoming `Res3bb8cE`'s tag word into one of the four typed resource slots (`res02`/`res05`/`res10`/`res20`) after forwarding to the base class's own `addChild` -- a textbook override-then-chain-to-base shape. `TaskObjF__` prefix: see the unit header comment.

## Constants (round 98, track 7)

`(id & 0xF) == 2` / `== 5` are `(id & CLASS_ID_ROOT_MASK) == PAD_CLASS_ID` /
`FRAMECLOCK_CLASS_ID` (BasicClass.h, Pad.h, FrameClock.h; gPadMethods' and
gFrameClockMethods' word +0x000 are 0x2 and 0x5). The two-nibble tests stay
literal: `& 0xFF` against 0x10 (gTextEntryMethods' word +0x000) and 0x20
(gItemListMethods'), proposed to the head as `TEXTENTRY_CLASS_ID` and
`ITEMLIST_CLASS_ID` because class_3bb8c_f.c's OnNotify spells the same ids.
Zero bytes changed.
