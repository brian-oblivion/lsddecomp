# Obj86B60__SetCallback — MATCH (3/3 words)

> Renamed from `func_8003CAEC` on 2026-09-24 (tools/rename.py). Address 0x8003caec.

**Unit:** code_2cc8c · **Size:** 3 instructions

## What it does

```c
void Obj86B60__SetCallback(Obj86B60 *self, void (*a1)(void *ctx), void *a2)
{
    self->unk9C = a1;
    self->unkA0 = a2;
}
```

A pure setter, establishing `Obj86B60::unk9C`/`unkA0` as an
(unconditional) callback/context pair -- confirmed against
`Obj86B60__RefreshViewValue`'s use of both (see that report).

## Provenance

round 2026-09-02, runner echo, unit code_2cc8c. 1 attempt.

## Naming (round 78, delta)

**Tier A** (pure setter). `func_8003CAEC` -> `Obj86B60__SetCallback`. Body:
`self->unk9C = a1; self->unkA0 = a2;` -- stores a callback pointer and its
context argument verbatim, no other logic. Corroborated by
`Obj86B60__RefreshViewValue`, the sole invoker of this pair
(`self->unk9C(self->unkA0)`).

## Proposed field names (round 78, delta -- NOT applied, cross-unit)

`Obj86B60::unk9C` (`void (*)(void *ctx)`, +0x09C) -> `viewCallback`;
`Obj86B60::unkA0` (`void *`, +0x0A0) -> `viewCallbackCtx`. Tier B (mechanics:
a callback+context pair invoked by `Obj86B60__RefreshViewValue`, hence
"view"; not a guess about what the callback itself does). Grep shows
`unk9C`/`unkA0` textual hits in class_3bb8c_n.c/class_3bb8c_q.c/code_2c054.c/
code_179d8_i.c (unrelated structs sharing the name), so proposal only.


**Head disposition, round 78.** `unk9C`/`unkA0` -> `viewCallback`/`viewCallbackCtx` APPLIED (type scope, 3 + 2 accessors).
