# BoxFill__BoxFill -- MATCH (33/33 words, first attempt)

> Renamed from `ClassEAC0__ClassEAC0` on 2026-09-25 (tools/rename.py). Address 0x8004054c.

> Renamed from `func_8004054C` on 2026-09-20 (tools/rename.py). Address 0x8004054c.

Unit `code_2cc8c_e`, carved round 14. `ClassEAC0Obj`'s own constructor
(`ClassEAC0Methods::ctor`, slot `+0x008`) -- one level further down the
same "call the further-base ctor first, reset methods, redispatch finishConstruct"
chain `FadeBox__FadeBox` uses one level up:

```c
void BoxFill__BoxFill(ClassEAC0Obj *self, BoxFillSize *a1, void *a2, s32 a3) {
    GetSceneNodeMethods()->ctor(self);
    self->methods = GetBoxFillMethods();
    self->methods->finishConstruct(self, a1, a2, a3);
}
```

`GetSceneNodeMethods` is `code_d294.h`'s own bare getter for the ACTUAL
`SceneNodeObj` table (`gSceneNodeMethods`) -- this is the point where the chain
bottoms out at the REAL base class two units over. Its `ctor` slot there
takes only `self` (`void *(*ctor)(void *self)`, `code_d294.h`), matching
this call site's own single-argument setup.

## Naming (round 61, track 3)

**`BoxFill__BoxFill`** -- tier A. `ClassEAC0Methods::ctor` (`+0x008`),
one level further down the same "call the further-base ctor first, reset
`self->methods`, redispatch `finishConstruct`" chain
`FadeBox__FadeBox` uses one level up. Named per the same
`Class__Class` constructor convention.

## Track 4 (2026-09-25, round 85, charlie)

Class 0x64 (was D_8006EAC0) is unified as BoxFill in include/BoxFill.h: Viewport__DrawNode draws a node whose class-id low byte is 0x64 with GsSortBoxFill over the GsBOXF at +0x058 (pri +0x044, `relative` +0x048, x/y +0x050/+0x054). The body now takes `BoxFill *`; zero bytes changed. Renamed from `ClassEAC0__ClassEAC0`: gBoxFillMethods's +0x008 occupant (tier A). Ctor chain: it calls GetSceneNodeMethods()->ctor first, so the id parent (0x4) is the ctor-chain parent; FadeBox__FadeBox calls this one first. The +0x040 dispatch now goes through the inherited `reset` slot, cast to BoxFillResetFn (Reset takes the ctor's arguments).

## Track 6 (round 99, alpha)

`SkipShort2` (`s16 x; u8 pad2[2]; s16 y;`, named for its layout) is now
`BoxFillSize` (`tools/renametype.py SkipShort2 BoxFillSize --any-stem`), and
its fields are what every caller passes: two words, `s32 w; s32 h;`. Storing
an s32 field into the u16 `boxW`/`boxH` still reads only the low halfword
(`lhu` at +0x000/+0x004): measured byte-exact with the whole image, so the
halfword-and-gap view was never load-bearing. Accessors `size->x`/`->y`
became `size->w`/`->h` in BoxFill__Reset, FadeBox__PushPosition and
BoxFill__SetSize (which still casts: the setSize slot keeps `s32 *`, since
its caller in TaskViewport passes an `s32 size[2]`).
