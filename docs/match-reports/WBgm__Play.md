# WBgm__Play -- MATCHED (22/22 words), round 81

> Renamed from `func_80039CBC` on 2026-09-25 (tools/rename.py). Address 0x80039cbc.

Round 81, runner echo. Unit `src/sound/WBgm.c` (carved from `psyq_2a0e0` in
FINISHING-PLAN revision 18). This was fresh ground with no prior attempt.

- **Where:** gWBgmMethods slot +0x044 (resolved with `tools/classtable.py gWBgmMethods`).
- **What:** play: if not `playing`, `SsSeqSetVol(id, 0x34, 0x34)`, `SsSeqPlay(id, 1, 0)`, set `playing`.
- **Result:** byte-exact on the first build. `funcdiff.py` reports 0
  insertions / 0 deletions, and the whole-image SHA1 is green
  (`OK: build matches retail`). No levers were needed.
- **Name:** renamed round 82 to `WBgm__Play` (see `## Naming` below).

## Naming

`WBgm__Play`, tier A. vtable slot `play`; body is exactly SsSeqPlay plus the `playing` guard -- a leaf whose mechanics are its purpose.

## Source

```c
void WBgm__Play(WBgm *self) {
    if (self->playing == 0) {
        SsSeqSetVol(self->seqId, 0x34, 0x34);
        SsSeqPlay(self->seqId, 1, 0);
        self->playing = 1;
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
extern u8 sSsSizeTableBuf[];
```

## Track 4 (2026-09-26, round 88, alpha)

The class is now declared once, in `include/WBgm.h` (table `gWBgmMethods`, renamed from `D_8006E48C` with tools/rename.py this round); `src/code_2a0e0.c` keeps no view of it, so the view quoted in this report's source section is historical. Image byte-identical after every step.

No change to this function's signature or body.
