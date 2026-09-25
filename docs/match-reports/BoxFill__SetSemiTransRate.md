# BoxFill__SetSemiTransRate — MATCHED (11/11 words)

> Renamed from `func_80040740` on 2026-09-25 (tools/rename.py). Address 0x80040740.

Unit: `src/code_2cc8c_f.c`. First attempt.

```c
s32 BoxFill__SetSemiTransRate(Obj6EAC0 *self, s32 a1) {
    return GetSetBitField(&self->unk58, 0x1C, 2, a1);
}
```

Sibling of `BoxFill__SetDisplay`/`BoxFill__SetSemiTrans`; base-table occupant of
`Obj6EAC0Methods::slot0x68`. Shift 0x1C, width 2, raw `a1` passed
through unchanged -- matches `code_d294.c`'s `Class6B5CC__SetSemiTransRate` shape.

### Proposed learning

None beyond `BoxFill__SetDisplay`'s entry.

## Naming

Round 54 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `BoxFill__SetSemiTransRate` | (kept `BoxFill__SetSemiTransRate`) | C |

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
