# func_8004CFB0 — MATCHED (2/2 words)

Two-instruction leaf: `jr $ra` / `sw $a1, 0x1DC($a0)`. Not a vtable slot,
no caller found anywhere in the executable — a plain setter, presumably
called from as-yet-uncarved ground (`class_3bb8c_c`) or from outside this
365-function block entirely.

## Disassembly

```
jr   $ra
 sw   $a1, 0x1DC($a0)     ; self->unk1DC = arg1
```

## Final C

```c
void func_8004CFB0(Obj866E8 *self, Bounds866E8_3bb8c_b *arg1) {
    self->unk1DC = arg1;
}
```

## New struct knowledge

- `Obj866E8::unk1DC` (`Bounds866E8_3bb8c_b *`, +0x1DC) — typed from
  `func_8004CD38`'s own body (see that function's report), the only place
  in this unit that reads it back. `func_8004CD38` itself stalled, but the
  field's TYPE derivation (a 4-field min/max bounding-box struct) does not
  depend on that function's byte-match — it comes from reading the
  occupant's own disassembly, independent of whether the source shape
  that produces it byte-exactly has been found yet.

## Attempts

1 (matched on first attempt).

### Proposed learning

None new.
