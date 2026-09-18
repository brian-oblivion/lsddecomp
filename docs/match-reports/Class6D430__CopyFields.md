> Renamed from `func_80026D88` on 2026-09-18 (tools/rename.py). Address 0x80026d88.

# Class6D430__CopyFields

**Unit:** code_171e0 · **Size:** 33 instructions · **Status:** MATCHED (33/33 words, whole-image build verified byte-exact)

## What it does

A field-by-field copy from one `UnkFlagsObj_171e0` instance into another:
`+0x40`..`+0x58` (7 words), then a gap of 3 words (`+0x5C`..`+0x64`, not
copied), then `+0x68`..`+0x74` (4 words).

## Derivation

```
lw $v0, 0x40($a1)  / sw $v0, 0x40($a0)
lw $v0, 0x44($a1)  / sw $v0, 0x44($a0)
lw $v0, 0x48($a1)  / sw $v0, 0x48($a0)
lw $v0, 0x4C($a1)  / sw $v0, 0x4C($a0)
lw $v0, 0x50($a1)  / sw $v0, 0x50($a0)
lw $v0, 0x54($a1)  / sw $v0, 0x54($a0)
lw $v0, 0x58($a1)  / sw $v0, 0x58($a0)
lw $v0, 0x68($a1)  / sw $v0, 0x68($a0)
lw $v0, 0x6C($a1)  / sw $v0, 0x6C($a0)
lw $v0, 0x70($a1)  / sw $v0, 0x70($a0)
lw $v0, 0x74($a1)  / sw $v0, 0x74($a0)
```

Each field is an independent `lw`/`sw` pair through `$v0` — not GCC's
inlined block-move codegen (contrast `func_80025E1C.md` in `class_16334`,
which *is* a block copy) — so this is a straight sequence of per-field
assignments, not a `struct` value copy or a loop. The 3-word gap
(`+0x5C`..`+0x64`) is real: retail's instruction stream jumps straight from
`+0x58` to `+0x68` with no intervening load/store, so those offsets are
deliberately excluded from the copy, not merely unread — `UnkFlagsObj_171e0`
leaves them as an unnamed `pad5C` array rather than guessing a name for
them.

## Final C

```c
void Class6D430__CopyFields(UnkFlagsObj_171e0 *dst, UnkFlagsObj_171e0 *src) {
    dst->unk40 = src->unk40;
    dst->unk44 = src->unk44;
    dst->unk48 = src->unk48;
    dst->unk4C = src->unk4C;
    dst->unk50 = src->unk50;
    dst->unk54 = src->unk54;
    dst->unk58 = src->unk58;
    dst->unk68 = src->unk68;
    dst->unk6C = src->unk6C;
    dst->unk70 = src->unk70;
    dst->unk74 = src->unk74;
}
```

## Attempt log

Matched on the first attempt — a direct transcription of the field list in
address order. No branches, no calls, no register-allocation choices to get
wrong.

## Head broadcast levers — applicability

- **goto-vs-return:** not applicable, no branches, void return.
- **loop-invariant hoisting:** not applicable — deliberately *not* written as
  a loop, since retail's instruction stream shows independent straight-line
  field copies rather than a counted loop (and a loop would also be the
  wrong shape across the `+0x5C`..`+0x64` gap).
- **prologue store-order barrier:** not applicable; this function has no
  prologue register saves at all (leaf, no callee-saved registers used).

## Proposed learning

None new — this is a clean instance of the "copy specific named fields, not
a block" shape already covered by existing guidance; worth noting only that
the *gap* in the offset list is itself informative (there's a real field or
fields at `+0x5C..+0x64` that this particular copy path intentionally skips,
likely cache/derived state rather than persistent data).

## Naming

Round 52 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_80026D88` | `Class6D430__CopyFields` | B |

**Evidence.** A field-by-field copy of a fixed subset of one
`UnkFlagsObj_171e0` instance's fields into another (`+0x40..+0x58`,
`+0x68..+0x74`, skipping a real 0xC-byte gap). Mechanics fully derived
(confirmed instruction-by-instruction, not a struct-value copy); why these
specific fields travel together and not the gap is not established.
