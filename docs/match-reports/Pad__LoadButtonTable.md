# Pad__LoadButtonTable

> Renamed from `func_80025E1C` on 2026-09-24 (tools/rename.py). Address 0x80025e1c.

**Unit:** `src/class_16334.c` (runner ALPHA, `runner/alpha`)
**Status:** MATCHED (30/30 words, full build verified byte-exact)
**Vtable slot:** `gPadMethods+0x50` (`PadMethods.loadButtonTable`)

## Context

This class's method table (`gPadMethods`, 21 slots, see `tools/classtable.py
gPadMethods`) inherits 14 slots verbatim from `BASICCLASS_METHODS`
(`gBasicClassMethods`, returned by `Get_vtable_BasicClass`) and adds 7 of its own at
`+0x38..+0x54`. `Pad__LoadButtonTable` is the slot at `+0x50`.

It copies a 0x40-byte (16-word) block from `sDefaultButtonMasks` into the runtime
global `sButtonMasks`. `sDefaultButtonMasks`'s vram falls inside the `psyq_15d04`
rodata blob (file offset 0xF64, between the `0xF28` and `0xFA4` rodata
segment boundaries in `config/splat.slps01556.lsdde.yaml`) -- i.e. this
function copies a Psy-Q-owned constant table into game-owned bss. Given the
surrounding class also drives `func_80025EAC`/`func_80025EFC`/`func_80025F2C`
(which disassemble as part of `psyq_PadInit`, immediately adjacent in the
yaml at file offset 0x166ac) and does edge-detected held/pressed/released
button masking (see `Pad__UpdateMasks`), the working hypothesis for this whole
unit is a Pad/controller wrapper class, and `sDefaultButtonMasks`/`sButtonMasks` are the
16 canonical digital-button bit masks. See `include/class_16334.h` for the
full writeup and struct layout.

## Final C

```c
typedef struct { u32 w[16]; } Block64;
extern Block64 sDefaultButtonMasks;
extern u32 sButtonMasks[16];

void Pad__LoadButtonTable(void) {
    Block64 local;
    u32 *dst;
    u32 *src;
    s32 i;

    dst = sButtonMasks;
    local = sDefaultButtonMasks;
    i = 0;
    src = local.w;
    for (; i < 16; i++) {
        *dst++ = *src++;
    }
}
```

## Derivation notes

- The first half of the retail body copies `sDefaultButtonMasks` into a stack-local
  0x40-byte buffer using GCC's inlined block-move codegen (four registers,
  4 words per iteration) rather than a per-word loop. That only happens for
  a **struct assignment** (`local = sDefaultButtonMasks;`), not a `for` loop over
  `u32[16]` -- an indexed-loop translation produces a single-word-per-iteration
  copy and is 4 words *shorter*, which also shows up as address drift in
  every function after it. This was the first (wrong) attempt; see below.
- The second half is a real source-level `for` loop copying word-by-word from
  the local struct into `sButtonMasks`, using two independently-incrementing
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
2. Introduced `Block64` and `local = sDefaultButtonMasks;` struct assignment (matches
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

## Naming

**Tier A.** Vtable slot `+0x50`, already named `loadButtonTable`. Copies
Sony's own default 16-entry digital-button mask table (`sDefaultButtonMasks`, inside
the `psyq_15d04` rodata blob) into this unit's runtime copy (`sButtonMasks`)
-- mechanics fully derived in this report, matching the slot's existing name
exactly.

## Naming of `sDefaultButtonMasks` (round 101, track 7)

Renamed from `D_80010764` (`tools/rename.py`). **Tier A** for what it is: the
16 read-only words this function copies into `sButtonMasks`, and nothing else
reads them (`grep` over `src/` and `asm/`). Its values are libetc's pad masks
(`PADLup` 0x1000 ... `PADstart` 0x0800) in `enum PadButton` order, so it is
the default contents of the table `Pad__DispatchEvents` scans. `s` prefix,
not `g`: it is this unit's own data, from the link order below.

### Correction: the table is class_16334's rodata, not Psy-Q's

This report and `include/class_16334.h` used to call the table "Psy-Q's own
... inside the `psyq_15d04` rodata blob". The link order says otherwise.
Rodata follows the code objects' order: `libetc/intr_dma` .rdata at 0xF28 and
`libetc/vsync` .rdata at 0xF54 are placed, the next code objects are
`libc2/puts` (no .rdata; its only data is 7 bytes of .sdata),
`class_16334`, `libetc/pad` (the linked 3.5 `pad.o`: .text and .bss only, no
.rdata) and `libapi/a22`, and the next rodata slot, 0xFA4, is
`GameApplicationFileResource`'s (the unit after them). The only object in
that stretch able to own 0x40 bytes of .rodata at 0xF64 is this unit. The yaml
line for 0xF64 still says "owner not placed yet (libetc intr?)"; proposed to
the head, not edited (runners do not edit the yaml).
