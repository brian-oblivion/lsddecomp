# FrameClock__FrameClock -- MATCHED (22/22 words), round 82

> Renamed from `D8006EF50__D8006EF50` on 2026-09-26 (tools/rename.py). Address 0x80042450.

> Renamed from `func_80042450` on 2026-09-25 (tools/rename.py). Address 0x80042450.

Round 82, runner alpha (fourth slot on code_322b4). Unit `src/code_322b4.c`. Fresh ground, no prior attempt.

- **Where:** gFrameClockMethods slot +0x008 (ctor).
- **What:** calls the BasicClass ctor on self, installs the gFrameClockMethods table (`Get_vtable_FrameClock()`), then calls its slot +0x040 (reset, FrameClock__Reset) with `(self, 0)`. Returns nothing (no `$v0` set after the last call).
- **Result:** byte-exact, 22/22 words, 0 ins / 0 del, whole-image SHA1 green. First build. The `move a0,s0; sw v0,0(a0)` store through `$a0` came for free from the natural three-statement body (charlie's round-82 note: the next call takes self as arg1).
- **Types:** the unit-local `D_8006EF50Obj` view gained `methods` at +0x000 (was padding) and a local `D_8006EF50Methods` with `reset` at +0x040; no shared header touched.

## Source

```c
typedef struct D_8006EF50Methods D_8006EF50Methods;
typedef struct D_8006EF50Obj {
    D_8006EF50Methods *methods; /* +0x000 */
    u8 pad04[0xC - 0x4];
    s32 count;
    s32 flag10;
    s32 flag14;
    s32 parentCursor;
} D_8006EF50Obj;
struct D_8006EF50Methods {
    u8 pad00[0x40];
    void (*reset)(D_8006EF50Obj *self, s32 a1); /* +0x040 = FrameClock__Reset */
};

void FrameClock__FrameClock(D_8006EF50Obj *self) {
    Get_vtable_BasicClass()->ctor((BasicClass *)self);
    self->methods = Get_vtable_FrameClock();
    self->methods->reset(self, 0);
}
```

## Naming

- `FrameClock__FrameClock` -- tier A. Ctor (slot +0x008): the BasicClass ctor, installs the table, then reset(0).

## Track 4 (2026-09-26, round 88, delta)

Renamed from `D8006EF50__D8006EF50`: the ctor, named `Class__Class` by convention. The class (id 0x5, table `gFrameClockMethods`, formerly `D_8006EF50`) is unified as `FrameClock` in `include/FrameClock.h`, whose banner holds the evidence for the class name: its tick (+0x044) is called by IntermediateBase__OnTag1Notify on the DrawSystem's per-VSync event 2, counts one frame and tells its parents event 2, or 3 while paused, or 4 while flag14 is set. Fields renamed: `count` -> `frameCount`, `flag10` -> `paused`. Any source block above is the pre-unification spelling (`D_8006EF50Obj`); the live body in `src/code_322b4.c` takes `FrameClock *`, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
