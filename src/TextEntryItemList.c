/*
 * TextEntryItemList -- two classes, in ROM order: TextEntry whole, then the
 * first half of ItemList (its allocator to detachTarget). ItemList's list
 * methods and its getter follow in ObjMStyleActor.c.
 *
 * A TextEntry (include/TextEntry.h) edits a caller-owned string on screen.
 * setText keeps the caller's buffer and copies it into its own `editBuf`
 * (decoding full-width SJIS in TEXTENTRY_MODE_FULLWIDTH). loadCardResources
 * makes the panel (CARD\COMINPUT.TIM), the text row and the '_' cursor
 * (CARD\FONTICON.TIM). attachTarget adds a Pad and a FrameClock as children
 * and keeps a VabStreamObj to play sounds on. onNotify sends the Pad's button
 * events to handleCommand: left/right move the cursor, up/down step the
 * character under it, L1/L2 reset it/every character, Select switches
 * between acting on presses and on held buttons, circle writes the edit back
 * and closes ACCEPTED, cross closes CANCELLED. It sends the FrameClock's
 * ticks to tickState, which reports the result to the parents on the second
 * tick after the close. GetTextEntryMethods ends the class.
 *
 * ItemList (include/ItemList.h), the list of strings the player picks one
 * from: see the section banner below.
 *
 * What decided its edges (python3 tools/tuboundary.py): the placed object
 * libcard/a80 precedes it ("start edge possible"). The old carve edge
 * class_3bb8c_i|class_3bb8c_j cut TextEntry off PrevChar..SetCharAt and its
 * getter ("start edge possible, soft-unlikely (0x80086f7c, 0x80086ed0)"), so
 * the two were merged. The end edge is kept because the binary forces it: the
 * jump tables of TextEntry__HandleCommand (0x80011628) and
 * ItemList__HandleInputCode (0x800116f4, ObjMStyleActor.c) differ in parity,
 * so a file boundary lies between those two functions ("a forced boundary
 * lies in this stretch: tables 0x80011628 / 0x800116f4"), and with the
 * first edge merged this is the one carve edge left in that stretch.
 * PARKED: content puts that boundary inside this file, between
 * GetTextEntryMethods and New_ItemList (the getter closes its class, and
 * that gap is "boundary possible" between "unlikely" ones); a split is a new
 * carve, so the file keeps ItemList's first half and is named for both.
 */
#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <strings.h>
#include "class_3bb8c.h"
#include "TextEntry.h"
#include "CharSprite.h"
#include "TextRow.h"
#include "TimImage.h"
#include "VabStreamObj.h"
#include "Pad.h"
#include "FrameClock.h"
#include "ItemList.h"
#include "ScreenSprite.h"

/* ScreenWidgets.c's, which types both u8 *(u8 *dst, u8 *src); declared on
 * TextEntry's char buffers. Decode turns full-width SJIS into one byte a
 * character, Encode turns it back. */
extern char *DecodeFullWidthSjis(char *dest, char *src);
extern void EncodeFullWidthSjis(char *dest, char *src);

TextEntry *New_TextEntry(char *text, s32 mode) {
    TextEntry *self;

    self = BMemPMgrAlloc(sizeof(TextEntry));
    if (self != NULL) {
        GetTextEntryMethods()->ctor(self, text, mode);
        return self;
    }
    return NULL;
}

/* The characters nextChar/prevChar step through, NUL-terminated. */
extern u8 *gNameCharTable;

void TextEntry__TextEntry(TextEntry *self, char *text, s32 mode) {
    u8 *p;
    s32 count;

    Get_vtable_BasicClass()->ctor((BasicClass *)self);
    self->methods = GetTextEntryMethods();
    self->textLen = strlen(text);
    self->editBuf = BMemPMgrAlloc(self->textLen + 4);

    /* MATCHING: counted inline; strlen(gNameCharTable) would be a call. */
    p = gNameCharTable;
    count = 0;
    while (*p != 0) {
        p++;
        count++;
    }
    self->charCount = count;

    TextEntry__ClearChildRefs(self);
    self->methods->setText(self, text, mode);
}

void TextEntry__ClearChildRefs(TextEntry *self) {
    self->inputSource = NULL;
    self->tickSource = NULL;
    self->panelSprite = NULL;
}

void TextEntry__Finalize(TextEntry *self) {
    BMemPMgrFree(self->editBuf);
    Get_vtable_BasicClass()->finalize((BasicClass *)self);
}

