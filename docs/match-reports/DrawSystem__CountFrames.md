# DrawSystem__CountFrames -- MATCHED (22/22 words), round 81

> Renamed from `func_80020AF4` on 2026-09-25 (tools/rename.py). Address 0x80020af4.

Round 81, runner alpha. Unit `src/code_10ee0.c`. Fresh ground, no prior attempt.

- **Where:** D_8006C070 slot +0x06C (slots resolved with `tools/classtable.py D_8006C070`).
- **What:** ignores self; on the singleton GetDrawSystem() increments +0x24, and once it reaches +0x20 and +0xC is clear, sets +0xC = 1 and resets +0x24.
- **Result:** byte-exact on the FIRST build, no levers; `funcdiff.py` reports
  22/22 words, 0 insertions / 0 deletions, and the whole-image SHA1 is
  green (`OK: build matches retail`).

## Source

```c
void DrawSystem__CountFrames(Class6C070 *self) {
    Class6C070 *obj = GetDrawSystem();

    obj->unk24++;
    if (obj->unk24 >= obj->unk20 && obj->unkC == 0) {
        obj->unkC = 1;
        obj->unk24 = 0;
    }
}
```

The declarations it needs (unit-local view in `src/code_10ee0.c`; the class
structs start with `BASICCLASS_SLOTS`/`BASICCLASS_FIELDS` from
`include/BasicClass.h`, and the SDK externs are local copies of the
LIBGPU.H/LIBGS.H prototypes):

```c
typedef struct Class6C070 Class6C070;
typedef struct Class6C070Methods Class6C070Methods;
typedef struct {
    /* +0x0 */ s32 w;
    /* +0x4 */ s32 h;
} Class6C070Size;

struct Class6C070 {
    BASICCLASS_FIELDS(Class6C070Methods);
    /* +0x00C */ s32 unkC;            /* 80020AF4 sets to 1 once unk24 reaches unk20 */
    /* +0x010 */ s32 unk10;           /* cleared by 8002089C; 80020B4C stores unk20 only while 0 */
    /* +0x014 */ Class6C070Size size; /* 80020C08 returns its address */
    /* +0x01C */ u8 pad1C[0x20 - 0x1C];
    /* +0x020 */ s32 unk20;           /* 80020B4C sets, 80020B68 gets */
    /* +0x024 */ s32 unk24;           /* 80020AF4 counts up to unk20 */
    /* +0x028 */ u8 pad28[0x2C - 0x28];
    /* +0x02C */ s32 unk2C;           /* 80020C3C sets */
    /* +0x030 */ s32 unk30;           /* 80020C44 sets */
};
```

## Naming

`DrawSystem__CountFrames`, tier B. Ignores its own `self` and instead
increments the SINGLETON's frame counter (+0x24) toward a threshold (+0x20,
shared with `DrawSystem__SetVSyncCount`'s field), setting a "done" flag
(+0xC) once. It occupies a real vtable slot (+0x06C) but is never dispatched
through `->methods->` from within this unit; what calls it, and what the
+0xC flag then gates, isn't established here.
