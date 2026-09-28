# Actor__MoveAlongLocalAxis -- MATCHED (31/31)

> Renamed from `DreamSys__ApplyOffsetSlotAndNotify` on 2026-09-25 (tools/rename.py). Address 0x80057534.

> Renamed from `func_80057534` on 2026-09-19 (tools/rename.py). Address 0x80057534.

Unit: `src/world/dream_scene.c`. Class: `DreamSys` (self+0, vtable
`vtable_DreamSys`, resolved via `tools/classtable.py gDreamSysMethods`).

## Signature

```c
void Actor__MoveAlongLocalAxis(DreamSys *self, s16 *slot, s32 val, void *extra, volatile s32 count);
```

Called only by this unit's own `Actor__MoveLocalX`/`Actor__MoveLocalY` (its own
`+0x0C8`/`+0x0CC` vtable slots).

## Body

```c
void Actor__MoveAlongLocalAxis(DreamSys *self, s16 *slot, s32 val, void *extra, volatile s32 count) {
    s16 val16 = (s16) val;
    *slot = val16;
    self->lastOffsetValue = val16;
    self->vt->Actor__AddLocalTranslation(self, &sActorLocalMove[0]);
    *slot = 0;
    if (extra != NULL) {
        self->vt->DreamSys__NotifyLinkAttempt(self, count);
    }
}
```

Writes `val` (truncated to 16 bits) into `*slot` and into
`self->lastOffsetValue`, dispatches through the shared base table's `+0x0C0`
slot (`Actor__AddLocalTranslation`, resolves to `Actor__MoveLocalZ`'s neighbour -- out of
this unit's range, see `include/dream_sys.h`) with a hardcoded
`&sActorLocalMove[0]` argument (always the FIRST element, regardless of which
`slot` was written), unconditionally resets `*slot` to 0, then -- only if
`extra` is non-NULL -- dispatches through `+0x088`
(`DreamSys__NotifyLinkAttempt`, already named in `include/dream_sys.h`) with `count`.

The `*slot = 0` reset is unconditional even though it reads as though it
belongs to the `if`: retail schedules it into the `beqz`'s delay slot,
which always executes. This is ordinary instruction scheduling, not a
control-flow difference -- reproduced by plain statement order (`*slot=0;`
followed by `if (extra) ...`).

## Two things that were NOT obvious from the disassembly alone

1. **`count` must be `volatile`.** `count` is the function's 5th
   (stack-passed) argument, read only once, after the intervening
   `Actor__AddLocalTranslation` call. Without `volatile`, GCC 2.6.3 -O2 promotes it to a
   callee-saved register (`s3`) at function entry -- unconditionally,
   confirmed with a standalone reproducer through the pinned toolchain
   (`tools/gcc263/cpp | cc1 | maspsx | as`) -- growing the frame from
   `0x20` to `0x28` bytes and shifting every subsequent stack offset.
   Retail instead re-reads it directly from the stack
   (`lw $a1, 0x30($sp)`) at the one use site. `volatile` is the only
   source-level way found to suppress the promotion; it is ordinary C, not
   a register-pinning construct.

2. **`val` must be `s32` in the SIGNATURE, but truncated to a named `s16`
   local before either store.** Two variants were tried and rejected:
   - `s16 val` in the signature: matches THIS function's own body exactly
     (register order s2/s1/ra/s0, instruction order), but its two callers
     (`Actor__MoveLocalX`/`Actor__MoveLocalY`) then have to re-sign-extend their
     own incoming `s32` value into a `s16` argument at every call site
     (`sll`/`sra` pair) -- confirmed with a standalone reproducer that
     GCC 2.6.3 ALWAYS re-normalizes a `short` argument when forwarding it
     as another `short` argument, even a pure pass-through with no
     intervening computation. Retail's callers have NO such pair.
   - `s32 val` in the signature, truncated inline at each of the two
     `sh` stores (`*slot = (s16) val; self->lastOffsetValue = (s16) val;`):
     fixes the callers (no more forced truncation, since `s32`->`s32`
     needs no conversion), but SWAPS which of `s0`/`s1` holds `slot` vs
     `extra` inside this function -- register-identity residue, reproduced
     in isolation with a standalone `Vt`/`DS` reproducer.
   - The fix that matches BOTH ends: `s32 val` in the signature (so the
     callers pass it straight through, no truncation), but truncate ONCE
     into a named `s16 val16` local and store `val16` twice. This keeps
     the callers clean AND reproduces this function's own original
     register assignment. Verified with the same standalone reproducer
     before applying to `src/`.

## Naming

**`Actor__MoveAlongLocalAxis` -- tier B.** Mechanics are fully
confirmed and are the whole of what's named: writes the truncated value
into the caller-supplied `slot` pointer AND into
`self->lastOffsetValue`, unconditionally applies the shared local-offset
buffer through the inherited `Actor__AddLocalTranslation`, resets
`*slot` back to 0, and -- only if `extra` is non-NULL -- forwards `count`
through `self->vt->DreamSys__NotifyLinkAttempt` (a notify/dispatch call, already named
in `include/dream_sys.h` but not yet given a friendly name by that slot's
own owning unit). "AndNotify" covers that conditional tail without
asserting what the notification means.

## Verify

```
./build-and-verify.sh   # build exit=0, OK: build matches retail
tools/funcdiff.py Actor__MoveAlongLocalAxis   # 31/31
```

### Proposed learning

**A stack-passed scalar argument used only after an intervening call needs
`volatile` to avoid a spurious callee-saved-register promotion.** GCC
2.6.3 -O2 promotes ANY stack argument referenced anywhere in the function
to a hard register at entry, unconditionally, the moment its live range
crosses a call -- confirmed with an isolated reproducer through the pinned
toolchain (drop `volatile`, the frame grows by one register). Marking the
parameter `volatile` forces a direct stack reload at the use site instead,
matching retail's shape when retail does not cache it. This is a new,
more general case of the existing "cache a `this->field` across a call ->
spurious promotion" learning in `docs/DECOMPILATION_LEARNINGS.md` -- same
mechanism, but for a plain scalar PARAMETER rather than a struct field
read, where there is no "re-read at each use site" option because a
parameter has no address to re-read from other than its own already-single
use site.

**A parameter typed narrower than its call-site value costs an
instruction pair EVERY time it is forwarded, even as a pure pass-through
with no intervening computation.** GCC 2.6.3 unconditionally
re-sign-extends a `short`-typed argument at every call site where it is
passed as another `short` argument, regardless of whether the source
register can be proven already sign-extended. If a value is read from a
wider (`s32`) incoming register and only NEEDS 16 bits at one or two store
sites deep inside the callee, keep the PARAMETER `s32` all the way through
the call chain and truncate with a single named `s16` local at the store
site(s) -- do not narrow the parameter type itself, and do not truncate
inline at each store separately (that second form reproduces the value
correctly but can still perturb unrelated register allocation elsewhere
in the same function; truncate once into a named local instead).

## Track 4 (2026-09-25, round 82, delta)

Renamed from `DreamSys__ApplyOffsetSlotAndNotify`. The body behind the three moves: store val in one component (`axis`) of the local move vector sActorLocalMove, keep it in lastOffsetValue, addLocalTranslation (+0x0C0) the whole vector, clear the component, and when `notify` is non-NULL call notifyIfUnk20Active (+0x088) with the move's event (6 z, 7 x, 8 y). DreamSys__NotifyLinkAttempt was DreamSys's override of that slot, not the slot. The class (id 0x34, table `gActorMethods`, formerly `D_800878D4`) is unified as `Actor` in `include/Actor.h`: a SceneNode subclass and the base of TodActor/Entity, DreamSys and StyleEffect. Any source block above is the pre-unification spelling; the live body in `src/world/dream_scene.c` takes the unified types and field/slot names, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 7 (2026-09-27, round 96, bravo)

Comments quoted below are verbatim as the file stood before this round's
comment pass, i.e. with this round's renames already applied (the
`LinkQueryBuf` one as it stood before step 2).