void TextEntry__AddChild(TextEntry *self, void *child) {
    s32 kind;

    if (child != NULL) {
        Get_vtable_BasicClass()->addChild((BasicClass *)self, (BasicClass *)child);
        kind = ((BasicClass *)child)->methods->header & CLASS_ID_ROOT_MASK;
        if (kind == PAD_CLASS_ID) {
            self->inputSource = child;
        } else if (kind == FRAMECLOCK_CLASS_ID) {
            self->tickSource = child;
        }
    }
}

void TextEntry__RemoveChild(TextEntry *self, void *child) {
    s32 kind;

    if (child != NULL) {
        kind = ((BasicClass *)child)->methods->header & CLASS_ID_ROOT_MASK;
        if (kind == PAD_CLASS_ID) {
            self->inputSource = NULL;
        } else if (kind == FRAMECLOCK_CLASS_ID) {
            self->tickSource = NULL;
        }
        Get_vtable_BasicClass()->removeChild((BasicClass *)self, (BasicClass *)child);
    }
}

void TextEntry__RemoveAllChildren(TextEntry *self) {
    self->inputSource = NULL;
    self->tickSource = NULL;
    self->panelSprite = NULL;
    Get_vtable_BasicClass()->removeAllChildren((BasicClass *)self);
}

void TextEntry__OnNotify(TextEntry *self, void *sender, s32 event) {
    s32 kind;

    Get_vtable_BasicClass()->onNotify((BasicClass *)self, sender, event);

    kind = ((BasicClass *)sender)->methods->header & CLASS_ID_ROOT_MASK;
    if (kind == PAD_CLASS_ID) {
        self->methods->handleCommand(self, sender, event);
    } else if (kind == FRAMECLOCK_CLASS_ID) {
        self->methods->tickState(self, sender, event);
    }
}

void TextEntry__SetText(TextEntry *self, char *text, s32 mode) {
    self->mode = mode;
    self->textBuf = text;
    self->cursorIndex = 0;
    self->charIndex = 0;
    if (mode == TEXTENTRY_MODE_FULLWIDTH) {
        DecodeFullWidthSjis(self->editBuf, text);
        self->textLen /= 2;
    } else {
        strcpy(self->editBuf, text);
    }
}

/* LoadCardResources' data. BuildFileName (GameApplicationFileResource.c) writes dir, name
 * and ext into dest and returns it. Positions are percent of half the
 * screen from the centre (include/ScreenSprite.h). */
extern char *BuildFileName(char *dest, const char *name, const char *dir, const char *ext);

extern const char sStrComInput[];          /* "COMINPUT" */
extern const char sStrFontIcon[];          /* "FONTICON" */
extern const char sCardPathPrefix[];       /* "CARD\\" */
extern const char sTimExt[];               /* ".TIM" */
extern SpriteRect gTextEntryPanelRect;     /* COMINPUT's cell: 224 x 120 from (0, 0) */
extern SpriteRgb gTextEntryTextColor;      /* the text row's colour: (128, 128, 0) */
extern ScreenSpritePos gTextEntryPanelPos; /* (-70, -60) */
extern ScreenSpritePos gTextEntryTextPos;  /* (-62, -15) */

void TextEntry__LoadCardResources(TextEntry *self, void *parent) {
    char path[32];
    const char *dir;
    const char *ext;
    TimImage *panelTim;
    TimImage *fontTim;

    if (parent == NULL) {
        return;
    }
    if (self->panelSprite != NULL) {
        return;
    }

    dir = sCardPathPrefix;
    ext = sTimExt;

    panelTim = New_TimImage(BuildFileName(path, sStrComInput, dir, ext));
    ((TimImageUploadFn)panelTim->methods->processBuffer)(panelTim);
    self->panelSprite = New_ScreenSprite(panelTim, &gTextEntryPanelRect, 0);
    panelTim->methods->release(panelTim);
    self->panelSprite->methods->attachToParent(self->panelSprite, (SceneNode *)parent,
                                               (LongVec3 *)&gTextEntryPanelPos);

    fontTim = New_TimImage(BuildFileName(path, sStrFontIcon, dir, ext));
    ((TimImageUploadFn)fontTim->methods->processBuffer)(fontTim);
    self->textRow = New_TextRow(fontTim, self->textLen, self->editBuf);
    self->cursorSprite = New_CharSprite(fontTim, '_');
    fontTim->methods->release(fontTim);
    self->textRow->methods->attachToParent(self->textRow, (SceneNode *)parent,
                                           (LongVec3 *)&gTextEntryTextPos);
    self->textRow->methods->setColor(self->textRow, &gTextEntryTextColor);
    self->cursorSprite->methods->attachToParent(self->cursorSprite, (SceneNode *)parent,
                                                (LongVec3 *)&gTextEntryCursorPos);
}

