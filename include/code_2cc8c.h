#ifndef CODE_2CC8C_H
#define CODE_2CC8C_H

#include "common.h"
#include "BasicClass.h"
#include "Class6B5CC.h"
#include "BoxFill.h"
#include "IntermediateBase.h"
#include "TaskCore.h"

/* Forward typedefs, used by `extern` declarations further up this file
 * than their own struct bodies (round 14, code_2cc8c_e's own local views,
 * referenced by earlier code_2cc8c_d call-site declarations). */
typedef struct TexPageDesc TexPageDesc;
typedef struct Class6E99CObj Class6E99CObj;

/*
 * TaskCore (class id 0x130, gTaskCoreMethods) is declared once, in
 * include/TaskCore.h (track 4, round 84): code_2cc8c.c, code_2cc8c_b.c and
 * the first function of code_2cc8c_c.c are its methods from +0x058 on. Its
 * object used to be viewed here as `Obj86B60`, after the address of a
 * SUBCLASS table (gClass86B60Methods); the views are gone.
 *
 * What stays below are this unit family's views of the classes TaskCore
 * holds and has no header for yet: TaskCore.h types those fields
 * `BasicClass *` and the accessors here cast to the view (`Unk48Obj`:
 * `sound`, a VabStreamObj; `Unk78Obj`: `bgLayer`; `Unk74Obj`: `subHandle`
 * and TaskCoreTarget's `handle`; `Unk64Elem`: the slot and item widgets,
 * New_Obj6EAC0; `listView` is a BoxFill, include/BoxFill.h). `SlotEntry` and
 * `SrcDesc` are two readings of one TaskCoreTarget::unk24[] record.
 */
typedef struct Unk48Obj Unk48Obj;
typedef struct Unk48ObjMethods Unk48ObjMethods;
typedef struct Unk78Obj Unk78Obj;
typedef struct Unk78ObjMethods Unk78ObjMethods;
typedef struct Unk74Obj Unk74Obj;
typedef struct Unk74ObjMethods Unk74ObjMethods;
typedef struct Unk64Elem Unk64Elem;
typedef struct Unk64ElemMethods Unk64ElemMethods;
typedef struct SrcDesc SrcDesc;
typedef struct HeaderObj HeaderObj;
typedef struct EventArg EventArg;
typedef struct GenericObj GenericObj;
typedef struct GenericObjMethods GenericObjMethods;

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
    s32 unk10;   /* +0x010 */
    s32 unk14;   /* +0x014, combined with unk10 and a per-slot counter into
                    a 2-word stack buffer (`{unk10, unk14 - counter*10}`)
                    passed by address to an Unk64Elem slotBC call, then
                    incremented by 10 per loop iteration -- see
                    TaskCore__CommitElementScroll/TaskCore__RefreshSlotView */
};

/* TaskCore::sound's pointee, a VabStreamObj (New_VabStreamObj): its +0x080 is
 * VabStreamObj__PlayTone. Only TaskCore__PlaySound calls through it, so only
 * that one slot is modelled. */
struct Unk48ObjMethods {
    u8 pad000[0x080];
    void (*slot80)(Unk48Obj *self, s32 a1, s32 a2, s32 a3); /* +0x080 */
};
struct Unk48Obj {
    Unk48ObjMethods *methods; /* +0x000 */
};

/* TaskCore::bgLayer's pointee (a BgLayer; +0x0B8 is BgLayer__SetColor). Only TaskCore__TickColorFade touches it, calling one slot
 * with a literal 1 and the same 3-byte colour buffer TaskCore__TickColorFade builds
 * for its own self->methods->broadcastToSlots call just above. */
struct Unk78ObjMethods {
    u8 pad000[0x0B8];
    void (*slotB8)(Unk78Obj *self, s32 a1, u8 *buf); /* +0x0B8 */
};
struct Unk78Obj {
    Unk78ObjMethods *methods; /* +0x000 */
};

/* TaskCore::subHandle's pointee ("sub-resource handle"). Only TaskCore__SetSubHandle touches
 * it, loaded via `func_8003B39C(path)` (already matched, `class_39e08.c`,
 * where it returns the unit's own local view `SubObjG *` -- this unit keeps
 * its own local view of the same table per the project's established
 * multiple-independent-local-views convention). slot4's return value is
 * discarded at its one call site here, so it is typed `void *` rather than
 * copying `class_39e08.h`'s `SubObjG *` return type -- a discarded return is
 * never evidence of the callee's real return type (see
 * DECOMPILATION_LEARNINGS), and this unit has no use for the more specific
 * type. */
struct Unk74ObjMethods {
    u8 pad000[0x004];
    void *(*slot4)(Unk74Obj *self);  /* +0x004 */
    u8 pad008[0x05C - 0x008];
    void (*slot5C)(Unk74Obj *self);  /* +0x05C */
    u8 pad060[0x078 - 0x060];
    void (*slot78)(Unk74Obj *self);  /* +0x078 */
};
struct Unk74Obj {
    Unk74ObjMethods *methods;        /* +0x000 */
};

