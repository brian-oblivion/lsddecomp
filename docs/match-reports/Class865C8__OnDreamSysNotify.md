# Class865C8__OnDreamSysNotify -- MATCHED (splat-generated, empty body)

> Renamed from `Obj865C8__Noop80` on 2026-09-26 (tools/rename.py). Address 0x80049eac.

`Class865C8Methods` slot +0x080. Dispatched by `Class865C8__OnNotify` with real `EventArg *`/`s32` arguments (loaded into `$a1`/`$a2` at the call site), which this occupant ignores. Entire function body:

```c
void Class865C8__OnDreamSysNotify(void) {
}
```

A no-op BODY is not evidence the SLOT's signature takes no arguments (CLAUDE.md) -- the slot itself stays typed with its full signature in the header even though this occupant's own C ignores both parameters. Never had its own report before this round's rename; created here per the one-report-per-function rule.


## Naming

`Class865C8__OnDreamSysNotify` -- tier A. Empty body (`{}`), occupies +0x080; dispatched from `Class865C8__OnNotify` with real `EventArg`/tag arguments that this occupant simply ignores. Empty body is direct evidence; CLAUDE.md's caveat that a no-op body is not evidence the SLOT takes no arguments is why the slot itself stays typed with its full signature.

## Track 4 (2026-09-26, round 88, Class865C8)

The class (table D_800865C8, id 0x1F230, TimedTask's subclass) is unified as Class865C8 in include/Class865C8.h; the Obj865C8/Class865C8Methods views in class_39e08.h are gone. Renamed from Obj865C8__Noop80: own slot +0x080, which Class865C8__OnNotify calls for a sender whose id & 0xFFFF is 0x1F34, DreamSys's. Empty.
