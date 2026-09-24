# ObjM__OnRegistrantEvent

> Renamed from `func_80052E7C` on 2026-09-24 (tools/rename.py). Address 0x80052e7c.

**Unit:** class_3bb8c_l · **Size:** 16 words (0x40 bytes) ·
**Status: MATCHED 16/16**, whole-image SHA1 green.

## What it does

The callback registered by `ObjM__AttachTarget` (address-taken there). Dispatches
on `code` to one of two still-uncarved helpers (`asm/psyq_memset.s`, despite
the filename these are not Psy-Q library code).

```c
void ObjM__OnRegistrantEvent(Obj87034_3bb8c_l *self, s32 code, s32 arg2, s32 arg3) {
    if (code >= 0) {
        func_80049060(self->unk38);
    } else {
        func_80049098(self->unk38, arg2, arg3);
    }
}
```

## Notes

- **Branch polarity is not free to choose even when semantically
  equivalent.** The natural-reading translation `if (code < 0) { A } else
  { B }` compiles to a `bgez` test; retail's actual instruction is `bltz`
  at the SAME branch target. Writing the mathematically-equivalent `if
  (code >= 0) { B-was-A } else { A-was-B }` (i.e. swap which literal
  comparison is written, keep the target/fallthrough content mapped to
  the same branch) reproduces retail's exact `bltz` byte-for-byte. GCC
  2.6.3 consistently compiles `if (C) THEN else ELSE` as: machine test =
  NOT(written C), THEN-body placed at the FALLTHROUGH, ELSE-body placed at
  the branch TARGET. To match a specific target/fallthrough placement,
  solve for the written `C` that makes `NOT(C)` equal the actual machine
  test, don't guess from the semantics alone.
- `func_80049060` is genuinely called at two different arities across the
  executable (2 real args from `func_80049098`'s own forwarding call, only
  1 here — the second register is `code`, a leftover value from this
  function's own parameter, never actually intended as an argument).
  Declared K&R/unprototyped (`extern s32 func_80049060();`) in
  `include/class_3bb8c.h`, the documented escape hatch for this pattern.

### Proposed learning

Confirms (third instance, after `ObjM__OnRegistrantEvent`'s own sibling case in
`func_800531CC` and one more below) that GCC 2.6.3's `if`/`else` codegen
convention is completely mechanical: `NOT(written condition)` is always the
compiled test, and the `if`-body always lands at the fallthrough. When a
residue is "right content, wrong branch instruction and swapped
target/fallthrough", don't hunt for a different algorithm — just invert
the written comparison and swap the two bodies.
