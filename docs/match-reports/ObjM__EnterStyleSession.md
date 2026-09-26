# ObjM__EnterStyleSession -- MATCHED

> Renamed from `func_80053764` on 2026-09-24 (tools/rename.py). Address 0x80053764.

Unit: `src/class_3bb8c_l.c`. Runner: echo, round 16.

118/118 words, byte-exact. `./build-and-verify.sh` green (whole-image SHA1
verified).

## Signature

```c
void ObjM__EnterStyleSession(Obj87034_3bb8c_l *self);
```

## Final C

```c
void ObjM__EnterStyleSession(Obj87034_3bb8c_l *self) {
    DreamSysMethods_3bb8c_l *m;
    DreamSysMethods_3bb8c_l *m2;
    DreamSysObj_3bb8c_l *unk18;
    DreamSysObj_3bb8c_l *newObj;
    StyleConfig *unk50;
    s32 local10;
    s32 ret;
    s32 a2;
    void *a1;

    self->unk68 = 1;
    self->unk3C->methods->slotF8(self->unk3C, self->unk44, self->unk40);
    self->unk14->methods->slotEC(self->unk14);

    unk18 = self->unk18;
    unk50 = self->unk50;
    unk18->methods->slot60(unk18, 1);
    unk18->methods->slot64(unk18, unk50->unkC);
    unk18->methods->slot6C(unk18, unk50->unk1C);
    m = unk18->methods;
    if (unk50->unk14 != 1) {
        a1 = unk50->unk18;
    } else {
        a1 = unk50->unkC;
    }
    m->slot68(unk18, a1);
    unk18->methods->slotB0(unk18, 0);
    unk18->methods->slotB4(unk18, 1);

    newObj = unk18->methods->slotAC(unk18);
    self->methods->slot10(self, (s32)newObj);

    ret = self->unk3C->methods->slotF0(self->unk3C, &local10, -1);
    newObj->methods->slotF0(newObj, (s32 *)ret, (ret != 0) ? 3 : 0);
    m2 = newObj->methods;
    if (ret == 0) {
        a2 = -1;
    } else {
        a2 = local10;
    }
    m2->slotD4(newObj, self->unk10, a2, 0);
}
```

## New fields/slots added to `include/class_3bb8c.h` (all additive)

- `Obj87034_3bb8c_l::unk10` (`s32`, +0x010) -- replaced a same-sized
  padding gap that already ended exactly there, so this is a pure
  padding-to-field conversion, not a slide.
- `Obj87034_3bb8c_l::unk40`, `::unk44` (`s32` each, +0x040/+0x044) --
  carved out of the existing `pad40[0x050-0x040]` gap.
