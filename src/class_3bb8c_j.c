/*
 * class_3bb8c_j -- fourth carved slice of the class_3bb8c block
 * (0x41F84..0x429D4, vram 0x80051784..0x800521D4), 20 functions, carved
 * round 15. include/class_3bb8c.h is SHARED with every other class_3bb8c_*
 * slice; header edits here must be strictly ADDITIVE.
 *
 * Two unrelated classes' methods live in this address range:
 *  - The first six functions (TextEntry__PrevChar .. TextEntry__SetCharAt)
 *    and GetTextEntryMethods are TextEntry's (gTextEntryMethods slots
 *    +0x094..+0x0A8, `tools/classtable.py gTextEntryMethods`), declared in
 *    include/TextEntry.h; the rest of the class is class_3bb8c_i.
 *  - Everything else is `Class86F88_3bb8c_j` (gClass86F88Methods, a BasicClass
 *    subclass, alloc size 0x54, reached through `GetClass86F88Methods()` in
 *    class_3bb8c_k which already holds this SAME table under the bare
 *    name `Class86F88`) -- kept LOCAL under a disambiguating suffix
 *    rather than reusing that name, to avoid a collision in this TU.
 *
 * Both attributions were WRONG before round 75 (the first group was typed
 * as Class866E8's `Obj866E8`, the second named after gTextEntryMethods).
 * See `Class86F88__Class86F88.md` for the `tools/classtable.py` evidence.
 */
#include "common.h"
#include "class_3bb8c.h"
#include "TextEntry.h"
#include "ScreenSprite.h"
#include "TimImage.h"

void TextEntry__PrevChar(TextEntry *self)
{
    s32 count;

    if (self->panelSprite) {
        count = self->charIndex - 1;
        self->charIndex = count;
        if (count > 0) {
            self->methods->setCharAt(self, self->cursorIndex, count, 1);
        } else {
            self->charIndex = self->charCount;
        }
    }
}

void TextEntry__ToggleAltCommands(TextEntry *self)
{
    if (self->panelSprite) {
        self->altCommands ^= 1;
    }
}

void TextEntry__ResetChar(TextEntry *self)
{
    if (self->panelSprite) {
        self->charIndex = 0;
        self->methods->setCharAt(self, self->cursorIndex, 0, 1);
    }
}

void TextEntry__ResetAllChars(TextEntry *self)
{
    s32 i;

    if (self->panelSprite) {
        self->charIndex = 0;
        i = self->textLen - 1;
        if (i >= 0) {
            do {
                self->cursorIndex = i;
                self->methods->setCharAt(self, i, self->charIndex, 0);
                i--;
            } while (i >= 0);
        }
        self->methods->setCursorPos(self, self->cursorIndex, 1);
    }
}

extern s32 D_8008AADC; /* VALUE-of here: the cursor's x at position 0 (class_3bb8c_i takes its address) */
extern s32 D_8008AAE0; /* VALUE-of, TextEntry__SetCursorPos only: the cursor's y */

void TextEntry__SetCursorPos(TextEntry *self, s32 pos, s32 notify)
{
    ScreenSpritePos local;
    CharSprite *obj;

    if (self->panelSprite) {
        local.y = D_8008AAE0;
        local.x = pos * 7 + D_8008AADC;
        obj = self->cursorSprite;
        obj->methods->setPosition(obj, &local);
        self->cursorIndex = pos;
        if (notify) {
            self->methods->notifyTarget(self, 0);
        }
    }
}

/* VALUE-of `%gp_rel`, round 45's TextEntry__SetCharAt only -- a byte lookup table
 * (ROM image initialises it to D_800115D0, still-uncarved rodata). */
extern u8 *gNameCharTable;

void TextEntry__SetCharAt(TextEntry *self, s32 pos, s32 charIndex, s32 notify)
{
    ChildObj86ED0 *obj;

    if (self->panelSprite) {
        self->editBuf[pos] = gNameCharTable[charIndex];
        obj = self->textRow;
        obj->methods->slotC4(obj, gNameCharTable[charIndex], pos);
        self->cursorIndex = pos;
        self->charIndex = charIndex;
        if (notify) {
            self->methods->notifyTarget(self, 0);
        }
    }
}

