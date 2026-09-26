/*
 * code_2cc8c_f -- two classes and three free functions.
 *
 * BoxFill (class id 0x64, gBoxFillMethods, include/BoxFill.h), the
 * functions from BoxFill__AttachToParent to GetBoxFillMethods: a GsBOXF
 * screen rectangle's attach, attribute bits, colour, position, size,
 * priority and mask (its allocator, ctor and Reset close code_2cc8c_e).
 *
 * TextRow (class id 0x11144, gTextRowMethods, include/TextRow.h; below
 * CharSprite, NOT below BoxFill), from New_TextRow to GetTextRowMethods: a
 * row of CharSprite character cells.
 *
 * Then `DecodeFullWidthSjis`/`EncodeFullWidthSjis`/`FormatFullWidthNumber`,
 * confirmed by their OWN callers elsewhere to take a plain buffer.
 *
 * All non-trivial functions are MATCHED; zero live INCLUDE_ASM, zero
 * NON_MATCHING bodies. `TextRow__NoOpGetCell`/`TextRow__NoOpSlotD0` are
 * splat-generated `jr $ra; nop` occupants.
 */
#include "common.h"
#include "code_2cc8c.h"
#include "TextRow.h"

void BoxFill__AttachToParent(BoxFill *self, Class6B5CC *parent, Pair32E99C *pos) {
    if (self->parent == NULL) {
        GetClass6B5CCMethods()->attachToParent((Class6B5CC *)self, parent, 0);
        self->methods->setPosition(self, pos);
    }
}

s32 BoxFill__SetDisplay(BoxFill *self, s32 on) {
    return GetSetBitField(&self->boxAttribute, 0x1F, 1, on == 0) == 0;
}

s32 BoxFill__SetSemiTrans(BoxFill *self, s32 on) {
    return GetSetBitField(&self->boxAttribute, 0x1E, 1, on != 0);
}

s32 BoxFill__SetSemiTransRate(BoxFill *self, s32 rate) {
    return GetSetBitField(&self->boxAttribute, 0x1C, 2, rate);
}

void BoxFill__SetColor(BoxFill *self, s32 overwrite, u8 *rgb) {
    BoxFill__ApplyColor(self, self->color, rgb, overwrite);
}

typedef struct {
    s8 r, g, b;
} RGB80040790;

void BoxFill__ApplyColor(BoxFill *self, u8 *dst, u8 *src, s32 overwrite) {
    u8 *d;
    d = dst;
    if (overwrite) {
        *(RGB80040790 *)d = *(RGB80040790 *)src;
    } else {
        d[0] += src[0];
        d[1] += src[1];
        d[2] += src[2];
    }
}

void BoxFill__SetPosition(BoxFill *self, Pair32E99C *pos) {
    if (self->parent != NULL) {
        *(Pair32E99C *)&self->posX = *pos;
    }
}

void BoxFill__SetSize(BoxFill *self, s32 *size) {
    if (self->parent != NULL) {
        self->boxW = ((u16 *)size)[0];
        self->boxH = ((u16 *)&size[1])[0];
    }
}

/* +0x04C is called with FOUR arguments through an unprototyped pointer: its
 * occupant reads three, and the fourth is this function's own a3, already in
 * $a3 (BoxFill.h's banner). */
void BoxFill__AttachAbsolute(BoxFill *self, Class6B5CC *parent, Pair32E99C *pos, s32 arg3) {
    void (*fn)();
    BoxFill *q;

    q = self;
    fn = (void (*)())q->methods->attachToParent;
    /* The do/while(0) wrapper is a no-op scoping device, load-bearing for
     * delay-slot scheduling only -- see the match report. Without it GCC
     * swaps the prologue's $ra/$s1 callee-save STORE ORDER. */
    do {
        fn(q, parent, pos, arg3);
        q->relative = 0;
        q->unk4C = arg3;
    } while (0);
}

void BoxFill__SetPri(BoxFill *self, s32 pri) {
    self->pri = pri;
}

s32 BoxFill__SetMask(BoxFill *self, s32 bits) {
    return self->mask = (1 << bits) - 1;
}

BoxFillMethods *GetBoxFillMethods(void) {
    return &gBoxFillMethods;
}

TextRow *New_TextRow(void *texture, s32 count, char *text) {
    TextRow *self = BMemPMgrAlloc(0xB8);
    if (self != NULL) {
        GetTextRowMethods()->ctor(self, texture, count, text);
        return self;
    }
    return NULL;
}

