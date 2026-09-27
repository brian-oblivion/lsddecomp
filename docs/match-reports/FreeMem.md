# FreeMem

> Renamed from `func_80017AA8` on 2026-09-24 (tools/rename.py). Address 0x80017aa8.

**Unit:** BMemPMgr · **Size:** 8 instructions · **Status:** MATCHED (8/8 words)

## Verdict correction (round 74)

This report used to describe a call to `func_80011F68`, a still-uncarved
Psy-Q block. That address has since been identified as Sony's own `free`
(the object is now linked from `lib/` under its real name), so the body
below is what actually ships: a plain tail-call to `free`, `void` this
time (matching `free`'s own signature), not the `s32`-returning
`func_80011F68` wrapper the old text described. Byte-exact status is
unaffected; only the prose and the callee's identity were stale.

## What it does

A one-line wrapper: tail-calls libc/Psy-Q `free(ptr)` with its own
argument forwarded unchanged. No relationship to `BMemPMgr`/`BMemBlockHdr`
is visible in the body itself — it is a plain `free` wrapper, not a
pool-specific destructor, and no caller has been carved yet to confirm
what it is used for.

## The C

```c
void FreeMem(void *ptr)
{
    free(ptr);
}
```

## Naming

`FreeMem`, **tier A**: the body is exactly `free(ptr);`, a pure
mechanical wrapper whose mechanics ARE its purpose (per the naming rule
for a pure leaf). Deliberately NOT named `BMemPMgrFree` or similar —
nothing in the body touches a `BMemPMgr`/`BMemBlockHdr`, and no caller is
carved yet to establish a narrower purpose; naming it after the pool
allocator would be a guess from proximity, not evidence.

## Provenance

round 11 (2026-09-03), runner delta, unit BMemPMgr (fresh carve).
round 74 (2026-09-24), runner alpha: renamed `func_80017AA8` ->
`FreeMem`, corrected the callee identity (`func_80011F68` -> `free`) and
return type in this report to match the current source.
