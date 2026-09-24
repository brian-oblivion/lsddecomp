/*
 * ROUND 42 CORRECTION (2026-09-15) -- READ BEFORE ANY "BLOCKED" LINE BELOW:
 * every claim in this comment that a function is BLOCKED by `gp_rel`,
 * `nop_mflo_mfhi` or `addiu_at` is STALE.  All three constructs are RESOLVED
 * by pinned maspsx flags (CLAUDE.md, "Open toolchain blockers");
 * `tools/nearmiss.py` reports them tagged (RESOLVED-not-a-blocker) and counts
 * none of them.  Any "do NOT spend attempts on these" directive below is
 * therefore RETRACTED: those functions are ordinary matching work, and most
 * carry a mechanism-correct partial derivation already.  The rest of this
 * comment still stands -- only the blocker verdicts are withdrawn.
 * Screen: `python3 tools/nearmiss.py`, round 43 (2026-09-15).
 *
 * class_3bb8c_k -- fifth carved slice of the class_3bb8c block
 * (0x429D4..0x435E0, vram 0x800521D4..0x80052DE0), 20 functions.
 * Carved round 15.
 *
 * Blocker profile (head's Gate 1 THREE-grep screen at carve time -- the screen
 * is TWO greps as of round 21, and screening for `addiu_at` now INVENTS
 * blockers, which is the strictly worse failure). Corrected round 27
 * (2026-09-10), re-screened with `python3 tools/nearmiss.py`:
 *   Class86F88__HandleInputCode  was addiu-$at ONLY -- NOT BLOCKED. `addiu_at` was RESOLVED
 *                  in round 21 (maspsx `--addiu-at`;
 *                  docs/research/addiu-at-blocker.md). ALREADY MATCHED, so
 *                  this correction costs nothing -- but the directive below
 *                  was live for six rounds and would have warned a runner off
 *                  matchable ground. It still OWNS jtbl_800116F4, which is why
 *                  the rodata slot at 0x1EF4 is attached to this unit.
 *   func_80052644  gp_rel -- MATCHED round 45 (82/82 words). The `gp_rel`
 *                  blocker itself was RESOLVED round 42; see the file-top
 *                  banner above.
 * So: ONE blocked function, not two, and even that one is resolved now. The
 * old line here read "Both have stub
 * reports; do not attempt either" -- a stale DIRECTIVE, which is worse than a
 * stale fact, because a reader acts on it without re-measuring.
 *
 * include/class_3bb8c.h is SHARED with every other class_3bb8c_* slice.
 * Header edits must be strictly ADDITIVE.
 */
#include "common.h"
#include "class_3bb8c.h"
#include "class_39e08.h"

/*
 * class_3bb8c_k's own view of the class whose method table is D_80087034
 * (GetObjMMethods's return value) -- a DIFFERENT class from Class86F88
 * above, and a DIFFERENT (disjoint-slot) view from class_3bb8c_l's own
 * Obj87034_3bb8c_l/Obj87034Methods_3bb8c_l (see include/class_3bb8c.h):
 * that unit reaches slots 0x004/0x010/0x014/0x048/0x074/0x07C/0x080/0x084/
 * 0x088/0x08C/0x0C0/0x0C4/0x0C8/0x0D0/0x0D4; this unit's own two functions
 * (func_80052B70, the New_X allocator, and func_80052C10, its ctor target)
 * reach only +0x008 (ctor) and +0x040 (a post-construct hook dispatched by
 * func_80052C10 itself). Kept LOCAL to this unit (not added to either
 * shared header) per the project's multiple-independent-local-views
 * convention and this round's header-contention rule -- echo is live on
 * class_3bb8c_l's own view of the SAME table this round.
 *
 * func_80052B70 itself is declared with a NARROWER opaque return type,
 * `Obj4C *`, by the pre-existing prototype in include/class_39e08.h (that
 * unit's own independent view, established from Obj865C8__EnterState2's call
 * site) -- this file includes class_39e08.h, so func_80052B70's definition
 * below must match that prototype exactly (return type and first-argument
 * type) or the two conflict. The richer view below is used only inside
 * func_80052B70/func_80052C10's own bodies.
 */
typedef struct Class87034Methods_3bb8c_k Class87034Methods_3bb8c_k;
typedef struct Obj87034_3bb8c_k Obj87034_3bb8c_k;

