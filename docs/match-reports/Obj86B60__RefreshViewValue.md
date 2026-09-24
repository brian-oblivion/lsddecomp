# Obj86B60__RefreshViewValue — MATCH (22/22 words)

> Renamed from `func_8003CA94` on 2026-09-24 (tools/rename.py). Address 0x8003ca94.

**Unit:** code_2cc8c · **Size:** 22 instructions

## What it does

```c
void Obj86B60__RefreshViewValue(Obj86B60 *self)
{
    if (self->unk9C != NULL) {
        self->unk9C(self->unkA0);
    }
    self->methods->slot60(self, 7);
}
```

`self->unk9C` is a callback taking one opaque argument (`self->unkA0`,
itself passed through unchanged); both are set together by
`Obj86B60__SetCallback` (see that report, next in ROM order below this one but
established first since it is the 3-instruction setter). The `slot60(self,
7)` call is unconditional -- happens whether or not the callback fired.

## Struct knowledge established

- `Obj86B60::unk9C` (`void (*)(void*)`, +0x09C) and `::unkA0` (`void *`,
  +0x0A0) -- OBSERVED here as call targets; set by `Obj86B60__SetCallback`.

## Provenance

round 2026-09-02, runner echo, unit code_2cc8c. 1 attempt.

## Naming (round 78, delta)

**Tier A.** `func_8003CA94` -> `Obj86B60__RefreshViewValue`. Occupies slot94
in `gTaskCoreMethods`; `gClass86B60Methods` overrides the same slot with
`Class86B60__RefreshViewValue`, same evidence shape as `Tick`/`SetState`
above. Corroborated independently: this function's own body invokes
`self->unk9C(self->unkA0)` when set, which is exactly the callback+ctx pair
`Obj86B60__SetCallback` installs -- see that report.
