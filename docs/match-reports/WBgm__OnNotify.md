# WBgm__OnNotify -- MATCHED (35/35 words), round 81

> Renamed from `func_80039B04` on 2026-09-25 (tools/rename.py). Address 0x80039b04.

Round 81, runner delta. Unit `src/code_2a0e0.c` (carved from `psyq_2a0e0` in
FINISHING-PLAN revision 18). Fresh ground, no prior attempt.

- **Where:** D_8006E48C slot +0x038 (onNotify) (slots resolved with `tools/classtable.py D_8006E48C`).
- **What:** onNotify override: base `onNotify`, then if the sender's class id (`sender->methods->header & 0xF`) is 1, calls own `update` (+0x040) with the same arguments.
- **Result:** byte-exact on the first build. `funcdiff.py` reports 35/35
  words, 0 insertions / 0 deletions, and the whole-image SHA1 is green
  (`OK: build matches retail`).
- **Levers:** none.
- **Name:** kept as the bare `func_` name (the class has no confirmed name;
  rodata `D_80010FEC` calls the code around it `WBgmHandleMonitorEvent`, a
  lead for naming the class `WBgm...`, not evidence yet).

## View changes (additive, unit-local)

Round 81 delta extended echo's `SeqObj` view in `src/code_2a0e0.c`: the ctor
slot's parameter list is now `(SeqObj *self, s32 vabArg, s32 seqArg, s32
autoPlay)` (was `(SeqObj *self)`; no matched function called it), slots
+0x048..+0x060 are declared, and +0x0C / +0x10 are typed as the local views
`SeqVab` / `SeqData` instead of `void *`. Every other function in the unit
still matches after the change.

## Source

```c
void WBgm__OnNotify(SeqObj *self, void *sender, s32 event) {
    Get_vtable_BasicClass()->onNotify((BasicClass *)self, sender, event);
    if ((((BasicClass *)sender)->methods->header & 0xF) == 1) {
        self->methods->update(self, (s32)sender, event);
    }
}
```

The unit-local view it needs, from the top of `src/code_2a0e0.c`:

```c
#include "BasicClass.h"

/* Local view of D_8006E48C's objects: a SEQ player. Fields named from the
 * libsnd calls they feed. */
typedef struct SeqObj SeqObj;
typedef struct SeqObjMethods SeqObjMethods;

struct SeqObjMethods {
    BASICCLASS_SLOTS(SeqObj, (SeqObj *self, s32 vabArg, s32 seqArg, s32 autoPlay)); /* WBgm__WBgm */
    /* +0x040 */ void (*update)(SeqObj *self, s32 arg1, s32 arg2); /* WBgm__Update */
    /* +0x044 */ void (*play)(SeqObj *self);                       /* WBgm__Play */
    /* +0x048 */ void (*stop)(SeqObj *self);                       /* WBgm__Stop */
    /* +0x04C */ void (*pause)(SeqObj *self);                      /* WBgm__Pause */
    /* +0x050 */ void (*resume)(SeqObj *self);                     /* WBgm__Resume */
    /* +0x054 */ void (*setVol)(SeqObj *self, s16 l, s16 r);       /* WBgm__SetVol */
    /* +0x058 */ void (*crescendo)(SeqObj *self, s16 v, s32 s);    /* WBgm__Crescendo */
    /* +0x05C */ void (*setSeq)(SeqObj *self, s32 arg);            /* WBgm__SetSeq */
    /* +0x060 */ void (*setVab)(SeqObj *self, s32 arg);            /* WBgm__SetVab */
};

/* What +0x0C holds: a New_VabStreamObj object. Only the fields read here. */
typedef struct SeqVab {
    BASICCLASS_FIELDS(BasicClassMethods);
    /* +0x00C */ u8 padC[0x54 - 0xC];
    /* +0x054 */ s16 vabId;
    /* +0x056 */ u8 pad56[2];
    /* +0x058 */ u16 ready;
} SeqVab;

/* What +0x10 holds: a func_800422CC object. Only the fields read here. */
typedef struct SeqData {
    BASICCLASS_FIELDS(BasicClassMethods);
    /* +0x00C */ u8 padC[0x10 - 0xC];
    /* +0x010 */ unsigned long *addr;
    /* +0x014 */ u8 pad14[0x2C - 0x14];
    /* +0x02C */ s32 loaded;
} SeqData;

struct SeqObj {
    BASICCLASS_FIELDS(SeqObjMethods);
    /* +0x00C */ SeqVab *unkC;
    /* +0x010 */ SeqData *unk10;
    /* +0x014 */ s16 seqId;
    /* +0x016 */ u8 pad16[0x1A - 0x16];
    /* +0x01A */ u16 state;
    /* +0x01C */ u16 paused;
    /* +0x01E */ u16 playing;
    /* +0x020 */ s32 unk20;
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
extern BasicClass *func_80020C5C(void);
extern SeqData *func_800422CC(s32 arg);
extern SeqVab *New_VabStreamObj(s32 arg0);
extern void printf(const char *fmt);
extern const char D_80010FEC[]; /* "Seq Open error in WBgmHandleMonitorEvent" */
SeqObjMethods *Get_vtable_WBgm(void);

extern s32 func_8002CC28(void);
s32 WBgm__HandleMonitorEvent(SeqObj *self);

extern SeqObjMethods D_8006E48C;
extern s32 gWBgmActive;
extern u8 gSsSizeTableBuf[];
```