/*
 * Class86F88_3bb8c_j -- a small BasicClass-derived sibling class, LOCAL to this
 * unit (see the file header comment for why this is not added to the
 * shared class_3bb8c.h). Alloc size 0x54 (New_Class86F88). Its real
 * vtable is gClass86F88Methods, reached through GetClass86F88Methods() (class_3bb8c_k).
 * `GetTextEntryMethods` immediately below is UNRELATED to this class -- it
 * is TextEntry's own getter (include/TextEntry.h), merely defined in this
 * same file.
 *
 * unk34/unk38 are single-slot caches for the most recently added child of
 * two distinguished "tag" kinds (established from Class86F88__AddChild/
 * Class86F88__RemoveChild: a child object's own `*(s32*)(*(void**)child) & 0xF`
 * selects unk34 for tag 2, unk38 for tag 5), layered on top of the
 * INHERITED BasicClass generic children list (added/removed via
 * Get_vtable_BasicClass()'s addChild/removeChild in the same two functions).
 */
typedef struct Class86F88Methods_3bb8c_j Class86F88Methods_3bb8c_j;
typedef struct Class86F88_3bb8c_j Class86F88_3bb8c_j;

/*
 * Class86F88_3bb8c_j's own opaque "handle" object (self->unk50's pointee, a
 * ScreenSprite built by Class86F88__LoadResources from a TimImage loaded
 * with New_TimImage from a "CARD\\<name>.TIM" path, as in class_3bb8c_i's
 * TextEntry__LoadCardResources). Only the slots this unit's own functions
 * dispatch through are named. The TimImage handles themselves used this
 * view until round 88 and are `TimImage *` now (include/TimImage.h), so
 * slot78 has no accessor; slot8C's third argument is still typed with it.
 */
typedef struct Class86F88Handle_3bb8c_j Class86F88Handle_3bb8c_j;
typedef struct Class86F88HandleMethods_3bb8c_j Class86F88HandleMethods_3bb8c_j;
struct Class86F88HandleMethods_3bb8c_j {
    u8 pad000[0x004];
    void *(*slot4)(Class86F88Handle_3bb8c_j *self);                          /* +0x004, Class86F88__ReleaseResources */
    u8 pad008[0x04C - 0x008];
    void *(*slot4C)(Class86F88Handle_3bb8c_j *self, void *arg1, void *arg2); /* +0x04C, Class86F88__LoadResources */
    u8 pad050[0x078 - 0x050];
    void (*slot78)(Class86F88Handle_3bb8c_j *self);                          /* +0x078; no accessor since round 88 */
};
struct Class86F88Handle_3bb8c_j {
    Class86F88HandleMethods_3bb8c_j *methods; /* +0x000 */
};

/*
 * Class86F88_3bb8c_j's own vtable. `ctor` at +0x008 is the standard New_X
 * constructor slot -- GetClass86F88Methods() (a real function, defined in the
 * sibling unit class_3bb8c_k, still INCLUDE_ASM there) returns THIS EXACT
 * pointer type: New_Class86F88 calls `GetClass86F88Methods()->ctor(...)` to reach
 * it, and Class86F88__Class86F88 (that very ctor occupant) separately does
 * `self->methods = GetClass86F88Methods();` -- both compile against the same
 * declared return type, which is why this is not split into a separate
 * "ctor table" type the way class_3bb8c_c's BaseCtorTable_3bb8c_c was for
 * an unrelated base class (that view is gone: it was Viewport's table,
 * include/Viewport.h, round 85).
 */
