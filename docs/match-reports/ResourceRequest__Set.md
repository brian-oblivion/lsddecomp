# ResourceRequest__Set

> Renamed from `SetVec3` on 2026-09-27 (tools/rename.py). Address 0x80026ce8.

> Renamed from `func_80026CE8` on 2026-09-18 (tools/rename.py). Address 0x80026ce8.

**Unit:** code_171e0 · **Size:** 5 instructions · **Status:** MATCHED (5/5 words)

## What it does

Writes three words (`x`, `y`, `z`) into the first 12 bytes of its first
argument and returns that pointer. A `Vec3`-style "set and return this"
setter.

## Derivation

```
addu  $v0, $a0, $zero
sw    $a1, 0x0($v0)
sw    $a2, 0x4($v0)
jr    $ra
 sw   $a3, 0x8($v0)
```

The leading `addu $v0, $a0, $zero` copies the incoming pointer into the return
register before the stores, which only makes sense if the function's C source
actually returns it — nothing else in the body needs a copy of `$a0` in
`$v0`. Declared accordingly, rather than as `void`, on that positive evidence
(the one caller found, in `asm/nonmatchings/code_4cd08/InitDreamAux.s`,
discards the return value, so a `void` guess would have looked equally
plausible from the call site alone — the `addu` in this function's own body is
what settles it):

```c
typedef struct ResourceRequest {
    s32 x;
    s32 y;
    s32 z;
} ResourceRequest;

ResourceRequest *ResourceRequest__Set(ResourceRequest *this, s32 x, s32 y, s32 z) {
    this->x = x;
    this->y = y;
    this->z = z;
    return this;
}
```

Only the first three words are known; there may be a fourth field the struct
doesn't yet claim (this function simply never touches it).

## Proposed learning

A leaf function that copies its first argument into `$v0` before doing
anything else, with no other use for that copy, is returning the pointer —
even when the one caller you can find ignores the result. Check the
function's OWN body for the `addu $v0, $a0, ...` idiom before trusting a
caller's ignored return value as evidence for `void`.

## Naming

Round 52 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_80026CE8` | `ResourceRequest__Set` | A |

**Evidence.** Writes `x`/`y`/`z` into a `ResourceRequest` and returns the pointer
-- a pure "set and return this" setter. Mechanics are its purpose.
