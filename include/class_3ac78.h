#ifndef CLASS_3AC78_H
#define CLASS_3AC78_H

#include "common.h"

typedef struct Class866E8 Class866E8;
typedef struct Class866E8Methods Class866E8Methods;
typedef struct Class86668 Class86668;
typedef struct Class86668Methods Class86668Methods;
typedef struct UnkListObj_3ac78 UnkListObj_3ac78;
typedef struct UnkChildMethods_3ac78 UnkChildMethods_3ac78;
typedef struct UnkChildObj_3ac78 UnkChildObj_3ac78;
typedef struct UnkSlotChildMethods_3ac78 UnkSlotChildMethods_3ac78;
typedef struct UnkSlotChildObj_3ac78 UnkSlotChildObj_3ac78;
typedef struct UnkSlotListObj_3ac78 UnkSlotListObj_3ac78;
typedef struct UnkSlotEntry_3ac78 UnkSlotEntry_3ac78;
typedef struct GridRect_3ac78 GridRect_3ac78;
typedef struct GridRectList_3ac78 GridRectList_3ac78;
typedef struct UnkPtr68Obj_3ac78 UnkPtr68Obj_3ac78;
typedef struct UnkArgObj_3ac78 UnkArgObj_3ac78;
typedef struct GenericMethodsHeader GenericMethodsHeader;
typedef struct GenericObject GenericObject;

/*
 * One entry of Class866E8::elems[7] (Class866E8__ResetAllElements). 0x1C bytes; only the
 * three fields that function touches are typed. Its own two pointer
 * members' full types (UnkSlotChildObj_3ac78, UnkSlotListObj_3ac78) are
 * defined further down -- fine, since a pointer only needs the forward
 * `typedef struct X X;` above, not the full body. This struct itself has
 * to come before Class866E8 below because Class866E8::elems is an ARRAY
 * member, which (unlike a pointer) needs a COMPLETE type.
 */
struct UnkSlotEntry_3ac78 {
    u16 flag;                        /* zeroed at the top of each loop pass (Class866E8__ResetAllElements) */
    u16 key;                         /* Class866E8__Class866E8 (ctor): set to the loop index (0..6), and copied on into target->key. class_3bb8c's independent view (Elem::unk2) has func_8004B700 copy a caller-supplied key byte into the same field. */
    UnkSlotChildObj_3ac78 *target;
    UnkSlotListObj_3ac78 *list;
    GenericObject *cellParent;             /* Class866E8__Finalize: refreshed (discarded) through ->methods->release when non-NULL */
    /* RETYPED from `GenericObject **` (Class866E8__Finalize's earlier,
     * still-unconfirmed, INCLUDE_ASM-only comment) to `Class866E8 **`: a 2D
     * grid of pointers, row stride 20 cells (0x50 bytes), each cell holding
     * ANOTHER Class866E8 instance -- Class866E8__DispatchToRectCells forwards a cell's value
     * straight into NotifyGridCell, which dereferences `self->flags36`
     * (a genuine Class866E8 field, +0x036), and walks a `->unk38` chain off
     * the SAME pointer (also added to Class866E8 this round). Class866E8__Finalize
     * (round 19, MATCHED, 113/113) confirms the field is also walked as a
     * flat 0x668-byte array of 4-byte cells there -- its teardown loop reads
     * each cell generically through `GenericObject`'s shared `methods->release`
     * base-class slot, legitimate for a `Class866E8 *` element since
     * `methods` sits at the same +0x000 offset either way; this does not
     * contradict the 2D-grid-of-Class866E8 reading above. */
    /* TYPE CAVEAT (round 67, naming pass, NOT fixed here): the element type
     * below is this unit's own weaker reading. class_3bb8c's independently
     * derived view calls the same block `EntryChildObj **`, and its evidence
     * is stronger -- the ctor here ORs 0x80000000 into each freshly-built
     * cell's +0x010, which is EntryChildObj::unk10 exactly (func_8004C0AC
     * does the same OR, func_8004CE24 clears the same bit), and the
     * `flags36`/`nextInCell` fields NotifyGridCell walks line up with
     * EntryChildObj::unk36/unk38. Unifying the two views is a track-4 job,
     * not a naming one, so the declared type is left alone; read it as
     * "a cell object", not as "another Class866E8". */
    Class866E8 **cells;              /* Class866E8__DispatchToRectCells: 2D grid, row stride 20 cells. Class866E8__Class866E8 (ctor): allocates 0x668 raw bytes (func_80017B34) and fills it with pointers built in an inner loop. Class866E8__Finalize (dtor, MATCHED): walks the same 0x668-byte span tearing down each non-NULL cell, then frees it. */
    GenericObject *heldObj;          /* Class866E8__Class866E8 (ctor): zeroed. Class866E8__OnElementEvent releases it through the shared BasicClass `release` slot on event 6 and stores the result back, the project's standard release-and-clear shape. RETYPED from `s32` this round: the only code that touches it dereferences it as an object with a vtable at +0x000. */
    s32 unk18;                       /* Class866E8__Class866E8 (ctor): zeroed */
};

