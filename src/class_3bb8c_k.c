/*
 * class_3bb8c_k -- fifth carved slice of the class_3bb8c block
 * (0x429D4..0x435E0, vram 0x800521D4..0x80052DE0), 20 functions, carved
 * round 15, all matched. Named round 75 (track 3).
 *
 * Methods of two classes (`tools/classtable.py` resolves every one):
 *  - Class86F88 (table gClass86F88Methods, slots +0x054..+0x09C, plus the
 *    two non-virtual helpers FormatRowText/SetView and the table getter
 *    GetClass86F88Methods). A scrolling list selector: up to 4 visible
 *    rows of 26-character item text (one New_TextRow text object each),
 *    a highlighted cursor row (gClass86F88CursorColor, others
 *    gClass86F88RowColor), cursor up/down that scrolls the window at its
 *    edges, and a horizontal column offset. HandleInputCode maps input
 *    codes 25/23 to closing with result 2/3; SetState(4) then reports the
 *    result to the parents, which read the chosen item back through
 *    GetCursorIndex. Its ctor, child and resource methods are in
 *    class_3bb8c_j. Declared in include/Class86F88.h (track 4, round 89).
 *  - ObjM (table gObjMMethods, slots +0x008/+0x00C/+0x038, plus New_ObjM):
 *    its allocator, ctor, dtor and OnNotify. The rest of ObjM is in
 *    class_3bb8c_l and class_3bb8c_m.
 *
 * include/class_3bb8c.h is SHARED with every other class_3bb8c_* slice.
 * Header edits must be strictly ADDITIVE.
 */
#include "common.h"
#include "class_3bb8c.h"
#include "class_39e08.h"
#include "Class86668.h"
#include "TextRow.h"
#include "TimImage.h"
#include "Class86F88.h"

/*
 * class_3bb8c_k's own view of ObjM (method table gObjMMethods, returned by
 * GetObjMMethods; the shared `ObjM`/`Obj87034_3bb8c_l` views are in
 * include/class_3bb8c.h). Only what New_ObjM, ObjM__ObjM and
 * ObjM__OnNotify reach is typed. Kept local to this unit per the
 * multiple-independent-local-views convention.
 *
 * New_ObjM is declared `Obj4C *New_ObjM(SubObjB *, ...)` by
 * include/class_39e08.h (Class865C8__StartObjM's call-site view), which
 * this file includes, so its definition below must keep that exact return
 * and first-argument type.
 */
typedef struct ObjMMethods_3bb8c_k ObjMMethods_3bb8c_k;
typedef struct ObjM_3bb8c_k ObjM_3bb8c_k;

/* Slot names are the method each slot holds in gObjMMethods. */
struct ObjMMethods_3bb8c_k {
    u8 pad000[0x008];
    /* +0x008 ObjM__ObjM, New_ObjM's ctor call. */
    void (*ctor)(void *self, SubObjB *a0, s32 a1, s32 a2, s32 a3, s32 a4);
    u8 pad00C[0x040 - 0x00C];
    /* +0x040 ObjM__NoOpSlot40 (class_3bb8c_l, an empty body), ObjM__ObjM's last call. */
    void (*slot40)(void *self);
    u8 pad044[0x090 - 0x044];
    /* +0x090/+0x0B0/+0x0B4, ObjM__OnNotify's 3-way dispatch on the event
     * target's header tag; all get (self, arg1, arg2) forwarded. */
    void (*slot90)(void *self, EventArg *arg1, s32 arg2);          /* +0x090 ObjM__OnDreamSysNotify */
    u8 pad094[0x0B0 - 0x094];
    void (*handleEvent5Or6)(void *self, EventArg *arg1, s32 arg2); /* +0x0B0 ObjM__OnFadeNotify */
    void (*handleEvent7)(void *self, EventArg *arg1, s32 arg2);    /* +0x0B4 ObjM__OnClass866E8Notify */
};

struct ObjM_3bb8c_k {
    ObjMMethods_3bb8c_k *methods;       /* +0x000, ObjM__ObjM */
    u8 pad004[0x038 - 0x004];
    s32 unk38;                          /* +0x038, ObjM__ObjM: arg5 */
    u8 pad3C[0x054 - 0x03C];
    s32 unk54;                          /* +0x054, ObjM__ObjM: arg2 (shared ObjM view: a FieldM50 *) */
    u8 pad58[0x060 - 0x058];
    s32 unk60;                          /* +0x060, ObjM__ObjM: set to 1 */
    s32 unk64;                          /* +0x064, ObjM__ObjM: zeroed */
    s32 unk68;                          /* +0x068, ObjM__ObjM: zeroed */
    SubObjB *unk6C;                     /* +0x06C, ObjM__ObjM: arg1, also forwarded as the base ctor's own arg2 */
    s32 unk70;                          /* +0x070, ObjM__ObjM: arg4 */
    s32 unk74;                          /* +0x074, ObjM__ObjM: arg3 (shared ObjM view: New_TextRow's ctx) */
    u8 pad78[0x080 - 0x078];
    s32 pauseSetupStep;                 /* +0x080, ObjM__ObjM: zeroed; ObjM__AdvancePauseSetup's step counter (class_3bb8c_m, shared view unk80) */
    s32 closeReady;                     /* +0x084, ObjM__ObjM: zeroed; set by ObjM__UpdateCloseReadyFlag, cleared by ObjM__ClearCloseReadyFlag, tested by ObjM__CloseAndNotifyC/D (shared view unk84) */
};

