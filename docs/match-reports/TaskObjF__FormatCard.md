# TaskObjF__FormatCard — MATCH (27/27 words)

> Renamed from `func_8004E940` on 2026-09-24 (tools/rename.py). Address 0x8004e940.

**Unit:** title_menu (round 14, `Node3bb8cE` class).

## What it does

`s32 TaskObjF__FormatCard(Node3bb8cE *self)`. Retries up to 10 times: pick one of
two candidate path/name constants (`sMcDevicePath1` or `sMcDevicePath0`) based on
`self->unkC`'s truth value, call `func_80050918` on it, and stop as soon
as it returns nonzero or the retry budget is exhausted.

## Result

Reached 25/27 first try with the tag values swapped (my ternary picked
`sMcDevicePath0` when `unkC != 0`; retail picks `sMcDevicePath1`). Swapping the
ternary arms matched immediately:

```c
s32 TaskObjF__FormatCard(Node3bb8cE *self)
{
    s32 retries;
    s32 result;
    s32 *path;

    retries = 10;
    do {
        path = self->unkC != 0 ? &sMcDevicePath1 : &sMcDevicePath0;
        result = func_80050918(path);
    } while (result == 0 && retries-- != 0);
    return result;
}
```

Note the loop test uses `retries-- != 0` (matching retail's `bnez v0`,
i.e. branch-if-not-equal-zero), not `retries-- > 0` (which would compile
to a different comparison instruction, `bgtz`/`slt`) — written to match
the actual instruction retail emits rather than the more idiomatic-looking
`> 0`.

### Proposed learning

None new — a straightforward "10 retries, then give up" idiom already
established elsewhere in this project (`TaskObjF__ProbeMemcardFile`/`TaskObjF__CheckCardSpace` in
this same unit share the shape).

## Naming (round 78, track 3)

`func_8004E940` -> `TaskObjF__FormatCard`. **Tier A.** Sits at `gTaskObjFMethods` +0x050. Retries up to 10 times: pick `sMcDevicePath1`/`sMcDevicePath0` (BIOS device names "bu10:"/"bu00:", asm/data/7B12C.sdata.s) by `self->cardSlot`, and call the linked BIOS `format()` on it. Direct call to a BIOS function named `format` -- tier A.

## Track 4b (2026-09-25, round 85)

`sMcDevicePath0`/`sMcDevicePath1` were declared `s32` here and
`McDevicePath` in `include/class_3bb8c.h`. This unit now includes that
header, `path` is a `McDevicePath *`, and `format()` is declared with the
BIOS's device-name parameter (`char *`). Byte-identical; no new `-Wall`
warning.

## Source comment moved here (round 98, track 7)

The unit declared `format` itself, under: "The PS-X BIOS format(), which
takes a device name. sMcDevicePath0/1 are the "bu00:"/"bu10:" templates
include/class_3bb8c.h declares (track 4b, round 85: this unit had its own
`s32` view for this address-only use)." `format` now comes from
`<kernel.h>`, and `MEMCARD_RETRIES` (include/task_objf.h) replaces the 10.
The `(char *)path` cast stays: `McDevicePath` is the device name typed as a
struct so BuildMemcardPath's copy matches, and `format` takes the string.
