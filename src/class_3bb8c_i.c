/*
 * class_3bb8c_i -- third carved slice of the class_3bb8c block, 20 functions,
 * carved round 14. All 20 are TextEntry methods (gTextEntryMethods,
 * `D_80086ED0`, 42 slots; `tools/classtable.py gTextEntryMethods`), declared
 * in include/TextEntry.h, whose banner has the evidence for the class name.
 * The other seven own methods (PrevChar .. SetCharAt, GetTextEntryMethods)
 * are class_3bb8c_j's. TextEntry edits a caller-owned string: it loads the
 * `CARD\COMINPUT.TIM`/`CARD\FONTICON.TIM` panel, text row and cursor
 * (LoadCardResources), keeps the caller's buffer (`textBuf`) and its own
 * working copy (`editBuf`, decoded/encoded with DecodeFullWidthSjis/
 * EncodeFullWidthSjis in mode 1), and routes a numeric command switch
 * (HandleCommand) to cursor moves, character stepping, commit (25) and
 * cancel (23).
 */
#include "common.h"
#include "class_3bb8c.h"
#include "TextEntry.h"
#include "CharSprite.h"
#include "TextRow.h"
#include "TimImage.h"

/* This project's own strcpy (matched elsewhere) -- TextEntry__SetText's own
 * caller, same local-declaration convention as class_3bb8c_e.c/others. */
extern char *strcpy(char *dest, char *src);

/* Uncarved helper, `code_2cc8c_f`, still INCLUDE_ASM -- TextEntry__SetText's own
 * call. Translates each byte of `src` (a name string) into `dest` (folding a
 * couple of special-case byte ranges) and returns `dest`, same convention as
 * `strcpy`. Typed purely from this call site's own register usage. Declared
 * HERE, not in include/class_3bb8c.h: src/class_3bb8c_j.c types the same
 * (still undefined) function as `void (void *, void *)` from its own call
 * site, and two call-site typings of one function cannot share a header. */
extern char *DecodeFullWidthSjis(char *dest, char *src);