void Class86F88__SetState(Class86F88 *self, s32 state)
{
    self->closeTicks = 0;
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
    self->methods->removeChild(self, self->inputSource);
    self->methods->releaseResources(self);
    self->result = state;
    goto end;
case_eq4:
    self->methods->notifyParents(self, self->result);
end:
    return;
}

void Class86F88__TickClosing(Class86F88 *self)
{
    s32 old;

    if (self->result >= 4) {
        return;
    }
    if (self->result < 2) {
        return;
    }
    old = self->closeTicks;
    self->closeTicks = old + 1;
    if (old == 0) {
        return;
    }
    self->methods->setState(self, 4);
}

void Class86F88__HandleInputCode(Class86F88 *self, void *source, s32 code) {
    switch (code) {
    case 25:
        self->methods->forwardToTarget(self, 0x10);
        self->methods->setState(self, 2);
        break;
    case 23:
        self->methods->forwardToTarget(self, 0x10);
        self->methods->setState(self, 3);
        break;
    case 5:
        self->methods->scrollRight(self);
        break;
    case 4:
        self->methods->scrollLeft(self);
        break;
    case 18:
        self->methods->cursorUp(self);
        break;
    case 19:
        self->methods->cursorDown(self);
        break;
    }
}

void Class86F88__ForwardToTarget(Class86F88 *self, s32 code)
{
    struct TargetObj86ED0 *target = self->target;

    if (target != NULL) {
        target->methods->slot80(target, code, 0x60, 0x60);
    }
}

void Class86F88__ScrollRight(Class86F88 *self)
{
    Class86F88Methods *methods;
    s32 tmp;
    s32 column;

    if (!self->panelSprite) {
        return;
    }
    tmp = self->column;
    column = tmp;
    if (column + 0x1A >= self->maxTextLen) {
        return;
    }
    methods = self->methods;
    column++;
    self->column = column;
    methods->refreshRows(self, self->topIndex, column, self->cursorIndex, 1);
}

void Class86F88__ScrollLeft(Class86F88 *self)
{
    s32 column;

    if (!self->panelSprite) {
        return;
    }
    column = self->column - 1;
    if (column < 0) {
        return;
    }
    self->column = column;
    self->methods->refreshRows(self, self->topIndex, column, self->cursorIndex, 1);
}

void Class86F88__CursorUp(Class86F88 *self, s32 arg1, s32 arg2, s32 arg3)
{
    s32 cursor;
    s32 newTop;
    s32 newCursor;

    if (!self->panelSprite) {
        return;
    }
    cursor = self->cursorIndex;
    if (cursor - 1 < 0) {
        return;
    }
    if (cursor - self->topIndex > 0) {
        self->methods->stepCursorInView(self, 0, 1, arg3);
    } else {
        self->topIndex--;
        newTop = self->topIndex;
        self->cursorIndex--;
        newCursor = self->cursorIndex;
        self->methods->refreshRows(self, newTop, self->column, newCursor, 1);
    }
}

void Class86F88__CursorDown(Class86F88 *self, s32 arg1, s32 arg2, s32 arg3)
{
    s32 newTop;
    s32 newCursor;
    s32 prevTop;

    if (!self->panelSprite) {
        return;
    }
    if (self->cursorIndex + 1 >= self->itemCount) {
        return;
    }
    prevTop = self->topIndex - 1;
    if (self->cursorIndex - prevTop < 4) {
        self->methods->stepCursorInView(self, 1, 1, arg3);
    } else {
        self->topIndex++;
        newTop = self->topIndex;
        self->cursorIndex++;
        newCursor = self->cursorIndex;
        self->methods->refreshRows(self, newTop, self->column, newCursor, 1);
    }
}

/* The first row's position, two sdata words (-0x5C, -0xF). Read by value
 * into Class86F88__CreateRows's `pos`; each further row is 0xA lower. */
extern s32 gClass86F88RowOriginX;
extern s32 gClass86F88RowOriginY;

void Class86F88__CreateRows(Class86F88 *self, Class6B5CC *parent, TimImage *font, s32 top, s32 column, s32 cursor)
{
    char buf[0x20];
    ScreenSpritePos pos;
    TextRow **p;
    s32 count;
    s32 i;

    if (!self->panelSprite) {
        return;
    }

    pos.x = gClass86F88RowOriginX;
    pos.y = gClass86F88RowOriginY;
    count = self->itemCount;
    p = &self->rows[0];
    if (count >= 5) {
        count = 4;
    }

    for (i = 0; i < count; i++) {
        Class86F88__FormatRowText(self, buf, i, top, column);
        *p = New_TextRow(font, 0x1A, buf);
        (*p)->methods->attachToParent(*p, parent, (Vec3_d294 *)&pos);
        (*p)->methods->setColor(*p, &gClass86F88RowColor);
        pos.y += 0xA;
        p++;
    }

    Class86F88__SetView(self, top, column, cursor, 1);
}