struct Class86F88Methods_3bb8c_j {
    u8 pad000[0x008];
    void (*ctor)(Class86F88_3bb8c_j *self, void *arg0, s32 arg1); /* +0x008, Class86F88__Class86F88 (occupant); New_Class86F88's call site */
    u8 pad00C[0x010 - 0x00C];
    /* Called by Class86F88__AddChildAndSetState at two arities: (self,arg1,arg2,arg3) at its
     * first call site (a straight passthrough of that function's own
     * params) and (self,arg2) at its second -- see that function's report. */
    void (*slot10)(Class86F88_3bb8c_j *self, void *arg1, s32 arg2, s32 arg3); /* +0x010, Class86F88__AddChildAndSetState */
    void (*slot14)(Class86F88_3bb8c_j *self, void *arg1);          /* +0x014, Class86F88__RemoveCachedChildren */
    u8 pad018[0x040 - 0x018];
    void (*slot40)(Class86F88_3bb8c_j *self);                        /* +0x040, Class86F88__Class86F88 tail */
    u8 pad044[0x058 - 0x044];
    void (*slot58)(Class86F88_3bb8c_j *self, void *arg1, s32 arg2);    /* +0x058, Class86F88__NotifyChild (tag==5) */
    void (*slot5C)(Class86F88_3bb8c_j *self, void *arg1, s32 arg2);     /* +0x05C, Class86F88__NotifyChild (tag==2) */
    u8 pad060[0x08C - 0x060];
    void (*slot8C)(Class86F88_3bb8c_j *self, void *arg1, Class86F88Handle_3bb8c_j *arg2, s32 arg3, s32 arg4, s32 arg5); /* +0x08C, Class86F88__LoadResources */
    void (*slot90)(Class86F88_3bb8c_j *self);                          /* +0x090, Class86F88__ReleaseResources */
};

struct Class86F88_3bb8c_j {
    Class86F88Methods_3bb8c_j *methods;    /* +0x000 */
    u8 pad04[0x0C - 0x04];
    s32 unkC;                       /* +0x00C, Class86F88__Class86F88: set to its own arg2 (a mode: 0 or 1) */
    s32 unk10;                       /* +0x010, Class86F88__Finalize/Class86F88__Class86F88: element count for unk18[]/unk1C[] */
    s32 unk14;                        /* +0x014, Class86F88__Class86F88: a running MAX over the per-entry lengths computed in its fill loop */
    void **unk18;                      /* +0x018, Class86F88__Finalize/Class86F88__Class86F88: array of unk10 individually-allocated buffers */
    s32 *unk1C;                         /* +0x01C, Class86F88__Finalize (freed as one block)/Class86F88__Class86F88 (array of unk10 per-entry lengths) */
    s32 unk20;                           /* +0x020, Class86F88__ResetCounters */
    s32 unk24;                            /* +0x024, Class86F88__ResetCounters */
    s32 unk28;                             /* +0x028, Class86F88__ResetCounters */
    s32 unk2C;                               /* +0x02C, Class86F88__AddChildAndSetState: cleared to 0 */
    u8 pad30[0x34 - 0x30];
    void *unk34;                            /* +0x034, "tag==2" registered-child cache */
    void *unk38;                             /* +0x038, "tag==5" registered-child cache */
    s32 unk3C;                                /* +0x03C, Class86F88__RemoveCachedChildren (cleared)/Class86F88__AddChildAndSetState (set from its own arg3) */
    u8 pad40[0x50 - 0x40];
    Class86F88Handle_3bb8c_j *unk50;                  /* +0x050 */
};

/* TextEntry's own table getter (include/TextEntry.h), defined here in ROM
 * order; not Class86F88's. */
TextEntryMethods *GetTextEntryMethods(void)
{
    return &gTextEntryMethods;
}

/*
 * New_Class86F88. BMemPMgrAlloc/BMemPMgrFree already declared for the
 * Obj866E8 group above are the same generic pool allocator/free pair --
 * not redeclared here.
 *
 * BasicClass's method table and its getter are include/BasicClass.h's
 * (through class_3bb8c.h); the base-class calls below upcast `self`.
 */
extern void *BMemPMgrAlloc(s32 size);
extern void *BMemPMgrFree(void *ptr);

extern Class86F88Methods_3bb8c_j *GetClass86F88Methods(void);

