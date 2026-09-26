# FadeBox__SetDivisorMode -- MATCH (3/3 words, first attempt, trivial)

> Renamed from `Class6E99C__SetDivisorMode` on 2026-09-26 (tools/rename.py). Address 0x800404b4.

> Renamed from `func_800404B4` on 2026-09-20 (tools/rename.py). Address 0x800404b4.

Unit `code_2cc8c_e`, carved round 14. `FadeBoxMethods::setDivisorMode` (`+0x0F0`),
a plain two-field setter: `void FadeBox__SetDivisorMode(FadeBoxObj *self, s32 a1,
s32 a2) { self->altMode = a1; self->divisor = a2; }`.

## Naming (round 61, track 3)

**`FadeBox__SetDivisorMode`** -- tier B. `FadeBoxMethods::setDivisorMode`
(`+0x0F0`), a plain two-field setter: `altMode = a1; divisor = a2;`.
`altMode` gates a division-vs-skip branch in both `FadeBox__Configure`
(`q1 / self->divisor`) and `FadeBox__StartFadeUp`/`FadeBox__Stop`
(resume-vs-fade-default branches), and `divisor` is only ever read as a
denominator -- hence "divisor mode" rather than a generic two-arg setter
name. What selecting the alternate mode represents in-game is not
established.

## Track 6 (2026-09-26, round 93, charlie)

Renamed with the class: `Class6E99C` is now `FadeBox`
(`python3 tools/renametype.py Class6E99C FadeBox`, table
`python3 tools/rename.py D_8006E99C gFadeBoxMethods`), tier A; the evidence
is in New_FadeBox.md's Track 6 section. The method's own name was kept: it
already says what the body does. renametype.py rewrote the old class name in
this report's earlier history too (known, pending an operator decision).

