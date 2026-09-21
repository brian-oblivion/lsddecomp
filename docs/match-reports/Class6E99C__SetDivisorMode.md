# Class6E99C__SetDivisorMode -- MATCH (3/3 words, first attempt, trivial)

> Renamed from `func_800404B4` on 2026-09-20 (tools/rename.py). Address 0x800404b4.

Unit `code_2cc8c_e`, carved round 14. `Class6E99CMethods::setDivisorMode` (`+0x0F0`),
a plain two-field setter: `void Class6E99C__SetDivisorMode(Class6E99CObj *self, s32 a1,
s32 a2) { self->altMode = a1; self->divisor = a2; }`.

## Naming (round 61, track 3)

**`Class6E99C__SetDivisorMode`** -- tier B. `Class6E99CMethods::setDivisorMode`
(`+0x0F0`), a plain two-field setter: `altMode = a1; divisor = a2;`.
`altMode` gates a division-vs-skip branch in both `Class6E99C__Configure`
(`q1 / self->divisor`) and `Class6E99C__StartFadeDefault`/`Class6E99C__Stop`
(resume-vs-fade-default branches), and `divisor` is only ever read as a
denominator -- hence "divisor mode" rather than a generic two-arg setter
name. What selecting the alternate mode represents in-game is not
established.
