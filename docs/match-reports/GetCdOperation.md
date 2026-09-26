# GetCdOperation

> Renamed from `func_80027EE0` on 2026-09-17 (tools/rename.py). Address 0x80027ee0.

**Unit:** code_179d8_q (fresh carve) · **Size:** 3 instructions · **Status:** MATCHED (3/3 words)

## What this function does

A plain `$gp`-relative getter, no arguments. Reads the scalar `s32` global
`gCdOperation` (in `.sdata`, zero-initialized) and returns it.

## The C

```c
extern s32 gCdOperation;

s32 GetCdOperation(void)
{
    return gCdOperation;
}
```

## Provenance

round 45 (2026-09-15), runner echo, unit code_179d8_q (fresh carve). See
IsCdBusy.md for the sibling-accessor context.

## Naming

Round 51 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_80027EE0` | `GetCdOperation` | A |
| `D_8008A874` | `gCdOperation` | A |

**Evidence.** The global has exactly one writer that sets it non-zero:
`StartCdOperation(arg0, arg1)` (code_179d8_r), which stores `arg0` into it. Its
five call sites are the five methods of this class in `code_179d8_s`, each
passing a distinct constant -- `CdDriver__Close` 0 (close), `CdDriver__Open` 1
(open by name), `CdDriver__Seek` 2 (seek), `CdDriver__Read` 3 (read),
`CdDriver__LoadFile` 4 (load file). `ResetCdStateMachine` (reset) clears it. A per-method
constant written at operation start and cleared at the end is an
"operation in progress" code, which is what the name says.

### Proposed learning

**Name a getter from the global's WRITERS, not from its readers.** Four
identically-shaped three-instruction getters sit in a row in this unit and
carry no information at all in themselves; three of the four became tier-A
names purely because the writing side is a single function with call sites
that disagree usefully (a distinct constant per method). The fourth
(`GetCdDriverMode`'s second value) has one writer that stores a caller's
argument verbatim, and stayed unnamed for exactly that reason. The
discriminator is not how much the getter is called, it is whether the write
sites vary in a way that means something.
