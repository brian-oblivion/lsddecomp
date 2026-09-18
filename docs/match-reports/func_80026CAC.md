# func_80026CAC

**Unit:** code_171e0 · **Size:** 15 words · **Status:** MATCHED, round 43
(2026-09-15, runner bravo). 15/15 words, byte-exact whole-image build.

## History

The round-2026-08-29-a/2026-08-30-a stub report never attempted this
function (it was routed straight to a `gp_rel`-blocked stub, no derivation).
Round 42 resolved `gp_rel` project-wide. Round 43 derived the body fresh
from `asm/nonmatchings/code_171e0/func_80026CAC.s` and matched on the first
build.

## What it does

An `if`/`else` mode dispatch, both arms tail-calling a function and
returning its result:

```
lw    $v1, %gp_rel(D_8008A84C)($gp)
ori   $v0, $zero, 0x23
beq   $v1, $v0, .L80026CD0     # D_8008A84C == 0x23 -> the "then" arm
  jal GetClass6D4E8Methods             # fallthrough (not equal) -- called first in ROM order
  j .L80026CD8
.L80026CD0:
  jal GetVabDriverMethods             # equal-to-0x23 arm
.L80026CD8:
  <epilogue, returns whatever $v0 holds>
```

The `beq` branches to the **equal** case directly (no negation), so the
direct reading `if (cond) A(); else B();` is the right mapping (not the
inverted-condition idiom GCC sometimes uses) -- confirmed by matching, first
try, with the straightforward `if`/`else` written in source order.

`GetVabDriverMethods` is confirmed elsewhere in the repo
(`src/code_179d8_e.c`: `TableD9BC *GetVabDriverMethods(void) { return &gVabDriverMethods; }`)
to return a pointer, not void -- direct positive evidence this whole function
is non-void per CLAUDE.md's tail-call caution. `GetClass6D4E8Methods` is still
uncarved (`asm/code_179d8.s`) but is declared elsewhere in the repo
(`code_179d8_h.c`, `code_179d8_o.c`) as returning a table pointer too, under
each unit's own independent local type -- this report follows the same
"multiple independent local views" convention and declares it `void *` here.

## Final body

```c
extern s32 D_8008A84C;
extern void *GetVabDriverMethods(void);
extern void *GetClass6D4E8Methods(void);

void *func_80026CAC(void) {
    if (D_8008A84C == 0x23) {
        return GetVabDriverMethods();
    } else {
        return GetClass6D4E8Methods();
    }
}
```

## Proposed learning

**A second shared idiom in this unit, alongside the six-function
`if (D_8008A84C == 0x13) return func(); return N;` family
(`func_80026E0C.md`):** an `if`/`else` mode dispatch where BOTH arms
tail-call a function and neither sets an explicit constant. Three functions
in this unit share this exact shape --  `func_80026CAC` (mode `0x23`),
`func_80026FAC` and `func_80026FE8` (both mode `0x13`, different callees).
Per CLAUDE.md's caution, a byte match here proves nothing about void-ness on
its own, but `func_8002C478` (the `func_80026FE8` else-arm) is independently
confirmed non-void (`s32 func_8002C478(void) { return 0; }` in
`code_179d8_e.c`), so treating all three as `s32`/`void *`-returning tail
calls is not a guess -- it is the only reading consistent with a callee whose
real return type is already known.
