#ifndef CODE_2CC8C_H
#define CODE_2CC8C_H

#include "common.h"
#include "BasicClass.h"
#include "SceneNode.h"
#include "BoxFill.h"
#include "FadeBox.h"
#include "IntermediateBase.h"
#include "TaskCore.h"
#include "TextRow.h"

/* Forward typedefs, used by `extern` declarations further up this file
 * than their own struct bodies (round 14, code_2cc8c_e's own local views,
 * referenced by earlier code_2cc8c_d call-site declarations). */
typedef struct TexPageDesc TexPageDesc;

/*
 * TaskCore (class id 0x130, gTaskCoreMethods) is declared once, in
 * include/TaskCore.h (track 4, round 84): code_2cc8c.c, code_2cc8c_b.c and
 * the first function of code_2cc8c_c.c are its methods from +0x058 on. Its
 * object used to be viewed here as `Obj86B60`, after the address of a
 * SUBCLASS table (gTitleMenuMethods); the views are gone.
 *
 * What stays below are this unit family's views of the classes TaskCore
 * holds and has no header for yet: TaskCore.h types those fields
 * `BasicClass *` and the accessors here cast to the view (`sound`, a
 * VabStreamObj, is cast to include/VabStreamObj.h's type; `bgLayer` is a
 * BgLayer, include/BgLayer.h since round 88 (its `Unk78Obj` view is gone);
 * `subHandle` and TaskCoreTarget's `handle` are TimImages, include/TimImage.h,
 * cast at code_2cc8c_b's accessors; the slot and item widgets are TextRows,
 * include/TextRow.h; `listView` is a BoxFill, include/BoxFill.h). `SlotEntry` and
 * `SrcDesc` are two readings of one TaskCoreTarget::unk24[] record.
 */
typedef struct SrcDesc SrcDesc;
typedef struct HeaderObj HeaderObj;
typedef struct EventArg EventArg;

/*
 * FOR THE NEXT RUNNER (code_2cc8c_b, same 153-function block, same class
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
 *   moment a function in `code_2cc8c_b` (or any other unit) reads inside
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


/*
 * The pointee of TaskCoreTarget::unk24[idx] (round 12, from TaskCore__CommitElementScroll and
 * TaskCore__RefreshSlotView, cross-checked against already-matched TaskCore__CancelElementScroll's own
 * `((s32 *)self->unk4C->unk24[idx])[1]` read at the same +0x004 offset).
 * Only the three offsets these functions actually touch are modelled;
 * TaskCore__CancelElementScroll/TaskCore__SetSlotCursor's own `(u8 *)...unk24[idx] + 8` buffer usage
 * is left as a raw cast in those (already-matched) functions rather than
 * retrofitted onto this type, per this project's convention of not
 * editing matched functions to adopt a later, more specific type.
 */
/* Renamed from Unk24Elem, round 78 -- tier B, exclusive to this unit (only
 * code_2cc8c_b.c casts to this type; see this struct's own comment above
 * for the cross-unit `target->unk24[idx]` reads that stay untyped). */
typedef struct SlotEntry SlotEntry;

