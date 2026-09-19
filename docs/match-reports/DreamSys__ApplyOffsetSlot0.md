> Renamed from `func_800574C4` on 2026-09-19 (tools/rename.py). Address 0x800574c4.

# DreamSys__ApplyOffsetSlot0 -- MATCHED (14/14)

Unit: `src/class_3bb8c_p.c`. Class: `DreamSys`, own vtable slot `+0x0C8`
(base-class-inherited; resolved via `tools/classtable.py DREAMSYS_METHODS`
and confirmed unchanged in the DreamSys-level table too).

## Signature

```c
void DreamSys__ApplyOffsetSlot0(DreamSys *self, s32 val, void *extra);
```

## Body

```c
void DreamSys__ApplyOffsetSlot0(DreamSys *self, s32 val, void *extra) {
    DreamSys__ApplyOffsetSlotAndNotify(self, &D_8008ABA4[0], val, extra, 7);
}
```

A thin wrapper: forwards to `DreamSys__ApplyOffsetSlotAndNotify` (this unit's own helper, see
its own match report) with a fixed pointer into element 0 of a
newly-identified 2-element `s16` array `D_8008ABA4`, and the constant `7`.

`val` is `s32`, not `s16`, even though `DreamSys__ApplyOffsetSlotAndNotify` only ever uses it
truncated to 16 bits -- see `DreamSys__ApplyOffsetSlotAndNotify`'s report for why: typing it
`s16` here forces a spurious `sll`/`sra` re-sign-extend pair at this call
site that retail does not have.

## `D_8008ABA4` is a 2-element `s16` array, not a lone `s32`

splat's single-word `dlabel D_8008ABA4` (`asm/data/7B008.sdata.s`) is really
`s16 D_8008ABA4[2]`: this function writes element 0
(`%hi/%lo(D_8008ABA4)`), its sibling `DreamSys__ApplyOffsetSlot1` writes element 1
(`%hi/%lo(D_8008ABA4 + 0x2)`). Not referenced anywhere else in the repo
(checked with `grep -rn D_8008ABA4 src/ include/` before this round), so
declared locally in `src/class_3bb8c_p.c` rather than added to a shared
header.

## Verify

```
./build-and-verify.sh   # build exit=0, OK: build matches retail
tools/funcdiff.py DreamSys__ApplyOffsetSlot0   # 14/14
```
