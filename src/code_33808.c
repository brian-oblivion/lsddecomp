/*
 * code_33808 -- GAME code carved from the head of psyq_33808 on 2026-09-25
 * (FINISHING-PLAN revision 18). 0x33808..0x36654 (vram
 * 0x80043008..0x80045E54). It was counted as Psy-Q SDK by segment name;
 * tools/gameinsdk.py measured it as game (a call into game code, a method-
 * table entry beside game methods, or contiguity with those, and no Sony
 * fingerprint). All 97 functions matched in round 82; named in round 83
 * (track 3 naming pass).
 *
 * Eleven method tables, nine of them Class6D430 (data-source) subclasses
 * reached through nine of D_8006D430's own `Get...Methods` getter slots
 * (from +0x07C):
 *
 *   - TimBlockSrc  (D_8006F0B8): a sector-header + block loader with four
 *     CLUT palette-fade channels (FadeClutRow).
 *   - LinkResource (D_8006F13C): a NULL-ended array of TMD models
 *     (New_TmdModel); named from external call sites (class_3bb8c.c,
 *     code_55dd4.h) that already declare it `LinkResource *`.
 *   - TimArraySrc  (D_8006F1C4): an array of TimImage objects
 *     (code_2bb9c.c's New_TimImage).
 *   - Tod / TodSet (D_8006F240 / D_8006F590, TodSet a Tod subclass): one
 *     TOD's packet stream (ScanTodPackets/DecodeTodPacketWord) and an array
 *     of them; named from include/code_55dd4.h's own "TOD set" (Unk30Obj).
 *   - ModelData / TriggerWorld (D_8006F384 / D_8006F40C, TriggerWorld a
 *     ModelData subclass): a LinkResource+TodSet pair, and an array of
 *     those pairs; ModelData named from code_55dd4.h/.c's own "tmd"/"tods"/
 *     "modelData" fields, TriggerWorld from code_4cd08.c's own declared
 *     return type.
 *   - TileMap / TileAtlas (D_8006F498 / D_8006F514): a 20x15 grid of
 *     16x16-cell map data (a GsMAP, consumed by BgLayer as its map source)
 *     and the 300-GsCELL texture atlas it indexes; built together and used
 *     together in src/code_2c054.c's TaskCore__TaskCore.
 *
 * Two more classes, not Class6D430 subclasses:
 *
 *   - BgLayer (D_8006F2C4): a Class6B5CC subclass wrapping one GsBG
 *     scrolling background layer (its own fields are GsBG's own layout).
 *   - MoviePlayer (D_8006F614): a BasicClass subclass driving CD-streamed,
 *     MDEC-decoded FMV playback (open a CD stream, decode/upload strips,
 *     play/stop/tick controls); called from code_2c054.c.
 *
 * libpress starts right after, at DecDCTReset (now psyq_36654).
 */
#include "common.h"
#include "BasicClass.h"
#include "Class6B5CC.h"
#include "Class6D430.h"
#include "TimBlockSrc.h"
#include "ModelData.h"
#include "Tod.h"
#include "TodSet.h"
#include "TriggerWorld.h"
#include "TmdModel.h"
#include "DrawSystem.h"
#include "CdStream.h"

typedef struct DataSrc33808 DataSrc33808;

/* Unit-local view of this unit's Class6D430 data-source subclasses: the
 * interface plus the extra slots their methods call. Unprototyped where a
 * caller passes no argument. Not ModelData's (D_8006F384) nor Tod's
 * (D_8006F240) nor TodSet's (D_8006F590) nor TriggerWorld's (D_8006F40C):
 * those classes are include/ModelData.h (track 4, round 84), include/Tod.h
 * (round 86), include/TodSet.h and include/TriggerWorld.h (round 88). */
typedef struct DataSrc33808Methods {
    CLASS6D430_SLOTS(DataSrc33808, (DataSrc33808 *self));
    /* +0x07C */ s32 (*slot7C)();
    /* +0x080 */ void *(*slot80)();
} DataSrc33808Methods;

struct DataSrc33808 {
    CLASS6D430_FIELDS(DataSrc33808Methods);
    /* +0x02C */ s32 unk2C;
    /* +0x030 */ DataSrc33808 *unk30;
    /* +0x034 */ s32 unk34;
    /* +0x038 */ s32 unk38;            /* D_8006F498: an allocation */
};

/* A buffer that starts with a word, a count, then that many words. */
typedef struct CountedBuf33808 {
    /* +0x00 */ s32 unk0;
    /* +0x04 */ u32 count;
    /* +0x08 */ s32 entries[1];
} CountedBuf33808;

extern Class6D430Methods *GetActiveDataSourceMethods(void);
extern void ReleaseBasicClassArray(BasicClass **array, s32 count);
extern void BMemPMgrFree(void *arg);
extern void *BMemPMgrAlloc(s32 size);
void *GetLinkResourceMethods(void);
void *GetTimArraySrcMethods(void);
void *GetBgLayerMethods(void);
void *GetTileMapMethods(void);
void *GetTileAtlasMethods(void);

/* The allocators below reach a class's constructor through its table
 * getter; the constructor's parameters vary, so the slot is unprototyped. */
typedef struct Ctor33808 {
    /* +0x000 */ s32 header;
    /* +0x004 */ void *release;
    /* +0x008 */ s32 (*ctor)();
} Ctor33808;

/* Allocate and construct a D_8006F0B8 object. */
void *New_TimBlockSrc(s32 arg0) {
    void *obj = BMemPMgrAlloc(0x84);

    if (obj != NULL) {
        GetTimBlockSrcMethods()->ctor(obj, (char *)arg0);
        return obj;
    }
    return NULL;
}
/* D_8006F0B8 +0x008: constructor -- the active driver's, then this table;
 * clear +0x2C..+0x3C and lay out the four channel entries at +0x40 (shift
 * gTimBlockClutShift, its mask, consecutive slots from 0x1E0); then adopt a 0x24-byte
 * header buffer (state 9 at +0x2A), allocate the 0x800-byte sector buffer
 * at +0x34, open `name` and read the first sector into it. */
extern s16 gTimBlockClutShift;

void TimBlockSrc__TimBlockSrc(TimBlockSrc *self, char *name) {
    TimBlockSrcEntry *e;
    void *hdr;
    s32 i;
    u16 addr;
    s16 shift;
    u16 mask;

    GetActiveDataSourceMethods()->ctor((Class6D430 *)self);
    self->methods = GetTimBlockSrcMethods();
    self->blockCount = 0;
    self->blocks = NULL;
    self->loaded = 0;
    self->sector = NULL;
    self->sectorSize = 0;
    addr = 0;
    mask = 1 << gTimBlockClutShift;
    shift = gTimBlockClutShift;
    for (i = 0; i < 4; i++) {
        e = &self->entries[i];
        e->shift = shift;
        e->mask = mask;
        e->clutX = 0;
        e->clutY = addr + 0x1E0;
        addr += mask;
        e->clutW = 0x100;
        e->clutH = 1;
    }
    hdr = BMemPMgrAlloc(0x24);
    if (hdr != NULL) {
        self->sector = BMemPMgrAlloc(0x800);
        if (self->sector != NULL) {
            self->bufferSize = 0x24;
            self->buffer = hdr;
            self->unk2A = 9;
            self->failed = 0;
            self->methods->open(self, name, 1, 0);
            self->methods->read(self, self->sector, 0x800);
        }
    }
}
/* D_8006F0B8 +0x00C: finalize -- release the object array at +0x30 (+0x2C
 * entries), free it, then the active driver's. */
void TimBlockSrc__Finalize(TimBlockSrc *self) {
    ReleaseBasicClassArray((BasicClass **)self->blocks, self->blockCount);
    BMemPMgrFree(self->blocks);
    GetActiveDataSourceMethods()->finalize((Class6D430 *)self);
}
/* D_8006F0B8 +0x064: the loader's state machine, under the data-source
 * lock. State 9 (header sector read): copy the 0x24-byte header (a count
 * and eight file offsets) out of the sector buffer into the buffer, free
 * the sector, allocate the object array (+0x30) and a sector buffer of the
 * largest offset (+0x34/+0x38), seek to the first block and read it: state
 * 10. State 10 (a block read): hand the block to a new D_8006F1C4 source
 * (its CLUT base the entries at +0x40), run its setFlag and +0x078, and
 * read the next block -- or, after the last, free the sector buffer, mark
 * +0x3C done and run the active driver's setFlag. An allocation failure
 * sets +0x80. */
typedef struct Hdr43200 {
    u8 bytes[0x24];
} Hdr43200;

extern void LockActiveDataSource(void);
extern void UnlockActiveDataSource(void);
u32 MaxOfBufferWords(Class6D430 *self);
void *New_TimArraySrc(s32 arg0);

