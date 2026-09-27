# TaskObjF__TaskObjF -- MATCHED 42/42, round 43

> Renamed from `func_8004E34C` on 2026-09-24 (tools/rename.py). Address 0x8004e34c.

Unit `class_3bb8c_d`. **REOPENED -- ASSIGNABLE** from round 42's `gp_rel`
resolution. The round-14 stub recorded 2 `gp_rel` hits and no derivation;
this round wrote and matched the function from scratch.

## What this function IS

`TaskObjF__TaskObjF` is the constructor for the class whose vtable is
`gTaskObjFMethods` -- `tools/classtable.py 0x80086DC4` places it exactly at that
table's own **+0x008 ctor slot**. This is the same real class
`GetTitleMenuMethods`/`New_TaskObjF` (already matched, earlier in this unit)
allocate and construct through `GetTaskObjFMethods()->ctor(self, arg0, arg1)`,
and the SAME real object `class_3bb8c_e.c` independently names `Node3bb8cE`
(its own local view, established there round 14 from ITS 19 functions --
`TaskObjF__ClearLinks`/`TaskObjF__Finalize`/etc. all operate on this same table).

## Body

```c
void TaskObjF__TaskObjF(GenericCtorObj_3bb8c_d *self, s32 arg1, s32 arg2)
{
    s32 count;

    Get_vtable_BasicClass()->ctor(self);
    self->methods = GetTaskObjFMethods();
    count = sTaskObjFCount;
    sTaskObjFCount = count + 1;
    if (count == 0) {
        InitCARD(arg1);
        StartCARD();
        _bu_init();
    }
    TaskObjF__ClearLinks(self);
    self->methods->slot40(self, arg2);
}
```

Byte-exact on the first build: `funcdiff.py TaskObjF__TaskObjF` -> `42/42 words
match (file 0x3EB4C-0x3EBF4)`; whole-image `OK: build matches retail
SLPS_015.56`.

## Derivation

1. `Get_vtable_BasicClass()->ctor(self)` -- the base-class (`BasicClass`) ctor
   chain call, one argument, already-canonical
   (`BasicMethods866E8F::ctor`, `include/class_3bb8c.h`).
2. `self->methods = GetTaskObjFMethods();` -- `GetTaskObjFMethods()` returns
   `&gTaskObjFMethods`, i.e. this class sets its OWN `methods` pointer directly
   to its own table, the same "base-ctor-chain" pattern already documented
   for `TitleMenu::TitleMenu__TitleMenu` in this same header.
3. A one-shot init guard: `sTaskObjFCount` (a plain `.sdata` `s32`, zero-
   initialized, `asm/data/7B12C.sdata.s`) is read, incremented
   unconditionally, and the PRE-increment value gates a block that runs
   `InitCARD(arg1)`/`StartCARD()`/`_bu_init()` -- all three PSX libcard
   BIOS entry points, linked SDK objects (`config/psyq-objects.txt`:
   `libcard/a74`, `libcard/a75`, `libcard/c112`) -- **only on the very
   first construction** of this class (a classic "init memory cards once"
   singleton-style guard).
4. `TaskObjF__ClearLinks(self)` -- already matched, `class_3bb8c_e.c`, under that
   unit's own `Node3bb8cE *` view; zeroes four resource-slot fields.
5. `self->methods->slot40(self, arg2)` -- forwards this function's own 3rd
   parameter verbatim to a new table slot at +0x040 (immediately after the
   existing `ctor` field's implicit end, with a `pad00C[0x040-0x00C]` gap
   filled in). `class_3bb8c_e.c`'s own independent view already names the
   concrete function at this address `TaskObjF__SetCardSlot` (still uncarved
   there).

## Header changes

`include/class_3bb8c.h`, all additive:

- `GenericCtorTable_3bb8c_d`: added `slot40` at +0x040 (new
  `pad00C[0x040-0x00C]` gap + the slot itself; the existing `ctor` field
  at +0x008 is untouched).
- New `GenericCtorObj_3bb8c_d` instance struct (just `methods` at +0x000)
  -- this unit's own minimal, independent local view of the same real
  object `class_3bb8c_e.c` calls `Node3bb8cE`.
- New extern `sTaskObjFCount` (`s32`, the one-shot init counter).

`src/class_3bb8c_d.c`: local (not shared-header) externs for
`TaskObjF__ClearLinks` (already matched elsewhere, under this unit's own `void *`
view rather than `class_3bb8c_e`'s `Node3bb8cE *`) and for `InitCARD`/
`StartCARD`/`_bu_init` (Sony's, linked from `lib/libcard`, declared the
same way `malloc`/`free`/`printf` are per-unit rather than in a shared
header -- CLAUDE.md's "To `include/` has one exception").

`TaskObjF__TaskObjF`'s own parameter types (`GenericCtorObj_3bb8c_d *`, `s32`,
`s32`) intentionally do NOT match the existing `GenericCtorTable_3bb8c_d
::ctor` field's declared type (`void *`, `void *`, `void *`) -- nothing in
this codebase type-checks the two against each other (the vtable's own
initializer is still raw, uncarved `.data`, not a C initializer), so this
is the same "independent arities for the same real callee" situation
already documented for `Get_vtable_TaskCore`/`BaseTaskCtorTable_3bb8c_c` versus
`TaskCoreMethods` in `include/code_2c054.h`.

### Proposed learning

When `tools/classtable.py <table>` places the function you are matching at
that table's OWN ctor/method slot (not merely a callee it calls into), the
function IS that class's identity method -- worth checking early, since it
explains why the function's first act is often `self->methods =
<table-getter>()` (the same-table self-assignment idiom already seen for
`TitleMenu`/`TitleMenu__TitleMenu`) rather than reading `self->methods` from
somewhere else.

## Naming (round 77, naming runner delta)

Renamed `func_8004E34C` -> `TaskObjF__TaskObjF`. **Tier A**: `tools/classtable.py 0x80086DC4` places this function exactly at `gTaskObjFMethods`'s own +0x008 ctor slot -- the constructor for the class `class_3bb8c_f.c`/`class_3bb8c_g.c` already name `TaskObjF` tree-wide. Follows the `New_Class`/`Class__Class` constructor convention.

## Track 7 (round 96, echo)

Naming: `D_8008AA30` -> `sTaskObjFCount` (tier A: the count of TaskObjF
constructions; the card libraries start when it was 0).

Comment moved here from the unit: InitCARD/StartCARD/_bu_init are
libcard, linked SDK objects (config/psyq-objects.txt: libcard/a74,
libcard/a75, libcard/c112), declared locally rather than in the shared
header, the same policy as malloc/free/printf (CLAUDE.md, "To include/
has one exception").
