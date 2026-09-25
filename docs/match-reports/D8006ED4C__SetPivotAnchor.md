# D8006ED4C__SetPivotAnchor -- MATCHED (32/32 words), round 82

> Renamed from `func_80041E58` on 2026-09-25 (tools/rename.py). Address 0x80041e58.

Round 82, runner alpha (fifth slot on code_322b4). Unit `src/code_322b4.c`. Fresh ground, no prior body attempt.

- **Where:** D_8006EC74 and D_8006ED4C slot +0x0C0 (`tools/classtable.py`).
- **What:** When `parent` (+0x00C) is set, a five-case switch (owns jtbl_80011290) moves the GsSPRITE pivot: 0 = (w/2, h/2), 1 = mx 0, 2 = mx w, 3 = my 0, 4 = my h. Unsigned anchor (`sltiu`).
- **Result:** byte-exact; 32/32 words, 0 insertions / 0 deletions, whole-image SHA1 green (`./build-and-verify.sh` OK). First build.
- **Types:** typed through the UNIFIED `Sprite` (sprite.w/h/mx/my), unchanged.

## Source

```c
/* D_8006EC74 and D_8006ED4C slot +0x0C0: when attached, move the sprite's
 * pivot: 0 centre, 1 left, 2 right, 3 top, 4 bottom. */
void D8006ED4C__SetPivotAnchor(Sprite *self, u32 anchor) {
    if (self->parent != NULL) {
        switch (anchor) {
        case 0:
            self->sprite.mx = self->sprite.w >> 1;
            self->sprite.my = self->sprite.h >> 1;
            break;
        case 1:
            self->sprite.mx = 0;
            break;
        case 2:
            self->sprite.mx = self->sprite.w;
            break;
        case 3:
            self->sprite.my = 0;
            break;
        case 4:
            self->sprite.my = self->sprite.h;
            break;
        }
    }
}
```
