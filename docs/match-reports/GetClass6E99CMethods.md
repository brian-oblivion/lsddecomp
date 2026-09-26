# GetClass6E99CMethods -- MATCH (4/4 words, first attempt, trivial)

> Renamed from `Class6E99C__GetMethods` on 2026-09-20 (tools/rename.py). Address 0x800404c0.

> Renamed from `func_800404C0` on 2026-09-20 (tools/rename.py). Address 0x800404c0.

Unit `code_2cc8c_e`, carved round 14. `Class6E99CObj`'s own bare
no-argument table getter, same idiom as `GetSceneNodeMethods`
(`include/code_d294.h`) for `SceneNodeObj`: `Class6E99CMethods
*GetClass6E99CMethods(void) { return &D_8006E99C; }`.

## Naming (round 61, track 3)

**`GetClass6E99CMethods`** -- tier A. Bare no-argument getter,
`return &D_8006E99C;` -- the exact same idiom as `code_d294.h`'s
`GetSceneNodeMethods` for `SceneNodeObj`'s own table. Mechanics fully
determine the name.

## Round 61 head review: renamed at merge

Alpha named this `Class6E99C__GetMethods`. The evidence for the name was
sound (a bare getter returning the class's method table, used as
`self->methods = ...()` in the ctor), but the SPELLING invented a third
style for a construct that already has two on `main`: `Get_vtable_X` (5
occurrences) and `GetXMethods` (7, including `GetSceneNodeMethods` and
`GetClass6D430Methods` for address-derived class names). FINISHING-PLAN
track 3 says do not invent a new style, so the head renamed it to
`GetClass6E99CMethods`, the closer of the two precedents because this
class's name is address-derived. Tier A unchanged; image byte-identical.