/*
 * One entry of Class866E8::rects.e[4] (Class866E8__ApplyToSenderFootprint/Class866E8__SetFootprintRect/
 * Class866E8__DispatchToRectCells). 0xC bytes, densely packed -- REVISED this round
 * (Class866E8__DispatchToRectCells) from an earlier 0x10-byte/3-element guess. Two
 * independent pieces of evidence now agree:
 *  - Class866E8__SetFootprintRect (matched round 71) writes `self->rects.e[0].elemIdx = self->methods->
 *    slot124(...)` into element [0]'s first field; `slot124`'s real
 *    occupant (func_8004C5D0) was independently retyped, round 8, to
 *    return an s32 LOOP INDEX, never a pointer (see Class866E8Methods::
 *    slot124's own comment) -- so this field is an index, not a pointer.
 *  - Class866E8__DispatchToRectCells reads that same first field back and uses it as
 *    exactly that: an index into `self->elems[]` (`&self->elems[elemIdx]`,
 *    reproduced by retail as a multiply-by-0x1C, matching
 *    `UnkSlotEntry_3ac78`'s own 0x1C-byte size). The four following
 *    fields it reads as a dense, PADDING-FREE run of four `s16`s
 *    immediately after the `s32` (offsets +4/+6/+8/+0xA, no gap before
 *    the next element at +0xC) -- a grid rectangle descriptor: starting
 *    column, starting row, width, height. This is the SAME field shape
 *    (index + col + row + width + height) as `GridSlot866E8` in the
 *    SIBLING unit's `include/class_3bb8c.h` (a different unit, different
 *    concrete `self` type -- independent view, not the same C type) --
 *    recognizing the shape is what caught the stale trailing-padding
 *    guess here.
 * Renamed accordingly (`unk0`->`elemIdx`, `unk4`->`col`, `unk6`->`row`,
 * `unk8`->`width`, `unkA`->`height`); `Class866E8__SetFootprintRect.md`'s STALL report
 * keeps its OLD field names in its preserved best-attempt body (per this
 * project's "preserved code, not doctrine" policy) with a note pointing
 * here.
 */
struct GridRect_3ac78 {
    s32 elemIdx;
    s16 col;
    s16 row;
    s16 width;
    s16 height;
};