TextEntry *New_TextEntry(char *text, s32 mode) {
    TextEntry *self;

    self = BMemPMgrAlloc(0x4C);
    if (self != NULL) {
        GetTextEntryMethods()->ctor(self, text, mode);
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

void TextEntry__TextEntry(TextEntry *self, char *arg1, s32 arg2) {
    u8 *p;
    s32 count;

    Get_vtable_BasicClass()->ctor((BasicClass *)self);
    self->methods = GetTextEntryMethods();
    self->textLen = strlen(arg1);
    self->editBuf = BMemPMgrAlloc(self->textLen + 4);

    p = gNameCharTable;
    count = 0;
    while (*p != 0) {
        p++;
        count++;
    }
    self->charCount = count;

    TextEntry__ClearChildRefs(self);
    self->methods->setText(self, arg1, arg2);
}

void TextEntry__ClearChildRefs(TextEntry *self) {
    self->childType2 = NULL;
    self->childType5 = NULL;
    self->panelSprite = NULL;
}

void TextEntry__Finalize(TextEntry *self) {
    BMemPMgrFree(self->editBuf);
    Get_vtable_BasicClass()->finalize((BasicClass *)self);
}

void TextEntry__AddChild(TextEntry *self, void *arg1) {
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

void TextEntry__RemoveChild(TextEntry *self, void *arg1) {
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

void TextEntry__RemoveAllChildren(TextEntry *self) {
    self->childType2 = NULL;
    self->childType5 = NULL;
    self->panelSprite = NULL;
    Get_vtable_BasicClass()->removeAllChildren((BasicClass *)self);
}

void TextEntry__OnNotify(TextEntry *self, void *arg1, s32 arg2) {
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

void TextEntry__SetText(TextEntry *self, char *arg1, s32 mode) {
    self->mode = mode;
    self->textBuf = arg1;
    self->cursorIndex = 0;
    self->charIndex = 0;
    if (mode == 1) {
        DecodeFullWidthSjis(self->editBuf, arg1);
        self->textLen /= 2;
    } else {
        strcpy(self->editBuf, arg1);
    }
}

/*
 * TextEntry__LoadCardResources's own helpers/data -- builds two "CARD\\<name>.TIM"
 * paths (BuildFileName, code_171e0.c), loads each through New_TimImage, and
 * makes panelSprite (New_ScreenSprite), textRow (New_TextRow) and
 * cursorSprite (New_CharSprite) from the loaded handles.
 */
extern char *BuildFileName(char *dest, char *arg1, char *arg2, char *arg3);

extern const char sStrComInput[];    /* "COMINPUT" */
extern const char sStrFontIcon[];    /* "FONTICON" */
extern const char sCardPathPrefix[]; /* "CARD\\" */
extern const char sTimExt[];         /* ".TIM" */
extern s32 D_80086F7C; /* 3 words, New_ScreenSprite's rect: a SpriteRect {0, 0, 224, 120} */
extern s32 D_8008AAC8; /* opaque block, slotB8's arg1, address-only here */
extern s32 D_8008AACC; /* panelSprite's attachToParent position, address-only here */
extern s32 D_8008AAD4; /* textRow's slot4C position, address-only here */
extern s32 D_8008AADC; /* cursorSprite's attachToParent position; class_3bb8c_j reads its x */

void TextEntry__LoadCardResources(TextEntry *self, void *arg1) {
    char path[0x20];
    const char *dir;
    const char *ext;
    TimImage *handle1;
    TimImage *handle2;

    if (arg1 == NULL) {
        return;
    }
    if (self->panelSprite != NULL) {
        return;
    }

    dir = sCardPathPrefix;
    ext = sTimExt;

    handle1 = New_TimImage(BuildFileName(path, sStrComInput, dir, ext));
    ((TimImageUploadFn)handle1->methods->slot78)(handle1);
    self->panelSprite = New_ScreenSprite(handle1, (SpriteRect *)&D_80086F7C, 0);
    handle1->methods->release(handle1);
    self->panelSprite->methods->attachToParent(self->panelSprite, (Class6B5CC *)arg1,
                                               (Vec3_d294 *)&D_8008AACC);

    handle2 = New_TimImage(BuildFileName(path, sStrFontIcon, dir, ext));
    ((TimImageUploadFn)handle2->methods->slot78)(handle2);
    self->textRow = (ChildObj86ED0 *)New_TextRow(handle2, self->textLen, self->editBuf);
    self->cursorSprite = New_CharSprite(handle2, 0x5F);
    handle2->methods->release(handle2);
    self->textRow->methods->slot4C(self->textRow, arg1, (void *)&D_8008AAD4);
    self->textRow->methods->slotB8(self->textRow, (void *)&D_8008AAC8);
    self->cursorSprite->methods->attachToParent(self->cursorSprite, (Class6B5CC *)arg1,
                                                (Vec3_d294 *)&D_8008AADC);
}

void TextEntry__ReleaseCardResources(TextEntry *self) {
    if (self->panelSprite != NULL) {
        self->panelSprite = self->panelSprite->methods->release(self->panelSprite);
        self->textRow->methods->release(self->textRow);
        self->cursorSprite->methods->release(self->cursorSprite);
    }
}

void TextEntry__AttachTarget(TextEntry *self, void *arg1, void *arg2, TargetObj86ED0 *arg3) {
    self->methods->addChild(self, arg1);
    self->methods->addChild(self, arg2);
    self->target = arg3;
    self->closeState = 0;
    self->altCommands = 0;
}

void TextEntry__DetachTarget(TextEntry *self) {
    self->methods->removeChild(self, self->childType2);
    self->methods->removeChild(self, self->childType5);
    self->target = NULL;
}

void TextEntry__SetState(TextEntry *self, s32 arg1) {
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

void TextEntry__TickState(TextEntry *self) {
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

/* TextEntry__HandleCommand's own name-copy helper -- uncarved elsewhere (`code_2cc8c_f`,
 * still `INCLUDE_ASM`), typed purely from this call site's own register
 * usage: `a0`/`a1` are `self->textBuf`/`self->editBuf` (both `char *`, the same
 * pair `strcpy` is fed in the other arm), return value unused. Same
 * declare-locally convention as `DecodeFullWidthSjis` above (a different unit
 * types this same-shaped function with a different signature from its own
 * call site). */
extern void EncodeFullWidthSjis(char *dest, char *src);

void TextEntry__HandleCommand(TextEntry *self, void *arg1, s32 arg2) {
    switch (arg2) {
        default:
            return;
        case 25:
            if (self->mode == 1) {
                EncodeFullWidthSjis(self->textBuf, self->editBuf);
            } else {
                strcpy(self->textBuf, self->editBuf);
            }
            self->methods->notifyTarget(self, 0x10);
            self->methods->setState(self, 2);
            return;
        case 23:
            self->methods->notifyTarget(self, 0x10);
            self->methods->setState(self, 3);
            return;
        case 32:
            self->methods->resetAllChars(self);
            return;
        case 31:
            self->methods->resetChar(self);
            return;
        case 28:
            self->methods->toggleAltCommands(self);
            return;
        case 21:
            if (self->altCommands != 0) {
                return;
            }
            self->methods->moveCursorRight(self);
            return;
        case 5:
            if (self->altCommands == 0) {
                return;
            }
            self->methods->moveCursorRight(self);
            return;
        case 20:
            if (self->altCommands != 0) {
                return;
            }
            self->methods->moveCursorLeft(self);
            return;
        case 4:
            if (self->altCommands == 0) {
                return;
            }
            self->methods->moveCursorLeft(self);
            return;
        case 18:
            if (self->altCommands != 0) {
                return;
            }
            self->methods->nextChar(self);
            return;
        case 2:
            if (self->altCommands == 0) {
                return;
            }
            self->methods->nextChar(self);
            return;
        case 19:
            if (self->altCommands == 0) {
                goto slot94Call;
            }
            return;
        case 3:
            if (self->altCommands == 0) {
                return;
            }
        slot94Call:
            self->methods->prevChar(self);
            return;
    }
}

void TextEntry__NotifyTarget(TextEntry *self, s32 arg1) {
    TargetObj86ED0 *target;

    target = self->target;
    if (target != NULL) {
        target->methods->slot80(target, arg1, 0x60, 0x60);
    }
}

void TextEntry__MoveCursorRight(TextEntry *self) {
    s32 old;
    s32 v;

    if (self->panelSprite != NULL) {
        old = self->cursorIndex;
        v = old + 1;
        self->cursorIndex = v;
        if (v < self->textLen) {
            self->methods->setCursorPos(self, v, 1);
        } else {
            self->cursorIndex = old;
        }
    }
}

void TextEntry__MoveCursorLeft(TextEntry *self) {
    s32 old;
    s32 v;

    if (self->panelSprite != NULL) {
        old = self->cursorIndex;
        v = old - 1;
        self->cursorIndex = v;
        if (v >= 0) {
            self->methods->setCursorPos(self, v, 1);
        } else {
            self->cursorIndex = old;
        }
    }
}

void TextEntry__NextChar(TextEntry *self) {
    s32 v;

    if (self->panelSprite != NULL) {
        v = self->charIndex + 1;
        self->charIndex = v;
        if (v < self->charCount) {
            self->methods->setCharAt(self, self->cursorIndex, v, 1);
        } else {
            self->charIndex = 0;
        }
    }
}
