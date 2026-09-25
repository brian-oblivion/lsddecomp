# New_DrawSystem -- MATCHED (20/20 words), round 81

> Renamed from `new_class_6c078` on 2026-09-25 (tools/rename.py). Address 0x800206e0.

Round 81, runner alpha. Unit `src/code_10ee0.c`. Fresh ground, no prior attempt.

- **Where:** allocator (slots resolved with `tools/classtable.py D_8006C070`).
- **What:** allocates 0x34 bytes with BMemPMgrAlloc and runs the ctor through the class table (Get_vtable_DrawSystem()->ctor); the broadcast alloc-then-ctor shape `if (p != NULL) { ctor; return p; } return NULL;`.
- **Result:** byte-exact on the FIRST build, no levers; `funcdiff.py` reports
  20/20 words, 0 insertions / 0 deletions, and the whole-image SHA1 is
  green (`OK: build matches retail`).

## Source

```c
Class6C070 *New_DrawSystem(void) {
    Class6C070 *p = BMemPMgrAlloc(0x34);

    if (p != NULL) {
        Get_vtable_DrawSystem()->ctor(p);
        return p;
    }
    return NULL;
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

struct Class6C070Methods {
    BASICCLASS_SLOTS(Class6C070, (Class6C070 *self));
    /* +0x040 */ void (*init)(Class6C070 *self);                 /* DrawSystem__Init */
    /* +0x044 */ void *slot44;
    /* +0x048 */ void *slot48;
    /* +0x04C */ void *slot4C;
    /* +0x050 */ void *slot50;
    /* +0x054 */ void *slot54;
    /* +0x058 */ void *slot58;
    /* +0x05C */ void *slot5C;
    /* +0x060 */ void *slot60;
    /* +0x064 */ void *slot64;
    /* +0x068 */ void (*slot68)(Class6C070 *self);               /* DrawSystem__RunLoop */
    /* +0x06C */ void *slot6C;
    /* +0x070 */ void (*slot70)(Class6C070 *self, s32 value);    /* DrawSystem__SetVSyncCount */
    /* +0x074 */ void *slot74;
    /* +0x078 */ void *slot78;
    /* +0x07C */ void *slot7C;
    /* +0x080 */ void (*slot80)(Class6C070 *self, s32 value);    /* DrawSystem__SetSyncMode */
};
```

## Naming

Proposed name `New_Class6C070` (tier B: shape of every other New_* allocator); not applied, class unnamed.
