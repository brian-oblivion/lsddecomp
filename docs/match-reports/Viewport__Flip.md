# Viewport__Flip — MATCHED

> Renamed from `Unk18Obj__Flip` on 2026-09-25 (tools/rename.py). Address 0x8003f04c.

> Renamed from `func_8003F04C` on 2026-09-23 (tools/rename.py). Address 0x8003f04c.

Unit: `Task`. Round 14, runner delta. 87/87 words, full match (2
real attempts).

## Signature

```c
void Viewport__Flip(Unk18Obj *self);
```

`Unk18ObjMethods`'s own `+0x0A4` slot occupant (`slotA4`, dispatched by
`Viewport__OnNotifyTag1`, this round).

## What it does

Recomputes `self->unk74` from `self->unkC->methods->slot54`. If
`self->unkB8` is set, resets the graphics context (PsyQ `ResetGraph`) and
notifies `self->unkC->methods->slot50` — once unconditionally, plus once
more if `unk74` is still 0 — then forwards the current `unk74`-indexed
slot to two rendering helpers. If `self->unkB4` is also set, notifies
`slot50` a further time when `unk74` is still 0. Finally collapses
`self->unk74` to a plain boolean: 1 if it's 0, else 0.

```c
void Viewport__Flip(Unk18Obj *self) {
    s32 idx;
    u8 *rawBytes;

    if (self->unk70 == 0) {
        return;
    }

    self->unk74 = self->unkC->methods->slot54(self->unkC);
    if (self->unkB8 == 0) {
        goto tail_check;
    }

    ResetGraph(1);
    self->unkC->methods->slot50(self->unkC);

    if (self->unkB4 != 0) {
        if (self->unk74 == 0) {
            self->unkC->methods->slot50(self->unkC);
        }
    }

    idx = self->unk74;
    rawBytes = (u8 *)&self->unk58;
    GsSortClear(rawBytes[0], rawBytes[1], rawBytes[2],
                  *(s32 *)((u8 *)self + 0x78 + idx * 4));

    idx = self->unk74;
    func_8003FBF4(*(s32 *)((u8 *)self + 0x78 + idx * 4));

    if (self->unkB4 != 0 && self->unk74 == 0) {
        self->unkC->methods->slot50(self->unkC);
    }

tail_check:
    self->unk74 = (self->unk74 == 0);
}
```

## What the first attempt got wrong

