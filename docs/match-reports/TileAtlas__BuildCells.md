# TileAtlas__BuildCells -- MATCHED (61/61 words)

> Renamed from `func_800450B4` on 2026-09-25 (tools/rename.py). Address 0x800450b4.

Round 82, runner echo (code_33808 session, echo #9), 2026-09-25. Unit `code_33808`.
Byte-exact; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 61/61 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

When +0x30 is set, allocate 300 GsCELLs (0x960 bytes) at +0x2C and fill them as 16x16-texel cells over VRAM from x 0x280: u steps by 16, x by 16; at x 0x3C0 the row wraps (u 0, x 0x280, v += 16); whenever x crosses a 64-pixel texture page the cell's tpage becomes x >> 6 (plus 16 once v reaches 0x100) and u restarts at 0. The first page comes from GetTPage(2, 0, 0x280, 0). cba and flag are zero. The gTileMapMethods object (20 x 15 grid of 16 x 16 cells, index table 0..299) is its GsMAP partner.

Table slot (`tools/classtable.py`): D_8006F514 +0x078.

## Source

The unit-local views `DataSrc33808` (Class6D430 subclass via the unified
`CLASS6D430_SLOTS`/`CLASS6D430_FIELDS` macros plus `slot7C`/`slot80` and own
fields +0x2C..+0x38), `Ctor33808`, `CountedBuf33808` and `Req44858` sit at the
top of / earlier in `src/code_33808.c`.

```c
/* D_8006F514 +0x078: when +0x30 is set, build 300 GsCELLs (16 x 16 texels
 * each) at +0x2C over the texture pages from x 0x280: u,v step by 16, a new
 * row at x 0x3C0, a new texture page every 64 x (the lower half from v
 * 0x100). */
typedef struct Cell450B4 {       /* LIBGS.H GsCELL */
    /* +0x00 */ u8 u;
    /* +0x01 */ u8 v;
    /* +0x02 */ u16 cba;
    /* +0x04 */ u16 flag;
    /* +0x06 */ u16 tpage;
} Cell450B4;

/* LIBGPU.H */
extern u16 GetTPage(int tp, int abr, int x, int y);

void TileAtlas__BuildCells(Obj6F514 *self) {
    Cell450B4 *c;
    s32 x = 0x280;
    s32 u;
    s32 v;
    s32 tpage;
    s32 i;
    s32 n;

    if (self->unk30 != 0) {
        v = 0;
        u = 0;
        tpage = GetTPage(2, 0, 0x280, 0);
        self->cells = BMemPMgrAlloc(300 * sizeof(Cell450B4));
        if (self->cells != NULL) {
            i = 0;
            c = self->cells;
            n = 300;
            for (; i < n; i++, c++) {
                c->u = u;
                c->tpage = tpage;
                c->v = v;
                c->cba = 0;
                c->flag = 0;
                u += 16;
                x += 16;
                if (x >= 0x3C0) {
                    u = 0;
                    x = 0x280;
                    v += 16;
                }
                if ((x & 0x3F) == 0) {
                    tpage = x >> 6;
                    if (v >= 0x100) {
                        tpage += 16;
                    }
                    u = 0;
                }
            }
        }
    }
}
```

## Notes

Eleventh build. The cell is LIBGS.H's GsCELL (u, v, cba, flag, tpage), unit-local as `Cell450B4`; the unit-local `Obj6F514` view's `pad2C[4]` became `struct Cell450B4 *cells` (no other function reads it through that view; whole image green). Three separate levers, each measured: (1) store order `u, tpage, v, cba, flag` -- written in field order the second loop pointer (retail `addiu v1,a0,4`) came out at +6 and the stores reordered (21/61); (2) the loop bound in a register (`li a2,0x12c` + `slt`) = a local `n = 300` assigned just before the loop -- the literal gives `slti`, and assigning n before the allocation kept it in s4 across the call; (3) `self->cells = BMemPMgrAlloc(...); if (self->cells != NULL) { i = 0; c = self->cells; n = 300; for (; i < n; ...)` -- that exact statement order i, c, n (the other three orders tried each moved one `move`/`li`, 58-59/61), and `self->cells = c = ...; if (c)` added a `move a0,v0` ahead of the test.

## Naming

- **TileAtlas__BuildCells**, tier A. Slot +0x078: builds 300 GsCELLs (16x16 texels each) tiling the texture pages from x 0x280, wrapping rows and switching texture pages every 64 texels.

## Track 4 (2026-09-26, round 88, alpha)

Class unified in `include/TileAtlas.h` (D_8006F514, 0x303, a Class6D430 subclass, 0x38 bytes). `self` is `TileAtlas *` (was `Obj6F514`); the cell type is LIBGS.H's `GsCELL`, now defined in the header (was the unit-local `Cell450B4`, same layout), and the flag at +0x030 is `defaultCells`. `cells` is the array TileMap__BuildMap takes as its GsMAP base. No rename. Byte-identical.
