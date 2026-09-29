/*
 * graphics_resources.c -- the FileResource data sources that turn loaded or
 * built buffers into graphics objects. Each class's header documents it:
 *  - TimBlockSrc (tim_block_src.h), TimArraySrc (tim_array_src.h): TIM
 *    blocks read one CD read at a time into TimImages, and the CLUT fades;
 *  - LinkResource (link_resource.h): a TMD file's TmdModels;
 *  - Tod (tod.h): a TOD animation and its packets.
 * BgLayer, the background layer, ModelData, a model file's TMD and TODs,
 * TriggerWorld, a counted set of model files, TileMap and TileAtlas, the
 * background grid's GsMAP and GsCELLs, TodSet, a set of Tods, and
 * MoviePlayer, the FMV player, follow in their own files.
 * Each FileResource class has an allocator (New_<Class>), a ctor that adopts
 * a buffer or requests a file (a ResourceSource, include/file_resource.h),
 * a finalizer, and its own load steps. The method tables close the file.
 */
#include "common.h"
#include "tim_block_src.h"
#include "trigger_world.h"
#include "tim_image.h"
#include "tim_array_src.h"
#include "bg_layer.h"
#include "link_resource.h"
#include "bmem_pmgr.h"
#include "data_source.h"
#include "cd_driver.h"
#include <stdio.h>

/* The fade CLUTs: 256-colour rows from VRAM y 480. TimBlockSrc lays its
 * four ramps out there and TimArraySrc maps an image's CLUT row back to
 * its ramp from it. */
#define CLUT_FADE_Y 480
#define CLUT_COLORS 256
#define CLUT_STP 0x8000 /* a 15-bit colour's semi-transparency bit */

/* Allocate a TimBlockSrc and construct it over the file `name`. */
TimBlockSrc *New_TimBlockSrc(char *name) {
    TimBlockSrc *obj = BMemPMgrAlloc(sizeof(TimBlockSrc));

    if (obj != NULL) {
        GetTimBlockSrcMethods()->ctor(obj, name);
        return obj;
    }
    return NULL;
}

/** @brief A TIM-block file's header: a block count, then the blocks' file
 * offsets and their sizes. */
typedef struct TimBlockHeader {
    /* +0x00 */ u32 count;      /**< how many TIM blocks the file holds */
    /* +0x04 */ u32 offsets[4]; /**< each block's offset in the file, seeked to before its read */
    /* +0x14 */ u32 sizes[4]; /**< each block's size in bytes; FindMaxTimBlockSize takes the largest */
} TimBlockHeader;

/** @brief The same header as AdvanceLoadState copies it out of the sector
 * buffer. */
/* MATCHING: bytes, so the copy is a byte-aligned block move; copying the
 * TimBlockHeader itself loses retail's test of the pointers' alignment. */
typedef struct TimBlockHeaderBytes {
    u8 bytes[sizeof(TimBlockHeader)]; /**< a TimBlockHeader's bytes */
} TimBlockHeaderBytes;

extern s16 sTimBlockClutShift;

/* ctor (+0x008): lay out the four fade ramps (2^sTimBlockClutShift rows
 * each, one after another from CLUT_FADE_Y), then open `name` and read its
 * first sector, whose header TIMBLOCK_LOAD_HEADER takes. */