Misread the tail's control flow as an early-exit special case (`if
(self->unk74 != 0) { self->unk74 = 0; return; }`) instead of recognizing
it as the SAME "only dispatch `slot50` when `unk74 == 0`" guard already
used earlier in the function, both converging on the identical final
`self->unk74 = (self->unk74 == 0);` computation. Retail's own `bnez
v0,.L8003F190` jumps INTO the shared tail computation (with the boolean
already computed in the branch's delay slot), not to a separate inline
epilogue — writing the guard as `if (self->unkB4 != 0 && self->unk74 ==
0) { dispatch }` followed by one shared tail statement reproduces this
exactly, and is simpler than the first attempt besides.

## Header changes

`include/Task.h`:
- `GenericObjMethods` gains `slot50` (`void`, return unused) and `slot54`
  (`s32`, return stored into `self->unk74`) — real occupants unknown
  (`self->unkC`'s dynamic class isn't otherwise identified in this unit).
- New externs: `ResetGraph` (PsyQ, `LIBGPU.H`'s own declared signature —
  declared locally rather than including the whole SDK header, matching
  this unit's existing style for PsyQ calls), `GsSortClear` (PsyQ,
  reads `self->unk58`'s bytes UNSIGNED — same "writer reads signed, this
  reader reads unsigned" situation as `unk5B`/`Viewport__Update`, this
  round), `func_8003FBF4` (next slice, uncarved).

## Naming

`Unk18Obj__Flip` -- tier A. The `slotA4` occupant: recomputes `unk74` (a 0/1 double-buffer index) from a child object's own method, optionally resets the graphics context, clears+draws the CURRENT OT half (`GsSortClear`+`GsDrawOt`, both indexed by `unk74`), then collapses `unk74` to the other of 0/1 for next call -- the standard double-buffer flip shape (recompute index, drain current buffer, toggle for next frame).

## Track 4 (2026-09-25, round 85, bravo)

Renamed from `Unk18Obj__Flip`. Slot +0x0A4 `flip`. `unkC` is `drawSystem` (the class-1 child, gDrawSystemMethods), whose +0x054 (DrawSystem__GetActiveBuffer) gives otIndex and +0x050 (DrawSystem__SwapBuffers) swaps; reached through Task.h's GenericObj view with a cast, since DrawSystem has no header. `unkB8` is `drawEnabled`; the OT reads are `ot[idx]`. The class (id 0x7, table `gViewportMethods`, formerly `D_8006E8E4`) is unified as `Viewport` in `include/Viewport.h`, whose banner gives the evidence for the name: its methods hold a GsRVIEW2 (GsSetRefView2), the projection and near clip, a double-buffered GsOT pair, draw the scene tree into it and flip it; IntermediateBase and TaskCore already called the field holding it `viewport`. Any source block above is the pre-unification spelling; the live body takes the unified types and field and slot names, byte-identical.

## Sony's headers (round 95, alpha, polish pass)

src/app/Task.c now includes `<libgte.h>`, `<libgpu.h>` and `<libgs.h>` and its local prototypes of Sony functions are gone; every call takes Sony's own declaration, byte-identical. Interim casts at this function's call sites, until include/Viewport.h's ViewportOt/ViewportRefView become Sony's GsOT/GsRVIEW2: `GsSortClear(..., (GsOT *)self->ot[idx])` and `GsDrawOt((GsOT *)self->ot[idx])`.

The comments that sat on the deleted prototypes, moved here verbatim:

```c
/* Sony's `GsDrawOt` (libgs/gs_111, linked from the SDK object since round
 * 34; was func_8003FBF4, and was declared in include/Task.h until this
 * round). Local for the same collision reason as the three above. Sony's own
 * argument is a `GsOT *`; this call site passes the same otIndex-indexed slot
 * it hands GsClearOt, as a plain word, and is left that way.
 * gs_111 and gs_112 are byte-identical objects defining GsDrawOt and
 * GsDrawOtIO at this one address -- gs_111/GsDrawOt is what the build links. */
extern void GsDrawOt(ViewportOt *ot);

/* Sony's `GsSortClear` (libgs/gs_001, fingerprint exact vs the disc corpus,
 * not yet linked from an SDK object). Local for the same collision reason as
 * the three above: LIBGS.H's own prototype is `void GsSortClear(u_char r,
 * u_char g, u_char b, GsOT *ot);`. This call site reads self->clearColor's own
 * three bytes UNSIGNED (same "writer reads signed, this reader reads
 * unsigned" situation as farColor/Viewport__Update) and passes the fourth as a
 * plain word, same as GsClearOt/GsDrawOt above. */