- **`volatile` dropped from `event`** (here and in `include/Actor.h`'s
  prototype). It was never needed: `event` is the fifth argument, so it
  arrives on the caller's stack and GCC loads it there at its one use.
  Measured: 31/31 and the whole image byte-identical without it. The
  "dropping `volatile` grows the frame" note below predates the current
  body and no longer reproduces.
- The `val16` local IS still needed (measured this round: storing `val`
  directly at both sites changes the function's length). It keeps one
  `/* MATCHING: */` line.

The function comment, which carried the derivation, verbatim (its `count`,
`slot` and `extra` are names from an earlier body):

```c
/* `count` is `volatile` so it stays a stack reference reloaded at its one use
 * site, rather than being promoted to a callee-saved register across the
 * intervening Actor__AddLocalTranslation call -- confirmed with a standalone reproducer
 * through the pinned toolchain: dropping `volatile` grows the frame by one
 * callee-saved register (s3) and changes 0x1c/0x20 byte offsets throughout,
 * which is not what retail does (round 2026-09-04).
 *
 * `val` arrives as `s32` (its callers forward an incoming register with no
 * conversion -- typing it `s16` here made the CALLERS re-sign-extend it on
 * every call, which retail does not do), but the two stores below are
 * genuinely 16-bit (`sh`). Truncating once into a local `s16` and storing
 * THAT (rather than truncating `val` twice inline) is what reproduces
 * retail's callee-saved register assignment for `slot`/`extra`
 * (confirmed with a standalone reproducer: inline truncation swaps which
 * of s0/s1 holds which, round 2026-09-04). */
```
