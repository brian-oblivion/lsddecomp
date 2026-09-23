# SpuVmGetSeqRVol -- MATCH (21/21 words)

> Renamed from `func_80030648` on 2026-09-23 (tools/rename.py). Address 0x80030648.

Unit `code_179d8_j`, round 22 (2026-09-06). Sibling of `SpuVmGetSeqLVol` (see
that report for the shared `D_800902E8[screen][slot]` shape and the
inline-vs-named `sra`/`srl` finding). This one reads the OTHER leading
field (`+0x76` instead of `+0x74`) and stores the FULL packed `p0` into
`D_8008EA22` (not just the `screen` byte, unlike its sibling).

## The residue, and what closed it

Same class as `SpuVmGetSeqLVol`: right instruction count, wrong order. GCC
initially hoisted the `D_8008EA22` store immediately after the table
lookup and allocated `channel` into `$v1` for the whole function, where
retail allocates it into `$v0` and defers the store. A bare `__asm__("")`
scheduling barrier placed right after the `tbl` lookup (before the store)
was sufficient -- no named-intermediate rework needed here since `p0`
itself (not a derived byte) is what's stored, so there's no dual-cast
duplication to work around.

## Final source

```c
s32 SpuVmGetSeqRVol(s32 p0)
{
    Entry90902E8 *tbl = D_800902E8[(u8) p0];

    __asm__("");
    D_8008EA22 = p0;
    return tbl[(p0 & 0xFF00) >> 8].unk76;
}
```

(`Entry90902E8` is declared once, above `SpuVmGetSeqVol` in the unit --
see that function's or `SpuVmGetSeqLVol`'s report for the full typedef.)
