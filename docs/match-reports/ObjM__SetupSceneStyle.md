# ObjM__SetupSceneStyle — MATCHED (round 45, 122/122 words)

> Renamed from `func_800534C8` on 2026-09-24 (tools/rename.py). Address 0x800534c8.

**Unit:** ObjMStyleActor · **Size:** 122 words (0x1E8 bytes)

Filed as a `gp_rel`-blocked stub in round 15, then re-affirmed "STILL
BLOCKED, stub report stands" in the round-24 re-screen. That blocker was
RESOLVED in round 42 (`--gp-symbols`, pinned in the Makefile). Matched
after two straightforward register/ordering fixes.

## Derivation

```c
typedef struct UnkCObj_3bb8c_l UnkCObj_3bb8c_l;
typedef struct UnkCObjMethods_3bb8c_l UnkCObjMethods_3bb8c_l;
struct UnkCObjMethods_3bb8c_l {
    u8 pad000[0x07C];
    s32 *(*slot7C)(UnkCObj_3bb8c_l *self, s32 arg1); /* +0x07C */
};
struct UnkCObj_3bb8c_l {
    UnkCObjMethods_3bb8c_l *methods; /* +0x000 */
};

extern void SetDreamAuxWorld(s32 a0, s32 a1, s32 a2, s32 a3, s32 a4);
extern void *GetStageGridDimensions(s32 index);
extern s32 sObjMProjectionBias;
extern s32 sObjMAcceptedClassIds;

void ObjM__SetupSceneStyle(Obj87034_3bb8c_l *self) {
    DreamSysObj_3bb8c_l *unk18 = self->unk18;
    StyleConfig *unk50 = self->unk50;
    UnkCObj_3bb8c_l *obj;
    s32 val;
    Obj14_3bb8c_l *unk14;

    unk18->methods->slot74(unk18);

    obj = *(UnkCObj_3bb8c_l **)self->unkC;
    val = *obj->methods->slot7C(obj, 0);
    unk18->methods->slot54(unk18, val / 2 * 5 / 3 + sObjMProjectionBias);

    unk18->methods->slot70(unk18, self->unk3C, &sObjMViewPoint, &sObjMViewRefPoint, 0);

    SetDreamAuxWorld((s32)self->unk38, (s32)self->unk14, (s32)self->unk3C, self->unk34, self->unk10);

    unk14 = self->unk14;
    self->methods->slot10(self, (s32)unk14);

    unk14->methods->slotBC(unk14, unk50->unk8, 0);
    unk14->methods->slotC4(unk14, 3, unk50->unk0, unk50->unk4);
    unk14->methods->slotE0(unk14, GetStageGridDimensions((s32)self->unk38));
    self->unk3C->methods->slot4C(self->unk3C, unk14);
    unk14->methods->slotDC(unk14, self->unk48);
    unk14->methods->slotCC(unk14, &sObjMAcceptedClassIds);
}
```

## Reading it

This is the biggest function of the round and mostly a straight-line chain
of method dispatches on `self->unk18` (a `DreamSysObj_3bb8c_l`) and
`self->unk14` (an `Obj14_3bb8c_l`), with one small piece of arithmetic in
the middle.

- **`self->unkC`'s pointee needed a SECOND, INDEPENDENT reading of the same
  field.** The existing shared type (`RegistrantObj_3bb8c_l`, established
  by `ObjM__AttachTarget`) is a single-`methods`-field object — exactly the
  layout this call site also needs (`*(self->unkC)` dereferences to an
  object with its OWN `methods` at offset 0), so no struct-field retype was
  needed; `*(UnkCObj_3bb8c_l **)self->unkC` reinterprets the SAME memory
  shape under a locally-scoped name (`UnkCObj_3bb8c_l`) for this one call
  site's own `slot7C` dispatch, per the project's independent-arities
  convention.
- **The division chain is plain C, not hand-derived bit twiddling.**
  Retail's `val/2` (round-to-zero signed halving: `srl`+`addu`+`sra`) and
  `(val/2*5)/3` (the classic `0x55555556` magic-multiply for signed
  division by 3) are BOTH canonical GCC 2.6.3 constant-division sequences —
  writing `val / 2 * 5 / 3` in ordinary C reproduces both exactly; no need
  to spell out the magic constant or shift amounts by hand.
- **`SetDreamAuxWorld`** (matched round 43, `src/world/DreamAux.c`) has no header
  prototype anywhere, so this unit's own call-site typing (all `s32`,
  matching its real definition) is local, same convention as
  `PickStageBgm`/`PickStageTexture`/etc. already declared in this file.
- **`GetStageGridDimensions`** (already matched, `src/world/StageGrid.c`) has a
  real prototype in `include/StageGrid.h` returning `StageGridDimensions
  *`, but this unit doesn't include that header and only forwards the
  return value opaquely, so a local `void *`-returning declaration is used
  instead — a different return type from the canonical one is fine for an
  external function across separate translation units, same convention as
  `New_TextRow`'s `FieldM7C *` vs. `Unk64Elem *` split noted in the
  shared header already.

**Two register/ordering traps, both closed without changing any dispatch
target or argument value:**

1. **`self->unk50` needed to be cached in a local (`unk50`), not read
   fresh at each of its three uses.** Reading `self->unk50->unk8` /
   `self->unk50->unk0` / `self->unk50->unk4` directly left the compiler
   with one FEWER live callee-saved value than retail (retail loads
   `self->unk50` into its own register, `s2`, right at the top of the
   function, before the very first dispatch call, and reuses that register
   for all three field reads at the end) — without the cache, the whole
   function came out one saved-register short and only 12/122 words
   matched, with every address after the first few instructions shifted.