/*
 * Class866E8::rects as a whole, 0x30 bytes -- CONFIRMED by Class866E8__ApplyToSenderFootprint's
 * byte-exact whole-struct copy (a batched 4-word-per-iteration block
 * move whose total width does not depend on how the elements inside are
 * sliced). Now modeled as 4 x 0xC-byte GridRect_3ac78 (4*0xC == 0x30
 * exactly, zero padding needed) rather than the earlier 3 x 0x10 guess
 * (which needed 4 bytes of unexplained trailing padding per element) --
 * see GridRect_3ac78's own comment for the evidence. Total size is
 * unchanged, so this revision does not disturb Class866E8__ApplyToSenderFootprint's match
 * (reverified: whole-image SHA1 stays green with Class866E8__ApplyToSenderFootprint still
 * compiled as real C).
 *
 * Wrapped in a struct (rather than left as a bare array field) so it can
 * be copied with a plain `=` -- GCC 2.6.3 compiles a whole-struct
 * assignment to a batched 4-word-per-iteration block move, which is
 * exactly what Class866E8__ApplyToSenderFootprint's save/restore pair does. An indexed loop
 * translation does NOT reproduce this codegen; see
 * docs/match-reports/func_80025E1C.md for the earlier-established
 * precedent this follows.
 */
struct GridRectList_3ac78 {
    GridRect_3ac78 e[4];
};

/*
 * Opaque target of Class866E8::config (Class866E8__ApplyToSenderFootprint dereferences it and
 * reads only the field at +0x4). Class866E8__SetConfig (matched) only ever
 * STORES a raw word there and never dereferences it itself, so retyping
 * its own `arg1` parameter to this pointer type changes nothing about
 * its compiled bytes -- confirmed after the retype, still byte-exact.
 */
struct UnkPtr68Obj_3ac78 {
    u8 pad0[0x4];
    s32 unk4;
};

/*
 * self->origin's pointee (Class866E8__Class866E8, the ctor) -- a 3-word block copied
 * via a WHOLE-STRUCT assignment (`self->origin = *arg1;`, retail: 3 loads
 * then 3 stores, batched) from either the ctor's own `arg1` when non-NULL,
 * or the default global `gDefaultOrigin` otherwise. Same "whole-struct
 * assignment compiles to a batched load/store block" idiom as
 * `GridRectList_3ac78` above.
 */
typedef struct Vec3_3ac78 Vec3_3ac78;
struct Vec3_3ac78 {
    s32 unk0;
    s32 unk4;
    s32 unk8;
};

/*
 * Class866E8 -- constructed by New_Class866E8 (New_Class866E8: allocates
 * 0x1E8 bytes, gets the vtable via func_8004D244, calls ctor slot +0x008).
 * Vtable is D_800866E8 (80 slots, header 0x114), resolved with
 * tools/classtable.py D_800866E8. Ctor is Class866E8__Class866E8 (177 words,
 * MATCHED) and the finalize slot (+0x00C) is Class866E8__Finalize (113 words, MATCHED round 19 --
 * see its match report; mirrors the ctor's own per-slot teardown, one
 * `elems[]` entry at a time).
 *
 * No FirecatFG name survives for this class (only anonymous func_ symbols
 * in the symbol file), so it is named by its vtable address, same
 * convention as Class6D3C8.h. Only the slots and fields this round's
 * functions actually reach are typed; the rest stays opaque padding.
 *
 * ROUND 67 (naming pass): the class is a GRID MANAGER, and the grid's
 * geometry is now arithmetic rather than inference. gDefaultGridSpan is
 * 0xA000; Class866E8__SetGridSpan stores it at +0x74 and derives
 * gridCells = span >> 11 = 20 and gridHalfCells = span >> 12 = 10. The
 * ctor spaces cells 0x800 apart (0xA000 / 0x800 == 20), and 20 is also the
 * row stride class_3bb8c_b's BYTE-MATCHED func_8004CE24 uses over the same
 * cell block. Three independent facts, one number.
 *
 * One thing that does NOT reconcile, recorded rather than resolved: the
 * ctor's placement loop wraps X after 21 columns, not 20 (it resets when
 * x > 0xA400, and x runs 0x400 + k * 0x800), and 0x668 bytes is 410 cell
 * pointers, which is neither 20 * 20 nor a whole number of 21-cell rows.
 * The ctor is byte-exact, so the constants are certainly right; what the
 * extra column and the 10 spare pointers are for is unknown. Do not "fix"
 * the stride to 21 on the strength of the ctor alone -- func_8004CE24's
 * 20 is the byte-verified one.
 */
