# Class65650__OnClass6EF50Notify

> Renamed from `func_80065B80` on 2026-09-24 (tools/rename.py). Address 0x80065b80.

**Unit:** code_55dd4 · **Size:** 29 words (0x74 bytes) · **Status:** MATCHED
(29/29 words, whole-image `./build-and-verify.sh` green)

## What it does

Dispatches on a small integer `val`: if `2`, calls the class's own vtable
slot `+0x108` (`Class65650__Tick`) on `self`; if `4`, calls slot `+0x004`
(`BasicClass__Release`, inherited). Both are checked independently (two
separate `if`s, not `if/else if`), matching the two independent `bne`
guards in the disassembly.

```c
void Class65650__OnClass6EF50Notify(Class65650 *self, void *arg1, s32 val)
{
    if (val == 2) {
        self->methods->slot108(self);
    }
    if (val == 4) {
        self->methods->slot04(self);
    }
}
```

## The residue: a parameter that must exist but must not be *used*

First attempt wrote both calls as `slot108(self, arg1)` /
`slot04(self, arg1)`, forwarding this function's own second parameter —
reasoning by analogy with `Class65650__SetupModelData`'s forwarding shape. That compiled
29 words too **long** (30 vs 29) and every word from the prologue on
differed: with `arg1` referenced after a call (the first `if`'s call sits
between the two uses), GCC has to assume the callee might clobber `$a1` and
spills it into a saved register (`$s1`) across the call — an extra
`sw`/`lw` pair neither branch in retail has.

Retail's own bytes prove the opposite is true: the **first** call
(`slot108`) doesn't set up `$a0` OR `$a1` in its delay slot at all (a bare
`nop`) — it relies on both still holding their function-entry values,
including `$a1`, un-clobbered by anything in between. That is only
consistent with the call site **never naming a second argument** — not
"the value happens to survive", but "the C source never asked the compiler
to track it as live here". Removing `arg1` from both call sites (typing
`slot108`/`slot04` as taking only `self` in `include/code_55dd4.h`) is what
matched: with no reference to `arg1` after the call, GCC never spills it,
and `$a1` is left holding whatever it already had — which, from the
*outside*, looks like "forwarded", but which the source never actually
says.

`arg1` is kept in `Class65650__OnClass6EF50Notify`'s own signature (unused) purely because
retail's caller-side convention still passes something in `$a1` here and a
future caller in this same slot family may turn out to need it — but this
function's *body* must not reference it.

### Proposed learning

Refines the "forward an argument nobody names" pattern from
`Class65650__func_800661D4.md`: forwarding only reproduces retail when the forwarded
value is used **at most once, with no intervening call** (or, as here, used
zero times but still physically present in the register at entry). The
moment a source explicitly re-uses a parameter *after* a call that might
clobber it, GCC 2.6.3 must spill it to a saved register — so if retail's
disassembly shows a later call relying on an argument register that
survived an EARLIER call untouched, the source cannot have named that
argument at either call site. Check whether the surviving register's use
is genuinely referenced by the C, or merely a side effect of the callee
never being asked to preserve it.

## Naming

Round 75 (charlie), track 3.

- `Class65650__OnClass6EF50Notify` (was `func_80065B80`), tier B. Occupies +0x098, which Class6B5CC__OnNotify (code_d294.c) dispatches to when the sender's header tag is 5 (TAG_CLASS6EF50, D_8006EF50). Code 2 calls tick (+0x108), code 4 calls release. Mechanics known; what Class6EF50 is (the tag-5 companion held in BaseObjO companion2) is not, hence B. Entity overrides this slot as Entity__Update.
