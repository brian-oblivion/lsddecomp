# Obj865C8__Noop80 -- MATCHED (splat-generated, empty body)

`Class865C8Methods` slot +0x080. Dispatched by `Obj865C8__OnNotify` with real `EventArg *`/`s32` arguments (loaded into `$a1`/`$a2` at the call site), which this occupant ignores. Entire function body:

```c
void Obj865C8__Noop80(void) {
}
```

A no-op BODY is not evidence the SLOT's signature takes no arguments (CLAUDE.md) -- the slot itself stays typed with its full signature in the header even though this occupant's own C ignores both parameters. Never had its own report before this round's rename; created here per the one-report-per-function rule.


## Naming

`Obj865C8__Noop80` -- tier A. Empty body (`{}`), occupies +0x080; dispatched from `Obj865C8__OnNotify` with real `EventArg`/tag arguments that this occupant simply ignores. Empty body is direct evidence; CLAUDE.md's caveat that a no-op body is not evidence the SLOT takes no arguments is why the slot itself stays typed with its full signature.