struct Class866E8Methods {
    /* +0x000 */ s32 header;
    /* +0x004 */ void *release;                                                   /* BasicClass__func_17eb0 */
    /* +0x008 */ void (*ctor)(Class866E8 *self, s32 arg1, s32 arg2);            /* Class866E8__Class866E8; called by New_Class866E8 */
    /* +0x00C */ void *finalize;                                                    /* Class866E8__Finalize (MATCHED); not dispatched by any of this project's decompiled callers yet */
    /* +0x010 */ void (*addChild)(Class866E8 *self, s32 arg1);                    /* Class866E8__Class866E8 (ctor); called with func_80020C5C()'s return, after the 7-entry elems[] init loop */
    /* +0x014 */ void (*removeChild)(Class866E8 *self, void *arg1);                  /* func_8001CCB4; called by Class866E8__Finalize */
    /* +0x018 */ u8 pad018[0x030 - 0x018];
    /* +0x030 */ void (*notifyParents)(Class866E8 *self, s32 command);                    /* BasicClass__NotifyParents; called by Class866E8__OnElementEvent */
    /* +0x034 */ u8 pad034[0x038 - 0x034];
    /* +0x038 */ void (*onNotify)(Class866E8 *self);                              /* Class866E8__OnNotify; called by NotifyGridCell */
    /* +0x03C */ u8 pad03C[0x040 - 0x03C];
    /* +0x040 */ void (*reset)(Class866E8 *self);                              /* Class866E8__Reset; called by Class866E8__SetConfig */
    /* +0x044 */ u8 pad044[0x080 - 0x044];
    /* +0x080 */ void (*setFlag8)(Class866E8 *self, s32 arg1, s32 arg2, s32 arg3); /* Class6B5CC__GetSetUnk10Flag8; called by Class86668__SetChildFlag8 through Class86668::unk34 */
    /* +0x084 */ u8 pad084[0x088 - 0x084];
    /* +0x088 */ void (*onElementEvent)(Class866E8 *self, s32 command, void *elem, s32 index); /* Class866E8__OnElementEvent; called by Class866E8__ResetAllElements with a 4th arg AA6C's own body never reads */
    /* +0x08C */ u8 pad08C[0x0B8 - 0x08C];
    /* +0x0B8 */ UnkChildObj_3ac78 *(*getChild)(Class866E8 *self, s32 index);      /* func_80042828; called by Class866E8__SetChildParams */
    /* +0x0BC */ u8 pad0BC[0x0D0 - 0x0BC];
    /* +0x0D0 */ void (*forwardAcceptedCommand)(Class866E8 *self, void *sender, s32 command);        /* Class866E8__ForwardAcceptedCommand; called by Class866E8__OnCommand */
    /* +0x0D4 */ u8 pad0D4[0x0DC - 0x0D4];
    /* +0x0DC */ void (*setGridSpan)(Class866E8 *self, s32 span);                    /* Class866E8__Reset; called with self and the loaded value of gDefaultGridSpan (a lone .word, 0x0000A000, no other reference in the image) */
    /* +0x0E0 */ u8 pad0E0[0x0F4 - 0x0E0];
    /* +0x0F4 */ void (*slotF4)(Class866E8 *self);                              /* func_8004B5BC; called by Class866E8__UpdateIfEnabled */
    /* +0x0F8 */ u8 pad0F8[0x100 - 0x0F8];
    /* +0x100 */ void (*slot100)(Class866E8 *self, void *arg1, s32 arg2);        /* func_8004BD14; called by Class866E8__OnNotify */
    /* +0x104 */ u8 pad104[0x108 - 0x104];
    /* +0x108 */ void (*slot108)(Class866E8 *self, UnkSlotEntry_3ac78 *entry);   /* func_8004C0AC; called by Class866E8__ResetAllElements */
    /* +0x10C */ u8 pad10C[0x110 - 0x10C];
    /* +0x110 */ s32 (*slot110)(Class866E8 *self, UnkArgObj_3ac78 *arg1, s32 arg2); /* func_8004C1C0; called by Class866E8__ApplyToSenderFootprint */
    /* +0x114 */ u8 pad114[0x124 - 0x114];
    /* +0x124 */ s32 (*slot124)(Class866E8 *self, s32 key);                     /* func_8004C5D0; called by Class866E8__SetFootprintRect.
                  * RETYPED round 8 from `void *(*)(Class866E8 *, void *)` on the strength of the
                  * occupant's own BYTE-EXACT body, matched that round in class_3bb8c_b as
                  * `s32 func_8004C5D0(Obj866E8 *self, s32 key)`: it returns a loop INDEX
                  * (`move v0,a2`) or -1, never a pointer, and `key` is compared against a
                  * s16 field so it is a scalar. Safe to change because slot124 has no C call
                  * site yet, and the whole-image SHA1 was re-verified after the change.
                  * Confirmed round 71: Class866E8__SetFootprintRect matched byte-exact calling it
                  * with this signature. */
    /* +0x128 */ u8 pad128[0x12C - 0x128];
    /* +0x12C */ void (*applyToSenderFootprint)(Class866E8 *self, void *sender, s32 command);       /* Class866E8__ApplyToSenderFootprint; called by Class866E8__ForwardAcceptedCommand */
    /* +0x130 */ u8 pad130[0x13C - 0x130];
    /* +0x13C */ void (*slot13C)(Class866E8 *self);                             /* func_8004D028; called by Class866E8__UpdateIfEnabled */
    /* +0x140 */ void (*slot140)(Class866E8 *self);                             /* func_8004D088; called by Class866E8__ResetAllElements */
};