void TimBlockSrc__TimBlockSrc(TimBlockSrc *self, char *name) {
    TimBlockSrcEntry *e;
    void *hdr;
    s32 i;
    u16 addr;
    s16 shift;
    u16 mask;

    GetActiveDataSourceMethods()->ctor((FileResource *)self);
    self->methods = GetTimBlockSrcMethods();
    self->blockCount = 0;
    self->blocks = NULL;
    self->loaded = 0;
    self->sector = NULL;
    self->sectorSize = 0;
    addr = 0;
    mask = 1 << sTimBlockClutShift;
    shift = sTimBlockClutShift;
    for (i = 0; i < ARRAY_COUNT(self->entries); i++) {
        e = &self->entries[i];
        e->shift = shift;
        e->mask = mask;
        e->clutX = 0;
        e->clutY = addr + CLUT_FADE_Y;
        addr += mask;
        e->clutW = CLUT_COLORS;
        e->clutH = 1;
    }
    hdr = BMemPMgrAlloc(sizeof(TimBlockHeaderBytes));
    if (hdr != NULL) {
        self->sector = BMemPMgrAlloc(CD_SECTOR_SIZE);
        if (self->sector != NULL) {
            self->bufferSize = sizeof(TimBlockHeaderBytes);
            self->buffer = hdr;
            self->loadState = TIMBLOCK_LOAD_HEADER;
            self->failed = 0;
            self->methods->open(self, name, 1, 0);
            self->methods->read(self, self->sector, CD_SECTOR_SIZE);
        }
    }
}

/* finalize (+0x00C): release the TimArraySrcs built so far. */
void TimBlockSrc__Finalize(TimBlockSrc *self) {
    ReleaseBasicClassArray((BasicClass **)self->blocks, self->blockCount);
    BMemPMgrFree(self->blocks);
    GetActiveDataSourceMethods()->finalize((FileResource *)self);
}

u32 FindMaxTimBlockSize(FileResource *self);

/* onRequestDone (+0x064), run when a read completes: once the header sector is
 * in, keep the header and read the first block into a buffer the size of
 * the largest; once a block is in, build a TimArraySrc over it (its images
 * take their CLUTs from `entries`), upload it, and read the next, until the
 * last. An allocation failure sets `failed`. */
void TimBlockSrc__AdvanceLoadState(TimBlockSrc *self) {
    TimArraySrc **p;
    s32 max;
    s32 n;

    LockActiveDataSource();
    switch (self->loadState) {
        case TIMBLOCK_LOAD_HEADER:
            if (self->flags & CD_FLAG_READ_DONE) {
                *(TimBlockHeaderBytes *)self->buffer = *(TimBlockHeaderBytes *)self->sector;
                BMemPMgrFree(self->sector);
                max = FindMaxTimBlockSize((FileResource *)self);
                self->blocks =
                    BMemPMgrAlloc(((TimBlockHeader *)self->buffer)->count * sizeof(*self->blocks));
                if (self->blocks == NULL) {
                    goto fail;
                }
                self->sector = BMemPMgrAlloc(max);
                if (self->sector == NULL) {
                    goto fail;
                }
                self->sectorSize = max;
                self->methods->seek(self, ((TimBlockHeader *)self->buffer)->offsets[0], SEEK_SET);
                self->methods->read(self, self->sector, max);
                self->loadState = TIMBLOCK_LOAD_BLOCK;
            }
            break;
        case TIMBLOCK_LOAD_BLOCK:
            if (self->flags & CD_FLAG_READ_DONE) {
                n = self->blockCount;
                p = self->blocks + n;
                *p = New_TimArraySrc(NULL);
                (*p)->buffer = self->sector;
                (*p)->bufferSize = 0;
                (*p)->clutBase = (s32)self->entries;
                n++;
                (*p)->methods->onRequestDone(*p);
                ((TimArraySrcUploadFn)(*p)->methods->processBuffer)(*p);
                self->blockCount = n;
                if (n < ((TimBlockHeader *)self->buffer)->count) {
                    self->methods->seek(self, ((TimBlockHeader *)self->buffer)->offsets[n], SEEK_SET);
                    self->methods->read(self, self->sector, self->sectorSize);
                    self->loadState = TIMBLOCK_LOAD_BLOCK;
                } else {
                    BMemPMgrFree(self->sector);
                    self->sector = NULL;
                    self->sectorSize = 0;
                    self->loadState = TIMBLOCK_LOAD_IDLE;
                    self->loaded = 1;
                    GetActiveDataSourceMethods()->onRequestDone((FileResource *)self);
                }
            }
            break;
    }
    goto out;
/* MATCHING: both failures jump to one tail after the switch; setting failed
 * and breaking where each is found lays those stores out inside the case. */
fail:
    self->failed = 1;
out:
    UnlockActiveDataSource();
}

