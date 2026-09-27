# GetVabUseVSyncCallback -- MATCHED (2/2 words)

> Renamed from `func_8002C478` on 2026-09-27 (tools/rename.py). Address 0x8002c478.

Unit: `code_179d8_e`. Runner: echo, round 17.

## Result

```c
s32 GetVabUseVSyncCallback(void) {
    return 0;
}
```

Byte-exact, 2/2 words (`jr $ra; addu $v0,$zero,$zero`) -- identical body to
`VabDriver__Read` but a different, unrelated function.

## Notes

Not a slot in either `gVabDriverMethods` or `gVabStreamObjMethods` (checked
`tools/classtable.py --scan` output for both tables -- absent from each).
Its only caller is `code_171e0/GetActiveDataSourceUseVSyncCallback.s`, which `jal`s it with no
argument register set up, and its return value flows straight through as
the caller's own return. `s32` chosen over `void` because the body
explicitly materialises `$v0 = 0` before returning -- CLAUDE.md's
tail-call-ambiguity note doesn't strictly apply here (there's no tail
call), but the explicit `addu $v0,$zero,$zero` is the same evidence: a
`void` function has no reason to touch `$v0` at all.

**Round-52 correction:** the line above used to read "a fallback arm in
what looks like a per-class-ID constructor dispatcher, alongside
`func_8002C438` for a different ID" -- `tools/rename.py` textually carried
that name through its later rename to `GetVabDriverMethods`, but the
CLAIM itself was never right. `code_171e0.c`'s actual source (read in full
this round, not just the one `.s` file) shows `func_80026FE8`'s caller
context is: `if (gActiveDataSource == 0x13) return GetCdUseVSyncCallback(); else
return GetVabUseVSyncCallback();` -- this function is the `else` arm alongside
`GetCdUseVSyncCallback` (still unnamed), not `func_8002C438`/`GetVabDriverMethods`
(that one is `func_80026CAC`'s own accessor pair, a different dispatcher
entirely). See the unit header comment and this unit's `GetVabDriverMode`/
`SetVabDriverMode` reports for the real family this function belongs to.

## Naming

Kept `GetVabUseVSyncCallback`, tier C. It's the same generic driver-mode-interface
family as `GetVabDriverMode`/`SetVabDriverMode` (this backend's own
implementation of whatever `func_80026FE8` needs when
`gActiveDataSource != 0x13`), but its own counterpart `GetCdUseVSyncCallback` is
still unnamed, so there's no established purpose to name this AS a
stand-in for -- naming it "GetVabDriverState" or similar would assert
purpose from a body that's just `return 0;`.

### Round 98 (charlie, track 7): `func_8002C478` -> `GetVabUseVSyncCallback`, tier A

The objection above ("its own counterpart is still unnamed") no longer
holds: the counterpart is `GetCdUseVSyncCallback` (code_179d8_h.c, returns
`gCdUseVSyncCallback`), and the one caller, code_171e0.c's
`GetActiveDataSourceUseVSyncCallback`, calls it when `gActiveDataSource` is
DATASOURCE_CD and this function otherwise. This is the VAB driver's answer
to the same query, and the answer is a constant 0: the VAB backend never
uses a VSync callback. Named on the `GetVab*`/`GetCd*` pattern
`GetVabDriverMode`/`GetCdDriverMode` already follow; a leaf returning a
constant, tier A.
