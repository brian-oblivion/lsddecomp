# InitDivPolygonPtrs — MATCHED (17/17 words)

> Renamed from `InitVtxRecordPtrs` on 2026-09-26 (tools/rename.py). Address 0x8001a224.

> Renamed from `func_8001A224` on 2026-09-24 (tools/rename.py). Address 0x8001a224.

Unit: `src/code_8220_c.c`. Immediately precedes `UpdatePolyBBoxAndCull` in ROM
order — likely a helper it calls, though `UpdatePolyBBoxAndCull` itself is not yet
matched (queued next). `kind` is the same "primitive kind" code seen
already in `code_8220_b` (`ProjectTriFace`/`ProjectQuadFace` pass `3`
triangle / `4` quad to `UpdatePolyBBoxAndCull`): here it doubles as the LOOP COUNT
too, since a triangle needs 3 slots written and a quad 4 — one register,
two jobs, matching how compactly retail keeps live values.

Writes the same running pointer (`src`, starting at `arg1+0x18` and
advancing by `0x18` — one "vertex slot" stride) into two parallel output
arrays: `arg0[i]` and `arg1[0xF0 or 0xA8][i]` (the destination array itself
selected by `kind == 4`), `kind` times.

## Final source

```c
void InitDivPolygonPtrs(void *arg0, void *arg1, s32 kind)
{
    u8 *src = (u8 *)arg1 + 0x18;
    u8 *dst0 = (u8 *)arg0;
    u8 *dst1 = (kind == 4) ? (u8 *)arg1 + 0xF0 : (u8 *)arg1 + 0xA8;

    while (kind-- > 0) {
        *(void **)dst1 = src;
        *(void **)dst0 = src;
        src += 0x18;
        dst1 += 4;
        dst0 += 4;
    }
}
```

## Notes

Byte-exact on the first attempt. The loop is `while (kind-- > 0)` reached
via a jump straight to the condition (the standard `for`/`while` lowering,
same family as `ReleaseBasicClassArray`'s `if(count--)...while(count--)` in
`code_8220_b`, but here with no outer guard since the test is at the loop's
natural entry point already). `arg1`'s value used for `src` is the
UNMODIFIED parameter, computed before the `dst1` selection — the two never
alias the same C expression, matching retail's use of the original `$a1`
for one purpose and a reassigned local pointer for the other.

## Naming (round 77, alpha)

`func_8001A224` -> `InitDivPolygonPtrs`, parameters (`arg0`, `arg1`) ->
(`dst`, `table`); `kind` kept (dual-purpose primitive-kind/loop-count
code, already documented in this report). **Tier B.** Mechanics are
established (writes a running pointer through `table`'s own per-vertex
records into two parallel arrays), matched by call site in code_8220_b:
`InitDivPolygonPtrs(ctx + 0x88, gDivPolygon3, 3)` and
`InitDivPolygonPtrs(ctx + 0x94, gDivPolygon4, 4)`, confirming
`dst`/`table` and that this is a one-time setup of the render context's
vertex-record pointer slots (the same `ctx+0x88`/`ctx+0x94` the Submit*
wrappers later read). WHY `table` also keeps its own mirror copy of the
same pointers (at `+0xA8`/`+0xF0`) is not established, so the function
name states only the mechanics, not that purpose.
