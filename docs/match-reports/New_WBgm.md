# New_WBgm -- MATCHED (31/31 words), round 81

> Renamed from `func_800398E0` on 2026-09-25 (tools/rename.py). Address 0x800398e0.

Round 81, runner delta. Unit `src/WBgm.c` (carved from `psyq_2a0e0` in
FINISHING-PLAN revision 18). Fresh ground, no prior attempt.

- **Where:** gWBgmMethods slot none (not in the table); allocator (slots resolved with `tools/classtable.py gWBgmMethods`).
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

The unit-local view it needs, from the top of `src/WBgm.c`:

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
WBgmMethods *Get_vtable_WBgm(void);

extern s32 GetSsTicksPerSecond(void);
s32 WBgm__HandleMonitorEvent(WBgm *self);

extern WBgmMethods gWBgmMethods;
extern s32 gWBgmActive;
extern u8 gSsSizeTableBuf[];
```

## Track 4 (2026-09-26, round 88, alpha)

The class is now declared once, in `include/WBgm.h` (table `gWBgmMethods`, renamed from `D_8006E48C` with tools/rename.py this round); `src/code_2a0e0.c` keeps no view of it, so the view quoted in this report's source section is historical. Image byte-identical after every step.

Parameters retyped `(s32 vabArg, s32 seqArg, s32 autoPlay)` -> `(char *vabPath, char *seqPath, s32 autoPlay)`. Caller checked: the only one is `DayTask__DayTask` (src/class_39e08.c), `New_WBgm(PickSoundBank(0), 0, 1)`; PickSoundBank (src/GameFiles.c) returns a word of gSoundBankPaths, and all seven words point at VAB path strings ("SND\\AMBIENT", "SND\\CARTOON", "SND\\ELECTRO", "SND\\ETHNOVA", "SND\\HUMAN", "SND\\LOVELY", "SND\\STANDERD"; asm/data/1B84.rodata.s), and the path reaches New_VabStreamObj(char *) through setVab. That call site now casts `(char *)` on the word and `(SubObjG *)` on the result, and `include/class_39e08.h`'s local `extern SubObjG *New_WBgm(s32, s32, s32)` view is deleted.

## Proposed field names

For the owners of those classes (NOT applied: they are other classes' views):
- `Obj865C8::unk40` (include/class_39e08.h, table gObjMMethods family): `bgm`, typed `struct WBgm *`, tier A. Evidence: assigned from `New_WBgm` in DayTask__DayTask, released through +0x004 in DayTask__Finalize, forwarded as New_ObjM's 2nd argument.
- The ObjM field at +0x054 (`ObjM_3bb8c_k::unk54`, src/class_3bb8c_k.c; shared view `FieldM50 *unk54`, include/class_3bb8c.h): `bgm`, `struct WBgm *`, tier A. Evidence: ObjM__ObjM stores New_ObjM's 2nd argument there (the WBgm above); ObjM__AdvancePauseSetup calls its +0x04C (WBgm__Pause) and ObjM__TeardownPauseOverlay its +0x050 (WBgm__Resume), both with `self` only, which agrees with WBgm's slot arity. `FieldM50` is also the type of ObjM's +0x010, a different object, so the retype would split the two fields' types rather than retype FieldM50.

## Track 4 (2026-09-26, round 88, DayTask)

Applied: the proposed `Obj865C8::unk40` -> `bgm`, `struct WBgm *` is DayTask::bgm in include/DayTask.h.

## History: the unit banner of src/code_2a0e0.c (moved here round 101, track 7)

The banner of `src/code_2a0e0.c` carried this history until round 101, when
track 7 rewrote it as documentation. Verbatim:

```c
/*
 * code_2a0e0 -- GAME code carved from psyq_2a0e0 on 2026-09-25 (FINISHING-PLAN
 * revision 18). 0x2A0E0..0x2A878 (vram 0x800398E0..0x8003A078). It was counted
 * as Psy-Q SDK by segment name; tools/gameinsdk.py measured it as game (a call
 * into game code, a method-table entry beside game methods, or contiguity with
 * those, and no Sony fingerprint). What it holds: WBgm, a background-music
 * SEQ player built on New_VabStreamObj; the yaml had called this gap "the
 * game's own libspu build".
 *
 * All 17 functions matched in round 81 (runners echo and delta); named round
 * 82 (runner bravo, FINISHING-PLAN track 3). Class named WBgm from rodata
 * sSeqOpenErrorMsg ("Seq Open error in WBgmHandleMonitorEvent"): the string is part
 * of WBgm__HandleMonitorEvent's own matched body (the printf sits right where
 * it is read), so it is body evidence for that function's name and, via its
 * "WBgm" prefix, a lead for the class -- weighed as evidence, not proof.
 *
 * Track 4 (round 88): the class is declared once, in include/WBgm.h (table
 * gWBgmMethods); this unit keeps no view of it.
 */
```
