# GetRequestedFileMethods -- MATCHED (4/4 words), round 82

> Renamed from `GetClass6EED8Methods` on 2026-09-26 (tools/rename.py). Address 0x800423f0.

> Renamed from `Get_vtable_D8006EED8` on 2026-09-26 (tools/rename.py). Address 0x800423f0.

> Renamed from `func_800423F0` on 2026-09-25 (tools/rename.py). Address 0x800423f0.

Round 82, runner alpha (second re-staffed slot of the round). Unit `src/graphics/sprite.c`. Fresh ground (carved in FINISHING-PLAN revision 18), no prior attempt, no report before this one.

- **Where:** not in any method table (`tools/classtable.py`).
- **What:** Returns the gRequestedFileMethods method table.
- **Result:** byte-exact; 4/4 words, 0 insertions / 0 deletions, whole-image SHA1 green (`./build-and-verify.sh` OK). First build.
- **Types:** local views and `extern s32 D_XXXXXXXX[];` table declarations live in the unit; no shared header was touched.

## Source

```c
/* Returns the gRequestedFileMethods method table. */
void *GetRequestedFileMethods(void) {
    return gRequestedFileMethods;
}
```

## Naming

- `GetRequestedFileMethods` -- tier A. Table getter ("return gRequestedFileMethods;").

## Track 4 (2026-09-26, round 87, delta)

Class id 0xB03 is unified as `RequestedFile` in `include/RequestedFile.h`
(FILERESOURCE_SLOTS/FIELDS, 0x30 bytes, one own field `loaded` at +0x02C). The
unit-local views `D_8006EED8Obj`/`D_8006EED8Methods` and the single-slot cast
views `Slot0CMethods_322b4`, `Slot08Arg0Methods_322b4` and
`CtorArg1Methods_322b4` are gone; `GetActiveDataSourceMethods` is declared
`FileResourceMethods *`. The Source block above is the round-82 text; the live
body in `src/graphics/sprite.c` is byte-identical.

Renamed from `Get_vtable_D8006EED8` with rename.py (the getter
convention); the table `D_8006EED8` is `gRequestedFileMethods` (rename.py).
It returns `RequestedFileMethods *` now (`&gRequestedFileMethods`).

## Track 6 (2026-09-26, round 93, alpha)

The class `Class6EED8` (table `gClass6EED8Methods`, id 0xB03) is now
`RequestedFile` (`python3 tools/renametype.py Class6EED8 RequestedFile`,
tier A): its whole behaviour is to request one named file from the active
driver at construction (the ctor's requestLoadFile, +0x06C) and record in
`loaded` that the driver's setFlag (+0x064) reported it read; the ctor and
finalize clear `loaded`. It adds no buffer-consuming step, so the name says
what the methods do and no more. Round 87's header kept the address-derived
name because the only thing known beyond those mechanics was the caller's use
(WBgm loads SEQ files through it); a mechanics name sidesteps that objection
rather than overriding it, and `SeqFile` was rejected for the same reason.
The table and getter followed (`gRequestedFileMethods`,
`GetRequestedFileMethods`), and the header moved to `include/RequestedFile.h`.
renametype.py also rewrote the old class name inside earlier sections'
history prose in this and sibling reports (known, pending an operator
decision; not hand-reverted).
