# New_Sprite -- MATCHED (40/40 words), round 82

Round 82, runner alpha (fifth slot on code_322b4). Unit `src/code_322b4.c`. Fresh ground, no prior body attempt.

- **Where:** not in any method table (allocator) (`tools/classtable.py`).
- **What:** `BMemPMgrAlloc(0xA0)`; if non-NULL, Sprite's ctor through `GetSpriteMethods()` with all five arguments (two on the stack), returns the object, else NULL. The round's allocator shape, first build.
- **Result:** byte-exact; 40/40 words, 0 insertions / 0 deletions, whole-image SHA1 green (`./build-and-verify.sh` OK). First build.
- **Types:** matches the `include/Sprite.h` prototype unchanged.

## Source

```c
/* Allocate and construct a Sprite (0xA0 bytes). */
Sprite *New_Sprite(void *texture, s32 abr, SpriteRect *rect, void *arg3, s32 arg4) {
    Sprite *obj = BMemPMgrAlloc(0xA0);

    if (obj != NULL) {
        GetSpriteMethods()->ctor(obj, texture, abr, rect, arg3, arg4);
        return obj;
    }
    return NULL;
}
```
