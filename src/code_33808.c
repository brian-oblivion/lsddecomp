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
 *   - LinkResource (gLinkResourceMethods): a NULL-ended array of TMD models
 *     (New_TmdModel), one per object of a loaded TMD (include/LinkResource.h,
 *     track 4, round 89).
 *   - TimArraySrc  (D_8006F1C4): an array of TimImage objects
 *     (code_2bb9c.c's New_TimImage), one per TimBlockSrc block
 *     (include/TimArraySrc.h, track 4, round 88).
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
 *     together in src/code_2c054.c's TaskCore__TaskCore (include/TileMap.h,
 *     include/TileAtlas.h, track 4, round 88).
 *
 * Two more classes, not Class6D430 subclasses:
 *
 *   - BgLayer (D_8006F2C4): a Class6B5CC subclass wrapping one GsBG
 *     scrolling background layer (its own fields are GsBG's own layout;
 *     include/BgLayer.h, track 4, round 88).
 *   - MoviePlayer (gMoviePlayerMethods): a BasicClass subclass driving CD-streamed,
 *     MDEC-decoded FMV playback (open a CD stream, decode/upload strips,
 *     play/stop/tick controls); called from code_2c054.c
 *     (include/MoviePlayer.h, track 4, round 89).
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
#include "TimImage.h"
#include "TimArraySrc.h"
#include "BgLayer.h"
#include "TileMap.h"
#include "TileAtlas.h"
#include "TmdModel.h"
#include "LinkResource.h"
#include "DrawSystem.h"
#include "CdStream.h"
#include "MoviePlayer.h"

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

