# func_80057DBC -- MATCHED (14/14)

Round 47 (runner charlie). Unit: `src/class_3bb8c_q.c`, a BRAND NEW carve
(this unit did not exist before round 47). Class: table `D_800879C4` (49
slots, resolved with `tools/classtable.py 0x800879C4`) -- this is slot40,
the function tail-called by this class's own ctor `D800879C4__D800879C4` (see
that report, `src/class_3bb8c_p.c`) as `self->methods->slot40(self,
arg1)` once construction is otherwise complete.

Frameless leaf, zero `addiu $sp, $sp, -N`. The unit's carve-note history
had this filed as `addiu_at`-blocked; that construct was RESOLVED in
round 21 (`--addiu-at`), and `tools/uncarved.py` measures this function
blocker-clean. Matched first attempt, no iteration needed.

## Signature

```c
void func_80057DBC(D_800879C4Obj_q *self, s32 arg1);
```

`D_800879C4Obj_q` is THIS unit's own local view of the class -- the
neighbouring `class_3bb8c_p.c` already carries its own, smaller local
view (`D_800879C4Obj`, only the vtable pointer and `+0xA4`) of the SAME
table/object, established from `D800879C4__D800879C4`/`New_D800879C4`. Per the
project's multiple-independent-local-views convention this unit does not
edit that file; it defines its own struct sized for what these two
functions read/write. The allocator (`class_3bb8c_p.c`'s `New_D800879C4`)
sizes the object at `0xA8` bytes, which every offset in both views stays
inside.

## Body

```c
typedef struct D_800879C4Obj_q D_800879C4Obj_q;
struct D_800879C4Obj_q {
    u8 pad00[0x58];
    s32 unk58;                  /* +0x058 */
    s32 unk5C;                  /* +0x05C */
    s32 unk60;                  /* +0x060 */
    u8 pad64[0x74 - 0x64];
    s16 unk74;                  /* +0x074 */
    s16 unk76;                  /* +0x076 */
    u8 pad78[0x80 - 0x78];
    s16 unk80;                  /* +0x080 */
    s16 unk82;                  /* +0x082 */
    u8 pad84[0xA0 - 0x84];
    s32 unkA0;                  /* +0x0A0 */
};

extern const s16 D_80087AA4[];
extern const s16 D_80087AA6[];

void func_80057DBC(D_800879C4Obj_q *self, s32 arg1) {
    self->unkA0 = arg1;
    self->unk74 = D_80087AA4[arg1 * 2];
    self->unk76 = D_80087AA6[arg1 * 2];
}
```

## The rodata shape

`D_80087AA4` and `D_80087AA6` are splat's own dlabels in
`asm/data/76DC8.data.s`, 2 bytes and 6 bytes long respectively, sitting
back to back (`D_80087AA4` at `0x80087AA4`, `D_80087AA6` immediately
after at `0x80087AA6`) -- conceptually one 8-byte array of two
`{s16 a; s16 b;}` entries, but retail takes TWO SEPARATE `%hi`/`%lo`
bases (one lui/addiu pair per table) rather than a single struct-array
base, so declaring one combined struct type would not reproduce the two
relocations splat already emitted. The only spelling that does is two
parallel `s16[]` externs, each indexed at `arg1 * 2` (i.e. `arg1 * 4`
bytes -- retail computes the shift once, `sll $a1, $a1, 2`, and reuses it
for both address calculations, which the two-array C form reproduces
without any extra hoisting). Values, read at the real stride:
`D_80087AA4 = {0x03D0, 0x03E0}`, `D_80087AA6 = {0x01FF, 0x01FF}`.

## Verification

`./build-and-verify.sh` -- whole-image SHA1 matches retail. Committed
alongside `func_80057DF4` (same unit, same commit, ROM-address order
preserved).

### Proposed learning

Two splat dlabels that are contiguous in memory and individually shorter
than the addressing stride the code actually uses (`D_80087AA4` is 2
bytes but addressed at a 4-byte stride) are not necessarily one struct
array that a single C declaration should unify -- check whether the
disassembly takes ONE relocation or TWO before merging them. Here it's
two, so the correct C is two parallel arrays over one strided index, not
one struct-array declaration.