void TimBlockSrc__AdvanceLoadState(TimBlockSrc *self) {
    DataSrc33808 **p;
    s32 max;
    s32 n;

    LockActiveDataSource();
    switch (self->unk2A) {
        case 9:
            if (self->flags & 0x80) {
                *(Hdr43200 *)self->buffer = *(Hdr43200 *)self->sector;
                BMemPMgrFree(self->sector);
                max = MaxOfBufferWords((Class6D430 *)self);
                self->blocks = BMemPMgrAlloc(*(u32 *)self->buffer * 4);
                if (self->blocks == NULL) {
                    goto fail;
                }
                self->sector = BMemPMgrAlloc(max);
                if (self->sector == NULL) {
                    goto fail;
                }
                self->sectorSize = max;
                self->methods->seek(self, ((u32 *)self->buffer)[1], 0);
                self->methods->read(self, self->sector, max);
                self->unk2A = 10;
            }
            break;
        case 10:
            if (self->flags & 0x80) {
                n = self->blockCount;
                p = (DataSrc33808 **)self->blocks + n;
                *p = New_TimArraySrc(0);
                (*p)->buffer = self->sector;
                (*p)->bufferSize = 0;
                (*p)->unk34 = (s32)self->entries;
                n++;
                (*p)->methods->setFlag(*p);
                ((void (*)())(*p)->methods->slot78)(*p);
                self->blockCount = n;
                if (n < *(u32 *)self->buffer) {
                    self->methods->seek(self, ((u32 *)self->buffer)[n + 1], 0);
                    self->methods->read(self, self->sector, self->sectorSize);
                    self->unk2A = 10;
                } else {
                    BMemPMgrFree(self->sector);
                    self->sector = NULL;
                    self->sectorSize = 0;
                    self->unk2A = 0;
                    self->loaded = 1;
                    GetActiveDataSourceMethods()->setFlag((Class6D430 *)self);
                }
            }
            break;
    }
    goto out;
fail:
    self->failed = 1;
out:
    UnlockActiveDataSource();
}
/* The largest of the buffer's `count` words from +0x14. */
typedef struct Buf434DC {
    /* +0x00 */ u32 count;
    /* +0x04 */ u8 pad4[0x10];
    /* +0x14 */ u32 vals[1];
} Buf434DC;

u32 MaxOfBufferWords(Class6D430 *self) {
    Buf434DC *buf = self->buffer;
    u32 i;
    u32 max = 0;

    for (i = 0; i < buf->count; i++) {
        if (max < buf->vals[i]) {
            max = buf->vals[i];
        }
    }
    return max;
}
/* A three-byte vector. */
typedef struct Vec3S8 {
    s8 x;
    s8 y;
    s8 z;
} Vec3S8;

/* D_8006F0B8 +0x078: set entry `index`'s shift, and its mask from it. */
void TimBlockSrc__SetEntryShift(TimBlockSrc *self, s32 index, s32 shift) {
    TimBlockSrcEntry *e = &self->entries[index];

    e->shift = shift;
    e->mask = 1 << e->shift;
}
/* D_8006F0B8 +0x07C: slot +0x080 for entries 0..3, under the data-source
 * lock. */
extern void LockActiveDataSource(void);
extern void UnlockActiveDataSource(void);

void TimBlockSrc__FadeAllEntries(TimBlockSrc *self, TimBlockSrcColor *color) {
    s32 i;

    LockActiveDataSource();
    for (i = 0; i < 4; i++) {
        self->methods->fadeEntry(self, i, color);
    }
    UnlockActiveDataSource();
}
/* D_8006F0B8 +0x080: under the data-source lock, set entry `index`'s
 * three-byte vector and hand the entry to FadeClutRow. */
void TimBlockSrc__FadeEntry(TimBlockSrc *self, s32 index, TimBlockSrcColor *src) {
    TimBlockSrcEntry *e;

    LockActiveDataSource();
    e = &self->entries[index];
    e->color = *src;
    FadeClutRow(e, index);
    UnlockActiveDataSource();
}
/* Fade one 256-colour CLUT row (the entry's `index`, from VRAM y 0x1E0)
 * toward the entry's colour: read the row back, then for each of
 * mask - 1 steps blend every non-zero colour (step << (12 - shift)) / 0x1000
 * of the way to the colour and upload the result to the next row down. */
typedef struct Rect43648 {       /* LIBGPU.H RECT */
    s16 x;
    s16 y;
    s16 w;
    s16 h;
} Rect43648;

/* LIBGPU.H */
extern int StoreImage(Rect43648 *rect, u32 *p);
extern int LoadImage(Rect43648 *rect, u32 *p);
extern int DrawSync(int mode);

void FadeClutRow(TimBlockSrcEntry *e, s32 index) {
    Rect43648 dst;
    Rect43648 src;
    u16 out[256];
    u16 in[256];
    s32 i;
    s32 j;
    s32 shift;
    s32 r;
    s32 g;
    s32 b;
    s32 f;
    s32 rr;
    s32 gg;
    s32 bb;
    u32 c;
    s32 cr;
    s32 cg;
    s32 cb;

    src.x = 0;
    src.w = 0x100;
    src.h = 1;
    src.y = (index << gTimBlockClutShift) + 0x1E0;
    StoreImage(&src, (u32 *)in);
    DrawSync(0);
    dst.h = 1;
    dst.x = 0;
    dst.y = 0;
    dst.w = 0x100;
    r = (u8)e->color.r;
    g = (u8)e->color.g;
    b = (u8)e->color.b;
    shift = 12 - e->shift;
    e->clutH = e->mask;
    for (i = 0; i < e->mask - 1; i++) {
        f = (i + 1) << shift;
        rr = r * f;
        gg = g * f;
        bb = b * f;
        f = 0x1000 - f;
        for (j = 0; j < src.w; j++) {
            c = in[j];
            if (c == 0) {
                out[j] = in[j];
            } else {
                cr = (in[j] & 0x1F) << 3;
                cg = (c >> 2) & 0xF8;
                cb = (c >> 7) & 0xF8;
                cr = (cr * f + rr) >> 15;
                cg = (cg * f + gg) >> 15;
                cb = (cb * f + bb) >> 15;
                out[j] = (in[j] & 0x8000) | cr | (cg << 5) | (cb << 10);
            }
        }
        dst.y = src.y + i + src.h;
        DrawSync(0);
        LoadImage(&dst, (u32 *)out);
    }
}
TimBlockSrcMethods *GetTimBlockSrcMethods(void) {
    return &D_8006F0B8;
}
/* Allocate and construct a D_8006F13C object; freed and NULL when the constructor fails. */
void *New_LinkResource(s32 arg0) {
    void *obj = BMemPMgrAlloc(0x30);

    if (obj != NULL) {
        if (((Ctor33808 *)GetLinkResourceMethods())->ctor(obj, arg0)) {
            return obj;
        }
        BMemPMgrFree(obj);
    }
    return NULL;
}
/* A data source's construction descriptor: an existing buffer to adopt, or
 * a file name to request. */
typedef struct Src6F240 {
    /* +0x00 */ void *buffer;
    /* +0x04 */ char *name;
} Src6F240;

/* D_8006F13C +0x008: constructor -- the active driver's, then this table;
 * with a descriptor, adopt its buffer (size 0) and run its own +0x064, whose
 * nonzero result fails the construction (NULL), or else request its file. */
void *LinkResource__LinkResource(DataSrc33808 *self, Src6F240 *src) {
    GetActiveDataSourceMethods()->ctor((Class6D430 *)self);
    self->methods = GetLinkResourceMethods();
    if (src != NULL) {
        if (src->buffer != NULL) {
            self->buffer = src->buffer;
            self->bufferSize = 0;
            if (((s32 (*)())self->methods->setFlag)(self)) {
                goto fail;
            }
        } else {
            self->methods->requestLoadFile(self, src->name);
        }
    }
    return self;
fail:
    return NULL;
}
/* D_8006F13C +0x00C: finalize -- release every object in the NULL-ended
 * array at +0x2C, free the array, then the active driver's. */
void LinkResource__Finalize(DataSrc33808 *self) {
    DataSrc33808 **objs = (DataSrc33808 **)self->unk2C;

    while (*objs != NULL) {
        (*objs)->methods->release(*objs);
        objs++;
    }
    BMemPMgrFree((void *)self->unk2C);
    GetActiveDataSourceMethods()->finalize((Class6D430 *)self);
}
/* D_8006F13C +0x064: build a NULL-ended array at +0x2C of one
 * New_TmdModel object per 0x1C-byte record of the buffer (from +0x0C,
 * +0x08 of them), after mapping the TMD (own +0x078); 1 when an allocation
 * fails (everything built so far released and the array freed), otherwise
 * the active driver's setFlag and 0. */
typedef struct Buf439EC {
    /* +0x00 */ u8 pad0[8];
    /* +0x08 */ u32 count;
    /* +0x0C */ u8 recs[1][0x1C];
} Buf439EC;

s32 LinkResource__BuildModels(DataSrc33808 *self) {
    DataSrc33808 **objs;
    u32 i;

    objs = BMemPMgrAlloc((((Buf439EC *)self->buffer)->count + 1) * 4);
    if (objs == NULL) {
        return 1;
    }
    self->unk2C = (s32)objs;
    ((void (*)())self->methods->slot78)(self);
    for (i = 0; i < ((Buf439EC *)self->buffer)->count; i++) {
        *objs = (DataSrc33808 *)New_TmdModel((TmdObject *)((Buf439EC *)self->buffer)->recs[i]);
        if (*objs == NULL) {
            while (i != 0) {
                i--;
                objs--;
                (*objs)->methods->release(*objs);
            }
            BMemPMgrFree(objs);
            return 1;
        }
        objs++;
    }
    *objs = NULL;
    GetActiveDataSourceMethods()->setFlag((Class6D430 *)self);
    return 0;
}
/* LIBGS.H: void GsMapModelingData(unsigned long *p); */
void GsMapModelingData(u32 *p);

