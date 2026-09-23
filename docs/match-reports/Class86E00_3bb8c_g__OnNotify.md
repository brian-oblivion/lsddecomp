# Class86E00_3bb8c_g__OnNotify -- MATCH

> Renamed from `func_8004FF90` on 2026-09-23 (tools/rename.py). Address 0x8004ff90.

Unit `class_3bb8c_g`, round 14. `./build-and-verify.sh` exit 0; whole-image
SHA1 matches retail. `funcdiff.py Class86E00_3bb8c_g__OnNotify`: 25/25 words match.

## Source

```c
void Class86E00_3bb8c_g__OnNotify(Class86E00_3bb8c_g *self, s32 arg1, s32 arg2)
{
    if (self->unk28 != 0) {
        if (arg2 == 0x19) {
            self->methods->slot90(self);
        } else if (arg2 == 0x17) {
            self->methods->slot94(self);
        }
    }
}
```

First attempt, byte-exact. `arg1` (`$a1`) is never read anywhere in the
body -- confirmed genuinely unused, kept as a declared-but-unread
parameter per the established "an unused parameter just needs declaring,
not using" convention.

## Struct changes (additive, `include/class_3bb8c.h`)

- `Class86E00Methods_3bb8c_g::slot90`/`slot94` -- both already declared
  ahead of this function while surveying the whole unit
  (`Class86E00_3bb8c_g__TickCardIcon`'s report); this is the function that exercises them.

### Proposed learning

None new.

## Naming

`Class86E00_3bb8c_g__OnNotify` (was `func_8004FF90`), tier B: signature
shape `(self, arg1, arg2)` with `arg1` unread and `arg2` a small dispatch
code (`0x19`/`0x17`) gated on `self->unk28 != 0` matches this project's
established `(self, sender, event)` notify-receiver shape
(`BasicClass__OnNotify`/`slot38`, `include/code_8220.h`) closely enough to
use the same verb, though this class's own table is not BasicClass's and
`arg1` is never read as a sender here. What the two event codes mean is
not established.