/* Object size is 0x1E8, from New_Class866E8's allocator call. Field offsets
 * below are only the ones this round's functions touch. */
struct Class866E8 {
    /* +0x000 */ Class866E8Methods *methods;
    /* +0x004 */ u8 pad004[0x036 - 0x004];
    /* +0x036 */ u16 flags36;                /* bit 0x80 tested by NotifyGridCell */
    /* +0x038 */ Class866E8 *nextInCell;          /* Class866E8__DispatchToRectCells: singly-linked chain of OTHER Class866E8 instances sharing one grid cell, walked while non-NULL -- same "chained instances in one slot" idiom as the sibling unit's EntryChildObj::unk38 (include/class_3bb8c.h) */
    /* +0x03C */ u8 pad03C[0x054 - 0x03C];
    /* +0x054 */ Vec3_3ac78 origin;              /* Class866E8__Class866E8 (ctor): copied field-by-field from its own arg1 (when non-NULL) or the default global gDefaultOrigin */
    /* +0x060 */ s32 valueFn;                  /* Class866E8__SetCallback arg1 */
    /* +0x064 */ s32 valueFnCtx;                  /* Class866E8__SetCallback arg2 */
    /* +0x068 */ UnkPtr68Obj_3ac78 *config;   /* Class866E8__SetConfig arg1, stored raw; Class866E8__ApplyToSenderFootprint dereferences ->unk4 */
    /* +0x06C */ s32 unk6C;                  /* Class866E8__Class866E8 (ctor): zeroed */
    /* +0x070 */ s32 enabled;                  /* Class866E8__UpdateIfEnabled: gates slotF4/slot13C dispatch */
    /* +0x074 */ s32 gridSpan;                  /* Class866E8__SetGridSpan arg1, stored raw */
    /* +0x078 */ s16 gridHalfCells;                  /* Class866E8__SetGridSpan: arg1 >> 12 */
    /* +0x07A */ s16 gridCells;                  /* Class866E8__SetGridSpan: arg1 >> 11 */
    /* +0x07C */ s16 footprintCol;                  /* Class866E8__SetFootprintFromCell: (s8)arg1->unk2 - 1 */
    /* +0x07E */ s16 footprintRow;                  /* Class866E8__SetFootprintFromCell: (s8)arg1->unk3 - 1 */
    /* +0x080 */ s32 footprintW;                  /* Class866E8__SetFootprintFromCell's `span`, stored raw */
    /* +0x084 */ s32 footprintH;                  /* Class866E8__SetFootprintFromCell's `span`, stored raw (same value as footprintW) */
    /* +0x088 */ s32 rectCount;                  /* Class866E8__SetFootprintRect (matched round 71): always set to 1; Class866E8__ApplyToSenderFootprint saves/restores it around a pair of calls */
    /* +0x08C */ GridRectList_3ac78 rects;   /* Class866E8__ApplyToSenderFootprint: saved/restored via whole-struct assignment; Class866E8__SetFootprintRect (matched round 71) writes element [0]'s fields directly -- see GridRect_3ac78 */
    /* +0x0BC */ u16 cellTag;                  /* Class866E8__DispatchToRectCells: copied verbatim into curCellTag on every grid cell visited (a "current pass" tag, plausibly) */
    /* +0x0BE */ u8 pad0BE_tail[0x0E8 - 0x0BE];
    /* +0x0E8 */ s32 acceptedTags;                  /* Class866E8__SetAcceptedTags arg1; Class866E8__ForwardAcceptedCommand reads it back as a NUL-terminated s32 tag array -- true element type still s32, only usage differs per call site */
    /* +0x0EC */ UnkSlotEntry_3ac78 elems[7]; /* Class866E8__ResetAllElements: 7 x 0x1C-byte slots, walked 0..6 */
    /* +0x1B0 */ s32 unk1B0;                 /* Class866E8__Class866E8 (ctor): zeroed */
    /* +0x1B4 */ s16 unk1B4;                 /* Class866E8__ResetAllElements: zeroed after the loop */
    /* +0x1B6 */ u8 pad1B6[0x1B8 - 0x1B6];
    /* +0x1B8 */ s32 unk1B8;                 /* Class866E8__ResetAllElements: zeroed after the loop */
    /* +0x1BC */ UnkSlotEntry_3ac78 *lastEventElem;   /* Class866E8__OnElementEvent arg2, stored raw */
    /* +0x1C0 */ u16 curCellTag;                 /* Class866E8__DispatchToRectCells: set to self->cellTag on every grid cell visited */
    /* +0x1C2 */ u8 curCellCol;                  /* Class866E8__DispatchToRectCells: current grid column (entry->col + loop offset), truncated to a byte */
    /* +0x1C3 */ u8 curCellRow;                  /* Class866E8__DispatchToRectCells: current grid row (entry->row + loop offset), truncated to a byte */
    /* +0x1C4 */ u8 pad1C4[0x1CC - 0x1C4];   /* Class866E8__GetCurrentCellKey still only takes the address of the block as a whole; real extent of further fields unknown */
    /* +0x1CC */ s32 unk1CC;                 /* Class866E8__Reset: set to -1 */
    /* +0x1D0 */ s32 unk1D0;                 /* Class866E8__Reset: set to -1 */
    /* +0x1D4 */ s32 unk1D4;                 /* Class866E8__Reset: set to -1 */
    /* +0x1D8 */ s32 unk1D8;                 /* Class866E8__Reset: set to -1 */
    /* +0x1DC */ u8 pad1DC[0x1E0 - 0x1DC];
    /* +0x1E0 */ s32 unk1E0;                 /* Class866E8__Class866E8 (ctor): zeroed */
    /* +0x1E4 */ u8 pad1E4[0x1E8 - 0x1E4];
};