/* D_8006F13C +0x078: map the TMD in the buffer (past its id word). */
void LinkResource__MapModel(Class6D430 *self) {
    GsMapModelingData((u32 *)self->buffer + 1);
}
/* D_8006F13C +0x07C: the address of record `index`, 0x1C bytes each,
 * from +0x0C of the buffer. */
typedef struct Rec6F13C {
    u8 data[0x1C];
} Rec6F13C;

typedef struct Buf6F13C {
    /* +0x00 */ u8 pad0[0xC];
    /* +0x0C */ Rec6F13C recs[1];
} Buf6F13C;

Rec6F13C *LinkResource__GetRecord(Class6D430 *self, s32 index) {
    return &((Buf6F13C *)self->buffer)->recs[index];
}
/* D_8006F13C +0x080: returns entry `index` of the word array at +0x2C
 * (the first field past the 0x2C-byte Class6D430 base). */
typedef struct Obj6F13C {
    /* +0x000 */ u8 pad0[0x2C];
    /* +0x02C */ s32 *entries;
} Obj6F13C;

s32 LinkResource__GetEntry(Obj6F13C *self, s32 index) {
    return self->entries[index];
}
void LinkResource__NoOp(void) {
}
extern s32 D_8006F13C[];

void *GetLinkResourceMethods(void) {
    return D_8006F13C;
}
/* Allocate and construct a D_8006F1C4 object. */
void *New_TimArraySrc(s32 arg0) {
    void *obj = BMemPMgrAlloc(0x3C);

    if (obj != NULL) {
        ((Ctor33808 *)GetTimArraySrcMethods())->ctor(obj, arg0);
        return obj;
    }
    return NULL;
}
/* D_8006F1C4 +0x008: constructor -- the active driver's, then this table,
 * clear +0x2C/+0x30/+0x38, and request `name` when there is one. */
void TimArraySrc__TimArraySrc(DataSrc33808 *self, char *name) {
    GetActiveDataSourceMethods()->ctor((Class6D430 *)self);
    self->methods = GetTimArraySrcMethods();
    self->unk2C = 0;
    self->unk30 = NULL;
    self->unk38 = 0;
    if (name != NULL) {
        self->methods->requestLoadFile(self, name);
    }
}
/* D_8006F1C4 +0x00C: finalize -- same shape as D_8006F0B8's. */
void TimArraySrc__Finalize(DataSrc33808 *self) {
    ReleaseBasicClassArray((BasicClass **)self->unk30, self->unk2C);
    BMemPMgrFree(self->unk30);
    GetActiveDataSourceMethods()->finalize((Class6D430 *)self);
}
/* D_8006F1C4 +0x064: when the buffer is there (or flag 0x200 is set),
 * build one TimImage (New_TimImage(NULL)) per image of the buffer -- a
 * count, then that many offsets -- into an array at +0x30 (+0x2C entries),
 * each adopting its image in place (size 0), and set each one's +0x4C from
 * the CLUT row its GsGetTimInfo reports (from y 0x1E0, >> gTimClutRowShift, 16
 * bytes a step past +0x34); then mark +0x38 and the active driver's
 * setFlag. */
typedef struct Image43CB8 {      /* LIBGS.H GsIMAGE */
    /* +0x00 */ u32 pmode;
    /* +0x04 */ s16 px;
    /* +0x06 */ s16 py;
    /* +0x08 */ u16 pw;
    /* +0x0A */ u16 ph;
    /* +0x0C */ u32 *pixel;
    /* +0x10 */ s16 cx;
    /* +0x12 */ s16 cy;
    /* +0x14 */ u16 cw;
    /* +0x16 */ u16 ch;
    /* +0x18 */ u32 *clut;
} Image43CB8;

typedef struct Tim43CB8 Tim43CB8;

typedef struct TimMethods43CB8 {
    CLASS6D430_SLOTS(Tim43CB8, (Tim43CB8 *self, char *name));
    /* +0x07C */ u8 pad7C[0x20];
    /* +0x09C */ void (*getTimInfo)(Tim43CB8 *self, Image43CB8 *info);
} TimMethods43CB8;

struct Tim43CB8 {                /* TimImage (code_2bb9c.c) */
    CLASS6D430_FIELDS(TimMethods43CB8);
    /* +0x02C */ u8 pad2C[0x20];
    /* +0x04C */ s32 clutBase;
};

typedef struct Obj43CB8 {
    CLASS6D430_FIELDS(DataSrc33808Methods);
    /* +0x02C */ s32 count;
    /* +0x030 */ Tim43CB8 **images;
    /* +0x034 */ s32 base;
    /* +0x038 */ s32 ready;
} Obj43CB8;

extern Tim43CB8 *New_TimImage(char *name);
extern s16 gTimClutRowShift;

void TimArraySrc__BuildImages(Obj43CB8 *self) {
    Image43CB8 info;
    Tim43CB8 **objs;
    s32 i;
    s32 *offs;

    if ((self->flags & 0x200) || self->buffer != NULL) {
        self->count = *(s32 *)self->buffer;
        self->images = BMemPMgrAlloc(*(s32 *)self->buffer * 4);
        if (self->images != NULL) {
            objs = self->images;
            offs = (s32 *)self->buffer + 1;
            for (i = 0; i < self->count; i++) {
                *objs = New_TimImage(NULL);
                (*objs)->buffer = (u8 *)self->buffer + *offs;
                (*objs)->bufferSize = 0;
                (*objs)->methods->getTimInfo(*objs, &info);
                (*objs)->clutBase = ((info.cy - 0x1E0) >> gTimClutRowShift) * 16 + self->base;
                offs++;
                objs++;
            }
            self->ready = 1;
            GetActiveDataSourceMethods()->setFlag((Class6D430 *)self);
        }
    }
}
/* D_8006F1C4 +0x078: slot +0x078 of every object in the array at +0x30
 * (+0x2C entries). */
void TimArraySrc__NotifyImages(DataSrc33808 *self) {
    DataSrc33808 **objs = (DataSrc33808 **)self->unk30;
    s32 i;

    for (i = 0; i < self->unk2C; i++) {
        ((void (*)())(*objs)->methods->slot78)(*objs);
        objs++;
    }
}
extern s32 D_8006F1C4[];

void *GetTimArraySrcMethods(void) {
    return D_8006F1C4;
}
/* Allocate and construct a D_8006F240 object. */
Tod *New_Tod(Src6F240 *src) {
    void *obj = BMemPMgrAlloc(0x2C);

    if (obj != NULL) {
        ((Ctor33808 *)GetTodMethods())->ctor(obj, src);
        return obj;
    }
    return NULL;
}
/* D_8006F240 +0x008: constructor -- the active driver's, then this table;
 * adopt a buffer handed in (size 0) and run its own +0x064, or else request
 * the named file. */
void Tod__Tod(Tod *self, Src6F240 *src) {
    GetActiveDataSourceMethods()->ctor((Class6D430 *)self);
    self->methods = GetTodMethods();
    if (src->buffer != NULL) {
        self->buffer = src->buffer;
        self->bufferSize = 0;
        self->methods->setFlag(self);
    } else {
        self->methods->requestLoadFile(self, src->name);
    }
}
/* D_8006F240 +0x00C: finalize, straight to the active driver's. */
void Tod__Finalize(Tod *self) {
    GetActiveDataSourceMethods()->finalize((Class6D430 *)self);
}
/* D_8006F240 +0x078: slot +0x07C over the buffer past its first two words. */
u8 Tod__ScanPackets(Tod *self, u8 *out, u32 *sel) {
    return self->methods->scanTodPackets(self, out, sel, (u32 *)((u8 *)self->buffer + 8));
}

/* D_8006F240/D_8006F590 +0x07C: walk the packet words after the u16 count
 * at data +2 (from data +8), each decoded by +0x080 into a value, a type, a
 * sub-type and a length in words. Type 8 sub-type 0 appends the value to
 * `out` (when given) and counts it; type 2 either, with `out`, looks the
 * value up among the ones appended so far when its halfword at +4 matches
 * `*sel` -- keeping its index -- or, without `out`, counts it. The index /
 * count goes back through `sel`; returns the number appended. */
u8 ScanTodPackets(Tod *self, u8 *out, u32 *sel, u32 *data) {
    u8 value;
    u8 type;
    u8 sub;
    u8 len;
    u32 n;
    u32 i;
    s32 j;
    u8 cnt;
    s32 found;

    n = ((u16 *)data)[1];
    data += 2;
    i = 0;
    cnt = 0;
    found = 0;
    for (; i < n; i++) {
        DecodeTodPacketWord(self, data, &value, &type, &sub, &len);
        if (type == 8 && sub == 0) {
            cnt++;
            if (out != NULL) {
                *out++ = value;
            }
        } else if (type == 2) {
            if (out != NULL) {
                if (sel != NULL && ((u16 *)data)[2] == *sel) {
                    for (j = 0, out -= cnt; j < cnt; j++) {
                        if (*out++ == value) {
                            found = j;
                            break;
                        }
                    }
                }
            } else if (sel != NULL) {
                found++;
            }
        }
        data += len;
    }
    if (sel != NULL) {
        *sel = found;
    }
    return cnt;
}
/* D_8006F240/D_8006F590 +0x080: decode one packet word -- the low byte, then
 * the two nibbles at bits 16 and 20, then the top byte -- and return the
 * pointer past it. */