u32 FindMaxTimBlockSize(FileResource *self) {
    TimBlockHeader *buf = self->buffer;
    u32 i;
    u32 max = 0;

    for (i = 0; i < buf->count; i++) {
        if (max < buf->sizes[i]) {
            max = buf->sizes[i];
        }
    }
    return max;
}

/* +0x078: set ramp `index`'s shift, and its row count from it. */
void TimBlockSrc__SetEntryShift(TimBlockSrc *self, s32 index, s32 shift) {
    TimBlockSrcEntry *e = &self->entries[index];

    e->shift = shift;
    e->mask = 1 << e->shift;
}

/* fadeAllEntries (+0x07C): fadeEntry every ramp toward `color`. */
void TimBlockSrc__FadeAllEntries(TimBlockSrc *self, ColorRgb *color) {
    s32 i;

    LockActiveDataSource();
    for (i = 0; i < ARRAY_COUNT(self->entries); i++) {
        self->methods->fadeEntry(self, i, color);
    }
    UnlockActiveDataSource();
}

/* fadeEntry (+0x080): set ramp `index`'s colour and rebuild it. */
void TimBlockSrc__FadeEntry(TimBlockSrc *self, s32 index, ColorRgb *color) {
    TimBlockSrcEntry *e;

    LockActiveDataSource();
    e = &self->entries[index];
    e->color = *color;
    FadeClutRow(e, index);
    UnlockActiveDataSource();
}

/* Rebuild ramp `index` from its CLUT row: read the row back from VRAM, then
 * write mask - 1 rows below it, row i + 1 blending every non-zero colour
 * (i + 1) / mask of the way toward the ramp's colour (15-bit colours,
 * worked in 8 bits and 20.12 fixed point; the semi-transparency bit kept). */
void FadeClutRow(TimBlockSrcEntry *entry, s32 index) {
    RECT dst;
    RECT src;
    u16 out[CLUT_COLORS];
    u16 in[CLUT_COLORS];
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
    src.w = CLUT_COLORS;
    src.h = 1;
    src.y = (index << sTimBlockClutShift) + CLUT_FADE_Y;
    StoreImage(&src, (u32 *)in);
    DrawSync(0);
    dst.h = 1;
    dst.x = 0;
    dst.y = 0;
    dst.w = CLUT_COLORS;
    r = entry->color.r;
    g = entry->color.g;
    b = entry->color.b;
    shift = FIX12_SHIFT - entry->shift;
    entry->clutH = entry->mask;
    for (i = 0; i < entry->mask - 1; i++) {
        f = (i + 1) << shift;
        rr = r * f;
        gg = g * f;
        bb = b * f;
        f = ONE - f;
        for (j = 0; j < src.w; j++) {
            c = in[j];
            if (c == 0) {
                out[j] = in[j];
            } else {
                cr = (in[j] & 0x1F) << 3;
                cg = (c >> 2) & 0xF8;
                cb = (c >> 7) & 0xF8;
                cr = (cr * f + rr) >> (FIX12_SHIFT + 3);
                cg = (cg * f + gg) >> (FIX12_SHIFT + 3);
                cb = (cb * f + bb) >> (FIX12_SHIFT + 3);
                out[j] = (in[j] & CLUT_STP) | cr | (cg << 5) | (cb << 10);
            }
        }
        dst.y = src.y + i + src.h;
        DrawSync(0);
        LoadImage(&dst, (u32 *)out);
    }
}

TimBlockSrcMethods *GetTimBlockSrcMethods(void) {
    return &gTimBlockSrcMethods;
}

