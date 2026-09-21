# Class6D430__InstallCdReadDriver -- MATCHED (18/18 words)

> Renamed from `func_80028898` on 2026-09-21 (tools/rename.py). Address 0x80028898.

Unit: `code_179d8_h`. Runner: echo, round 17 (second assignment).

## Result

```c
void Class6D430__InstallCdReadDriver(Class6D430 *self) {
    ((Class6D430Methods *)GetClass6D430Methods())->ctor(self);
    self->methods = GetClass6D4E8Methods();
    self->pendingGeneration = 0;
}
```

(`self->unk0C` above was `code_171e0.h`'s own field, renamed to
`pendingGeneration` by that unit's owner before this round; this report's
code sample was stale and is corrected here, round 64.)

with `#include "code_171e0.h"` (already-established header, reused
UNCHANGED -- not copied or redefined) and a new local
`extern Class6D430Methods *GetClass6D4E8Methods(void);`.

Byte-exact, 18/18 words.

## Notes

The standard "chain to base ctor, then install the derived vtable" idiom:
calls `D_8006D430`'s own ctor slot (`GetClass6D430Methods()->ctor`, i.e.
`Class6D430__Class6D430`, matched in `code_171e0.c`) directly rather than through
`self->methods` (since `self->methods` isn't set up yet), then overwrites
`self->methods` with `GetClass6D4E8Methods()` -- a DIFFERENT class table
(`D_8006D4E8`, confirmed via `tools/classtable.py --scan`: 29 slots, header
`0x13`, vs. `D_8006D430`'s 0x03). `GetClass6D4E8Methods` itself is still uncarved
(`asm/code_179d8.s`); typed against `Class6D430Methods` for the
assignment only -- the two classes are different but share the base's
leading slot layout, which is all the type is asked to express here.

Needed an explicit cast (`(Class6D430Methods *)`) on
`GetClass6D430Methods()`'s result: it returns plain `void *` (per its own
established signature in `code_171e0.c`), so `->ctor` on the bare call
doesn't compile without one -- first attempt failed with `request for
member 'ctor' in something not a structure or union`.

### Proposed learning

`code_171e0.h`'s `Class6D430`/`Class6D430Methods` describe a
class that OTHER units' functions construct/chain into, not just
`code_171e0.c`'s own methods -- worth checking this header before
redefining a local struct whenever a function dispatches through
`GetClass6D430Methods()` or receives a `self` whose fields line up with its
offsets. `GetClass6D430Methods()` itself returns bare `void *`, so every external
call site needs its own cast to the slot-bearing type; this is not
`code_171e0.c`'s problem to fix (its own call sites go through
`this->methods`, already correctly typed).

## Naming (round 64, runner alpha)

`func_80028898` -> `Class6D430__InstallCdReadDriver`, tier B. `self` is
`Class6D430*`, the header's own established type (this match is byte-exact
against that typing) -- so `Class6D430__` follows track 3's convention
letter-for-letter ("methods `Class__Method`, where `Class` is the struct's
type name"). The rest of the name describes only confirmed MECHANICS: chain
to `Class6D430`'s own ctor, then overwrite `self->methods` with
`GetClass6D4E8Methods()`'s table -- `D_8006D4E8`, independently named
elsewhere in the tree (`src/code_179d8_q.c`'s own header comment) as "the
CD-ROM read driver", not a guess coined here. WHICH broader class or game
subsystem this function itself belongs to (why a `Class6D430` instance gets
reclassified this way here, distinct from `src/code_179d8_o.c`'s own
confirmed ctor `func_80027228` for the same `D_8006D4E8` class) is NOT
established -- no caller is visible yet (only referenced from the
still-uncarved `code_179d8` remainder) and this is flagged as such rather
than guessed at. See `Class6D430__DestroyCdReadDriver.md` for the paired
dtor.
