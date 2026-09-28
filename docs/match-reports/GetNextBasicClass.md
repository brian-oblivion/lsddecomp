# GetNextBasicClass — MATCHED (15/15 words)

> Renamed from `func_800183A0` on 2026-09-17 (tools/rename.py). Address 0x800183a0.

Unit: `src/graphics/tmd_renderer.c`. Signature (already in `include/code_8220.h`):
`void GetNextBasicClass(BasicClass **outValue, BasicClassListNode **cursor);` —
pop `*cursor` into `*outValue` (or `NULL` if the cursor is exhausted), then
advance `*cursor` to the popped node's `next`.

## Final source

```c
void GetNextBasicClass(BasicClass **outValue, BasicClassListNode **cursor)
{
    if (*cursor != NULL) {
        *outValue = (*cursor)->value;
        *cursor = (*cursor)->next;
    } else {
        *outValue = NULL;
    }
}
```

## Notes

Byte-exact on the first attempt. Retail reloads `*cursor` from memory twice
(once for `->value`, once for `->next`) rather than caching it in a register
across the two field reads — writing the source the same way, dereferencing
`*cursor` at each use site instead of caching it in a local `node` variable,
reproduced that directly. This is the callee `BasicClass__NotifyParents` uses to
walk `parentRefs`/`children` lists (see `docs/match-reports/BasicClass__NotifyParents.md`
if present, and the class comment in `include/code_8220.h`).

## Naming (round 51, bravo)

`func_800183A0` -> `GetNextBasicClass`. **Tier A** -- pure leaf, mechanics
are its purpose.

Evidence: pops the `BasicClass *` out of the node `*cursor` points at,
advances `*cursor` to that node's `next`, and writes `NULL` when the cursor
is exhausted -- one iterator step, nothing else. It is what both of
BasicClass's public iteration slots are built from: `+0x01C` getNextChild
(`BasicClass__GetNextChild`) and `+0x02C` getNextParentRef
(`BasicClass__GetNextParentRef`), and it is called directly by
`BasicClass__NotifyParents` in this unit. The name is `BasicClass`, not
`BasicClassListNode`, because the value it yields is the `BasicClass *`;
the node is the cursor's business.