/* Allocate and construct a LinkResource; NULL, the object freed, when the
 * ctor fails. */
LinkResource *New_LinkResource(ResourceSource *src) {
    void *obj = BMemPMgrAlloc(sizeof(LinkResource));

    if (obj != NULL) {
        if (((UnprototypedCtorTable *)GetLinkResourceMethods())->ctor(obj, src)) {
            return obj;
        }
        BMemPMgrFree(obj);
    }
    return NULL;
}

/* ctor (+0x008): adopt the descriptor's buffer and build the models (NULL
 * when that fails), or request its file. */
void *LinkResource__LinkResource(LinkResource *self, ResourceSource *src) {
    GetActiveDataSourceMethods()->ctor((FileResource *)self);
    self->methods = GetLinkResourceMethods();
    if (src != NULL) {
        if (src->buffer != NULL) {
            self->buffer = src->buffer;
            self->bufferSize = 0;
            if (((LinkResourceBuildModelsFn)self->methods->onRequestDone)(self)) {
                goto fail; /* MATCHING: a return NULL here lays the failure out before the success return */
            }
        } else {
            self->methods->requestLoadFile(self, src->name);
        }
    }
    return self;
fail:
    return NULL;
}

/* finalize (+0x00C): release every model, then the array. */
void LinkResource__Finalize(LinkResource *self) {
    TmdModel **models = self->models;

    while (*models != NULL) {
        (*models)->methods->release(*models);
        models++;
    }
    BMemPMgrFree(self->models);
    GetActiveDataSourceMethods()->finalize((FileResource *)self);
}

/* onRequestDone (+0x064): map the TMD, then build a NULL-ended array of one
 * TmdModel per TMD object. 1 when an allocation fails, with everything
 * built so far released; else 0. */
s32 LinkResource__BuildModels(LinkResource *self) {
    TmdModel **models;
    u32 i;

    models = BMemPMgrAlloc((((TmdFile *)self->buffer)->nobj + 1) * sizeof(*models));
    if (models == NULL) {
        return 1;
    }
    self->models = models;
    ((LinkResourceMapModelFn)self->methods->processBuffer)(self);
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
    GetActiveDataSourceMethods()->onRequestDone((FileResource *)self);
    return 0;
}

/* +0x078: GsMapModelingData over the TMD in the buffer (from its flags
 * word, past the id). */
void LinkResource__MapModel(LinkResource *self) {
    GsMapModelingData((unsigned long *)&((TmdFile *)self->buffer)->flags);
}

/* +0x07C: the TMD's object `index`. */
TmdObject *LinkResource__GetTmdObject(LinkResource *self, s32 index) {
    return &((TmdFile *)self->buffer)->objects[index];
}

/* +0x080: model `index`. */
TmdModel *LinkResource__GetModel(LinkResource *self, s32 index) {
    return self->models[index];
}

void LinkResource__NoOp(void) {}

LinkResourceMethods *GetLinkResourceMethods(void) {
    return &gLinkResourceMethods;
}

/* Allocate a TimArraySrc and construct it over the file `name`, or over
 * none. */
TimArraySrc *New_TimArraySrc(char *name) {
    void *obj = BMemPMgrAlloc(sizeof(TimArraySrc));

    if (obj != NULL) {
        GetTimArraySrcMethods()->ctor(obj, name);
        return obj;
    }
    return NULL;
}

/* ctor (+0x008): request `name` when there is one. */
void TimArraySrc__TimArraySrc(TimArraySrc *self, char *name) {
    GetActiveDataSourceMethods()->ctor((FileResource *)self);
    self->methods = GetTimArraySrcMethods();
    self->count = 0;
    self->images = NULL;
    self->ready = 0;
    if (name != NULL) {
        self->methods->requestLoadFile(self, name);
    }
}

