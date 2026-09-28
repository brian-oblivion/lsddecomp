# ReleaseBasicClassArray — MATCHED (28/28 words)

> Renamed from `func_800183DC` on 2026-09-17 (tools/rename.py). Address 0x800183dc.

Unit: `src/TmdRenderer.c`. `void ReleaseBasicClassArray(BasicClass **array, s32 count)`
— releases every element of a `BasicClass*` array (calling each element's
vtable slot `+0x004`, `release`, and storing the result back into the slot)
and advances a pointer walk over `count` elements.

## Final source

```c
void ReleaseBasicClassArray(BasicClass **array, s32 count)
{
    if (count-- > 0) {
        do {
            *array = (BasicClass *)(*array)->methods->release(*array);
            array++;
        } while (count-- > 0);
    }
}
```

## Notes

Three earlier shapes all missed, each in a different way:

1. `for (i = 0; i < count; i++) { array[i] = ...; }` — 8/28. Introduced a
   third live register (a separate `i` alongside `array` and `count`),
   adding a spurious extra callee-save.
2. `while (count > 0) { ...; array++; count--; }` — 3/28, and address drift
   (build differs outside the function's own range: 311611 bytes). Same
   symptom in a different loop spelling.
3. `s32 remaining = count - 1; if (count > 0) { do {...} while (remaining-- >
   0); }` — 4/28, still drifted. Closer (two registers, right general
   shape) but wrong: retail's registers swap roles from this (retail keeps
   `array` in `$s1`/`count` in `$s0`; this variant put `array` in `$s0`)
   and, more importantly, retail computes the guard test from a **copy of
   the not-yet-decremented count**, then decrements the SAME register the
   copy came from — i.e. a genuine post-decrement side effect on the
   original `count`, not a separately-initialized `remaining`.

The fix: drop `remaining` entirely and reuse `count` itself with
POST-decrement in both the entry guard and the loop condition:
`if (count-- > 0) { do {...} while (count-- > 0); }`. Retail's redundant-
looking `addu $v0,$s0,0` / `addiu $s0,$s0,-1` pair (copy-then-decrement,
appearing twice — once before the loop, once at the loop bottom) is exactly
the codegen for `x-- > 0` used as a boolean test: the compiler must save the
pre-decrement value to compare against zero while the side effect lands in
the original register.

### Proposed learning

**A guard test and a loop test that "reuse" the same counter without an
extra `remaining` variable often means the source uses `count-- > 0`
(post-decrement-in-comparison) TWICE — once for the initial guard, once for
the loop's continuation test — not `while (count > 0) { ...; count--; }` and
not a separately-initialized `remaining = count - 1`.** The tell in the
disassembly: a `copy-then-decrement-the-original` pair (`addu $v0,$sN,0` /
`addiu $sN,$sN,-1`) appearing at BOTH the loop's entry and its bottom, using
the SAME saved register both times. `while(count-->0)` inside an
`if(count-->0)` do-while wrapper reproduces this exactly; a fresh
`remaining` local (even initialized identically) puts the decrement in a
different register than the one being tested and drifts the function's
length. (`ReleaseBasicClassArray`, 4/28 -> 28/28.)

## Naming (round 51, bravo)

`func_800183DC` -> `ReleaseBasicClassArray`. **Tier A** -- pure leaf,
mechanics are its purpose.

Evidence: walks `count` entries of a `BasicClass *` array, calls each
element's vtable slot `+0x004` (`release`) and stores the returned pointer
back into the slot. Every one of the six call sites outside this unit
(`ObjMStyleActor.c`, `class_3bb8c_o.c`, `class_3bb8c_s.c`, `Task.c`,
`ScreenWidgets.c`, plus `include/Task.h`'s declaration) passes a
contiguous array of object pointers and an element count. "Release" is the
slot's own established name in `BasicClassMethods`, not a new word.
