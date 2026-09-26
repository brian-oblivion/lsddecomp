
## Naming

Round 52 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_80026C80` | `NoOp` | A |

**Evidence.** An empty function body (`{ }`), used as a shared do-nothing
filler slot across at least six different vtables (`asm/data/5DB70.data.s`,
`5E140.data.s`, `5F474.data.s`, `720F0.data.s`), not specific to `D_8006D430`
or this unit's class at all. A pure leaf whose mechanics (does nothing) are
its whole purpose -- tier A by the plan's own rule -- and deliberately given
a bare, unprefixed name rather than `FileResource__...` since it does not
belong to any one class.
