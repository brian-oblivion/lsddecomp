# CloseCdFile -- MATCHED (7/7 words)

> Renamed from `func_80028A34` on 2026-09-21 (tools/rename.py). Address 0x80028a34.

Unit: `code_179d8_h`. Runner: echo, round 17 (second assignment).

## Result

```c
void CloseCdFile(ObjA34_179D8H *self) {
    if (self->unk0C != 0) {
        self->unk0C = 0;
    }
}
```

with a new local `ObjA34_179D8H` type (see `GetCdFileSize`'s report for why
it's local rather than an extension of `code_171e0.h`'s
`Class6D430`, whose `unk0C` field and documented-unknown `pad18`
padding this struct's fields happen to coincide with).

Byte-exact, 7/7 words.

## Notes

Simple guarded clear: only writes `self->unk0C = 0` when it was already
nonzero (matches the `beqz`-skip-the-store shape exactly; an unconditional
`self->unk0C = 0;` would compile without the branch at all).
