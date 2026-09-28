# TaskObjF__CardLoadStatus — MATCH (44/44 words)

> Renamed from `func_8004E890` on 2026-09-24 (tools/rename.py). Address 0x8004e890.

**Unit:** TitleMenuTaskObjF (round 14, `Node3bb8cE` class).

## What it does

`s32 TaskObjF__CardLoadStatus(Node3bb8cE *self, s32 *p1, s32 *p2)`. Structural sibling
of `TaskObjF__CardInfoStatus`: zeroes `*p1`, sets `*p2` to the default status (1),
calls `TaskObjF__TestEvents(self)`, busy-waits on `func_80050B08(self->unk10)`
until nonzero, reads a status code from `TaskObjF__WaitForReadyEvent(self)` and maps it
identically to `TaskObjF__CardInfoStatus`'s first two cases (`0x100` -> status 0 with
no out-writes; `0x8000` -> status 0, `*p1 = 1`), but its `0x2000` arm
differs: it just sets `*p2 = 0` (no follow-up library call, unlike
`TaskObjF__CardInfoStatus`'s `_card_clear`).

## Result

Applied `TaskObjF__CardInfoStatus`'s already-derived fix up front and matched
immediately. Since this function's two initial stores write DIFFERENT
values (`*p1 = 0`, `*p2 = 1`, not the same value), the chained-assignment
form from `TaskObjF__CardInfoStatus` (`*p2 = *p1 = 0;`) doesn't apply verbatim; the
comma-operator equivalent does:

```c
s32 TaskObjF__CardLoadStatus(Node3bb8cE *self, s32 *p1, s32 *p2)
{
    s32 status;
    s32 code;

    status = 1;
    *p2 = (*p1 = 0, status);
    TaskObjF__TestEvents(self);
    while (func_80050B08(self->unk10) == 0)
        ;
    code = TaskObjF__WaitForReadyEvent(self);
    if (code == 0x100) {
        status = 0;
    } else if (code == 0x8000) {
        status = 0;
        *p1 = 1;
    } else if (code == 0x2000) {
        *p2 = 0;
    }
    return status;
}
```

### Proposed learning

Generalizes `TaskObjF__CardInfoStatus`'s finding: when the two stores that need to be
forced adjacent (ahead of an intervening void call) write DIFFERENT
values, the comma operator (`*a = (*b = v1, v2);`) is the equivalent of
the same-value chained assignment (`*a = *b = v;`) — both remove GCC
2.6.3's freedom to hoist the call across just one of the two stores.

## Naming (round 78, track 3)

`func_8004E890` -> `TaskObjF__CardLoadStatus`. **Tier B.** Private helper called only by `TaskObjF__CardInfoAndLoadStatus`. Same shape as `TaskObjF__CardInfoStatus` but polls `_card_load` instead of `_card_info`, and does not clear the card on code 0x2000. Named for the BIOS call it wraps.

## Constants (round 98, track 7)

The answers `TaskObjF__WaitForReadyEvent` returns are `gCardEventSpecs`
entries, i.e. `<kernel.h>` event specs: 0x100 `EvSpTIMOUT` (no card
answered), 0x8000 `EvSpERROR`, 0x2000 `EvSpNEW` (for `_card_info` a newly
inserted card, which is then `_card_clear`ed; for `_card_load` an
unformatted one). The BIOS calls come from `<kernel.h>` instead of local
externs. Zero bytes changed.
