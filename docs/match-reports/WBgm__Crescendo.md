# WBgm__Crescendo -- MATCHED (22/22 words), round 81

> Renamed from `func_80039E24` on 2026-09-25 (tools/rename.py). Address 0x80039e24.

Round 81, runner echo. Unit `src/code_2a0e0.c` (carved from `psyq_2a0e0` in
FINISHING-PLAN revision 18). This was fresh ground with no prior attempt.

- **Where:** D_8006E48C slot +0x058 (resolved with `tools/classtable.py D_8006E48C`).
- **What:** crescendo: `SsSeqSetCrescendo(self->seqId, vol, func_8002CC28() * scale)`.
- **Result:** byte-exact on the first build. `funcdiff.py` reports 0
  insertions / 0 deletions, and the whole-image SHA1 is green
  (`OK: build matches retail`). No levers were needed.
- **Name:** renamed round 82 to `WBgm__Crescendo` (see `## Naming` below).

## Naming

`WBgm__Crescendo`, tier A. vtable slot `crescendo`; body is exactly SsSeqSetCrescendo, scaled by func_8002CC28()'s return.

## Source

```c
void WBgm__Crescendo(WBgm *self, s16 vol, s32 scale) {
    SsSeqSetCrescendo(self->seqId, vol, func_8002CC28() * scale);
}
```

The unit-local view it needs, from the top of `src/code_2a0e0.c`:

```c
#include "BasicClass.h"


/* Local view of D_8006E48C's objects: a SEQ player. Fields named from the
 * libsnd calls they feed. */
typedef struct WBgm WBgm;
typedef struct WBgmMethods WBgmMethods;

struct WBgmMethods {
    BASICCLASS_SLOTS(WBgm, (WBgm *self));
    /* +0x040 */ void (*update)(WBgm *self, s32 arg1, s32 arg2); /* WBgm__Update */
    /* +0x044 */ void (*play)(WBgm *self);                       /* WBgm__Play */
};

struct WBgm {
    BASICCLASS_FIELDS(WBgmMethods);
    /* +0x00C */ void *vab;
    /* +0x010 */ void *seqData;
    /* +0x014 */ s16 seqId;
    /* +0x016 */ u8 pad16[0x1A - 0x16];
    /* +0x01A */ u16 openState;
    /* +0x01C */ u16 paused;
    /* +0x01E */ u16 playing;
    /* +0x020 */ s32 autoPlay;
};

/* libsnd (LIBSND.H) */
extern void SsSeqPlay(short, char, short);
extern void SsSeqPause(short);
extern void SsSeqReplay(short);
extern void SsSeqStop(short);
extern void SsSeqSetVol(short, short, short);
extern void SsSeqSetCrescendo(short, short, long);
extern void SsSeqClose(short);

extern s32 func_8002CC28(void);
s32 WBgm__HandleMonitorEvent(WBgm *self);

extern WBgmMethods D_8006E48C;
extern s32 gWBgmActive;
extern u8 gSsSizeTableBuf[];
```
