# SsUtAutoPan -- MATCH (21/21 words, 2 rebuild attempts)

> Renamed from `func_80031EE8` on 2026-09-23 (tools/rename.py). Address 0x80031ee8.

Unit `code_179d8_j`, round 21 (2026-09-06). Sibling of `SsUtAutoVol`
(see that report) -- identical shape, calls `BeginVoiceFade` instead of
`SeAutoVol`.

```c
extern void BeginVoiceFade(s16 a0, s16 a1, s16 a2, s16 a3);

s32 SsUtAutoPan(s16 p0, s16 p1, s16 p2, s16 p3)
{
    if ((u16) p0 < 0x18) {
        BeginVoiceFade(p0, p1, p2, p3);
        return 0;
    }
    return -1;
}
```

Same call-site-typed extern and same discarded-return-value shape as
`SsUtAutoVol`; same branch-polarity fix needed once, matched on the
second attempt.

### Proposed learning

None beyond `SsUtGetDetVVol.md`'s branch-polarity note.
