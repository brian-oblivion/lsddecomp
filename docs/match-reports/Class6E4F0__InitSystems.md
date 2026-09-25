# Class6E4F0__InitSystems

> Renamed from `func_8003B044` on 2026-09-25 (tools/rename.py). Address 0x8003b044.

**Round 81 (delta)** · **Unit:** code_2b78c · **Size:** 49 words · **Status:** MATCHED (49/49 words, whole-image SHA1 green, first build)

## What it does

Slot `+0x044` of D_8006E4F0; Class6D3C8's own +0x044
(ForwardToBaseSlot44UnlessFlagged) forwards here. One-time system init,
guarded by `self->initialized` (+0x18):

- `SetDrawSystem(source)` (stores the source to a gp global);
- `source->methods->slot44(source, &self->dims, self->dimsArg)`;
- `SsInit(); GsInit3D();`
- allocates a 0x14-byte aux block into `self->aux` (+0x1C) holding
  {source, arg, 0, 0, 0};
- sets `initialized = 1`.

```c
void Class6E4F0__InitSystems(Class6E4F0 *self, Class6E4F0Source *source, s32 arg) {
    if (self->initialized == 0) {
        SetDrawSystem(source);
        source->methods->slot44(source, &self->dims, self->dimsArg);
        SsInit();
        GsInit3D();
        self->aux = BMemPMgrAlloc(0x14);
        self->aux->source = source;
        self->aux->arg = arg;
        self->aux->unk08 = 0;
        self->aux->unk0C = 0;
        self->aux->unk10 = 0;
        self->initialized = 1;
    }
}
```

Retail reloads `self->aux` before every store (`lw v0,0x1C(s0)` five
times), which is exactly what writing through `self->aux->` each time
gives; a local `aux` would have kept it in a register.

The `source` object's class is unknown (its +0x044 takes a
`ScreenDims *` and an s32); typed as the local `Class6E4F0Source`.
code_1677c calls this slot with a 4th argument `0` that this body never
reads.

## Naming

**Round 81 (delta), track 3.** Renamed `func_8003B044` -> `Class6E4F0__InitSystems`.
**Tier B**: the body does one-time init of multiple genuine Psy-Q subsystems
(`SsInit` sound, `GsInit3D` graphics, plus forwarding to the source object's
own display-setup slot and allocating the aux block) guarded by
`self->initialized`, so "init" and "systems" (plural) are both evident from
the body. It stops short of A because the overall GAME purpose of this
one-time setup -- what it is initializing the game system FOR -- is not
established, only which SDK calls it makes.

## Track 4

**2026-09-25, round 84 (echo).** The class is declared once, in
`include/Class6E4F0.h`. The parameters are now `drawSystem` and `pad`, and
the aux block's first two fields likewise: the one caller chain is main()
-> Class6D3C8's +0x044 override -> this slot, and main passes
`New_DrawSystem()` and `New_Pad(0, 0)` (src/main.c). The draw system's
+0x044 slot is `initGraph` (DrawSystem__InitGraph), `dimsArg` is `vramMode`.
The slot type (`CLASS6E4F0_SLOTS`) carries a fourth `s32` argument this body
does not declare: Class6D3C8__ForwardToBaseSlot44UnlessFlagged calls the slot
with `(self, a1, a2, 0)`, and the `move a3,zero` in its jalr delay slot is
retail's. The slot's return is `void`, the occupant's; the old subclass view
typed it `s32`, and its one caller ignores $v0. Bytes unchanged.