2. **The final `Obj14_3bb8c_l *unk14 = self->unk14;` assignment had to move
   BEFORE the `self->methods->slot10(self, (s32)self->unk14)` call, using
   the `unk14` local as that call's own argument rather than re-reading
   `self->unk14` inline.** Retail's own register `s0` — which had held
   `self->unk18` for the whole function up to this point — gets reloaded
   with `self->unk14` exactly here and stays that way for the rest of the
   function; leaving the field read inline inside the call expression
   produced the right VALUE but a different instruction-scheduling order
   (9 words still mismatched, addresses shifted by 4 from this point on).

### Proposed learning

Two variants of a pattern this round's other reports already touched on
(`StampSaveTitleFileLetter`, `ItemList__CreateRows`): **when a struct field is read more
than once across a function, especially after any intervening call, cache
it in a local matching retail's own apparent register lifetime** — the
tell is a whole-function register-count mismatch (one fewer/more `s`-reg
used start to finish) rather than a localized diff. And **when a value is
about to become the "current object" for a long tail of dispatches AND is
also needed as a plain argument to an EARLIER call, assign it to a local
before that earlier call and use the local as the argument** — this reads
as a no-op in C but changes whether the compiler treats the field-read as
part of the earlier call's argument-evaluation (different scheduling) or
as a standalone statement whose result is reused verbatim by the call.

## Naming

Round 78 (charlie), FINISHING-PLAN track 3.

| was | now | tier | evidence |
| --- | --- | --- | --- |
| `func_800534C8` | `ObjM__SetupSceneStyle` | B | see below |

**Evidence.** vtable slot +0x080. The larger of the two `unk14`-configuring routines: computes a value from `*(UnkCObj_3bb8c_l **)self->unkC`'s own `slot7C`, dispatches `self->target`'s `slot54` with it, then configures `self->unk14` (grid dimensions via `GetStageGridDimensions`, world settings, style block) and links it back into `self->target` via `slot4C`.


## Track 4 (2026-09-26, round 89, echo)

The class is unified as ObjM in include/ObjM.h (table gObjMMethods, was D_80087034); the ObjMStyleActor/_l/_m views (ObjM_3bb8c_k, Obj87034_3bb8c_l, ObjM) and DayTaskStageMap.h's Obj4C/SubObjB/EventArg are gone. Byte-identical. `self->unkC` was IntermediateBase::initArgs (its unk0 is read), `world` the viewport (setProjection +0x054, attachViewChild), `unk14` the StageMap (setAmbientColor +0x0BC, setChildParams, setConfig, setGridSpan, setAcceptedTags), `unk34` TimedTask::sound, `unk10` the FrameClock.

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

## Round 95 (track 7, echo)

The unit-local view `UnkCObj_3bb8c_l` / `UnkCObjMethods_3bb8c_l` was deleted:
it was the DrawSystem (IntermediateBaseInitArgs::drawSystem) seen through
+0x07C, which include/DrawSystem.h has as `getDims(self, DrawRect *out)`
returning `ScreenDims *`. The body now reads
`drawSystem->methods->getDims(drawSystem, NULL)->w`, byte-identical. Its
comment, moved here:

> initArgs->unk0 as SetupSceneStyle reads it: its +0x07C returns a pointer
> to one word (DayTaskStageMap.h's SubObjE is the same call from
> DayTask__OnInit).

Proposed for the head: DayTaskStageMap.h's `SubObjE` is the same DrawSystem
call and can go the same way.

Comment history moved from the unit's externs:

> DreamAux.c's (MATCHED round 43); no header declares it. `world` is the
> DreamSys it installs as sDreamAuxWorld (track 4, round 88).

> GetStageGridDimensions comes from include/StageGrid.h, through DreamSys.h.

> The StageMap's accepted tags (setAcceptedTags), an opaque .data block
> (asm/data/76DC8.data.s) reached by address.

SetDreamAuxWorld's local prototype takes its parameter names from what the
definition does with them (stage, grid, world, sound, clock).
`sObjMViewPoint`, `sObjMViewRefPoint` and `sObjMAcceptedClassIds` are typed
(LongVec3, s32[]), dropping their casts. `/* GsFOG */` on EnterStyleSession's
setLightMode(vp, 1): SceneNode__SetLightMode writes a 3-bit field at bit 3
of the GsDOBJ2 attribute, where 1 is libgs's GsFOG (1<<3).

## Track 10 (2026-09-28, round 104, echo)

The six per-class aliases of `ColorRgb` (include/DrawSystem.h) -- BgLayerRgb, BoxFillRgb, FlatLightColor, LightRigRgb, ViewportRgb, TimBlockSrcColor -- are deleted and every use is spelled `ColorRgb`. Byte-identical. Measured for the MATCHING line in TaskCore__SetColors: the whole-struct copy is three `lb` then three `sb`, and rewriting one of the copies byte by byte loads each byte with `lbu` and interleaves the stores (asm-differ on the experiment), so the struct copy stays; the old line's "signed bytes" was wrong (ColorRgb's channels are u8; the lb comes from the block copy, not the type), and the same claim in GraphRoom__BuildGraphPoints' colour comment is corrected.

Viewport's `ViewportSize` merged into DrawSystem.h's `ScreenDims` (both `{s32, s32}`: getDims's result is what DayTask__OnInit hands to setScreenSize), whose fields are now `width`/`height`; DayTask__OnInit's cast between the two is gone. Byte-identical.
