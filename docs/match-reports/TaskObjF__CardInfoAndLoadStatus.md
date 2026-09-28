# TaskObjF__CardInfoAndLoadStatus — MATCH (21/21 words)

> Renamed from `func_8004E77C` on 2026-09-24 (tools/rename.py). Address 0x8004e77c.

**Unit:** title_menu (round 14, `Node3bb8cE` class).

## What it does

`void TaskObjF__CardInfoAndLoadStatus(Node3bb8cE *self, s32 *p1, s32 *p2, s32 p3)`. Calls
`TaskObjF__CardInfoStatus(self, p1, p2)`; if the result is non-zero, also calls
`TaskObjF__CardLoadStatus(self, p1, p3)`.

## Result

Matched on the first attempt.

```c
void TaskObjF__CardInfoAndLoadStatus(Node3bb8cE *self, s32 *p1, s32 *p2, s32 *p3)
{
    if (TaskObjF__CardInfoStatus(self, p1, p2) != 0) {
        TaskObjF__CardLoadStatus(self, p1, p3);
    }
}
```

Note `p3` is typed `s32 *`, not `s32` — `TaskObjF__CardLoadStatus` (defined later in
this unit) dereferences its own matching 3rd parameter, so the type comes
from the callee's usage, not from this function's own body (which only
forwards it opaquely). Retyped once `TaskObjF__CardLoadStatus` was written; no
behavioural difference for this function's own bytes.

### Extern added for a function outside this unit

None new here (forward declarations of `TaskObjF__CardInfoStatus`/`TaskObjF__CardLoadStatus`
are for functions defined LATER in this SAME unit, not another unit).

### Return type revisited while deriving TaskObjF__CheckCardStatus (still `void` in `src/`)

`TaskObjF__CheckCardStatus` (this round's stall, see its report) needs to READ this
function's return value, so it briefly needed `TaskObjF__CardInfoAndLoadStatus` retyped to
`s32`. **Adding explicit `return TaskObjF__CardLoadStatus(self, p1, p3); return 0;`
statements (semantically identical to the `void` body below) grew the
compiled function from 21 to 23 words** and broke the whole unit's build
— confirmed via `objdump`, not just funcdiff. The fix was to retype the
DECLARATION to `s32` while leaving the BODY exactly as originally
written (no explicit `return`, relying on `$v0` already holding the
right value when control falls off the end — triggers a harmless
"control reaches end of non-void function" warning). Left as `void` here
since no function in `src/ui/title_menu.c` currently reads this
function's return value; `TaskObjF__CheckCardStatus`'s own stalled body (preserved
in its report) shows the `s32`-declared form for when/if it's needed.

### Proposed learning

None new.

## Naming (round 78, track 3)

`func_8004E77C` -> `TaskObjF__CardInfoAndLoadStatus`. **Tier B.** Private helper (not a `gTaskObjFMethods` entry) called only by `TaskObjF__CheckCardStatus`. Calls `TaskObjF__CardInfoStatus`, and if it reports OK, `TaskObjF__CardLoadStatus`. Mechanics only; the composed purpose (why info-then-load) is inferred, not proven.

## Round 98 (track 7)

The source declares it `s32` (as task_objf.h does) with the body still
falling off the end, and a MATCHING line says why: it returns whatever the
last call left in `$v0`, and an explicit return adds two words.
