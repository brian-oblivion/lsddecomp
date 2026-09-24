# func_8004E9AC — MATCH (35/35 words)

**Unit:** class_3bb8c_e (round 14, `Node3bb8cE` class).

## What it does

`s32 func_8004E9AC(Node3bb8cE *self, u8 *destBuf, u8 *filterName)`. Guards
`filterName`: if it's NULL or points at an empty string, returns 0
immediately without calling anything. Otherwise calls
`func_8004EA38(self, destBuf, filterName)` — with a retry-loop skeleton
identical in shape to `func_8004E940`/`func_8004EC5C` but initialized to
`retries = 0`, so it structurally never loops (a single attempt).

## Result

Matched on the first attempt.

```c
extern s32 func_8004EA38(Node3bb8cE *self, u8 *destBuf, u8 *filterName);

s32 func_8004E9AC(Node3bb8cE *self, u8 *destBuf, u8 *filterName)
{
    s32 retries;
    s32 result;

    retries = 0;
    if (filterName == NULL || *filterName == 0) {
        return 0;
    }
    do {
        result = func_8004EA38(self, destBuf, filterName);
    } while (result == 0 && retries-- != 0);
    return result;
}
```

## Parameter naming, corrected mid-derivation

Reading the disassembly forward, it initially looked like this function's
2nd parameter (forwarded as `func_8004EA38`'s 2nd argument) might be an
opaque "arg1" distinct from the "name" string tested at the top — but
`func_8004EA38`'s own body (see its report) reveals its 2nd parameter is
the `strcpy` DESTINATION buffer, and its 3rd parameter (this function's
`filterName`, the one actually tested here) is never read by
`func_8004EA38` at all. Named accordingly once `func_8004EA38` was read.

### Proposed learning

The `do { } while (result == 0 && retries-- != 0)` skeleton, seen three
times in this unit (`func_8004E940` with `retries = 10`,
`func_8004EC5C` with `retries = 10`, this function with `retries = 0`) is
a single retry-loop idiom parameterized purely by the initial constant —
including the degenerate "run once" case. Reproduce the loop skeleton
literally rather than special-casing `retries == 0` into a plain `if`; the
skeleton's bytes are present in the disassembly even when the loop body
only ever executes once.

## Head correction, round 75

The two-argument / `filterName` reading of `BuildMemcardPath` above is
superseded. Round 75 matched `func_8004EA38` by calling it with THREE
arguments `(pathBuf, self->unkC, suffix)`, the third forwarded from the
caller's own third parameter already in `$a2` (so no `$a2` set-up is
emitted, which is why it read as two). `src/class_3bb8c_e.c` now declares
one real prototype, `extern void *BuildMemcardPath(void *dest, s32 selector,
void *suffix);`, replacing the unprototyped `arity-ok` declarations; this
function's bytes are unchanged (see `func_8004EA38.md`).
