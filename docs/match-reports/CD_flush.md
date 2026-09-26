# CD_flush

> Renamed from `func_8002A510` on 2026-09-23 (tools/rename.py). Address 0x8002a510.

**Unit:** code_179d8_g · **Size:** 58 words · **Status:** MATCHED (58/58 words)

## What it does

Drains the link driver's status byte (`*D_8006D8CC & 7`) by repeatedly
poking mode byte 1 and status 7 into the staging pointers until it clears,
then runs the same "close port" tail seen in `CD_init`'s middle and
`cd_read_retry`'s tail: clear `D_8006D8DA`, mirror it into `D_8006D8D9`,
clear `D_8006D61C`, set `D_8006D8D8 = 2`, zero `*D_8006D8C0`/`*D_8006D8CC`,
and program `*D_8006D8D0 = 0x1325`.

## The C

```c
void CD_flush(void)
{
    volatile u8 *q;

    *D_8006D8C0 = 1;
    while (*D_8006D8CC & 7) {
        *D_8006D8C0 = 1;
        *D_8006D8CC = 7;
        *D_8006D8C8 = 7;
    }

    D_8006D8DA = 0;
    q = &D_8006D8D9;
    D_8006D61C = 0;
    *q = D_8006D8DA;
    __asm__("");
    D_8006D8D8 = 2;
    *D_8006D8C0 = 0;
    *D_8006D8CC = 0;
    *D_8006D8D0 = 0x1325;
}
```

## Notable findings

**Retail's disassembly ends with `jr $ra` / `nop`, not a zeroed `$v0`.** The
`INCLUDE_ASM` body never sets `$v0` to 0 (or anything) before returning --
the value at that point is whatever `*D_8006D8D0 = 0x1325`'s store left
sitting in `$v0` (0x1325), and the delay slot is a plain `nop`. That is only
consistent with a **`void` return type**, not `s32 { ...; return 0; }` -- a
`return 0;` would need an explicit `li $v0, 0`. Declaring it `s32` and
returning 0 compiled a full match everywhere EXCEPT the final delay slot
(`move v0,zero` vs retail's `nop`), which was the tell.

**`D_8006D8DA` needed `volatile` for the same reason `D_8006D8EC` did in
`CD_readm`**: without it, `D_8006D8D9 = D_8006D8DA;` right after
`D_8006D8DA = 0;` constant-propagates to `D_8006D8D9 = 0;` and drops the
reload retail's disassembly shows.

**The "close port" tail's scheduling needed a bare `__asm__("")` barrier**
between `*q = D_8006D8DA;` and `D_8006D8D8 = 2;` to reproduce retail's
instruction order. Without it, GCC hoists the *later* `*D_8006D8C0 = 0;`
statement's address load earlier (ahead of `D_8006D8D8 = 2;`), which is
functionally identical but is not what retail's bytes show. This is a pure
scheduling difference -- confirmed by removing the barrier: the register each
value lives in stays the same, only instruction ORDER changes, which is
exactly the permitted use per `docs/MATCHING-GUIDE.md`. It also happened to
fix an unrelated-looking register swap earlier in the function (the loop's
`$v1`/`$a0` constant-load order) as a side effect -- one scheduling decision
downstream evidently influenced register choice upstream in this build.

A local `volatile u8 *q = &D_8006D8D9;`, computed as an ordinary top-level
local (not nested in a block), reproduces retail's *unfolded* address
computation for `D_8006D8D9` (`lui`+`addiu` into a real register, rather than
the folded `lui $at`/`sw ...(at)` form GCC otherwise prefers for the plain
`D_8006D8D9 = ...;` when it doesn't need to keep the address around). This is
the same lever documented in `CD_readm`'s report.

### Proposed learning

**A trailing `nop` in retail's delay slot where your compiled function's
return statement would need to materialize a value is a `void`-vs-`s32`
signature tell**, not just for tail-call wrappers (already documented) but
for functions with real bodies too: if the function's last live use of `$v0`
was for something other than the literal return value, and retail's delay
slot is a bare `nop`, the function almost certainly returns `void`.

## asm sites

Round 89 (runner delta, track 5 `asm-sites`): the bare `__asm__("")` after
`*q = D_8006D8DA;` is **justified** and now commented at the site. Measured by
deleting it alone (35 bytes of the image differ; `funcdiff` cannot score this
function because its symbol is absolute, so the evidence is `objdump -d -r` of
`build/src/code_179d8_g.c.o` with and without it): without the barrier the
`lw` of the `D_8006D8C0` pointer and the `li 2` / `sb` to `D_8006D8D8` are
hoisted above the `sb zero` to `D_8006D61C` and the `sb` through `q` to
`D_8006D8D9`; retail performs those two stores first. As knock-on effects the
pointer lands in `a0` instead of `v1` and the loop's `li v1,0x7` / `li a0,0x1`
pair swaps order. The primary change is instruction order.