extern Unk74Obj *func_8003B39C(const char *path); /* already matched in
                                                       class_39e08.c; local
                                                       view retyped to this
                                                       unit's own Unk74Obj */

extern void *BMemPMgrAlloc(s32 size);   /* allocator, confirmed across many
                                            units */
extern void *BMemPMgrFree(void *ptr);  /* matching free/release. Its own
                                            disassembly (still INCLUDE_ASM,
                                            asm/nonmatchings/code_8220/
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
    s32 unk4;    /* +0x004, becomes self->unk60[idx] */
    u8 pad008[0x018 - 0x008];
    char **unk18; /* +0x018, NULL-terminated array of C strings -- each
                      element is passed to strlen (already typed
                      `s32 strlen(char *s)` in code_171e0.h) and to
                      New_Obj6EAC0 */
};

extern s32 strlen(char *s); /* Psy-Q libc2/strlen, linked from Sony's
                                        own object; local view here */
extern Unk64Elem *New_Obj6EAC0(void *ctx, s32 len, char *name); /* not
                                        yet seen elsewhere; typed from
                                        TaskCore__CreateSlotElements's own call site --
                                        its return value is stored directly
                                        into the same self->unk64[idx]
                                        array TaskCore__BroadcastToSlotElements/TaskCore__BroadcastToSlots/
                                        TaskCore__BeginElementScroll/TaskCore__SetSlotCursor walk as
                                        Unk64Elem * */

/*
 * self->unk64[idx]'s pointee, as walked by TaskCore__BroadcastToSlotElements -- a DIFFERENT
 * reading of the same field TaskCore__ReleaseSlotElements/TaskCore__AdvanceSlotCursor/TaskCore__RetreatSlotCursor use
 * as an opaque resource handle. TaskCore__BroadcastToSlotElements reinterprets that handle as
 * `Unk64Elem **` (an array of `self->unk5C[idx]` object pointers) and
 * dispatches through each element's own +0x0B8 slot. Both readings are
 * kept -- the field itself stays `void **` in `TaskCore` (the generic,
 * more common usage) and this function alone casts locally, per this
 * project's "empty-bodied vtable occupant is not evidence the SLOT takes
 * no arguments" family of narrow-evidence cautions applied to a field
 * instead of a slot.
 */
struct Unk64ElemMethods {
    u8 pad000[0x004];
    void (*slot4)(Unk64Elem *self);            /* +0x004, OBSERVED:
                                                    TaskCore__ReleaseTarget (round 12) */
    u8 pad008[0x04C - 0x008];
    void (*slot4C)(Unk64Elem *self, void *a1, void *buf); /* +0x04C,
                                                    OBSERVED: TaskCore__UpdateSlotElements
                                                    and TaskCore__RefreshSlotView
                                                    (round 12) -- both pass a
                                                    raw buffer pointer as the
                                                    3rd arg (an 8-byte-stride
                                                    external record in
                                                    TaskCore__UpdateSlotElements, the
                                                    address of a 2-word stack
                                                    pair in TaskCore__RefreshSlotView),
                                                    so `buf` stays untyped */
    void (*slot50)(Unk64Elem *self);            /* +0x050, OBSERVED:
                                                    TaskCore__RefreshSlotView (round 12) */
    u8 pad054[0x060 - 0x054];
    void (*slot60)(Unk64Elem *self, s32 a1);   /* +0x060, OBSERVED:
                                                    TaskCore__CancelElementScroll */
    u8 pad064[0x0B8 - 0x064];
    void (*slotB8)(Unk64Elem *self, void *a1); /* +0x0B8 */
    void (*slotBC)(Unk64Elem *self, void *a1); /* +0x0BC, OBSERVED:
                                                    TaskCore__CommitElementScroll (round 12),
                                                    address of a 2-word
                                                    stack pair */
};
struct Unk64Elem {
    Unk64ElemMethods *methods; /* +0x000 */
};

/* listView (+0x068) is a BoxFill (include/BoxFill.h): the accessors cast
 * TaskCore's `BasicClass *` to it. */

/* New_BoxFill: include/BoxFill.h. */
extern s32 D_8008A8E8[2];   /* address-taken only by this unit */
extern char D_8008A8F0[4];  /* address-taken only by this unit */

