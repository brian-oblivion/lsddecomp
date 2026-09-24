# Class86B60__BeginMemcardSave -- MATCHED 60/60, round 43

> Renamed from `func_8004DF64` on 2026-09-24 (tools/rename.py). Address 0x8004df64.

Unit `class_3bb8c_d`, class `Class86B60`. **REOPENED -- ASSIGNABLE** from
round 42's `gp_rel` resolution. The round-14 stub recorded 1 `gp_rel` hit
and no derivation; this round wrote and matched the function from scratch.

## Body

```c
extern GenericReleaseObj_3bb8c_d *func_8003B39C(const char *path);

void Class86B60__BeginMemcardSave(Class86B60 *self)
{
    if (self->unkAC == NULL) {
        self->unkA8 = func_8003B39C(D_800114F8);
        self->unkAC = New_TaskObjF((void *)1, NULL);
    }
    self->unkAC->methods->slot6C(self->unkAC, D_8008A9D0, &D_80086D6C,
                                  self->unkC->unk4, self->unk10, self->unk14,
                                  self->unk48);
    self->methods->slot10(self, self->unkAC);
    self->methods->slot14(self, self->unkC->unk4);
    self->methods->slot14(self, self->unk10);
}
```

Byte-exact on the first build: `funcdiff.py Class86B60__BeginMemcardSave` -> `60/60 words
match (file 0x3E764-0x3E854)`; whole-image `OK: build matches retail
SLPS_015.56`.

## Derivation

- Lazy-init guard: `if (self->unkAC == NULL) { ... }`, the mirror image of
  `Class86B60__Dtor`'s destructor guard on the same two fields
  (`unkA8`/`unkAC`).
- `func_8003B39C(D_800114F8)` -- `func_8003B39C` is already matched
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
  registers, 3 on the stack) at `Class86B60UnkACObjMethods_3bb8c_d`'s
  +0x06C, immediately before the already-known `slot70`
  (`Class86B60__EndMemcardSave`) with no gap. Every non-`self` argument is forwarded
  opaquely: `D_8008A9D0` (a NEW `%gp_rel` VALUE-of global, same pattern as
  `D_8008AA10`/`D_8008AA18`/`D_8008AA14` -- holds `0x80011454`, the
  "BISLPS-01556" string in the same unowned `D_80011434` rodata block,
  again with no `dlabel` of its own), `&D_80086D6C` (a real 16-entry
  pointer table, `asm/data/76DC8.data.s`, reached only by address here),
  `self->unkC->unk4` (already `void *`), `self->unk10` (already `void
  *`), a NEW field `self->unk14` (`void *`, established here -- lands
  exactly at +0x014, right after `unk10` with no gap), and `self->unk48`
  (already typed `struct Class86B60Unk48Obj *`).
- The three trailing calls reuse two ALREADY-established slots verbatim:
  `self->methods->slot10(self, self->unkAC)` and two calls to
  `self->methods->slot14`, once with `self->unkC->unk4` and once with
  `self->unk10` -- both already `void *`-typed parameters, so both
  compile with no cast.

## Header changes

`include/class_3bb8c.h`, all additive:

- `Class86B60`: new field `unk14` (`void *`) at +0x014, splitting the
  existing `pad014[0x02C-0x014]` gap (now `pad018[0x02C-0x018]`).
- `Class86B60UnkACObjMethods_3bb8c_d`: new `slot6C` (7 args) inserted
  before `slot70`, splitting the existing `pad008[0x070-0x008]` gap into
  `pad008[0x06C-0x008]` + the new slot (0x06C-0x070, exactly 4 bytes, no
  remainder).
- New externs: `D_800114F8` (`const char[]`, a real string dlabel),
  `D_8008A9D0` (`void *`, VALUE-of `%gp_rel`), `D_80086D6C` (`s32`,
  address-of placeholder for a real 16-entry pointer table).
- `src/class_3bb8c_d.c`: local extern for `func_8003B39C` (own arity/
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