void TimBlockSrc__AdvanceLoadState(TimBlockSrc *self) {
    TimArraySrc **p;
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
                p = self->blocks + n;
                *p = New_TimArraySrc(NULL);
                (*p)->buffer = self->sector;
                (*p)->bufferSize = 0;
                (*p)->clutBase = (s32)self->entries;
                n++;
                (*p)->methods->setFlag(*p);
                ((TimArraySrcUploadFn)(*p)->methods->slot78)(*p);
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
/* A data source's construction descriptor: an existing buffer to adopt, or
 * a file name to request. */
typedef struct Src6F240 {
    /* +0x00 */ void *buffer;
    /* +0x04 */ char *name;
} Src6F240;

/* Allocate and construct a LinkResource (gLinkResourceMethods) object; freed and NULL when the constructor fails. */
LinkResource *New_LinkResource(Src6F240 *src) {
    void *obj = BMemPMgrAlloc(0x30);

    if (obj != NULL) {
        if (((Ctor33808 *)GetLinkResourceMethods())->ctor(obj, src)) {
            return obj;
        }
        BMemPMgrFree(obj);
    }
    return NULL;
}
/* gLinkResourceMethods +0x008: constructor -- the active driver's, then this table;
 * with a descriptor, adopt its buffer (size 0) and run its own +0x064, whose
 * nonzero result fails the construction (NULL), or else request its file. */
void *LinkResource__LinkResource(LinkResource *self, Src6F240 *src) {
    GetActiveDataSourceMethods()->ctor((Class6D430 *)self);
    self->methods = GetLinkResourceMethods();
    if (src != NULL) {
        if (src->buffer != NULL) {
            self->buffer = src->buffer;
            self->bufferSize = 0;
            if (((LinkResourceBuildModelsFn)self->methods->setFlag)(self)) {
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
/* gLinkResourceMethods +0x00C: finalize -- release every model in the NULL-ended
 * array at +0x2C, free the array, then the active driver's. */
void LinkResource__Finalize(LinkResource *self) {
    TmdModel **models = self->models;

    while (*models != NULL) {
        (*models)->methods->release(*models);
        models++;
    }
    BMemPMgrFree(self->models);
    GetActiveDataSourceMethods()->finalize((Class6D430 *)self);
}
/* gLinkResourceMethods +0x064: build a NULL-ended array at +0x2C of one
 * New_TmdModel object per object of the TMD in the buffer, after mapping
 * the TMD (own +0x078); 1 when an allocation fails (everything built so far
 * released and the array freed), otherwise the active driver's setFlag
 * and 0. */
s32 LinkResource__BuildModels(LinkResource *self) {
    TmdModel **models;
    u32 i;

    models = BMemPMgrAlloc((((TmdFile *)self->buffer)->nobj + 1) * 4);
    if (models == NULL) {
        return 1;
    }
    self->models = models;
    ((LinkResourceMapModelFn)self->methods->slot78)(self);
    for (i = 0; i < ((TmdFile *)self->buffer)->nobj; i++) {
        *models = New_TmdModel(&((TmdFile *)self->buffer)->objects[i]);
        if (*models == NULL) {
            while (i != 0) {
                i--;
                models--;
                (*models)->methods->release(*models);
            }
            BMemPMgrFree(models);
            return 1;
        }
        models++;
    }
    *models = NULL;
    GetActiveDataSourceMethods()->setFlag((Class6D430 *)self);
    return 0;
}
/* LIBGS.H: void GsMapModelingData(unsigned long *p); */
void GsMapModelingData(u32 *p);

/* gLinkResourceMethods +0x078: map the TMD in the buffer (past its id word). */
void LinkResource__MapModel(LinkResource *self) {
    GsMapModelingData(&((TmdFile *)self->buffer)->flags);
}
/* gLinkResourceMethods +0x07C: the address of the TMD's object `index`,
 * 0x1C bytes each, from +0x0C of the buffer. */
TmdObject *LinkResource__GetTmdObject(LinkResource *self, s32 index) {
    return &((TmdFile *)self->buffer)->objects[index];
}
/* gLinkResourceMethods +0x080: model `index` of the array BuildModels
 * filled at +0x2C (the first field past the 0x2C-byte Class6D430 base). */
TmdModel *LinkResource__GetModel(LinkResource *self, s32 index) {
    return self->models[index];
}
void LinkResource__NoOp(void) {
}
LinkResourceMethods *GetLinkResourceMethods(void) {
    return &gLinkResourceMethods;
}
/* Allocate and construct a D_8006F1C4 object. */
TimArraySrc *New_TimArraySrc(char *name) {
    void *obj = BMemPMgrAlloc(0x3C);

    if (obj != NULL) {
        GetTimArraySrcMethods()->ctor(obj, name);
        return obj;
    }
    return NULL;
}
/* D_8006F1C4 +0x008: constructor -- the active driver's, then this table,
 * clear count/images/ready, and request `name` when there is one. */
void TimArraySrc__TimArraySrc(TimArraySrc *self, char *name) {
    GetActiveDataSourceMethods()->ctor((Class6D430 *)self);
    self->methods = GetTimArraySrcMethods();
    self->count = 0;
    self->images = NULL;
    self->ready = 0;
    if (name != NULL) {
        self->methods->requestLoadFile(self, name);
    }
}
/* D_8006F1C4 +0x00C: finalize -- same shape as D_8006F0B8's. */
void TimArraySrc__Finalize(TimArraySrc *self) {
    ReleaseBasicClassArray((BasicClass **)self->images, self->count);
    BMemPMgrFree(self->images);
    GetActiveDataSourceMethods()->finalize((Class6D430 *)self);
}
/* D_8006F1C4 +0x064: when the buffer is there (or flag 0x200 is set),
 * build one TimImage (New_TimImage(NULL)) per image of the buffer -- a
 * count, then that many offsets -- into `images` (`count` entries),
 * each adopting its image in place (size 0), and set each one's clutBase
 * from the CLUT row its GsGetTimInfo reports (from y 0x1E0, >>
 * gTimClutRowShift, 16 bytes a step past `clutBase`); then mark `ready`
 * and run the active driver's setFlag. */
extern s16 gTimClutRowShift;

void TimArraySrc__BuildImages(TimArraySrc *self) {
    GsIMAGE info;
    TimImage **objs;
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
                (*objs)->clutBase = ((info.cy - 0x1E0) >> gTimClutRowShift) * 16 + self->clutBase;
                offs++;
                objs++;
            }
            self->ready = 1;
            GetActiveDataSourceMethods()->setFlag((Class6D430 *)self);
        }
    }
}
/* D_8006F1C4 +0x078: every image's +0x078 (TimImage__Upload). */
void TimArraySrc__UploadImages(TimArraySrc *self) {
    TimImage **objs = self->images;
    s32 i;

    for (i = 0; i < self->count; i++) {
        ((TimImageUploadFn)(*objs)->methods->slot78)(*objs);
        objs++;
    }
}
TimArraySrcMethods *GetTimArraySrcMethods(void) {
    return &D_8006F1C4;
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
BgLayer *New_BgLayer(TileMap *src, s32 mode) {
    BgLayer *obj = BMemPMgrAlloc(0x68);

    if (obj != NULL) {
        GetBgLayerMethods()->ctor(obj, src, mode);
        return obj;
    }
    return NULL;
}
/* D_8006F2C4 +0x008: constructor -- Class6B5CC's, then this table, then
 * slot +0x040 with the two arguments. */
void BgLayer__BgLayer(BgLayer *self, TileMap *src, s32 mode) {
    GetClass6B5CCMethods()->ctor((Class6B5CC *)self);
    self->methods = GetBgLayerMethods();
    ((BgLayerResetFn)self->methods->reset)(self, src, mode);
}
/* D_8006F2C4 +0x040: reset -- lay out the GsBG at +0x044 over a map
 * source: mode 0 sizes it to the map (cell size x cell count), mode 1 to a
 * 320 x 240 screen (with its own attribute); then zero position and
 * scroll, take the colour in gBgLayerDefaultColor, point it at the source's GsMAP
 * (+0x2C), unit scale, no rotation, and centre the pivot. */

extern BgLayerRgb gBgLayerDefaultColor;

void BgLayer__Reset(BgLayer *self, TileMap *src, s32 mode) {
    if (mode == 0) {
        self->bgAttribute = 0x1000000;
        self->w = src->map.cellw * src->map.ncellw;
        self->h = src->map.cellh * src->map.ncellh;
    } else if (mode == 1) {
        self->bgAttribute = 0x2000000;
        self->w = 320;
        self->h = 240;
    }
    self->x = 0;
    self->y = 0;
    self->scrollx = 0;
    self->scrolly = 0;
    self->color = gBgLayerDefaultColor;
    self->map = &src->map;
    self->scalex = 0x1000;
    self->scaley = 0x1000;
    self->rotate = 0;
    self->mx = self->w / 2;
    self->my = self->h / 2;
}
/* D_8006F2C4 +0x044 (updateRotation): entry [2] (the z angle) of the
 * {num, den} ratio table Class6B5CC's updateRotation reads, in 20.12 fixed
 * point, stored in the GsBG's rotate when `set`, else added. */
void BgLayer__UpdateRotation(BgLayer *self, s32 set, WholeFrac_d294 *table) {
    s32 num = table[2].whole;
    s32 den = table[2].frac;
    s32 v = ((num / den) << 12) + (((num % den) << 12) / den);

    if (set) {
        self->rotate = v;
    } else {
        self->rotate += v;
    }
}
/* D_8006F2C4 +0x048 (updateScale): ratio-table entries [0] and [1] in
 * 20.12 fixed point become the GsBG's scale -- stored when `set` (0x1000
 * for a zero divisor, at most 30000), else added, a sum over 30000 giving
 * 30000, or 1 when either term of that ratio was negative. */
void BgLayer__UpdateScale(BgLayer *self, s32 set, WholeFrac_d294 *src) {
    s32 negX;
    s32 negY;
    s32 den;
    s16 sx;
    s16 sy;
    s32 v;

    negX = 0;
    negY = 0;
    if (src[0].whole < 0 || src[0].frac < 0) {
        negX = 1;
    }
    if (src[1].whole < 0 || src[1].frac < 0) {
        negY = 1;
    }
    den = src[0].frac;
    if (den != 0) {
        sx = ((src[0].whole / den) << 12) + (((src[0].whole % den) << 12) / den);
    }
    if (src[1].frac != 0) {
        sy = ((src[1].whole / src[1].frac) << 12) + (((src[1].whole % src[1].frac) << 12) / src[1].frac);
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
        if (src[1].frac == 0) {
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
void BgLayer__SetColor(BgLayer *self, s32 enable, BgLayerRgb *rgb) {
    if (enable) {
        self->color = *rgb;
    }
}
void BgLayer__NoOp(void) {
}
BgLayerMethods *GetBgLayerMethods(void) {
    return &D_8006F2C4;
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
/* D_8006F384 +0x078: when +0x34 is set, build a LinkResource source over the
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
        self->linkResource = New_LinkResource((Src6F240 *)&req);
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
TileMap *New_TileMap(s32 arg0, TileAtlas *atlas) {
    TileMap *obj = BMemPMgrAlloc(0x44);

    if (obj != NULL) {
        GetTileMapMethods()->ctor(obj, arg0, atlas);
        return obj;
    }
    return NULL;
}
/* D_8006F498 +0x008: constructor -- the active driver's, then this table;
 * store the atlas, clear `loaded`, and with no `arg1` set defaultGrid,
 * clear +0x2A and run its own +0x064. */
void TileMap__TileMap(TileMap *self, s32 arg1, TileAtlas *atlas) {
    s32 unused[8];

    GetActiveDataSourceMethods()->ctor((Class6D430 *)self);
    self->methods = GetTileMapMethods();
    self->atlas = atlas;
    self->loaded = 0;
    if (arg1 == 0) {
        self->defaultGrid = 1;
        self->unk2A = 0;
        self->methods->setFlag(self);
    }
}
/* D_8006F498 +0x00C: finalize -- free the map's index table, then the
 * active driver's. */
void TileMap__Finalize(TileMap *self) {
    BMemPMgrFree(self->map.index);
    GetActiveDataSourceMethods()->finalize((Class6D430 *)self);
}
/* D_8006F498 +0x064: unless +0x2A is set, slot +0x078 (BuildMap) and mark
 * `loaded`. */

void TileMap__Load(TileMap *self) {
    if (self->unk2A == 0) {
        ((TileMapBuildMapFn)self->methods->slot78)();
        self->loaded = 1;
    }
}
/* D_8006F498 +0x078: take the atlas's cells as the map's base;
 * with defaultGrid, lay out a 20 x 15 grid of 16 x 16 cells and fill an
 * allocated index table 0..n-1; otherwise, or when the allocation fails,
 * free the buffer (own +0x05C). */
void TileMap__BuildMap(TileMap *self) {
    s32 n;
    s32 i;
    u16 *p;

    self->map.base = self->atlas->cells;
    if (self->defaultGrid != 0) {
        self->map.ncellw = 20;
        self->map.cellw = 16;
        self->map.cellh = 16;
        self->map.ncellh = 15;
        n = self->map.ncellw * self->map.ncellh;
        self->map.index = BMemPMgrAlloc(n * 2);
        if (self->map.index != NULL) {
            p = self->map.index;
            for (i = 0; i < n; i++) {
                *p++ = i;
            }
            return;
        }
    }
    self->methods->freeBuffer(self);
}
TileMapMethods *GetTileMapMethods(void) {
    return &D_8006F498;
}
/* Allocate and construct a D_8006F514 object. */
TileAtlas *New_TileAtlas(s32 arg0) {
    TileAtlas *obj = BMemPMgrAlloc(0x38);

    if (obj != NULL) {
        GetTileAtlasMethods()->ctor(obj, arg0);
        return obj;
    }
    return NULL;
}
/* D_8006F514 +0x008: constructor -- the active driver's, then this table;
 * clear unk34/loaded, and with no `arg1` set defaultCells, clear +0x2A and
 * run its own +0x064. */
void TileAtlas__TileAtlas(TileAtlas *self, s32 arg1) {
    s32 unused[8];

    GetActiveDataSourceMethods()->ctor((Class6D430 *)self);
    self->methods = GetTileAtlasMethods();
    self->unk34 = 0;
    self->loaded = 0;
    if (arg1 == 0) {
        self->defaultCells = 1;
        self->unk2A = 0;
        self->methods->setFlag(self);
    }
}
/* D_8006F514 +0x00C: finalize -- free unk34 and the cells, then the active
 * driver's. */
void TileAtlas__Finalize(TileAtlas *self) {
    BMemPMgrFree(self->unk34);
    BMemPMgrFree(self->cells);
    GetActiveDataSourceMethods()->finalize((Class6D430 *)self);
}
/* D_8006F514 +0x064: unless +0x2A is set, slot +0x078 (BuildCells) and
 * mark `loaded`. */

void TileAtlas__Load(TileAtlas *self) {
    s32 unused[8];

    if (self->unk2A == 0) {
        ((TileAtlasBuildCellsFn)self->methods->slot78)();
        self->loaded = 1;
    }
}
/* D_8006F514 +0x078: with defaultCells, build 300 GsCELLs (16 x 16 texels
 * each) at `cells` over the texture pages from x 0x280: u,v step by 16, a
 * new row at x 0x3C0, a new texture page every 64 x (the lower half from v
 * 0x100). */

/* LIBGPU.H */
extern u16 GetTPage(int tp, int abr, int x, int y);

void TileAtlas__BuildCells(TileAtlas *self) {
    GsCELL *c;
    s32 x = 0x280;
    s32 u;
    s32 v;
    s32 tpage;
    s32 i;
    s32 n;

    if (self->defaultCells != 0) {
        v = 0;
        u = 0;
        tpage = GetTPage(2, 0, 0x280, 0);
        self->cells = BMemPMgrAlloc(300 * sizeof(GsCELL));
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
TileAtlasMethods *GetTileAtlasMethods(void) {
    return &D_8006F514;
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
/* Allocate and construct a MoviePlayer; freed and NULL when the constructor
 * returns nonzero (this ctor reports failure, not self). */
MoviePlayer *New_MoviePlayer(DrawRect *frame, s32 speed, s32 external) {
    MoviePlayer *obj = BMemPMgrAlloc(0x6C);

    if (obj != NULL) {
        if (GetMoviePlayerMethods()->ctor(obj, frame, speed, external) == 0) {
            return obj;
        }
        BMemPMgrFree(obj);
    }
    return NULL;
}
/* +0x008 ctor -- BasicClass's, then this table; a CdStream
 * (New_CdStream(speed, 15, 0)) and the decode buffers (MoviePlayer__InitFrame);
 * 1 when either fails. Then reset the MDEC the first time any player is built
 * (gMdecInitialized), route its output callback to OnMdecFrameReady, hand the
 * stream the ring (0x12000), clear unk50 and setAutoPlay(1). 0. */
extern void DecDCTReset(int mode);
extern int DecDCToutCallback(void (*func)());

s32 MoviePlayer__MoviePlayer(MoviePlayer *self, DrawRect *frame, s32 speed, s32 external) {
    Get_vtable_BasicClass()->ctor((BasicClass *)self);
    self->methods = GetMoviePlayerMethods();
    self->stream = New_CdStream(speed, 15, 0);
    if (self->stream != NULL) {
        if (MoviePlayer__InitFrame(self, frame, external) == 0) {
            if (gMdecInitialized == 0) {
                DecDCTReset(0);
            }
            gMdecInitialized = 1;
            DecDCToutCallback(OnMdecFrameReady);
            self->stream->methods->setRing(self->stream, self->ring, 0x12000);
            self->unk50 = 0;
            self->methods->setAutoPlay(self, 1);
            return 0;
        }
    }
    return 1;
}
/* +0x00C finalize -- release the stream, detach and reset the MDEC decoder,
 * free the four buffers (MoviePlayer__FreeFrameBuffers), then BasicClass's
 * finalize. */
void MoviePlayer__Finalize(MoviePlayer *self) {
    self->stream = self->stream->methods->release(self->stream);
    DecDCToutCallback(NULL);
    DecDCTReset(0);
    MoviePlayer__FreeFrameBuffers(self);
    Get_vtable_BasicClass()->finalize((BasicClass *)self);
}
/* Set up the frame: keep `external` and, unless the caller provides the
 * buffers, allocate the two VLC buffers (w * h * 2 + 0x1000 each), the
 * 0x12000 ring and the h * 32 strip buffer -- on a failure free what was
 * allocated (MoviePlayer__FreeFrameBuffers) and return 1. Then `frame` and
 * `stripRect` are the caller's rectangle, the strip 16 wide, and stripSize
 * its size in words. 0. */
s32 MoviePlayer__InitFrame(MoviePlayer *self, DrawRect *frame, s32 external) {
    s32 size;
    s32 unused[2];

    self->external = external;
    if (external == 0) {
        self->strip = NULL;
        self->frames[1] = NULL;
        self->frames[0] = NULL;
        self->ring = NULL;
        size = frame->w * frame->h * 2 + 0x1000;
        if ((self->frames[0] = BMemPMgrAlloc(size)) == NULL) {
            goto fail;
        }
        if ((self->frames[1] = BMemPMgrAlloc(size)) == NULL) {
            goto fail;
        }
        if ((self->ring = BMemPMgrAlloc(0x12000)) == NULL) {
            goto fail;
        }
        if ((self->strip = BMemPMgrAlloc(frame->h << 5)) == NULL) {
            goto fail;
        }
    }
    self->stripRect = *frame;
    self->frame = self->stripRect;
    self->stripRect.w = 16;
    self->stripSize = (self->stripRect.h << 4) >> 1;
    return 0;
fail:
    MoviePlayer__FreeFrameBuffers(self);
    return 1;
}
/* Unless `external`, free the four allocations. Not referenced by any data
 * word. */
void MoviePlayer__FreeFrameBuffers(MoviePlayer *self) {
    if (self->external == 0) {
        BMemPMgrFree(self->frames[0]);
        BMemPMgrFree(self->frames[1]);
        BMemPMgrFree(self->ring);
        BMemPMgrFree(self->strip);
    }
}
/* +0x040 play -- only when no movie is active (gActiveMoviePlayer):
 * MoviePlayer__MarkPlaying first with autoPlay, keep `frameCount`, open `name`
 * on the stream (100 tries); 1 when that fails. Otherwise become the active
 * movie, reset the state words, keep `arg3`/`loops` and clear `frame`
 * through DrawSystem's clearImage, color gMovieClearColor. 0. */
s32 MoviePlayer__Play(MoviePlayer *self, char *name, s32 frameCount, s32 arg3, s32 loops) {
    DrawSystem *ds;

    if (gActiveMoviePlayer == NULL) {
        if (self->autoPlay != 0) {
            MoviePlayer__MarkPlaying(self);
        }
        self->frameCount = frameCount;
        if (self->stream->methods->open(self->stream, name, 100) == 0) {
            gActiveMoviePlayer = self;
            self->haveFrame = 0;
            self->frameIndex = 0;
            self->frameDone = 1;
            self->streamEnded = 0;
            self->finished = 0;
            self->unk54 = arg3;
            self->loops = loops;
            ds = GetDrawSystem();
            ds->methods->clearImage(ds, gMovieClearColor, &self->frame);
            return 0;
        }
        return 1;
    }
    return 0;
}
void MoviePlayer__MarkPlaying(MoviePlayer *self) {
    self->unk50 = 1;
}
/* +0x044 stop -- when this is gActiveMoviePlayer, reset its state words,
 * hand the stream MoviePlayer__MarkStopped (and self) through its slot7C,
 * clear `started`, and restart the stream. */
void MoviePlayer__Stop(MoviePlayer *self) {
    MoviePlayer *cur = gActiveMoviePlayer;

    if (cur == self) {
        cur->haveFrame = 0;
        cur->frameIndex = 0;
        cur->frameDone = 1;
        cur->streamEnded = 0;
        cur->finished = 0;
        cur->stream->methods->slot7C(cur->stream, MoviePlayer__MarkStopped, cur);
        cur->started = 0;
        cur->stream->methods->restart(cur->stream);
    }
}
void MoviePlayer__MarkStopped(MoviePlayer *self) {
    self->unk50 = -1;
}
/* +0x048 advance -- when this is gActiveMoviePlayer: with unk50 set, start
 * the stream reading (startRead(1, frameCount)); if unk50 was negative,
 * count down `loops` and at the last one (or with none) mute the stream;
 * clear unk50, set `started`, 0. With unk50 clear and `started` set:
 * tail-return decodeFrame. */
s32 MoviePlayer__Advance(MoviePlayer *self) {
    MoviePlayer *cur = gActiveMoviePlayer;

    if (cur == self) {
        if (cur->unk50 == 0) {
            if (cur->started == 0) {
                goto out;
            }
        } else {
            cur->stream->methods->startRead(cur->stream, 1, cur->frameCount);
            if (cur->unk50 < 0) {
                if (cur->loops == 0 || --cur->loops == 0) {
                    cur->stream->methods->mute(cur->stream);
                }
            }
            self->unk50 = 0;
            self->started = 1;
            return 0;
        }
        return cur->methods->decodeFrame(cur);
    }
out:
    ;
}
/* +0x04C abort -- when this is gActiveMoviePlayer: streamEnded, clear
 * unk54, close the stream, finished, and the first time (`started` clear)
 * clear the stream's slot7C callback and set `started`. */
void MoviePlayer__Abort(MoviePlayer *self) {
    MoviePlayer *cur = gActiveMoviePlayer;

    if (cur == self) {
        cur->streamEnded = 1;
        cur->unk54 = 0;
        cur->stream->methods->close(cur->stream);
        cur->finished = 1;
        if (cur->started == 0) {
            cur->stream->methods->slot7C(cur->stream, 0, 0);
            cur->started = 1;
            cur->finished = 1;
        }
    }
}
void MoviePlayer__NoOpSlot50(void) {
}
void MoviePlayer__NoOpSlot54(void) {
}
/* +0x058 pullFrame -- unless streamEnded, take the next frame from the
 * stream (getNextFrame); 1 when there is none. With data, flip frameIndex
 * and VLC-decode into that frame buffer, then hand the sectors back
 * (freeRing); a negative result sets streamEnded and stops the stream. 0. */

/* LIBPRESS.H */
extern int DecDCTvlc(u32 *bs, u32 *buf);

s32 MoviePlayer__PullFrame(MoviePlayer *self) {
    u32 *data;
    s32 size;
    s32 r;

    if (self->streamEnded == 0) {
        r = self->stream->methods->getNextFrame(self->stream, &data, &size, 0x800000);
        if (r != 0) {
            if (size != 0) {
                self->frameIndex ^= 1;
                DecDCTvlc(data, self->frames[self->frameIndex]);
            }
            self->stream->methods->freeRing(self->stream, data);
            if (r < 0) {
                self->streamEnded = 1;
                self->stream->methods->stop(self->stream);
            }
            return 0;
        }
    }
    return 1;
}
void MoviePlayer__NoOpSlot5C(void) {
}
/* +0x060 drawStrip -- upload `strip` at `stripRect` (DrawSystem loadImage),
 * step stripRect right by its width, and while it is still inside `frame`
 * decode the next strip (DecDCTout, after a DrawSync when the frame is under
 * 0x80 lines); at the end, frameDone, rewind stripRect to frame's origin,
 * and `finished` once streamEnded. */

/* LIBGPU.H / LIBPRESS.H */
extern int DrawSync(int mode);
extern void DecDCTout(u32 *buf, int size);

void MoviePlayer__DrawStrip(MoviePlayer *self) {
    DrawSystem *ds = GetDrawSystem();

    ds->methods->loadImage(ds, &self->stripRect, self->strip);
    self->stripRect.x += self->stripRect.w;
    if (self->stripRect.x < self->frame.x + self->frame.w) {
        if (self->stripRect.h < 0x80) {
            DrawSync(0);
        }
        DecDCTout(self->strip, self->stripSize);
    } else {
        self->frameDone = 1;
        self->stripRect.x = self->frame.x;
        self->stripRect.y = self->frame.y;
        if (self->streamEnded != 0) {
            self->finished = 1;
        }
    }
}
/* +0x064 pollActive -- while unk54 is set, count calls in gMoviePollCounter
 * and once the count before the increment passes 100, reset it to 1 and
 * stop (which restarts the stream); 0. Otherwise clear gActiveMoviePlayer,
 * 1. */
s32 MoviePlayer__PollActive(MoviePlayer *self) {
    if (self->unk54 != 0) {
        if (gMoviePollCounter++ > 100) {
            gMoviePollCounter = 1;
            self->methods->stop(self);
        }
        return 0;
    }
    gActiveMoviePlayer = NULL;
    return 1;
}
/* +0x068 decodeFrame -- when this is gActiveMoviePlayer: once `finished`,
 * return pollActive; otherwise, with a frame pulled (haveFrame), wait for
 * the previous frame's last strip (frameDone), clear it, DrawSync when the
 * frame is under 0x80 lines, and feed frames[frameIndex] to DecDCTin and
 * the first strip to DecDCTout; then haveFrame = pullFrame returned 0, and
 * 0. */

/* LIBPRESS.H */
extern void DecDCTin(u32 *buf, int mode);

s32 MoviePlayer__DecodeFrame(MoviePlayer *self) {
    MoviePlayer *cur = gActiveMoviePlayer;

    if (cur == self) {
        if (cur->finished == 0) {
            if (cur->haveFrame != 0) {
                MoviePlayer__WaitFrameReady(cur);
                cur->frameDone = 0;
                if (cur->stripRect.h < 0x80) {
                    DrawSync(0);
                }
                DecDCTin(cur->frames[cur->frameIndex], 2);
                DecDCTout(cur->strip, cur->stripSize);
            }
            self->haveFrame = self->methods->pullFrame(self) == 0;
            return 0;
        }
        return cur->methods->pollActive(cur);
    }
}
/* The MDEC's DecDCTout callback: drawStrip of gActiveMoviePlayer, when
 * there is one. */
void OnMdecFrameReady(void) {
    if (gActiveMoviePlayer != NULL) {
        gActiveMoviePlayer->methods->drawStrip(gActiveMoviePlayer);
    }
}
/* Hang until frameDone is nonzero (it is read once). */
void MoviePlayer__WaitFrameReady(MoviePlayer *self) {
    while (self->frameDone == 0) {
    }
}
/* +0x06C setAutoPlay -- stores its argument; play tests it. */
void MoviePlayer__SetAutoPlay(MoviePlayer *self, s32 autoPlay) {
    self->autoPlay = autoPlay;
}

MoviePlayerMethods *GetMoviePlayerMethods(void) {
    return &gMoviePlayerMethods;
}