/* Retyped round 14 once code_2cc8c_e's own body was matched: this is the
   New_X allocator for `Class6E99CObj` (this unit's own view, see the
   Class6E99CObj section far below) -- `BMemPMgrAlloc(0xA0)`
   then `GetClass6E99CMethods()->ctor(self, a1, a2, a3)`. `a1`/`a2`/`a3` forward
   straight through to that ctor unmodified; `a1` is a pointer (confirmed
   by THIS unit's own two real callers, `code_2cc8c_c.c` passing
   `D_8008A90C` and `Entity.c` passing its own `name` parameter, both
   already-matched). The return type differs from the narrower
   `SubHandleObj *`/`Unk100Obj *` views those two callers (and
   `include/Entity.h`) keep for their OWN local field types -- ABI-
   identical (a plain pointer either way), so this is a compatible
   retype: both existing call sites already assign the result into their
   OWN separately-typed local, so this only changes an implicit-conversion
   warning at the assignment, not the compiled bytes. Verified with a full
   rebuild. */
extern Class6E99CObj *New_Class6E99C(void *a1, s32 a2, s32 a3);
/* New_Class6B5CC: include/Class6B5CC.h (it was a local Unk18AcObj view). */
extern u8 D_8008A90C[]; /* address-taken only by this unit, passed as
                            New_Class6E99C's "name" argument */
extern u8 D_8008A904[]; /* address-taken only by this unit: (-100, -100), the
                            screen position Viewport's ctor and SetSubHandle
                            attach the sub handle at (include/Viewport.h) */
extern WholeFrac_d294 D_8008A8F4; /* {0, 1}: Viewport__AttachViewChild's twist
                            when its own is NULL (asm/data/7B048.sdata.s) */

/*
 * This unit family's view of the DrawSystem (D_8006C070, class id 0x1),
 * which has no header yet: Viewport caches it as `drawSystem` (typed
 * BasicClass, include/Viewport.h), and Viewport__Flip reaches its
 * +0x050 DrawSystem__SwapBuffers and +0x054 DrawSystem__GetActiveBuffer
 * (`tools/classtable.py D_8006C070`) through this view. Until round 85 it
 * was also the generic reading of Viewport's children; those accessors now
 * use BasicClass and Class6B5CC.
 */
struct GenericObjMethods {
    s32 header; /* +0x000 */
    u8 pad004[0x050 - 0x004];
    void (*slot50)(GenericObj *self); /* +0x050, DrawSystem__SwapBuffers */
    s32 (*slot54)(GenericObj *self);  /* +0x054, DrawSystem__GetActiveBuffer: Flip's next otIndex */
};
struct GenericObj {
    GenericObjMethods *methods; /* +0x000 */
};

/* GsSetRefView2 is NO LONGER DECLARED HERE, round 33. It is Sony's
   (`libgs/gs_131.o`, linked from the SDK object) and will one day sit next to
   `include/psyq/LIBGS.H`'s own prototype for it -- two declarations of one
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
   with identical codegen, same precedent as Class6B5CC__DispatchLinkCommand/Class6B5CC__TryAttachNearby in
   code_d294.h); the retypes come from the functions' own definitions and are
   ABI-identical (word-sized values either way), so they do not change this
   call site's own compiled bytes.
   ROUND 34: both are still GAME CODE, but neither lives in code_2cc8c_e any
   more -- they are the two one-function units src/code_2cc8c_e0.c and
   src/code_2cc8c_e1.c, wedged between Sony objects. Those files do NOT
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

extern void *New_D8006EF50(void); /* external, no args; local view returns
                                    void* (used as a generic word/child
                                    pointer here); same callee as
                                    class_39e08.h's own `SubObjG *` view */
extern void *New_D8006EFAC(void); /* external, no args; not yet seen
                                    elsewhere in this project */



/*
 * BoxFill (class id 0x64, gBoxFillMethods) is declared once, in
 * include/BoxFill.h (track 4, round 85). Its methods are code_2cc8c_e's
 * New_BoxFill/ctor/Reset and code_2cc8c_f's first thirteen functions; the
 * two views this header used to hold of it (`ClassEAC0Obj` and the 0x64 half
 * of `Obj6EAC0`) are gone.
 */

/*
 * `Obj6EAC0` -- code_2cc8c_f's view of D_8006EB90's class (id 0x11144), the
 * methods from New_Obj6EAC0 to Obj6EAC0__GetDerivedMethods. Round 14 read
 * D_8006EB90 as an override table of D_8006EAC0 and named this view after
 * the latter; the class ids say otherwise, and so do the ctors:
 * Obj6EAC0__Construct chains to GetCharSpriteMethods()->ctor, and CharSprite
 * (0x1144) is a ScreenSprite (0x144, include/ScreenSprite.h) subclass. So
 * 0x11144 is NOT below BoxFill (0x64, gBoxFillMethods, include/BoxFill.h):
 * the thirteen 0x64 methods that used this view now use BoxFill, and the
 * fields only they touched are padding here. Slot comments below that name
 * a BoxFill__* occupant "(base)" date from the old reading; those occupants
 * are BoxFill's, in gBoxFillMethods, not this class's. This view is
 * 0x11144's job (FINISHING-PLAN track 4) and is otherwise left as it was.
 *
 * Its reading (round 54, tier B, this unit's evidence only): an N-child
 * text row. `New_Obj6EAC0(ctx, count, text)` makes `count` CharSprite cells
 * (New_CharSprite) in `children`; Obj6EAC0__SetText hands one byte of a
 * NUL-terminated string to each child's +0x0C4; the layout slots walk
 * `children[childStart .. childStart + childCount)`, advancing a running
 * position by `childPitch`, with one extra +0x10 gap at `gapIndex`. The
 * +0x00C word called `hasChildren` here is at Class6B5CC's `parent` offset.
 *
 * Slot arity is per-CALL-SITE: `slot4C` and `slotC4` are called here with
 * different argument counts, so both stay unprototyped.
 */
