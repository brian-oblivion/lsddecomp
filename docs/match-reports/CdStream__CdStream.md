# CdStream__CdStream -- MATCHED (exact length, 59/59 words), round 82

> Renamed from `CdStreamObj__CdStreamObj` on 2026-09-26 (tools/rename.py). Address 0x80046f88.

> Renamed from `func_80046F88` on 2026-09-25 (tools/rename.py). Address 0x80046f88.

Round 82, runner delta (second session). Unit `src/code_3770c.c`. Fresh
ground, no prior attempt. Byte-exact on build 8; whole-image SHA1 green.

- **Where:** slot +0x008 (ctor) of gCdStreamMethods; `New_CdStream` (the
  allocator) calls it with `(obj, arg1, arg2, arg3)`.
- **What:** BasicClass ctor; install the method table; +0x34 = arg1 (a
  speed/mode selector: `< 4` unsigned picks 300, else 150); muted = 0;
  +0x38 = `(rate / arg2 / 2) * 2054`; +0x3C = arg3; ring and the three
  callbacks NULL; state 0.
- **Levers:** the rate MUST be an inline conditional expression inside the
  division, not a local:
  `self->unk38 = (((arg1 < 4) ? 300 : 150) / arg2 / 2) * 2054;`.

  | form | score |
  | --- | --- |
  | `rate = 150; if (arg1 < 4) rate = 300;` then `unk38 = rate/arg2/2*2054` | 58 words (1 short), 29/59 |
  | `rate = (arg1 < 4) ? 300 : 150;` (local) | same |
  | `if/else` into the local | same |
  | local + `__asm__("")` after the unk38 store | 54/59 (tail fixed, `li 0x96` still hoisted out of the beqz delay slot) |
  | `rate = 150` hoisted above the stores | 5/59 |
  | local + barrier before the if | 51/59 |
  | condition on `(u32)self->unk34` | 17/59 |
  | **inline ternary, no local, no barrier** | **59/59** |

  With a named local, cc1's scheduler hoisted `li v1,150` to the top of the
  block (the delay slot got `sw zero,0x30`) and pulled the six trailing
  stores up into the `mflo` shadow. Inlined, both go away. The barrier was
  never needed.
- **Context:** local view gained `s32 unk38` (+0x38) and `s32 unk3C` (+0x3C).

## Naming

Tier A. `CdStream__CdStream` -- slot +0x008, the ctor (`Class__Class` convention, BasicClass.h's model). Evidence: installs the method table, initializes every field the rest of the unit reads (state, muted, speed, bytesPerFrame, ring, the three callbacks).

### Field, slot and global names (round 82 naming pass)

`struct CdStreamObj`/`CdStreamObjMethods` are unit-local (no other unit
references them, `gCdStreamMethods` or `gActiveCdStream` -- confirmed
by `grep -rn` over `src/`, `include/`), so these were renamed directly
(CLAUDE.md/FINISHING-PLAN track 3 step 3's ownership rule), not proposed:

| old | new | tier | evidence |
| --- | --- | --- | --- |
| `D_800817E0` (table) | `gCdStreamMethods` | A | `Get_vtable_<Class>` / `g<Class>Methods` convention (`include/Pad.h`) |
| `D_8008A950` (global) | `gActiveCdStream` | A | the single active-stream pointer every state-changing method compares `self` against |
| slot +0x044 | `open` | A | dispatches to `CdStream__Open` |
| slot +0x048 | `close` | A | dispatches to `CdStream__Close` |
| slot +0x050 | `startRead` | A | dispatches to `CdStream__StartRead` |
| slot +0x054 | `stop` | A | dispatches to `CdStream__Stop` |
| slot +0x058 | `restart` | A | dispatches to `CdStream__Restart` |
| slot +0x06C | `getNextFrame` | A | dispatches to `CdStream__GetNextFrame` |
| slot +0x05C/+0x060/+0x07C | left `slot5C`/`slot60`/`slot7C` | C | empty overrides, no evidence of intended purpose |
| `loc` (+0x00C) | `seekLoc` | B | the buffer passed to `seek`/`CdControl`'s `CdlLOC` argument |
| `unk2C` | `state` | A | the 0/1/2/4 state machine every method gates on |
| `unk34` | `speed` | B | selects both the ctor's rate divisor and StartRead's read mode |
| `unk38` | `bytesPerFrame` | A | divides the file size into `totalFrames` in `CdStream__Open` |
| `unk3C` | left `unk3C` | C | ctor stores `arg3` there; no further read is visible in this unit |
| `unk40` | `totalFrames` | A | compared against the running frame index in `GetNextFrame` |
| `unk58` | `lastFrame` | A | the last frame index `GetNextFrame` recorded |
| `cb48` | `onFrameReady` | B | called in `CdStream__ReleaseFrame` when a frame's buffer is ready to release |
| `cb4C` | `onStreamEnd` | B | tested (never itself invoked) to gate `CdStream__OnStreamEnd`'s notify-on-end path |
| `cb54` | `onSeekDone` | A | the callback `OnCdSeekComplete` invokes when an async seek completes |

## Source

```c
void CdStream__CdStream(CdStreamObj *self, u32 arg1, s32 arg2, s32 arg3) {
    Get_vtable_BasicClass()->ctor((BasicClass *)self);
    self->methods = Get_vtable_CdStream();
    self->unk34 = arg1;
    self->muted = 0;
    self->unk38 = (((arg1 < 4) ? 300 : 150) / arg2 / 2) * 2054;
    self->unk3C = arg3;
    self->ring = NULL;
    self->cb4C = NULL;
    self->cb48 = NULL;
    self->cb54 = NULL;
    self->unk2C = 0;
}
```

### Proposed learning

A constant select that feeds a division (`li` in the beqz delay slot, the
other `li` in the fall-through, then `div`) came out right only when written
as an inline `?:` inside the arithmetic. Writing it through a named local
changed the schedule of the whole block (the `li` was hoisted and the
trailing stores moved into the `mflo` shadow), even though it compiles to
the same instructions.
