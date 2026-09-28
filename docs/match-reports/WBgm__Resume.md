# WBgm__Resume -- MATCHED (17/17 words), round 81

> Renamed from `func_80039DB0` on 2026-09-25 (tools/rename.py). Address 0x80039db0.

Round 81, runner echo. Unit `src/sound/WBgm.c` (carved from `psyq_2a0e0` in
FINISHING-PLAN revision 18). This was fresh ground with no prior attempt.

- **Where:** gWBgmMethods slot +0x050 (resolved with `tools/classtable.py gWBgmMethods`).
- **What:** resume: if `paused`, `SsSeqReplay` and clear it.
- **Result:** byte-exact on the first build. `funcdiff.py` reports 0
  insertions / 0 deletions, and the whole-image SHA1 is green
  (`OK: build matches retail`). No levers were needed.
- **Name:** renamed round 82 to `WBgm__Resume` (see `## Naming` below).

## Naming

`WBgm__Resume`, tier A. vtable slot `resume`; body is exactly SsSeqReplay plus the `paused` guard.

## Source

```c
void WBgm__Resume(WBgm *self) {
    if (self->paused != 0) {
        SsSeqReplay(self->seqId);
        self->paused = 0;
    }
}
```

The unit-local view it needs, from the top of `src/sound/WBgm.c`:

```c
#include "BasicClass.h"


/* Local view of gWBgmMethods's objects: a SEQ player. Fields named from the
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

extern s32 GetSsTicksPerSecond(void);
s32 WBgm__HandleMonitorEvent(WBgm *self);

extern WBgmMethods gWBgmMethods;
extern s32 gWBgmActive;
extern u8 gSsSizeTableBuf[];
```

## Track 4 (2026-09-26, round 88, alpha)

The class is now declared once, in `include/WBgm.h` (table `gWBgmMethods`, renamed from `D_8006E48C` with tools/rename.py this round); `src/code_2a0e0.c` keeps no view of it, so the view quoted in this report's source section is historical. Image byte-identical after every step.

No change to this function's signature or body.