typedef struct Obj6EAC0 Obj6EAC0;
typedef struct Obj6EAC0Methods Obj6EAC0Methods;
struct Obj6EAC0Methods {
    u8 pad000[0x008];
    void (*slot08)(Obj6EAC0 *self, s32 a1, s32 a2, s32 a3); /* +0x008,
                                  ctor-shaped: OBSERVED forwarded 3 raw
                                  args by New_Obj6EAC0's New_X wrapper.
                                  IS Obj6EAC0__Construct (this unit, STALL) in
                                  the derived table. */
    void (*slot0C)(Obj6EAC0 *self); /* +0x00C, IS Obj6EAC0__Destruct (derived,
                                  this unit) -- takes no extra args */
    u8 pad010[0x040 - 0x010];
    void (*slot40)(Obj6EAC0 *self, s32 a1); /* +0x040, IS Obj6EAC0__FinishConstruct
                                  (derived, this unit) */
    u8 pad044[0x04C - 0x044];
    void (*slot4C)(); /* +0x04C, DELIBERATELY UNPROTOTYPED (K&R style):
                                  call sites in this unit need it at BOTH
                                  3 and 4 explicit arguments
                                  (BoxFill__AttachAbsolute forwards 4;
                                  Obj6EAC0__LayoutChildrenWithGap calls it at 3, twice, with
                                  different argument MEANINGS each time)
                                  and C requires an exact arg-count match
                                  through a prototyped function-pointer
                                  type, which no single prototype here
                                  could satisfy. IS BoxFill__AttachToParent (base,
                                  this unit, reads 3) and Obj6EAC0__LayoutChildrenWithGap
                                  (derived, this unit, reads 3) */
    void (*slot50)(Obj6EAC0 *self); /* +0x050, IS func_80040C00 (derived,
                                  this unit) */
    u8 pad054[0x060 - 0x054];
    s32 (*slot60)(Obj6EAC0 *self, s32 a1); /* +0x060, IS Obj6EAC0__QueryChildren
                                  (derived, this unit), which recurses
                                  into a child's own slot60 with the same
                                  a1 and threads the return value through
                                  as its own return (last-iteration wins) */
    s32 (*slot64)(Obj6EAC0 *self, s32 a1); /* +0x064, IS BoxFill__SetSemiTrans
                                  (base, this unit) -- tail-returns a
                                  packed-bitfield accessor */
    s32 (*slot68)(Obj6EAC0 *self, s32 a1); /* +0x068, IS BoxFill__SetSemiTransRate
                                  (base, this unit), same shape as
                                  slot64 */
    u8 pad06C[0x0B8 - 0x06C];
    void (*slotB8)(Obj6EAC0 *self, s32 a1); /* +0x0B8, the only OBSERVED
                                  CALL through this slot is
                                  Obj6EAC0__PropagateColor's own child dispatch, at
                                  2 args. BoxFill__SetColor (base occupant)
                                  takes a 3rd (`u8 *src`) in its own
                                  definition, which is fine -- an
                                  occupant's own arity need not match a
                                  narrower call site (nothing in this
                                  unit calls slotB8 at 3 args) */
    void (*slotBC)(Obj6EAC0 *self, Pair32E99C *a1); /* +0x0BC, IS BoxFill__SetPosition
                                  (base, this unit) and Obj6EAC0__LayoutChildren
                                  (derived, this unit); a1 a 2-word
                                  struct pointer in both -- same shape as
                                  Class6E99CObj's own Pair32E99C (see
                                  Class6E99C__PushPosition) */
    void (*slotC0)(Obj6EAC0 *self, void *a1); /* +0x0C0, IS BoxFill__SetSize
                                  (base, this unit); a1 a 2-halfword
                                  struct pointer */
    void (*slotC4)(); /* +0x0C4, DELIBERATELY UNPROTOTYPED, same reason as
                                  slot4C above: BoxFill__AttachAbsolute forwards 4
                                  args, Obj6EAC0__SetText dispatches a CHILD's
                                  slotC4 at only 2. IS BoxFill__AttachAbsolute
                                  (base, this unit, reads 3) and
                                  Obj6EAC0__SetChildChar (derived, this unit,
                                  reads 2) */
    void (*slotC8)(Obj6EAC0 *self, s32 a1); /* +0x0C8, IS BoxFill__SetPri
                                  (base, this unit, setter) and
                                  Obj6EAC0__NoOpSetter (derived, this unit,
                                  splat-generated trivial jr $ra; nop) */
    s32 (*slotCC)(Obj6EAC0 *self, s32 a1); /* +0x0CC, IS BoxFill__SetMask
                                  (base, this unit) and Obj6EAC0__SetText
                                  (derived, this unit) */
    void (*slotD0)(Obj6EAC0 *self); /* +0x0D0, derived-only, IS
                                  Obj6EAC0__NoOpSlotD0 (this unit, splat-
                                  generated trivial) */
    void (*slotD4)(Obj6EAC0 *self, s32 a1); /* +0x0D4, derived-only, IS
                                  Obj6EAC0__SetChildPitch (this unit, setter) */
};