/* Get-vtable helper for Class866E8. Still raw asm: it lives in class_3bb8c,
 * an uncarved monolithic segment, not yet a carved src/ unit anywhere. */
extern Class866E8Methods *func_8004D244(void);

/* The generic allocator, established already in DreamSys.h/Entity.h/etc. */
extern void *func_80017B34(s32 size);

/*
 * Class86668 -- vtable gClass86668Methods (28 slots, header 0x230), resolved with
 * tools/classtable.py gClass86668Methods. Its own last slot (+0x070) is
 * Class86668__SetChildFlag8, which is the only reason this class is visible from this
 * unit at all: Class86668__SetChildFlag8 dispatches through a Class866E8 instance held
 * at Class86668::unk34. Everything else about Class86668 is unknown.
 */
struct Class86668Methods {
    /* +0x000 */ s32 header;
    /* +0x004 */ u8 pad004[0x070 - 0x004];
};

struct Class86668 {
    /* +0x000 */ Class86668Methods *methods;
    /* +0x004 */ u8 pad004[0x034 - 0x004];
    /* +0x034 */ Class866E8 *unk34;          /* Class86668__SetChildFlag8 dispatches through this, guarded by a NULL check */
};

extern Class86668Methods gClass86668Methods;

/*
 * Opaque descriptor buffer passed as Class866E8__SetFootprintFromCell's arg1, Class866E8__ApplyToSenderFootprint's
 * stack-local `buf` (passed on to slot110/Class866E8__SetFootprintFromCell/Class866E8__SetFootprintRect),
 * and Class866E8__SetFootprintRect's desc. Populated by a call through
 * Class866E8Methods slot +0x110 (func_8004C1C0, not decompiled anywhere
 * yet), so only the bytes those functions actually read are typed. Named
 * after the convention in code_171e0.h (`Unk*Obj_<unit>`).
 */
