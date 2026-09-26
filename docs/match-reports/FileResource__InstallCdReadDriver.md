# FileResource__InstallCdReadDriver -- MATCHED (18/18 words)

> Renamed from `FileResource__InstallCdReadDriver` on 2026-09-26 (tools/rename.py). Address 0x80028898.

> Renamed from `func_80028898` on 2026-09-21 (tools/rename.py). Address 0x80028898.

Unit: `code_179d8_h`. Runner: echo, round 17 (second assignment).

## Result

```c
void FileResource__InstallCdReadDriver(FileResource *self) {
    ((FileResourceMethods *)GetFileResourceMethods())->ctor(self);
    self->methods = GetCdDriverMethods();
    self->pendingGeneration = 0;
}
```

(`self->unk0C` above was `code_171e0.h`'s own field, renamed to
`pendingGeneration` by that unit's owner before this round; this report's
code sample was stale and is corrected here, round 64.)

with `#include "code_171e0.h"` (already-established header, reused
UNCHANGED -- not copied or redefined) and a new local
`extern FileResourceMethods *GetCdDriverMethods(void);`.

Byte-exact, 18/18 words.

## Notes

The standard "chain to base ctor, then install the derived vtable" idiom:
calls `gFileResourceMethods`'s own ctor slot (`GetFileResourceMethods()->ctor`, i.e.
`FileResource__FileResource`, matched in `code_171e0.c`) directly rather than through
`self->methods` (since `self->methods` isn't set up yet), then overwrites
`self->methods` with `GetCdDriverMethods()` -- a DIFFERENT class table
(`gCdDriverMethods`, confirmed via `tools/classtable.py --scan`: 29 slots, header
`0x13`, vs. `gFileResourceMethods`'s 0x03). `GetCdDriverMethods` itself is still uncarved
(`asm/code_179d8.s`); typed against `FileResourceMethods` for the
assignment only -- the two classes are different but share the base's
leading slot layout, which is all the type is asked to express here.

Needed an explicit cast (`(FileResourceMethods *)`) on
`GetFileResourceMethods()`'s result: it returns plain `void *` (per its own
established signature in `code_171e0.c`), so `->ctor` on the bare call
doesn't compile without one -- first attempt failed with `request for
member 'ctor' in something not a structure or union`.

### Proposed learning

`code_171e0.h`'s `FileResource`/`FileResourceMethods` describe a
class that OTHER units' functions construct/chain into, not just
`code_171e0.c`'s own methods -- worth checking this header before
redefining a local struct whenever a function dispatches through
`GetFileResourceMethods()` or receives a `self` whose fields line up with its
offsets. `GetFileResourceMethods()` itself returns bare `void *`, so every external
call site needs its own cast to the slot-bearing type; this is not
`code_171e0.c`'s problem to fix (its own call sites go through
`this->methods`, already correctly typed).

## Naming (round 64, runner alpha)

`func_80028898` -> `FileResource__InstallCdReadDriver`, tier B. `self` is
`FileResource*`, the header's own established type (this match is byte-exact
against that typing) -- so `FileResource__` follows track 3's convention
letter-for-letter ("methods `Class__Method`, where `Class` is the struct's
type name"). The rest of the name describes only confirmed MECHANICS: chain
to `FileResource`'s own ctor, then overwrite `self->methods` with
`GetCdDriverMethods()`'s table -- `gCdDriverMethods`, independently named
elsewhere in the tree (`src/code_179d8_q.c`'s own header comment) as "the
CD-ROM read driver", not a guess coined here. WHICH broader class or game
subsystem this function itself belongs to (why a `FileResource` instance gets
reclassified this way here, distinct from `src/code_179d8_o.c`'s own
confirmed ctor `CdDriver__CdDriver` for the same `gCdDriverMethods` class) is NOT
established -- no caller is visible yet (only referenced from the
still-uncarved `code_179d8` remainder) and this is flagged as such rather
than guessed at. See `FileResource__DestroyCdReadDriver.md` for the paired
dtor.
