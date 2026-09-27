# TmdModel__NextPrimitive -- MATCHED (288/288 words), round 82

> Renamed from `func_80020050` on 2026-09-25 (tools/rename.py). Address 0x80020050.

Round 82, runner charlie (matching slot, second pass on the unit). Unit `src/TmdModel.c`. Fresh ground, no prior attempt.

- **What:** a TMD primitive iterator over the object `self->object` (TMD object-table entry: +0x10 `prims`, +0x14 `nprims`). Returns NULL once `*count` reaches `nprims`; on `*count == 0` it restarts at `prims`. It switches on the primitive's mode byte (0x20..0x3F, `jtbl_80010354`, which this function owns). Each case picks the three or four vertex-index words out of the primitive (their position depends on the mode and on `flag & 4`) and the primitive's size. It then copies the indexed vertices' x,y,z into `out`, sets `*n` (3, 4, or 0 for an unknown mode), bumps `*count`, and returns `p + size`. `size` is uninitialised on the default path, as in retail.
- **Result:** byte-exact; 288/288 words, whole-image SHA1 green. Build 14 on this function.
- **Builds / levers, measured:**
  1. natural switch (`*n = 3;` written in every 3-vertex case): 280/288. Right length, and cross-jumping already produced retail's shared tails, including the 0x38-with-flag case jumping into case 0x3C's body. The only residue was `n` (a2 copy) and `verts` swapped between t4/t5.
  2. `verts` declared first / `rec` removed / `verts` in an inner block / `*out++` / a `{Vec3 v; s16 pad;}` vertex view: all 280, no change.
  3. reading `self->object->verts` inside the loop (no local): 5/288, size changed.
  4. `goto tri;` with `tri: *n = 3;` as a separate label at the END of the switch: 60/288, block order changed.
  5. **`goto tri;` from every 3-vertex case, with `tri:` placed at the tail of the LAST 3-vertex case (0x35), where retail's shared block sits**: 287/288. Fewer `*n` refs before allocation lowered `n`'s priority and fixed the t4/t5 swap. Left: `addu v0,v0,t4` where retail has `addu v0,t4,v0`.
  6. `verts + idx[i]`, `idx[i] + verts`, `(u8 *)verts + idx[i] * 8`, `u32 idx[]`: all 287.
  7. **`(u8 *)verts + (idx[i] << 3)`**: 288/288.
- **Types:** `TmdObject_fa50` got its remaining fields (`normals`, `nnormals`, `prims`, `nprims` (u32: retail compares with `sltu`), `scale`). New `TmdPrim_fa50` (4 header bytes + u16 words). All in this unit.

## Source

