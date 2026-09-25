# Pad__Finalize -- MATCHED (21/21 words)

> Renamed from `Pad__Destroy` on 2026-09-25 (tools/rename.py). Address 0x80025c30.

> Renamed from `func_80025C30` on 2026-09-24 (tools/rename.py). Address 0x80025c30.

**Unit:** class_16334 · **Round:** 44 (2026-09-15)

## Provenance

Round-42's "REOPENED -- ASSIGNABLE" banner applies (previously stub-stalled as
`gp_rel`-blocked on the same `sPadRefCount` global as `Pad__Pad`). The
preserved body from the earlier runner/alpha attempt named the guarded call
as `func_80025F2C()`; re-reading the `.s` directly shows the call target is
`PadStop` (already declared in `include/class_16334.h`,
`void PadStop(void)`), not `func_80025F2C`. Matched byte-exact on the FIRST
build with the corrected call target.

## What it is

The Pad destructor: decrements the live-instance counter and, only when it
reaches zero, stops the Psy-Q Pad driver before tail-calling the inherited
base-class destructor through the method table.

## C

```c
void *Pad__Finalize(Pad *self) {
    if (--sPadRefCount == 0) {
        PadStop();
    }
    return Get_vtable_BasicClass()->dtor(self);
}
```

## Notes

`PadStop` takes no arguments (matches its header declaration); the delay slot
under the `bnez` only sets up `$s0 = self` for the later tail call, not an
argument for `PadStop`. No residue.

### Proposed learning

A preserved/inherited body's call target should be re-verified against the
`.s` directly rather than trusted from a prior report, even when the report
otherwise looks solid — here the classification of "which stall this is" was
right but the specific callee name (`func_80025F2C` vs. the already-known
`PadStop`) had drifted.

## Naming

**Tier A.** Occupies vtable slot `+0x0C`, BasicClass's `finalize`
(`classtable.py gPadMethods --vs D_8006B58C`), and forwards to
`BasicClass__Finalize` after stopping the pad library on the last instance.
Named `Pad__Destroy` in round 77 after `VabDriver__Destroy`; renamed
2026-09-25 (track 4, BasicClass unification) to `Pad__Finalize`, because an
override is named for the slot it occupies (FINISHING-PLAN track 4 recipe).
There was no convention to follow instead: of the 23 NAMED occupants of +0x00C
before this round, 8 said `Finalize`, 7 `Destroy`, 4 `Dtor`, 2 `Destructor`,
1 `Destruct`, 1 `Close`. Return type `void`, like the base it forwards to: the old `void *`
returned `finalize`'s result and was byte-identical either way.
