# GetCharSpriteMethods -- MATCHED (4/4 words), round 82

> Renamed from `Get_vtable_D8006EC74` on 2026-09-26 (tools/rename.py). Address 0x80041c3c.

> Renamed from `func_80041C3C` on 2026-09-25 (tools/rename.py). Address 0x80041c3c.

Round 82, runner alpha (second re-staffed slot of the round). Unit `src/graphics/Sprite.c`. Fresh ground (carved in FINISHING-PLAN revision 18), no prior attempt, no report before this one.

- **Where:** not in any method table (called directly) (`tools/classtable.py`).
- **What:** Returns the gCharSpriteMethods method table: `lui/addiu $v0; jr; nop`. Callers in `ScreenWidgets.c` (via `include/Task.h`) declare it as `Obj6EAC0Methods *`; this unit declares `void *`, which is legal because neither TU sees the other.
- **Result:** byte-exact; 4/4 words, 0 insertions / 0 deletions, whole-image SHA1 green (`./build-and-verify.sh` OK). First build.
- **Types:** local views and `extern s32 D_XXXXXXXX[];` table declarations live in the unit; no shared header was touched.

## Source

```c
/* Returns the CharSprite method table. */
CharSpriteMethods *GetCharSpriteMethods(void) {
    return &gCharSpriteMethods;
}
```

## Naming

- `Get_vtable_D8006EC74` -- tier A. Table getter ("return D_8006EC74;"), the Get_vtable_<Class> convention already used for anonymous classes (GetBasicClassMethods, Get_vtable_Pad, Get_vtable_Entity).

## Track 4

2026-09-26, round 86 (bravo): class 0x1144 unified as CharSprite in `include/CharSprite.h`. Renamed from `Get_vtable_D8006EC74`, tier A: it returns the class's table, and the name is the class's getter convention (GetScreenSpriteMethods, GetSpriteMethods). Declared once, in the header, returning `CharSpriteMethods *` and `&gCharSpriteMethods` (formerly `D_8006EC74`): the same lui/addiu as the array-decay spelling. The two other declarations are gone: this unit's `void *` and include/Task.h's `Obj6EAC0Methods *`. The Source block above is the unified spelling. Image byte-identical.