```c
typedef struct SVec_fa50 { s16 x, y, z, pad; } SVec_fa50;
typedef struct Vec3_fa50 { s16 x, y, z; } Vec3_fa50;
typedef struct TmdPrim_fa50 { u8 olen; u8 ilen; u8 flag; u8 mode; u16 h[20]; } TmdPrim_fa50;
typedef struct TmdObject_fa50 {
    SVec_fa50 *verts; s32 nverts; void *normals; s32 nnormals;
    TmdPrim_fa50 *prims; u32 nprims; s32 scale;
} TmdObject_fa50;
/* TmdModel: +0x010 TmdObject_fa50 *object (see New_TmdModel.md) */

TmdPrim_fa50 *TmdModel__NextPrimitive(TmdModel *self, TmdPrim_fa50 *p, s32 *n, Vec3_fa50 *out, u32 *count) {
    s32 idx[4];
    TmdObject_fa50 *rec = self->object;
    SVec_fa50 *verts;
    s32 size;
    s32 i;

    if (rec->nprims == 0 || *count >= rec->nprims) {
        return NULL;
    }
    if (*count == 0) {
        p = rec->prims;
    }
    *n = 4;
    switch (p->mode) {
    case 0x20:
    case 0x22:
        if (p->flag & 4) {
            idx[0] = p->h[7];
            idx[1] = p->h[8];
            idx[2] = p->h[9];
            size = 0x18;
            goto tri;
        } else {
            idx[0] = p->h[3];
            idx[1] = p->h[4];
            idx[2] = p->h[5];
            size = 0x10;
            goto tri;
        }
        break;
    case 0x21:
    case 0x23:
        idx[0] = p->h[2];
        idx[1] = p->h[3];
        idx[2] = p->h[4];
        size = 0x10;
        goto tri;
    case 0x24:
    case 0x26:
        idx[0] = p->h[7];
        idx[1] = p->h[8];
        idx[2] = p->h[9];
        size = 0x18;
        goto tri;
    case 0x25:
    case 0x27:
        idx[0] = p->h[8];
        idx[1] = p->h[9];
        idx[2] = p->h[10];
        size = 0x1C;
        goto tri;
    case 0x28:
    case 0x2A:
        if (p->flag & 4) {
            idx[0] = p->h[9];
            idx[1] = p->h[10];
            idx[2] = p->h[11];
            idx[3] = p->h[12];
            size = 0x20;
        } else {
            idx[0] = p->h[3];
            idx[1] = p->h[4];
            idx[2] = p->h[5];
            idx[3] = p->h[6];
            size = 0x14;
        }
        break;
    case 0x29:
    case 0x2B:
        idx[0] = p->h[2];
        idx[1] = p->h[3];
        idx[2] = p->h[4];
        idx[3] = p->h[5];
        size = 0x10;
        break;
    case 0x2C:
    case 0x2E:
        idx[0] = p->h[9];
        idx[1] = p->h[10];
        idx[2] = p->h[11];
        idx[3] = p->h[12];
        size = 0x20;
        break;
    case 0x2D:
    case 0x2F:
        idx[0] = p->h[10];
        idx[1] = p->h[11];
        idx[2] = p->h[12];
        idx[3] = p->h[13];
        size = 0x20;
        break;
    case 0x30:
    case 0x32:
        if (p->flag & 4) {
            idx[0] = p->h[7];
            idx[1] = p->h[9];
            idx[2] = p->h[11];
            size = 0x1C;
            goto tri;
        } else {
            idx[0] = p->h[3];
            idx[1] = p->h[5];
            idx[2] = p->h[7];
            size = 0x14;
            goto tri;
        }
        break;
    case 0x31:
    case 0x33:
        idx[0] = p->h[6];
        idx[1] = p->h[7];
        idx[2] = p->h[8];
        size = 0x18;
        goto tri;
    case 0x34:
    case 0x36:
        idx[0] = p->h[7];
        idx[1] = p->h[9];
        idx[2] = p->h[11];
        size = 0x1C;
        goto tri;
    case 0x35:
    case 0x37:
        idx[0] = p->h[12];
        idx[1] = p->h[13];
        idx[2] = p->h[14];
        size = 0x24;
    tri:
        *n = 3;
        break;
    case 0x38:
    case 0x3A:
        if (p->flag & 4) {
            idx[0] = p->h[9];
            idx[1] = p->h[11];
            idx[2] = p->h[13];
            idx[3] = p->h[15];
            size = 0x24;
        } else {
            idx[0] = p->h[3];
            idx[1] = p->h[5];
            idx[2] = p->h[7];
            idx[3] = p->h[9];
            size = 0x18;
        }
        break;
    case 0x39:
    case 0x3B:
        idx[0] = p->h[8];
        idx[1] = p->h[9];
        idx[2] = p->h[10];
        idx[3] = p->h[11];
        size = 0x1C;
        break;
    case 0x3C:
    case 0x3E:
        idx[0] = p->h[9];
        idx[1] = p->h[11];
        idx[2] = p->h[13];
        idx[3] = p->h[15];
        size = 0x24;
        break;
    case 0x3D:
    case 0x3F:
        idx[0] = p->h[16];
        idx[1] = p->h[17];
        idx[2] = p->h[18];
        idx[3] = p->h[19];
        size = 0x2C;
        break;
    default:
        *n = 0;
        break;
    }
    verts = self->object->verts;
    for (i = 0; i < *n; i++) {
        out[i] = *(Vec3_fa50 *)((u8 *)verts + (idx[i] << 3));
    }
    (*count)++;
    return (TmdPrim_fa50 *)((u8 *)p + size);
}
```

### Proposed learning

- A multiply-by-power-of-two index where retail's `addu` has the BASE register first (`addu v0, base, v0`) is an explicit shift on byte arithmetic, `(u8 *)base + (i << 3)`: `&base[i]`, `base + i`, `i + base` and `(u8 *)base + i * 8` all give the canonical `addu v0, v0, base`.
- In a switch whose several cases end in the same statement, a register swap involving that statement's pointer can come from its reference COUNT before allocation (cross-jumping merges the copies only after reload). Writing it once, behind a label at the tail of the case where retail's shared block physically sits, with `goto` from the others, keeps the layout and lowers the count. A label at the end of the switch moves the block and costs the layout.

