# Obj6EAC0__Destruct — MATCHED (22/22 words)

> Renamed from `func_80040A30` on 2026-09-18 (tools/rename.py). Address 0x80040a30.

Unit: `src/code_2cc8c_f.c`. First attempt.

```c
void Obj6EAC0__Destruct(Obj6EAC0 *self) {
    ReleaseBasicClassArray(self->unkB4, self->unkA9);
    self->unkB4 = BMemPMgrFree(self->unkB4);
    Get_vtable_D8006EC74()->slot0C(self);
}
```

Derived-table occupant of `Obj6EAC0Methods::slot0C`. Retail's own
delay slot after the `Get_vtable_D8006EC74()` call stores the PRECEDING
call's return (`BMemPMgrFree`'s), not `Get_vtable_D8006EC74`'s own -- read
per the established "a delay slot after a `jal` carries the value from
the PRECEDING call's return" idiom, and it is what motivated retyping
`BMemPMgrFree`'s shared declaration (see below).

## Existing declaration retyped

`include/code_2cc8c.h`'s `BMemPMgrFree` was declared `void
BMemPMgrFree(void *ptr)`, following `code_171e0.h`/`Entity.h`'s
typing. Its own (still-`INCLUDE_ASM`) disassembly
(`asm/nonmatchings/code_8220/BMemPMgrFree.s`) ends with an explicit
`addu $v0, $zero, $zero` -- it genuinely returns `NULL`, and this
function is the first in this unit to actually USE that return value
(`self->unkB4 = BMemPMgrFree(self->unkB4)`). Retyped to
`void *BMemPMgrFree(void *ptr)`. Every other call site in this unit
(`code_2cc8c_b.c`, `code_2cc8c_d.c`) discards the result as a bare
statement, so the retype changes no compiled bytes there; confirmed by
a full green `build-and-verify.sh`.

### Proposed learning

Another instance of "a discarded return value is never evidence of
void" -- this one caught BEFORE it caused a stall, because the callee's
own disassembly was checked directly rather than trusted from another
unit's caller-side typing.

## Naming

Round 54 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_80040A30` | `Obj6EAC0__Destruct` | B |

**Evidence.** Derived occupant of `slot0C`: releases the whole
`children` array (`ReleaseBasicClassArray(self->children,
self->totalChildCount)`), frees the array pointer itself
(`BMemPMgrFree`), then chains to the third sibling table's own `slot0C`
-- the mirror-image teardown of `Obj6EAC0__Construct`'s own setup. Tier B
for the same reason as `Construct`: the destructor role is certain from
the mechanics, the class's broader purpose is not independently
confirmed.