/* finalize (+0x00C): release the images. */
void TimArraySrc__Finalize(TimArraySrc *self) {
    ReleaseBasicClassArray((BasicClass **)self->images, self->count);
    BMemPMgrFree(self->images);
    GetActiveDataSourceMethods()->finalize((FileResource *)self);
}

extern s16 sTimClutRowShift;

/** @brief A TimArraySrc's buffer: an image count, then each image's byte
 * offset from the start of the buffer. */
typedef struct TimArrayBuf {
    /* +0x00 */ s32 count;      /**< how many TIM images follow */
    /* +0x04 */ s32 offsets[1]; /**< each image's offset from the buffer's start */
} TimArrayBuf;

/* onRequestDone (+0x064): once the buffer is in, build one TimImage over each of
 * its images, in place, each with the fade ramp (`clutBase`'s entries) its
 * CLUT row falls in. */
void TimArraySrc__BuildImages(TimArraySrc *self) {
    GsIMAGE info;
    TimImage **objs;
    s32 i;
    s32 *offs;

    if ((self->flags & CD_FLAG_LOAD_FILE_DONE) || self->buffer != NULL) {
        self->count = ((TimArrayBuf *)self->buffer)->count;
        self->images = BMemPMgrAlloc(((TimArrayBuf *)self->buffer)->count * sizeof(*self->images));
        if (self->images != NULL) {
            objs = self->images;
            offs = ((TimArrayBuf *)self->buffer)->offsets;
            for (i = 0; i < self->count; i++) {
                *objs = New_TimImage(NULL);
                (*objs)->buffer = (u8 *)self->buffer + *offs;
                (*objs)->bufferSize = 0;
                (*objs)->methods->getTimInfo(*objs, &info);
                (*objs)->clutBase =
                    ((info.cy - CLUT_FADE_Y) >> sTimClutRowShift) * sizeof(TimBlockSrcEntry) +
                    self->clutBase;
                offs++;
                objs++;
            }
            self->ready = 1;
            GetActiveDataSourceMethods()->onRequestDone((FileResource *)self);
        }
    }
}

/* +0x078: upload every image (TimImage__Upload). */
void TimArraySrc__UploadImages(TimArraySrc *self) {
    TimImage **objs = self->images;
    s32 i;

    for (i = 0; i < self->count; i++) {
        ((TimImageUploadFn)(*objs)->methods->processBuffer)(*objs);
        objs++;
    }
}

TimArraySrcMethods *GetTimArraySrcMethods(void) {
    return &gTimArraySrcMethods;
}

/* Allocate and construct a Tod. */
Tod *New_Tod(ResourceSource *src) {
    void *obj = BMemPMgrAlloc(sizeof(Tod));

    if (obj != NULL) {
        ((UnprototypedCtorTable *)GetTodMethods())->ctor(obj, src);
        return obj;
    }
    return NULL;
}

/* ctor (+0x008): adopt the descriptor's buffer, or request its file. */
void Tod__Tod(Tod *self, ResourceSource *src) {
    GetActiveDataSourceMethods()->ctor((FileResource *)self);
    self->methods = GetTodMethods();
    if (src->buffer != NULL) {
        self->buffer = src->buffer;
        self->bufferSize = 0;
        self->methods->onRequestDone(self);
    } else {
        self->methods->requestLoadFile(self, src->name);
    }
}

/* finalize (+0x00C): nothing of its own. */
void Tod__Finalize(Tod *self) {
    GetActiveDataSourceMethods()->finalize((FileResource *)self);
}

/* +0x078: scanTodPackets over the TOD's first frame. */
u8 Tod__ScanPackets(Tod *self, u8 *out, u32 *tmdId) {
    return self->methods->scanTodPackets(self, out, tmdId, ((TodFile *)self->buffer)->frames);
}

/* scanTodPackets (+0x07C, both tables): walk the TOD frame at `data`.
 * Returns the number of object-create packets, whose object ids go to `out`
 * when there is one. With `out`, a model-id packet naming TMD `*tmdId`
 * sets `*tmdId` to the index in `out` of the object it belongs to; without,
 * `*tmdId` becomes the number of model-id packets. */