## Naming

`TmdModel__NextPrimitive` -- tier A. A TMD primitive iterator over
`self->object` (the object-table entry): decodes the current primitive's mode
byte, extracts its vertex indices, writes their positions to `out`, advances
`*count`, and returns the next primitive pointer (or NULL when exhausted).
Owns `jtbl_80010354`.

## Track 7 (2026-09-26, round 94, bravo)

Added a `/* MATCHING: ... */` comment on the vertex-copy line per this
file's build log above (build 7): `verts[idx[i]]` and every other
pointer-arithmetic spelling tested (`&base[i]`, `base + i`, `i + base`,
`(u8 *)base + i * 8`) score 287/288, one register swapped from retail; only
`(u8 *)verts + (idx[i] << 3)` reaches 288/288. No source change, comment
only.

## Track 7, re-send (2026-09-26, round 94, bravo)

Constants, all from Sony's `<libgs.h>` (the unit now includes `<libgpu.h>`
and `<libgs.h>` after `<libgte.h>`, which also supplies the
`GsMapModelingData` prototype the unit used to declare itself). Byte-identical,
first build.

- **Case labels.** The `switch` is on the packet's mode byte, which is the GPU
  command code of the face it draws. Each case pair is one of libgs.h's
  `GPU_COM_*` codes and the same code with bit 0x02 set: `0x20/0x22` is
  `GPU_COM_F3`, `0x21` `GPU_COM_NF3`, `0x24` `GPU_COM_TF3`, `0x25`
  `GPU_COM_NTF3`, `0x28` `GPU_COM_F4`, `0x29` `GPU_COM_NF4`, `0x2C`
  `GPU_COM_TF4`, `0x2D` `GPU_COM_NTF4`, and the same eight with 0x10 set for
  gouraud (`GPU_COM_G3` .. `GPU_COM_NTG4`). Bit 0x02 has no Sony constant; it
  is the bit libgpu's `setSemiTrans` macro sets in a primitive's code byte, so
  the unit spells it `TMD_MODE_ABE` (Sony's TMD documentation calls the bit
  ABE), a `#define` with that evidence on its definition.
- **`flag & 4`** is `GsTMDFlagGRD` (libgs.h), the TMD flag for a face with a
  colour per vertex: the branch it selects is the `TMD_P_*G` layout.
- **Packet sizes** are `sizeof` Sony's packet struct for that code, checked
  one by one against the old literals: F3 16 / F3G 24, NF3 16, TF3 24, TNF3 28,
  F4 20, NF4 16, TF4 32, TNF4 32, G3 20 / G3G 28, NG3 24, TG3 28, TNG3 36,
  G4 24, NG4 28, TG4 36, TNG4 44. (Sony's struct for code `NTF3` is spelled
  `TMD_P_TNF3`, and likewise `TNF4`, `TNG3`, `TNG4`.)
- **Vertex indices** are read as those structs' own `v0`..`v3` fields through
  a unit-local `PRIM(type)` cast macro, instead of `p->h[k]`: every `k` was
  checked to be the struct's `vN` offset (`(offset - 4) / 2`, `h` starting
  after the 4-byte header).
- **Two sizes stay literals, in decimal:** the gradated quads (`GPU_COM_F4`
  and `GPU_COM_G4` with `GsTMDFlagGRD`) have no Sony struct. Their layouts are
  `TMD_P_F4` / `TMD_P_G4` plus three colour words (rgb1..rgb3, as `TMD_P_F3G`
  adds two to `TMD_P_F3`): 20 + 12 = 32 and 24 + 12 = 36, with v0..v3 at
  `h[9..12]` and `h[9], h[11], h[13], h[15]`. These keep `p->h[]` and a
  one-line comment each; inventing a `TMD_P_F4G` would look like a Sony name.

`TmdPrim` itself (`olen`, `ilen`, `flag`, `mode`, `h[20]`) is Sony's packet
header `out, in, dummy, cd`; replacing it is a type job (track 6), proposed to
the head.

Later in the same pass: a `MATCHING:` line on the shared `tri:` tail (the
second proposed learning above). Comment only.
