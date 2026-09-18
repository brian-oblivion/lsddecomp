> Renamed from `func_80056DF8` on 2026-09-18 (tools/rename.py). Address 0x80056df8.

# LinkOwnerObj__ReleaseLinks -- MATCHED (9/9 words)

Unit: `class_3bb8c_o` (round 17). A one-line wrapper releasing a 5-element
`BasicClass *` array inline at `self+0x84`.

## Final source

```c
typedef struct LinkElemObj LinkElemObj;
typedef struct LinkElemMethods {
    u8 pad0[0x48];
    void (*slot48)(LinkElemObj *self, s32 arg1, Vec3O *arg2); /* +0x048 */
} LinkElemMethods;
struct LinkElemObj {
    LinkElemMethods *methods; /* +0x000 */
    u8 pad4[0x80];             /* +0x004 .. +0x083, unknown */
    s32 unk84;                   /* +0x084 */
};

typedef struct LinkOwnerObj {
    u8 pad0[0x84];              /* +0x000 .. +0x083, unknown */
    LinkElemObj *arr84[5];        /* +0x084 .. +0x097 */
} LinkOwnerObj;

extern void ReleaseBasicClassArray(void **array, s32 count);

void LinkOwnerObj__ReleaseLinks(LinkOwnerObj *this) {
    ReleaseBasicClassArray((void **)this->arr84, 5);
}
```

## Derivation

`addiu $a0, $a0, 0x84` then `jal ReleaseBasicClassArray` with `$a1 = 5` -- the
address of `this+0x84` is passed directly (not loaded through it), so
`+0x84` is an INLINE array field, not a pointer field. `ReleaseBasicClassArray` is
already established elsewhere (`code_8220_b.c`) as
`void ReleaseBasicClassArray(BasicClass **array, s32 count)` -- a release-all-N loop.
Kept generic `void **` here rather than pulling in `BasicClass` from
`code_8220.h`, matching this project's existing looser per-unit reading of
the same symbol (`code_2cc8c.h`'s `void ReleaseBasicClassArray(void *a0, void *a1)`).

`this` is NOT the same class as `BaseObjO` (the shared intermediate base
class the rest of this unit implements, see the file banner) -- `+0x84`
would overflow that class's 0x58-byte allocation (`New_BaseObjO`). It is
kept as its own independent local type, `LinkOwnerObj`, established
together with `LinkOwnerObj__RandomizeLinks` (which walks indices 1..4 of the SAME
5-element array) and `LinkOwnerObj__ReleaseLinksB` (byte-identical body to this
function).

### Proposed learning

None beyond what's already documented -- straightforward wrapper.
