# WBgm__WBgm -- MATCHED (54/54 words), round 81

> Renamed from `func_8003995C` on 2026-09-25 (tools/rename.py). Address 0x8003995c.

Round 81, runner delta. Unit `src/WBgm.c` (carved from `psyq_2a0e0` in
FINISHING-PLAN revision 18). Fresh ground, no prior attempt.

- **Where:** gWBgmMethods slot +0x008 (ctor) (slots resolved with `tools/classtable.py gWBgmMethods`).
- **What:** constructor: base ctor via `Get_vtable_BasicClass()->ctor`, installs `Get_vtable_WBgm()`, zeroes +0x0C/+0x10/+0x14/+0x1A/+0x1C/+0x1E, stores the third argument at +0x20 (auto-play flag), sets `gWBgmActive = 1` (gp_rel global), then calls its own slots +0x05C (with arg 2) and +0x060 (with arg 1) and registers itself as a child of the `GetDrawSystem()` singleton.
- **Result:** byte-exact on the first build. `funcdiff.py` reports 54/54
  words, 0 insertions / 0 deletions, and the whole-image SHA1 is green
  (`OK: build matches retail`).
- **Levers:** none; the `lw v0,0(s0)` reload after the `sw` to `self->methods` falls out of writing `self->methods->...` plainly.
- **Name:** renamed round 82 to `WBgm__WBgm` (see `## Naming` below).

## Naming

`WBgm__WBgm`, tier A. the ctor slot (+0x008); matches `Class__Class` convention (BasicClass.h). Body is the whole object's field init, confirmed by every setter it calls.

## Proposed names for symbols defined outside this unit

Not renamed here -- both are defined in a unit this round did not touch, and
one of them (`DrawSystem.c`) has a live matching runner this round
(`GetDrawSystem`, per the round-82 broadcast: alpha finished it just before
this pass started). Recorded as proposals for the head to apply with
`tools/rename.py` once safe.

- `GetDrawSystem` (defined `src/DrawSystem.c`, returns `Class6C070 *`): called
  here only as `addChild`/`removeChild`'s argument, registering `WBgm` as a
  child of that singleton for lifecycle notification -- the same pattern
  `class_3ac78.c` and `TimImage.c` use it for. No WBgm-specific evidence for
  its own name; not proposing one.
- `New_RequestedFile` (defined `src/Sprite.c`, an un-matched `INCLUDE_ASM`
  stall, signature `SeqData *New_RequestedFile(s32 arg)`): the only function that
  produces a `SeqData` object (the +0x10 child this unit reads `addr`/`loaded`
  from). A name like `GetSeqData`/`LoadSeqData` is plausible from this call
  site alone but not confirmed -- the function itself is still assembly, so
  its own body is not available as evidence. Low confidence; flagging only
  because it is the sole producer of a type this unit named.

## View changes (additive, unit-local)

Round 81 delta extended echo's `WBgm` view in `src/code_2a0e0.c`: the ctor
slot's parameter list is now `(WBgm *self, s32 vabArg, s32 seqArg, s32
autoPlay)` (was `(WBgm *self)`; no matched function called it), slots
+0x048..+0x060 are declared, and +0x0C / +0x10 are typed as the local views
`SeqVab` / `SeqData` instead of `void *`. Every other function in the unit
still matches after the change.

## Source

```c
void WBgm__WBgm(WBgm *self, s32 vabArg, s32 seqArg, s32 autoPlay) {
    Get_vtable_BasicClass()->ctor((BasicClass *)self);
    self->methods = Get_vtable_WBgm();
    self->vab = NULL;
    self->seqData = NULL;
    self->seqId = 0;
    self->openState = 0;
    self->paused = 0;
    self->playing = 0;
    self->autoPlay = autoPlay;
    gWBgmActive = 1;
    self->methods->setSeq(self, seqArg);
    self->methods->setVab(self, vabArg);
    self->methods->addChild(self, GetDrawSystem());
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

## Track 4 (2026-09-26, round 87, bravo)

The local GetDrawSystem/New_DrawSystem extern this unit carried is gone; it comes from `include/DrawSystem.h` (gDrawSystemMethods unified), with a pointer cast where this unit's own slot type asks for one. Byte-identical.

## Track 4 (2026-09-26, round 88, alpha)

The class is now declared once, in `include/WBgm.h` (table `gWBgmMethods`, renamed from `D_8006E48C` with tools/rename.py this round); `src/code_2a0e0.c` keeps no view of it, so the view quoted in this report's source section is historical. Image byte-identical after every step.

Parameters retyped to `(WBgm *self, char *vabPath, char *seqPath, s32 autoPlay)`, and the ctor slot with them: both paths are forwarded unchanged to setSeq/setVab, whose bodies hand them to New_RequestedFile(char *) and New_VabStreamObj(char *). See New_WBgm's Track 4 paragraph for the one caller.
