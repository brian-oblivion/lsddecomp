/*
 * code_8220_b -- the tail of the BasicClass block, then the game's own
 * per-face polygon renderer. Two unrelated halves, split at func_80018464.
 *
 * 0x80018288..0x80018458 finishes BasicClass, the hand-rolled base class
 * whose framework and 14-slot method table live in code_8220.c and
 * include/code_8220.h: the two list primitives BasicClass's own methods
 * call (FreeBasicClassList, GetNextBasicClass), the array release helper,
 * the vtable accessor, the two notification slots (BasicClass__NotifyParents
 * at +0x030 and BasicClass__OnNotify at +0x038) and the empty +0x034 hook,
 * plus the accessor pair for the pool allocator's re-entrancy flag.
 *
 * 0x80018464..0x8001974C is the renderer. func_80018464 (the unit's one
 * stall, still INCLUDE_ASM) walks a model's face groups, dispatches on each
 * group's tag to one of 13 cases, and per face calls SetupPrimCode, then
 * ProjectTriFace or ProjectQuadFace, then one of Sony's RCpoly* packers via
 * the wrappers in code_8220_c. ProjectTri/QuadFace index the shared vertex
 * array, hand the vertices to the GTE, call TransformAndCullPoly for the
 * transform-and-cull decision, write each vertex's Z into its sort slot, and
 * call back into one of the six StoreSxyPoly** leaves to write the screen XY
 * into the Psy-Q POLY_xx primitive at that primitive type's own offsets.
 * Every GTE access goes through the gte_* macros in include/gte.h; those are
 * Sony's names and are never renamed.
 */

#include "common.h"
#include "code_8220.h"
#include "gte.h"

void FreeBasicClassList(BasicClassListNode **head)
{
    BasicClassListNode *node = *head;

    while (node != NULL) {
        BasicClassListNode *cur = node;
        node = node->next;
        BMemPMgrFree(cur);
    }
}

/* BasicClassMethods slot +0x030. Tell every object holding a reference to
 * `self` that `event` happened, by calling each one's own +0x038 slot with
 * `self` as the sender. BasicClass__Finalize (finalize, code_8220.c) is
 * the only caller in carved C and passes 1. */
void BasicClass__NotifyParents(BasicClass *self, s32 event)
{
    BasicClassListNode *cursor = self->parentRefs;
    BasicClass *parent;

    for (GetNextBasicClass(&parent, &cursor); parent != NULL; GetNextBasicClass(&parent, &cursor)) {
        parent->methods->slot38(parent, self, event);
    }
}

/* BasicClassMethods slot +0x034, and deliberately still a placeholder name.
 * The body is empty, and of the 60 method tables tools/classtable.py finds,
 * 58 have this exact address in slot +0x034; the two that differ
 * (D_8006C0F8, whose header word is a pointer and which is probably a
 * mis-detected table start, and gStyleCueCallbacks, a wholly independent 14-slot
 * class that overrides every slot) are not BasicClass-derived. Nothing in
 * the game overrides it, so nothing establishes what it is for. */
void BasicClass__func_18350(void) {
}

/* BasicClassMethods slot +0x038, the receiving half of NotifyParents:
 * `sender` is telling `self` that `event` happened. The base class treats
 * event 1 as "sender is going away" and drops it from its own children.
 * Subclasses override this and forward to the base first -- Class6B5CC__OnNotify
 * (code_d294) then dispatches on the sender's class tag, func_80065790
 * (code_55dd4) then checks the sender's tag and the same event == 1 -- so
 * `event` is a general notification code, not a boolean. */
void BasicClass__OnNotify(BasicClass *self, void *sender, s32 event)
{
    if (event == 1) {
        self->methods->removeChild(self, (BasicClass *)sender);
    }
}

BasicClassMethods *Get_vtable_BasicClass(void)
{
    return &D_8006B58C;
}

void GetNextBasicClass(BasicClass **outValue, BasicClassListNode **cursor)
{
    if (*cursor != NULL) {
        *outValue = (*cursor)->value;
        *cursor = (*cursor)->next;
    } else {
        *outValue = NULL;
    }
}

void ReleaseBasicClassArray(BasicClass **array, s32 count)
{
    if (count-- > 0) {
        do {
            *array = (BasicClass *)(*array)->methods->release(*array);
            array++;
        } while (count-- > 0);
    }
}

void SetBMemPMgrBusy(s32 val)
{
    gBMemPMgrBusy = val;
}

s32 GetBMemPMgrBusy(void)
{
    return gBMemPMgrBusy;
}

#ifdef NON_MATCHING
/* NON_MATCHING: 982/954 words, +28 long, length no longer exact. Residue:
 * GCC 2.6.3 strength-reduces `elem` into a second induction variable in
 * every one of the thirteen per-element loops (an inline-asm "r" operand
 * on the loop pointer defeats biv elimination), costing 2 words per loop
 * plus 2 more once the extra register pushes the frame past seven
 * callee-saved registers (docs/match-reports/func_80018464.md, round 56
 * revisit). Hand-derived -- built from the disassembly and Sony's
 * include/psyq/INLINE.H operand shapes, not permuter-searched. */

