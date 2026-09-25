/*
 * class_3bb8c_i -- third carved slice of the class_3bb8c block, 20 functions,
 * carved round 14. include/class_3bb8c.h is SHARED with every other
 * class_3bb8c_* slice; header edits here must be strictly ADDITIVE.
 *
 * All 20 functions are `Obj86ED0` methods (vtable `gObj86ED0Methods`,
 * `D_80086ED0`, 42 slots, `tools/classtable.py gObj86ED0Methods`) -- the
 * ONLY class this unit defines methods for. `Obj86ED0` is a BasicClass
 * subclass that resolves and drives the memory-card save-name-entry UI: it
 * loads the `CARD\COMINPUT.TIM`/`CARD\FONTICON.TIM` icon/font resources
 * (`Obj86ED0__LoadCardResources`), holds both a caller-owned name buffer
 * (`nameBuf`) and its own half-width working copy (`unk28`, decoded/encoded
 * via `DecodeFullWidthSjis`/`EncodeFullWidthSjis`), and routes a dense
 * numeric command switch (`Obj86ED0__HandleCommand`) to cursor-move
 * (`MoveCursorRight`/`Left`), character-select-cycle (`AdvanceCharSelect`)
 * and the countdown/blink group class_3bb8c_j already named
 * (`AdvanceCountdown`/`ToggleFlag20`/`ResetCountdown`/`ResetAllAndFinish`).
 * Four vtable slots (`advanceCountdown`/`toggleFlag20`/`resetCountdown`/
 * `resetAllAndFinish`) resolve to class_3bb8c_j's own functions but are
 * referenced ONLY here, so they are named in the shared header as this
 * unit's own (classtable.py-verified, compiler-checked clean). See
 * `docs/match-reports/Obj86ED0__HandleCommand.md` for the remaining
 * cross-unit fields/slots this unit could not rename alone.
 */
#include "common.h"
#include "class_3bb8c.h"

/* This project's own strcpy (matched elsewhere) -- Obj86ED0__SetName's own
 * caller, same local-declaration convention as class_3bb8c_e.c/others. */
extern char *strcpy(char *dest, char *src);

/* Uncarved helper, `code_2cc8c_f`, still INCLUDE_ASM -- Obj86ED0__SetName's own
 * call. Translates each byte of `src` (a name string) into `dest` (folding a
 * couple of special-case byte ranges) and returns `dest`, same convention as
 * `strcpy`. Typed purely from this call site's own register usage. Declared
 * HERE, not in include/class_3bb8c.h: src/class_3bb8c_j.c types the same
 * (still undefined) function as `void (void *, void *)` from its own call
 * site, and two call-site typings of one function cannot share a header. */
extern char *DecodeFullWidthSjis(char *dest, char *src);

/* This class's method table. Declared HERE and not in include/class_3bb8c.h
 * because src/class_3bb8c_j.c declares the same object as its own
 * `Class86ED0Methods` local view, and two incompatible declarations of one
 * symbol in a shared header reach both translation units. See the HEAD NOTE
 * next to Obj86ED0Methods in that header. */
extern Obj86ED0Methods gObj86ED0Methods;

/* This class's own table getter -- New_Obj86ED0/Obj86ED0__Obj86ED0's shared
 * dispatch. DEFINED in src/class_3bb8c_j.c (matched round 15 by runner
 * bravo, which returns it as its own `Class86ED0Methods *` local view of
 * the same table). Returns `&gObj86ED0Methods`; confirmed in the disassembly as
 * `lui/addiu` materialising that exact address then `jr $ra`, the same
 * no-argument-getter shape as `Get_vtable_BasicClass`. Declared here rather than
 * in the shared header because the two units' return types differ. */
extern Obj86ED0Methods *Get_vtable_Obj86ED0(void);

void *New_Obj86ED0(s32 arg0, s32 arg1)
{
    Obj86ED0 *self;

    self = BMemPMgrAlloc(0x4C);
    if (self != NULL) {
        Get_vtable_Obj86ED0()->ctor(self, arg0, arg1);
        return self;
    }
    return NULL;
}

/* Sony's, from libc2 (already declared above via class_3bb8c_j's own
 * convention -- but not yet in this unit; local view). */
