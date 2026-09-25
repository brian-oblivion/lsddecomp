# Class6D4E8__Close -- MATCHED (round 47, alpha)

> Renamed from `func_80027480` on 2026-09-25 (tools/rename.py). Address 0x80027480.

42/42 words, byte-exact, file 0x17C80-0x17D28. Cold ground, first attempt to
diverge; second attempt (after fixing an if/else-if inversion) matched.

## Source

```c
/* Local view of the object Class6D4E8__Close/EnqueueCdRequest/CloseCdFile read
 * through -- the real struct is ObjA34_179D8H (src/code_179d8_h.c), but that
 * type is that unit's own local reading, not a shared header, so this unit
 * carries its own minimal view of the two offsets it actually touches. */
typedef struct Obj80027480 {
    u8 pad0[0xC];
    s32 unk0C;
    u8 pad10[0x28 - 0x10];
    u16 unk28;
} Obj80027480;

extern s32 gCdAsyncEnabled;
extern s32 D_8008A860;
extern s32 gCdBusy;

extern void CloseCdFile(Obj80027480 *self);
extern void LockCd(void);
extern void StartCdOperation(s32 arg0, s32 arg1);
extern void ResetCdStateMachine(void);
extern void EnqueueCdRequest(Obj80027480 *arg0, s32 arg1, s32 arg2, s32 arg3,
                           s32 arg4);
extern void UnlockCd(void);

void Class6D4E8__Close(Obj80027480 *self) {
    if (gCdAsyncEnabled == 0 && D_8008A860 == 0) {
        CloseCdFile(self);
        return;
    }
    LockCd();
    if (self->unk28 != 0) {
        if (gCdBusy == 0) {
            StartCdOperation(0, 0);
            self->unk0C = 0;
            ResetCdStateMachine();
        }
    } else {
        EnqueueCdRequest(self, 0, 3, 0, 0);
    }
    UnlockCd();
}
```

## What it took

The `&&`-guarded early return (`CloseCdFile`) was straightforward and
matched immediately (retail compiles a plain short-circuit `&&` as two
separate `bnez`-to-same-target branches, which is exactly this shape).

The one miss: I first wrote the second half as
`if (self->unk28 == 0) { EnqueueCdRequest(...); } else if (gCdBusy == 0) { ...
}` -- logically identical to the nested form, but GCC 2.6.3 lays out the
`then`-arm of an `if` as the fall-through immediately after the test, so this
inverted the *physical order* of the two blocks relative to retail (retail's
`beqz` falls through to the `gCdBusy` check block, with the
`EnqueueCdRequest` call living at the branch TARGET, further down). `m2ctx.py
--run`'s seed already had the correct nested shape
(`if (self->unk28 != 0) { if (gCdBusy == 0) {...} } else { EnqueueCdRequest(...); }`)
-- switching to it matched on the very next build with zero other changes.
This is the same family as round 46's "put the longer continuation in the
`if` body" lever, but on the OTHER axis: here it was about which of two
sibling blocks the compiler places first, driven purely by which one is the
`if`'s own `then`-arm vs its `else`, not about a trivial-early-return shape.

## Struct note

`self` is `ObjA34_179D8H` (see `src/code_179d8_h.c`), which this unit does
not include -- that type is `code_179d8_h`'s own local reading, not a shared
header. This unit's local view (`Obj80027480`) only names the two offsets
this function touches (`unk0C` at +0xC, a `s32`; `unk28` at +0x28, a `u16`,
loaded with `lhu`). Padding through +0xC and +0x28 is otherwise unestablished
here; do not treat the gaps as confirmed-empty, only as untouched by this
function.

### Proposed learning

**A logically-equivalent `if (a==0) {X} else if (b==0) {Y}` vs
`if (a!=0) { if (b==0) {Y} } else {X}` are NOT interchangeable under GCC
2.6.3** -- picking which condition is the OUTER `if`'s own true-arm decides
which of the two sibling blocks lands as the fall-through immediately after
the branch test and which lands at the branch target, further down in the
function. When the two blocks are unequal in size (as here: a 4-call block vs
a single call), get this from a seed (`m2ctx.py --run` reads the asm
directly and will not invent the wrong nesting) rather than restructuring the
seed's shape for readability -- restructuring it cost one full build/measure
cycle here for no functional reason.