struct UnkArgObj_3ac78 {
    u8 pad0[2];
    s8 unk2;
    s8 unk3;
    u8 pad4[0x28 - 0x4];
    s32 unk28;     /* Class866E8__SetFootprintRect (matched round 71): reread and forwarded to self->methods->slot124
                    * as its `key` argument -- scalar, not a pointer; see slot124 above. */
};

/*
 * Generic "object with a vtable pointer at offset 0" view, used only by
 * Class866E8__OnCommand to read the low byte of another object's vtable header
 * word as a type tag (0x34 here). Several unrelated class tables share
 * that low byte (D_800878D4 and DREAMSYS_METHODS among them, per
 * docs/research/class-framework.md), so this reads as a family/base-class
 * check, not an exact-class check. True type of the pointed-to object is
 * unconfirmed; only the one byte this function reads is modeled here.
 *
 * release is the shared "BasicClass" base-method slot at +0x004 -- the same
 * slot Class866E8Methods documents as `BasicClass__func_17eb0`. Its exact
 * semantics are unknown; Class866E8__OnElementEvent calls it on a GenericObject and
 * stores the (pointer) return value back where the object came from,
 * suggesting a release-and-replace pattern, but that is not confirmed.
 */
struct GenericMethodsHeader {
    s32 header;
    void *(*release)(void *self);
    u8 pad008[0x04C - 0x008];
    /* +0x04C, Class866E8__Class866E8 (ctor). Same offset as every other
     * "generic base object" vtable in this project (FieldM7CMethods,
     * ChildMethods86ED0, Class86ED0Handle -- all sibling units, all
     * independent local views); called at two different arities here
     * across two call sites in the SAME function, so kept generic
     * (`void *`) rather than picking one call site's shape. */
    void (*slot4C)(void *self, void *arg1, void *arg2);
    u8 pad050[0x070 - 0x050];
    void (*slot70)(void *self, s32 arg1); /* +0x070, Class866E8__Class866E8 (ctor) */
};

struct GenericObject {
    GenericMethodsHeader *methods;
    u8 pad004[0x010 - 0x004];
    u32 unk10; /* +0x010, Class866E8__Class866E8 (ctor): OR'd with 0x80000000 (a flag bit) */
};

