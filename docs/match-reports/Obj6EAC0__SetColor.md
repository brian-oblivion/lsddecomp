# Obj6EAC0__SetColor — MATCHED (9/9 words)

Unit: `src/code_2cc8c_f.c`. No prior report on file (oversight -- this is
a genuine one-line wrapper, not a splat-generated trivial body).

```c
void Obj6EAC0__SetColor(Obj6EAC0 *self, s32 overwrite, u8 *src) {
    Obj6EAC0__ApplyColor(self, self->color, src, overwrite);
}
```

Base occupant of `slotB8`: a thin forwarder into `Obj6EAC0__ApplyColor`
(this unit), always passing `self->color` as the destination.

## Naming

Round 54 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_8004076C` | `Obj6EAC0__SetColor` | A |

**Evidence.** Mechanics are the whole purpose (tier A): the ENTIRE body
is a single call into `Obj6EAC0__ApplyColor` with `self->color` as the
fixed destination -- "set (or blend) this object's colour" is exactly
what the code does, nothing more to guess at.
