# TaskObjF__ProbeCardFreeSpace — MATCH (29/29 words)

> Renamed from `func_8004ECCC` on 2026-09-24 (tools/rename.py). Address 0x8004eccc.

**Unit:** class_3bb8c_e (round 14, `Node3bb8cE` class).

## What it does

`s32 TaskObjF__ProbeCardFreeSpace(Node3bb8cE *self, u8 id, s32 sizeArg)`. Computes a
sector count from `sizeArg` (`(sizeArg + 0x21FF) >> 13`, i.e. round up to
an 8KB/0x2000-byte boundary and divide by it), formats a path via
`BuildMemcardPath(pathBuf, self->unkC, &gMcTempFileSuffix)` (a 3-argument call, one
more argument than `TaskObjF__OpenAndReadMemcardFile`'s 2-argument call to the same
function — `id` itself is unused in this function's own body, same
"unused-but-forwarded parameter" shape already established elsewhere),
opens it with `func_80050938(path, (sectors << 16) | 0x200)`, and on
success closes the handle and calls `func_80050908(pathBuf)` before
returning 1 (0 on open failure).

## Where it stood, and the fix

Three separate residues stacked, closed incrementally:

1. **The `(sizeArg + 0x21FF) >> 13 << 16` computation split across the
   call to `BuildMemcardPath`.** Writing it as one combined expression before
   the call put the `>>13` in the wrong position — retail computes
   `sizeArg + 0x21FF` early (right after the prologue), defers the `>>13`
   to the call's own delay slot, and folds the final `<<16 | 0x200` into
   the SECOND call's own argument expression. Splitting into
   `sectors = sizeArg + 0x21FF; sectors = sectors >> 13; ...;
   handle = func_80050938(path, (sectors << 16) | 0x200);` (three
   separate mentions across the right statement boundaries, not one
   expression) moved this from garbled ordering to 28/29.
2. **A missing redundant `move v0, a0` for `self`**, the same class as
   `TaskObjF__OpenAndReadMemcardFile`'s open stall (see that report) — but here it resolved
   itself once the `sectors` computation was split as above, without any
   extra source-level lever. The extra arithmetic apparently gave GCC a
   reason to evacuate `self` from `a0` into `v0` on its own.
3. **`sra` where retail has `srl`** — a SIGNED right shift where retail's
   is UNSIGNED (logical). `sizeArg` is `s32` and `>>` on a signed type is
   arithmetic in C; casting the shifted value's operand to `u32` forced
   the logical form:

```c
sectors = (u32)(sizeArg + 0x21FF) >> 13;
path = BuildMemcardPath(pathBuf, self->unkC, &gMcTempFileSuffix);
handle = func_80050938(path, (sectors << 16) | 0x200);
```

## Full body

```c
extern void *BuildMemcardPath();
extern s32 func_80050938(void *arg0, s32 arg1);
extern s32 func_800508F8(s32 arg0);
extern s32 func_80050908(void *arg0);

s32 TaskObjF__ProbeCardFreeSpace(Node3bb8cE *self, u8 id, s32 sizeArg)
{
    s32 pathBuf[8];
    void *path;
    s32 handle;
    s32 sectors;

    sectors = (u32)(sizeArg + 0x21FF) >> 13;
    path = BuildMemcardPath(pathBuf, self->unkC, &gMcTempFileSuffix);
    handle = func_80050938(path, (sectors << 16) | 0x200);
    if (handle == -1) {
        return 0;
    }
    func_800508F8(handle);
    func_80050908(pathBuf);
    return 1;
}
```

### Proposed learning

**A right-shift residue that looks like a register/order problem may
actually be `sra` vs `srl` (signed vs unsigned shift).** Before chasing
scheduling, diff the single differing word's mnemonic — `sra`/`srl` are
easy to miss as "the same shift, just off by one word" when skimming a
funcdiff, since both are exactly one word and both come from a `>> N`
source expression. Cast the shifted quantity to an unsigned type
(`(u32)expr >> N`) to force `srl`.

**This is also a second confirmed instance of "one instruction short
describes the score, not the defect count"** (CLAUDE.md /
DECOMPILATION_LEARNINGS) — the fully garbled first attempt (18/29,
several words scattered) was TWO independent residues (the expression
split, and the signed/unsigned shift), not one; fixing the split first
revealed the shift issue as a clean single-word residue underneath.

## Head correction, round 75

The two-argument / `filterName` reading of `BuildMemcardPath` above is
superseded. Round 75 matched `TaskObjF__OpenAndReadMemcardFile` by calling it with THREE
arguments `(pathBuf, self->unkC, suffix)`, the third forwarded from the
caller's own third parameter already in `$a2` (so no `$a2` set-up is
emitted, which is why it read as two). `src/class_3bb8c_e.c` now declares
one real prototype, `extern void *BuildMemcardPath(void *dest, s32 selector,
void *suffix);`, replacing the unprototyped `arity-ok` declarations; this
function's bytes are unchanged (see `TaskObjF__OpenAndReadMemcardFile.md`).
