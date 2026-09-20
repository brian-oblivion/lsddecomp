> Renamed from `func_8004001C` on 2026-09-20 (tools/rename.py). Address 0x8004001c.

# Class6E99C__SetStep -- MATCH (2/2 words, first attempt, trivial)

Unit `code_2cc8c_e`, carved round 14. Plain setter, `Class6E99CMethods::slotD0`
(`+0x0D0`): `void Class6E99C__SetStep(Class6E99CObj *self, s32 a1) { self->unk74 =
a1; }`. Counted as trivial work (a `jr $ra`-adjacent two-instruction body)
per the round's "count trivial functions separately" convention.

## Naming (round 61, track 3)

**`Class6E99C__SetStep`** -- tier B. `Class6E99CMethods::slotD0` (`+0x0D0`),
a plain one-field setter for `step` (`self->step = a1;`). Mechanics alone
(plain setter) would normally make this tier A, but the field it sets
(`step`) is itself a tier-B name (see that field's own note), so the
setter inherits the same uncertainty about what the value ultimately
represents in-game.