struct Obj6EAC0 {
    Obj6EAC0Methods *methods; /* +0x000 */
    u8 pad004[0x00C - 0x004];
    s32 hasChildren;           /* +0x00C, OBSERVED: tested by
                                  Obj6EAC0__LayoutChildrenWithGap, func_80040C00,
                                  Obj6EAC0__LayoutChildren (Class6B5CC's
                                  `parent` offset; see the banner) */
    u8 pad010[0x0A9 - 0x010];
    u8 totalChildCount;         /* +0x0A9, OBSERVED: Obj6EAC0__Destruct (passed
                                  as ReleaseBasicClassArray's count arg),
                                  Obj6EAC0__LayoutChildren (loop bound) */
    u8 gapIndex;                /* +0x0AA, OBSERVED: Obj6EAC0__LayoutChildrenWithGap -- a
                                  one-shot "extra offset" gate compared
                                  against the loop index */
    u8 childCount;               /* +0x0AB, OBSERVED: a per-slice element
                                  COUNT, paired with childStart as the base
                                  index -- Obj6EAC0__LayoutChildrenWithGap, func_80040C00,
                                  Obj6EAC0__QueryChildren, Obj6EAC0__PropagateColor */
    u8 childStart;               /* +0x0AC, OBSERVED: a per-slice element
                                  START INDEX into children, paired with
                                  childCount above */
    u8 padAD[0x0B0 - 0x0AD];
    s32 childPitch;             /* +0x0B0, OBSERVED: Obj6EAC0__SetChildPitch (setter);
                                  read and added into a local running total
                                  by Obj6EAC0__LayoutChildrenWithGap/Obj6EAC0__LayoutChildren */
    Obj6EAC0 **children;        /* +0x0B4, OBSERVED: an array of child
                                  objects of this SAME class, indexed by
                                  childStart..childStart+childCount and dispatched
                                  through their own `->methods` */
};

/* The packed-bitfield accessor Class6B5CC's attribute setters use
 * (code_d294), reached here by BoxFill's over `&self->boxAttribute`. */
extern u32 GetSetBitField(u32 *word, s32 shift, s32 width, u32 value);

/* gBoxFillMethods and GetBoxFillMethods: include/BoxFill.h. */
extern Obj6EAC0Methods D_8006EB90; /* the override table itself, so
                                       Obj6EAC0__GetDerivedMethods's own definition
                                       (this unit) can return &D_8006EB90 */
/* GetCharSpriteMethods, New_CharSprite and their table: include/CharSprite.h
 * (track 4, round 86), included by code_2cc8c_f, the one unit that calls
 * them. */

/*
 * MEASURED elsewhere (round 9, include/class_3bb8c.h / src/class_3ac78.c):
 * GetClass6B5CCMethods takes NO arguments and its whole body is a fixed
 * `lui/addiu %hi/%lo(gClass6B5CCMethods); jr $ra` -- it always returns the SAME
 * global table regardless of caller, a shared "default handler" utility
 * reached the same way IntermediateBaseMethods/TaskUtilMethods are
 * reached elsewhere in this project. This unit's own call touches only
 * slot 0x04C, at a NARROWER 2-extra-argument arity than
 * Obj6EAC0Methods::slot4C's widest use, so it gets its own tiny local
 * view rather than reusing that struct (same per-call-site-arity
 * reasoning as slot4C/slotC4 above). Do not reconcile this declaration
 * with code_d294.h's or class_3ac78.h's own differently-typed views of
 * the same symbol -- see class_3ac78.c's comment on GetClass6B5CCMethods for
 * why that is expected.
 */
/* GetClass6B5CCMethods and its table: include/Class6B5CC.h (track 4, round 81). */