extern void GsSortClear(u8 a0, u8 a1, u8 a2, ViewportOt *ot);
```


## Track 7 (round 95, alpha, polish pass)

The clear colour is passed as `self->clearColor.r/.g/.b` (GsSortClear's own `unsigned char` parameters make the load `lbu`), no `u8 *` over the struct; the slot-offset notes (+0x050, +0x054) are gone from the comment, the slot names say it. Byte-identical.

Round 96 (alpha, track 6). include/Viewport.h's local `ViewportOt` (a
0x14-byte view of the GsOT header: length, org, pad) is deleted: `ot[2]` is
Sony's `GsOT *`, `otTags[2]` Sony's `GsOT_TAG *` (each header's `org`) and
`workBase[2]` Sony's `PACKET *` (GsSetWorkBase's argument), and every unit
including Viewport.h takes Sony's headers after common.h. The `(GsOT *)`
casts at GsClearOt, GsSortClear, GsDrawOt and drawNode's five sort calls and
Update's `(PACKET *)` cast are gone. Byte-identical (whole image green).


## Track 7 (round 100, echo, polish pass)

Field `unkB4` is now `extraSwap` (see Viewport__SetExtraSwap.md).

## History: include/code_2cc8c.h before round 100 (moved, verbatim)

Round 100 merged the carve slices code_2c054, code_2cc8c, _b, _c and _d into
src/Task.c (track 8), and the head folded their header (include/code_2cc8c.h,
briefly include/TaskViewport.h) into include/Task.h. Its live declarations
moved; `TexPageDesc`, `HeaderObj` and `EventArg` had no user left and were
dropped. The header as it stood, for the record:

```c
#ifndef TASKVIEWPORT_H
#define TASKVIEWPORT_H

#include "common.h"
#include "BasicClass.h"
#include "SceneNode.h"
#include "BoxFill.h"
#include "FadeBox.h"
#include "IntermediateBase.h"
#include "TaskCore.h"
#include "TextRow.h"

/*
 * src/Task.c's own declarations (it was include/code_2cc8c.h, the
 * header of the whole code_2cc8c carve block, and code_2cc8c_e.c and
 * code_2cc8c_f.c still include it). The classes are in their own headers:
 * TaskCore.h, IntermediateBase.h, Viewport.h, BoxFill.h, FadeBox.h,
 * TextRow.h.
 */

/* Forward typedefs, used by `extern` declarations further up this file
 * than their own struct bodies (round 14, code_2cc8c_e's own local views,
 * referenced by earlier call-site declarations in Task.c). */
typedef struct TexPageDesc TexPageDesc;

/*
 * TaskCore (class id 0x130, gTaskCoreMethods) is declared once, in
 * include/TaskCore.h (track 4, round 84): Task.c's sections 1 to 3
 * are its methods from +0x058 on. Its
 * object used to be viewed here as `Obj86B60`, after the address of a
 * SUBCLASS table (gTitleMenuMethods); the views are gone.
 *
 * What stays below are this unit family's views of the classes TaskCore
 * holds and has no header for yet: TaskCore.h types those fields
 * `BasicClass *` and the accessors here cast to the view (`sound`, a
 * VabStreamObj, is cast to include/VabStreamObj.h's type; `bgLayer` is a
 * BgLayer, include/BgLayer.h since round 88 (its `Unk78Obj` view is gone);
 * `subHandle` and TaskCoreTarget's `handle` are TimImages, include/TimImage.h,
 * cast at Task's accessors; the slot and item widgets are TextRows,
 * include/TextRow.h; `listView` is a BoxFill, include/BoxFill.h). The
 * record TaskCoreTarget::unk24[] points to is Task.c's `SlotEntry`.
 */
typedef struct HeaderObj HeaderObj;
typedef struct EventArg EventArg;