void TextEntry__ReleaseCardResources(TextEntry *self) {
    if (self->panelSprite != NULL) {
        self->panelSprite = self->panelSprite->methods->release(self->panelSprite);
        self->textRow->methods->release(self->textRow);
        self->cursorSprite->methods->release(self->cursorSprite);
    }
}

void TextEntry__AttachTarget(TextEntry *self, void *inputSource, void *tickSource, VabStreamObj *target) {
    self->methods->addChild(self, inputSource);
    self->methods->addChild(self, tickSource);
    self->target = target;
    self->closeState = 0;
    self->actOnHeld = 0;
}

void TextEntry__DetachTarget(TextEntry *self) {
    self->methods->removeChild(self, self->inputSource);
    self->methods->removeChild(self, self->tickSource);
    self->target = NULL;
}

void TextEntry__SetState(TextEntry *self, s32 state) {
    self->closeTickCount = 0;
    if (state < TEXTENTRY_RESULT_ACCEPTED) {
        return;
    }
    switch (state) {
        case TEXTENTRY_RESULT_ACCEPTED:
        case TEXTENTRY_RESULT_CANCELLED:
            self->methods->removeChild(self, self->inputSource);
            self->methods->releaseCardResources(self);
            self->closeState = state;
            break;
        case TEXTENTRY_STATE_REPORT:
            self->methods->notifyParents(self, self->closeState);
            break;
    }
}

void TextEntry__TickState(TextEntry *self) {
    s32 state;
    s32 old;

    state = self->closeState;
    if (state >= TEXTENTRY_STATE_REPORT) {
        return;
    }
    if (state < TEXTENTRY_RESULT_ACCEPTED) {
        return;
    }
    old = self->closeTickCount;
    self->closeTickCount = old + 1;
    if (old != 0) {
        self->methods->setState(self, TEXTENTRY_STATE_REPORT);
    }
}

/* With actOnHeld clear the arrows act on presses, with it set on held
 * buttons. MATCHING: the arms are in retail's code order, default first,
 * and the down press jumps into the held down's call rather than making
 * its own. */
void TextEntry__HandleCommand(TextEntry *self, void *sender, s32 command) {
    switch (command) {
        default:
            return;
        case PAD_EVENT_PRESSED + PAD_BUTTON_RRIGHT:
            if (self->mode == TEXTENTRY_MODE_FULLWIDTH) {
                EncodeFullWidthSjis(self->textBuf, self->editBuf);
            } else {
                strcpy(self->textBuf, self->editBuf);
            }
            self->methods->playSound(self, 1 << 4); /* VAB program 1, tone 0 */
            self->methods->setState(self, TEXTENTRY_RESULT_ACCEPTED);
            return;
        case PAD_EVENT_PRESSED + PAD_BUTTON_RDOWN:
            self->methods->playSound(self, 1 << 4); /* VAB program 1, tone 0 */
            self->methods->setState(self, TEXTENTRY_RESULT_CANCELLED);
            return;
        case PAD_EVENT_PRESSED + PAD_BUTTON_L2:
            self->methods->resetAllChars(self);
            return;
        case PAD_EVENT_PRESSED + PAD_BUTTON_L1:
            self->methods->resetChar(self);
            return;
        case PAD_EVENT_PRESSED + PAD_BUTTON_SELECT:
            self->methods->toggleActOnHeld(self);
            return;
        case PAD_EVENT_PRESSED + PAD_BUTTON_LRIGHT:
            if (self->actOnHeld != 0) {
                return;
            }
            self->methods->moveCursorRight(self);
            return;
        case PAD_EVENT_HELD + PAD_BUTTON_LRIGHT:
            if (self->actOnHeld == 0) {
                return;
            }
            self->methods->moveCursorRight(self);
            return;
        case PAD_EVENT_PRESSED + PAD_BUTTON_LLEFT:
            if (self->actOnHeld != 0) {
                return;
            }
            self->methods->moveCursorLeft(self);
            return;
        case PAD_EVENT_HELD + PAD_BUTTON_LLEFT:
            if (self->actOnHeld == 0) {
                return;
            }
            self->methods->moveCursorLeft(self);
            return;
        case PAD_EVENT_PRESSED + PAD_BUTTON_LUP:
            if (self->actOnHeld != 0) {
                return;
            }
            self->methods->nextChar(self);
            return;
        case PAD_EVENT_HELD + PAD_BUTTON_LUP:
            if (self->actOnHeld == 0) {
                return;
            }
            self->methods->nextChar(self);
            return;
        case PAD_EVENT_PRESSED + PAD_BUTTON_LDOWN:
            if (self->actOnHeld == 0) {
                goto callPrevChar;
            }
            return;
        case PAD_EVENT_HELD + PAD_BUTTON_LDOWN:
            if (self->actOnHeld == 0) {
                return;
            }
        callPrevChar:
            self->methods->prevChar(self);
            return;
    }
}

