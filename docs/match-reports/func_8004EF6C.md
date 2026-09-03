# func_8004EF6C

**Unit:** class_3bb8c_f · **Size:** 240 words (0x3C0) · **Status:** STALL
— predicted-hard, screened and read but not attempted in C given scale;
no C written, `INCLUDE_ASM` untouched

## Why this one was not attempted

This is both the largest function in this unit's queue (240 words — the
next largest, `func_8004F8A4`, is 77) and one of the two 9-register-
saturated functions the round's register census flagged (`func_8004EEA0`
= 9, `func_8004EF6C` = 9). `func_8004EEA0` — the smaller (51-word) sibling
that calls THIS function in a retry loop — was attempted first and hit
CLAUDE.md's documented zero-drift register-PERMUTATION residue (see its
own report): every branch and field settled, exact length, but a full
9-value bijection permuted relative to retail's own, and per that class's
established behavior in this project (`func_8004C93C`, 7 reshaping
variants, zero movement), reshaping does not resolve it.

Given that finding, sinking a full multi-hundred-instruction transcription
effort into this function's OWN body — four times the size, same register
saturation, same class of hazard — was judged low-expected-value against
this round's attempt budget, which covers 20 functions across this unit.
This is a scope decision, not a failed attempt: no C was written, so
there is no near-miss body to preserve, and `funcdiff`/`asm-differ` were
not run against it.

## What it does (read from the disassembly, not verified by compiling)

`s32 func_8004EF6C(TaskObjF *self, s32 a1, s32 handle, s32 flag, s32
arg5, s32 arg6, s32 arg7)` (signature per its one caller,
`func_8004EEA0`). Builds a memory-card path via `func_8004F32C` (this
unit, matched) and opens it (`func_80050908`), computes a size from
`arg5`/`arg7` (bit-shift arithmetic against `0x2000`/`0x1000`-ish
boundaries), and — if the open failed — logs an error
(`func_80012C20` with a format string at `D_80011530`) and returns 0.
On success it seeks (`func_800508F8`/`func_80050938` with mode 2),
allocates a 0x200-byte request buffer, writes a small fixed header into
it (`'S'`, `'C'`, then two computed bytes) followed by a filename
(`strcpy`), then whole-struct-copies roughly 0x100-0x180 bytes from
`self`'s own fields into the freshly-built buffer in four `0x40`-byte
chunks — each chunk independently choosing between an ALIGNED
(`lw`/`sw`) and an UNALIGNED (`lwl`/`lwr`/`swl`/`swr`) copy path based on
a runtime `(src | dst) & 3` check, repeated per chunk rather than decided
once for the whole transfer. Finally it submits two `func_80013488` calls
(read-request submission, by the signature shape: address, position,
size) — one for a small computed header-sized read, one for the full
payload sized from `arg7` — frees the temporary request buffer, closes
the handle (`func_800508F8`), and returns 1.

This reads as a **CD/memory-card streaming request builder**: the
per-chunk alignment branching (four separate aligned/unaligned checks
rather than one for the whole copy) suggests the SOURCE struct-copies four
separate SUB-STRUCTS in sequence (matching a `Descriptor`-style compound
object build), not one large `memcpy`.

## Screening done (per the round's standing checks)

```
grep -n 'gp_rel' asm/nonmatchings/class_3bb8c_f/func_8004EF6C.s        # no hits
grep -n 'addiu *$at, *$at, *%lo' asm/nonmatchings/class_3bb8c_f/func_8004EF6C.s  # no hits
grep -oE 'sw +\$(s[0-7]|fp),' asm/nonmatchings/class_3bb8c_f/func_8004EF6C.s | sort -u | wc -l  # 9 ($fp + $s0-$s7, fully saturated)
```
Clean of both open toolchain blockers; one prologue, no jump table, no
`alabel`.

## For whoever picks this up next

- Do NOT re-run the register screen — it is already confirmed 9/9
  saturated above.
- Given `func_8004EEA0`'s finding, budget for this being ANOTHER
  permutation-class stall rather than a straightforward match; a
  worthwhile first move is a `--sig`-seeded `m2c` pass to get real field
  names for the four `self`-offset struct copies (`self+0x40`,
  `self+0xC0`, `self+0x140`(?), `self+0x1C0`(?) by the pattern) before
  attempting a full transcription, since establishing that struct
  correctly is useful even if the function itself ultimately stalls.
- `func_80013488`'s signature (read-request submission) is established
  only by this function's own call sites; typing it accurately here would
  be new ground, not yet in `include/class_3bb8c.h`.