u32 *DecodeTodPacketWord(Tod *self, u32 *acc, u8 *out0, u8 *out1, u8 *out2, u8 *out3) {
    u32 v = *acc;

    *out0 = v;
    *out1 = (v >> 16) & 0xF;
    *out2 = (v >> 20) & 0xF;
    *out3 = v >> 24;
    return acc + 1;
}
TodMethods *GetTodMethods(void) {
    return &D_8006F240;
}
/* Allocate and construct a D_8006F2C4 object. */
void *New_BgLayer(s32 arg0, s32 arg1) {
    void *obj = BMemPMgrAlloc(0x68);

    if (obj != NULL) {
        ((Ctor33808 *)GetBgLayerMethods())->ctor(obj, arg0, arg1);
        return obj;
    }
    return NULL;
}
/* The D_8006F2C4 object (a Class6B5CC subclass). */
typedef struct Obj6F2C4 {
    CLASS6B5CC_FIELDS(Class6B5CCMethods);
    /* +0x044 */ u32 bgAttribute; /* +0x044..+0x067 has GsBG's layout */
    /* +0x048 */ s16 x;
    /* +0x04A */ s16 y;
    /* +0x04C */ s16 w;
    /* +0x04E */ s16 h;
    /* +0x050 */ s16 scrollx;
    /* +0x052 */ s16 scrolly;
    /* +0x054 */ Vec3S8 unk54;    /* GsBG r, g, b */
    /* +0x057 */ u8 pad57;
    /* +0x058 */ void *map;
    /* +0x05C */ s16 mx;
    /* +0x05E */ s16 my;
    /* +0x060 */ s16 scalex;
    /* +0x062 */ s16 scaley;
    /* +0x064 */ s32 unk64;       /* GsBG rotate, 20.12 fixed point */
} Obj6F2C4;

/* D_8006F2C4 +0x008: constructor -- Class6B5CC's, then this table, then
 * slot +0x040 with the two arguments. */
void BgLayer__BgLayer(Obj6F2C4 *self, s32 arg1, s32 arg2) {
    GetClass6B5CCMethods()->ctor((Class6B5CC *)self);
    self->methods = GetBgLayerMethods();
    ((void (*)())self->methods->reset)(self, arg1, arg2);
}
/* D_8006F2C4 +0x040: reset -- lay out the GsBG at +0x044 over a map
 * source: mode 0 sizes it to the map (cell size x cell count), mode 1 to a
 * 320 x 240 screen (with its own attribute); then zero position and
 * scroll, take the colour in gBgLayerDefaultColor, point it at the source's GsMAP
 * (+0x2C), unit scale, no rotation, and centre the pivot. */
typedef struct Map44294 {
    /* +0x00 */ u8 pad0[0x2C];
    /* +0x2C */ u8 cellw;         /* a GsMAP from here */
    /* +0x2D */ u8 cellh;
    /* +0x2E */ u16 ncellw;
    /* +0x30 */ u16 ncellh;
} Map44294;

extern Vec3S8 gBgLayerDefaultColor;

void BgLayer__Reset(Obj6F2C4 *self, Map44294 *src, s32 mode) {
    if (mode == 0) {
        self->bgAttribute = 0x1000000;
        self->w = src->cellw * src->ncellw;
        self->h = src->cellh * src->ncellh;
    } else if (mode == 1) {
        self->bgAttribute = 0x2000000;
        self->w = 320;
        self->h = 240;
    }
    self->x = 0;
    self->y = 0;
    self->scrollx = 0;
    self->scrolly = 0;
    self->unk54 = gBgLayerDefaultColor;
    self->map = &src->cellw;
    self->scalex = 0x1000;
    self->scaley = 0x1000;
    self->unk64 = 0;
    self->mx = self->w / 2;
    self->my = self->h / 2;
}
/* D_8006F2C4 +0x044: the ratio of two halfwords of `src` (+0x08 over
 * +0x0A) in 20.12 fixed point, stored at +0x64 when `set`, else added. */
typedef struct Ratio44380 {
    /* +0x00 */ u8 pad0[8];
    /* +0x08 */ s16 num;
    /* +0x0A */ s16 den;
} Ratio44380;

void BgLayer__SetRotation(Obj6F2C4 *self, s32 set, Ratio44380 *src) {
    s32 num = src->num;
    s32 den = src->den;
    s32 v = ((num / den) << 12) + (((num % den) << 12) / den);

    if (set) {
        self->unk64 = v;
    } else {
        self->unk64 += v;
    }
}
/* D_8006F2C4 +0x048: the two ratios of `src` (+0 over +2, +4 over +6) in
 * 20.12 fixed point become the GsBG's scale -- stored when `set` (0x1000
 * for a zero divisor, at most 30000), else added, a sum over 30000 giving
 * 30000, or 1 when either term of that ratio was negative. */
typedef struct Scale4441C {
    /* +0x00 */ s16 xnum;
    /* +0x02 */ s16 xden;
    /* +0x04 */ s16 ynum;
    /* +0x06 */ s16 yden;
} Scale4441C;

void BgLayer__SetScale(Obj6F2C4 *self, s32 set, Scale4441C *src) {
    s32 negX;
    s32 negY;
    s32 den;
    s16 sx;
    s16 sy;
    s32 v;

    negX = 0;
    negY = 0;
    if (src->xnum < 0 || src->xden < 0) {
        negX = 1;
    }
    if (src->ynum < 0 || src->yden < 0) {
        negY = 1;
    }
    den = src->xden;
    if (den != 0) {
        sx = ((src->xnum / den) << 12) + (((src->xnum % den) << 12) / den);
    }
    if (src->yden != 0) {
        sy = ((src->ynum / src->yden) << 12) + (((src->ynum % src->yden) << 12) / src->yden);
    }
    if (set) {
        if (den == 0) {
            self->scalex = 0x1000;
        } else {
            v = sx;
            if (v > 30000) {
                v = 30000;
            }
            self->scalex = v;
        }
        if (src->yden == 0) {
            self->scaley = 0x1000;
        } else {
            v = sy;
            if (v > 30000) {
                v = 30000;
            }
            self->scaley = v;
        }
    } else {
        if (self->scalex + sx > 30000) {
            if (negX) {
                self->scalex = 1;
            } else {
                self->scalex = 30000;
            }
        } else {
            self->scalex = sx + self->scalex;
        }
        if (self->scaley + sy > 30000) {
            if (negY) {
                self->scaley = 1;
            } else {
                self->scaley = 30000;
            }
        } else {
            self->scaley = sy + self->scaley;
        }
    }
}
/* D_8006F2C4 (a Class6B5CC subclass) +0x0B8: when `enable`, copy a
 * three-byte vector to +0x54. */
void BgLayer__SetColor(Obj6F2C4 *self, s32 enable, Vec3S8 *src) {
    if (enable) {
        self->unk54 = *src;
    }
}
void BgLayer__NoOp(void) {
}
extern s32 D_8006F2C4[];

void *GetBgLayerMethods(void) {
    return D_8006F2C4;
}
/* Allocate and construct a D_8006F384 object (second constructor argument 1); freed and NULL when the constructor fails. */
ModelData *New_ModelData(Src6F240 *src) {
    void *obj = BMemPMgrAlloc(0x38);

    if (obj != NULL) {
        if (((Ctor33808 *)GetModelDataMethods())->ctor(obj, src, 1)) {
            return obj;
        }
        BMemPMgrFree(obj);
    }
    return NULL;
}
/* D_8006F384 +0x008: constructor -- the active driver's, then this table,
 * `owns` at +0x34; adopt the descriptor's buffer (size 0) and run its own
 * +0x064, whose nonzero result fails the construction (NULL), or else
 * request its file. */
void *ModelData__ModelData(ModelData *self, Src6F240 *src, s32 owns) {
    GetActiveDataSourceMethods()->ctor((Class6D430 *)self);
    self->methods = GetModelDataMethods();
    self->ownsResources = owns;
    if (src->buffer != NULL) {
        self->buffer = src->buffer;
        self->bufferSize = 0;
        if (((s32 (*)())self->methods->setFlag)(self)) {
            goto fail;
        }
    } else {
        self->methods->requestLoadFile(self, src->name);
    }
    return self;
fail:
    return NULL;
}
/* D_8006F384 +0x00C: finalize -- slot +0x07C, then the active driver's. */
void ModelData__Finalize(ModelData *self) {
    self->methods->releaseResources(self);
    GetActiveDataSourceMethods()->finalize((Class6D430 *)self);
}
/* D_8006F384 +0x064: the active driver's setFlag, then slot +0x078. */
void ModelData__Load(ModelData *self) {
    GetActiveDataSourceMethods()->setFlag((Class6D430 *)self);
    ((s32 (*)())self->methods->slot78)(self);
}
/* D_8006F384 +0x078: when +0x34 is set, build a D_8006F13C source over the
 * buffer's sub-block (at the offset in its third word) and a D_8006F590 one
 * over the buffer past +0x0C, into +0x2C and +0x30; 0 when both exist,
 * otherwise slot +0x07C (release) and 1. */
