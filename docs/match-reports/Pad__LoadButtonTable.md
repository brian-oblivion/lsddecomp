# Pad__LoadButtonTable

> Renamed from `func_80025E1C` on 2026-09-24 (tools/rename.py). Address 0x80025e1c.

**Unit:** `src/class_16334.c` (runner ALPHA, `runner/alpha`)
**Status:** MATCHED (30/30 words, full build verified byte-exact)
**Vtable slot:** `D_8006D370+0x50` (`PadMethods.loadButtonTable`)

## Context

This class's method table (`D_8006D370`, 21 slots, see `tools/classtable.py
D_8006D370`) inherits 14 slots verbatim from `BASICCLASS_METHODS`
(`D_8006B58C`, returned by `Get_vtable_BasicClass`) and adds 7 of its own at
`+0x38..+0x54`. `Pad__LoadButtonTable` is the slot at `+0x50`.

It copies a 0x40-byte (16-word) block from `D_80010764` into the runtime
global `D_8008B388`. `D_80010764`'s vram falls inside the `psyq_15d04`
rodata blob (file offset 0xF64, between the `0xF28` and `0xFA4` rodata
segment boundaries in `config/splat.slps01556.lsdde.yaml`) -- i.e. this
function copies a Psy-Q-owned constant table into game-owned bss. Given the
surrounding class also drives `func_80025EAC`/`func_80025EFC`/`func_80025F2C`
(which disassemble as part of `psyq_PadInit`, immediately adjacent in the
yaml at file offset 0x166ac) and does edge-detected held/pressed/released
button masking (see `Pad__UpdateMasks`), the working hypothesis for this whole
unit is a Pad/controller wrapper class, and `D_80010764`/`D_8008B388` are the
16 canonical digital-button bit masks. See `include/class_16334.h` for the
full writeup and struct layout.

## Final C

```c
typedef struct { u32 w[16]; } Block64;
extern Block64 D_80010764;
extern u32 D_8008B388[16];

void Pad__LoadButtonTable(void) {
    Block64 local;
    u32 *dst;
    u32 *src;
    s32 i;

    dst = D_8008B388;
    local = D_80010764;
    i = 0;
    src = local.w;
    for (; i < 16; i++) {
        *dst++ = *src++;
    }
}
```

## Derivation notes

- The first half of the retail body copies `D_80010764` into a stack-local
  0x40-byte buffer using GCC's inlined block-move codegen (four registers,
  4 words per iteration) rather than a per-word loop. That only happens for
  a **struct assignment** (`local = D_80010764;`), not a `for` loop over
  `u32[16]` -- an indexed-loop translation produces a single-word-per-iteration
  copy and is 4 words *shorter*, which also shows up as address drift in
  every function after it. This was the first (wrong) attempt; see below.
- The second half is a real source-level `for` loop copying word-by-word from
  the local struct into `D_8008B388`, using two independently-incrementing
  pointers (`v1`/read, `t0`/write) plus a separate loop counter (`a0`) --
  i.e. the source uses an explicit `for (i = 0; i < 16; i++) *dst++ = *src++;`
  rather than pointer-limit comparison.
- Register identity: with `i` declared in the same declaration list as `dst`/
  `src` and initialized via a normal `for (i = 0; ...)` header, GCC put the
  loop counter in `v1` and the source pointer in `a0` -- backwards from
  retail (`a0`=counter, `v1`=source pointer). Moving `i = 0;` to its own
  statement **before** `src = local.w;`, and turning the for-loop into
  `for (; i < 16; i++)` (init already done), flipped the allocation to match
  retail exactly. This is a pure reshaping fix (statement order), not a
  register pin -- no `asm` or operand constraint was used.

## Attempt log (abbreviated)

1. Two independent `for` loops over `u32[16]` (no struct type) -- wrong
   instruction count (26 words vs retail's 30) and address drift in every
   later function. Diagnosed via `asm-differ`: retail's first loop batches
   4 words/iteration, mine did 1.
2. Introduced `Block64` and `local = D_80010764;` struct assignment (matches
   GCC's block-move codegen) plus a pointer-based second loop -- sizes
   matched (30/30 words compared) but two registers were swapped in the
   second loop (`a0`/`v1`).
3. Reordered the `i = 0;` initializer ahead of `src = local.w;` and changed
   the `for` to `for (; i < 16; i++)` -- full match.

### Proposed learning

For a source-owned block copy from an external (SDK/rodata) constant into a
local buffer of matching size, prefer a **whole-struct assignment**
(`typedef struct { u32 w[N]; } BlockNN;`) over an indexed `for` loop when the
retail disassembly shows a first loop batching 4 words per iteration with
four distinct temp registers (`v0,v1,a0,a1` cycling) -- that shape is GCC
2.6.3's inlined block-move for a struct/array-of-structs assignment, not
achievable from a per-word `for` loop, and an indexed loop will be both the
wrong instruction count *and* will silently shift every following function's
address (funcdiff's "differs OUTSIDE this range" warning is the tell).
