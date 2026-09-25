# Class6D430__Release

> Renamed from `DestroyChained` on 2026-09-25 (tools/rename.py). Address 0x800269f0.

> Renamed from `func_800269F0` on 2026-09-18 (tools/rename.py). Address 0x800269f0.

**Unit:** code_171e0 · **Size:** 24 instructions · **Status:** MATCHED (24/24 words, whole-image build verified byte-exact)

## What it does

Slot `+0x004` of the `D_8006D430` method table (see `include/code_171e0.h`'s
`Class6D430Methods`). It clears one flag, then explicitly chains
**both** destructors available to it: the class's own (`this->methods->dtor`,
itself `Class6D430__Finalize`, resolved through the vtable rather than by name) and
the base class's (`Get_vtable_BasicClass()->dtor`, `BasicClassMethods.dtor`), then
calls `BMemPMgrFree(this)` (a still-uncarved release/free routine, address
only) before returning `NULL` unconditionally.

## Derivation

```
lw    $v0, 0x0($s0)      ; v0 = this->methods
sh    $zero, 0x20($s0)   ; this->unk20 = 0
lw    $v0, 0xC($v0)      ; v0 = methods->dtor
jalr  $v0                ; this->methods->dtor(this)   -- return discarded
jal   Get_vtable_BasicClass
lw    $v0, 0xC($v0)      ; v0 = (base table)->dtor
jalr  $v0                ; Get_vtable_BasicClass()->dtor(this) -- return discarded
 addu $a0, $s0, $zero
jal   BMemPMgrFree
 addu $a0, $s0, $zero
addu  $v0, $zero, $zero  ; explicit return 0, not derived from any callee
```

The first pass at this function only wrote 3 calls (own dtor, base dtor via
`Get_vtable_BasicClass()->dtor`) and used its return value directly — that produced a
16/24-word body, 8 words short, because it silently dropped the 4th call
(`BMemPMgrFree(this)`) and the *explicit* `addu v0,zero,zero` at the end.
The `v0=0` at the tail is real, not incidental: nothing after the last call
touches `$v0`, so `return NULL;` (rather than `return BMemPMgrFree(this);`,
which would also compile with a `nop` in the same slot were `BMemPMgrFree`
non-void) is the form retail actually took — the byte pattern alone can't
distinguish these two, but the presence of the explicit zeroing instruction
after the 4th call rules out a tail-call return.

Once the missing call was restored, this matched immediately — no register
or scheduling residue at all, just an incomplete initial read of the
disassembly.

## Final C

```c
void *Class6D430__Release(Class6D430 *this) {
    this->unk20 = 0;
    this->methods->dtor(this);
    Get_vtable_BasicClass()->dtor(this);
    BMemPMgrFree(this);
    return NULL;
}
```

## Head broadcast levers — applicability

- **goto-vs-return (New_Pad lever):** not applicable. No branch in this
  function; it is straight-line code with one unconditional `return NULL;`.
- **loop-invariant hoisting (Pad__DispatchEvents lever 2):** not applicable, no loop.
- **prologue store-order barrier (Pad__DispatchEvents lever 1):** not applicable,
  matched on the first correct attempt with no store-order residue.

## Proposed learning

`D_8006D430`'s own vtable follows the exact same convention as
`BasicClassMethods` (header @0, an own-class slot @+0x004, ctor @+0x008,
dtor @+0x00C) — confirmed by dumping the table's raw words directly from
`disk/SLPS_015.56` rather than trusting a null-slot scan in isolation (see
`include/code_171e0.h`). A function occupying a class's own `+0x004` slot
explicitly re-invoking `+0x00C` (its own dtor) *and* the base class's dtor is
a legitimate "Destroy()" idiom distinct from the raw dtor slot itself, and is
worth checking for on any other class's `+0x004`-equivalent slot before
assuming it's an ordinary virtual.

## Naming

Round 52 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_800269F0` | `Class6D430__Release` | B |

**Track 4 (2026-09-25, Class6D430 unification): renamed from `DestroyChained`, tier A.** Occupies BasicClass's `release` slot (+0x004), inherited by all sixteen Class6D430 subclasses; like `BasicClass__Release` it finalizes (its own `finalize` slot, then BasicClass's) and frees the object, returning NULL. An override is named for the slot it occupies (FINISHING-PLAN track 4).

**Evidence.** `D_8006D430`'s `+0x004` own-class slot -- but verbatim-shared
with `D_8006D4E8` at the identical offset (`docs/match-reports/GetClass6D4E8Methods.md`),
so it is not really "Class6D430's own" and a `Class6D430__` prefix would
misattribute it. Clears the busy flag, then explicitly chains its own dtor
(`this->methods->dtor`) and the base BasicClass dtor before releasing the
object -- this function's own report already characterised this as a
"Destroy()" idiom distinct from the raw dtor slot. Named for the mechanic
(chains both destructors then frees), not for a resolved purpose.