typedef struct Req44858 {
    /* +0x00 */ void *buffer;
    /* +0x04 */ s32 unk4;
    /* +0x08 */ s32 unk8;
} Req44858;

typedef struct Buf44858 {
    /* +0x00 */ u8 pad0[8];
    /* +0x08 */ s32 offset;
} Buf44858;

extern Req44858 *SetVec3(Req44858 *req, void *buffer, s32 unk4, s32 unk8);  /* code_171e0.c: stores its three words into *req, returns req */

s32 ModelData__BuildResources(ModelData *self) {
    Req44858 req;

    if (self->ownsResources != 0) {
        SetVec3(&req, (u8 *)self->buffer + ((Buf44858 *)self->buffer)->offset, 0, 1);
        self->linkResource = New_LinkResource((s32)&req);
        if (self->linkResource != NULL) {
            req.buffer = (u8 *)self->buffer + 0xC;
            self->todSet = (Class6D430 *)New_TodSet((Src6F240 *)&req);
            if (self->todSet != NULL) {
                return 0;
            }
            self->todSet = NULL;
        }
        self->methods->releaseResources(self);
        return 1;
    }
    return 0;
}
/* D_8006F384 +0x07C: when +0x34 is set, release the objects at +0x30 and
 * +0x2C (each when there is one). */
void ModelData__ReleaseResources(ModelData *self) {
    if (self->ownsResources != 0) {
        if (self->todSet != NULL) {
            self->todSet->methods->release(self->todSet);
        }
        if (self->linkResource != NULL) {
            self->linkResource->methods->release(self->linkResource);
        }
    }
}
/* D_8006F384/D_8006F40C +0x080: forwarded to slot +0x078 of the object at +0x30. */
u8 ModelData__ForwardScanPackets(ModelData *self, s32 arg1, s32 arg2) {
    return ((s32 (*)())self->todSet->methods->slot78)(self->todSet, arg1, arg2);
}
/* D_8006F384/D_8006F40C +0x084: forwarded to slot +0x080 of the object at +0x30. */
void *ModelData__ForwardDecodePacketWord(ModelData *self, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5) {
    return ((TodSet *)self->todSet)->methods->decodePacketWord((TodSet *)self->todSet, (u32 *)arg1, (u8 *)arg2, (u8 *)arg3, (u8 *)arg4, (u8 *)arg5);
}
ModelDataMethods *GetModelDataMethods(void) {
    return &D_8006F384;
}
/* Allocate and construct a TriggerWorld; freed and NULL when the constructor fails. */
TriggerWorld *New_TriggerWorld(Src6F240 *src) {
    TriggerWorld *obj = BMemPMgrAlloc(0x3C);

    if (obj != NULL) {
        if (((Ctor33808 *)GetTriggerWorldMethods())->ctor(obj, src)) {
            return obj;
        }
        BMemPMgrFree(obj);
    }
    return NULL;
}
/* D_8006F40C +0x008: constructor -- ModelData's (third argument 0: not
 * owning), then this table; when the descriptor has a buffer, its own
 * +0x064 runs, and a nonzero result fails the construction (NULL). */
void *TriggerWorld__TriggerWorld(TriggerWorld *self, Src6F240 *src) {
    ((Ctor33808 *)GetModelDataMethods())->ctor(self, src, 0);
    self->methods = GetTriggerWorldMethods();
    if (src->buffer != NULL) {
        if (((s32 (*)())self->methods->setFlag)(self)) {
            return NULL;
        }
    }
    return self;
}
/* D_8006F40C +0x00C: finalize -- releaseResources, then ModelData's. */
void TriggerWorld__Finalize(TriggerWorld *self) {
    self->methods->releaseResources(self);
    GetModelDataMethods()->finalize((ModelData *)self);
}
/* D_8006F40C +0x064: slot +0x078 (TriggerWorld__BuildResources). */
void TriggerWorld__Load(TriggerWorld *self) {
    ((s32 (*)())self->methods->slot78)(self);
}
/* D_8006F40C +0x078: build a ModelData (not owning) over each sub-block of
 * the buffer's counted offset table, into the table's own words, counting
 * them at +0x38; 0 when all exist, otherwise releaseResources and 1. */
s32 TriggerWorld__BuildResources(TriggerWorld *self) {
    Req44858 req;
    CountedBuf33808 *buf;
    s32 *p;
    s32 i;
    s32 n;

    SetVec3(&req, 0, 0, 1);
    buf = self->buffer;
    i = 0;
    n = buf->count;
    p = buf->entries;
    self->modelDataCount = 0;
    for (; i < n; i++) {
        req.buffer = (u8 *)self->buffer + ((CountedBuf33808 *)self->buffer)->entries[i];
        *p = (s32)New_ModelData((Src6F240 *)&req);
        if (*p == 0) {
            goto fail;
        }
        self->modelDataCount++;
        p++;
    }
    return 0;
fail:
    self->methods->releaseResources(self);
    return 1;
}
/* D_8006F40C +0x07C: release the ModelData array in the buffer (past its
 * first two words), modelDataCount entries long, and zero the count. */
void TriggerWorld__ReleaseResources(TriggerWorld *self) {
    ReleaseBasicClassArray((BasicClass **)((u8 *)self->buffer + 8), self->modelDataCount);
    self->modelDataCount = 0;
}
/* D_8006F40C +0x088: entry `index` of the buffer's counted word array (a
 * ModelData once BuildResources has run), 0 when out of range. */
ModelData *TriggerWorld__GetModelData(TriggerWorld *self, u32 index) {
    CountedBuf33808 *buf = self->buffer;

    if (index < buf->count) {
        return (ModelData *)buf->entries[index];
    }
    return NULL;
}
TriggerWorldMethods *GetTriggerWorldMethods(void) {
    return &D_8006F40C;
}
/* Allocate and construct a D_8006F498 object. */
void *New_TileMap(s32 arg0, s32 arg1) {
    void *obj = BMemPMgrAlloc(0x44);

    if (obj != NULL) {
        ((Ctor33808 *)GetTileMapMethods())->ctor(obj, arg0, arg1);
        return obj;
    }
    return NULL;
}
/* The D_8006F498 object. */
typedef struct Obj6F498 {
    CLASS6D430_FIELDS(DataSrc33808Methods);
    /* +0x02C */ u8 unk2C;
    /* +0x02D */ u8 unk2D;
    /* +0x02E */ u16 unk2E;
    /* +0x030 */ u16 unk30;
    /* +0x032 */ u8 pad32[2];
    /* +0x034 */ s32 unk34;
    /* +0x038 */ u16 *unk38;
    /* +0x03C */ s32 unk3C;       /* an object: its +0x2C is read */
    /* +0x040 */ u16 unk40;
    /* +0x042 */ u16 unk42;
} Obj6F498;

/* D_8006F498 +0x008: constructor -- the active driver's, then this table;
 * store `arg2` at +0x3C, clear +0x42, and with no `arg1` set +0x40, clear
 * +0x2A and run its own +0x064. */
void TileMap__TileMap(Obj6F498 *self, s32 arg1, s32 arg2) {
    s32 unused[8];

    GetActiveDataSourceMethods()->ctor((Class6D430 *)self);
    self->methods = GetTileMapMethods();
    self->unk3C = arg2;
    self->unk42 = 0;
    if (arg1 == 0) {
        self->unk40 = 1;
        self->unk2A = 0;
        self->methods->setFlag((DataSrc33808 *)self);
    }
}
/* D_8006F498 +0x00C: finalize -- free +0x38, then the active driver's. */
void TileMap__Finalize(DataSrc33808 *self) {
    BMemPMgrFree((void *)self->unk38);
    GetActiveDataSourceMethods()->finalize((Class6D430 *)self);
}
/* D_8006F498 +0x064: unless +0x2A is set, slot +0x078 and mark +0x42. */

void TileMap__Load(Obj6F498 *self) {
    if (self->unk2A == 0) {
        ((void (*)())self->methods->slot78)();
        self->unk42 = 1;
    }
}
/* D_8006F498 +0x078: copy +0x2C of the object at +0x3C to +0x34; when +0x40
 * is set, lay out a 20 x 15 grid (16 x 16 cells) and fill an allocated
 * index table 0..n-1 at +0x38; otherwise, or when the allocation fails,
 * free the buffer (own +0x05C). */
void TileMap__BuildMap(Obj6F498 *self) {
    s32 n;
    s32 i;
    u16 *p;

    self->unk34 = ((DataSrc33808 *)self->unk3C)->unk2C;
    if (self->unk40 != 0) {
        self->unk2E = 20;
        self->unk2C = 16;
        self->unk2D = 16;
        self->unk30 = 15;
        n = self->unk2E * self->unk30;
        self->unk38 = BMemPMgrAlloc(n * 2);
        if (self->unk38 != NULL) {
            p = self->unk38;
            for (i = 0; i < n; i++) {
                *p++ = i;
            }
            return;
        }
    }
    self->methods->freeBuffer((DataSrc33808 *)self);
}
extern s32 D_8006F498[];

