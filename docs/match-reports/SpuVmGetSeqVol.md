# SpuVmGetSeqVol -- MATCH (28/28 words)

> Renamed from `func_80030584` on 2026-09-23 (tools/rename.py). Address 0x80030584.

Unit `code_179d8_j`, round 22 (2026-09-06). Third sibling of
`SpuVmGetSeqLVol`/`SpuVmGetSeqRVol` -- the "get both fields" form: writes both
`+0x74` and `+0x76` through output pointers, and returns the full packed
`p0` (read back from `D_8008EA22` rather than the parameter register
directly -- confirmed via `asm-differ` that retail genuinely reloads it
rather than reusing `$a0`).

## The residue, and what closed it

Two independent issues, both from the same family as the two sibling
reports:

1. **The `sra`/`srl` instruction-choice issue** (see `SpuVmGetSeqLVol`'s
   report) -- fixed the same way, by writing the high-byte extraction
   inline at both use sites rather than through a named `recIdx` local.
2. **`D_8008EA22`'s address needs to be CACHED as a real pointer, not
   re-derived.** Retail computes `&D_8008EA22` once (`lui`+`addiu` into
   `$a3`) and reuses that address for BOTH the store and the final reload
   (`lh $v0, 0($a3)`). Writing `D_8008EA22 = p0; ... return D_8008EA22;`
   as two separate global accesses lets GCC re-derive the short
   `lui`+op form independently each time (no `addiu`, no reuse) --
   right byte total, wrong shape. Introducing an explicit `s16 *cur = (s16
   *) &D_8008EA22;` local forces GCC to materialize the address once and
   reuse it, matching retail's `addiu`+reuse shape exactly.

   **The pointer must be typed `s16 *`, not `u16 *`.** `D_8008EA22` is
   declared `u16` elsewhere in this file (used by the already-matched
   `func_800319B4`), and a `u16 *` reload emits `lhu` (zero-extend) where
   retail's return path uses `lh` (sign-extend). Since the return type is
   `s32`, retail's source is reading the global back through a route that
   sign-extends -- a local cast to `s16 *` reproduces that without needing
   to touch the global's own declared type (which stays `u16` for the
   sibling that only ever stores to it).

## Final source

```c
s32 SpuVmGetSeqVol(s32 p0, s16 *out1, s16 *out2)
{
    Entry90902E8 *tbl = _ss_score[(u8) p0];
    s16 *cur = (s16 *) &D_8008EA22;

    *cur = (s16) p0;
    *out1 = tbl[(p0 & 0xFF00) >> 8].unk74;
    *out2 = tbl[(p0 & 0xFF00) >> 8].unk76;
    return *cur;
}
```

(`Entry90902E8` typedef and `_ss_score` extern are declared once, above
this function in the unit -- both leading `s16` fields at `+0x74`/`+0x76`.
Retail reuses ONE computed entry-pointer for both `unk74`/`unk76` reads;
writing the same `(p0 & 0xFF00) >> 8` index expression at both textual
sites was sufficient for GCC to CSE it into a single address computation,
same as the already-matched `SsUtGetDetVVol`'s double-field read of
`D_8006DAD4[idx]`.)

### Proposed learning

Add alongside the `sra`/`srl` finding: **a global reloaded later in the
same function needs its ADDRESS cached in an explicit local pointer to get
retail's `addiu`-then-reuse shape** -- two separate `global = x; ... return
global;` statements let GCC re-derive the short two-instruction form
independently each time, which is the right byte count but the wrong
instruction shape (no `addiu`, no register reuse). Cast the pointer to the
signedness the RETURN actually needs, independent of the global's own
declared type.