/*
 * FOR THE NEXT RUNNER (Task, same 153-function block, same class
 * framework): a note on how much to trust `Unk48Obj`/`Unk4CObj`/`Unk78Obj`
 * below, since all three are minimal placeholder types and it matters
 * which part of that is "confirmed small" vs. "not yet looked at".
 *
 * - **The `padNNN[K]` byte ranges in all three are UNOBSERVED, not
 *   confirmed-unused.** Nothing in this unit's 16 matched + 4 stalled
 *   functions ever reads or writes those bytes -- that is the entire
 *   basis for calling them padding. It is NOT evidence those bytes are
 *   inert; per this project's class-framework shape (CLAUDE.md, "Writing
 *   a class method"), every one of these three is almost certainly a full
 *   object with its own real fields beyond offset 0, this unit's
 *   functions simply never touch them. Treat every `padNNN` here as "ends
 *   here only because our evidence ends here", and extend/narrow it the
 *   moment a function in `Task` (or any other unit) reads inside
 *   one of these ranges.
 * - **Offset 0 being a method-table pointer IS confirmed for two of the
 *   three** (`Unk48Obj`, `Unk78Obj`) -- each is dereferenced through the
 *   `lw self,0; lw slot,N(methods); jalr` idiom at least once, which is
 *   real evidence, not a framework assumption.
 * - **`Unk4CObj` is now `TaskCoreTarget`** (include/TaskCore.h, round 84):
 *   a plain data record, the menu description the ctor and setTarget take.
 * - **`Unk78Obj` is now `BgLayer`** (include/BgLayer.h, round 88): 0x68
 *   bytes (New_BgLayer), its +0x0B8 is BgLayer__SetColor.
 * - **None of the three has a known SIZE.** Each struct below is only as
 *   large as its highest observed field plus that field's own size --
 *   `Unk48Obj` could plausibly be anywhere from 0x084 bytes (just past
 *   `slot80`) to much larger; `Unk78Obj` is at least 0x0BC bytes. If a
 *   future unit's allocator call reveals a literal byte count for any of
 *   these three classes, record it here.
 */

/*
 * A generic "event" argument, round 13: `arg1->target->header` is read by
 * IntermediateBase__OnNotify to pick which of self->methods->slot54/58/5C to forward to.
 * Same two-type shape as class_39e08.h's own independent `EventArg`/
 * `HeaderObj` local view (a `target` pointer to an object whose first word
 * is a low-nibble-coded header/kind value) -- this unit keeps its own
 * separate local view per the project's established convention. Only the
 * one field/offset IntermediateBase__OnNotify touches is modelled.
 */
struct HeaderObj {
    s32 header; /* +0x000 */
};

struct EventArg {
    HeaderObj *target; /* +0x000 */
};

extern void *BMemPMgrAlloc(s32 size);                   /* allocator, confirmed across many
                                            units */
extern void *BMemPMgrFree(void *ptr);                   /* matching free/release. Its own
                                            disassembly (still INCLUDE_ASM,
                                            asm/nonmatchings/BMemPMgr/
                                            BMemPMgrFree.s) ends with an
                                            explicit `addu $v0,$zero,$zero`
                                            -- it genuinely returns NULL,
                                            not void. code_171e0.h/Entity.h
                                            type it `void` because every
                                            caller there discards the
                                            result (the established
                                            "a discarded return value is
                                            never evidence of void" trap);
                                            round 14 (code_2cc8c_f) needs
                                            the real return value, so this
                                            unit's shared view is retyped.
                                            Every existing call site in
                                            this unit (Task.c's
                                            sections 2 and 4) discards the
                                            result too, so this is a
                                            zero-byte-cost retype -- full
                                            build reconfirmed green. */
extern void ReleaseBasicClassArray(void *a0, void *a1); /* not yet seen elsewhere in
                                                    this project; typed from
                                                    TaskCore__ReleaseSlotElements's own call
                                                    site only */


/* New_FadeBox: include/FadeBox.h. */
/* New_SceneNode: include/SceneNode.h (it was a local Unk18AcObj view). */
extern u8 gViewportFadeBoxSize[]; /* address-taken only by this unit: (320, 240), the
                            size Viewport's ctor passes New_FadeBox */
extern u8 gFadeBoxAttachPos[];    /* address-taken only by this unit: (-100, -100), the
                            screen position Viewport's ctor and SetSubHandle
                            attach the sub handle at (include/Viewport.h) */
extern Ratio16 gDefaultViewTwist; /* {0, 1}: Viewport__AttachViewChild's twist
                            when its own is NULL (asm/data/7B048.sdata.s) */

/* GsSetRefView2 is NO LONGER DECLARED HERE, round 33. It is Sony's
   (`libgs/gs_131.o`, linked from the SDK object) and will one day sit next to
   `include/psyq/libgs.h`'s own prototype for it -- two declarations of one
   Sony name in a header six units include is the `conflicting types` failure
   that CLAUDE.md and the SDK guide both warn about, and it would surface in a
   unit that never touched this line. Its one caller, Viewport__AttachViewChild, now
   declares it locally in src/Task.c with that call site's own shape. */

