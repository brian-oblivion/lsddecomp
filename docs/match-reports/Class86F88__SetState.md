# Class86F88__SetState -- MATCH

> Renamed from `func_800521D4` on 2026-09-24 (tools/rename.py). Address 0x800521d4.

Unit `class_3bb8c_k`, round 15. `./build-and-verify.sh` exit 0 (checked once
the whole unit's batch of matches was in place); whole-image SHA1 matches
retail. `funcdiff.py Class86F88__SetState`: 42/42 words match.

This is vtable slot `+0x054` of `gClass86F88Methods` (`Class86F88Methods::slot54`),
dispatched by `Class86F88__TickClosing` (this unit) as `self->methods->slot54(self, 4)`.

## Source

```c
void Class86F88__SetState(Class86F88 *self, s32 state)
{
    self->unk30 = 0;
    if (state < 2) {
        goto end;
    }
    if (state < 4) {
        goto case_lt4;
    }
    if (state == 4) {
        goto case_eq4;
    }
    goto end;
case_lt4:
    self->methods->slot14(self, self->unk34);
    self->methods->slot48(self);
    self->unk2C = state;
    goto end;
case_eq4:
    self->methods->slot30(self, self->unk2C);
end:
    return;
}
```

## Notes

A 3-way dispatch on `state` (`<2` no-op, `2..3` one path, `==4` a second
path, `>4` no-op), with `self->unk30 = 0` unconditional at the top (it sits
in the delay slot of the first branch in the disassembly, so it executes
on every call regardless of `state`).

**Neither a nested `if`/`else if` nor an early-return-per-case reproduced
retail's physical block layout or its branch instructions**, even though
both are semantically identical to the final form. Both alternate shapes
compiled 2 words short (a whole `beq`+`nop`+`j`+`nop` 4-instruction group
collapsed into a 2-instruction `bne`), and when the `state==4` code was
written textually before the `state<4` case, GCC swapped which body was
placed first in the physical layout (matching the *source* order, not
retail's ROM order). Only writing the three-way test as explicit
`goto`/label pairs -- with `case_lt4` appearing textually (and therefore
physically) **before** `case_eq4`, replicating retail's own branch
polarities exactly (`bnez` into `case_lt4`, `beq` into `case_eq4`, implicit
fallthrough to `end`) -- reproduced retail's instructions exactly.

### Proposed learning

A three-way dispatch (`if (a) ... else if (b) ... else ...`) where two
branches jump into non-adjacent blocks is not reliably reproduced by
nested `if`/`else if`, even when every value and every condition is
correct -- GCC 2.6.3 chooses its own branch polarity and block order from
the *source's* textual/control-flow shape, not from some canonical
lowering of the semantics. When a residue is "off by a `beq`+`j` pair" or
"the two case bodies are swapped in physical order", try explicit
`goto`/label pairs whose textual order and branch polarity mirror the
disassembly's own `bnez`/`beq`/fallthrough sequence one-for-one, rather
than continuing to reshape nested conditionals.
