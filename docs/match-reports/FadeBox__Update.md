# FadeBox__Update -- MATCH (54/54 words, first attempt)

> Renamed from `Class6E99C__Update` on 2026-09-26 (tools/rename.py). Address 0x8003ff44.

> Renamed from `func_8003FF44` on 2026-09-20 (tools/rename.py). Address 0x8003ff44.

Unit `ScreenWidgets`, carved round 14. `FadeBoxMethods::update` (`+0x098`).

```c
void FadeBox__Update(FadeBoxObj *self, void *a1, s32 a2) {
    s32 old;

    if (a2 != 2) {
        return;
    }
    old = self->unk80;
    self->unk80 = old - 1;
    if (old > 0) {
        if (self->unk7C == 9) {
            return;
        }
        if (self->unk78 & 4) {
            self->unk64 += (u8)self->step;
        }
        if (self->unk78 & 2) {
            self->unk65 += (u8)self->step;
        }
        if (self->unk78 & 1) {
            self->unk66 += (u8)self->step;
        }
    } else {
        self->methods->stop(self, a1);
    }
}
```

**Correction: the `old <= 0` guard needs a POSITIVE `if (old > 0) {...}
else {...}` shape, not an early-return guard clause.** An earlier version
of this report used `if (old <= 0) { stop(...); return; }` followed by
the rest of the body at the top level -- that version reproduced retail's
branch CONDITION but put the wrong block as the physical branch target vs.
fallthrough (confirmed via `tools/asm-differ/diff.py`: retail falls
through into the `old > 0` body and branches AWAY into the `stop` call;
the early-return form did the reverse). Same branch-polarity family as
`func_8003F674`'s own confirmed lesson, generalised here to a guard clause
with a non-trivial body on BOTH sides rather than a bare early return.

## Notes

- The `unk80--` happens UNCONDITIONALLY before the `old <= 0` test (retail's
  delay slot stores the decremented value regardless of which way the
  branch on the OLD value goes) -- read as `old = self->unk80; self->unk80
  = old - 1; if (old <= 0) ...` rather than a naive `self->unk80--; if
  (self->unk80 < 0) ...`, which would test the wrong (already-decremented)
  value.
- `stop`'s own argument: nothing at the call site re-sets `$a1` before the
  `jalr`, so whatever this function's OWN `a1` parameter still holds gets
  forwarded through untouched -- an ordinary "argument register survives
  because nothing overwrote it" case (own `a1` is unused for anything else
  in this function).
- `self->unk64`/`unk65`/`unk66` are three independent `u8` counters, each
  incremented by the LOW BYTE of `self->step` (a full `s32` elsewhere)
  gated by a separate bit of the `unk78` flags word -- named
  `FadeBoxObj::unk64`/`unk65`/`unk66` in `include/Task.h`.

## Naming (round 61, track 3)

**`FadeBox__Update`** -- tier B. `FadeBoxMethods::update` (`+0x098`).
Mechanics: ignores every call except `a2 == 2`, then decrements a countdown
(`self->unk80`, still unnamed -- no other function gives it a purpose
beyond "the thing this loop decrements") and either accumulates a
per-tick `step` into three byte counters (gated per-bit by `unk78`) or
calls `stop` once the countdown expires. The `(self, void *a1, s32 a2)`
shape with `a2` acting as an event-code gate matches this project's own
generic per-object dispatch idiom (`Entity__Update(this, a1, a2)`,
`include/entity.h`'s slot98 note), which is the evidence for "Update"
specifically rather than a bespoke name. Purpose in the actual game (what
the color accumulation drives) is not established -- tier B, not A.

## Track 4 (2026-09-26, round 87, echo)

The +0x098 override of SceneNode's `update(self, sender, event)`
slot, parameters named for it. Fields: unk80 -> ticksLeft, unk78 ->
channels, unk64/65/66 -> BoxFill's color[0..2]; the sender goes to `stop`,
which removes it as a child. Image byte-identical.

## Track 6 (2026-09-26, round 93, charlie)

Renamed with the class: `Class6E99C` is now `FadeBox`
(`python3 tools/renametype.py Class6E99C FadeBox`, table
`python3 tools/rename.py D_8006E99C gFadeBoxMethods`), tier A; the evidence
is in New_FadeBox.md's Track 6 section. The method's own name was kept: it
already says what the body does. renametype.py rewrote the old class name in
this report's earlier history too (known, pending an operator decision).


## Track 7 (round 100, charlie)

- Local `old` -> `ticks`: the countdown's value before the decrement.
- `event != 2` is `event != FRAMECLOCK_EVENT_RUNNING`: the sender is the
  fade's source, which every caller passes as a FrameClock (ObjM's
  IntermediateBase `unk10`, "init's own New_FrameClock()"; Entity's
  `ticker`, Actor.h's class-5 FrameClock child), and FrameClock's tick
  sends event 2 to its parents when it counts a frame.
