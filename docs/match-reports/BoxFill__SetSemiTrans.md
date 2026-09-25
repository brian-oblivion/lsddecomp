# BoxFill__SetSemiTrans — MATCHED (11/11 words)

> Renamed from `func_80040714` on 2026-09-25 (tools/rename.py). Address 0x80040714.

Unit: `src/code_2cc8c_f.c`. First attempt.

```c
s32 BoxFill__SetSemiTrans(Obj6EAC0 *self, s32 a1) {
    return GetSetBitField(&self->unk58, 0x1E, 1, a1 != 0);
}
```

Sibling of `BoxFill__SetDisplay`/`BoxFill__SetSemiTransRate`; base-table occupant of
`Obj6EAC0Methods::slot0x64`. Shift 0x1E, width 1, no negation on the
result -- matches `code_d294.c`'s `Class6B5CC__SetSemiTrans` shape exactly modulo
field name.

### Proposed learning

None beyond `BoxFill__SetDisplay`'s entry.

## Naming

Round 54 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `BoxFill__SetSemiTrans` | (kept `BoxFill__SetSemiTrans`) | C |

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