/* Globals this renderer publishes for SetupPrimCode and the code_8220_c
 * submit wrappers to read back. D_8008E248 is already in code_8220.h; the
 * other four are this unit's own view and stay local per CLAUDE.md's
 * cross-unit-declaration rule. */
extern s32 D_80090C18;
extern s32 D_8008E250;
extern s32 D_8008E24C;
extern s32 D_800902E0;
extern s8 D_8008A82C[3];
extern void *D_8008E794;

extern void func_8001A224(void *dst, void *table, s32 count);
extern void func_80019774(void *dst, s32 storeFirst3);
extern void func_8001979C(void *dst, s32 storeFirst3);

/*
 * The eight submit wrappers in code_8220_c, which are still INCLUDE_ASM there
 * and declared `void`. That is provably a placeholder: every one of them is a
 * tail call whose last instruction before its epilogue is `jal RCpolyXX` with
 * no intervening store to $v0, so Sony's return value falls straight out --
 * the standard `p = RCpolyF3(p);` work-buffer-advance idiom. This function
 * consumes exactly that value, so this file declares its own view rather than
 * importing the stale one (round 50's finding; see the report).
 */
extern void *func_800197C4(void *prim, void *ctx);
extern void *func_8001989C(void *prim, void *ctx);
extern void *func_800199EC(void *prim, void *ctx);
extern void *func_80019B24(void *prim, void *ctx);
extern void *func_80019C04(void *prim, void *ctx);
extern void *func_80019D84(void *prim, void *ctx);
extern void *func_80019EE4(void *prim, void *ctx);
extern void *func_8001A064(void *prim, void *ctx);

/* Defined below, in ROM order. Forward-declared because this function comes
 * first in the segment and calls all of them. */
void SetupPrimCode(void *prim, void *ctx);
s32 ProjectTriFace(void *prim, u8 *ctx, u16 idx0, u16 idx1, u16 idx2, void (*storeSxy)(void *));
s32 ProjectQuadFace(void *prim, u8 *ctx, u16 idx0, u16 idx1, u16 idx2, u16 idx3, void (*storeSxy)(void *, s32));
void StoreSxyPolyF3(void *dst);
void StoreSxyPolyG3(void *dst);
void StoreSxyPolyFT3(void *dst);
void StoreSxyPolyGT3(void *dst);
void StoreSxyPolyF4(void *dst, s32 storeFirst3);
void StoreSxyPolyG4(void *dst, s32 storeFirst3);

/*
 * Walk one model's face groups and emit a GPU primitive per surviving face.
 *
 * `obj` is the drawable, `arg1` supplies the ordering table, `arg2` its shift,
 * and `ctx` is the per-object scratch block the caller places in the PS1
 * scratchpad. Each group header is a 2-byte element count plus a 2-byte
 * primitive tag (and one bit of the same word, the semi-transparency flag);
 * the tag selects one of thirteen case bodies, each with its own element
 * stride, index offsets, GTE colour op and submit wrapper. `prim` is the
 * packet-buffer write cursor, reloaded from D_8008E794 at the top of every
 * group and advanced by each submit wrapper's return value.
 *
 * Every GTE access goes through include/gte.h. The RGB store macros are the
 * one-pointer-at-offset-0 Psy-Q forms, which is why `gte_strgb(prim + 0x4)`
 * and not `swc2 $22, 0x4(prim)`: the addiu that materialises the sum is part
 * of retail.
 */
