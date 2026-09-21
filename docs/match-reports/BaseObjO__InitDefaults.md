# BaseObjO__InitDefaults -- MATCHED (4/4 words)

> Renamed from `func_800571E8` on 2026-09-18 (tools/rename.py). Address 0x800571e8.

Unit: `class_3bb8c_o` (round 17). A two-field setter: `self->unk48 = 0x12C`
(halfword), `self->unk54 = 0` (word).

## Final source

```c
void BaseObjO__InitDefaults(BaseObjO *self) {
    self->unk48 = 0x12C;
    self->unk54 = 0;
}
```

## Derivation

`sh $v0,0x48($a0)` (`$v0` pre-loaded with the literal `0x12C`) then
`sw $zero,0x54($a0)` in the branch delay slot of `jr $ra`. Two independent
field writes, no control flow -- matched first try. `unk48`'s width
(`s16`) is confirmed by `BaseObjO__func_571f8`'s own `lh` read of the same field
(sign-extending load).

### Proposed learning

None -- straightforward setter.

## Naming

**`BaseObjO__InitDefaults` -- tier B.** Mechanics fully described: sets
`self->unk48 = 0x12C` (300) and `self->unk54 = 0`, unconditionally, no
control flow. Named for the mechanic (initializing two fields to fixed
default values) without asserting what those fields represent in the
game -- `unk48`/`unk54` are left unrenamed. `BaseObjO__func_571f8` reads
both fields together (adding/subtracting `unk54` from `unk48` depending on
sign) which is suggestive of a paired value/adjustment relationship, but
not strong enough evidence for a purpose-asserting name like "heading".