/*
 * Class6E99C (class id 0x164, D_8006E99C): code_2cc8c_e's own view of
 * BoxFill's one subclass (include/BoxFill.h). Its ctor,
 * Class6E99C__Class6E99C, calls GetBoxFillMethods()->ctor first, then sets
 * its own table and dispatches +0x040 through it; New_Class6E99C allocates
 * 0xA0 bytes against BoxFill's 0x6C. This view is flat and does not expand
 * BOXFILL_FIELDS/BOXFILL_SLOTS yet: it is the subclass's job (FINISHING-PLAN
 * track 4). Field names are offset-based (`unkNN`) until real names are
 * known; +0x050..+0x068 are BoxFill's posX/posY, boxW/boxH, color and mask.
 */
typedef struct Class6E99CMethods Class6E99CMethods;

/* SkipShort2 and Pair32E99C: include/BoxFill.h. */

/* A small "shift/stride/width/height" texture-page-like descriptor --
 * func_8003FC18's own 3rd argument (round 14, code_2cc8c_e). Only the
 * fields that function touches are named. (typedef forward-declared near
 * the top of this file, see there.) */
struct TexPageDesc {
    s32 shift;   /* +0x000 */
    s32 stride;  /* +0x004 */
    s32 width;   /* +0x008 */
    s32 height;  /* +0x00C */
    s32 size;    /* +0x010, computed = (4 << shift) + stride - 4 */
};



