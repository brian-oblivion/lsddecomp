# CdDriver__Finalize -- MATCHED (21/21 words)

> Renamed from `Class6D4E8__Destroy` on 2026-09-26 (tools/rename.py). Address 0x80027274.

> Renamed from `func_80027274` on 2026-09-25 (tools/rename.py). Address 0x80027274.

Unit `code_179d8_o`, round 26 (2026-09-09). A two-call vtable dispatcher on
`self`, both calls through `self->methods` (the class table set up by
`CdDriver__CdDriver`'s constructor).

## What it is

```c
void CdDriver__Finalize(Obj6D4E8 *self)   /* round-26 text; see "Naming" for the current source */
{
    self->methods->slot74();
    self->methods->slot5C(self);
}
```

Both dispatches reread `self->methods` fresh (two separate `lw`s in
retail, not a cached local) -- a direct translation of two independent
source-level statements, no caching needed since there is no call in
between that would force a saved register either way.

**Argument arity per slot was read off the register setup at each call
site, not assumed uniform.** The `+0x074` call's `jalr` carries a plain
`nop` delay slot with no register load anywhere above it -- a genuine
zero-argument call, so the slot is typed `void (*slot74)(void);`. The
`+0x05C` call three instructions later explicitly sets `$a0 = self` in its
own delay slot, so that slot is `void (*slot5C)(void *self);`. Per the
project's round-21 finding, "a `jalr` with a plain `nop` delay slot is not
proof of a zero-argument call" in general -- but the CONVERSE check (does
this specific call site load a register for the callee) is exactly what
distinguishes the two slots here, and both readings were confirmed by the
byte-exact result.

Matched on the first attempt.

## Naming

Round 79 (delta).

- **`CdDriver__Finalize`** (was `func_80027274`) -- **tier A**. Table slot
  +0x00C of gCdDriverMethods, overriding `FileResource__Finalize` in the parent
  table (and `BasicClass__Finalize` in the root). The body cancels this
  object's queued CD requests (+0x074, `CdDriver__CancelRequests`) and
  then frees its buffer (+0x05C, inherited `FileResource__FreeBuffer`) -- the
  same two-dispatch shape as `FileResource__Finalize` (`onBufferChanged` then
  `freeBuffer`). Named after the slot it overrides, following the parent.
- Slots, in the unit's local `Class6D4E8Methods` view (only this unit
  accesses it, so renamed directly; both oracles green): `slot74` ->
  `cancelRequests`, `slot5C` -> `freeBuffer` (the parent header's own name
  for +0x05C), `unk04` -> `ownDtorChain` (the parent header's name for
  +0x004).

### Round 79 correction: the +0x074 call passes `self`

The section above types `slot74` as `void (*)(void)` because its `jalr` has
a nop delay slot. That reading is wrong about the callee: at that `jalr`,
`$a0` still holds this function's own `self` (the prologue's `move s0,a0`
copies it and leaves `$a0` intact), and `CdDriver__CancelRequests`
(code_179d8_q.c) reads `self` from `$a0`. So the call does pass `self`.
Retyped to `void (*cancelRequests)(void *self)` and called as
`self->methods->cancelRequests(self)`: **byte-identical** (build exit 0,
SHA1 OK, check-nonmatching green). `FileResource__Finalize` in GameApplicationFileResource.c
has the identical compiled shape with `this` passed explicitly, which is
the precedent. The nop delay slot said only that no argument register
needed LOADING, not that none was read.

### Proposed learning

- A `jalr` whose delay slot is a `nop`, in a function that saved `$a0`
  into an s-register with `move sN,a0` and has not called anything since,
  is still passing its own first argument. Check the callee before typing
  the slot zero-argument.


Track 4, 2026-09-26 (round 88). The class of gCdDriverMethods (was D_8006D4E8, id 0x13 = DATASOURCE_CD) is CdDriver, in include/CdDriver.h: its ctor calls InitCdDrive, its slots enqueue CD_OP_* requests and drive the CD read state machine, and it is VabDriver's sibling. The object views this function was typed against are replaced by CdDriver, whose fields are all FileResource's (the driver runs on its clients' objects; FileResource's +0x018/+0x01C were named pos/size for it). Byte-identical. `Class6D4E8__Destroy` -> `CdDriver__Finalize` by rename.py. Renamed from Destroy for its slot, +0x00C finalize (track 4 step 6): the body cancels the object's requests and frees its buffer, which is what the slot does.
