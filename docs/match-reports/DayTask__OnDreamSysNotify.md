# DayTask__OnDreamSysNotify -- MATCHED (splat-generated, empty body)

> Renamed from `Class865C8__OnDreamSysNotify` on 2026-09-26 (tools/rename.py). Address 0x80049eac.

> Renamed from `Obj865C8__Noop80` on 2026-09-26 (tools/rename.py). Address 0x80049eac.

`DayTaskMethods` slot +0x080. Dispatched by `DayTask__OnNotify` with real `EventArg *`/`s32` arguments (loaded into `$a1`/`$a2` at the call site), which this occupant ignores. Entire function body:

```c
void DayTask__OnDreamSysNotify(void) {
}
```

A no-op BODY is not evidence the SLOT's signature takes no arguments (CLAUDE.md) -- the slot itself stays typed with its full signature in the header even though this occupant's own C ignores both parameters. Never had its own report before this round's rename; created here per the one-report-per-function rule.


## Naming

`DayTask__OnDreamSysNotify` -- tier A. Empty body (`{}`), occupies +0x080; dispatched from `DayTask__OnNotify` with real `EventArg`/tag arguments that this occupant simply ignores. Empty body is direct evidence; CLAUDE.md's caveat that a no-op body is not evidence the SLOT takes no arguments is why the slot itself stays typed with its full signature.

## Track 4 (2026-09-26, round 88, DayTask)

The class (table D_800865C8, id 0x1F230, TimedTask's subclass) is unified as DayTask in include/day_task.h; the Obj865C8/DayTaskMethods views in dream_day.h are gone. Renamed from Obj865C8__Noop80: own slot +0x080, which DayTask__OnNotify calls for a sender whose id & 0xFFFF is 0x1F34, DreamSys's. Empty.