struct Class6E99CMethods {
    s32 header;                                     /* +0x000 */
    void *unk04;                                     /* +0x004, BasicClass__Release, inherited, unused here */
    void (*ctor)(Class6E99CObj *self, void *a1, s32 a2, s32 a3); /* +0x008,
                                Class6E99C__Class6E99C (this unit). `a2` is a RAW
                                index/mode (0 or a small positive count),
                                not a pointer -- Class6E99C__Class6E99C's own body
                                converts it into a tableEntry pointer
                                internally before forwarding to the next
                                ctor down the chain (BoxFillMethods::ctor,
                                whose OWN `a2` really is a pointer). */
    void (*dtor)(Class6E99CObj *self);               /* +0x00C, Class6B5CC__Finalize, shared */
    /* +0x010/+0x014/+0x018, IS Class6B5CCMethods's own +0x010/+0x014/+0x018
       (Class6B5CC__AddChild/Class6B5CC__RemoveChild/Class6B5CC__RemoveAllChildren) -- identical addresses in
       both tables per the file banner's classtable.py census. */
    void (*slot10)(Class6E99CObj *self); /* +0x010, OBSERVED: Class6E99C__Configure */
    void (*slot14)(Class6E99CObj *self, s32 a1); /* +0x014, OBSERVED: Class6E99C__Stop */
    u8 pad018[0x030 - 0x018];
    /* +0x030, BasicClass-inherited (per the file banner's census, matches
       D_8006B58C's own +0x030 verbatim) -- OBSERVED: Class6E99C__Stop
       dispatches it as `(self, s32 a1)` with a1 a small literal (5 or 6). */
    void (*slot30)(Class6E99CObj *self, s32 a1);
    u8 pad034[0x040 - 0x034];
    void (*finishConstruct)(Class6E99CObj *self, s32 a1); /* +0x040, Class6E99C__FinishConstruct
                                (this unit). Two args, not four: its own
                                call site (Class6E99C__Class6E99C) only sets `a1`;
                                `a2`/`a3` are leftover from the preceding
                                ctor call and the occupant's own body never
                                reads them. RENAMED round 61 (was slot40). */
    u8 pad044[0x060 - 0x044];
    /* +0x060/+0x064, OBSERVED: Class6E99C__FinishConstruct/Class6E99C__Stop, both dispatched
       as `(self, s32 a1)`. Occupants (code_2cc8c_f, bravo's own functions):
       BoxFill__SetDisplay (+0x060), BoxFill__SetSemiTrans (+0x064). */
    void (*slot60)(Class6E99CObj *self, s32 a1);
    void (*slot64)(Class6E99CObj *self, s32 a1);
    /* +0x068, OBSERVED: Class6E99C__Configure, dispatched as `(self, s32 flag)`
       where `flag` is that same function's own locally-computed 1-or-2
       mode value. */
    void (*slot68)(Class6E99CObj *self, s32 a1);
    u8 pad06C_[0x098 - 0x06C];
    /* +0x098, OBSERVED: Class6E99C__Update's own call target when `a2 == 2`.
       This unit's own function. */
    void (*update)(Class6E99CObj *self, void *a1, s32 a2); /* +0x098, Class6E99C__Update.
                                RENAMED round 61 (was slot98). */
    u8 pad09C[0x0B8 - 0x09C];
    /* +0x0B8/+0x0CC, IS BoxFill's own +0x0B8/+0x0CC
       (BoxFill__SetColor/BoxFill__SetMask, both code_2cc8c_f) -- identical
       addresses in both tables (this class does not override them), same
       fingerprint as the other shared slots above. OBSERVED:
       Class6E99C__StartFadeToIndex/Class6E99C__StartFadeDefault (both this unit). */
    void (*slotB8)(Class6E99CObj *self, s32 a1, void *tableEntry);
    u8 pad0BC[0x0CC - 0x0BC];
    void (*slotCC)(Class6E99CObj *self, s32 a1);
    void (*setStep)(Class6E99CObj *self, s32 a1);     /* +0x0D0, Class6E99C__SetStep.
                                RENAMED round 61 (was slotD0). */
    void (*startFadeToIndex)(Class6E99CObj *self);             /* +0x0D4, Class6E99C__StartFadeToIndex.
                                RENAMED round 61 (was slotD4). */
    void (*startFadeDefault)(Class6E99CObj *self, s32 a1, s32 a2); /* +0x0D8, Class6E99C__StartFadeDefault.
                                RENAMED round 61 (was slotD8). */
    /* +0x0DC, this class's own (BoxFill's table ends at +0x0CC). RENAMED
       round 61 (was slotDC). */
    s32 (*configure)(Class6E99CObj *self);              /* +0x0DC, Class6E99C__Configure */
    /* Round 73 note: the slot is declared `(self)` only, but its occupant
       reads all four argument registers and both StartFade* callers forward
       their own a1..a3 to it; code_2cc8c_e.c calls it through a file-local
       4-argument view (Configure6E99CFn) rather than retyping this shared
       slot. The same holds for startFadeToIndex/startFadeDefault, whose
       definitions take (self, a1, a2, a3). */
    void (*stop)(Class6E99CObj *self, void *a1);   /* +0x0E0, Class6E99C__Stop.
                                RENAMED round 61 (was slotE0). */
    void *(*getColor)(Class6E99CObj *self);            /* +0x0E4, Class6E99C__GetColor.
                                RENAMED round 61 (was slotE4). */
    void (*pushPosition)(Class6E99CObj *self, SkipShort2 *a1, Pair32E99C *a2); /* +0x0E8, Class6E99C__PushPosition.
                                RENAMED round 61 (was slotE8). */
    void (*popPosition)(Class6E99CObj *self);             /* +0x0EC, Class6E99C__PopPosition.
                                RENAMED round 61 (was slotEC). */
    void (*setDivisorMode)(Class6E99CObj *self, s32 a1, s32 a2); /* +0x0F0, Class6E99C__SetDivisorMode.
                                RENAMED round 61 (was slotF0). */
};
struct Class6E99CObj {
    Class6E99CMethods *methods; /* +0x000 */
    u8 pad004[0x00C - 0x004];
    void *unkC;                /* +0x00C, OBSERVED: Class6E99C__PushPosition, truthy-tested only */
    /* +0x010, STALE CITATION FIXED round 61: this comment previously cited
       "func_8003FBF4" as evidence, but that address is `GsDrawOt`
       (`libgs/gs_111.o`, a linked Sony object -- see this file's own
       round-34 banner) and has never been part of this unit; nothing in
       this unit's current 17 functions reads or writes this field at all
       (confirmed: `grep -n 'unk10' src/code_2cc8c_e.c` matches only this
       declaration). The citation predates the round-34 segment split, when
       the address now known as `GsDrawOt` still lived in this file under a
       different, since-reclassified reading. Left untyped and unrenamed --
       there is no live evidence left for it in this unit -- but kept as a
       `void *` (not folded into surrounding padding) since offsets past it
       are load-bearing for +0x050 onward. Same base offset as
       Class6B5CC's own inherited `unk10` (include/Class6B5CC.h `attribute`, a `u32` packed
       bit-flags word) -- plausibly the same underlying field reused
       opaquely here, but kept independent per this project's
       multiple-local-views convention; that parallel is the only reason to
       keep the slot typed at all. */
    void *unk10;
    u8 pad014[0x050 - 0x014];
    s32 unk50;                 /* +0x050, OBSERVED: Class6E99C__PopPosition/Class6E99C__PushPosition */
    s32 unk54;                 /* +0x054, OBSERVED: Class6E99C__PopPosition/Class6E99C__PushPosition */
    u8 pad058[0x060 - 0x058];
    /* +0x060/+0x062, OBSERVED: Class6E99C__PopPosition/Class6E99C__PushPosition -- `u16`, not
       `s16`: Class6E99C__PushPosition widens these into the `s32` unk88/unk8C fields
       via a plain assignment, and retail's `lhu` there (zero-extending)
       only matches when the source type is unsigned. */
    u16 unk60;
    u16 unk62;
    /* +0x064/+0x065/+0x066, OBSERVED: Class6E99C__Update -- three independent
       byte counters, each incremented by the low byte of `unk74` when the
       corresponding bit of `unk78` (0x4/0x2/0x1) is set. */
    u8 unk64;
    u8 unk65;
    u8 unk66;
    u8 pad067[0x068 - 0x067];
    s32 unk68;                 /* +0x068, OBSERVED: Class6E99C__Configure, a divisor */
    /* +0x06C, OBSERVED: Class6E99C__Stop/Class6E99C__StartFadeToIndex/Class6E99C__StartFadeDefault/
       Class6E99C__FinishConstruct (zeroed by the ctor override) -- a small dispatch-state
       tag: 0 == idle, 1 == fading to an indexed color (StartFadeToIndex),
       2 == fading to the default color (StartFadeDefault). RENAMED round
       61 (was unk6C); not a BoxFill field (BoxFill ends at +0x06C). */
    s32 state;
    s32 unk70;                 /* +0x070, OBSERVED: Class6E99C__FinishConstruct, set from
                                   its own `a1` parameter */
    /* +0x074, OBSERVED: Class6E99C__FinishConstruct (ctor override sets it to 0xA),
       Class6E99C__StartFadeToIndex (negated on the "already had one" path), Class6E99C__SetStep
       (a plain setter, `self->step = a1`); also read a BYTE at a time by
       Class6E99C__Update via its low byte -- the per-tick amount added into
       unk64/unk65/unk66 while a fade is running. RENAMED round 61 (was
       unk74); not a BoxFill field. */
    s32 step;
    s32 unk78;                 /* +0x078, OBSERVED: Class6E99C__Configure/Class6E99C__Update/
                                   Class6E99C__GetColor, a flags/mode word tested
                                   against 0xF and against bit masks
                                   0x1/0x2/0x4 */
    /* +0x07C, OBSERVED: Class6E99C__FinishConstruct (ctor override zeroes it),
       Class6E99C__Update (tested `== 9`), Class6E99C__Configure (set from its own a3
       parameter). */
    s32 unk7C;
    s32 unk80;                 /* +0x080, OBSERVED: Class6E99C__Update, a countdown */
    s32 unk84;                 /* +0x084, OBSERVED: Class6E99C__Configure, a division result */
    /* +0x088/+0x08C, OBSERVED: Class6E99C__PopPosition (read via `lhu`, into `s16`
       unk60/unk62 -- a narrowing read of only the low halfword) and
       Class6E99C__PushPosition (WRITTEN via a plain WORD `sw`, from `lhu`-loaded
       unk60/unk62 -- a genuine `s32` field, widened on write). Retail's
       own `sw` at this offset is why these are `s32`, not `s16` -- an
       earlier reading typed them `s16` from Class6E99C__PopPosition's read alone and
       inserted a 2-byte pad to keep unk90 at the right offset; the pad was
       the wrong fix for the wrong field width. */
    s32 unk88;
    s32 unk8C;
    s32 unk90;                 /* +0x090, OBSERVED: Class6E99C__PopPosition/Class6E99C__PushPosition */
    s32 unk94;                 /* +0x094, OBSERVED: Class6E99C__PopPosition/Class6E99C__PushPosition */
    s32 altMode;               /* +0x098, OBSERVED: Class6E99C__SetDivisorMode, setter arg1;
                                   RENAMED round 61 (was unk98). */
    s32 divisor;               /* +0x09C, OBSERVED: Class6E99C__SetDivisorMode, setter arg2;
                                   Class6E99C__Configure also reads it as a divisor
                                   (gated on altMode != 0). RENAMED round 61
                                   (was unk9C). */
    u8 padA0[0xA0 - 0xA0];
};