void *GetTileMapMethods(void) {
    return D_8006F498;
}
/* Allocate and construct a D_8006F514 object. */
void *New_TileAtlas(s32 arg0) {
    void *obj = BMemPMgrAlloc(0x38);

    if (obj != NULL) {
        ((Ctor33808 *)GetTileAtlasMethods())->ctor(obj, arg0);
        return obj;
    }
    return NULL;
}
/* The D_8006F514 object. */
typedef struct Obj6F514 {
    CLASS6D430_FIELDS(DataSrc33808Methods);
    /* +0x02C */ struct Cell450B4 *cells;  /* 300 GsCELLs, built by TileAtlas__BuildCells */
    /* +0x030 */ u16 unk30;
    /* +0x032 */ u16 unk32;
    /* +0x034 */ s32 unk34;
} Obj6F514;

/* D_8006F514 +0x008: constructor -- the active driver's, then this table;
 * clear +0x34/+0x32, and with no `arg` set +0x30, clear +0x2A and run its
 * own +0x064. */
void TileAtlas__TileAtlas(Obj6F514 *self, s32 arg) {
    s32 unused[8];

    GetActiveDataSourceMethods()->ctor((Class6D430 *)self);
    self->methods = GetTileAtlasMethods();
    self->unk34 = 0;
    self->unk32 = 0;
    if (arg == 0) {
        self->unk30 = 1;
        self->unk2A = 0;
        self->methods->setFlag((DataSrc33808 *)self);
    }
}
/* D_8006F514 +0x00C: finalize -- free +0x34 and +0x2C, then the active
 * driver's. */
void TileAtlas__Finalize(DataSrc33808 *self) {
    BMemPMgrFree((void *)self->unk34);
    BMemPMgrFree((void *)self->unk2C);
    GetActiveDataSourceMethods()->finalize((Class6D430 *)self);
}
/* D_8006F514 +0x064: unless +0x2A is set, slot +0x078 and mark +0x32. */

void TileAtlas__Load(Obj6F514 *self) {
    s32 unused[8];

    if (self->unk2A == 0) {
        ((void (*)())self->methods->slot78)();
        self->unk32 = 1;
    }
}
/* D_8006F514 +0x078: when +0x30 is set, build 300 GsCELLs (16 x 16 texels
 * each) at +0x2C over the texture pages from x 0x280: u,v step by 16, a new
 * row at x 0x3C0, a new texture page every 64 x (the lower half from v
 * 0x100). */
typedef struct Cell450B4 {       /* LIBGS.H GsCELL */
    /* +0x00 */ u8 u;
    /* +0x01 */ u8 v;
    /* +0x02 */ u16 cba;
    /* +0x04 */ u16 flag;
    /* +0x06 */ u16 tpage;
} Cell450B4;

/* LIBGPU.H */
extern u16 GetTPage(int tp, int abr, int x, int y);

void TileAtlas__BuildCells(Obj6F514 *self) {
    Cell450B4 *c;
    s32 x = 0x280;
    s32 u;
    s32 v;
    s32 tpage;
    s32 i;
    s32 n;

    if (self->unk30 != 0) {
        v = 0;
        u = 0;
        tpage = GetTPage(2, 0, 0x280, 0);
        self->cells = BMemPMgrAlloc(300 * sizeof(Cell450B4));
        if (self->cells != NULL) {
            i = 0;
            c = self->cells;
            n = 300;
            for (; i < n; i++, c++) {
                c->u = u;
                c->tpage = tpage;
                c->v = v;
                c->cba = 0;
                c->flag = 0;
                u += 16;
                x += 16;
                if (x >= 0x3C0) {
                    u = 0;
                    x = 0x280;
                    v += 16;
                }
                if ((x & 0x3F) == 0) {
                    tpage = x >> 6;
                    if (v >= 0x100) {
                        tpage += 16;
                    }
                    u = 0;
                }
            }
        }
    }
}
extern s32 D_8006F514[];

void *GetTileAtlasMethods(void) {
    return D_8006F514;
}
/* Allocate and construct a TodSet; freed and NULL when the constructor fails. */
TodSet *New_TodSet(Src6F240 *src) {
    void *obj = BMemPMgrAlloc(0x2C);

    if (obj != NULL) {
        if (((Ctor33808 *)GetTodSetMethods())->ctor(obj, src)) {
            return obj;
        }
        BMemPMgrFree(obj);
    }
    return NULL;
}
/* D_8006F590 +0x008: constructor -- the parent Tod's, then this table; when
 * the descriptor holds a buffer, its own +0x064 (TodSet__BuildTods) runs,
 * and a nonzero result fails the construction (NULL). */
void *TodSet__TodSet(TodSet *self, Src6F240 *src) {
    GetTodMethods()->ctor((Tod *)self, src);
    self->methods = GetTodSetMethods();
    if (src->buffer != NULL) {
        if (((s32 (*)())self->methods->setFlag)(self)) {
            return NULL;
        }
    }
    return self;
}
/* D_8006F590 +0x00C: finalize -- release the buffer's counted Tod array,
 * then the parent Tod's. */
void TodSet__Finalize(TodSet *self) {
    CountedBuf33808 *buf = self->buffer;

    ReleaseBasicClassArray((BasicClass **)buf->entries, buf->count);
    GetTodMethods()->finalize((Tod *)self);
}
/* D_8006F590 +0x064: build a Tod over each sub-block of the buffer's
 * counted offset table, into the table's own words; 0 when all exist,
 * otherwise release the ones already built and 1. */
s32 TodSet__BuildTods(TodSet *self) {
    Req44858 req;
    CountedBuf33808 *buf;
    Tod **p;
    s32 i;
    s32 n;

    SetVec3(&req, 0, 0, 1);
    buf = self->buffer;
    i = 0;
    n = buf->count;
    p = (Tod **)buf->entries;
    for (; i < n; i++) {
        req.buffer = (u8 *)self->buffer + ((CountedBuf33808 *)self->buffer)->entries[i];
        *p = New_Tod((Src6F240 *)&req);
        if (*p == NULL) {
            while (i != 0) {
                i--;
                p--;
                (*p)->methods->release(*p);
            }
            return 1;
        }
        p++;
    }
    return 0;
}
/* D_8006F590 +0x078: Tod's +0x07C scanner over the data past the buffer's
 * counted array. */
u8 TodSet__ScanPackets(TodSet *self, u8 *out, u32 *sel) {
    CountedBuf33808 *buf = self->buffer;

    return self->methods->scanTodPackets(self, out, sel, (u32 *)&buf->entries[buf->count] + 2);
}
TodSetMethods *GetTodSetMethods(void) {
    return &D_8006F590;
}
/* Allocate and construct a D_8006F614 object; freed and NULL when the
 * constructor returns nonzero (this ctor reports failure, not self). */
void *GetMoviePlayerMethods(void);

void *New_MoviePlayer(s32 arg0, s32 arg1, s32 arg2) {
    void *obj = BMemPMgrAlloc(0x6C);

    if (obj != NULL) {
        if (((Ctor33808 *)GetMoviePlayerMethods())->ctor(obj, arg0, arg1, arg2) == 0) {
            return obj;
        }
        BMemPMgrFree(obj);
    }
    return NULL;
}
/* D_8006F614 +0x008: constructor -- BasicClass's, then this table; open a
 * CD stream object (New_CdStream(arg2, 15, 0)) at +0x60 and set up the
 * decode buffers (MoviePlayer__InitFrame); 1 when either fails. Then reset the MDEC
 * the first time any player is built (gMdecInitialized), route its output
 * callback to OnMdecFrameReady, hand the stream the ring buffer at +0x10
 * (0x12000), clear +0x50 and store 1 through its own +0x06C. 0. */
typedef struct Methods454C4 {
    /* +0x000 */ u8 pad0[0x6C];
    /* +0x06C */ void (*slot6C)();
} Methods454C4;

typedef struct Obj454C4 {
    /* +0x000 */ Methods454C4 *methods;
    /* +0x004 */ u8 pad4[0xC];
    /* +0x010 */ void *ring;
    /* +0x014 */ u8 pad14[0x3C];
    /* +0x050 */ s32 unk50;
    /* +0x054 */ u8 pad54[0xC];
    /* +0x060 */ CdStream *stream;
} Obj454C4;

s32 MoviePlayer__InitFrame();
extern s32 gMdecInitialized;
extern void DecDCTReset(int mode);
extern int DecDCToutCallback(void (*func)());
void OnMdecFrameReady(void);

s32 MoviePlayer__MoviePlayer(Obj454C4 *self, s32 arg1, s32 arg2, s32 arg3) {
    Get_vtable_BasicClass()->ctor((BasicClass *)self);
    self->methods = GetMoviePlayerMethods();
    self->stream = New_CdStream(arg2, 15, 0);
    if (self->stream != NULL) {
        if (MoviePlayer__InitFrame(self, arg1, arg3) == 0) {
            if (gMdecInitialized == 0) {
                DecDCTReset(0);
            }
            gMdecInitialized = 1;
            DecDCToutCallback(OnMdecFrameReady);
            self->stream->methods->setRing(self->stream, self->ring, 0x12000);
            self->unk50 = 0;
            self->methods->slot6C(self, 1);
            return 0;
        }
    }
    return 1;
}
/* D_8006F614 +0x00C: finalize -- release the object at +0x60, detach and
 * reset the MDEC decoder, free the four buffers (MoviePlayer__FreeFrameBuffers), then
 * BasicClass's finalize. */