extern s32 strlen(char *s);

/* VALUE-of `%gp_rel`, round 45's own local view -- same global as
 * class_3bb8c_j's `gNameCharTable` (a byte lookup table whose length this
 * function counts by hand rather than via `strlen`, since GCC 2.6.3 with
 * `-fno-builtin` never turns a `strlen` CALL into inline code -- the
 * inline loop below has to be literal source, not a call). */
extern u8 *gNameCharTable;

/* Defined later in this file (ROM order); forward-declared here since
 * Obj86ED0__Obj86ED0 calls it, same convention as Obj865C8__EnterState2 in
 * src/class_39e08.c. */
extern void Obj86ED0__ClearChildRefs(Obj86ED0 *self);

void Obj86ED0__Obj86ED0(Obj86ED0 *self, char *arg1, s32 arg2)
{
    u8 *p;
    s32 count;

    Get_vtable_BasicClass()->ctor((BasicClass *)self);
    self->methods = Get_vtable_Obj86ED0();
    self->nameLen = strlen(arg1);
    self->unk28 = BMemPMgrAlloc(self->nameLen + 4);

    p = gNameCharTable;
    count = 0;
    while (*p != 0) {
        p++;
        count++;
    }
    self->unk14 = count;

    Obj86ED0__ClearChildRefs(self);
    self->methods->setName(self, arg1, arg2);
}

void Obj86ED0__ClearChildRefs(Obj86ED0 *self)
{
    self->childType2 = NULL;
    self->childType5 = NULL;
    self->unk48 = NULL;
}

void Obj86ED0__Finalize(Obj86ED0 *self)
{
    BMemPMgrFree(self->unk28);
    Get_vtable_BasicClass()->finalize((BasicClass *)self);
}

void Obj86ED0__AddChild(Obj86ED0 *self, void *arg1)
{
    s32 tag;
    s32 mask;

    if (arg1 != NULL) {
        Get_vtable_BasicClass()->addChild((BasicClass *)self, (BasicClass *)arg1);
        tag = **(s32 **)arg1;
        mask = tag & 0xF;
        if (mask == 2) {
            self->childType2 = arg1;
        } else if (mask == 5) {
            self->childType5 = arg1;
        }
    }
}

void Obj86ED0__RemoveChild(Obj86ED0 *self, void *arg1)
{
    s32 tag;
    s32 mask;

    if (arg1 != NULL) {
        tag = **(s32 **)arg1;
        mask = tag & 0xF;
        if (mask == 2) {
            self->childType2 = NULL;
        } else if (mask == 5) {
            self->childType5 = NULL;
        }
        Get_vtable_BasicClass()->removeChild((BasicClass *)self, (BasicClass *)arg1);
    }
}

void Obj86ED0__RemoveAllChildren(Obj86ED0 *self)
{
    self->childType2 = NULL;
    self->childType5 = NULL;
    self->unk48 = NULL;
    Get_vtable_BasicClass()->removeAllChildren((BasicClass *)self);
}

void Obj86ED0__Notify(Obj86ED0 *self, void *arg1, s32 arg2)
{
    s32 tag;
    s32 mask;

    Get_vtable_BasicClass()->onNotify((BasicClass *)self, arg1, arg2);

    tag = **(s32 **)arg1;
    mask = tag & 0xF;
    if (mask == 2) {
        self->methods->handleCommand(self, arg1, arg2);
    } else if (mask == 5) {
        self->methods->tickState(self, arg1, arg2);
    }
}

void Obj86ED0__SetName(Obj86ED0 *self, char *arg1, s32 mode)
{
    self->mode = mode;
    self->nameBuf = arg1;
    self->cursorIndex = 0;
    self->unk1C = 0;
    if (mode == 1) {
        DecodeFullWidthSjis(self->unk28, arg1);
        self->nameLen /= 2;
    } else {
        strcpy(self->unk28, arg1);
    }
}

