# SpuVmSeKeyOff -- MATCH (13/13 words, 1 rebuild attempt)

> Renamed from `func_800303C8` on 2026-09-23 (tools/rename.py). Address 0x800303c8.

Unit `code_179d8_j`, round 21 (2026-09-06). One-line tail-call wrapper.

```c
extern s32 SpuVmKeyOff(s32 a0, s16 a1, s16 a2, u16 a3);

s32 SpuVmSeKeyOff(s16 p0, s16 p1, u16 p2)
{
    return SpuVmKeyOff(0x21, p0, p1, p2);
}
```

Matched first try (m2c's seed was already this exact shape). Per
CLAUDE.md's note on one-line wrapper tail calls, wrote `return
callee(...)` rather than a bare statement + implicit return -- the bytes
would be identical either way, but the return-type guess (`s32`, not
`void`) is the one that generalises if another unit ever calls this
function and inspects `$v0`.

### Proposed learning

None new.

## Round 97 types pass (echo)

Parameters renamed `p0..p2` -> `vabId, prog, note` (forwarded to
`SpuVmKeyOff`'s matching slots); `0x21` -> `SPUVM_SE_SEQ` (see
SpuVmSeKeyOn.md). Byte-exact unchanged.