void TextEntry__PlaySound(TextEntry *self, s32 tone) {
    VabStreamObj *target;

    target = self->target;
    if (target != NULL) {
        target->methods->playTone(target, tone, 96, 96);
    }
}

void TextEntry__MoveCursorRight(TextEntry *self) {
    s32 old;
    s32 next;

    if (self->panelSprite != NULL) {
        old = self->cursorIndex;
        next = old + 1;
        self->cursorIndex = next;
        if (next < self->textLen) {
            self->methods->setCursorPos(self, next, 1);
        } else {
            self->cursorIndex = old;
        }
    }
}

void TextEntry__MoveCursorLeft(TextEntry *self) {
    s32 old;
    s32 next;

    if (self->panelSprite != NULL) {
        old = self->cursorIndex;
        next = old - 1;
        self->cursorIndex = next;
        if (next >= 0) {
            self->methods->setCursorPos(self, next, 1);
        } else {
            self->cursorIndex = old;
        }
    }
}

void TextEntry__NextChar(TextEntry *self) {
    s32 next;

    if (self->panelSprite != NULL) {
        next = self->charIndex + 1;
        self->charIndex = next;
        if (next < self->charCount) {
            self->methods->setCharAt(self, self->cursorIndex, next, 1);
        } else {
            self->charIndex = 0;
        }
    }
}