/*
 * Obj86ED0__LoadCardResources's own helpers/data -- resolves two "CARD\\<name>.TIM"
 * memory-card icon/font resource paths (BuildFileName, already matched in
 * code_171e0.c) and loads each through func_8003B39C, then converts/wraps
 * the loaded handle into a ChildObj86ED0-shaped resource object (unk48 via
 * func_80041C9C, unk44/unk40 via New_Obj6EAC0/New_D8006EC74 -- both still
 * uncarved elsewhere, typed purely from this call site's own register
 * usage, same convention as DecodeFullWidthSjis above).
 */
extern char *BuildFileName(char *dest, char *arg1, char *arg2, char *arg3);
extern ChildObj86ED0 *func_8003B39C(char *path);
extern ChildObj86ED0 *func_80041C9C(ChildObj86ED0 *arg0, void *arg1, s32 arg2);
extern ChildObj86ED0 *New_D8006EC74(ChildObj86ED0 *arg0, s32 arg1);

extern const char sStrComInput[]; /* "COMINPUT" */
extern const char sStrFontIcon[]; /* "FONTICON" */
extern const char sCardPathPrefix[]; /* "CARD\\" */
extern const char sTimExt[]; /* ".TIM" */
extern s32 D_80086F7C; /* 3-word opaque block, func_80041C9C's arg1, address-only here */
extern s32 D_8008AAC8; /* opaque block, slotB8's arg1, address-only here */
extern s32 D_8008AACC; /* opaque block, self->unk48's slot4C arg2, address-only here */
extern s32 D_8008AAD4; /* opaque block, self->unk44's slot4C arg2, address-only here */
extern s32 D_8008AADC; /* opaque block, self->unk40's slot4C arg2, address-only here */

void Obj86ED0__LoadCardResources(Obj86ED0 *self, void *arg1)
{
    char path[0x20];
    const char *dir;
    const char *ext;
    ChildObj86ED0 *handle1;
    ChildObj86ED0 *handle2;

    if (arg1 == NULL) {
        return;
    }
    if (self->unk48 != NULL) {
        return;
    }

    dir = sCardPathPrefix;
    ext = sTimExt;

    handle1 = func_8003B39C(BuildFileName(path, sStrComInput, dir, ext));
    handle1->methods->slot78(handle1);
    self->unk48 = func_80041C9C(handle1, (void *)&D_80086F7C, 0);
    handle1->methods->release(handle1);
    self->unk48->methods->slot4C(self->unk48, arg1, (void *)&D_8008AACC);

    handle2 = func_8003B39C(BuildFileName(path, sStrFontIcon, dir, ext));
    handle2->methods->slot78(handle2);
    self->unk44 = New_Obj6EAC0(handle2, self->nameLen, self->unk28);
    self->unk40 = New_D8006EC74(handle2, 0x5F);
    handle2->methods->release(handle2);
    self->unk44->methods->slot4C(self->unk44, arg1, (void *)&D_8008AAD4);
    self->unk44->methods->slotB8(self->unk44, (void *)&D_8008AAC8);
    self->unk40->methods->slot4C(self->unk40, arg1, (void *)&D_8008AADC);
}

void Obj86ED0__ReleaseCardResources(Obj86ED0 *self)
{
    if (self->unk48 != NULL) {
        self->unk48 = self->unk48->methods->release(self->unk48);
        self->unk44->methods->release(self->unk44);
        self->unk40->methods->release(self->unk40);
    }
}

void Obj86ED0__AttachTarget(Obj86ED0 *self, void *arg1, void *arg2, TargetObj86ED0 *arg3)
{
    self->methods->addChild(self, arg1);
    self->methods->addChild(self, arg2);
    self->target = arg3;
    self->closeState = 0;
    self->unk20 = 0;
}

void Obj86ED0__DetachTarget(Obj86ED0 *self)
{
    self->methods->removeChild(self, self->childType2);
    self->methods->removeChild(self, self->childType5);
    self->target = NULL;
}

void Obj86ED0__SetState(Obj86ED0 *self, s32 arg1)
{
    self->closeTickCount = 0;
    if (arg1 < 2) {
        return;
    }
    switch (arg1) {
    case 2:
    case 3:
        self->methods->removeChild(self, self->childType2);
        self->methods->releaseCardResources(self);
        self->closeState = arg1;
        break;
    case 4:
        self->methods->notifyParents(self, self->closeState);
        break;
    }
}

