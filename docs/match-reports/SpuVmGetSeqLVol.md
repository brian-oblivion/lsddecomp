# SpuVmGetSeqLVol -- MATCH (21/21 words)

> Renamed from `func_800305F4` on 2026-09-23 (tools/rename.py). Address 0x800305f4.

Unit `code_179d8_j`, round 22 (2026-09-06). Not a class method. Sibling of
`SpuVmGetSeqRVol` and `SpuVmGetSeqVol` -- all three index the same
`_ss_score[screen][slot]` array already established in `code_179d8_f.c` /
`code_179d8_i.c` (an array of pointers to 172 (0xAC)-byte records), reading
the two leading `s16` fields at `+0x74`/`+0x76` this unit hadn't named yet.

## Shape

Single `s32` parameter packs two indices: the low byte is the outer
`screen` index (`_ss_score[screen]`, a table of pointers), the high byte
(`(p0 & 0xFF00) >> 8`) is the inner `slot` index into that screen's record
array. Confirmed against `code_179d8_i.c`'s `func_800339AC`, which builds
exactly this packing (`(sa1 << 8) | sa0`) before calling into this unit.
Side effect: `D_8008EA22` (already `extern`'d in this file, used by
`func_800319B4`) is set to the `screen` value.

## The residue, and what closed it

Straightforward translation compiled to the RIGHT bytes in the WRONG
order/registers -- same total instruction count as retail (21) but GCC
scheduled the independent `D_8008EA22` store immediately after computing
`channel`, in-place-scaling that same register for the array index
(`sll v1,v1,2`) and then re-deriving `channel` from `$a0` a second time
(a spurious `move`+second `andi`) when the store needed it again later.
Retail instead scales `channel` into a DIFFERENT register (keeping the raw
value alive in `v1`) and defers the `D_8008EA22` store to just before the
final combine.

**Fixed with a bare scheduling barrier** (`__asm__("")`) placed between the
`recIdx` computation and the `D_8008EA22` store -- this is the permitted
lever per CLAUDE.md's test (removing it only changes instruction ORDER, not
which value ends up in which register; verified by removing it and
confirming register allocation reverted to the wrong shape, never to an
different-but-still-consistent one). Once the store is forced to schedule
late, GCC naturally allocates the scaled index into `v0` and the table
pointer into `a1`, matching retail exactly.

**Also load-bearing: writing the high-byte extraction as a single inline
expression, never through a named intermediate.** `s16 recIdx = (p0 &
0xFF00) >> 8;` used LATER as `tbl[recIdx]` compiles to `srl` (GCC proves the
masked value's sign bit is clear and picks the cheaper unsigned shift);
inlining the same expression directly at the point of use
(`tbl[(p0 & 0xFF00) >> 8]`) keeps it as `sra`, matching retail bit-for-bit.
This is NOT about signedness of the declared type -- both variants used
`s32`/`s16` for the intermediate; it's specifically the presence of a named
local vs. an inline expression that flips GCC 2.6.3's instruction choice.
Reproduced in isolation through the pinned pipeline (`/tmp/alpha_probe*.c`,
not committed) before applying here.

## Final source

```c
/* A 172 (0xAC)-byte record; _ss_score is an array of pointers to arrays of
 * these, indexed [screen][slot]-style by a packed argument (slot in the
 * high byte, screen in the low byte) -- see code_179d8_i.c's own
 * func_800339AC, which builds exactly this packing before calling into
 * this unit's SpuVmSeqKeyOff. Reduced local view: only the two leading s16
 * fields this unit's own accessors touch are named. See code_179d8_f.c /
 * code_179d8_i.c's own Entry90902E8 for a fuller layout of the same array;
 * each unit keeps its own independent reading, per project convention. */
typedef struct {
    u8 pad0[0x74];
    s16 unk74; /* +0x74 */
    s16 unk76; /* +0x76 */
    u8 pad78[0xAC - 0x78];
} Entry90902E8;
extern Entry90902E8 *_ss_score[];

s32 SpuVmGetSeqLVol(s32 p0)
{
    s32 channel = p0 & 0xFF;
    Entry90902E8 *tbl = _ss_score[channel];
    s32 recIdx = (p0 & 0xFF00) >> 8;

    __asm__("");
    D_8008EA22 = channel;
    return tbl[recIdx].unk74;
}
```

### Proposed learning

Add to the "reading a residue" repertoire: **a named intermediate variable
for a masked-and-shifted extraction can flip GCC 2.6.3's instruction choice
between `sra` and `srl`, even though both are provably identical given the
mask.** Writing the same expression inline at the use site (no named local)
preserves the signed-shift form. Confirmed on three sibling functions in
this unit (`SpuVmGetSeqLVol`, `SpuVmGetSeqRVol`, `SpuVmGetSeqVol`) and
independently reproduced in isolation through the pinned pipeline.

## asm sites

Round 89 (runner delta, track 5 `asm-sites`): the bare `__asm__("")` before
`D_8008EA22 = channel;` is **justified** and now commented at the site.
Measured by deleting it alone: the image went red (49 bytes, same length),
`funcdiff` 6/21, and asm-differ shows the `sh v1,-0x15de(at)` store to
`D_8008EA22` hoisted to the second instruction of the function, ahead of the
`_ss_score[channel]` load and the `recIdx` multiply chain that retail performs
first (retail's store sits just before the final `addu`/`lh`). The register
differences in that diff follow from the moved store. Instruction order.