/* func_8003FC18 is NO LONGER DECLARED HERE, round 34 -- exactly the
   GsSetRefView2 case above. It is Sony's `GsClearOt` (`libgs/gs_113.o`,
   linked from the SDK object), and a second declaration of that name in a
   header six units include is the `conflicting types` failure CLAUDE.md and
   the SDK guide both warn about. Its callers declare it locally under Sony's
   name, with their own call sites' shapes, in src/Task.c.
   The `TexPageDesc *` view that used to hang off this prototype was
   code_2cc8c_e.c's reading of a Psy-Q `GsOT`; the struct stays in this header
   because other code uses it, and no byte depends on the naming. */

/* DrawSync is NO LONGER DECLARED HERE, round 53. It is Sony's
   `DrawSync` (`libgpu/sys.o`, fingerprint exact vs the disc corpus, not yet
   linked from an SDK object) -- same collision reason as GsSetRefView2 and
   GsClearOt above: LIBGPU.H carries its own prototype (`extern int
   DrawSync(int mode);`), and a second declaration of that name in a header
   six units include is the `conflicting types` failure CLAUDE.md and the SDK
   guide both warn about. Its one caller, Viewport__DeinitOt, now declares it
   locally in src/Task.c with that call site's own shape. */

/* The following are called only from Viewport__Update (this unit). They are
   plain `void *` global setters (this call site happens to pass an
   already-`s32`-shaped value, which is an ordinary int-to-pointer conversion
   with identical codegen, same precedent as SceneNode__DispatchLinkCommand/SceneNode__TryAttachNearby in
   code_d294.h); the retypes come from the functions' own definitions and are
   ABI-identical (word-sized values either way), so they do not change this
   call site's own compiled bytes.
   ROUND 34: both are still GAME CODE, but neither lives in code_2cc8c_e any
   more -- they are the two one-function units src/libgs_gs_101.c and
   src/libgs_gs_124.c, wedged between Sony objects. Those files do NOT
   include this header, so these two declarations are not checked against
   their definitions by the compiler; they agree today and must be kept in
   step by hand.
   Their two former neighbours, func_8003FC70 and func_8003FD4C, are gone from
   here: they are Sony's `GsSetLightMode` (libgs/gs_108) and `SetFogNear`
   (libgte/fog_01), declared locally in src/Task.c under those names
   for the same collision reason as GsSetRefView2 and GsClearOt above. */
/* ROUND 78: the two setters that used to be declared here are Sony's too --
   GsSetNearClip (libgs/gs_101) and GsSetWorkBase (libgs/gs_124) -- and are
   now declared locally in src/Task.c from LIBGS.H, like the pair
   above. */

/* ResetGraph (asm/psyq_10ee0.s, PsyQ library, LIBGPU.H's own
   declared signature is `extern int ResetGraph(int mode);` -- declared
   locally here rather than including the whole SDK header, matching this
   unit's existing PsyQ-declaration style). Viewport__Flip calls it with a
   literal 1 and ignores the return. */
extern s32 ResetGraph(s32 mode);

/* GsSortClear is NO LONGER DECLARED HERE, round 53. It is Sony's
   `GsSortClear` (`libgs/gs_001.o`, fingerprint exact vs the disc corpus, not
   yet linked from an SDK object) -- same collision reason as GsClearOt above:
   LIBGS.H carries its own prototype (`void GsSortClear(u_char r, u_char g,
   u_char b, GsOT *ot);`), and a second declaration of that name in a header
   six units include is the `conflicting types` failure CLAUDE.md and the SDK
   guide both warn about. Its one caller, Viewport__Flip, now declares it
   locally in src/Task.c with that call site's own shape (self->unk58's
   three bytes read unsigned, same "writer reads signed, this reader reads
   unsigned" situation as unk5B/Viewport__Update, plus one more word). */