/* Round-14 static tables this unit's own functions index into or pass by
 * address -- real element shape not derived (nothing this unit's chosen
 * functions dereference beyond taking the address), so left as opaque
 * byte blobs sized only by their known stride. `Class6E99C__Class6E99C`/
 * `Class6E99C__StartFadeToIndex`/`Class6E99C__StartFadeDefault`/`Class6E99C__GetColor` all compute the index as
 * a raw BYTE offset (`sll v0,i,1; addu v0,v0,i` = `i*3`, added directly to
 * the base address with no further `*4`) -- i.e. `D_8006EA90` holds 3-BYTE
 * entries (plausibly a signed-byte triple, same shape as this file's own
 * `SByte3_d294`), not 0xC-byte ones. `D_8006EAA8` is indexed the SAME way
 * by `Class6E99C__StartFadeDefault` (not a single fixed entry as an earlier reading of
 * `Class6E99C__Class6E99C` alone suggested -- that one just always passes index 0),
 * so left unsized rather than fixed at 3 bytes. `D_8008A924` has only the
 * one (unindexed) use, so kept at a single entry's size. */
extern u8 D_8006EA90[];
extern u8 D_8006EAA8[];
extern u8 D_8008A924[3];

extern Class6E99CMethods D_8006E99C;
extern Class6E99CMethods *GetClass6E99CMethods(void); /* returns &D_8006E99C */

#endif
