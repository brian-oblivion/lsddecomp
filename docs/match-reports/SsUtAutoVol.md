# SsUtAutoVol -- MATCH (21/21 words, 2 rebuild attempts)

> Renamed from `func_80031E94` on 2026-09-23 (tools/rename.py). Address 0x80031e94.

Unit `code_179d8_l`, round 21 (2026-09-06). Not a class method (calls a
plain function, no vtable dereference). Sibling of `SsUtAutoPan`
(identical shape, different callee).

```c
extern void SeAutoVol(s16 a0, s16 a1, s16 a2, s16 a3);

s32 SsUtAutoVol(s16 p0, s16 p1, s16 p2, s16 p3)
{
    if ((u16) p0 < 0x18) {
        SeAutoVol(p0, p1, p2, p3);
        return 0;
    }
    return -1;
}
```

`SeAutoVol` is still `INCLUDE_ASM` in whatever unit owns it (not
found in any `src/*.c` at time of writing), so this is a per-call-site
guess: all four arguments are `s16`-sign-extended before the `jal`, and
its return value is explicitly discarded (an unconditional `addu
$v0,$zero,$zero` executes right after the call, overwriting whatever
`$v0` held) -- the C reflects that by not using the call's result at
all; this function always returns 0 on the valid path regardless of
what the callee produced.

Same branch-polarity fix as `SsUtGetDetVVol` was needed on the first
attempt: `if (valid) { call; return 0; } return -1;`, not the inverted
guard-clause form. See that report for the general note.

### Proposed learning

None beyond `SsUtGetDetVVol.md`'s branch-polarity note, which this
confirms a second time in the same unit.

## Round 97 (bravo, track 6)

The definition now takes <libsnd.h>'s prototype: return type `s32` became `s16` (Sony's `short`). Zero bytes.