void TextRow__TextRow(TextRow *self, void *texture, s32 count, char *text) {
    s32 i;
    CharSprite **cursor;

    GetCharSpriteMethods()->ctor((CharSprite *)self, texture, 0x20);
    self->methods = GetTextRowMethods();
    self->cellCount = count;
    self->visibleCount = count;
    self->firstVisible = 0;
    self->gapIndex = 0;
    cursor = BMemPMgrAlloc(count * 4);
    if (cursor != NULL) {
        self->cells = cursor;
        i = 0;
        if (i < count) {
            do {
                *cursor = New_CharSprite(texture, 0x20);
                i++;
                cursor++;
            } while (i < count);
        }
        ((TextRowResetFn)self->methods->reset)(self, text);
    }
}

void TextRow__Finalize(TextRow *self) {
    ReleaseBasicClassArray(self->cells, self->cellCount);
    self->cells = BMemPMgrFree(self->cells);
    GetCharSpriteMethods()->finalize((CharSprite *)self);
}

void TextRow__Reset(TextRow *self, char *text) {
    self->methods->setCellPitch(self, 7);
    self->methods->setText(self, text);
}

void TextRow__AttachToParent(TextRow *self, Class6B5CC *parent, ScreenSpritePos *pos) {
    ScreenSpritePos buf;
    s32 i, bound;
    CharSprite **elemp;

    if (self->parent != NULL) {
        return;
    }
    GetCharSpriteMethods()->attachToParent((CharSprite *)self, parent, (Vec3_d294 *)pos);
    buf = *pos;
    elemp = self->cells + self->firstVisible;
    i = self->firstVisible;
    bound = i;
    if (i < bound + self->visibleCount) {
        do {
            if (self->gapIndex != 0 && i == self->gapIndex) {
                buf.x += 0x10;
            }
            (*elemp)->methods->attachToParent(*elemp, (Class6B5CC *)self, (Vec3_d294 *)&buf);
            buf.x += self->cellPitch;
            bound = self->firstVisible;
            elemp++;
            i++;
        } while (i < bound + self->visibleCount);
    }
}

void TextRow__DetachFromParent(TextRow *self) {
    CharSprite **elemp;
    s32 i, bound;

    if (self->parent != NULL) {
        if (self->cells != NULL) {
            elemp = self->cells + self->firstVisible;
            i = self->firstVisible;
            bound = i;
            if (i < bound + self->visibleCount) {
                do {
                    (*elemp)->methods->detachFromParent(*elemp);
                    elemp++;
                    bound = self->firstVisible;
                    i++;
                } while (i < bound + self->visibleCount);
            }
        }
        GetCharSpriteMethods()->detachFromParent((CharSprite *)self);
    }
}

s32 TextRow__SetDisplay(TextRow *self, s32 on, s32 result) {
    CharSprite **elemp = self->cells + self->firstVisible;
    s32 i = self->firstVisible;
    s32 bound = i;
    if (i < bound + self->visibleCount) {
        do {
            CharSprite *elem = *elemp;
            s32 r;
            elemp++;
            i++;
            r = elem->methods->setDisplay(elem, on);
            bound = self->firstVisible;
            result = r;
        } while (i < bound + self->visibleCount);
    }
    return result;
}

void TextRow__SetColor(TextRow *self, SpriteRgb *rgb) {
    CharSprite **elemp = self->cells + self->firstVisible;
    s32 i = self->firstVisible;
    s32 bound = i;
    s32 count = i + self->visibleCount;
    if (i < count) {
        do {
            CharSprite *elem = *elemp;
            s32 ab;
            elemp++;
            ab = self->visibleCount;
            elem->methods->setColor(elem, rgb);
            i++;
            bound = self->firstVisible;
        } while (i < (bound + self->visibleCount));
    }
}

void TextRow__SetPosition(TextRow *self, ScreenSpritePos *pos) {
    if (self->parent != NULL) {
        ScreenSpritePos buf;
        s32 i;
        s32 bound;
        CharSprite **elemp;

        GetCharSpriteMethods()->setPosition((CharSprite *)self, pos);
        buf = *pos;
        i = 0;
        elemp = self->cells;
        if (i < self->cellCount) {
            do {
                (*elemp)->methods->setPosition(*elemp, &buf);
                buf.x += self->cellPitch;
                bound = self->cellCount;
                elemp++;
                i++;
            } while (i < bound);
        }
    }
}

void TextRow__SetCellAt(TextRow *self, s32 cell, s32 index) {
    CharSprite *elem = self->cells[index];
    elem->methods->setCell(elem, cell & 0xFF);
}

void TextRow__NoOpGetCell(void) {}

