# Actor__ScanGridWindow -- MATCHED (79/79)

> Renamed from `DreamSys__ScanGridWindow` on 2026-09-25 (tools/rename.py). Address 0x80057a18.

> Renamed from `func_80057A18` on 2026-09-19 (tools/rename.py). Address 0x80057a18.

Unit: `src/class_3bb8c_p.c`. Class: `DreamSys` family -- plain internal
helper, not a vtable slot. Called by this unit's own `Actor__ScanLinkCandidates` (see
its report) and, indirectly, `Actor__FindNearbyLink` (still queued at the time
this was written).

## Signature

```c
void *Actor__ScanGridWindow(DreamSys *self, void *arg1, void *arg2, GridQuery *query, GridArrElem *source);
```

`self` is read from `a0` in the disassembly and then never touched again
-- present only to match the caller's convention, not used by this
function's own body.

## Body

```c
void *Actor__ScanGridWindow(DreamSys *self, void *arg1, void *arg2, GridQuery *query, GridArrElem *source) {
    s32 row, col;
    GridElem **bucket;

    bucket = (GridElem **) ((u8 *) source->buckets + query->startRow * 0x50 + query->startCol * 4);
    for (row = 0; row < query->numRows; row++) {
        for (col = 0; col < query->numCols; col++) {
            GridElem *node;

            if (AcceptGridElem(*bucket, arg1, arg2) != NULL) {
                return *bucket;
            }
            for (node = (*bucket)->next; node != NULL; node = node->next) {
                if (AcceptGridElem(node, arg1, arg2) != NULL) {
                    return node;
                }
            }
            bucket++;
        }
        bucket = (GridElem **) ((u8 *) bucket - (query->numCols * 4 + 0x50));
    }
    return NULL;
}
```

Scans a rectangular window of a grid of bucket lists rooted at
`source->buckets`: `query->numRows` rows by `query->numCols` columns, starting
at row `query->startRow`, column `query->startCol` (row stride `0x50` bytes = 20
bucket-head pointers; column stride 4 bytes, one pointer). For each
bucket, tries `AcceptGridElem` (already matched, this unit) against the
head first, then walks the linked chain (`GridElem::next`, at `+0x38` --
the same "self-typed next pointer at +0x38" idiom already established
for the unrelated `EntryChildObj` in `include/class_3bb8c.h`, per this
round's own research; convergent shape, not a shared type). Returns the
first element `AcceptGridElem` accepts, or NULL if the whole window comes
up empty.

## Shape note: do NOT cache the bucket head in a local

```c
GridElem *head = *bucket;
if (AcceptGridElem(head, ...) != NULL) { return head; }
for (node = head->next; ...)
```
scored 27/79 and grew the function -- retail re-dereferences `*bucket`
FRESH at every use (once before the call, again for the return value,
again to compute `->next`) rather than caching it in a register. Same
"do not cache across a call" family as `Actor__MoveAlongLocalAxis`'s report, but for
a plain pointer dereference rather than a `this->field` read. Rewriting
without the `head` local -- `*bucket` written out at each of the three
use sites -- matched exactly.

## Naming

**`Actor__ScanGridWindow` -- tier B.** Mechanics fully confirmed
(79/79): walks a `numRows` x `numCols` rectangular window of a bucketed
grid of `GridElem` linked lists, rooted at `source->buckets`, starting at
`(startRow, startCol)`, testing each element via `AcceptGridElem`
and returning the first accepted one. "Grid window" names the
`GridQuery`-described rectangle this function actually iterates.

## Verify

```
./build-and-verify.sh   # build exit=0, OK: build matches retail
tools/funcdiff.py Actor__ScanGridWindow   # 79/79
```

### Proposed learning

**Extends the existing "do not cache a `this->field` across an
intervening call" learning to plain pointer dereferences with no
intervening call at all.** Here `*bucket` is re-read three separate times
with NO function call between the first two of them -- caching it in a
local still cost a spurious register and grew the function. The lesson
generalises past "across a call": GCC 2.6.3 can prefer re-dereferencing a
cheap, unchanging pointer at each use site over holding the dereferenced
VALUE in a register, even with nothing between the uses that could
invalidate the cache. When a residue looks like a spurious extra
register tied to a value used more than once, try the un-cached,
re-dereferenced form before assuming a call is required to trigger it.

## Track 4 (2026-09-25, round 82, delta)

Renamed from `DreamSys__ScanGridWindow`. Helper of Actor__ScanLinkCandidates (self unused). The class (id 0x34, table `gActorMethods`, formerly `D_800878D4`) is unified as `Actor` in `include/Actor.h`: a SceneNode subclass and the base of TodActor/Entity, DreamSys and StyleEffect. Any source block above is the pre-unification spelling; the live body in `src/class_3bb8c_p.c` takes the unified types and field/slot names, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