void Class86F88__ReleaseRows(Class86F88 *self)
{
    s32 count;
    s32 i;
    u8 unused[8];

    if (!self->panelSprite) {
        return;
    }
    count = self->itemCount;
    i = 0;
    if (count >= 5) {
        count = 4;
    }
    if (count <= 0) {
        return;
    }
    do {
        self->rows[i]->methods->release(self->rows[i]);
        self->rows[i] = NULL;
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

void Class86F88__RefreshRows(Class86F88 *self, s32 top, s32 column, s32 cursor, s32 notify)
{
    s32 count;
    s32 i;
    char buf[0x20];
    TextRow **p;

    if (!self->panelSprite) {
        return;
    }
    count = self->itemCount;
    p = &self->rows[0];
    if (count >= 5) {
        count = 4;
    }
    for (i = 0; i < count; i++) {
        Class86F88__FormatRowText(self, buf, i, top, column);
        (*p)->methods->setText(*p, buf);
        p++;
    }
    Class86F88__SetView(self, top, column, cursor, 0);
    if (notify) {
        self->methods->forwardToTarget(self, 0);
    }
}

char *Class86F88__FormatRowText(Class86F88 *self, char *dest, s32 row, s32 top, s32 column)
{
    s32 idx = top + row;
    s32 len;
    s32 i;

    len = strlen(self->texts[idx] + column);
    if (len >= 0x1B) {
        len = 0x1A;
    }
    memcpy(dest, self->texts[idx] + column, len);
    i = len;
    if (i < 0x1A) {
        for (; i < 0x1A; i++) {
            dest[i] = ' ';
        }
    }
    dest[0x1A] = 0;
    return dest;
}

void Class86F88__SetView(Class86F88 *self, s32 top, s32 column, s32 cursor, s32 highlight)
{
    TextRow *elem;
    s32 flag = highlight;

    __asm__("");
    self->topIndex = top;
    self->column = column;
    self->cursorIndex = cursor;
    if (flag == 0) {
        return;
    }
    cursor -= top;
    elem = self->rows[cursor];
    elem->methods->setColor(elem, &gClass86F88CursorColor);
}

void Class86F88__StepCursorInView(Class86F88 *self, s32 dir, s32 notify)
{
    TextRow **p;
    s32 idx;

    if (!self->panelSprite) {
        return;
    }
    idx = self->cursorIndex - self->topIndex;
    p = &self->rows[idx];
    (*p)->methods->setColor(*p, &gClass86F88RowColor);
    if (dir) {
        self->cursorIndex++;
        p++;
    } else {
        self->cursorIndex--;
        p--;
    }
    (*p)->methods->setColor(*p, &gClass86F88CursorColor);
    if (notify) {
        self->methods->forwardToTarget(self, 0);
    }
}

s32 Class86F88__GetCursorIndex(Class86F88 *self)
{
    return self->cursorIndex;
}

Class86F88Methods *GetClass86F88Methods(void)
{
    return &gClass86F88Methods;
}

Obj4C *New_ObjM(SubObjB *a0, s32 a1, s32 a2, s32 a3, s32 a4)
{
    Obj4C *self;
    ObjMMethods_3bb8c_k *methods;

    self = BMemPMgrAlloc(0x88);
    if (self != NULL) {
        methods = GetObjMMethods();
        methods->ctor(self, a0, a1, a2, a3, a4);
        return self;
    }
    return NULL;
}

void ObjM__ObjM(ObjM_3bb8c_k *self, SubObjB *arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5)
{
    GetClass86668Methods()->ctor((Class86668 *)self, 0, (BasicClass *)arg1);
    self->methods = GetObjMMethods();
    self->unk64 = 0;
    self->unk68 = 0;
    self->unk60 = 1;
    self->unk54 = arg2;
    self->unk38 = arg5;
    self->unk6C = arg1;
    self->unk74 = arg3;
    self->unk70 = arg4;
    self->pauseSetupStep = 0;
    self->closeReady = 0;
    self->methods->slot40(self);
}

void ObjM__Finalize(ObjM_3bb8c_k *self)
{
    GetClass86668Methods()->finalize((Class86668 *)self);
}

void ObjM__OnNotify(ObjM_3bb8c_k *self, EventArg *arg1, s32 arg2)
{
    s32 tag;

    GetClass86668Methods()->onNotify((Class86668 *)self, arg1, arg2);
    tag = arg1->target->header;
    if ((tag & 0xFFF) == 0x114) {
        self->methods->handleEvent7(self, arg1, arg2);
    } else if ((tag & 0xFFF) == 0x164) {
        self->methods->handleEvent5Or6(self, arg1, arg2);
    } else if ((tag & 0xFFFF) == 0x1F34) {
        self->methods->slot90(self, arg1, arg2);
    }
}
