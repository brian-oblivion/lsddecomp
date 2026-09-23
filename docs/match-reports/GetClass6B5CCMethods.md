# GetClass6B5CCMethods -- MATCHED (4/4 words)

> Renamed from `func_8001E57C` on 2026-09-18 (tools/rename.py). Address 0x8001e57c.

Round 12, runner delta. `code_d294_b`.

## Summary

This unit's own no-argument vtable getter, already well documented in
`include/code_d294.h`'s file banner and in `include/class_3bb8c.h` (which
calls the same symbol with a different arity from a different unit --
established cross-unit precedent, not new). Whole body is `lui/addiu
%hi/%lo(D_8006B5CC); jr $ra`.

```c
Class6B5CCMethods *GetClass6B5CCMethods(void) {
    return &D_8006B5CC;
}
```

`D_8006B5CC` itself is still rodata (not carved as C in this round) -- only
its address is declared, `extern Class6B5CCMethods D_8006B5CC;`, typed to
this getter's own return type. Confirmed against `tools/classtable.py
D_8006B5CC`, which shows the table starting exactly there with 45 slots,
all of which belong to this and the sibling `code_d294`/`code_d294_c` units.

## Evidence

Disassembly (`asm/nonmatchings/code_d294_b/GetClass6B5CCMethods.s`):
```
lui   $v0, %hi(D_8006B5CC)
addiu $v0, $v0, %lo(D_8006B5CC)
jr    $ra
 nop
```

### Proposed learning

None new -- this is the same getter/arity-per-call-site precedent already
documented in `include/code_d294.h` and `include/class_3bb8c.h`; this round
just supplies the getter's own C body.

## Naming (round 54, bravo, track 3)

**Not renamed -- PROPOSED only.** Proposed name: `GetClass6B5CCMethods`
(tier A -- "return my own vtable's address" is a pure getter, mechanics
are its whole purpose), following the exact convention this project
already uses for the same shape elsewhere: `GetClass6D4E8Methods`,
`GetClass6D3C8Methods`, `GetClass6D430Methods` (all
`include/code_171e0.h`/`code_179d8_q`'s own file family). Held back
from an actual rename because this exact symbol is called, BY NAME,
from a large number of OTHER units with DIFFERENT per-call-site arities
and return types (the established "arity/signature is per-call-site,
not a callee property" precedent this project already documents at
length -- see this unit's own header banner and `include/class_3bb8c.h`):
`src/code_d294.c`, `src/class_3bb8c_c.c`, `src/class_3bb8c_o.c`,
`src/class_3bb8c_p.c`, `src/class_3ac78.c`, `src/code_2cc8c_e.c`,
`src/code_2cc8c_f.c` (an ACTIVE runner's own unit this exact round),
`include/class_3bb8c.h`, `include/code_2cc8c.h`, `include/DreamSys.h`.
Renaming this symbol would edit every one of those files -- squarely
out of this round's `code_d294_b`-only scope, and a live collision risk
with this round's `code_2cc8c_f` runner. Posted to the broadcast in
strong terms: this is the single highest-value rename in this unit
(11 files reference the placeholder name) and the evidence for
`GetClass6B5CCMethods` is as solid as any tier-A name gets, but it
needs a head-level cross-unit pass, not a single naming runner.

## Extern arity (round 59)

**Verdict: extern FIXED** (two declarations), and this is one of only two of the
round's 17 findings where a declaration was actually wrong rather than
deliberate.

**Callee evidence** (`0x8001E57C`, and the definition at
`src/code_d294_b.c:736`):

```
8001e57c:  lui   v0,0x8007
8001e580:  addiu v0,v0,-18996     ; &D_8006B5CC
8001e584:  jr    ra
8001e588:  nop
```

The whole body. It reads NO argument register — not `$a0`, not `$a1`. The
definition is `Class6B5CCMethods *GetClass6B5CCMethods(void)`. Arity is zero,
and it is not a judgement call.

**The discriminator that separates this from the other 15 findings: is the
extra argument byte-load-bearing at the call site?** For `BMemPMgrInit`,
`Class6B5CC__LocalOffsetToWorldPos`, `CheckObj866E8CountFlag`, `LinkOwnerObj__*`,
`Class876FC__BuildRandomSprites` and `func_8002CF18`, retail *emits an instruction* for the
extra argument (`move a1,zero`, `move a3,zero`, `lw a2,164(s0)`, `li a0,0xff`),
so the declaration has to keep its arity or the call site's bytes change. Here
it emits nothing — all three call sites already have the value in the register:

```
8004a9a0:  jal   8001e57c <GetClass6B5CCMethods>   ; Class866E8__OnNotify (class_3ac78.c, 2-arg call)
8004a9a4:  move  s2,a2                             ; callee-save SPILL, not argument setup

8004aa88:  jal   8001e57c <GetClass6B5CCMethods>   ; Class866E8__OnElementEvent (class_3ac78.c, 2-arg call)
8004aa8c:  move  s2,a2                             ; ditto

8004d3e8:  jal   8001e57c <GetClass6B5CCMethods>   ; Class86AA0__Class86AA0 (via include/class_3bb8c.h, 1-arg call)
8004d3ec:  move  s0,a0                             ; ditto
```

Every one of those delay slots is the caller spilling its own incoming
arguments to callee-saved registers, which it would do with or without the
call's arguments. `$a0`/`$a1` simply still hold the caller's own incoming
values.

**What changed, and why it does not overturn round 9.** Round 9 measured the
arity correctly and concluded that retail's source "called one zero-argument
getter with different argument counts from different files, which is what C89
does with no prototype in scope." The faithful spelling of *no prototype in
scope* is an unspecified parameter list, not a fabricated one- or
two-parameter prototype. Round 9's standing instruction was specifically not to
reduce either declaration to `(void)` — which is correct and unchanged, since
`(void)` would make both call sites a `too many arguments` compile error. `()`
is a different spelling and was never considered.

- `src/class_3ac78.c:182` — `extern void *GetClass6B5CCMethods(Class866E8 *self, s32 arg1);` -> `extern void *GetClass6B5CCMethods();`
- `include/class_3bb8c.h:952` — `extern BaseCtorTableB_3bb8c_c *GetClass6B5CCMethods(void *self);` -> `extern BaseCtorTableB_3bb8c_c *GetClass6B5CCMethods();`

Return types are untouched (they are this project's multiple-independent-local-views
convention and each unit's `->ctor`/`->slot9C` reads depend on them). No call
site was changed. Neither unit including the shared header declares the symbol
locally, so no new same-TU collision is possible — checked before editing.

Oracle green (`build exit=0`, `OK: build matches retail`) after both edits: a
parameter-list change that moves zero bytes is exactly what should be expected
when the arguments were never load-bearing, and the whole-image SHA1 is the
proof.

### Proposed learning

**A wrong extern arity and a deliberate one are told apart by one measurement,
not by reading the callee alone: does retail emit an instruction for the
extra argument at the call site?** Reading only the callee says "it ignores
`$a1`" in both cases. The call site says whether the source had to mention the
argument. When it does (a `move a1,zero` in the delay slot), the declaration is
load-bearing and gets `arity-ok`. When it does not (the delay slot is a
callee-save spill or a `nop` and the value is already in the register), the
declaration is free and must agree with the definition — via `()` when a call
site passes arguments the definition does not have, since call sites are not
ours to change.
