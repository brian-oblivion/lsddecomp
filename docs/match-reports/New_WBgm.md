# New_WBgm -- MATCHED (31/31 words), round 81

> Renamed from `func_800398E0` on 2026-09-25 (tools/rename.py). Address 0x800398e0.

Round 81, runner delta. Unit `src/code_2a0e0.c` (carved from `psyq_2a0e0` in
FINISHING-PLAN revision 18). Fresh ground, no prior attempt.

- **Where:** D_8006E48C slot none (not in the table); allocator (slots resolved with `tools/classtable.py D_8006E48C`).
- **What:** `New_` wrapper: `BMemPMgrAlloc(0x24)`, then the ctor through `Get_vtable_WBgm()->ctor` (slot +0x008) with the three arguments, `return self` / `return NULL`.
- **Result:** byte-exact on the first build. `funcdiff.py` reports 31/31
  words, 0 insertions / 0 deletions, and the whole-image SHA1 is green
  (`OK: build matches retail`).
- **Levers:** the round lever `if (p != NULL) { ctor; return p; } return NULL;`, applied first time.
- **Name:** renamed round 82 to `New_WBgm` (see `## Naming` below).

## Naming

`New_WBgm`, tier A. the `New_<Class>` allocator convention (BasicClass.h); mechanics are its purpose (alloc + call the ctor slot).

## View changes (additive, unit-local)

Round 81 delta extended echo's `WBgm` view in `src/code_2a0e0.c`: the ctor
slot's parameter list is now `(WBgm *self, s32 vabArg, s32 seqArg, s32
autoPlay)` (was `(WBgm *self)`; no matched function called it), slots
+0x048..+0x060 are declared, and +0x0C / +0x10 are typed as the local views
`SeqVab` / `SeqData` instead of `void *`. Every other function in the unit
still matches after the change.

## Source

```c
WBgm *New_WBgm(s32 vabArg, s32 seqArg, s32 autoPlay) {
    WBgm *self;

    self = BMemPMgrAlloc(0x24);
    if (self != NULL) {
        Get_vtable_WBgm()->ctor(self, vabArg, seqArg, autoPlay);
        return self;
    }
    return NULL;
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

/* What +0x10 holds: a New_D8006EED8 object. Only the fields read here. */
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
extern SeqData *New_D8006EED8(s32 arg);
extern SeqVab *New_VabStreamObj(s32 arg0);
extern void printf(const char *fmt);
extern const char D_80010FEC[]; /* "Seq Open error in WBgmHandleMonitorEvent" */
WBgmMethods *Get_vtable_WBgm(void);

extern s32 func_8002CC28(void);
s32 WBgm__HandleMonitorEvent(WBgm *self);

extern WBgmMethods D_8006E48C;
extern s32 gWBgmActive;
extern u8 gSsSizeTableBuf[];
```