struct SlotEntry {
    u8 pad000[0x004];
    s32 savedCursor; /* +0x004, renamed from unk4, round 78 -- the
                    slot's own persisted ring-cursor value: SET here by
                    TaskCore__CommitElementScroll, READ back as `newVal` by the
                    already-matched TaskCore__CancelElementScroll */
    u8 pad008[0x010 - 0x008];
    s32 unk10; /* +0x010 */
    s32 unk14; /* +0x014, combined with unk10 and a per-slot counter into
                    a 2-word stack buffer (`{unk10, unk14 - counter*10}`)
                    passed by address to a TextRow setPosition call, then
                    incremented by 10 per loop iteration -- see
                    TaskCore__CommitElementScroll/TaskCore__RefreshSlotView */
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
                                            this unit (code_2cc8c_b.c,
                                            code_2cc8c_d.c) discards the
                                            result too, so this is a
                                            zero-byte-cost retype -- full
                                            build reconfirmed green. */
extern void ReleaseBasicClassArray(void *a0, void *a1); /* not yet seen elsewhere in
                                                    this project; typed from
                                                    TaskCore__ReleaseSlotElements's own call
                                                    site only */

/*
 * TaskCore__CreateSlotElements's 2nd parameter -- an unrelated "source list" descriptor,
 * NOT a TaskCore or any class in this unit's own hierarchy (no method
 * table dereference anywhere in that function). Only the two fields it
 * touches are modelled.
 */
struct SrcDesc {
    u8 pad000[0x004];
    s32 unk4; /* +0x004, becomes self->unk60[idx] */
    u8 pad008[0x018 - 0x008];
    char **unk18; /* +0x018, NULL-terminated array of C strings -- each
                      element is passed to strlen (already typed
                      `s32 strlen(char *s)` in code_171e0.h) and to
                      New_TextRow */
};

extern s32 strlen(char *s); /* Psy-Q libc2/strlen, linked from Sony's
                                        own object; local view here */


/* listView (+0x068) is a BoxFill (include/BoxFill.h): the accessors cast
 * TaskCore's `BasicClass *` to it. */

/* New_BoxFill: include/BoxFill.h. */
extern s32 sListViewSize[2];  /* address-taken only by this unit */
extern char D_8008A8F0[4]; /* address-taken only by this unit */

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
   declares it locally in src/code_2cc8c_d.c with that call site's own shape. */

/* func_8003FC18 is NO LONGER DECLARED HERE, round 34 -- exactly the
   GsSetRefView2 case above. It is Sony's `GsClearOt` (`libgs/gs_113.o`,
   linked from the SDK object), and a second declaration of that name in a
   header six units include is the `conflicting types` failure CLAUDE.md and
   the SDK guide both warn about. Its callers declare it locally under Sony's
   name, with their own call sites' shapes, in src/code_2cc8c_d.c.
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
   locally in src/code_2cc8c_d.c with that call site's own shape. */

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
   (libgte/fog_01), declared locally in src/code_2cc8c_d.c under those names
   for the same collision reason as GsSetRefView2 and GsClearOt above. */
/* ROUND 78: the two setters that used to be declared here are Sony's too --
   GsSetNearClip (libgs/gs_101) and GsSetWorkBase (libgs/gs_124) -- and are
   now declared locally in src/code_2cc8c_d.c from LIBGS.H, like the pair
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
   locally in src/code_2cc8c_d.c with that call site's own shape (self->unk58's
   three bytes read unsigned, same "writer reads signed, this reader reads
   unsigned" situation as unk5B/Viewport__Update, plus one more word). */

/* func_8003FBF4 is NO LONGER DECLARED HERE, round 34. It is Sony's
   `GsDrawOt` (`libgs/gs_111.o`, linked from the SDK object) -- same
   collision reason as GsSetRefView2/GsClearOt above. Its one caller,
   Viewport__Flip, declares it locally in src/code_2cc8c_d.c with that call
   site's own shape. */

/* GsSetProjection is NO LONGER DECLARED HERE, round 79. It was
   `Unk18Obj__SetGeomScreen`, typed as a method; it is Sony's (libgs/gs_106,
   identified at merge from position and LIBGS.H's prototype) and its one
   caller, Viewport__Update in src/code_2cc8c_d.c, declares it
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

/* SkipShort2 and BoxFillPos: include/BoxFill.h. */

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
 * D_8006EA90 is eight RGB entries (0 and 7 FFFFFF, 1 0000FF, 2 00FF00,
 * 4 FF0000); D_8006EAA8 follows it and its entries are black (0x00), indexed
 * by FadeBox__StartFadeUp and used whole for mask 0xF. `D_8008A924` is
 * BoxFill__Reset's default colour, 808080, one use. */
extern u8 D_8006EA90[];
extern u8 D_8006EAA8[];
extern u8 D_8008A924[3];

#endif