/*
 * Opaque list/args object passed as Class866E8__OnElementEvent's arg2 (stashed at
 * self->lastEventElem) and as Class866E8__ApplyToSenderFootprint/Class866E8__ForwardAcceptedCommand's "list" argument.
 * Class866E8__ApplyToSenderFootprint additionally reaches +0x0C (a flag: nonzero selects
 * `unk14->unk38`, computed as raw pointer arithmetic -- `unk14` points to
 * something bigger than a GenericObject, and +0x38 is a field within it
 * this unit never otherwise names) as the gate for that computation.
 */
struct UnkListObj_3ac78 {
    u8 pad0[0xC];
    s32 unk0C;               /* Class866E8__ApplyToSenderFootprint: nonzero selects the unk14->unk38 branch */
    u8 pad10[0x14 - 0x10];
    GenericObject *unk14;    /* Class866E8__OnElementEvent: replaced via ->methods->release(unk14) when non-NULL. Class866E8__ApplyToSenderFootprint: (u8*)unk14 + 0x38 computed (not dereferenced) when unk0C != 0 */
};

/*
 * Opaque child object returned by Class866E8Methods::getChild. Class unknown;
 * only the two adjacent method slots Class866E8__SetChildParams dispatches through are
 * typed.
 */
struct UnkChildMethods_3ac78 {
    u8 pad0[0x44];
    void (*slot44)(UnkChildObj_3ac78 *self, s32 arg1, s32 arg2);
    void (*slot48)(UnkChildObj_3ac78 *self, s32 arg1, s32 arg2);
};

struct UnkChildObj_3ac78 {
    UnkChildMethods_3ac78 *methods;
};

/*
 * Opaque "child" object referenced by each Class866E8::elems[] slot entry
 * (Class866E8__ResetAllElements). Class unknown; only the two dispatched slots are typed.
 * A DIFFERENT class from UnkChildObj_3ac78 above (different offsets), kept
 * as its own type rather than reusing that one.
 */
struct UnkSlotChildMethods_3ac78 {
    u8 pad0[0x4];
    void *(*release)(UnkSlotChildObj_3ac78 *self);  /* Class866E8__Finalize: same shared BasicClass base slot as GenericMethodsHeader::release */
    u8 pad8[0x74 - 0x8];
    void (*slot74)(UnkSlotChildObj_3ac78 *self);
    u8 pad78[0x84 - 0x78];
    void (*slot84)(UnkSlotChildObj_3ac78 *self);
    void (*slot88)(UnkSlotChildObj_3ac78 *self, s32 arg1); /* +0x088, Class866E8__Class866E8 (ctor): called with self->elems[i].target dispatched right after the object is freshly returned by func_80048894, arg1 = Class866E8__Class866E8's own arg2 forwarded */
};

struct UnkSlotChildObj_3ac78 {
    UnkSlotChildMethods_3ac78 *methods;
    u8 pad4[0x10 - 0x4];
    s32 unk10;    /* Class866E8__Class866E8 (ctor): tested nonzero (sltu), the boolean result stored into unk20 */
    u8 pad14[0x20 - 0x14];
    u16 unk20;    /* Class866E8__Class866E8 (ctor): set to (unk10 != 0) */
    u8 pad22[0x2C - 0x22];
    s16 unk2C;    /* Class866E8__DispatchToRectCells: gates the whole per-history-entry grid walk (nonzero test) */
    u8 pad2E[0x32 - 0x2E];
    u16 key;      /* Class866E8__Class866E8 (ctor): set to the outer loop index (0..6) */
};

/*
 * Opaque "list" object referenced by each Class866E8::elems[] slot entry
 * (Class866E8__ResetAllElements). Only field +0x2C (a GenericObject*, refreshed through
 * its own ->methods->release base-class slot exactly like
 * Class866E8__OnElementEvent's arg2->unk14) is typed.
 */
struct UnkSlotListObj_3ac78 {
    GenericMethodsHeader *methods;   /* Class866E8__Finalize: refreshed via ->methods->release(self), stored back into the owning entry's `list` */
    u8 pad4[0x2C - 0x4];
    GenericObject *unk2C;
};

#endif
