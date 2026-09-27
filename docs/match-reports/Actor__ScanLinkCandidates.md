# Actor__ScanLinkCandidates -- MATCHED (49/49)

> Renamed from `DreamSys__ScanLinkCandidates` on 2026-09-25 (tools/rename.py). Address 0x80057954.

> Renamed from `func_80057954` on 2026-09-19 (tools/rename.py). Address 0x80057954.

Unit: `src/class_3bb8c_p.c`. Class: `DreamSys` family -- plain internal
helper, not a vtable slot.

## Signature

```c
void *Actor__ScanLinkCandidates(DreamSys *self, void *arg1, void *arg2, s32 count, GridQuery *arr1, GridArrElem **arr2);
```

Called only by this unit's own (still-queued at the time this was
written) `Actor__FindNearbyLink`.

## Body

```c
void *Actor__ScanLinkCandidates(DreamSys *self, void *arg1, void *arg2, s32 count, GridQuery *arr1, GridArrElem **arr2) {
    s32 i;

    for (i = 0; i < count;) {
        GridArrElem *elem = *arr2;
        i++;
        if (elem->info->enabled != 0) {
            void *result = Actor__ScanGridWindow(self, arg1, arg2, arr1, elem);
            if (result != NULL) {
                return result;
            }
        }
        arr1 = (GridQuery *) ((u8 *) arr1 + 0xC);
        arr2++;
    }
    return NULL;
}
```

Walks `count` entries of two parallel arrays -- `arr1` (`GridQuery[]`,
stride `0xC`) and `arr2` (`GridArrElem *[]`, stride 4) -- skipping any
entry whose `GridArrElem->info->enabled` flag is zero, calling
`Actor__ScanGridWindow` (this unit's own, see its report) on the rest, and
returning the first non-NULL result.

New local types introduced (shared with `Actor__ScanGridWindow`, declared once
above this function): `GridQuery` (12-byte `{s16,s16,s32,s32}` record,
built by this unit's own still-queued `Actor__BuildLinkQueries`), `GridArrElem`/
`GridArrElemInner` (only the two fields actually read are named).

## Shape note: `i++` must be the loop's FIRST statement, not its
increment clause

Three loop shapes were tried:
- `for (i = 0; i < count; i++) { ... }`: 46/49, with the `i++` and the
  two array-pointer advances (`arr1 += 0xC; arr2++;`) all present but in
  the WRONG relative order -- retail schedules `i++` immediately after
  dereferencing `*arr2` (right when the flag is tested), well before the
  conditional call, while the array-pointer advances stay at the end.
  GCC's own scheduler evidently treats a `for`-header increment as tied
  to the loop's tail, not free to float earlier.
- `do { ...; i++; ... } while (i < count);` wrapped in `if (count > 0)`:
  worse (22/49) -- lost a whole callee-saved register and swapped which
  of two others held the loop counter vs. an array pointer; the
  bottom-tested form changes register pressure enough to reassign
  everything.
- `for (i = 0; i < count;) { GridArrElem *elem = *arr2; i++; ...; }`
  (empty `for`-header increment clause, `i++` moved to be the SECOND
  statement of the body): exact match. Keeps the top-tested `for` shape
  (so `count <= 0` skips the whole loop via a single `blez`, matching
  retail) while placing `i++` where retail's scheduler put it.

## Naming

**`Actor__ScanLinkCandidates` -- tier B.** Mechanics fully confirmed
(49/49): walks `count` paired `GridQuery`/`GridArrElem*` entries,
skipping any whose `info->enabled` flag is zero, testing the rest via
`Actor__ScanGridWindow`, returning the first non-NULL result. Named
for exactly this "scan a set of link candidates" mechanic, which is
`Actor__BuildLinkQueries`'s own output.

## Verify

```
./build-and-verify.sh   # build exit=0, OK: build matches retail
tools/funcdiff.py Actor__ScanLinkCandidates   # 49/49
```

### Proposed learning

**A `for`-loop's own increment clause is not interchangeable with the
same statement written as the body's second line, even though they
execute at "the same point" in the loop's logical iteration.** GCC 2.6.3
schedules a `for`-header increment as fixed to the loop's tail; the
IDENTICAL statement written explicitly inside the body (right after the
value it doesn't depend on) is free to be scheduled into an earlier delay
slot. When a counter increment and unrelated array-pointer advances need
to end up in DIFFERENT positions relative to a conditional call inside
the loop, only the body-statement form can separate them -- the
`for`-header form ties them together at the tail.

## Track 4 (2026-09-25, round 82, delta)

Renamed from `DreamSys__ScanLinkCandidates`. Helper of Actor__FindNearbyLink. The class (id 0x34, table `gActorMethods`, formerly `D_800878D4`) is unified as `Actor` in `include/Actor.h`: a SceneNode subclass and the base of TodActor/Entity, DreamSys and StyleEffect. Any source block above is the pre-unification spelling; the live body in `src/class_3bb8c_p.c` takes the unified types and field/slot names, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 7 (2026-09-27, round 96, bravo)

Comments quoted below are verbatim as the file stood before this round's
comment pass, i.e. with this round's renames already applied (the
`LinkQueryBuf` one as it stood before step 2).

- `arr1 = (GridQuery *)((u8 *)arr1 + 0xC)` became `queries++`
  (byte-identical, 49/49). Parameters `arg1`/`arg2`/`arr1`/`arr2` ->
  `offset`/`pos`/`queries`/`slots` (AcceptGridElem's and
  SceneNode__RaycastVertical's roles), local `elem` -> `slot`.
- The `i++` placement keeps one `/* MATCHING: */` line.

The function comment, verbatim:

```c
/* Walks `count` entries of `arr1` (a `GridQuery[]`, stride 0xC) paired
 * element-for-element with `arr2` (a `ChunkSlot *[]`, stride 4),
 * skipping any entry whose element's loader does not have `headerReady`
 * set, and calling `Actor__ScanGridWindow` on the rest; returns the first
 * non-NULL result, or NULL if every entry was skipped or came back empty
 * (round 2026-09-04). */
```
