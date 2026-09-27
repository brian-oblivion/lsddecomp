# RequestedFile__Finalize -- MATCHED (15/15 words), round 82

> Renamed from `Class6EED8__Finalize` on 2026-09-26 (tools/rename.py). Address 0x800423a8.

> Renamed from `D8006EED8__Finalize` on 2026-09-26 (tools/rename.py). Address 0x800423a8.

> Renamed from `func_800423A8` on 2026-09-25 (tools/rename.py). Address 0x800423a8.

Round 82, runner alpha (third re-staffed slot of the round). Unit `src/Sprite.c`. Fresh ground (carved in FINISHING-PLAN revision 18), no prior attempt, no report before this one.

- **Where:** gRequestedFileMethods slot +0x00C (finalize) (`tools/classtable.py`).
- **What:** Clears +0x02C (the flag `RequestedFile__MarkLoaded` sets), then calls slot +0x00C of `GetActiveDataSourceMethods()` (code_171e0). The store lands in the `jal` delay slot. `GetActiveDataSourceMethods` is declared locally with a local `Slot0CMethods_322b4` return type, as `code_179d8_d.c` does with its own view.
- **Result:** byte-exact; 15/15 words, 0 insertions / 0 deletions, whole-image SHA1 green (`./build-and-verify.sh` OK). First build.
- **Types:** local views and prototypes live in the unit; no shared header was touched.

## Source

```c
/* gRequestedFileMethods slot +0x00C (finalize): clear +0x2C, then the base finalize. */
void RequestedFile__Finalize(D_8006EED8Obj *self) {
    self->flag2C = 0;
    GetActiveDataSourceMethods()->slot0C(self);
}
```

## Naming

- `RequestedFile__Finalize` -- tier A. Finalize (slot +0x00C): clears flag2C then calls the base (GetActiveDataSourceMethods) finalize.

## Track 4 (2026-09-26, round 87, delta)

Class id 0xB03 is unified as `RequestedFile` in `include/RequestedFile.h`
(FILERESOURCE_SLOTS/FIELDS, 0x30 bytes, one own field `loaded` at +0x02C). The
unit-local views `D_8006EED8Obj`/`D_8006EED8Methods` and the single-slot cast
views `Slot0CMethods_322b4`, `Slot08Arg0Methods_322b4` and
`CtorArg1Methods_322b4` are gone; `GetActiveDataSourceMethods` is declared
`FileResourceMethods *`. The Source block above is the round-82 text; the live
body in `src/Sprite.c` is byte-identical.

Renamed from `D8006EED8__Finalize` with rename.py (the class name).
Accessors: `flag2C` -> `loaded`, `slot0C` -> the inherited `finalize`
(`GetActiveDataSourceMethods()->finalize((FileResource *)self)`).

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
