#include "common.h"
#include "code_d294.h"

/* Sibling of func_8001D344/D374/D3A0/D3CC/D3F8 (code_d294.c): a thin
 * wrapper around func_8001EDAC over &self->unk10, shift 0 width 3. Raw
 * pass-through value and raw pass-through result -- same shape as
 * func_8001D374/D3A0/D3F8 (no `== 0` on either side). */
u32 func_8001D424(Class6B5CCObj *self, u32 a1) {
    return func_8001EDAC(&self->unk10, 0, 3, a1);
}

/* Sibling of func_8001D344 (the ONLY one of the five already-matched
 * self->unk10 bitfield accessors that both converts its input to a boolean
 * (`a1 == 0`) AND inverts its own result (`== 0`)). This function does
 * exactly that double-inversion, at shift 7 width 1, hence the same `s32`
 * return type as func_8001D344 rather than the plain `u32` of the other
 * three siblings. */
s32 func_8001D450(Class6B5CCObj *self, s32 a1) {
    return func_8001EDAC(&self->unk10, 7, 1, a1 == 0) == 0;
}

/* Same family as func_8001D424, shift 9 width 3. Raw pass-through. */
u32 func_8001D480(Class6B5CCObj *self, u32 a1) {
    return func_8001EDAC(&self->unk10, 9, 3, a1);
}

/* Same family as func_8001D450: double-inversion shape, shift 8 width 1. */
s32 func_8001D4AC(Class6B5CCObj *self, s32 a1) {
    return func_8001EDAC(&self->unk10, 8, 1, a1 == 0) == 0;
}

/* self->unk14->unk44 is a 0x28-byte heap block whose +0x10 holds an
 * S16Quad_d294 (see include/code_d294.h). a2 selects between negating x/y/z
 * into a local copy (the 4th short left uninitialised, exactly as retail's
 * own negate path never stores to it) or copying the quad verbatim, then
 * forwards the result -- plus a1, passed straight through -- to the PsyQ
 * helper func_800160B0. */
void func_8001D4DC(Class6B5CCObj *self, s32 a1, s32 a2) {
    S16Quad_d294 buf;
    S16Quad_d294 *src = &self->unk14->unk44->vec;

    if (a2) {
        buf.x = -src->x;
        buf.y = -src->y;
        buf.z = -src->z;
    } else {
        buf = *src;
    }
    func_800160B0(&buf, a1);
}

/* a1 gates a small range (2 <= a1 < 4). When self->unk20 is set and
 * func_8001F3A4(self->unk20) reports true, fills a stack buffer through
 * this class's own +0x8C slot (func_8001D600, already matched in this
 * unit -- fills it via func_8001F51C(self->unk20, dest)) then forwards
 * that same buffer, retyped as a GenericCountList_d294, into +0x90
 * (func_8001D624, also already matched in this unit), with the original
 * a1 passed through as func_8001D624's own a2. */
void func_8001D568(Class6B5CCObj *self, s32 a1) {
    /* Sized to reproduce retail's own frame (0x58): func_8001D600's own
     * target (func_8001F51C, PsyQ, asm/psyq_GsLinkObject4.s, not
     * decompiled here) fills fields out past +0x32 of its own `dest`
     * argument, so the true destination struct is bigger than the 8 bytes
     * GenericCountList_d294 alone would reserve -- not derived beyond its
     * size, since the field layout past what func_8001D624 itself reads
     * (+0x0/+0x4) is PsyQ-internal. */
    u8 buf[0x38];

    if (a1 >= 4) {
        return;
    }
    if (a1 < 2) {
        return;
    }
    if (self->unk20 == NULL) {
        return;
    }
    if (!func_8001F3A4(self->unk20)) {
        return;
    }
    self->methods->slot8C(self, buf);
    self->methods->slot90(self, (GenericCountList_d294 *)buf, a1);
}

/* Forwards self->unk20 (still opaque, retyped `void *` this round -- see
 * include/code_d294.h) and its own 2nd argument straight through to
 * func_8001F51C, untouched. func_8001F51C's own body (psyq_GsLinkObject4.s)
 * has no deliberate return value -- see the extern's own comment -- so this
 * wrapper is void, not `return func_8001F51C(...)`. */
void func_8001D600(Class6B5CCObj *self, void *dest) {
    func_8001F51C(self->unk20, dest);
}

/* Copies a1's own count*8 elements into self->unk14->unk24 (via
 * func_8001EE04, both its src and dest args are &a1->unk4 -- computed once,
 * copied, per the disassembly), zeroes unk28/unk2C, stashes a1 into unk30
 * for the duration of a single self->methods->slot30(self, a2) dispatch
 * (an inherited BasicClass slot, not this unit's own code), then clears
 * unk30 again. */
void func_8001D624(Class6B5CCObj *self, GenericCountList_d294 *a1, s32 a2) {
    func_8001EE04(&a1->unk4, &a1->unk4, a1->unk0 * 8, &self->unk14->unk24);
    self->unk28 = 0;
    self->unk2C = 0;
    self->unk30 = a1;
    self->methods->slot30(self, a2);
    self->unk30 = NULL;
}

void func_8001D6A4(void) {
}

void func_8001D6AC(void) {
}

/* a2 selects one of three behaviors: 2 or 3 dispatches through the vtable
 * (self->methods->slotA0), exactly 4 stores a1 into self->unk28, and
 * anything else (< 2 or > 4) is a no-op. */