u8 ScanTodPackets(Tod *self, u8 *out, u32 *tmdId, u32 *data) {
    TodPacketHeader head;
    u32 packetCount;
    u32 i;
    s32 j;
    u8 created;
    s32 index;

    packetCount = ((TodFrame *)data)->packetCount;
    data = ((TodFrame *)data)->packets;
    i = 0;
    created = 0;
    index = 0;
    for (; i < packetCount; i++) {
        DecodeTodPacketWord(self, data, &head.objectId, &head.type, &head.flag, &head.length);
        if (head.type == TOD_PACKET_OBJECT_CONTROL && head.flag == TOD_OBJECT_CREATE) {
            created++;
            if (out != NULL) {
                *out++ = head.objectId;
            }
        } else if (head.type == TOD_PACKET_MODEL_ID) {
            if (out != NULL) {
                if (tmdId != NULL && ((TodPacket *)data)->tmdId == *tmdId) {
                    for (j = 0, out -= created; j < created; j++) {
                        if (*out++ == head.objectId) {
                            index = j;
                            break;
                        }
                    }
                }
            } else if (tmdId != NULL) {
                index++;
            }
        }
        data += head.length;
    }
    if (tmdId != NULL) {
        *tmdId = index;
    }
    return created;
}

/* decodePacketWord (+0x080, both tables): split a packet's header word;
 * returns the word after it. */
u32 *DecodeTodPacketWord(Tod *self, u32 *packet, u8 *objId, u8 *type, u8 *flag, u8 *len) {
    u32 word = *packet;

    *objId = word;
    *type = (word >> TOD_PACKET_TYPE_SHIFT) & TOD_PACKET_NIBBLE;
    *flag = (word >> TOD_PACKET_FLAG_SHIFT) & TOD_PACKET_NIBBLE;
    *len = word >> TOD_PACKET_LEN_SHIFT;
    return packet + 1;
}

TodMethods *GetTodMethods(void) {
    return &gTodMethods;
}

/* The classes' method tables, in the order the image keeps them. Each
 * fills its class's header's slots with the class's own method or the
 * parent's. A (void *) entry is a method whose declared parameters differ
 * from the slot's, usually one inherited from a parent class and declared
 * on the parent's type. */

/* TimBlockSrc: the block-by-block load step, then the CLUT fades. */
TimBlockSrcMethods gTimBlockSrcMethods = {
    /* +0x000 header */ TIMBLOCKSRC_CLASS_ID,
    /* +0x004 release */ (void *)FileResource__Release,
    /* +0x008 ctor */ TimBlockSrc__TimBlockSrc,
    /* +0x00C finalize */ TimBlockSrc__Finalize,
    /* +0x010 addChild */ (void *)BasicClass__AddChild,
    /* +0x014 removeChild */ (void *)BasicClass__RemoveChild,
    /* +0x018 removeAllChildren */ (void *)BasicClass__RemoveAllChildren,
    /* +0x01C getNextChild */ (void *)BasicClass__GetNextChild,
    /* +0x020 addParentRef */ (void *)BasicClass__AddParentRef,
    /* +0x024 removeParentRef */ (void *)BasicClass__RemoveParentRef,
    /* +0x028 clearParentRefs */ (void *)BasicClass__ClearParentRefs,
    /* +0x02C getNextParentRef */ (void *)BasicClass__GetNextParentRef,
    /* +0x030 notifyParents */ (void *)BasicClass__NotifyParents,
    /* +0x034 slot34 */ BasicClass__NoOpSlot34,
    /* +0x038 onNotify */ (void *)BasicClass__OnNotify,
    /* +0x03C slot3C */ NULL,
    /* +0x040 slot40 */ NULL,
    /* +0x044 open */ NULL,
    /* +0x048 close */ NULL,
    /* +0x04C seek */ NULL,
    /* +0x050 slot50 */ NULL,
    /* +0x054 read */ NULL,
    /* +0x058 loadFile */ NULL,
    /* +0x05C freeBuffer */ (void *)FileResource__FreeBuffer,
    /* +0x060 slot60 */ NoOp,
    /* +0x064 onRequestDone */ TimBlockSrc__AdvanceLoadState,
    /* +0x068 runRequestQueue */ NULL,
    /* +0x06C requestLoadFile */ NULL,
    /* +0x070 stopService */ NULL,
    /* +0x074 cancelRequests */ NULL,
    /* +0x078 processBuffer */ TimBlockSrc__SetEntryShift,
    /* +0x07C fadeAllEntries */ TimBlockSrc__FadeAllEntries,
    /* +0x080 fadeEntry */ TimBlockSrc__FadeEntry,
};

