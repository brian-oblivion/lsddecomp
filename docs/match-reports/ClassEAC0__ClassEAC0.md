# ClassEAC0__ClassEAC0 -- MATCH (33/33 words, first attempt)

> Renamed from `func_8004054C` on 2026-09-20 (tools/rename.py). Address 0x8004054c.

Unit `code_2cc8c_e`, carved round 14. `ClassEAC0Obj`'s own constructor
(`ClassEAC0Methods::ctor`, slot `+0x008`) -- one level further down the
same "call the further-base ctor first, reset methods, redispatch finishConstruct"
chain `Class6E99C__Class6E99C` uses one level up:

```c
void ClassEAC0__ClassEAC0(ClassEAC0Obj *self, SkipShort2 *a1, void *a2, s32 a3) {
    GetClass6B5CCMethods()->ctor(self);
    self->methods = Obj6EAC0__GetBaseMethods();
    self->methods->finishConstruct(self, a1, a2, a3);
}
```

`GetClass6B5CCMethods` is `code_d294.h`'s own bare getter for the ACTUAL
`Class6B5CCObj` table (`gClass6B5CCMethods`) -- this is the point where the chain
bottoms out at the REAL base class two units over. Its `ctor` slot there
takes only `self` (`void *(*ctor)(void *self)`, `code_d294.h`), matching
this call site's own single-argument setup.

## Naming (round 61, track 3)

**`ClassEAC0__ClassEAC0`** -- tier A. `ClassEAC0Methods::ctor` (`+0x008`),
one level further down the same "call the further-base ctor first, reset
`self->methods`, redispatch `finishConstruct`" chain
`Class6E99C__Class6E99C` uses one level up. Named per the same
`Class__Class` constructor convention.