void func_80018464(void *objIn, void *otSrc, s32 otShift, void *ctxIn)
{
    u8 *obj = (u8 *)objIn;
    u8 *ctx = (u8 *)ctxIn;
    u8 *prim;
    u8 *list;
    s32 remaining;
    s32 dpShift;

    if (*(s32 *)obj < 0) {
        return;
    }

    *(void **)(ctx + 0x0) = *(void **)((u8 *)otSrc + 0x4);
    *(s32 *)(ctx + 0x4) = otShift;
    func_8001A224(ctx + 0x88, D_8008ACD0, 3);
    func_8001A224(ctx + 0x94, D_8008AEE8, 4);

    remaining = *(s32 *)(*(u8 **)(obj + 0x8) + 0x14);
    list = *(u8 **)(*(u8 **)(obj + 0x8) + 0x10);
    *(void **)(ctx + 0xC) = *(void **)(*(u8 **)(obj + 0x8) + 0x0);
    *(void **)(ctx + 0x10) = *(void **)(*(u8 **)(obj + 0x8) + 0x8);

    /* When the object carries a local light/world matrix, save the GTE's
     * current rotation matrix into the context, install the object's own,
     * run each of the three columns of the object's 3x3 through it, and put
     * the saved matrix back. */
    if (*(s32 *)(*(u8 **)(obj + 0x4) + 0x48) != 0) {
        u8 *lws = *(u8 **)(obj + 0x4);

        gte_ReadRotMatrix(ctx + 0x38);
        gte_SetRotMatrix(*(u8 **)(lws + 0x48) + 0x24);

        gte_ldclmv(lws + 0x24);
        gte_llir();
        gte_stclmv(lws + 0x24);

        gte_ldclmv(*(u8 **)(obj + 0x4) + 0x26);
        gte_llir();
        gte_stclmv(*(u8 **)(obj + 0x4) + 0x26);

        gte_ldclmv(*(u8 **)(obj + 0x4) + 0x28);
        gte_llir();
        gte_stclmv(*(u8 **)(obj + 0x4) + 0x28);

        gte_SetRotMatrix(ctx + 0x38);
    }

    /* Four independent reads of the flags word, not one cached copy: each
     * global store below kills the previous load for CSE, and retail shows
     * all four `lw`s. */
    *(s32 *)(ctx + 0x8) = 0xA;
    D_80090C18 = (*(u32 *)obj >> 9) & 0x7;
    D_8008E248 = (*(u32 *)obj >> 6) & 0x1;
    D_8008E250 = (*(u32 *)obj >> 5) & 0x1;
    D_8008E24C = (*(u32 *)obj >> 3) & 0x3;

    {
        s8 *tint = D_8008A82C;

        *(s8 *)(ctx + 0x34) = tint[0];
        *(s8 *)(ctx + 0x35) = tint[1];
        *(s8 *)(ctx + 0x36) = tint[2];
    }

    if (D_8008E250 != 0 && D_800902E0 != 0) {
        dpShift = 9;
    } else if (D_8008E24C != 0) {
        dpShift = 9;
    } else {
        dpShift = 0x10;
    }
    *(s32 *)(ctx + 0x2C) = dpShift;

    if (remaining == 0) {
        return;
    }

    {
        void (*cbG3)(void *) = StoreSxyPolyG3;

        do {
            s32 count;

            prim = (u8 *)D_8008E794;
            *(s32 *)(ctx + 0x18) = *(u16 *)(list + 0x2) & 0xFD07;
            count = *(u16 *)(list + 0x0);
            *(s32 *)(ctx + 0x1C) = (*(u32 *)(list + 0x0) >> 25) & 0x1;
            remaining -= count;


            /* Thirteen Psy-Q primitive flavours, one case each, in ascending
             * tag order. GCC 2.6.3 expands a switch over sparse values as a
             * balanced binary search -- `== median`, then `< median + 1` and
             * recurse -- which is where retail's sltiu/beq ladder comes from;
             * the case bodies then follow in source order, which is why they
             * sit at ascending addresses in ascending tag order. */
            switch (*(s32 *)(ctx + 0x18)) {
            case 0x2000: {
                u8 *elem = list + 0x4;

                /* A: POLY_F3, opaque */
                prim[3] = 4;
                prim[7] = 0x20;
                SetupPrimCode(prim, ctx);
                do {
                    if (ProjectTriFace(prim, ctx, *(u16 *)(elem + 0x6), *(u16 *)(elem + 0x8),
                                       *(u16 *)(elem + 0xA), StoreSxyPolyF3) == 0) {
                        gte_ldv0(*(u8 **)(ctx + 0x10) + (s32)*(u16 *)(elem + 0x4) * 8);
                        gte_ldrgb(elem);
                        gte_ncds();
                        gte_strgb(prim + 0x4);
                        prim[7] = ctx[0x15];
                        prim = (u8 *)func_800197C4(prim, ctx);
                    }
                    list += 0x10;
                    elem += 0x10;
                } while (--count != 0);
                break;
            }

            case 0x2004: {
                u8 *elem = list + 0xC;

                /* B: POLY_G3, opaque -- one ncds per vertex colour */
                prim[3] = 6;
                prim[7] = 0x30;
                SetupPrimCode(prim, ctx);
                do {
                    if (ProjectTriFace(prim, ctx, *(u16 *)(elem + 0x6), *(u16 *)(elem + 0x8),
                                       *(u16 *)(elem + 0xA), cbG3) == 0) {
                        gte_ldv0(*(u8 **)(ctx + 0x10) + (s32)*(u16 *)(elem + 0x4) * 8);
                        gte_ldrgb(list + 0x4);
                        gte_ncds();
                        gte_strgb(prim + 0x4);
                        prim[7] = ctx[0x15];
                        gte_ldrgb(list + 0x8);
                        gte_ncds();
                        gte_strgb(prim + 0xC);
                        gte_ldrgb(elem);
                        gte_ncds();
                        gte_strgb(prim + 0x14);
                        prim = (u8 *)func_8001989C(prim, ctx);
                    }
                    list += 0x18;
                    elem += 0x18;
                } while (--count != 0);
                break;
            }

            case 0x2101: {
                u8 *elem = list + 0x4;

                /* C: POLY_F3, depth-cued */
                prim[3] = 4;
                prim[7] = 0x20;
                SetupPrimCode(prim, ctx);
                do {
                    if (ProjectTriFace(prim, ctx, *(u16 *)(elem + 0x4), *(u16 *)(elem + 0x6),
                                       *(u16 *)(elem + 0x8), StoreSxyPolyF3) == 0) {
                        gte_ldrgb(elem);
                        gte_dpcs();
                        gte_strgb(prim + 0x4);
                        prim[7] = ctx[0x15];
                        prim = (u8 *)func_800197C4(prim, ctx);
                    }
                    list += 0x10;
                    elem += 0x10;
                } while (--count != 0);
                break;
            }

            case 0x2400: {
                u8 *elem = list + 0x10;

                /* D: POLY_FT3, opaque -- one constant colour for the whole
                 * group, loaded once before the loop. */
                prim[3] = 7;
                prim[7] = 0x24;
                SetupPrimCode(prim, ctx);
                gte_ldrgb(ctx + 0x34);
                do {
                    if (ProjectTriFace(prim, ctx, *(u16 *)(elem + 0x2), *(u16 *)(elem + 0x4),
                                       *(u16 *)(elem + 0x6), StoreSxyPolyFT3) == 0) {
                        *(u32 *)(prim + 0xC) = *(u32 *)(elem - 0xC);
                        *(u32 *)(prim + 0x14) = *(u32 *)(elem - 0x8);
                        *(u32 *)(prim + 0x1C) = *(u32 *)(elem - 0x4);
                        *(u16 *)(prim + 0xE) = *(u16 *)(prim + 0xE) +
                            (u16)((*(s32 *)(ctx + 0x24) >> *(s32 *)(ctx + 0x2C)) << 6);
                        gte_ldv0(*(u8 **)(ctx + 0x10) + (s32)*(u16 *)(elem + 0x0) * 8);
                        gte_ncds();
                        gte_strgb(prim + 0x4);
                        prim[7] = ctx[0x15];
                        prim = (u8 *)func_800199EC(prim, ctx);
                    }
                    list += 0x18;
                    elem += 0x18;
                } while (--count != 0);
                break;
            }

            case 0x2501: {
                u8 *elem = list + 0x10;

                /* E: POLY_FT3, depth-cued */
                prim[3] = 7;
                prim[7] = 0x24;
                SetupPrimCode(prim, ctx);
                do {
                    if (ProjectTriFace(prim, ctx, *(u16 *)(elem + 0x4), *(u16 *)(elem + 0x6),
                                       *(u16 *)(elem + 0x8), StoreSxyPolyFT3) == 0) {
                        *(u32 *)(prim + 0xC) = *(u32 *)(elem - 0xC);
                        *(u32 *)(prim + 0x14) = *(u32 *)(elem - 0x8);
                        *(u32 *)(prim + 0x1C) = *(u32 *)(elem - 0x4);
                        *(u16 *)(prim + 0xE) = *(u16 *)(prim + 0xE) +
                            (u16)((*(s32 *)(ctx + 0x24) >> *(s32 *)(ctx + 0x2C)) << 6);
                        gte_ldrgb(elem);
                        gte_dpcs();
                        gte_strgb(prim + 0x4);
                        prim[7] = ctx[0x15];
                        prim = (u8 *)func_800199EC(prim, ctx);
                    }
                    list += 0x1C;
                    elem += 0x1C;
                } while (--count != 0);
                break;
            }

            case 0x2800: {
                u8 *elem = list + 0x4;

                /* F: POLY_F4, opaque */
                prim[3] = 5;
                prim[7] = 0x28;
                SetupPrimCode(prim, ctx);
                do {
                    if (ProjectQuadFace(prim, ctx, *(u16 *)(elem + 0x6), *(u16 *)(elem + 0x8),
                                        *(u16 *)(elem + 0xA), *(u16 *)(elem + 0xC),
                                        StoreSxyPolyF4) == 0) {
                        gte_ldv0(*(u8 **)(ctx + 0x10) + (s32)*(u16 *)(elem + 0x4) * 8);
                        gte_ldrgb(elem);
                        gte_ncds();
                        gte_strgb(prim + 0x4);
                        prim[7] = ctx[0x15];
                        prim = (u8 *)func_80019B24(prim, ctx);
                    }
                    list += 0x14;
                    elem += 0x14;
                } while (--count != 0);
                break;
            }

            case 0x2901: {
                u8 *elem = list + 0x4;

                /* G: POLY_F4, depth-cued */
                prim[3] = 5;
                prim[7] = 0x28;
                SetupPrimCode(prim, ctx);
                do {
                    if (ProjectQuadFace(prim, ctx, *(u16 *)(elem + 0x4), *(u16 *)(elem + 0x6),
                                        *(u16 *)(elem + 0x8), *(u16 *)(elem + 0xA),
                                        StoreSxyPolyF4) == 0) {
                        gte_ldrgb(elem);
                        gte_dpcs();
                        gte_strgb(prim + 0x4);
                        prim[7] = ctx[0x15];
                        prim = (u8 *)func_80019B24(prim, ctx);
                    }
                    list += 0x10;
                    elem += 0x10;
                } while (--count != 0);
                break;
            }

            case 0x2C00: {
                u8 *elem = list + 0x14;

                /* H: POLY_FT4, opaque */
                prim[3] = 9;
                prim[7] = 0x2C;
                SetupPrimCode(prim, ctx);
                gte_ldrgb(ctx + 0x34);
                do {
                    if (ProjectQuadFace(prim, ctx, *(u16 *)(elem + 0x2), *(u16 *)(elem + 0x4),
                                        *(u16 *)(elem + 0x6), *(u16 *)(elem + 0x8),
                                        func_80019774) == 0) {
                        *(u32 *)(prim + 0xC) = *(u32 *)(elem - 0x10);
                        *(u32 *)(prim + 0x14) = *(u32 *)(elem - 0xC);
                        *(u32 *)(prim + 0x1C) = *(u32 *)(elem - 0x8);
                        *(u32 *)(prim + 0x24) = *(u32 *)(elem - 0x4);
                        *(u16 *)(prim + 0xE) = *(u16 *)(prim + 0xE) +
                            (u16)((*(s32 *)(ctx + 0x24) >> *(s32 *)(ctx + 0x2C)) << 6);
                        gte_ldv0(*(u8 **)(ctx + 0x10) + (s32)*(u16 *)(elem + 0x0) * 8);
                        gte_ncds();
                        gte_strgb(prim + 0x4);
                        prim[7] = ctx[0x15];
                        prim = (u8 *)func_80019D84(prim, ctx);
                    }
                    list += 0x20;
                    elem += 0x20;
                } while (--count != 0);
                break;
            }

            case 0x2D01: {
                u8 *elem = list + 0x14;

                /* I: POLY_FT4, depth-cued */
                prim[3] = 9;
                prim[7] = 0x2C;
                SetupPrimCode(prim, ctx);
                do {
                    if (ProjectQuadFace(prim, ctx, *(u16 *)(elem + 0x4), *(u16 *)(elem + 0x6),
                                        *(u16 *)(elem + 0x8), *(u16 *)(elem + 0xA),
                                        func_80019774) == 0) {
                        *(u32 *)(prim + 0xC) = *(u32 *)(elem - 0x10);
                        *(u32 *)(prim + 0x14) = *(u32 *)(elem - 0xC);
                        *(u32 *)(prim + 0x1C) = *(u32 *)(elem - 0x8);
                        *(u32 *)(prim + 0x24) = *(u32 *)(elem - 0x4);
                        *(u16 *)(prim + 0xE) = *(u16 *)(prim + 0xE) +
                            (u16)((*(s32 *)(ctx + 0x24) >> *(s32 *)(ctx + 0x2C)) << 6);
                        gte_ldrgb(elem);
                        gte_dpcs();
                        gte_strgb(prim + 0x4);
                        prim[7] = ctx[0x15];
                        prim = (u8 *)func_80019D84(prim, ctx);
                    }
                    list += 0x20;
                    elem += 0x20;
                } while (--count != 0);
                break;
            }

            case 0x3101: {
                u8 *elem = list + 0x4;

                /* J: POLY_G3, depth-cued -- three colours, one dpct */
                prim[3] = 6;
                prim[7] = 0x30;
                SetupPrimCode(prim, ctx);
                do {
                    if (ProjectTriFace(prim, ctx, *(u16 *)(elem + 0xC), *(u16 *)(elem + 0xE),
                                       *(u16 *)(elem + 0x10), cbG3) == 0) {
                        gte_ldrgb3c(elem);
                        gte_dpct();
                        gte_strgb3_g3(prim);
                        prim[7] = ctx[0x15];
                        prim = (u8 *)func_8001989C(prim, ctx);
                    }
                    list += 0x18;
                    elem += 0x18;
                } while (--count != 0);
                break;
            }

            case 0x3501: {
                u8 *elem = list + 0x18;

                /* K: POLY_GT3, depth-cued */
                prim[3] = 9;
                prim[7] = 0x34;
                SetupPrimCode(prim, ctx);
                do {
                    if (ProjectTriFace(prim, ctx, *(u16 *)(elem + 0x4), *(u16 *)(elem + 0x6),
                                       *(u16 *)(elem + 0x8), StoreSxyPolyGT3) == 0) {
                        *(u32 *)(prim + 0xC) = *(u32 *)(elem - 0x14);
                        *(u32 *)(prim + 0x18) = *(u32 *)(elem - 0x10);
                        *(u32 *)(prim + 0x24) = *(u32 *)(elem - 0xC);
                        *(u16 *)(prim + 0xE) = *(u16 *)(prim + 0xE) +
                            (u16)((*(s32 *)(ctx + 0x24) >> *(s32 *)(ctx + 0x2C)) << 6);
                        gte_ldrgb3(list + 0x10, list + 0x14, elem);
                        gte_dpct();
                        gte_strgb3(prim + 0x4, prim + 0x10, prim + 0x1C);
                        prim[7] = ctx[0x15];
                        prim = (u8 *)func_80019EE4(prim, ctx);
                    }
                    list += 0x24;
                    elem += 0x24;
                } while (--count != 0);
                break;
            }

            case 0x3901: {
                u8 *elem = list + 0x10;

                /* L: POLY_G4, depth-cued -- three colours by dpct, the
                 * fourth by a second dpcs. */
                prim[3] = 8;
                prim[7] = 0x38;
                SetupPrimCode(prim, ctx);
                do {
                    if (ProjectQuadFace(prim, ctx, *(u16 *)(elem + 0x4), *(u16 *)(elem + 0x6),
                                        *(u16 *)(elem + 0x8), *(u16 *)(elem + 0xA),
                                        StoreSxyPolyG4) == 0) {
                        gte_ldrgb3c(list + 0x4);
                        gte_dpct();
                        gte_strgb3_g3(prim);
                        prim[7] = ctx[0x15];
                        gte_ldrgb(elem);
                        gte_dpcs();
                        gte_strgb(prim + 0x1C);
                        prim = (u8 *)func_80019C04(prim, ctx);
                    }
                    list += 0x1C;
                    elem += 0x1C;
                } while (--count != 0);
                break;
            }

            case 0x3D01: {
                u8 *elem = list + 0x20;

                /* M: POLY_GT4, depth-cued -- the widest element, 0x2C bytes,
                 * all four UVs and all four colours. */
                prim[3] = 0xC;
                prim[7] = 0x3C;
                SetupPrimCode(prim, ctx);
                do {
                    if (ProjectQuadFace(prim, ctx, *(u16 *)(elem + 0x4), *(u16 *)(elem + 0x6),
                                        *(u16 *)(elem + 0x8), *(u16 *)(elem + 0xA),
                                        func_8001979C) == 0) {
                        *(u32 *)(prim + 0xC) = *(u32 *)(elem - 0x1C);
                        *(u32 *)(prim + 0x18) = *(u32 *)(elem - 0x18);
                        *(u32 *)(prim + 0x24) = *(u32 *)(elem - 0x14);
                        *(u32 *)(prim + 0x30) = *(u32 *)(elem - 0x10);
                        *(u16 *)(prim + 0xE) = *(u16 *)(prim + 0xE) +
                            (u16)((*(s32 *)(ctx + 0x24) >> *(s32 *)(ctx + 0x2C)) << 6);
                        gte_ldrgb3(list + 0x14, list + 0x18, list + 0x1C);
                        gte_dpct();
                        gte_strgb3(prim + 0x4, prim + 0x10, prim + 0x1C);
                        prim[7] = ctx[0x15];
                        gte_ldrgb(elem);
                        gte_dpcs();
                        gte_strgb(prim + 0x28);
                        prim = (u8 *)func_8001A064(prim, ctx);
                    }
                    list += 0x2C;
                    elem += 0x2C;
                } while (--count != 0);
                break;
            }

            default:
                return;
            }

            D_8008E794 = prim;
        } while (remaining != 0);
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/code_8220_b", func_80018464);
#endif

/*
 * Finish the GPU command byte of the POLY_xx primitive `prim`, then cache it
 * in the draw context. `prim[3]` is the P_TAG length byte and `prim[7]` is
 * the GPU command byte; func_80018464 writes the (len, code) pair for the
 * primitive type immediately before every one of its 13 calls to this, and
 * all eight pairs it uses are Sony's exactly -- (4, 0x20) POLY_F3, (6, 0x30)
 * POLY_G3, (7, 0x24) POLY_FT3, (5, 0x28) POLY_F4, (9, 0x2C) POLY_FT4,
 * (9, 0x34) POLY_GT3, (8, 0x38) POLY_G4, (12, 0x3C) POLY_GT4.
 *
 * Bit 0x2 of the command byte is the GPU's ABE (semi-transparency) bit and
 * is taken from the context's own flag at +0x1C; bit 0x1 is the shade-texture
 * bit (Psy-Q SetShadeTex) and is taken from the global D_8008E248, which
 * func_80018464 sets from bit 6 of the object's flags word. The two updates
 * are deliberately independent statements -- see this function's match
 * report, factoring them through one local costs the match.
 *
 * The length byte and the finished command byte are then cached at ctx+0x14
 * and ctx+0x15; TransformAndCullPoly re-stamps the length byte onto every
 * primitive it processes from ctx+0x14.
 */
void SetupPrimCode(void *prim, void *ctx)
{
    u8 *a = (u8 *)prim;

    if (*(s32 *)((u8 *)ctx + 0x1C) != 0) {
        a[7] = a[7] | 0x2;
    } else {
        a[7] = a[7] & 0xFD;
    }

    if (D_8008E248 != 0) {
        a[7] = a[7] | 0x1;
    } else {
        a[7] = a[7] & 0xFE;
    }

    *((u8 *)ctx + 0x14) = a[3];
    *((u8 *)ctx + 0x15) = a[7];
}

/*
 * Project one three-vertex face and say whether it survived.
 *
 * `idx0`..`idx2` index the 8-byte vertex array the draw context holds at
 * ctx+0xC; the three resulting pointers are parked in the context's vertex
 * slots at +0xA4/+0xA8/+0xAC and loaded into the GTE. TransformAndCullPoly
 * does the transform and the cull decision. On success each vertex's screen
 * Z goes into the sort slot at +0x14 of the three per-vertex records the
 * context lists at +0x88/+0x8C/+0x90, `storeSxy` writes the screen XY into
 * `prim` at that primitive type's own offsets (one of the StoreSxyPoly**
 * leaves at the bottom of this file), and func_8001A268 computes the screen
 * bounding box over the 3 vertices. Returns 0 drawn, 1 culled.
 *
 * `prim` is only ever handed straight through, so it stays void * here.
 */
s32 ProjectTriFace(void *prim, u8 *ctx, u16 idx0, u16 idx1, u16 idx2, void (*storeSxy)(void *))
{
    *(void **)(ctx + 0xa4) = (u8 *)(*(void **)(ctx + 0xc)) + (s32)idx0 * 8;
    *(void **)(ctx + 0xa8) = (u8 *)(*(void **)(ctx + 0xc)) + (s32)idx1 * 8;
    *(void **)(ctx + 0xac) = (u8 *)(*(void **)(ctx + 0xc)) + (s32)idx2 * 8;

    gte_ldv3(*(void **)(ctx + 0xa4), *(void **)(ctx + 0xa8),
             *(void **)(ctx + 0xac));

    if (TransformAndCullPoly(prim, ctx) != 0) {
        goto fail;
    }

    {
        u8 *p0 = *(u8 **)(ctx + 0x88) + 0x14;
        u8 *p1 = *(u8 **)(ctx + 0x8c) + 0x14;
        u8 *p2 = *(u8 **)(ctx + 0x90) + 0x14;

        gte_stsz3(p0, p1, p2);
    }
    storeSxy(prim);
    func_8001A268(ctx, 3);
    return 0;
fail:
    return 1;
}

/*
 * The four-vertex sibling of ProjectTriFace. The first three vertices go
 * through the same shared transform-and-cull; the fourth is transformed on
 * its own with a single rtps afterwards, which is why `storeSxy` is called
 * twice -- once with 1 to store the first three screen XYs, once with 0 to
 * store the fourth. Sort Zs go to four per-vertex records (+0x94..+0xA0),
 * the fourth vertex's screen XY is also cached at ctx+0x6C, and the bounding
 * box is computed over 4 vertices. Returns 0 drawn, 1 culled.
 */
s32 ProjectQuadFace(void *prim, u8 *ctx, u16 idx0, u16 idx1, u16 idx2, u16 idx3, void (*storeSxy)(void *, s32))
{
    u8 *vtxSlot = ctx + 0xa4;

    *(void **)(ctx + 0xa4) = (u8 *)(*(void **)(ctx + 0xc)) + (s32)idx0 * 8;
    *(void **)(ctx + 0xa8) = (u8 *)(*(void **)(ctx + 0xc)) + (s32)idx1 * 8;
    *(void **)(ctx + 0xac) = (u8 *)(*(void **)(ctx + 0xc)) + (s32)idx2 * 8;
    *(void **)(ctx + 0xb0) = (u8 *)(*(void **)(ctx + 0xc)) + (s32)idx3 * 8;

    gte_ldv3(*(void **)(vtxSlot + 0x0), *(void **)(vtxSlot + 0x4),
             *(void **)(vtxSlot + 0x8));

    if (TransformAndCullPoly(prim, ctx) != 0) {
        goto fail;
    }

    storeSxy(prim, 1);

    gte_ldv0(*(void **)(vtxSlot + 0xc));
    gte_rtps();

    {
        u8 *p0 = *(u8 **)(ctx + 0x94) + 0x14;
        u8 *p1 = *(u8 **)(ctx + 0x98) + 0x14;
        u8 *p2 = *(u8 **)(ctx + 0x9c) + 0x14;
        u8 *p3 = *(u8 **)(ctx + 0xa0) + 0x14;

        gte_stsz4(p0, p1, p2, p3);
    }

    storeSxy(prim, 0);

    gte_stsxy2(ctx + 0x6c);

    func_8001A268(ctx, 4);
    return 0;
fail:
    return 1;
}

/*
 * This unit's local view of the draw context TransformAndCullPoly works on
 * (the `ctx` its callers hand it -- one per-object scratch block, which
 * func_80018464's own caller places in the PS1 scratchpad at 0x1F800000).
 * Only the fields this function touches are typed; the callers still address
 * the rest by offset, and the offsets outside this view that this unit does
 * use are +0x0C the vertex array, +0x1C the semi-transparency flag,
 * +0x14/+0x15 the cached tag length and GPU command byte, +0x88..+0xA0 the
 * per-vertex sort records and +0xA4..+0xB0 the four vertex slots.
 *
 * The three SXY words are deliberately separate fields rather than an array:
 * retail stores them through three independently computed addresses, which
 * is gte_stsxy3()'s three-pointer form.
 */
typedef struct PolyDrawCtx {
    /* +0x000 */ u32 *otBase;        /* ordering table, 4-byte entries */
    /* +0x004 */ s32 otShift;        /* otz >> otShift indexes otBase */
    u8 pad008[0x014 - 0x008];
    /* +0x014 */ u8 tagLen;          /* SetupPrimCode's cached P_TAG length byte */
    u8 pad015[0x020 - 0x015];
    /* +0x020 */ s32 otz;            /* avsz3 result */
    /* +0x024 */ s32 dp;             /* IR0, the depth-cue factor */
    /* +0x028 */ s32 opz;            /* nclip result (MAC0) */
    u8 pad02C[0x030 - 0x02C];
    /* +0x030 */ u32 *otSlot;        /* &otBase[otz >> otShift] */
    u8 pad034[0x05C - 0x034];
    /* +0x05C */ s32 flag;           /* GTE FLAG & 0x40000 */
    /* +0x060 */ s32 sxy0;
    /* +0x064 */ s32 sxy1;
    /* +0x068 */ s32 sxy2;
    u8 pad06C[0x078 - 0x06C];
    /* +0x078 */ s32 saturated;      /* set when the transform saturated */
} PolyDrawCtx;

/*
 * The Psy-Q GPU primitive being filled in. Only the P_TAG length byte is
 * written from here; the GPU command byte one word later (+0x7) is
 * SetupPrimCode's, and the screen XYs are the StoreSxyPoly** leaves'.
 */
typedef struct GpuPrim {
    u8 pad000[3];
    /* +0x003 */ u8 tagLen;          /* P_TAG's length field, little-endian */
} GpuPrim;

/*
 * Perspective-transform the three vertices the caller already loaded into
 * the GTE (gte_ldv3), then cull: bail if the FLAG register shows anything
 * other than clean or SZ3-saturated, if the polygon is back-facing
 * (nclip <= 0), or if the depth-cue factor says it is too close. Otherwise
 * average the Z, cache the screen coordinates and compute the OT bucket.
 * Returns 0 on success, 1 when culled.
 *
 * Splat tagged this "handwritten"; it is ordinary C over the Psy-Q gte_*
 * macros (include/gte.h), and every one of retail's raw words is one of
 * those macros. Earlier rounds carried it as a whole-function __asm__.
 */
s32 TransformAndCullPoly(void *primIn, void *ctxIn)
{
    GpuPrim *prim = primIn;
    PolyDrawCtx *ctx = ctxIn;

    ctx->saturated = 0;
    gte_rtpt();
    prim->tagLen = ctx->tagLen;
    gte_stflg(&ctx->flag);
    if (ctx->flag != 0) {
        if (ctx->flag != 0x40000) {
            return 1;
        }
        ctx->saturated = 1;
    }
    gte_nclip();
    gte_stopz(&ctx->opz);
    if (ctx->opz <= 0) {
        return 1;
    }
    gte_stdp(&ctx->dp);
    if (ctx->dp >= 0x1000) {
        return 1;
    }
    gte_avsz3();
    gte_stotz(&ctx->otz);
    gte_stsxy3(&ctx->sxy0, &ctx->sxy1, &ctx->sxy2);
    ctx->otSlot = &ctx->otBase[ctx->otz >> ctx->otShift];
    return 0;
}

/* The six screen-XY store callbacks ProjectTri/QuadFace invoke, one per
 * Psy-Q primitive type: each writes the GTE's SXY FIFO into that POLY_xx's
 * own vertex offsets. POLY_FT4 and POLY_GT4's equivalents are func_80019774
 * and func_8001979C in code_8220_c. POLY_F3: xy0/xy1/xy2 at +0x8/+0xC/+0x10. */
void StoreSxyPolyF3(void *dst)
{
    gte_stsxy3_f3(dst);
}

/* POLY_G3: +0x8/+0x10/+0x18 (per-vertex RGB between the XYs). */
void StoreSxyPolyG3(void *dst)
{
    gte_stsxy3_g3(dst);
}

/* POLY_FT3: +0x8/+0x10/+0x18 -- byte-identical to the POLY_G3 store, since
 * POLY_G3's per-vertex RGB and POLY_FT3's per-vertex UV are both 4 bytes.
 * Retail keeps them as two separate functions and so does this file; which
 * is which is settled by the (len, code) pair at each one's call site, not
 * by the offsets. */
void StoreSxyPolyFT3(void *dst)
{
    gte_stsxy3_ft3(dst);
}

/* POLY_GT3: +0x8/+0x14/+0x20 (RGB and UV between the XYs). */
void StoreSxyPolyGT3(void *dst)
{
    gte_stsxy3_gt3(dst);
}

/*
 * POLY_F4: xy0/xy1/xy2 at +0x8/+0xC/+0x10, xy3 at +0x14. ProjectQuadFace
 * calls this twice -- storeFirst3 = 1 for the three vertices the shared
 * transform produced, then 0 for the fourth vertex's own rtps result.
 * `p` must be a real unconditionally-computed pointer, not a 0x14(%0)
 * offset inside the asm: see this function's match report.
 */
void StoreSxyPolyF4(void *dst, s32 storeFirst3)
{
    char *p = (char *)dst + 0x14;

    if (storeFirst3) {
        gte_stsxy3_f4(dst);
    } else {
        gte_stsxy2(p);
    }
}

/* POLY_G4: xy0/xy1/xy2 at +0x8/+0x10/+0x18, xy3 at +0x20. Same two-call
 * protocol as StoreSxyPolyF4. */
void StoreSxyPolyG4(void *dst, s32 storeFirst3)
{
    char *p = (char *)dst + 0x20;

    if (storeFirst3) {
        gte_stsxy3_g4(dst);
    } else {
        gte_stsxy2(p);
    }
}