void *New_Class86F88(void *arg0, s32 arg1)
{
    Class86F88_3bb8c_j *self = BMemPMgrAlloc(0x54);

    if (self == NULL) {
        goto fail;
    }
    GetClass86F88Methods()->ctor(self, arg0, arg1);
    return self;
fail:
    return NULL;
}

/*
 * Class86F88_3bb8c_j's own ctor (the GetClass86F88Methods()->ctor occupant, matched via
 * its own address -- classtable-style resolution doesn't apply here since
 * this vtable is a LOCAL, no `tools/classtable.py` slot list exists for
 * it; the identity comes from New_Class86F88's call site + this function's
 * own signature matching it exactly).
 *
 * arg1 is a NUL-terminated array of string pointers; arg2 is a mode flag
 * (0 or 1) also stashed into self->unkC. First pass counts entries; then
 * allocates two parallel self->unk10-length arrays (self->unk18: one
 * individually-allocated buffer per entry; self->unk1C: one s32 length
 * per entry, computed by strlen -- halved when arg2==1). Each buffer is
 * filled either via DecodeFullWidthSjis (arg2==1) or strcpy (otherwise),
 * and self->unk14 tracks the running max of the computed lengths. The max
 * is a ternary, not an `if`: retail stores the old value back
 * unconditionally before the conditional store of len.
 */
extern s32 strlen(void *arg0);
extern void DecodeFullWidthSjis(void *dst, void *src);
extern char *strcpy(char *dest, char *src);

void Class86F88__Class86F88(Class86F88_3bb8c_j *self, void **arg1, s32 arg2)
{
    void **p;
    s32 i;
    s32 len;

    i = 0;
    p = arg1;
    Get_vtable_BasicClass()->ctor((BasicClass *)self);
    self->methods = GetClass86F88Methods();

    while (*p++ != NULL) {
        i++;
    }

    self->unk10 = i;
    self->unk18 = BMemPMgrAlloc(i * 4);
    p = arg1;
    self->unk1C = BMemPMgrAlloc(self->unk10 * 4);
    self->unk14 = 0;

    for (i = 0; i < self->unk10; i++) {
        len = strlen(*p);
        if (arg2 == 1) {
            len /= 2;
        }
        self->unk1C[i] = len;
        self->unk18[i] = BMemPMgrAlloc(len + 4);
        if (arg2 == 1) {
            DecodeFullWidthSjis(self->unk18[i], *p);
        } else {
            strcpy(self->unk18[i], *p);
        }
        self->unk14 = (self->unk14 < len) ? len : self->unk14;
        p++;
    }

    self->unkC = arg2;
    Class86F88__ClearCachedRefs(self);
    self->methods->slot40(self);
}

void Class86F88__ClearCachedRefs(Class86F88_3bb8c_j *self)
{
    self->unk34 = NULL;
    self->unk38 = NULL;
    self->unk50 = NULL;
}

void Class86F88__Finalize(Class86F88_3bb8c_j *self)
{
    s32 i;

    for (i = 0; i < self->unk10; i++) {
        BMemPMgrFree(self->unk18[i]);
    }
    BMemPMgrFree(self->unk1C);
    BMemPMgrFree(self->unk18);
    Get_vtable_BasicClass()->finalize((BasicClass *)self);
}

void Class86F88__AddChild(Class86F88_3bb8c_j *self, void *arg1)
{
    s32 tag;

    if (arg1) {
        Get_vtable_BasicClass()->addChild((BasicClass *)self, (BasicClass *)arg1);
        tag = **(s32 **)arg1 & 0xF;
        if (tag == 2) {
            self->unk34 = arg1;
        } else if (tag == 5) {
            self->unk38 = arg1;
        }
    }
}

void Class86F88__RemoveChild(Class86F88_3bb8c_j *self, void *arg1)
{
    s32 tag;

    if (arg1) {
        tag = **(s32 **)arg1 & 0xF;
        if (tag == 2) {
            self->unk34 = NULL;
        } else if (tag == 5) {
            self->unk38 = NULL;
        }
        Get_vtable_BasicClass()->removeChild((BasicClass *)self, (BasicClass *)arg1);
    }
}

