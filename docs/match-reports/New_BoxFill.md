# New_BoxFill -- MATCHED (31/31 words)

> Renamed from `New_ClassEAC0` on 2026-09-25 (tools/rename.py). Address 0x800404d0.

> Renamed from `func_800404D0` on 2026-09-20 (tools/rename.py). Address 0x800404d0.

Unit `code_2cc8c_e`, carved round 14.

> **UPDATE (targeted permuter pass, round 17).** MATCHED, no permuter
> needed -- same fix as its sibling `New_FadeBox`, applied in the same
> pass. `docs/research/epilogue-merge-residue.md`'s discriminator (`return
> NULL;` must come textually LAST, after the success return) closed it on
> the first try once the preserved single-merged-return body was split:
>
> ```c
> ClassEAC0Obj *New_BoxFill(void *a0, void *a1, s32 a2) {
>     ClassEAC0Obj *self;
>
>     self = BMemPMgrAlloc(0x6C);
>     if (self != NULL) {
>         ((ClassEAC0Methods *)GetBoxFillMethods())->ctor(self, a0, a1, a2);
>         return self;
>     }
>     return NULL;
> }
> ```
>
> **One incidental fix needed along the way**: the preserved body's
> `GetBoxFillMethods()->ctor(...)` no longer compiles as written --
> `GetBoxFillMethods`'s declared return type is `Obj6EAC0Methods *` (per
> `include/code_2cc8c.h`), which has no field literally named `ctor` (its
> ctor-shaped slot is `slot08`, still unidentified as this unit's own). The
> header's own `ClassEAC0Methods` type (used identically by
> `FadeBox__FadeBox`, a few lines above this function in the same file) DOES
> have a `ctor` field at `+0x008`, and its own doc comment explicitly lists
> `New_BoxFill` as one of the vtable's callers through that slot -- so the
> correct call is an explicit cast, `((ClassEAC0Methods *)
> GetBoxFillMethods())->ctor(...)`, matching the pattern `FadeBox__FadeBox` already
> uses one statement earlier in this same unit (`base = (ClassEAC0Methods
> *)GetBoxFillMethods();`). No header change was needed; this is purely a
> call-site cast that the preserved body's snapshot predates.
>
> Verified: `./build-and-verify.sh` exit 0 (full-image SHA1 match),
> `funcdiff.py` 31/31.

## Shape

Sibling `New_X` allocator one level up the hierarchy, identical shape to
`New_FadeBox`.

## Residue (closed)

Identical single-word residue to `New_FadeBox`: retail's failure path
materialises `$v0` as a fresh zero (`addu $v0,$zero,$zero`); the earlier
merged single-return form produced `move $v0,$s0` instead. Same
"New_X epilogue-merge residue" class -- see
`docs/research/epilogue-merge-residue.md` for the discriminator (textual
order of the two `return`s) that closed both this function and
`New_FadeBox` in the same pass.

### Proposed learning

See `New_FadeBox`'s report for the primary learning (the epilogue-merge
class is a textual-return-order sensitivity, not an unfixable compiler
limitation). Specific to this function: when a preserved stall body fails
to compile against the CURRENT headers, check whether the call needs an
explicit cast to a sibling type that carries the field the body assumes --
`ClassEAC0Methods` vs. the accessor's own declared `Obj6EAC0Methods` return
type here -- rather than assuming the header itself needs changing. A
sibling function in the same unit (`FadeBox__FadeBox`) already demonstrated
the correct cast one statement earlier in the file.

## Naming (round 61, track 3)

**`New_BoxFill`** -- tier A. `New_X`-shaped allocator, mirroring
`New_FadeBox` one level down the chain: allocates 0x6C bytes
(`ClassEAC0Obj`'s own, smaller size) and dispatches
`((ClassEAC0Methods *)GetBoxFillMethods())->ctor(...)` on success.

## Track 4 (2026-09-25, round 85, charlie)

Class 0x64 (was D_8006EAC0) is unified as BoxFill in include/BoxFill.h: Viewport__DrawNode draws a node whose class-id low byte is 0x64 with GsSortBoxFill over the GsBOXF at +0x058 (pri +0x044, `relative` +0x048, x/y +0x050/+0x054). The body now takes `BoxFill *`; zero bytes changed. Renamed from `New_ClassEAC0`: BMemPMgrAlloc(0x6C) then GetBoxFillMethods()->ctor (tier A). Callers: TaskCore__SetTarget (listView), GraphRoom__BuildGraphPoints (100 dots), StyleBuildDecorSet (18 decor slots), ApplyStyleDecorationIfSet (gStyleDecorObj). The first argument is a {w, h} pair of words (read by halfword in Reset), the second the r,g,b bytes, the third the priority, so the parameters are (void *size, void *color, s32 pri).