struct Class87034Methods_3bb8c_k {
    u8 pad000[0x008];
    /* +0x008, func_80052B70's own dispatch (New_X's ctor call). */
    void (*ctor)(void *self, SubObjB *a0, s32 a1, s32 a2, s32 a3, s32 a4);
    u8 pad00C[0x040 - 0x00C];
    /* +0x040, func_80052C10's own dispatch, right after filling self's
     * fields -- a post-construct hook, self only. */
    void (*slot40)(void *self);
    u8 pad044[0x054 - 0x044];
    /* +0x054/+0x060, Class86F88__HandleInputCode's own event/code dispatch (below) --
     * both called with a constant second argument. */
    void (*slot54)(void *self, s32 arg1);
    u8 pad058[0x060 - 0x058];
    void (*slot60)(void *self, s32 arg1);
    u8 pad064[0x07C - 0x064];
    /* +0x07C/+0x080/+0x084/+0x088, Class86F88__HandleInputCode's own event/code
     * dispatch (below) -- self only, no other arguments. */
    void (*slot7C)(void *self);
    void (*slot80)(void *self);
    void (*slot84)(void *self);
    void (*slot88)(void *self);
    u8 pad08C[0x090 - 0x08C];
    /* +0x090/+0x0B0/+0x0B4, func_80052D10's own 3-way event-type dispatch
     * (self, the same EventArg* it was itself called with, and its own
     * arg2, forwarded verbatim to whichever slot the event's header tag
     * selects). */
    void (*slot90)(void *self, EventArg *arg1, s32 arg2);
    u8 pad094[0x0B0 - 0x094];
    void (*slotB0)(void *self, EventArg *arg1, s32 arg2);
    void (*slotB4)(void *self, EventArg *arg1, s32 arg2);
};

struct Obj87034_3bb8c_k {
    Class87034Methods_3bb8c_k *methods; /* +0x000, func_80052C10 */
    u8 pad004[0x038 - 0x004];
    s32 unk38;                          /* +0x038, func_80052C10: arg5 */
    u8 pad3C[0x054 - 0x03C];
    s32 unk54;                          /* +0x054, func_80052C10: arg2 */
    u8 pad58[0x060 - 0x058];
    s32 unk60;                          /* +0x060, func_80052C10: set to 1 */
    s32 unk64;                          /* +0x064, func_80052C10: zeroed */
    s32 unk68;                          /* +0x068, func_80052C10: zeroed */
    SubObjB *unk6C;                     /* +0x06C, func_80052C10: arg1, also forwarded as the base ctor's own arg2 */
    s32 unk70;                          /* +0x070, func_80052C10: arg4 */
    s32 unk74;                          /* +0x074, func_80052C10: arg3 */
    u8 pad78[0x080 - 0x078];
    s32 unk80;                          /* +0x080, func_80052C10: zeroed */
    s32 unk84;                          /* +0x084, func_80052C10: zeroed */
};

void Class86F88__SetState(Class86F88 *self, s32 state)
{
    self->unk30 = 0;
    if (state < 2) {
        goto end;
    }
    if (state < 4) {
        goto case_lt4;
    }
    if (state == 4) {
        goto case_eq4;
    }
    goto end;
case_lt4:
    self->methods->slot14(self, self->unk34);
    self->methods->slot48(self);
    self->unk2C = state;
    goto end;
case_eq4:
    self->methods->slot30(self, self->unk2C);
end:
    return;
}

void Class86F88__TickClosing(Class86F88 *self)
{
    s32 old;

    if (self->unk2C >= 4) {
        return;
    }
    if (self->unk2C < 2) {
        return;
    }
    old = self->unk30;
    self->unk30 = old + 1;
    if (old == 0) {
        return;
    }
    self->methods->slot54(self, 4);
}

void Class86F88__HandleInputCode(Obj87034_3bb8c_k *self, void *arg1, s32 code) {
    switch (code) {
    case 25:
        self->methods->slot60(self, 0x10);
        self->methods->slot54(self, 2);
        break;
    case 23:
        self->methods->slot60(self, 0x10);
        self->methods->slot54(self, 3);
        break;
    case 5:
        self->methods->slot7C(self);
        break;
    case 4:
        self->methods->slot80(self);
        break;
    case 18:
        self->methods->slot84(self);
        break;
    case 19:
        self->methods->slot88(self);
        break;
    }
}

void Class86F88__ForwardToTarget(Class86F88 *self, s32 arg1)
{
    Class86F88 *other = self->unk3C;

    if (other != NULL) {
        other->methods->slot80(other, arg1, 0x60, 0x60);
    }
}

void Class86F88__ScrollRight(Class86F88 *self)
{
    Class86F88Methods *methods;
    s32 tmp;
    s32 count;

    if (!self->unk50) {
        return;
    }
    tmp = self->unk24;
    count = tmp;
    if (count + 0x1A >= self->unk14) {
        return;
    }
    methods = self->methods;
    count++;
    self->unk24 = count;
    methods->slot94(self, self->unk20, count, self->unk28, 1);
}

