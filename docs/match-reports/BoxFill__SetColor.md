# BoxFill__SetColor — MATCHED (9/9 words)

> Renamed from `Obj6EAC0__SetColor` on 2026-09-25 (tools/rename.py). Address 0x8004076c.

Unit: `src/code_2cc8c_f.c`. No prior report on file (oversight -- this is
a genuine one-line wrapper, not a splat-generated trivial body).

```c
void BoxFill__SetColor(Obj6EAC0 *self, s32 overwrite, u8 *src) {
    BoxFill__ApplyColor(self, self->color, src, overwrite);
}
```

Base occupant of `slotB8`: a thin forwarder into `BoxFill__ApplyColor`
(this unit), always passing `self->color` as the destination.

## Naming

Round 54 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_8004076C` | `BoxFill__SetColor` | A |

**Evidence.** Mechanics are the whole purpose (tier A): the ENTIRE body
is a single call into `BoxFill__ApplyColor` with `self->color` as the
fixed destination -- "set (or blend) this object's colour" is exactly
what the code does, nothing more to guess at.
