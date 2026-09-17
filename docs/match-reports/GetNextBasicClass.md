> Renamed from `func_800183A0` on 2026-09-17 (tools/rename.py). Address 0x800183a0.

# GetNextBasicClass — MATCHED (15/15 words)

Unit: `src/code_8220_b.c`. Signature (already in `include/code_8220.h`):
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
