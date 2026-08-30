# func_80065DBC

**Unit:** code_55dd4 · **Size:** 12 words (0x30 bytes) · **Status:** MATCHED
(12/12 words, whole-image `./build-and-verify.sh` green)

## What it does

`Class65650Methods` slot `+0x100` (`slot_setup70`). The same guard/defer
shape as `func_80065BFC`, but for the `+0x70`/`+0x74` array pair instead of
`+0x5C`, and deferring to `func_80065E1C` instead of `func_80065C5C`. Unlike
`func_80065C5C`, `func_80065E1C` never references its own `$a1` (confirmed
by reading its body: it only ever writes outgoing `$a1`, never reads an
incoming one), so this dispatcher takes a single `self` parameter, not two.

```c
s32 func_80065DBC(Class65650 *self)
{
    if (self->unk70 != NULL) {
        return 0;
    }
    return func_80065E1C(self);
}
```

Matched on the direct translation — same zero-residue result as
`func_80065BFC`, for the same reason (see that report's note on the
`goto` lever: the fall-through arm's return value is the callee's own
`$v0`, nothing to materialize).

### Proposed learning

None beyond `func_80065BFC.md`'s.