/* LinkResource: build the TmdModels, map the TMD, then the getters. */
LinkResourceMethods gLinkResourceMethods = {
    /* +0x000 header */ LINKRESOURCE_CLASS_ID,
    /* +0x004 release */ (void *)FileResource__Release,
    /* +0x008 ctor */ (void *)LinkResource__LinkResource,
    /* +0x00C finalize */ LinkResource__Finalize,
    /* +0x010 addChild */ (void *)BasicClass__AddChild,
    /* +0x014 removeChild */ (void *)BasicClass__RemoveChild,
    /* +0x018 removeAllChildren */ (void *)BasicClass__RemoveAllChildren,
    /* +0x01C getNextChild */ (void *)BasicClass__GetNextChild,
    /* +0x020 addParentRef */ (void *)BasicClass__AddParentRef,
    /* +0x024 removeParentRef */ (void *)BasicClass__RemoveParentRef,
    /* +0x028 clearParentRefs */ (void *)BasicClass__ClearParentRefs,
    /* +0x02C getNextParentRef */ (void *)BasicClass__GetNextParentRef,
    /* +0x030 notifyParents */ (void *)BasicClass__NotifyParents,
    /* +0x034 slot34 */ BasicClass__NoOpSlot34,
    /* +0x038 onNotify */ (void *)BasicClass__OnNotify,
    /* +0x03C slot3C */ NULL,
    /* +0x040 slot40 */ NULL,
    /* +0x044 open */ NULL,
    /* +0x048 close */ NULL,
    /* +0x04C seek */ NULL,
    /* +0x050 slot50 */ NULL,
    /* +0x054 read */ NULL,
    /* +0x058 loadFile */ NULL,
    /* +0x05C freeBuffer */ (void *)FileResource__FreeBuffer,
    /* +0x060 slot60 */ NoOp,
    /* +0x064 onRequestDone */ (void *)LinkResource__BuildModels,
    /* +0x068 runRequestQueue */ NULL,
    /* +0x06C requestLoadFile */ NULL,
    /* +0x070 stopService */ NULL,
    /* +0x074 cancelRequests */ NULL,
    /* +0x078 processBuffer */ LinkResource__MapModel,
    /* +0x07C getTmdObject */ LinkResource__GetTmdObject,
    /* +0x080 getModel */ LinkResource__GetModel,
    /* +0x084 slot84 */ LinkResource__NoOp,
};