void TextRow__SetText(TextRow *self, char *text) {
    CharSprite **elemp = self->cells;
    char *p = text;
    if (p != NULL && *p != 0) {
        do {
            CharSprite *elem = *elemp;
            elem->methods->setCell(elem, *p);
            p++;
            elemp++;
        } while (*p != 0);
    }
}

void TextRow__NoOpSlotD0(void) {}

void TextRow__SetCellPitch(TextRow *self, s32 pitch) {
    self->cellPitch = pitch;
}

TextRowMethods *GetTextRowMethods(void) {
    return &gTextRowMethods;
}

/* DecodeFullWidthSjis -- MATCHED round 38 (24/24). A permuter search (208
 * iterations, rc=0) closed the last residue: retail materializes the
 * 0x40 comparison constant into its own register BEFORE copying `dst`
 * into `d`, and GCC 2.6.3 only reproduces that emission order when the
 * constant is named by a separate local assigned first. See
 * docs/match-reports/DecodeFullWidthSjis.md. */
u8 *DecodeFullWidthSjis(u8 *dst, u8 *src) {
    u8 *d;
    u32 special;
    u32 c;
    u32 v;
    u32 peek;

    if (*src++ != 0) {
        special = 0x40;
        d = dst;
        do {
            d++;
            c = *src;
            dst++;
            if (c < 0x80 && c != special) {
                v = c - 0x1F;
            } else {
                v = c - 0x20;
            }
            src++;
            d[-1] = v;
            peek = *src;
            src++;
        } while (peek != 0);
    }
    *dst = 0;
    return dst;
}

/* EncodeFullWidthSjis -- MATCHED round 38 (31/31). Round 37 got structure and
 * length exact via the two-cursor idiom (`d = dst; dst++; *d = x;`),
 * leaving a pure 3-way register-identity residue. A permuter search
 * (158 iterations, rc=0) closed it: copying the second byte's value
 * into its own local (`trail`) before using it in the comparisons and
 * arithmetic, instead of reusing `c` directly, changes GCC 2.6.3's
 * register allocation to match retail's exactly. See
 * docs/match-reports/EncodeFullWidthSjis.md. */
u8 *EncodeFullWidthSjis(u8 *dst, u8 *src) {
    u8 *d;
    u32 c;
    u32 v;
    u32 lead;
    u32 trail;

    if (*src != 0) {
        do {
            d = dst;
            dst++;
            c = *src;
            if (c >= 0x30) {
                lead = 0x82;
            } else {
                lead = 0x81;
            }
            *d = lead;
            d = dst;
            dst++;
            c = *src;
            trail = c;
            if (trail < 0x60 && trail != 0x20) {
                v = trail + 0x1F;
            } else {
                v = trail + 0x20;
            }
            src++;
            *d = v;
        } while (*src != 0);
    }
    *dst = 0;
    return dst;
}

/* FormatFullWidthNumber -- MATCHED round 38 (56/56). Round 35 got structure and
 * length exact (padded/text VLAs, strlen/itoa naming fixed post-SDK-object
 * renaming) leaving a 4-value register-identity residue (fill/text
 * swapped relative to retail). A permuter search (733 iterations, rc=0)
 * closed it: declaration order text/fill/padded (not round 35's
 * padded/text) PLUS splitting `fill = width - strlen(...)` into two
 * statements (`fill = strlen(...); fill = width - fill;`) together
 * reproduce retail's exact register assignment. Also fixed a stale
 * prototype: the preserved body's forward declaration of EncodeFullWidthSjis
 * as `(Obj6EAC0 *, char *)` predates that function's own round-38 match
 * as `u8 *EncodeFullWidthSjis(u8 *, u8 *)`. Its first parameter was typed as
 * the TextRow view `Obj6EAC0 *` and cast to `u8 *`; it is the output buffer
 * (track 4, round 88: `u8 *dst`, no cast). See docs/match-reports/FormatFullWidthNumber.md. */
extern char *strcpy(char *dst, char *src);
extern void *memset(unsigned char *dst, unsigned char c, int n);
extern int strlen(char *s);
extern char *itoa(int n);

void FormatFullWidthNumber(u8 *dst, s32 a1, s32 width, s32 unpadded) {
    char text[width + 1];
    s32 fill;
    char padded[width + 1];

    fill = strlen(strcpy(text, itoa(a1)));
    fill = width - fill;
    if (unpadded == 0) {
        memset((unsigned char *)padded, '0', width);
        strcpy(&padded[fill], text);
    }
    EncodeFullWidthSjis(dst, (u8 *)(unpadded != 0 ? text : padded));
}
