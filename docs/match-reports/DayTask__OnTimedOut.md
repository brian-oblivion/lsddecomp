# DayTask__OnTimedOut -- MATCHED (splat-generated, empty body)

> Renamed from `DayTask__OnState4` on 2026-09-28 (tools/rename.py). Address 0x80049ea4.

> Renamed from `Class865C8__OnState4` on 2026-09-26 (tools/rename.py). Address 0x80049ea4.

> Renamed from `Obj865C8__Noop7C` on 2026-09-26 (tools/rename.py). Address 0x80049ea4.

`DayTaskMethods` slot +0x07C. Entire function body:

```c
void DayTask__OnTimedOut(Obj865C8 *self) {
}
```

An empty override, byte-exact by construction (`jr $ra` / `nop`). Never had its own report before this round's rename; created here because CLAUDE.md requires one file per touched function, matched ones included.


## Naming

`DayTask__OnTimedOut` -- tier A. Empty body (`{}`), occupies the class's own +0x07C override slot (already named `noop7C` in the header before this round). An empty body is direct evidence of a no-op leaf.

## Track 4 (2026-09-26, round 88, DayTask)

The class (table D_800865C8, id 0x1F230, TimedTask's subclass) is unified as DayTask in include/day_task.h; the Obj865C8/DayTaskMethods views in dream_day.h are gone. Renamed from Obj865C8__Noop7C: occupant of TimedTask's +0x07C onState4 (setState(4) calls it), empty.