/* func_8003FBF4 is NO LONGER DECLARED HERE, round 34. It is Sony's
   `GsDrawOt` (`libgs/gs_111.o`, linked from the SDK object) -- same
   collision reason as GsSetRefView2/GsClearOt above. Its one caller,
   Viewport__Flip, declares it locally in src/Task.c with that call
   site's own shape. */

/* GsSetProjection is NO LONGER DECLARED HERE, round 79. It was
   `Unk18Obj__SetGeomScreen`, typed as a method; it is Sony's (libgs/gs_106,
   identified at merge from position and LIBGS.H's prototype) and its one
   caller, Viewport__Update in src/Task.c, declares it
   locally with LIBGS.H's own shape. */

/* New_FrameClock: include/FrameClock.h. */
/* New_LightRig: include/LightRig.h. */


/*
 * BoxFill (class id 0x64, gBoxFillMethods) is declared once, in
 * include/BoxFill.h (track 4, round 85). Its methods are code_2cc8c_e's
 * New_BoxFill/ctor/Reset and code_2cc8c_f's first thirteen functions; the
 * two views this header used to hold of it (`ClassEAC0Obj` and the 0x64 half
 * of `Obj6EAC0`) are gone.
 */

/*
 * TextRow (class id 0x11144, gTextRowMethods) is declared once, in
 * include/TextRow.h (track 4, round 88): code_2cc8c_f's New_TextRow to
 * GetTextRowMethods are its methods. This header's two views of it are gone:
 * `Obj6EAC0` (named after BoxFill's old table address; the class is below
 * CharSprite, not BoxFill) and `Unk64Elem`, the slot and item widgets
 * TaskCore holds (`slotElements`, `itemLists`), which New_TextRow makes.
 */

/* The packed-bitfield accessor SceneNode's attribute setters use
 * (code_d294), reached here by BoxFill's over `&self->boxAttribute`. */
extern u32 GetSetBitField(u32 *word, s32 shift, s32 width, u32 value);

/* gBoxFillMethods and GetBoxFillMethods: include/BoxFill.h. */
/* gTextRowMethods, GetTextRowMethods and New_TextRow: include/TextRow.h.
 * GetCharSpriteMethods and New_CharSprite: include/CharSprite.h. */

/* GetSceneNodeMethods and its table: include/SceneNode.h (track 4, round 81). */

/* FadeBox (class id 0x164, gFadeBoxMethods): include/FadeBox.h (track 4,
 * round 87; it was a local FadeBoxObj view here). */

/* BoxFillSize and BoxFillPos: include/BoxFill.h. */

/* A small "shift/stride/width/height" texture-page-like descriptor --
 * func_8003FC18's own 3rd argument (round 14, code_2cc8c_e). Only the
 * fields that function touches are named. (typedef forward-declared near
 * the top of this file, see there.) */
struct TexPageDesc {
    s32 shift;  /* +0x000 */
    s32 stride; /* +0x004 */
    s32 width;  /* +0x008 */
    s32 height; /* +0x00C */
    s32 size;   /* +0x010, computed = (4 << shift) + stride - 4 */
};

/* FadeBox's colour tables (include/FadeBox.h), indexed at a 3-byte
 * stride by a channel mask (`i*3`, no further scaling). Retail bytes:
 * gFadeBoxMaskColors is eight RGB entries (0 and 7 FFFFFF, 1 0000FF, 2 00FF00,
 * 4 FF0000); gFadeBoxBlackColors follows it and its entries are black (0x00), indexed
 * by FadeBox__StartFadeUp and used whole for mask 0xF. `gBoxFillDefaultColor` is
 * BoxFill__Reset's default colour, 808080, one use. */
extern u8 gFadeBoxMaskColors[];
extern u8 gFadeBoxBlackColors[];
extern u8 gBoxFillDefaultColor[3];

#endif
```
