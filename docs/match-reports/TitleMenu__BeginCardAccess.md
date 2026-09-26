# TitleMenu__BeginCardAccess -- MATCHED 60/60, round 43

> Renamed from `TitleMenu__BeginMemcardSave` on 2026-09-26 (tools/rename.py). Address 0x8004df64.

> Renamed from `Class86B60__BeginMemcardSave` on 2026-09-26 (tools/rename.py). Address 0x8004df64.

> Renamed from `func_8004DF64` on 2026-09-24 (tools/rename.py). Address 0x8004df64.

Unit `class_3bb8c_d`, class `TitleMenu`. **REOPENED -- ASSIGNABLE** from
round 42's `gp_rel` resolution. The round-14 stub recorded 1 `gp_rel` hit
and no derivation; this round wrote and matched the function from scratch.

## Body

```c
extern GenericReleaseObj_3bb8c_d *New_TimImage(const char *path);

void TitleMenu__BeginCardAccess(TitleMenu *self)
{
    if (self->unkAC == NULL) {
        self->iconHandle = New_TimImage(D_800114F8);
        self->unkAC = New_TaskObjF((void *)1, NULL);
    }
    self->unkAC->methods->slot6C(self->unkAC, D_8008A9D0, &D_80086D6C,
                                  self->handlerTable->unk4, self->unk10, self->unk14,
                                  self->unk48);
    self->methods->slot10(self, self->unkAC);
    self->methods->slot14(self, self->handlerTable->unk4);
    self->methods->slot14(self, self->unk10);
}
```

Byte-exact on the first build: `funcdiff.py TitleMenu__BeginCardAccess` -> `60/60 words
match (file 0x3E764-0x3E854)`; whole-image `OK: build matches retail
SLPS_015.56`.

## Derivation

- Lazy-init guard: `if (self->unkAC == NULL) { ... }`, the mirror image of
  `TitleMenu__Finalize`'s destructor guard on the same two fields
  (`iconHandle`/`unkAC`).
- `New_TimImage(D_800114F8)` -- `New_TimImage` is already matched
  project-wide under many independent local arities/return types (see
  e.g. `src/class_3bb8c_g.c`, `src/class_3bb8c_i.c`); this unit's own view
  returns exactly what it is stored into, `GenericReleaseObj_3bb8c_d *`.
  `D_800114F8` is a real dlabel string, `"CARD\FILEICN1.TIM"`
  (`asm/data/1C34.rodata.s`) -- referenced by symbol per CLAUDE.md's rule
  against re-writing an already-emitted string literal.
- `New_TaskObjF((void *)1, NULL)` -- the already-matched New_X allocator
  earlier in this same unit, called here with its own two literal
  arguments (`1`, `0`); return type `void *` needs no cast into
  `self->unkAC`.
- `self->unkAC->methods->slot6C(...)` -- a NEW 7-argument slot (4 in
  registers, 3 on the stack) at `TitleMenuUnkACObjMethods_3bb8c_d`'s
  +0x06C, immediately before the already-known `slot70`
  (`TitleMenu__EndMemcardSave`) with no gap. Every non-`self` argument is forwarded
  opaquely: `D_8008A9D0` (a NEW `%gp_rel` VALUE-of global, same pattern as
  `D_8008AA10`/`D_8008AA18`/`D_8008AA14` -- holds `0x80011454`, the
  "BISLPS-01556" string in the same unowned `D_80011434` rodata block,
  again with no `dlabel` of its own), `&D_80086D6C` (a real 16-entry
  pointer table, `asm/data/76DC8.data.s`, reached only by address here),
  `self->handlerTable->unk4` (already `void *`), `self->unk10` (already `void
  *`), a NEW field `self->unk14` (`void *`, established here -- lands
  exactly at +0x014, right after `unk10` with no gap), and `self->unk48`
  (already typed `struct TitleMenuUnk48Obj *`).
- The three trailing calls reuse two ALREADY-established slots verbatim:
  `self->methods->slot10(self, self->unkAC)` and two calls to
  `self->methods->slot14`, once with `self->handlerTable->unk4` and once with
  `self->unk10` -- both already `void *`-typed parameters, so both
  compile with no cast.

## Header changes

`include/class_3bb8c.h`, all additive:

- `TitleMenu`: new field `unk14` (`void *`) at +0x014, splitting the
  existing `pad014[0x02C-0x014]` gap (now `pad018[0x02C-0x018]`).
- `TitleMenuUnkACObjMethods_3bb8c_d`: new `slot6C` (7 args) inserted
  before `slot70`, splitting the existing `pad008[0x070-0x008]` gap into
  `pad008[0x06C-0x008]` + the new slot (0x06C-0x070, exactly 4 bytes, no
  remainder).
- New externs: `D_800114F8` (`const char[]`, a real string dlabel),
  `D_8008A9D0` (`void *`, VALUE-of `%gp_rel`), `D_80086D6C` (`s32`,
  address-of placeholder for a real 16-entry pointer table).
- `src/class_3bb8c_d.c`: local extern for `New_TimImage` (own arity/
  return type, per the project's established independent-views
  convention for this widely-shared external symbol).

No existing declaration was retyped or resized.

### Proposed learning

None beyond what round 43's earlier reports in this unit already
recorded. This function reused three already-established slots
(`slot10`, `slot14` twice) verbatim and needed only one genuinely new
slot (`slot6C`) plus one new scalar field (`unk14`) -- the smoothest of
the five matched so far, and further evidence that this unit's
`gp_rel`-blocked stalls were uniformly straightforward once the toolchain
blocker itself was the only thing stopping them.

## Naming (round 77, naming runner delta)

Renamed `func_8004DF64` -> `TitleMenu__BeginCardAccess`. **Tier B**: Lazy-inits `self->iconHandle` (`New_TimImage(D_800114F8)`, a real dlabel "CARD\\FILEICN1.TIM") and `self->unkAC` (`New_TaskObjF`), then dispatches `unkAC`'s `slot6C` with `D_8008A9D0` (VALUE-of, holds "BISLPS-01556" -- Sony's memcard save-header game-ID convention) plus filename-related fields. Named from the game-ID string as the strongest evidence this is the start of a memory-card save operation; the exact protocol is not established.

## Track 4 (2026-09-26, round 88)

TimImage is unified (`include/TimImage.h`); this unit's local `extern
GenericReleaseObj_3bb8c_d *New_TimImage(const char *)` is deleted. The call
casts its argument to `char *` and its result to
`GenericReleaseObj_3bb8c_d *`, the type TitleMenu's own view gives
`iconHandle` (a TimImage; retyping TitleMenu's field is that class's job).
Image byte-identical.

## Track 4 (2026-09-26, round 88, bravo)

TitleMenu is unified in include/TitleMenu.h (TASKCORE_SLOTS/TASKCORE_FIELDS plus its own). `handlerTable->unk4` is TaskCore's `initArgs->unk4`, `unk48` its `sound`; slot10/slot14 are BasicClass's addChild/removeChild (saveCtrl upcast to BasicClass *); iconHandle is a `struct TimImage *`, so the New_TimImage cast is gone. Byte-identical (whole image green, 0 new warnings, nonmatching green).
