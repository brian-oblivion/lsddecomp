# CloseCdFile -- MATCHED (7/7 words)

> Renamed from `func_80028A34` on 2026-09-21 (tools/rename.py). Address 0x80028a34.

Unit: `code_179d8_h`. Runner: echo, round 17 (second assignment).

## Result

```c
void CloseCdFile(ObjA34_179D8H *self) {
    if (self->isOpen != 0) {
        self->isOpen = 0;
    }
}
```

with a new local `ObjA34_179D8H` type (see `GetCdFileSize`'s report for why
it's local rather than an extension of `code_171e0.h`'s
`Class6D430`, whose `pendingGeneration` field and documented-unknown
`pad18` padding this struct's fields happen to coincide with).

Byte-exact, 7/7 words.

## Notes

Simple guarded clear: only writes `self->isOpen = 0` (field renamed round
64, was `unk0C`) when it was already nonzero (matches the `beqz`-skip-the-
store shape exactly; an unconditional `self->isOpen = 0;` would compile
without the branch at all).

## Naming (round 64, runner alpha)

- `func_80028A34` -> `CloseCdFile`, tier B. `src/code_179d8_s.c`'s
  `func_80027480` calls this function directly when CD-async mode is off,
  and otherwise (async path) does the same `self->unk0C = 0` clear plus
  `StartCdOperation(0, 0)`/`ResetCdStateMachine()` -- i.e. this is the
  sync-mode half of a close/cancel operation. Paired with `OpenCdFile`/
  `GetCdFileSize`/`ReadCdFile` (also this unit) as an Open/Close/Size/Read
  quad; see `OpenCdFile`'s report and this unit's header comment.
- `unk0C` -> `isOpen` (tier B): see `GetCdFileSize.md`'s `## Naming` for the
  full cross-unit evidence (`src/code_179d8_s.c`'s independent async
  reimplementation sets/clears the identical offset around the identical
  CdSearchFile/CdControl+CdSync sequence).