void func_8001D6B4(Class6B5CCObj *self, s32 a1, s32 a2) {
    switch (a2) {
    case 2:
    case 3:
        self->methods->slotA0(self);
        break;
    case 4:
        self->unk28 = a1;
        break;
    }
}

/* Range-checks `other` against `self` (each axis of position difference
 * must fit in +/-0x4000), then hands off to three vtable slots
 * (+0xA4 = func_8001D950, +0xA8 = func_8001DA28, +0xAC = func_8001DDF4)
 * with the resulting Vec3S16 difference, before registering `other` into
 * self->unk28 and notifying it via its own +0x038 slot. */
INCLUDE_ASM("asm/nonmatchings/code_d294_b", func_8001D714);

/* Fills buf1 from self's own +0x84 slot, then folds in every node of the
 * self->unkC list (each node's own +0x84 slot combined into buf1 via
 * func_80015BFC) before using buf1 as func_8001EE04's own "out" argument,
 * twice: once for (arg2, arg3, count), once more for (arg1, arg1, 1) when
 * arg1 is non-NULL. */
void func_8001D950(Class6B5CCObj *self, void *arg1, void *arg2, void *arg3, s32 count) {
    u8 buf2[0x20];
    u8 buf1[0x20];
    UnkOwner_d294 *node;

    self->methods->slot84(self, buf1, 1);

    node = self->unkC;
    if (node != NULL) {
        do {
            node->methods->slot84(node, buf2, 1);
            func_80015BFC(buf2, buf1);
            node = node->next;
        } while (node != NULL);
    }

    func_8001EE04(arg2, arg3, count, buf1);
    if (arg1 != NULL) {
        func_8001EE04(arg1, arg1, 1, buf1);
    }
}

INCLUDE_ASM("asm/nonmatchings/code_d294_b", func_8001DA28);

INCLUDE_ASM("asm/nonmatchings/code_d294_b", func_8001DDF4);

INCLUDE_ASM("asm/nonmatchings/code_d294_b", func_8001E110);

/* Bisects the segment [near, far] against `box` until the midpoint exactly
 * equals one endpoint, writing the running midpoint into `out` every
 * iteration (the caller's real result is whatever `*out` holds when this
 * returns). Each iteration computes an outcode (`flags`, matching the
 * project's already-confirmed `u8`-flags idiom -- an explicit `andi
 * $v0,$v1,0xFF` re-mask appears in retail wherever `flags` is read back)
 * from `box` against the midpoint; a non-zero outcode means the midpoint
 * overshot, so it becomes the new `far`, otherwise it becomes the new
 * `near` -- each written into one of two ping-pong stack buffers so the
 * OTHER endpoint's storage is never disturbed. */
void func_8001E2E8(Vec3S16_d294 *out, BoundsBox_d294 *box, Vec3S16_d294 *near, Vec3S16_d294 *far) {
    Vec3S16_d294 buf0;
    Vec3S16_d294 buf1;
    Vec3S16_d294 *dst;
    u8 flags;

    for (;;) {
        out->x = (near->x + far->x) >> 1;
        out->y = (near->y + far->y) >> 1;
        out->z = (near->z + far->z) >> 1;

        if (out->x == near->x && out->y == near->y && out->z == near->z) {
            return;
        }
        if (out->x == far->x && out->y == far->y && out->z == far->z) {
            return;
        }

        flags = 0;
        if (box->hi.x < out->x) {
            flags = 8;
        } else if (out->x < box->lo.x) {
            flags = 4;
        }
        if (box->hi.y < out->y) {
            flags |= 2;
        } else if (out->y < box->lo.y) {
            flags |= 1;
        }
        if (box->hi.z < out->z) {
            flags |= 0x20;
        } else if (out->z < box->lo.z) {
            flags |= 0x10;
        }

        if (flags != 0) {
            dst = &buf1;
            far = dst;
        } else {
            dst = &buf0;
            near = dst;
        }
        *dst = *out;
    }
}

void func_8001E49C(void) {
}

/* STALL -- see docs/match-reports/func_8001E4A4.md. Best reached this
 * round: 48/54 words in-range (up from the round-13 best of 47/54), a
 * clean self<->tag register-pair swap in $s1/$s2, no size drift.
 * Restored to INCLUDE_ASM per project rule. */
#if 0
void func_8001E4A4(Class6B5CCObj *self, void *node) {
    Class6B5CCObj *s;
    void *n;
    GenericObj_d294 *entry;
    void *cursor;
    s32 tag;
    s32 masked;

    s = self;
    n = node;
    tag = 4;
    entry = NULL;
loop:
    BasicClass__func_1816c(n, &entry, &cursor);
    if (entry == NULL) {
        goto check_cursor;
    }
    masked = entry->methods->header & 0xF;
    if (masked == tag) {
        goto dispatch;
    }
check_cursor:
    if (cursor != NULL) {
        goto loop;
    }
    entry = NULL;
dispatch:
    if (entry == NULL) {
        goto tail;
    }
    if (*(u8 *)entry->methods != 0x34) {
        goto tail;
    }
    entry->methods->slot10(entry, s);
tail:
    if (cursor != NULL) {
        goto loop;
    }
}
#endif

INCLUDE_ASM("asm/nonmatchings/code_d294_b", func_8001E4A4);

/* This unit's own no-argument vtable getter -- see the extended note on
 * D_8006B5CC in include/code_d294.h and the file banner up top. */
Class6B5CCMethods *func_8001E57C(void) {
    return &D_8006B5CC;
}