void Obj86ED0__TickState(Obj86ED0 *self)
{
    s32 tag;
    s32 old;

    tag = self->closeState;
    if (tag >= 4) {
        return;
    }
    if (tag < 2) {
        return;
    }
    old = self->closeTickCount;
    self->closeTickCount = old + 1;
    if (old != 0) {
        self->methods->setState(self, 4);
    }
}

/* Obj86ED0__HandleCommand's own name-copy helper -- uncarved elsewhere (`code_2cc8c_f`,
 * still `INCLUDE_ASM`), typed purely from this call site's own register
 * usage: `a0`/`a1` are `self->nameBuf`/`self->unk28` (both `char *`, the same
 * pair `strcpy` is fed in the other arm), return value unused. Same
 * declare-locally convention as `DecodeFullWidthSjis` above (a different unit
 * types this same-shaped function with a different signature from its own
 * call site). */
extern void EncodeFullWidthSjis(char *dest, char *src);

void Obj86ED0__HandleCommand(Obj86ED0 *self, void *arg1, s32 arg2)
{
    switch (arg2) {
    default:
        return;
    case 25:
        if (self->mode == 1) {
            EncodeFullWidthSjis(self->nameBuf, self->unk28);
        } else {
            strcpy(self->nameBuf, self->unk28);
        }
        self->methods->slot60(self, 0x10);
        self->methods->setState(self, 2);
        return;
    case 23:
        self->methods->slot60(self, 0x10);
        self->methods->setState(self, 3);
        return;
    case 32:
        self->methods->resetAllAndFinish(self);
        return;
    case 31:
        self->methods->resetCountdown(self);
        return;
    case 28:
        self->methods->toggleFlag20(self);
        return;
    case 21:
        if (self->unk20 != 0) {
            return;
        }
        self->methods->moveCursorRight(self);
        return;
    case 5:
        if (self->unk20 == 0) {
            return;
        }
        self->methods->moveCursorRight(self);
        return;
    case 20:
        if (self->unk20 != 0) {
            return;
        }
        self->methods->moveCursorLeft(self);
        return;
    case 4:
        if (self->unk20 == 0) {
            return;
        }
        self->methods->moveCursorLeft(self);
        return;
    case 18:
        if (self->unk20 != 0) {
            return;
        }
        self->methods->advanceCharSelect(self);
        return;
    case 2:
        if (self->unk20 == 0) {
            return;
        }
        self->methods->advanceCharSelect(self);
        return;
    case 19:
        if (self->unk20 == 0) {
            goto slot94Call;
        }
        return;
    case 3:
        if (self->unk20 == 0) {
            return;
        }
slot94Call:
        self->methods->advanceCountdown(self);
        return;
    }
}

void Obj86ED0__NotifyTarget(Obj86ED0 *self, s32 arg1)
{
    TargetObj86ED0 *target;

    target = self->target;
    if (target != NULL) {
        target->methods->slot80(target, arg1, 0x60, 0x60);
    }
}

void Obj86ED0__MoveCursorRight(Obj86ED0 *self)
{
    s32 old;
    s32 v;

    if (self->unk48 != NULL) {
        old = self->cursorIndex;
        v = old + 1;
        self->cursorIndex = v;
        if (v < self->nameLen) {
            self->methods->slotA4(self, v, 1);
        } else {
            self->cursorIndex = old;
        }
    }
}

void Obj86ED0__MoveCursorLeft(Obj86ED0 *self)
{
    s32 old;
    s32 v;

    if (self->unk48 != NULL) {
        old = self->cursorIndex;
        v = old - 1;
        self->cursorIndex = v;
        if (v >= 0) {
            self->methods->slotA4(self, v, 1);
        } else {
            self->cursorIndex = old;
        }
    }
}

void Obj86ED0__AdvanceCharSelect(Obj86ED0 *self)
{
    s32 v;

    if (self->unk48 != NULL) {
        v = self->unk1C + 1;
        self->unk1C = v;
        if (v < self->unk14) {
            self->methods->slotA8(self, self->cursorIndex, v, 1);
        } else {
            self->unk1C = 0;
        }
    }
}