typedef struct Obj455D4 {
    /* +0x000 */ u8 pad0[0x60];
    /* +0x060 */ CdStream *unk60;
} Obj455D4;

/* LIBPRESS.H */
extern void DecDCTReset(int mode);
extern int DecDCToutCallback(void (*func)());
void MoviePlayer__FreeFrameBuffers();

void MoviePlayer__Finalize(Obj455D4 *self) {
    self->unk60 = self->unk60->methods->release(self->unk60);
    DecDCToutCallback(NULL);
    DecDCTReset(0);
    MoviePlayer__FreeFrameBuffers(self);
    Get_vtable_BasicClass()->finalize((BasicClass *)self);
}
/* Set up the MDEC player's frame: keep `external` at +0x0C and, unless the
 * caller provides the buffers, allocate the two decode buffers (w * h * 2 +
 * 0x1000 each), the 0x12000 ring and the h * 32 strip buffer -- on a failure
 * free what was allocated (MoviePlayer__FreeFrameBuffers) and return 1. Then the frame
 * descriptor goes to +0x2C and +0x20, the strip at +0x2C is 16 wide and
 * +0x38 is its size in words. 0. */
typedef struct Frame4564C {
    /* +0x00 */ s16 x;
    /* +0x02 */ s16 y;
    /* +0x04 */ s32 w;
    /* +0x08 */ s32 h;
} Frame4564C;

typedef struct Obj4564C {
    /* +0x000 */ u8 pad0[0xC];
    /* +0x00C */ s32 external;
    /* +0x010 */ void *ring;
    /* +0x014 */ void *frames[2];
    /* +0x01C */ void *strip;
    /* +0x020 */ Frame4564C frame;
    /* +0x02C */ Frame4564C cur;
    /* +0x038 */ s32 stripSize;
} Obj4564C;

s32 MoviePlayer__InitFrame(Obj4564C *self, Frame4564C *desc, s32 external) {
    s32 size;
    s32 unused[2];

    self->external = external;
    if (external == 0) {
        self->strip = NULL;
        self->frames[1] = NULL;
        self->frames[0] = NULL;
        self->ring = NULL;
        size = desc->w * desc->h * 2 + 0x1000;
        if ((self->frames[0] = BMemPMgrAlloc(size)) == NULL) {
            goto fail;
        }
        if ((self->frames[1] = BMemPMgrAlloc(size)) == NULL) {
            goto fail;
        }
        if ((self->ring = BMemPMgrAlloc(0x12000)) == NULL) {
            goto fail;
        }
        if ((self->strip = BMemPMgrAlloc(desc->h << 5)) == NULL) {
            goto fail;
        }
    }
    self->cur = *desc;
    self->frame = self->cur;
    self->cur.w = 16;
    self->stripSize = (self->cur.h << 4) >> 1;
    return 0;
fail:
    MoviePlayer__FreeFrameBuffers(self);
    return 1;
}
/* Unless +0x0C is set, free the four allocations at +0x14, +0x18, +0x10,
 * +0x1C. Not referenced by any data word. */
typedef struct Obj4575C {
    /* +0x000 */ u8 pad0[0xC];
    /* +0x00C */ s32 unkC;
    /* +0x010 */ void *unk10;
    /* +0x014 */ void *unk14;
    /* +0x018 */ void *unk18;
    /* +0x01C */ void *unk1C;
} Obj4575C;

void MoviePlayer__FreeFrameBuffers(Obj4575C *self) {
    if (self->unkC == 0) {
        BMemPMgrFree(self->unk14);
        BMemPMgrFree(self->unk18);
        BMemPMgrFree(self->unk10);
        BMemPMgrFree(self->unk1C);
    }
}
/* D_8006F614 +0x040: start playing -- only when no movie is active
 * (gActiveMoviePlayer): optionally MoviePlayer__MarkPlaying first (+0x68), keep `arg2` at
 * +0x5C, open `name` on the stream object at +0x60 (its +0x044, 100); 1 when
 * that fails. Otherwise become the active movie, reset the state words, keep
 * `arg3`/`arg4` at +0x54/+0x58 and clear the frame rectangle (+0x20) through
 * DrawSystem's clearImage (+0x078), whose color argument is gMovieFrameRect
 * (a zero word: black; the name predates reading the slot). 0. */
typedef struct Obj457C0 {
    /* +0x000 */ u8 pad0[0x20];
    /* +0x020 */ s16 rect[4];
    /* +0x028 */ u8 pad28[0x14];
    /* +0x03C */ s32 unk3C;
    /* +0x040 */ s32 unk40;
    /* +0x044 */ s32 unk44;
    /* +0x048 */ s32 unk48;
    /* +0x04C */ s32 unk4C;
    /* +0x050 */ s32 unk50;
    /* +0x054 */ s32 unk54;
    /* +0x058 */ s32 unk58;
    /* +0x05C */ s32 unk5C;
    /* +0x060 */ CdStream *unk60;
    /* +0x064 */ s32 unk64;
    /* +0x068 */ s32 unk68;
} Obj457C0;

extern DataSrc33808 *gActiveMoviePlayer;
extern s32 gMovieFrameRect;
void MoviePlayer__MarkPlaying();

s32 MoviePlayer__Play(Obj457C0 *self, char *name, s32 arg2, s32 arg3, s32 arg4) {
    DrawSystem *ds;

    if (gActiveMoviePlayer == NULL) {
        if (self->unk68 != 0) {
            MoviePlayer__MarkPlaying(self);
        }
        self->unk5C = arg2;
        if (self->unk60->methods->open(self->unk60, name, 100) == 0) {
            gActiveMoviePlayer = (DataSrc33808 *)self;
            self->unk40 = 0;
            self->unk3C = 0;
            self->unk4C = 1;
            self->unk48 = 0;
            self->unk44 = 0;
            self->unk54 = arg3;
            self->unk58 = arg4;
            ds = GetDrawSystem();
            ds->methods->clearImage(ds, (u8 *)&gMovieFrameRect, (DrawRect *)self->rect);
            return 0;
        }
        return 1;
    }
    return 0;
}
/* A class with an s32 at +0x50, set to 1 / -1 by the two setters below;
 * the class is not yet identified (neither setter sits in a method table). */
typedef struct Obj33808_50 {
    u8 pad0[0x50];
    s32 unk50;
} Obj33808_50;

void MoviePlayer__MarkPlaying(Obj33808_50 *self) {
    self->unk50 = 1;
}
/* D_8006F614 +0x044: when this is the object in gActiveMoviePlayer, reset its
 * state words, hand the object at +0x60 MoviePlayer__MarkStopped (and self) through
 * that object's +0x07C, clear +0x64, and call its +0x058. */
typedef struct Obj458B8 {
    /* +0x000 */ u8 pad0[0x3C];
    /* +0x03C */ s32 unk3C;
    /* +0x040 */ s32 unk40;
    /* +0x044 */ s32 unk44;
    /* +0x048 */ s32 unk48;
    /* +0x04C */ s32 unk4C;
    /* +0x050 */ s32 unk50;
    /* +0x054 */ s32 unk54;
    /* +0x058 */ u8 pad58[8];
    /* +0x060 */ CdStream *unk60;
    /* +0x064 */ s32 unk64;
} Obj458B8;

extern DataSrc33808 *gActiveMoviePlayer;
void MoviePlayer__MarkStopped();

void MoviePlayer__Stop(Obj458B8 *self) {
    Obj458B8 *cur = (Obj458B8 *)gActiveMoviePlayer;

    if (cur == self) {
        cur->unk40 = 0;
        cur->unk3C = 0;
        cur->unk4C = 1;
        cur->unk48 = 0;
        cur->unk44 = 0;
        cur->unk60->methods->slot7C(cur->unk60, MoviePlayer__MarkStopped, cur);
        cur->unk64 = 0;
        cur->unk60->methods->restart(cur->unk60);
    }
}
void MoviePlayer__MarkStopped(Obj33808_50 *self) {
    self->unk50 = -1;
}
/* D_8006F614 +0x048: when this is the object in gActiveMoviePlayer -- with the
 * stream running (+0x50), call the stream object's +0x050 (1, +0x5C); if
 * +0x50 then went negative, count down the loops left at +0x58 and at the
 * last one (or with none) call the stream's +0x064; clear +0x50, set +0x64,
 * 0. Stopped with +0x64 set: tail-return its own +0x068. */
typedef struct Methods45948 {
    /* +0x000 */ u8 pad0[0x68];
    /* +0x068 */ s32 (*slot68)();
} Methods45948;

typedef struct Obj45948 {
    /* +0x000 */ Methods45948 *methods;
    /* +0x004 */ u8 pad4[0x4C];
    /* +0x050 */ s32 unk50;
    /* +0x054 */ s32 unk54;
    /* +0x058 */ s32 loops;
    /* +0x05C */ s32 unk5C;
    /* +0x060 */ CdStream *unk60;
    /* +0x064 */ s32 unk64;
} Obj45948;