void func_80052498(Class86F88 *self)
{
    s32 count;

    if (!self->unk50) {
        return;
    }
    count = self->unk24 - 1;
    if (count < 0) {
        return;
    }
    self->unk24 = count;
    self->methods->slot94(self, self->unk20, count, self->unk28, 1);
}

void func_800524F8(Class86F88 *self, s32 arg1, s32 arg2, s32 arg3)
{
    s32 count;
    s32 newUnk20;
    s32 newUnk28;

    if (!self->unk50) {
        return;
    }
    count = self->unk28;
    if (count - 1 < 0) {
        return;
    }
    if (count - self->unk20 > 0) {
        self->methods->slot98(self, 0, 1, arg3);
    } else {
        self->unk20--;
        newUnk20 = self->unk20;
        self->unk28--;
        newUnk28 = self->unk28;
        self->methods->slot94(self, newUnk20, self->unk24, newUnk28, 1);
    }
}

void func_80052598(Class86F88 *self, s32 arg1, s32 arg2, s32 arg3)
{
    s32 newUnk20;
    s32 newUnk28;
    s32 prevUnk20;

    if (!self->unk50) {
        return;
    }
    if (self->unk28 + 1 >= self->unk10) {
        return;
    }
    prevUnk20 = self->unk20 - 1;
    if (self->unk28 - prevUnk20 < 4) {
        self->methods->slot98(self, 1, 1, arg3);
    } else {
        self->unk20++;
        newUnk20 = self->unk20;
        self->unk28++;
        newUnk28 = self->unk28;
        self->methods->slot94(self, newUnk20, self->unk24, newUnk28, 1);
    }
}

/* Defined later in this file (ROM order); forward-declared here since
 * func_80052644 calls both, same convention as Obj865C8__EnterState2 in
 * src/class_39e08.c. Signatures must match their real definitions below
 * exactly. */
extern char *func_8005292C(Class86F88 *self, char *dest, s32 arg3, s32 arg4, char *base);
extern void func_800529FC(Class86F88 *self, s32 a1, s32 a2, s32 a3, s32 a4);

/* VALUE-of `%gp_rel`, round 45's own local view -- two plain s32
 * constants (`D_8008AB00`=-0x5C, `D_8008AB04`=-0xF in the ROM image,
 * `asm/data/7B12C.sdata.s`) seeding a 2-word stack-local this function
 * builds and passes to each freshly-created element's own `slot4C`. */
extern s32 D_8008AB00;
extern s32 D_8008AB04;

/* func_80052644's own stack-local argument to Class86F88ElemMethods::
 * slot4C -- `a` is D_8008AB00's value, set once; `b` starts at
 * D_8008AB04's value and accumulates by 0xA per loop iteration. Kept
 * local to this unit (see the shared header's own `void *arg2` for that
 * slot) since nothing else gives this argument any shape. */
typedef struct {
    s32 a;
    s32 b;
} Elem4CArg_3bb8c_k;

void func_80052644(Class86F88 *self, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5)
{
    char buf[0x20];
    Elem4CArg_3bb8c_k local;
    Class86F88Elem **p;
    s32 count;
    s32 i;

    if (!self->unk50) {
        return;
    }

    local.a = D_8008AB00;
    local.b = D_8008AB04;
    count = self->unk10;
    p = &self->unk40[0];
    if (count >= 5) {
        count = 4;
    }

    for (i = 0; i < count; i++) {
        func_8005292C(self, buf, i, arg3, (char *)arg4);
        *p = (Class86F88Elem *)New_Obj6EAC0((void *)arg2, 0x1A, buf);
        (*p)->methods->slot4C(*p, arg1, &local);
        (*p)->methods->slotB8(*p, &D_8008AB0C);
        local.b += 0xA;
        p++;
    }

    func_800529FC(self, arg3, arg4, arg5, 1);
}

void func_8005278C(Class86F88 *self)
{
    s32 count;
    s32 i;
    u8 unused[8];

    if (!self->unk50) {
        return;
    }
    count = self->unk10;
    i = 0;
    if (count >= 5) {
        count = 4;
    }
    if (count <= 0) {
        return;
    }
    do {
        self->unk40[i]->methods->release(self->unk40[i]);
        self->unk40[i] = NULL;
        i++;
    } while (i < count);
}

/* Psy-Q strlen and memcpy (libc2/strlen and libc2/memcpy, linked from
 * Sony's own SDK objects). libc2's memcpy guards a NULL dest, copies `n`
 * bytes a byte at a time and returns dest -- which is why this call site
 * was read as strncpy-like before the object gave it its name.
 * Both are declared LOCAL to this unit, not in a shared header, since
 * these are cross-unit prototypes for functions this unit does not define
 * (see CLAUDE.md's header-contention rule). strlen already has a
 * differently-typed local declaration in class_3bb8c_j.c
 * (`s32 strlen(void *arg0)`); this unit's own call site reads its
 * argument as a byte pointer, so it is typed `char *` here instead --
 * per-call-site typing of an undefined function's argument, same
 * convention as DecodeFullWidthSjis (see include/class_3bb8c.h HEAD NOTE).
 * memcpy's return type is `void *` to agree with include/psyq/MEMORY.H's
 * unprototyped declaration should this unit ever include it; the result
 * is discarded at the one call site either way. */
