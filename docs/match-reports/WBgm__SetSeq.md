# WBgm__SetSeq -- MATCHED (58/58 words), round 81

> Renamed from `func_80039E7C` on 2026-09-25 (tools/rename.py). Address 0x80039e7c.

Round 81, runner delta. Unit `src/sound/WBgm.c` (carved from `psyq_2a0e0` in
FINISHING-PLAN revision 18). Fresh ground, no prior attempt.

- **Where:** gWBgmMethods slot +0x05C (setSeq) (slots resolved with `tools/classtable.py gWBgmMethods`).
- **What:** set the SEQ data: stop if playing, release and clear +0x10, and if the argument is non-zero load `New_RequestedFile(arg)` into +0x10 and try-open; on success play if auto-play (+0x20), on failure mark `openState = 1` if it was 0.
- **Result:** byte-exact on the first build. `funcdiff.py` reports 58/58
  words, 0 insertions / 0 deletions, and the whole-image SHA1 is green
  (`OK: build matches retail`).
- **Levers:** none; nested `if` inside the success arm and `else if` for the failure arm give retail's `j` to the epilogue.
- **Name:** renamed round 82 to `WBgm__SetSeq` (see `## Naming` below).

## Naming

`WBgm__SetSeq`, tier A. vtable slot `setSeq`; body replaces the +0x10 SEQ-data child and re-tries HandleMonitorEvent, matching "set which SEQ data this player uses".

## View changes (additive, unit-local)

Round 81 delta extended echo's `WBgm` view in `src/code_2a0e0.c`: the ctor
slot's parameter list is now `(WBgm *self, s32 vabArg, s32 seqArg, s32
autoPlay)` (was `(WBgm *self)`; no matched function called it), slots
+0x048..+0x060 are declared, and +0x0C / +0x10 are typed as the local views
`SeqVab` / `SeqData` instead of `void *`. Every other function in the unit
still matches after the change.

## Source

```c
void WBgm__SetSeq(WBgm *self, s32 arg) {
    if (self->playing != 0) {
        self->methods->stop(self);
    }
    if (self->seqData != NULL) {
        self->seqData->methods->release((BasicClass *)self->seqData);
        self->seqData = NULL;
    }
    if (arg != 0) {
        self->seqData = New_RequestedFile(arg);
        if (WBgm__HandleMonitorEvent(self)) {
            if (self->autoPlay != 0) {
                self->methods->play(self);
            }
        } else if (self->openState == 0) {
            self->openState = 1;
        }
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
    BASICCLASS_SLOTS(WBgm, (WBgm *self, s32 vabArg, s32 seqArg, s32 autoPlay)); /* WBgm__WBgm */
    /* +0x040 */ void (*update)(WBgm *self, s32 arg1, s32 arg2); /* WBgm__Update */
    /* +0x044 */ void (*play)(WBgm *self);                       /* WBgm__Play */
    /* +0x048 */ void (*stop)(WBgm *self);                       /* WBgm__Stop */
    /* +0x04C */ void (*pause)(WBgm *self);                      /* WBgm__Pause */
    /* +0x050 */ void (*resume)(WBgm *self);                     /* WBgm__Resume */
    /* +0x054 */ void (*setVol)(WBgm *self, s16 l, s16 r);       /* WBgm__SetVol */
    /* +0x058 */ void (*crescendo)(WBgm *self, s16 v, s32 s);    /* WBgm__Crescendo */
    /* +0x05C */ void (*setSeq)(WBgm *self, s32 arg);            /* WBgm__SetSeq */
    /* +0x060 */ void (*setVab)(WBgm *self, s32 arg);            /* WBgm__SetVab */
};

/* What +0x0C holds: a New_VabStreamObj object. Only the fields read here. */
typedef struct SeqVab {
    BASICCLASS_FIELDS(BasicClassMethods);
    /* +0x00C */ u8 padC[0x54 - 0xC];
    /* +0x054 */ s16 vabId;
    /* +0x056 */ u8 pad56[2];
    /* +0x058 */ u16 ready;
} SeqVab;

/* What +0x10 holds: a New_RequestedFile object. Only the fields read here. */
typedef struct SeqData {
    BASICCLASS_FIELDS(BasicClassMethods);
    /* +0x00C */ u8 padC[0x10 - 0xC];
    /* +0x010 */ unsigned long *addr;
    /* +0x014 */ u8 pad14[0x2C - 0x14];
    /* +0x02C */ s32 loaded;
} SeqData;

struct WBgm {
    BASICCLASS_FIELDS(WBgmMethods);
    /* +0x00C */ SeqVab *vab;
    /* +0x010 */ SeqData *seqData;
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
extern short SsSeqOpen(unsigned long *addr, short vab_id);

extern void *BMemPMgrAlloc(s32 size);
extern BasicClass *GetDrawSystem(void);
extern SeqData *New_RequestedFile(s32 arg);
extern SeqVab *New_VabStreamObj(s32 arg0);
extern void printf(const char *fmt);
extern const char sSeqOpenErrorMsg[]; /* "Seq Open error in WBgmHandleMonitorEvent" */
WBgmMethods *GetWBgmMethods(void);

extern s32 GetSsTicksPerSecond(void);
s32 WBgm__HandleMonitorEvent(WBgm *self);

extern WBgmMethods gWBgmMethods;
extern s32 sWBgmActive;
extern u8 sSsSizeTableBuf[];
```

## Track 4 (2026-09-26, round 87, RequestedFile)

The unit-local view `SeqData` (BASICCLASS_FIELDS, +0x010 `addr`, +0x02C
`loaded`) and its `extern SeqData *New_RequestedFile(s32)` are gone: the
census missed them, as New_RequestedFile's return type. `WBgm::seqData` is
`RequestedFile *` (`include/RequestedFile.h`, round 87, delta). `addr` is
FileResource's `buffer` (+0x010), `loaded` the same field under the same
name; the release calls lose their `(BasicClass *)` casts and WBgm__SetSeq
casts its s32 argument to the ctor's `char *name`. The whole image stays
byte-identical; the Source block above is the earlier text.

## Track 4 (2026-09-26, round 88, alpha)

The class is now declared once, in `include/WBgm.h` (table `gWBgmMethods`, renamed from `D_8006E48C` with tools/rename.py this round); `src/code_2a0e0.c` keeps no view of it, so the view quoted in this report's source section is historical. Image byte-identical after every step.

Parameter retyped `s32 arg` -> `char *seqPath` (slot +0x05C with it); the `(char *)` cast before New_RequestedFile is gone and the test reads `seqPath != NULL`. Callers: WBgm__WBgm (its seqPath argument; the one construction passes NULL). No C outside this unit names the slot (grep; INCLUDE_ASM callers through +0x05C were not searched).