void Class86F88__RemoveAllChildren(Class86F88_3bb8c_j *self)
{
    self->unk34 = NULL;
    self->unk38 = NULL;
    self->unk50 = NULL;
    Get_vtable_BasicClass()->removeAllChildren((BasicClass *)self);
}

void Class86F88__NotifyChild(Class86F88_3bb8c_j *self, void *arg1, s32 arg2)
{
    s32 tag;

    Get_vtable_BasicClass()->onNotify((BasicClass *)self, arg1, arg2);
    tag = **(s32 **)arg1 & 0xF;
    if (tag == 2) {
        self->methods->slot5C(self, arg1, arg2);
    } else if (tag == 5) {
        self->methods->slot58(self, arg1, arg2);
    }
}

void Class86F88__ResetCounters(Class86F88_3bb8c_j *self)
{
    self->unk20 = 0;
    self->unk24 = 0;
    self->unk28 = 0;
}

extern char *BuildFileName(char *dest, const char *arg1, const char *arg2, const char *arg3);
extern const char D_8008AB14[]; /* "SELECT" */
extern const char D_8008AB1C[]; /* "CARD\\" */
extern const char D_8008AB24[]; /* ".TIM" */
extern s32 D_80087028; /* 3 words, New_ScreenSprite's rect: a SpriteRect {0, 0, 256, 160} */
extern s32 D_8008AAF8;
extern const char D_800116E4[]; /* "FONTICON" */

/*
 * Two handle variables, not one: handle1 and handle2 are disjoint live
 * ranges, and merging them into one `h` gives the rotation filed as the
 * round-18/19 stall (75/95, both addresses and the handle swapped among
 * $s0-$s2). Same shape as class_3bb8c_i's TextEntry__LoadCardResources.
 */
void Class86F88__LoadResources(Class86F88_3bb8c_j *self, void *arg1)
{
    char path[0x20];
    const char *dir;
    const char *ext;
    TimImage *handle1;
    TimImage *handle2;

    if (arg1 == NULL) {
        return;
    }
    if (self->unk50 != NULL) {
        return;
    }

    dir = D_8008AB1C;
    ext = D_8008AB24;

    handle1 = New_TimImage(BuildFileName(path, D_8008AB14, dir, ext));
    ((TimImageUploadFn)handle1->methods->slot78)(handle1);
    self->unk50 = (Class86F88Handle_3bb8c_j *)New_ScreenSprite(handle1, (SpriteRect *)&D_80087028, 0);
    handle1->methods->release(handle1);
    self->unk50->methods->slot4C(self->unk50, arg1, &D_8008AAF8);

    handle2 = New_TimImage(BuildFileName(path, D_800116E4, dir, ext));
    ((TimImageUploadFn)handle2->methods->slot78)(handle2);
    self->methods->slot8C(self, arg1, (Class86F88Handle_3bb8c_j *)handle2, self->unk20, self->unk24, self->unk28);
    handle2->methods->release(handle2);
}

void Class86F88__ReleaseResources(Class86F88_3bb8c_j *self)
{
    if (self->unk50) {
        self->methods->slot90(self);
        self->unk50 = self->unk50->methods->slot4(self->unk50);
    }
}

void Class86F88__AddChildAndSetState(Class86F88_3bb8c_j *self, void *arg1, s32 arg2, s32 arg3)
{
    typedef void (*Slot10NarrowFn)(Class86F88_3bb8c_j *self, s32 arg1);
    void (*fn)(Class86F88_3bb8c_j *self, void *arg1, s32 arg2, s32 arg3);
    s32 zero;

    zero = 0;
    fn = self->methods->slot10;
    do {
        fn(self, arg1, arg2, arg3);
        ((Slot10NarrowFn)self->methods->slot10)(self, arg2);
        self->unk3C = arg3;
        self->unk2C = zero;
    } while (0);
}

void Class86F88__RemoveCachedChildren(Class86F88_3bb8c_j *self)
{
    self->methods->slot14(self, self->unk34);
    self->methods->slot14(self, self->unk38);
    self->unk3C = 0;
}
