# BoxFill__SetDisplay — MATCHED (12/12 words)

> Renamed from `func_800406E4` on 2026-09-25 (tools/rename.py). Address 0x800406e4.

Unit: `src/code_2cc8c_f.c`. First attempt.

```c
s32 BoxFill__SetDisplay(Obj6EAC0 *self, s32 a1) {
    return GetSetBitField(&self->unk58, 0x1F, 1, a1 == 0) == 0;
}
```

Same generic packed-bitfield-word accessor already documented in
`include/code_d294.h` (`GetSetBitField(word, shift, width, value)`),
here over `self->unk58` (a fresh field, not `code_d294`'s `unk10`).
Shift/width/negate shape matches `code_d294.c`'s own
`Class6B5CC__SetDisplay` verbatim (`GetSetBitField(&self->unk10, 0x1F, 1,
a1 == 0) == 0`), just against a different field.

### Proposed learning

The `GetSetBitField`-wrapper family generalises past `code_d294`: any
unit with a packed-bitfield-word field can reuse the exact same
shift/width/`==0` idioms already catalogued there. Worth checking
`code_d294.h`'s comment block first whenever a new unit's disassembly
shows a `jal GetSetBitField` (or the sibling helper it's built from).

## Naming

Round 54 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `BoxFill__SetDisplay` | (kept `BoxFill__SetDisplay`) | C |

**What is known.** A thin wrapper around `GetSetBitField(&self->flags,
shift, width, value)` (see `include/code_2cc8c.h`'s own comment on
`flags`, renamed from `unk58` this round), the SAME generic
packed-bitfield-word accessor `code_d294.c`'s own sibling functions
(`Class6B5CC__SetDisplay`/`D374`/`D3A0`) wrap -- and those, the FIRST instances
of this exact idiom in the project, are still unnamed too, for the same
reason: `GetSetBitField` returns the bit's OLD value while setting a
NEW one, and without knowing what the individual bit MEANS, a name can
only restate the mechanics (shift/width/negation), which the existing
header/report prose already documents precisely. Kept `func_` rather
than invent a `Get`/`SetFlagNN`-shaped name for an unidentified bit,
consistent with this project's own precedent on the sibling group.
