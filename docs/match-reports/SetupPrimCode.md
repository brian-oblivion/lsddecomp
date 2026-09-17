> Renamed from `func_8001934C` on 2026-09-17 (tools/rename.py). Address 0x8001934c.

# SetupPrimCode — MATCHED (29/29 words)

Unit: `src/code_8220_b.c`. Not previously declared in `include/code_8220.h`;
called only from the still-`INCLUDE_ASM` giant `func_80018464` (13 call
sites), so its two arguments' real struct types are unknown outside this
function's own body. Treated both as raw byte-offset accesses rather than
guessing an unverified struct layout.

`arg0`: byte flags at `+0x3` and `+0x7`. `arg1`: `s32` at `+0x1C`, byte
outputs at `+0x14`/`+0x15`.

## Final source

```c
void SetupPrimCode(void *arg0, void *arg1)
{
    u8 *a = (u8 *)arg0;

    if (*(s32 *)((u8 *)arg1 + 0x1C) != 0) {
        a[7] = a[7] | 0x2;
    } else {
        a[7] = a[7] & 0xFD;
    }

    if (D_8008E248 != 0) {
        a[7] = a[7] | 0x1;
    } else {
        a[7] = a[7] & 0xFE;
    }

    *((u8 *)arg1 + 0x14) = a[3];
    *((u8 *)arg1 + 0x15) = a[7];
}
```

`D_8008E248` (a global `s32` flag, read-only from this function) is now
declared `extern` in `include/code_8220.h`; no other symbol name evidence
was available for it.

## Notes

First attempt cached the two bit-set/clear results in a shared local `u8
flags` variable reused between the read-modify-write for each of the two
independent flag bits (12/29, address-drifted). Retail treats the two
`a[7] = a[7] | BIT` / `a[7] = a[7] & ~BIT` updates as two fully independent
statements, each reloading `a[7]` fresh from memory rather than reading it
from a variable holding the previous statement's result — dropping the
shared temp and writing each update as its own direct
`a[7] = a[7] | BIT;`/`a[7] = a[7] & ~BIT;` matched byte-exact on the second
attempt.

### Proposed learning

Two back-to-back "set-or-clear one bit of the same byte field, based on an
independent condition" updates are two independent statements in the
source, each of the form `x->flags = x->flags | BIT;` /
`x->flags = x->flags & ~BIT;` — do not factor them through a shared local
that holds "the flags byte's new value" across both updates. Retail reloads
the field from memory at the start of each one rather than carrying the
previous statement's result forward in a register/variable, even though the
two updates are adjacent and touch the same byte. (`SetupPrimCode`, 12/29 ->
29/29.)