s32 MoviePlayer__Advance(Obj45948 *self) {
    Obj45948 *cur = (Obj45948 *)gActiveMoviePlayer;

    if (cur == self) {
        if (cur->unk50 == 0) {
            if (cur->unk64 == 0) {
                goto out;
            }
        } else {
            cur->unk60->methods->startRead(cur->unk60, 1, cur->unk5C);
            if (cur->unk50 < 0) {
                if (cur->loops == 0 || --cur->loops == 0) {
                    cur->unk60->methods->mute(cur->unk60);
                }
            }
            self->unk50 = 0;
            self->unk64 = 1;
            return 0;
        }
        return cur->methods->slot68(cur);
    }
out:
    ;
}
/* D_8006F614 +0x04C: when this is the object in gActiveMoviePlayer, set +0x48,
 * clear +0x54, call the +0x60 object's +0x048, set +0x44, and the first
 * time (+0x64 clear) clear that object's +0x07C callback and set +0x64. */
void MoviePlayer__Abort(Obj458B8 *self) {
    Obj458B8 *cur = (Obj458B8 *)gActiveMoviePlayer;

    if (cur == self) {
        cur->unk48 = 1;
        cur->unk54 = 0;
        cur->unk60->methods->close(cur->unk60);
        cur->unk44 = 1;
        if (cur->unk64 == 0) {
            cur->unk60->methods->slot7C(cur->unk60, 0, 0);
            cur->unk64 = 1;
            cur->unk44 = 1;
        }
    }
}
void MoviePlayer__NoOpSlot50(void) {
}
void MoviePlayer__NoOpSlot54(void) {
}
/* D_8006F614 +0x058: unless the stream has ended (+0x48), pull the next
 * frame from the object at +0x60 (its +0x06C); 1 when there is none. With
 * data, flip the frame index at +0x3C and VLC-decode into that frame's
 * buffer, then hand the sector buffer back (+0x070); a negative result
 * marks the end (+0x48) and calls that object's +0x054. 0. */
typedef struct Obj45AD8 {
    /* +0x000 */ u8 pad0[0x14];
    /* +0x014 */ u32 *frames[2];
    /* +0x01C */ u8 pad1C[0x20];
    /* +0x03C */ s32 frameIndex;
    /* +0x040 */ u8 pad40[8];
    /* +0x048 */ s32 unk48;
    /* +0x04C */ u8 pad4C[0x14];
    /* +0x060 */ CdStream *unk60;
} Obj45AD8;

/* LIBPRESS.H */
extern int DecDCTvlc(u32 *bs, u32 *buf);

s32 MoviePlayer__PullFrame(Obj45AD8 *self) {
    u32 *data;
    s32 size;
    s32 r;

    if (self->unk48 == 0) {
        r = self->unk60->methods->getNextFrame(self->unk60, &data, &size, 0x800000);
        if (r != 0) {
            if (size != 0) {
                self->frameIndex ^= 1;
                DecDCTvlc(data, self->frames[self->frameIndex]);
            }
            self->unk60->methods->freeRing(self->unk60, data);
            if (r < 0) {
                self->unk48 = 1;
                self->unk60->methods->stop(self->unk60);
            }
            return 0;
        }
    }
    return 1;
}
void MoviePlayer__NoOpFreeBuffer(void) {
}
/* D_8006F614 +0x060: upload the decoded strip at +0x1C into the rectangle
 * at +0x2C (DrawSystem +0x058), step the rectangle right by its width, and
 * while it is still inside the frame (+0x20 + +0x24) decode the next strip
 * (DecDCTout, after a DrawSync when +0x34 is under 0x80); at the end,
 * rewind the rectangle to +0x20/+0x22 and flag the frame done. */
typedef struct Rect45BC8 {
    s16 x;
    s16 y;
    s16 w;
    s16 h;
} Rect45BC8;

typedef struct Obj45BC8 {
    /* +0x000 */ u8 pad0[0x1C];
    /* +0x01C */ u32 *strip;
    /* +0x020 */ s16 x0;
    /* +0x022 */ s16 y0;
    /* +0x024 */ s32 width;
    /* +0x028 */ u8 pad28[4];
    /* +0x02C */ Rect45BC8 rect;
    /* +0x034 */ s32 unk34;
    /* +0x038 */ s32 stripSize;
    /* +0x03C */ u8 pad3C[8];
    /* +0x044 */ s32 unk44;
    /* +0x048 */ s32 unk48;
    /* +0x04C */ s32 unk4C;
} Obj45BC8;

/* LIBGPU.H / LIBPRESS.H */
extern int DrawSync(int mode);
extern void DecDCTout(u32 *buf, int size);

void MoviePlayer__DrawStrip(Obj45BC8 *self) {
    DrawSystem *ds = GetDrawSystem();

    ds->methods->loadImage(ds, (DrawRect *)&self->rect, self->strip);
    self->rect.x += self->rect.w;
    if (self->rect.x < self->x0 + self->width) {
        if (self->unk34 < 0x80) {
            DrawSync(0);
        }
        DecDCTout(self->strip, self->stripSize);
    } else {
        self->unk4C = 1;
        self->rect.x = self->x0;
        self->rect.y = self->y0;
        if (self->unk48 != 0) {
            self->unk44 = 1;
        }
    }
}
/* D_8006F614 +0x064: while +0x54 is set, count calls in gMoviePollCounter and
 * once the count before the increment passes 100, resets it to 1 and calls
 * slot +0x044; returns 0. Otherwise
 * clears gActiveMoviePlayer and returns 1. */
typedef struct Methods45C94 {
    /* +0x000 */ u8 pad0[0x44];
    /* +0x044 */ void (*slot44)();
} Methods45C94;

typedef struct Obj45C94 {
    /* +0x000 */ Methods45C94 *methods;
    /* +0x004 */ u8 pad4[0x50];
    /* +0x054 */ s32 unk54;
} Obj45C94;

extern s32 gMoviePollCounter;
extern DataSrc33808 *gActiveMoviePlayer;

s32 MoviePlayer__PollActive(Obj45C94 *self) {
    if (self->unk54 != 0) {
        if (gMoviePollCounter++ > 100) {
            gMoviePollCounter = 1;
            self->methods->slot44(self);
        }
        return 0;
    }
    gActiveMoviePlayer = NULL;
    return 1;
}
/* D_8006F614 +0x068: when this is the object in gActiveMoviePlayer -- with a
 * finished frame pending (+0x44) run its own +0x064 and return that;
 * otherwise, when a frame is going (+0x40), wait for its last strip (+0x4C),
 * clear the flag, DrawSync when +0x34 is under 0x80, and feed the next
 * frame's bitstream (+0x14[+0x3C]) to DecDCTin and the first strip to
 * DecDCTout; then +0x40 = its own +0x058 returned 0, and 0. */
typedef struct Methods45CFC {
    /* +0x000 */ u8 pad0[0x58];
    /* +0x058 */ s32 (*slot58)();
    /* +0x05C */ u8 pad5C[8];
    /* +0x064 */ s32 (*slot64)();
} Methods45CFC;

typedef struct Obj45CFC {
    /* +0x000 */ Methods45CFC *methods;
    /* +0x004 */ u8 pad4[0x10];
    /* +0x014 */ u32 *frames[2];
    /* +0x01C */ u32 *strip;
    /* +0x020 */ u8 pad20[0x14];
    /* +0x034 */ s32 unk34;
    /* +0x038 */ s32 stripSize;
    /* +0x03C */ s32 frameIndex;
    /* +0x040 */ s32 unk40;
    /* +0x044 */ s32 unk44;
    /* +0x048 */ u8 pad48[4];
    /* +0x04C */ s32 unk4C;
} Obj45CFC;

/* LIBPRESS.H */
extern void DecDCTin(u32 *buf, int mode);

void MoviePlayer__WaitFrameReady(Obj45CFC *self);  /* defined below (ROM order) */

s32 MoviePlayer__DecodeFrame(Obj45CFC *self) {
    Obj45CFC *cur = (Obj45CFC *)gActiveMoviePlayer;

    if (cur == self) {
        if (cur->unk44 == 0) {
            if (cur->unk40 != 0) {
                MoviePlayer__WaitFrameReady(cur);
                cur->unk4C = 0;
                if (cur->unk34 < 0x80) {
                    DrawSync(0);
                }
                DecDCTin(cur->frames[cur->frameIndex], 2);
                DecDCTout(cur->strip, cur->stripSize);
            }
            self->unk40 = self->methods->slot58(self) == 0;
            return 0;
        }
        return cur->methods->slot64(cur);
    }
}
extern DataSrc33808 *gActiveMoviePlayer;

/* Slot +0x060 of the object in gActiveMoviePlayer, when there is one. */
void OnMdecFrameReady(void) {
    if (gActiveMoviePlayer != NULL) {
        ((void (*)())gActiveMoviePlayer->methods->slot60)(gActiveMoviePlayer);
    }
}
/* Hang until +0x4C is nonzero (it is read once). MoviePlayer__DecodeFrame's only call
 * passes its gActiveMoviePlayer object, so the parameter is that Obj45CFC view. */
void MoviePlayer__WaitFrameReady(Obj45CFC *self) {
    while (self->unk4C == 0) {
    }
}
/* D_8006F614 +0x06C: stores its argument at +0x68. */
typedef struct Obj6F614 {
    u8 pad0[0x68];
    s32 unk68;
} Obj6F614;

void MoviePlayer__SetResult(Obj6F614 *self, s32 value) {
    self->unk68 = value;
}
extern s32 D_8006F614[];

void *GetMoviePlayerMethods(void) {
    return D_8006F614;
}