- `StyleConfig::unk1C` (`void *`, +0x01C) -- appended after the
  existing `unk18` (which was the struct's last field), so this only grows
  the struct, no slide of any existing member.
- `Obj14Methods_3bb8c_l::slotEC` (`void(void*)`, +0x0EC) -- carved out of
  the existing `pad00[0x10C]` gap.
- `DreamSysMethods_3bb8c_l`: six new slots -- `slot60(void*,s32)`,
  `slot64(void*,void*)`, `slot68(void*,void*)`, `slot6C(void*,void*)`,
  `slotAC(void*) -> struct DreamSysObj_3bb8c_l*`, `slotB0(void*,s32)`,
  `slotB4(void*,s32)`, `slotD4(void*,s32,s32,s32)`, `slotF8(void*,s32,s32)`
  -- all carved out of existing padding gaps, ascending offset order
  preserved.

### A forward-reference wrinkle (worth flagging)

`slotAC`'s return type is `struct DreamSysObj_3bb8c_l *`, spelled with the
elaborated `struct` keyword rather than the `DreamSysObj_3bb8c_l` typedef,
because `DreamSysMethods_3bb8c_l` (which contains `slotAC`) is defined
*before* the `DreamSysObj_3bb8c_l` typedef later in the same header. A
forward `typedef struct DreamSysObj_3bb8c_l DreamSysObj_3bb8c_l;` placed
before `DreamSysMethods_3bb8c_l` looked like the obvious fix but GCC 2.6.3
rejects a duplicate `typedef struct X X;` even when the underlying type is
identical each time (`redefinition of 'DreamSysObj_3bb8c_l'`), and this
header is included by two OTHER live units this round (alpha's
`class_3bb8c_i`, bravo's `class_3bb8c_k`) so the break wasn't visible until
a full `./build-and-verify.sh`. The elaborated-`struct` spelling sidesteps
the whole issue: C allows naming an incomplete struct tag via a pointer
before its full definition is in scope, no typedef required.

## `slotF0` now has two independently-typed call sites on the SAME slot

`self->unk3C->methods->slotF0(self->unk3C, &local10, -1)` here matches the
existing declaration exactly (`s32(void*, s32*, s32)`, established -- if
unmatched -- by the `ObjM__EnterState4` stall this same round). But this
function ALSO dispatches `slotF0` a second time, on a *different* instance
(`newObj`, the `slotAC` return value): `newObj->methods->slotF0(newObj,
(s32*)ret, (ret != 0) ? 3 : 0)`, where the 2nd argument is a plain `s32`
cast through a pointer type, not a real buffer pointer. Per this project's
"multiple call sites can type one slot's argument differently, as long as
each reproduces its own site's codegen" convention (see e.g.
`GridCellMethods::slotB8` in this same header), both are left as-is; the
cast documents the mismatch rather than hiding it.

## The levers that mattered (both are the same lever twice)

Two separate spots in this function needed the identical fix, and it is
the same one CLAUDE.md's stalled-and-rescued `FillRVectors3` guidance and
this round's `ObjM__EnterState5` report both independently rediscovered://
**GCC 2.6.3 -O2 hoists a load that both arms of an if/else need in common
to BEFORE the branch, but only when the source hands it a name to hoist.**

1. `unk18->methods->slot68(unk18, a1)`, called after an if/else that only
   picks `a1`: writing the call once, after the if/else, was necessary
   (matches CLAUDE.md's shared-tail idiom) but NOT sufficient -- with
   `unk18->methods` re-spelled inline at the call site, GCC reloaded it
   fresh AFTER the branch merge instead of hoisting it before, costing 2
   words and putting the wrong value load inside each arm. Introducing an
   explicit `DreamSysMethods_3bb8c_l *m = unk18->methods;` BEFORE the
   if/else, then calling `m->slot68(...)`, reproduced retail's hoist
   exactly.
2. The if/else's condition polarity mattered too, independently of the
   hoist: retail's actual branch instruction is `beq v0,s0,<equal-case>`
   (branches to the `unk50->unk14 == 1` case), with the NOT-equal case as
   the fallthrough. Source written the "natural" way
   (`if (== 1) {A} else {B}`) put `A` as the fallthrough instead (GCC's
   ordinary if/else lowering places the `if`-arm as the fallthrough and the
   `else`-arm as the branch target) -- the SAME polarity lesson as
   `ObjM__EnterState5` in this unit, this round. Inverting the source condition
   and swapping the two arms (`if (!= 1) {B} else {A}`) fixed it.
3. The identical hoist was needed AGAIN for `newObj->methods->slotD4(...)`
   after the `ret == 0` branch -- but reusing the SAME local (`m`) for both
   hoists picked the wrong register for the second one (`v1` instead of
   retail's `v0`), landing at 116/118. A second, independently-named local
   (`m2`) shortened each hoisted value's live range back down to what
   retail's own register allocator apparently assumed, and closed the gap
   to 118/118. This second-hoist-needs-its-own-name detail was NOT
   predictable from source structure alone -- it only showed up by
   comparing `tools/asm-differ/diff.py` output word-for-word after the
   first hoist already matched everything else.

### Proposed learning

When a byte-for-byte `asm-differ` diff shows the ENTIRE tail of a function
matching except one instruction that looks like "the same value, reloaded,
in a different register" (not a different value, not a different opcode),
suspect a hoist the C didn't ask for: introduce a named local for the
common sub-expression right before the branch that needs it. If a first
hoist like this fixes one divergence but a second, later, structurally
IDENTICAL divergence remains, don't reuse the same local for both -- their
live ranges can interact and change which physical register GCC 2.6.3
picks for the second one. Give each hoisted value its own name.

## Naming

Round 78 (charlie), FINISHING-PLAN track 3.

| was | now | tier | evidence |
| --- | --- | --- | --- |
| `func_80053764` | `ObjM__EnterStyleSession` | B | see below |

**Evidence.** vtable slot +0x088. The largest setup routine in the unit: configures `self->target`, then spawns a NEW `DreamSysObj_3bb8c_l` via `self->unk18`'s own `slotAC` and dispatches it via self's `slot10` -- a heavier "begin" step than `ObjM__SetupSceneStyle`, consistent with the `EnterState`-family naming used for the class's other heavy setup slots.


## Track 4 (2026-09-26, round 89, echo)

The class is unified as ObjM in include/ObjM.h (table gObjMMethods, was D_80087034); the class_3bb8c_k/_l/_m views (ObjM_3bb8c_k, Obj87034_3bb8c_l, ObjM) and class_39e08.h's Obj4C/SubObjB/EventArg are gone. Byte-identical. `world` is the viewport (NodeGuardedViewport: setLightMode, setClearColor, setFogNear, setFarColor, setUnkB4, setDrawEnabled), its getSubHandle the FadeBox fade box (setDivisorMode +0x0F0, startFadeDown +0x0D4); `attached` is `inSession`.

## Round 94 (track 6, charlie)

`Unk50Struct_3bb8c_l` is `StyleConfig` (ObjM::styleConfig's pointee,
include/class_3bb8c.h), its fields named from their readers: `unk0`/`unk4`
-> `lightDirs`/`lightColors` (the StageMap's setChildParams), `unk8` ->
`ambientColor` (setAmbientColor), `unkC` -> `clearColor` (the viewport's
setClearColor), `unk14` -> `colorMode` (1: the far colour is clearColor; 2:
the TimBlockSrc fades to clearColor), `unk18` -> `farColor` (setFarColor
otherwise), `unk1C` -> `fogNear` (setFogNear). Tier B for `colorMode`, whose
two tested values are read by different methods for different choices.
Zero bytes changed.