void TextEntry__PrevChar(TextEntry *self) {
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

void TextEntry__ToggleActOnHeld(TextEntry *self) {
    if (self->panelSprite) {
        self->actOnHeld ^= 1;
    }
}

void TextEntry__ResetChar(TextEntry *self) {
    if (self->panelSprite) {
        self->charIndex = 0;
        self->methods->setCharAt(self, self->cursorIndex, 0, 1);
    }
}

void TextEntry__ResetAllChars(TextEntry *self) {
    s32 i;

    if (self->panelSprite) {
        self->charIndex = 0;
        for (i = self->textLen - 1; i >= 0; i--) {
            self->cursorIndex = i;
            self->methods->setCharAt(self, i, self->charIndex, 0);
        }
        self->methods->setCursorPos(self, self->cursorIndex, 1);
    }
}

void TextEntry__SetCursorPos(TextEntry *self, s32 pos, s32 notify) {
    ScreenSpritePos screenPos;
    CharSprite *cursor;

    if (self->panelSprite) {
        screenPos.y = gTextEntryCursorPos.y;
        screenPos.x = pos * TEXTROW_DEFAULT_PITCH + gTextEntryCursorPos.x;
        cursor = self->cursorSprite;
        cursor->methods->setPosition(cursor, &screenPos);
        self->cursorIndex = pos;
        if (notify) {
            self->methods->playSound(self, 0);
        }
    }
}

/* The characters an entry can hold, in the order nextChar/prevChar step
 * through them: a pointer to a NUL-terminated byte string (TextEntry.h). */
extern u8 *gNameCharTable;

void TextEntry__SetCharAt(TextEntry *self, s32 pos, s32 charIndex, s32 notify) {
    TextRow *row;

    if (self->panelSprite) {
        self->editBuf[pos] = gNameCharTable[charIndex];
        row = self->textRow;
        ((TextRowSetCellAtFn)row->methods->setCell)(row, gNameCharTable[charIndex], pos);
        self->cursorIndex = pos;
        self->charIndex = charIndex;
        if (notify) {
            self->methods->playSound(self, 0);
        }
    }
}

/* TextEntry's own table getter (include/TextEntry.h), defined here in ROM
 * order; not ItemList's. */
TextEntryMethods *GetTextEntryMethods(void) {
    return &gTextEntryMethods;
}

/* ---- ItemList, first half ---------------------------------------------
 *
 * ItemList (include/ItemList.h), the list of strings the player picks one
 * from: its allocator and ctor, BasicClass's overrides (finalize, child
 * bookkeeping, onNotify), and the view and resource methods resetView,
 * loadResources, releaseResources, attachTarget and detachTarget. Its list
 * methods and GetItemListMethods are in ObjMStyleActor.c.
 *
 * Like TextEntry it keeps its input and tick children by kind (Pad,
 * FrameClock) and draws through a ScreenSprite panel and TextRows built from
 * CARD\ TIMs; its list methods do nothing until loading has made
 * `panelSprite`. The base-class calls go through BasicClass's table
 * (include/BasicClass.h) and upcast `self`.
 */

ItemList *New_ItemList(char **items, s32 mode) {
    ItemList *self = BMemPMgrAlloc(sizeof(ItemList));

    /* MATCHING: goto, not an early return: NULL fills the branch's delay slot. */
    if (self == NULL) {
        goto fail;
    }
    GetItemListMethods()->ctor(self, items, mode);
    return self;
fail:
    return NULL;
}

/*
 * The ctor. `items` is a NULL-terminated array of strings, copied into
 * buffers of the list's own: `texts[i]` holds item i and `textLens[i]` its
 * length in characters (bytes, halved for full-width SJIS), and
 * `maxTextLen` is the longest. Ends with resetView.
 */
extern char *DecodeFullWidthSjis(char *dest, char *src);

void ItemList__ItemList(ItemList *self, char **items, s32 mode) {
    char **item;
    s32 i;
    s32 len;

    /* MATCHING: both set before the base ctor call. */
    i = 0;
    item = items;
    Get_vtable_BasicClass()->ctor((BasicClass *)self);
    self->methods = GetItemListMethods();

    while (*item++ != NULL) {
        i++;
    }

    self->itemCount = i;
    self->texts = BMemPMgrAlloc(i * sizeof(*self->texts));
    item = items;
    self->textLens = BMemPMgrAlloc(self->itemCount * sizeof(*self->textLens));
    self->maxTextLen = 0;

    for (i = 0; i < self->itemCount; i++) {
        len = strlen(*item);
        if (mode == ITEMLIST_MODE_FULLWIDTH) {
            len /= 2;
        }
        self->textLens[i] = len;
        self->texts[i] = BMemPMgrAlloc(len + 4);
        if (mode == ITEMLIST_MODE_FULLWIDTH) {
            DecodeFullWidthSjis(self->texts[i], *item);
        } else {
            strcpy(self->texts[i], *item);
        }
        /* MATCHING: a ternary, not an `if`: the old value is stored back. */
        self->maxTextLen = (self->maxTextLen < len) ? len : self->maxTextLen;
        item++;
    }

    self->mode = mode;
    ItemList__ClearCachedRefs(self);
    self->methods->resetView(self);
}

void ItemList__ClearCachedRefs(ItemList *self) {
    self->inputSource = NULL;
    self->tickSource = NULL;
    self->panelSprite = NULL;
}

void ItemList__Finalize(ItemList *self) {
    s32 i;

    for (i = 0; i < self->itemCount; i++) {
        BMemPMgrFree(self->texts[i]);
    }
    BMemPMgrFree(self->textLens);
    BMemPMgrFree(self->texts);
    Get_vtable_BasicClass()->finalize((BasicClass *)self);
}

void ItemList__AddChild(ItemList *self, void *child) {
    s32 tag;

    if (child) {
        Get_vtable_BasicClass()->addChild((BasicClass *)self, (BasicClass *)child);
        tag = ((BasicClass *)child)->methods->header & CLASS_ID_ROOT_MASK;
        if (tag == PAD_CLASS_ID) {
            self->inputSource = child;
        } else if (tag == FRAMECLOCK_CLASS_ID) {
            self->tickSource = child;
        }
    }
}

void ItemList__RemoveChild(ItemList *self, void *child) {
    s32 tag;

    if (child) {
        tag = ((BasicClass *)child)->methods->header & CLASS_ID_ROOT_MASK;
        if (tag == PAD_CLASS_ID) {
            self->inputSource = NULL;
        } else if (tag == FRAMECLOCK_CLASS_ID) {
            self->tickSource = NULL;
        }
        Get_vtable_BasicClass()->removeChild((BasicClass *)self, (BasicClass *)child);
    }
}

void ItemList__RemoveAllChildren(ItemList *self) {
    self->inputSource = NULL;
    self->tickSource = NULL;
    self->panelSprite = NULL;
    Get_vtable_BasicClass()->removeAllChildren((BasicClass *)self);
}

void ItemList__OnNotify(ItemList *self, void *sender, s32 event) {
    s32 tag;

    Get_vtable_BasicClass()->onNotify((BasicClass *)self, sender, event);
    tag = ((BasicClass *)sender)->methods->header & CLASS_ID_ROOT_MASK;
    if (tag == PAD_CLASS_ID) {
        self->methods->handleInputCode(self, sender, event);
    } else if (tag == FRAMECLOCK_CLASS_ID) {
        self->methods->tickClosing(self, sender, event);
    }
}

void ItemList__ResetView(ItemList *self) {
    self->topIndex = 0;
    self->column = 0;
    self->cursorIndex = 0;
}

/* loadResources' path buffer, BuildFileName's dest: "CARD\\" + name + ".TIM". */
#define CARD_TIM_PATH_SIZE 32

extern char *BuildFileName(char *dest, const char *name, const char *dir, const char *ext);
extern const char sStrSelect[];              /* "SELECT" */
extern const char sItemListCardPathPrefix[]; /* "CARD\\" */
extern const char sItemListTimExt[];         /* ".TIM" */
extern SpriteRect gItemListPanelRect;        /* SELECT's cell: 256 x 160 from (0, 0) */
extern ScreenSpritePos gItemListPanelPos;    /* (-100, -60) */
extern const char sItemListStrFontIcon[];    /* "FONTICON" */

/*
 * Loads CARD\SELECT.TIM as the panel sprite, placed at gItemListPanelPos
 * under `parent`, and has createRows build the rows from CARD\FONTICON.TIM.
 * Does nothing without a parent or when already loaded. The same shape as
 * TextEntry__LoadCardResources (TextEntryItemList).
 */
void ItemList__LoadResources(ItemList *self, SceneNode *parent) {
    char path[CARD_TIM_PATH_SIZE];
    const char *dir;
    const char *ext;
    TimImage *panelTim; /* MATCHING: two handles, not one reused */
    TimImage *fontTim;

    if (parent == NULL) {
        return;
    }
    if (self->panelSprite != NULL) {
        return;
    }

    dir = sItemListCardPathPrefix;
    ext = sItemListTimExt;

    panelTim = New_TimImage(BuildFileName(path, sStrSelect, dir, ext));
    ((TimImageUploadFn)panelTim->methods->processBuffer)(panelTim);
    self->panelSprite = New_ScreenSprite(panelTim, &gItemListPanelRect, 0);
    panelTim->methods->release(panelTim);
    self->panelSprite->methods->attachToParent(self->panelSprite, parent, (LongVec3 *)&gItemListPanelPos);

    fontTim = New_TimImage(BuildFileName(path, sItemListStrFontIcon, dir, ext));
    ((TimImageUploadFn)fontTim->methods->processBuffer)(fontTim);
    self->methods->createRows(self, parent, fontTim, self->topIndex, self->column, self->cursorIndex);
    fontTim->methods->release(fontTim);
}

void ItemList__ReleaseResources(ItemList *self) {
    if (self->panelSprite) {
        self->methods->releaseRows(self);
        self->panelSprite = self->panelSprite->methods->release(self->panelSprite);
    }
}

/* Adds the input and tick children, keeps the sound as `target` and zeroes
 * `result`. The first addChild is handed all four words (its occupant reads
 * only the first; ItemListAddChildWideFn). */
void ItemList__AttachTarget(ItemList *self, void *inputSource, void *tickSource,
                            struct VabStreamObj *target) {
    ItemListAddChildWideFn addChildWide;
    s32 zero;

    /* MATCHING: the cached slot and zero and the do/while (0) fill a delay slot. */
    zero = 0;
    addChildWide = (ItemListAddChildWideFn)self->methods->addChild;
    do {
        addChildWide(self, inputSource, tickSource, target);
        self->methods->addChild(self, tickSource);
        self->target = target;
        self->result = zero;
    } while (0);
}

void ItemList__DetachTarget(ItemList *self) {
    self->methods->removeChild(self, self->inputSource);
    self->methods->removeChild(self, self->tickSource);
    self->target = NULL;
}
