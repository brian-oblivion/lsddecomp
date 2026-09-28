# GetTitleMenuMethods -- MATCH

> Renamed from `GetClass86B60Methods` on 2026-09-26 (tools/rename.py). Address 0x8004e2d0.

> Renamed from `func_8004E2D0` on 2026-09-24 (tools/rename.py). Address 0x8004e2d0.

Unit `TitleMenuTaskObjF`, round 14. `./build-and-verify.sh` exit 0; whole-image
SHA1 matches retail. `funcdiff.py GetTitleMenuMethods`: 4/4 words match.

## Source

```c
TitleMenuMethods *GetTitleMenuMethods(void)
{
    return &gTitleMenuMethods;
}
```

## Notes

Already fully declared in `include/class_3bb8c.h` from prior work on
`TitleMenuTaskObjF` (the comment at `TitleMenuMethods *GetTitleMenuMethods(void)`'s
declaration already named this exact function as the getter, and `gTitleMenuMethods`
was already `extern`-declared as `TitleMenuMethods`). No header changes
needed -- this function only had to be typed in and moved out of
`INCLUDE_ASM`.

First attempt, byte-exact.

## Naming (round 77, naming runner delta)

Renamed `func_8004E2D0` -> `GetTitleMenuMethods`. **Tier A**: Pure getter, returns `&gTitleMenuMethods` only. Matches the established `Get<Class>Methods` convention already used in this same unit (`GetTaskObjFMethods`) and elsewhere (`GetSceneNodeMethods`) for the identical no-argument vtable-getter shape.

## Track 4 (2026-09-26, round 88, bravo)

TitleMenu is unified in include/TitleMenu.h (TASKCORE_SLOTS/TASKCORE_FIELDS plus its own). Its extern is in include/TitleMenu.h; the class_3bb8c.h one is gone. Byte-identical (whole image green, 0 new warnings, nonmatching green).
