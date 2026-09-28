# TaskObjF__OpenEvents — MATCH (37/37 words)

> Renamed from `func_8004E5E4` on 2026-09-24 (tools/rename.py). Address 0x8004e5e4.

**Unit:** class_3bb8c_c (round 14, `Node3bb8cE` class).

## What it does

`s32 TaskObjF__OpenEvents(Node3bb8cE *self)`. Enters a critical section
(`func_80024CE0`), starts 4 PSX threads via `func_80038F7C` (an
`OpenTh`-style call: fixed mode `0xF4000001`, entry point from
`gCardEventSpecs[i]`, stack size `0x2000`, priority `0`), storing each returned
handle into `self->threads[i]` (new field, offsets `+0x014`..`+0x020`),
leaves the critical section (`func_80024CF0`), calls `TaskObjF__EnableEvents(self)`
and returns `1` unconditionally.

## Where it stood, and the fix

Reached 35/37 immediately using an explicit `s32 *slot = self->threads;`
walking pointer incremented each iteration — this pre-computes the
`self + 0x14` base ONCE (an `addiu` outside the loop) and stores through
it at offset 0 each time. Retail does the OPPOSITE: it keeps a pointer
equal to plain `self` (no offset baked into the register), advances THAT
by 4 bytes per iteration, and bakes the `+0x14` field offset into every
store's own immediate field instead. Two words differed (the initial
pointer setup and the first store's immediate), zero drift.

Fixed by walking a `Node3bb8cE *` copy of `self` itself (not a `threads`
sub-pointer) and always indexing the FIRST array element on the shifting
base:

```c
s32 TaskObjF__OpenEvents(Node3bb8cE *self)
{
    s32 i;
    Node3bb8cE *cur;

    func_80024CE0();
    i = 0;
    cur = self;
    do {
        cur->threads[0] = func_80038F7C(0xF4000001, gCardEventSpecs[i], 0x2000, 0);
        i++;
        cur = (Node3bb8cE *)((u8 *)cur + 4);
    } while (i < 4);
    func_80024CF0();
    TaskObjF__EnableEvents(self);
    return 1;
}
```

### Header/struct change

Added `s32 threads[4];` at `+0x014` to `Node3bb8cE` (local to
`src/class_3bb8c_c.c`, not the shared header), replacing what had been
undifferentiated padding there.

### Proposed learning

**When a loop stores through a struct field at a FIXED offset while
visibly incrementing the BASE pointer by less than that offset (here 4
bytes, for a field at +0x14), don't introduce a separate pre-offset
pointer variable.** Walk a copy of the ORIGINAL base pointer instead and
always index the target field at its own fixed (typically zero) position
relative to the shifting base. Introducing an explicit `&self->field`
sub-pointer instead computes the offset once outside the loop (one extra
`addiu`) and bakes a zero displacement into every store, rather than
reproducing retail's "cheap base register + per-store fixed immediate"
shape. This is a new variant of the already-recorded "explicit
intermediate element pointer" family — here the fix is the opposite
direction: NOT introducing the intermediate pointer.

## Naming (round 78, track 3)

`func_8004E5E4` -> `TaskObjF__OpenEvents`. **Tier A.** Sits at `gTaskObjFMethods` +0x044. Opens 4 PSX kernel events (`OpenEvent`) into `self->events[0..3]` (matches `TaskObjF::events`, include/class_3bb8c.h, at the identical +0x014 offset -- corroborated cross-unit since round 60, see TaskObjF__EnableEvents.md) inside a critical section, then calls `TaskObjF__EnableEvents(self)`. Paired with `TaskObjF__CloseEvents`.

## Constants (round 98, track 7)

`OpenEvent(0xF4000001, spec, 0x2000, NULL)` is
`OpenEvent(SwCARD, spec, EvMdNOINTR, NULL)` from `<kernel.h>` (`SwCARD` is
`DescSW | 0x01`, the BIOS memory-card event class). `gCardEventSpecs`
(asm/data/76DC8.data.s, 0x80086E78) holds 0x4, 0x8000, 0x100, 0x2000:
`EvSpIOE`, `EvSpERROR`, `EvSpTIMOUT`, `EvSpNEW`, the spec
`WaitForReadyEvent` returns for the event that fired. (splat's
`gCardIconNames` label at 0x80086E80 falls inside that 4-word table; only
the first two words sit under `gCardEventSpecs`'s own dlabel.) The loop
bound is `ARRAY_COUNT(self->events)`. Zero bytes changed.

The unit used to declare its own `extern s32 gCardEventSpecs[4]` under a
comment calling it a "PSX thread-table constant ... one per OpenTh-style
thread it starts", with a note that its lui/addiu-then-lw walk is never
gp-relative. The table is event specs, not threads; the local declaration
is gone in favour of include/class_3bb8c.h's.