extern s32 strlen(char *s);
extern void *memcpy(char *dest, char *src, s32 n);

void func_8005281C(Class86F88 *self, s32 arg1, s32 arg2, s32 arg3, s32 arg4)
{
    s32 count;
    s32 i;
    char buf[0x20];
    Class86F88Elem **p;

    if (!self->unk50) {
        return;
    }
    count = self->unk10;
    p = &self->unk40[0];
    if (count >= 5) {
        count = 4;
    }
    for (i = 0; i < count; i++) {
        func_8005292C(self, buf, i, arg1, (char *)arg2);
        (*p)->methods->slotCC(*p, buf);
        p++;
    }
    func_800529FC(self, arg1, arg2, arg3, 0);
    if (arg4) {
        self->methods->slot60(self, 0);
    }
}

char *func_8005292C(Class86F88 *self, char *dest, s32 arg3, s32 arg4, char *base)
{
    s32 idx = arg4 + arg3;
    s32 len;
    s32 i;

    len = strlen(base + self->unk18[idx]);
    if (len >= 0x1B) {
        len = 0x1A;
    }
    memcpy(dest, base + self->unk18[idx], len);
    i = len;
    if (i < 0x1A) {
        for (; i < 0x1A; i++) {
            dest[i] = ' ';
        }
    }
    dest[0x1A] = 0;
    return dest;
}

void func_800529FC(Class86F88 *self, s32 a1, s32 a2, s32 a3, s32 a4)
{
    Class86F88Elem *elem;
    s32 flag = a4;

    __asm__("");
    self->unk20 = a1;
    self->unk24 = a2;
    self->unk28 = a3;
    if (flag == 0) {
        return;
    }
    a3 -= a1;
    elem = self->unk40[a3];
    elem->methods->slotB8(elem, &D_8008AB10);
}

void func_80052A58(Class86F88 *self, s32 dir, s32 flag)
{
    Class86F88Elem **p;
    s32 idx;

    if (!self->unk50) {
        return;
    }
    idx = self->unk28 - self->unk20;
    p = &self->unk40[idx];
    (*p)->methods->slotB8(*p, &D_8008AB0C);
    if (dir) {
        self->unk28++;
        p++;
    } else {
        self->unk28--;
        p--;
    }
    (*p)->methods->slotB8(*p, &D_8008AB10);
    if (flag) {
        self->methods->slot60(self, 0);
    }
}

s32 func_80052B54(Class86F88 *self)
{
    return self->unk28;
}

Class86F88Methods *func_80052B60(void)
{
    return &D_80086F88;
}

Obj4C *func_80052B70(SubObjB *a0, s32 a1, s32 a2, s32 a3, s32 a4)
{
    Obj4C *self;
    Class87034Methods_3bb8c_k *methods;

    self = BMemPMgrAlloc(0x88);
    if (self != NULL) {
        methods = GetObjMMethods();
        methods->ctor(self, a0, a1, a2, a3, a4);
        return self;
    }
    return NULL;
}

void func_80052C10(Obj87034_3bb8c_k *self, SubObjB *arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5)
{
    GetClass86668Methods()->ctor((Obj865C8 *)self, 0, arg1);
    self->methods = GetObjMMethods();
    self->unk64 = 0;
    self->unk68 = 0;
    self->unk60 = 1;
    self->unk54 = arg2;
    self->unk38 = arg5;
    self->unk6C = arg1;
    self->unk74 = arg3;
    self->unk70 = arg4;
    self->unk80 = 0;
    self->unk84 = 0;
    self->methods->slot40(self);
}

void func_80052CD8(Obj865C8 *self)
{
    GetClass86668Methods()->dtor(self);
}

void func_80052D10(Obj87034_3bb8c_k *self, EventArg *arg1, s32 arg2)
{
    s32 tag;

    GetClass86668Methods()->slot38((Obj865C8 *)self, arg1, arg2);
    tag = arg1->target->header;
    if ((tag & 0xFFF) == 0x114) {
        self->methods->slotB4(self, arg1, arg2);
    } else if ((tag & 0xFFF) == 0x164) {
        self->methods->slotB0(self, arg1, arg2);
    } else if ((tag & 0xFFFF) == 0x1F34) {
        self->methods->slot90(self, arg1, arg2);
    }
}
