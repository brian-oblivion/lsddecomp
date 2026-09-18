## Naming

Round 54 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_80040F20` | `Obj6EAC0__NoOpSetter` | A |

**Evidence.** An empty function body (`{ }`), splat-generated (`jr $ra;
nop`), the derived table's own occupant of `slotC8` -- the same slot the
base table fills with a real one-field setter (`func_800408A0`, left
unnamed: `unk44`'s purpose is not established). Same shape as the
project's existing `NoOp`/`NoOpIgnoreArgs` precedent (a pure do-nothing
leaf, tier A by definition), given a class-prefixed name rather than a
bare one since it specifically overrides ONE slot of ONE class's table,
unlike those two shared, class-agnostic fillers.
