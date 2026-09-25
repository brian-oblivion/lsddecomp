# Class6D430__CopyFields

> Renamed from `func_80026D88` on 2026-09-18 (tools/rename.py). Address 0x80026d88.

**Unit:** code_171e0 · **Size:** 33 instructions · **Status:** MATCHED (33/33 words, whole-image build verified byte-exact)

## What it does

A field-by-field copy from one `Class6D430` instance into another:
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
inlined block-move codegen (contrast `Pad__LoadButtonTable.md` in `class_16334`,
which *is* a block copy) — so this is a straight sequence of per-field
assignments, not a `struct` value copy or a loop. The 3-word gap
(`+0x5C`..`+0x64`) is real: retail's instruction stream jumps straight from
`+0x58` to `+0x68` with no intervening load/store, so those offsets are
deliberately excluded from the copy, not merely unread — `Class6D430`
leaves them as an unnamed `pad5C` array rather than guessing a name for
them.

## Final C

```c
void Class6D430__CopyFields(Class6D430 *dst, Class6D430 *src) {
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
`Class6D430` instance's fields into another (`+0x40..+0x58`,
`+0x68..+0x74`, skipping a real 0xC-byte gap). Mechanics fully derived
(confirmed instruction-by-instruction, not a struct-value copy); why these
specific fields travel together and not the gap is not established.

## Proposed field names

**APPLIED by the head at merge, round 52** -- all four fields, both types
and all five vtable slots below are now in the tree, each one applied
separately with `./build-and-verify.sh` green and byte-exact after it. One
mis-hit had to be resolved by receiver type: `src/code_179d8_h.c:143`
accesses `pendingGeneration` on a `Class6D430 *self`, while the same file's
lines 97/174/175/182 are its OWN `ObjA34_179D8H::unk0C` and were left alone.
The compiler named that mis-hit (`structure has no member named 'unk0C'`),
which is the procedure working in the direction where it can work.

Same cross-unit exposure (`code_179d8_h.c`/`code_179d8_q.c` include
`Class6D430`), so PROPOSED, not renamed. None of `unk40`/`unk44`/
`unk48`/`unk4C`/`unk50`/`unk54`/`unk58`/`unk68`/`unk6C`/`unk70`/`unk74` has
any evidence beyond "copied together, in this order, with a real 0xC-byte
gap between the two runs" -- no read or write site elsewhere in this unit
distinguishes one from another. Proposing a rename for any single one of
them would be a purpose guess with nothing behind it, which the plan is
explicit is worse than the placeholder. No renames proposed for this block;
flagging it here so a future round with more context (e.g. once the
subclass that actually populates these fields is identified) does not have
to re-discover that they travel together.
