# TmdModel__GetHull -- MATCHED (84/84 words), round 82

> Renamed from `func_8001F51C` on 2026-09-25 (tools/rename.py). Address 0x8001f51c.

Round 82, runner charlie (matching slot, second pass on the unit). Unit `src/code_fa50.c`. Fresh ground, no prior attempt.

- **What:** computes the model's bounding box into a stack local (`TmdModel__ComputeBounds`) and writes its eight corners into `out` (bottom face min-z in order (minx,miny) (minx,maxy) (maxx,maxy) (maxx,miny), then the same at max-z), with `out->type = 1`. The local is `{ s32 type; Box box; }` at sp+0x10 and its `type` is set to 1 too (retail stores it, nothing reads it).
- **Result:** byte-exact; 84/84 words, whole-image SHA1 green. Second build.
- **Build 1:** `out->type = b.type;` -- 74/84, 3 words long: GCC reloaded the stack word after the call region. Retail keeps the constant 1 live in `$v1` from the `sw` to the local through to the final `sw 0(s0)`: **`out->type = 1;`** (a literal, CSE'd with the local's store) closes it.
- **Callers:** `SceneNode__ReadUnk20Data` (code_d294_b) via `include/code_d294.h`'s `void TmdModel__GetHull(void *, void *)` -- consistent with this definition (void return); that header was not touched.

## Source

```c
typedef struct Vec3_fa50 { s16 x, y, z; } Vec3_fa50;
typedef struct Box_fa50 { Vec3_fa50 min; Vec3_fa50 max; } Box_fa50;
typedef struct TypedBox_fa50 { s32 type; Box_fa50 box; } TypedBox_fa50;
typedef struct Hull_fa50 { s32 type; Vec3_fa50 v[8]; } Hull_fa50;
/* TmdModel and TmdModel__ComputeBounds: see TmdModel__ComputeBounds.md */

void TmdModel__GetHull(TmdModel *self, Hull_fa50 *out) {
    TypedBox_fa50 b;

    TmdModel__ComputeBounds(self, &b.box);
    b.type = 1;
    out->v[0].x = b.box.min.x;
    out->v[0].y = b.box.min.y;
    out->v[0].z = b.box.min.z;
    out->v[1].x = b.box.min.x;
    out->v[1].y = b.box.max.y;
    out->v[1].z = b.box.min.z;
    out->v[2].x = b.box.max.x;
    out->v[2].y = b.box.max.y;
    out->v[2].z = b.box.min.z;
    out->v[3].x = b.box.max.x;
    out->v[3].y = b.box.min.y;
    out->v[3].z = b.box.min.z;
    out->v[4].x = b.box.min.x;
    out->v[4].y = b.box.min.y;
    out->v[4].z = b.box.max.z;
    out->v[5].x = b.box.min.x;
    out->v[5].y = b.box.max.y;
    out->v[5].z = b.box.max.z;
    out->v[6].x = b.box.max.x;
    out->v[6].y = b.box.max.y;
    out->v[6].z = b.box.max.z;
    out->v[7].x = b.box.max.x;
    out->v[7].y = b.box.min.y;
    out->v[7].z = b.box.max.z;
    out->type = 1;
}
```

## Naming

`TmdModel__GetHull` -- tier A. Computes the model's bounding box
(`TmdModel__ComputeBounds`) and writes its eight corners into a
caller-supplied `Hull_fa50`, setting `out->type = 1` (the count). "Hull"
names the mechanics (an 8-corner box for a hit test) without claiming a
specific in-game role.