/* TimArraySrc: build the TimImages, then upload them. */
TimArraySrcMethods gTimArraySrcMethods = {
    /* +0x000 header */ TIMARRAYSRC_CLASS_ID,
    /* +0x004 release */ (void *)FileResource__Release,
    /* +0x008 ctor */ TimArraySrc__TimArraySrc,
    /* +0x00C finalize */ TimArraySrc__Finalize,
    /* +0x010 addChild */ (void *)BasicClass__AddChild,
    /* +0x014 removeChild */ (void *)BasicClass__RemoveChild,
    /* +0x018 removeAllChildren */ (void *)BasicClass__RemoveAllChildren,
    /* +0x01C getNextChild */ (void *)BasicClass__GetNextChild,
    /* +0x020 addParentRef */ (void *)BasicClass__AddParentRef,
    /* +0x024 removeParentRef */ (void *)BasicClass__RemoveParentRef,
    /* +0x028 clearParentRefs */ (void *)BasicClass__ClearParentRefs,
    /* +0x02C getNextParentRef */ (void *)BasicClass__GetNextParentRef,
    /* +0x030 notifyParents */ (void *)BasicClass__NotifyParents,
    /* +0x034 slot34 */ BasicClass__NoOpSlot34,
    /* +0x038 onNotify */ (void *)BasicClass__OnNotify,
    /* +0x03C slot3C */ NULL,
    /* +0x040 slot40 */ NULL,
    /* +0x044 open */ NULL,
    /* +0x048 close */ NULL,
    /* +0x04C seek */ NULL,
    /* +0x050 slot50 */ NULL,
    /* +0x054 read */ NULL,
    /* +0x058 loadFile */ NULL,
    /* +0x05C freeBuffer */ (void *)FileResource__FreeBuffer,
    /* +0x060 slot60 */ NoOp,
    /* +0x064 onRequestDone */ TimArraySrc__BuildImages,
    /* +0x068 runRequestQueue */ NULL,
    /* +0x06C requestLoadFile */ NULL,
    /* +0x070 stopService */ NULL,
    /* +0x074 cancelRequests */ NULL,
    /* +0x078 processBuffer */ TimArraySrc__UploadImages,
};

/* Tod: scan the animation's packets, with the free packet decoders. */
TodMethods gTodMethods = {
    /* +0x000 header */ TOD_CLASS_ID,
    /* +0x004 release */ (void *)FileResource__Release,
    /* +0x008 ctor */ Tod__Tod,
    /* +0x00C finalize */ Tod__Finalize,
    /* +0x010 addChild */ (void *)BasicClass__AddChild,
    /* +0x014 removeChild */ (void *)BasicClass__RemoveChild,
    /* +0x018 removeAllChildren */ (void *)BasicClass__RemoveAllChildren,
    /* +0x01C getNextChild */ (void *)BasicClass__GetNextChild,
    /* +0x020 addParentRef */ (void *)BasicClass__AddParentRef,
    /* +0x024 removeParentRef */ (void *)BasicClass__RemoveParentRef,
    /* +0x028 clearParentRefs */ (void *)BasicClass__ClearParentRefs,
    /* +0x02C getNextParentRef */ (void *)BasicClass__GetNextParentRef,
    /* +0x030 notifyParents */ (void *)BasicClass__NotifyParents,
    /* +0x034 slot34 */ BasicClass__NoOpSlot34,
    /* +0x038 onNotify */ (void *)BasicClass__OnNotify,
    /* +0x03C slot3C */ NULL,
    /* +0x040 slot40 */ NULL,
    /* +0x044 open */ NULL,
    /* +0x048 close */ NULL,
    /* +0x04C seek */ NULL,
    /* +0x050 slot50 */ NULL,
    /* +0x054 read */ NULL,
    /* +0x058 loadFile */ NULL,
    /* +0x05C freeBuffer */ (void *)FileResource__FreeBuffer,
    /* +0x060 slot60 */ NoOp,
    /* +0x064 onRequestDone */ (void *)FileResource__OnRequestDone,
    /* +0x068 runRequestQueue */ NULL,
    /* +0x06C requestLoadFile */ NULL,
    /* +0x070 stopService */ NULL,
    /* +0x074 cancelRequests */ NULL,
    /* +0x078 processBuffer */ Tod__ScanPackets,
    /* +0x07C scanTodPackets */ ScanTodPackets,
    /* +0x080 decodePacketWord */ DecodeTodPacketWord,
};
