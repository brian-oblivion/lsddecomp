# GetClass86B60Methods -- MATCH

> Renamed from `func_8004E2D0` on 2026-09-24 (tools/rename.py). Address 0x8004e2d0.

Unit `class_3bb8c_d`, round 14. `./build-and-verify.sh` exit 0; whole-image
SHA1 matches retail. `funcdiff.py GetClass86B60Methods`: 4/4 words match.

## Source

```c
Class86B60Methods *GetClass86B60Methods(void)
{
    return &gClass86B60Methods;
}
```

## Notes

Already fully declared in `include/class_3bb8c.h` from prior work on
`class_3bb8c_c` (the comment at `Class86B60Methods *GetClass86B60Methods(void)`'s
declaration already named this exact function as the getter, and `gClass86B60Methods`
was already `extern`-declared as `Class86B60Methods`). No header changes
needed -- this function only had to be typed in and moved out of
`INCLUDE_ASM`.

First attempt, byte-exact.

## Naming (round 77, naming runner delta)

Renamed `func_8004E2D0` -> `GetClass86B60Methods`. **Tier A**: Pure getter, returns `&gClass86B60Methods` only. Matches the established `Get<Class>Methods` convention already used in this same unit (`GetTaskObjFMethods`) and elsewhere (`GetClass6B5CCMethods`) for the identical no-argument vtable-getter shape.

## Track 4 (2026-09-26, round 88, bravo)

Class86B60 is unified in include/Class86B60.h (TASKCORE_SLOTS/TASKCORE_FIELDS plus its own). Its extern is in include/Class86B60.h; the class_3bb8c.h one is gone. Byte-identical (whole image green, 0 new warnings, nonmatching green).
