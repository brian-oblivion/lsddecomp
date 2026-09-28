/*
 * GraphicsResources -- the FileResource data sources that turn loaded files
 * into graphics objects, the tile-map background layer, and the FMV player.
 *
 * FileResource subclasses, each reached through one of
 * gFileResourceMethods's table getters: an allocator (New_<Class>), a ctor
 * that adopts a buffer or requests a file, finalize, and the class's own
 * load steps.
 *  - TimBlockSrc reads a file of TIM blocks one CD read at a time (a 36-byte
 *    header of block offsets and sizes, then each block into a new
 *    TimArraySrc) and fades up to four 256-colour CLUT rows at VRAM y 480
 *    toward a colour (FadeClutRow).
 *  - TimArraySrc builds one TimImage per image of its buffer and uploads
 *    them.
 *  - LinkResource builds one TmdModel per object of a TMD, NULL-ended.
 *  - Tod walks one TOD animation's packets: ScanTodPackets lists a frame's
 *    object-create packets and finds the object a TMD id belongs to.
 *    TodSet is a Tod over a counted array of Tods.
 *  - ModelData builds a LinkResource and a TodSet from one buffer;
 *    TriggerWorld, its subclass, a counted array of ModelData.
 *  - TileMap and TileAtlas are built rather than loaded: a GsMAP over a
 *    20 x 15 grid of 16 x 16 cells, and the 300 GsCELLs it indexes.
 * Two more classes:
 *  - BgLayer, a SceneNode wrapping one GsBG over a TileMap's GsMAP; its
 *    rotation and scale follow SceneNode's ratio tables.
 *  - MoviePlayer, CD-streamed and MDEC-decoded FMV (CdStream frames,
 *    DecDCTvlc, then DecDCTin/DecDCTout in 16-pixel strips uploaded as they
 *    finish), one movie at a time (sActiveMoviePlayer).
 * The ctors take include/FileResource.h's ResourceSource; the build steps
 * that make them fill one as a ResourceRequest's `src`. The unit's own
 * types: UnprototypedCtorTable, the view the allocators call a ctor slot
 * through when they test its result; TimBlockHeader (and its byte copy,
 * TimBlockHeaderBytes), ModelDataHeader and SubBlockTable, the layouts of
 * TimBlockSrc's, ModelData's and TodSet's / TriggerWorld's buffers.
 *
 * The file holds more than one subject (BgLayer and MoviePlayer are not
 * FileResources); where the original files inside it began is not known.
 */
#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <libpress.h>
#include "basic_class.h"
#include "scene_node.h"
#include "FileResource.h"
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
#include "bmem_pmgr.h"
#include "GameApplicationFileResource.h"
#include "cd_driver.h"

extern MoviePlayer *sActiveMoviePlayer; /* the playing movie, or NULL (play sets it, pollActive clears it) */
extern s32 sMdecInitialized;            /* set by the first ctor, which DecDCTReset(0)s the MDEC */
extern s32 sMoviePollCounter;           /* pollActive's call count */
extern u8 sMovieClearColor[4];          /* a zero word: play's clearImage color, black */

/* The fade CLUTs: 256-colour rows from VRAM y 480. TimBlockSrc lays its
 * four ramps out there and TimArraySrc maps an image's CLUT row back to
 * its ramp from it. */
#define CLUT_FADE_Y 480
#define CLUT_COLORS 256
#define CLUT_STP 0x8000 /* a 15-bit colour's semi-transparency bit */

/* GsBG attribute bits 24..25, the colour mode (LIBGS: 0 4-bit CLUT, 1 8-bit
 * CLUT, 2 15-bit direct). BgLayer's mode 1 is the one New_BgLayer's caller
 * uses, over TileAtlas's 15-bit texture pages. */
#define BG_ATTR_8BIT (1 << 24)
#define BG_ATTR_15BIT (2 << 24)
#define BG_SCREEN_W 320 /* mode 1's layer size */
#define BG_SCREEN_H 240
#define BG_SCALE_MAX 30000 /* BgLayer__UpdateScale's clamp, in 20.12 */

/* TileMap's default grid and TileAtlas's cells: 20 x 15 cells of 16 x 16
 * texels, one atlas cell per map cell, over 15-bit texture pages (GetTPage
 * tp 2) from VRAM x 640 to 960. */
#define TILEMAP_COLS 20
#define TILEMAP_ROWS 15
#define TILE_SIZE 16
#define TILE_ATLAS_CELLS (TILEMAP_COLS * TILEMAP_ROWS)
#define TILE_ATLAS_X 640
#define TILE_ATLAS_X_END 960
#define TPAGE_15BIT 2    /* GetTPage's tp: 15-bit direct */
#define TPAGE_WIDTH 64   /* a texture page's VRAM width */
#define TPAGE_HEIGHT 256 /* ... and height */
#define TPAGE_LOWER 0x10 /* the tpage word's page-y bit: pages from VRAM y 256 */

/* MoviePlayer's stream and decode geometry. */
#define MOVIE_FPS 15                          /* New_CdStream's fps */
#define MOVIE_RING_SIZE (36 * CD_SECTOR_SIZE) /* the sector ring handed to setRing */
#define MOVIE_STRIP_W 16                      /* one DecDCTout strip's pixel width */
#define MOVIE_OPEN_TRIES 100                  /* CdStream open's tries */
#define MOVIE_FRAME_TRIES 8388608             /* CdStream getNextFrame's tries */
#define MOVIE_SYNC_HEIGHT 128       /* frames shorter than this DrawSync before each strip */
#define MOVIE_KEEP_ACTIVE_POLLS 100 /* pollActive's stop interval while keepActive */

/* A TodSet's or TriggerWorld's buffer: a word, a count, then that many
 * offsets from the buffer's start, which BuildTods / BuildResources
 * overwrite with the objects built over them. */
typedef struct SubBlockTable {
    /* +0x00 */ u8 pad0[4];
    /* +0x04 */ u32 count;
    /* +0x08 */ s32 entries[1];
} SubBlockTable;

/* A method table's ctor slot, unprototyped: the allocators that check the
 * ctor's result call it through this. */
typedef struct UnprototypedCtorTable {
    /* +0x000 */ u8 pad0[8];
    /* +0x008 */ s32 (*ctor)();
} UnprototypedCtorTable;

/* Allocate a TimBlockSrc and construct it over the file `name`. */
TimBlockSrc *New_TimBlockSrc(char *name) {
    TimBlockSrc *obj = BMemPMgrAlloc(sizeof(TimBlockSrc));

    if (obj != NULL) {
        GetTimBlockSrcMethods()->ctor(obj, name);
        return obj;
    }
    return NULL;
}

/* A TIM-block file's header: a block count, then the blocks' file offsets
 * and their sizes. */
typedef struct TimBlockHeader {
    /* +0x00 */ u32 count;
    /* +0x04 */ u32 offsets[4];
    /* +0x14 */ u32 sizes[4];
} TimBlockHeader;

/* The same header as AdvanceLoadState copies it out of the sector buffer.
 * MATCHING: bytes, so the copy is a byte-aligned block move. */
typedef struct TimBlockHeaderBytes {
    u8 bytes[sizeof(TimBlockHeader)];
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
                /* MATCHING: a byte-aligned struct copy; a word-aligned one loses retail's runtime alignment test */
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
                self->methods->seek(self, ((TimBlockHeader *)self->buffer)->offsets[0], 0);
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
                    self->methods->seek(self, ((TimBlockHeader *)self->buffer)->offsets[n], 0);
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
void TimBlockSrc__FadeEntry(TimBlockSrc *self, s32 index, ColorRgb *src) {
    TimBlockSrcEntry *e;

    LockActiveDataSource();
    e = &self->entries[index];
    e->color = *src;
    FadeClutRow(e, index);
    UnlockActiveDataSource();
}

/* Rebuild ramp `index` from its CLUT row: read the row back from VRAM, then
 * write mask - 1 rows below it, row i + 1 blending every non-zero colour
 * (i + 1) / mask of the way toward the ramp's colour (15-bit colours,
 * worked in 8 bits and 20.12 fixed point; the semi-transparency bit kept). */
void FadeClutRow(TimBlockSrcEntry *e, s32 index) {
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
    r = e->color.r;
    g = e->color.g;
    b = e->color.b;
    shift = FIX12_SHIFT - e->shift;
    e->clutH = e->mask;
    for (i = 0; i < e->mask - 1; i++) {
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

/* A TimArraySrc's buffer: an image count, then each image's byte offset
 * from the start of the buffer. */
typedef struct TimArrayBuf {
    /* +0x00 */ s32 count;
    /* +0x04 */ s32 offsets[1];
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
    data += 2;
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

/* Allocate a BgLayer and construct it over `src`'s map in `mode`. */
BgLayer *New_BgLayer(TileMap *src, s32 mode) {
    BgLayer *obj = BMemPMgrAlloc(sizeof(BgLayer));

    if (obj != NULL) {
        GetBgLayerMethods()->ctor(obj, src, mode);
        return obj;
    }
    return NULL;
}

/* ctor (+0x008): SceneNode's, then reset. */
void BgLayer__BgLayer(BgLayer *self, TileMap *src, s32 mode) {
    GetSceneNodeMethods()->ctor((SceneNode *)self);
    self->methods = GetBgLayerMethods();
    ((BgLayerResetFn)self->methods->reset)(self, src, mode);
}

/* reset (+0x040): lay the GsBG over `src`'s map, sized to the map (mode 0,
 * 8-bit CLUT) or to the screen (mode 1, 15-bit), at the origin, unscaled,
 * unrotated, pivoting on its centre. */
extern ColorRgb sBgLayerDefaultColor;

void BgLayer__Reset(BgLayer *self, TileMap *src, s32 mode) {
    if (mode == 0) {
        self->bgAttribute = BG_ATTR_8BIT;
        self->w = src->map.cellw * src->map.ncellw;
        self->h = src->map.cellh * src->map.ncellh;
    } else if (mode == 1) {
        self->bgAttribute = BG_ATTR_15BIT;
        self->w = BG_SCREEN_W;
        self->h = BG_SCREEN_H;
    }
    self->x = 0;
    self->y = 0;
    self->scrollx = 0;
    self->scrolly = 0;
    self->color = sBgLayerDefaultColor;
    self->map = &src->map;
    self->scalex = ONE;
    self->scaley = ONE;
    self->rotate = 0;
    self->mx = self->w / 2;
    self->my = self->h / 2;
}

/* updateRotation (+0x044): the ratio table's z entry, in 20.12, becomes the
 * GsBG's rotation when `set`, else is added to it. */
void BgLayer__UpdateRotation(BgLayer *self, s32 set, Ratio16 *table) {
    s32 num = table[2].num;
    s32 den = table[2].den;
    s32 v = ((num / den) << FIX12_SHIFT) + (((num % den) << FIX12_SHIFT) / den);

    if (set) {
        self->rotate = v;
    } else {
        self->rotate += v;
    }
}

/* updateScale (+0x048): the ratio table's x and y entries, in 20.12,
 * become the GsBG's scale when `set` (ONE for a zero divisor, at most
 * BG_SCALE_MAX), else are added to it; a sum past BG_SCALE_MAX clamps
 * there, or to 1 when that ratio had a negative term. */
void BgLayer__UpdateScale(BgLayer *self, s32 set, Ratio16 *src) {
    s32 negX;
    s32 negY;
    s32 den;
    s16 sx;
    s16 sy;
    s32 v;

    negX = 0;
    negY = 0;
    if (src[0].num < 0 || src[0].den < 0) {
        negX = 1;
    }
    if (src[1].num < 0 || src[1].den < 0) {
        negY = 1;
    }
    den = src[0].den;
    if (den != 0) {
        sx = ((src[0].num / den) << FIX12_SHIFT) + (((src[0].num % den) << FIX12_SHIFT) / den);
    }
    if (src[1].den != 0) {
        sy = ((src[1].num / src[1].den) << FIX12_SHIFT) +
             (((src[1].num % src[1].den) << FIX12_SHIFT) / src[1].den);
    }
    if (set) {
        if (den == 0) {
            self->scalex = ONE;
        } else {
            v = sx;
            if (v > BG_SCALE_MAX) {
                v = BG_SCALE_MAX;
            }
            self->scalex = v;
        }
        if (src[1].den == 0) {
            self->scaley = ONE;
        } else {
            v = sy;
            if (v > BG_SCALE_MAX) {
                v = BG_SCALE_MAX;
            }
            self->scaley = v;
        }
    } else {
        if (self->scalex + sx > BG_SCALE_MAX) {
            if (negX) {
                self->scalex = 1;
            } else {
                self->scalex = BG_SCALE_MAX;
            }
        } else {
            self->scalex = sx + self->scalex;
        }
        if (self->scaley + sy > BG_SCALE_MAX) {
            if (negY) {
                self->scaley = 1;
            } else {
                self->scaley = BG_SCALE_MAX;
            }
        } else {
            self->scaley = sy + self->scaley;
        }
    }
}

/* +0x0B8: take `rgb` as the GsBG's colour when `enable`. */
void BgLayer__SetColor(BgLayer *self, s32 enable, ColorRgb *rgb) {
    if (enable) {
        self->color = *rgb;
    }
}

void BgLayer__NoOp(void) {}

BgLayerMethods *GetBgLayerMethods(void) {
    return &gBgLayerMethods;
}

/* Allocate and construct a ModelData that owns its LinkResource and TodSet;
 * NULL, the object freed, when the ctor fails. */
ModelData *New_ModelData(ResourceSource *src) {
    void *obj = BMemPMgrAlloc(sizeof(ModelData));

    if (obj != NULL) {
        if (((UnprototypedCtorTable *)GetModelDataMethods())->ctor(obj, src, 1)) {
            return obj;
        }
        BMemPMgrFree(obj);
    }
    return NULL;
}

/* ctor (+0x008): adopt the descriptor's buffer and load it (NULL when that
 * fails), or request its file. */
void *ModelData__ModelData(ModelData *self, ResourceSource *src, s32 owns) {
    GetActiveDataSourceMethods()->ctor((FileResource *)self);
    self->methods = GetModelDataMethods();
    self->ownsResources = owns;
    if (src->buffer != NULL) {
        self->buffer = src->buffer;
        self->bufferSize = 0;
        if (((s32 (*)())self->methods->onRequestDone)(self)) {
            goto fail;
        }
    } else {
        self->methods->requestLoadFile(self, src->name);
    }
    return self;
fail:
    return NULL;
}

/* finalize (+0x00C): releaseResources first. */
void ModelData__Finalize(ModelData *self) {
    self->methods->releaseResources(self);
    GetActiveDataSourceMethods()->finalize((FileResource *)self);
}

/* onRequestDone (+0x064): the driver's, then BuildResources. */
void ModelData__Load(ModelData *self) {
    GetActiveDataSourceMethods()->onRequestDone((FileResource *)self);
    ((s32 (*)())self->methods->processBuffer)(self);
}

/* A ModelData's buffer (a .MOM file: InitDreamAux requests ETC\\SYMSPY.MOM
 * through New_ModelData): the LinkResource's TMD at `tmdOffset`, the
 * TodSet's data from +0x0C. */
typedef struct ModelDataHeader {
    /* +0x00 */ u8 pad0[8];
    /* +0x08 */ s32 tmdOffset;
    /* +0x0C */ u8 tods[1];
} ModelDataHeader;

/* +0x078: when it owns them, build the LinkResource and the TodSet over
 * the buffer; 1, with both released, when either fails. */
s32 ModelData__BuildResources(ModelData *self) {
    ResourceRequest req;

    if (self->ownsResources != 0) {
        ResourceRequest__Set(&req, (u8 *)self->buffer + ((ModelDataHeader *)self->buffer)->tmdOffset,
                             0, 1);
        self->linkResource = New_LinkResource(&req.src);
        if (self->linkResource != NULL) {
            req.src.buffer = ((ModelDataHeader *)self->buffer)->tods;
            self->todSet = New_TodSet(&req.src);
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

/* releaseResources (+0x07C): release the TodSet and the LinkResource when
 * it owns them. */
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

/* scanPackets (+0x080): the TodSet's +0x078 (TodSet__ScanPackets). */
u8 ModelData__ForwardScanPackets(ModelData *self, u8 *out, u32 *tmdId) {
    return ((s32 (*)())self->todSet->methods->processBuffer)(self->todSet, out, tmdId);
}

/* decodePacketWord (+0x084): the TodSet's. */
void *ModelData__ForwardDecodePacketWord(ModelData *self, u32 *packet, u8 *objId, u8 *type,
                                         u8 *flag, u8 *len) {
    return self->todSet->methods->decodePacketWord(self->todSet, packet, objId, type, flag, len);
}

ModelDataMethods *GetModelDataMethods(void) {
    return &gModelDataMethods;
}

/* Allocate and construct a TriggerWorld; NULL, the object freed, when the
 * ctor fails. */
TriggerWorld *New_TriggerWorld(ResourceSource *src) {
    TriggerWorld *obj = BMemPMgrAlloc(sizeof(TriggerWorld));

    if (obj != NULL) {
        if (((UnprototypedCtorTable *)GetTriggerWorldMethods())->ctor(obj, src)) {
            return obj;
        }
        BMemPMgrFree(obj);
    }
    return NULL;
}

/* ctor (+0x008): ModelData's, not owning; with an adopted buffer, build the
 * ModelData array (NULL when that fails). */
void *TriggerWorld__TriggerWorld(TriggerWorld *self, ResourceSource *src) {
    ((UnprototypedCtorTable *)GetModelDataMethods())->ctor(self, src, 0);
    self->methods = GetTriggerWorldMethods();
    if (src->buffer != NULL) {
        if (((s32 (*)())self->methods->onRequestDone)(self)) {
            return NULL;
        }
    }
    return self;
}

/* finalize (+0x00C): releaseResources first. */
void TriggerWorld__Finalize(TriggerWorld *self) {
    self->methods->releaseResources(self);
    GetModelDataMethods()->finalize((ModelData *)self);
}

/* onRequestDone (+0x064): BuildResources. */
void TriggerWorld__Load(TriggerWorld *self) {
    ((s32 (*)())self->methods->processBuffer)(self);
}

/* +0x078: build a ModelData over each of the buffer's sub-blocks, in place
 * of its offset; 1, with those built released, when one fails. */
s32 TriggerWorld__BuildResources(TriggerWorld *self) {
    ResourceRequest req;
    SubBlockTable *buf;
    s32 *p;
    s32 i;
    s32 n;

    ResourceRequest__Set(&req, 0, 0, 1);
    buf = self->buffer;
    i = 0;
    n = buf->count;
    p = buf->entries;
    self->modelDataCount = 0;
    for (; i < n; i++) {
        req.src.buffer = (u8 *)self->buffer + ((SubBlockTable *)self->buffer)->entries[i];
        *p = (s32)New_ModelData(&req.src);
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

/* releaseResources (+0x07C): release the ModelData built so far. */
void TriggerWorld__ReleaseResources(TriggerWorld *self) {
    ReleaseBasicClassArray((BasicClass **)((SubBlockTable *)self->buffer)->entries, self->modelDataCount);
    self->modelDataCount = 0;
}

/* +0x088: ModelData `index`, NULL when out of range. */
ModelData *TriggerWorld__GetModelData(TriggerWorld *self, u32 index) {
    SubBlockTable *buf = self->buffer;

    if (index < buf->count) {
        return (ModelData *)buf->entries[index];
    }
    return NULL;
}

TriggerWorldMethods *GetTriggerWorldMethods(void) {
    return &gTriggerWorldMethods;
}

/* Allocate and construct a TileMap over `atlas`. */
TileMap *New_TileMap(s32 source, TileAtlas *atlas) {
    TileMap *obj = BMemPMgrAlloc(sizeof(TileMap));

    if (obj != NULL) {
        GetTileMapMethods()->ctor(obj, source, atlas);
        return obj;
    }
    return NULL;
}

/* ctor (+0x008): with no `source` (the one caller's), build the default
 * grid at once. What a nonzero `source` would be, no caller shows. */
void TileMap__TileMap(TileMap *self, s32 source, TileAtlas *atlas) {
    s32 unused[8]; /* MATCHING: retail's 0x40-byte frame */

    GetActiveDataSourceMethods()->ctor((FileResource *)self);
    self->methods = GetTileMapMethods();
    self->atlas = atlas;
    self->loaded = 0;
    if (source == 0) {
        self->defaultGrid = 1;
        self->loadState = 0;
        self->methods->onRequestDone(self);
    }
}

/* finalize (+0x00C): free the map's index table. */
void TileMap__Finalize(TileMap *self) {
    BMemPMgrFree(self->map.index);
    GetActiveDataSourceMethods()->finalize((FileResource *)self);
}

/* onRequestDone (+0x064): when idle, BuildMap. */
void TileMap__Load(TileMap *self) {
    if (self->loadState == 0) {
        ((TileMapBuildMapFn)self->methods->processBuffer)(); /* MATCHING: retail passes no argument */
        self->loaded = 1;
    }
}

/* +0x078: map the atlas's cells, the default grid indexing them in order;
 * without the default grid, or when the index table cannot be had, free
 * the buffer instead. */
void TileMap__BuildMap(TileMap *self) {
    s32 n;
    s32 i;
    u16 *p;

    self->map.base = self->atlas->cells;
    if (self->defaultGrid != 0) {
        self->map.ncellw = TILEMAP_COLS;
        self->map.cellw = TILE_SIZE;
        self->map.cellh = TILE_SIZE;
        self->map.ncellh = TILEMAP_ROWS;
        n = self->map.ncellw * self->map.ncellh;
        self->map.index = BMemPMgrAlloc(n * sizeof(*self->map.index));
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
    return &gTileMapMethods;
}

/* Allocate and construct a TileAtlas. */
TileAtlas *New_TileAtlas(s32 source) {
    TileAtlas *obj = BMemPMgrAlloc(sizeof(TileAtlas));

    if (obj != NULL) {
        GetTileAtlasMethods()->ctor(obj, source);
        return obj;
    }
    return NULL;
}

/* ctor (+0x008): with no `source` (the one caller's), build the default
 * cells at once. */
void TileAtlas__TileAtlas(TileAtlas *self, s32 source) {
    s32 unused[8]; /* MATCHING: retail's 0x40-byte frame */

    GetActiveDataSourceMethods()->ctor((FileResource *)self);
    self->methods = GetTileAtlasMethods();
    self->unk34 = 0;
    self->loaded = 0;
    if (source == 0) {
        self->defaultCells = 1;
        self->loadState = 0;
        self->methods->onRequestDone(self);
    }
}

/* finalize (+0x00C): free unk34 (which no method here sets) and the
 * cells. */
void TileAtlas__Finalize(TileAtlas *self) {
    BMemPMgrFree(self->unk34);
    BMemPMgrFree(self->cells);
    GetActiveDataSourceMethods()->finalize((FileResource *)self);
}

/* onRequestDone (+0x064): when idle, BuildCells. */
void TileAtlas__Load(TileAtlas *self) {
    s32 unused[8]; /* MATCHING: retail's 0x38-byte frame */

    if (self->loadState == 0) {
        ((TileAtlasBuildCellsFn)self->methods->processBuffer)(); /* MATCHING: retail passes no argument */
        self->loaded = 1;
    }
}

/* +0x078: with the default cells, lay the atlas's cells out row by row
 * across VRAM x TILE_ATLAS_X..TILE_ATLAS_X_END, u and v restarting at each
 * texture page. */
void TileAtlas__BuildCells(TileAtlas *self) {
    GsCELL *c;
    s32 x = TILE_ATLAS_X;
    s32 u;
    s32 v;
    s32 tpage;
    s32 i;
    s32 n;

    if (self->defaultCells != 0) {
        v = 0;
        u = 0;
        tpage = GetTPage(TPAGE_15BIT, 0, TILE_ATLAS_X, 0);
        self->cells = BMemPMgrAlloc(TILE_ATLAS_CELLS * sizeof(GsCELL));
        if (self->cells != NULL) {
            i = 0;
            c = self->cells;
            n = TILE_ATLAS_CELLS;
            for (; i < n; i++, c++) {
                c->u = u;
                c->tpage = tpage;
                c->v = v;
                c->cba = 0;
                c->flag = 0;
                u += TILE_SIZE;
                x += TILE_SIZE;
                if (x >= TILE_ATLAS_X_END) {
                    u = 0;
                    x = TILE_ATLAS_X;
                    v += TILE_SIZE;
                }
                if ((x & (TPAGE_WIDTH - 1)) == 0) {
                    tpage = x >> 6; /* x / TPAGE_WIDTH */
                    if (v >= TPAGE_HEIGHT) {
                        tpage += TPAGE_LOWER;
                    }
                    u = 0;
                }
            }
        }
    }
}

TileAtlasMethods *GetTileAtlasMethods(void) {
    return &gTileAtlasMethods;
}

/* Allocate and construct a TodSet; NULL, the object freed, when the ctor
 * fails. */
TodSet *New_TodSet(ResourceSource *src) {
    void *obj = BMemPMgrAlloc(sizeof(TodSet));

    if (obj != NULL) {
        if (((UnprototypedCtorTable *)GetTodSetMethods())->ctor(obj, src)) {
            return obj;
        }
        BMemPMgrFree(obj);
    }
    return NULL;
}

/* ctor (+0x008): Tod's; with an adopted buffer, build the Tods (NULL when
 * that fails). */
void *TodSet__TodSet(TodSet *self, ResourceSource *src) {
    GetTodMethods()->ctor((Tod *)self, src);
    self->methods = GetTodSetMethods();
    if (src->buffer != NULL) {
        if (((s32 (*)())self->methods->onRequestDone)(self)) {
            return NULL;
        }
    }
    return self;
}

/* finalize (+0x00C): release the Tods. */
void TodSet__Finalize(TodSet *self) {
    SubBlockTable *buf = self->buffer;

    ReleaseBasicClassArray((BasicClass **)buf->entries, buf->count);
    GetTodMethods()->finalize((Tod *)self);
}

/* onRequestDone (+0x064): build a Tod over each of the buffer's sub-blocks, in
 * place of its offset; 1, with those built released, when one fails. */
s32 TodSet__BuildTods(TodSet *self) {
    ResourceRequest req;
    SubBlockTable *buf;
    Tod **p;
    s32 i;
    s32 n;

    ResourceRequest__Set(&req, 0, 0, 1);
    buf = self->buffer;
    i = 0;
    n = buf->count;
    p = (Tod **)buf->entries;
    for (; i < n; i++) {
        req.src.buffer = (u8 *)self->buffer + ((SubBlockTable *)self->buffer)->entries[i];
        *p = New_Tod(&req.src);
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

/* +0x078: scanTodPackets over the first frame of the TOD that follows the
 * counted array. */
u8 TodSet__ScanPackets(TodSet *self, u8 *out, u32 *tmdId) {
    SubBlockTable *buf = self->buffer;

    return self->methods->scanTodPackets(self, out, tmdId, ((TodFile *)&buf->entries[buf->count])->frames);
}

TodSetMethods *GetTodSetMethods(void) {
    return &gTodSetMethods;
}

/* Allocate and construct a MoviePlayer; NULL, the object freed, when the
 * ctor fails (it returns nonzero). */
MoviePlayer *New_MoviePlayer(DrawRect *frame, s32 cdSpeed, s32 external) {
    MoviePlayer *obj = BMemPMgrAlloc(sizeof(MoviePlayer));

    if (obj != NULL) {
        if (GetMoviePlayerMethods()->ctor(obj, frame, cdSpeed, external) == 0) {
            return obj;
        }
        BMemPMgrFree(obj);
    }
    return NULL;
}

/* ctor (+0x008): a CdStream and the decode buffers (1 when either cannot be
 * had), the MDEC reset by the first player built, its output callback
 * OnMdecStripDone, and auto-play on. */
s32 MoviePlayer__MoviePlayer(MoviePlayer *self, DrawRect *frame, s32 cdSpeed, s32 external) {
    GetBasicClassMethods()->ctor((BasicClass *)self);
    self->methods = GetMoviePlayerMethods();
    self->stream = New_CdStream(cdSpeed, MOVIE_FPS, 0);
    if (self->stream != NULL) {
        if (MoviePlayer__InitFrame(self, frame, external) == 0) {
            if (sMdecInitialized == 0) {
                DecDCTReset(0);
            }
            sMdecInitialized = 1;
            DecDCToutCallback(OnMdecStripDone);
            self->stream->methods->setRing(self->stream, self->ring, MOVIE_RING_SIZE);
            self->pendingStart = 0;
            self->methods->setAutoPlay(self, 1);
            return 0;
        }
    }
    return 1;
}

/* finalize (+0x00C): release the stream, detach and reset the MDEC, free
 * the buffers. */
void MoviePlayer__Finalize(MoviePlayer *self) {
    self->stream = self->stream->methods->release(self->stream);
    DecDCToutCallback(NULL);
    DecDCTReset(0);
    MoviePlayer__FreeFrameBuffers(self);
    GetBasicClassMethods()->finalize((BasicClass *)self);
}

/* Unless `external` (the caller's buffers), allocate the two frame
 * buffers, the ring and the strip; 1, with what was had freed, when one
 * cannot be. Then take `frame`, and the first strip at its left edge. */
s32 MoviePlayer__InitFrame(MoviePlayer *self, DrawRect *frame, s32 external) {
    s32 size;
    s32 unused[2]; /* MATCHING: retail's 0x28-byte frame */

    self->external = external;
    if (external == 0) {
        self->strip = NULL;
        self->frames[1] = NULL;
        self->frames[0] = NULL;
        self->ring = NULL;
        size = frame->w * frame->h * 2 + 4096;
        if ((self->frames[0] = BMemPMgrAlloc(size)) == NULL) {
            goto fail;
        }
        if ((self->frames[1] = BMemPMgrAlloc(size)) == NULL) {
            goto fail;
        }
        if ((self->ring = BMemPMgrAlloc(MOVIE_RING_SIZE)) == NULL) {
            goto fail;
        }
        if ((self->strip = BMemPMgrAlloc(frame->h << 5)) == NULL) {
            goto fail;
        }
    }
    self->stripRect = *frame;
    self->frame = self->stripRect;
    self->stripRect.w = MOVIE_STRIP_W;
    self->stripSize = (self->stripRect.h << 4) >> 1;
    return 0;
fail:
    MoviePlayer__FreeFrameBuffers(self);
    return 1;
}

/* Unless `external`, free InitFrame's buffers. */
void MoviePlayer__FreeFrameBuffers(MoviePlayer *self) {
    if (self->external == 0) {
        BMemPMgrFree(self->frames[0]);
        BMemPMgrFree(self->frames[1]);
        BMemPMgrFree(self->ring);
        BMemPMgrFree(self->strip);
    }
}

/* play (+0x040): unless a movie is already active, open `name` on the
 * stream (1 when it will not open) and become the active movie, its frame
 * area cleared to black. */
s32 MoviePlayer__Play(MoviePlayer *self, char *name, s32 frameCount, s32 keepActive, s32 loops) {
    DrawSystem *ds;

    if (sActiveMoviePlayer == NULL) {
        if (self->autoPlay != 0) {
            MoviePlayer__RequestStart(self);
        }
        self->frameCount = frameCount;
        if (self->stream->methods->open(self->stream, name, MOVIE_OPEN_TRIES) == 0) {
            sActiveMoviePlayer = self;
            self->haveFrame = 0;
            self->frameIndex = 0;
            self->frameDone = 1;
            self->streamEnded = 0;
            self->finished = 0;
            self->keepActive = keepActive;
            self->loops = loops;
            ds = GetDrawSystem();
            ds->methods->clearImage(ds, sMovieClearColor, &self->frame);
            return 0;
        }
        return 1;
    }
    return 0;
}

void MoviePlayer__RequestStart(MoviePlayer *self) {
    self->pendingStart = 1;
}

/* rewind (+0x044): when active, reset the frame state and restart the stream
 * (its slot7C, handed RequestRestart, is empty). */
void MoviePlayer__Rewind(MoviePlayer *self) {
    MoviePlayer *cur = sActiveMoviePlayer;

    if (cur == self) {
        cur->haveFrame = 0;
        cur->frameIndex = 0;
        cur->frameDone = 1;
        cur->streamEnded = 0;
        cur->finished = 0;
        cur->stream->methods->slot7C(cur->stream, MoviePlayer__RequestRestart, cur);
        cur->started = 0;
        cur->stream->methods->restart(cur->stream);
    }
}

void MoviePlayer__RequestRestart(MoviePlayer *self) {
    self->pendingStart = -1;
}

/* advance (+0x048), StreamTask's per-tick call: when active, start the
 * stream reading on a pending start (on a restart, muting it once `loops`
 * runs out), else decode once started. */
s32 MoviePlayer__Advance(MoviePlayer *self) {
    MoviePlayer *cur = sActiveMoviePlayer;

    if (cur == self) {
        if (cur->pendingStart == 0) {
            if (cur->started == 0) {
                goto out;
            }
        } else {
            cur->stream->methods->startRead(cur->stream, 1, cur->frameCount);
            if (cur->pendingStart < 0) {
                if (cur->loops == 0 || --cur->loops == 0) {
                    cur->stream->methods->mute(cur->stream);
                }
            }
            self->pendingStart = 0;
            self->started = 1;
            return 0;
        }
        return cur->methods->decodeFrame(cur);
    }
out:; /* MATCHING: retail returns no value on this path */
}

/* abort (+0x04C): when active, close the stream and finish. */
void MoviePlayer__Abort(MoviePlayer *self) {
    MoviePlayer *cur = sActiveMoviePlayer;

    if (cur == self) {
        cur->streamEnded = 1;
        cur->keepActive = 0;
        cur->stream->methods->close(cur->stream);
        cur->finished = 1;
        if (cur->started == 0) {
            cur->stream->methods->slot7C(cur->stream, 0, 0);
            cur->started = 1;
            cur->finished = 1;
        }
    }
}

void MoviePlayer__NoOpSlot50(void) {}

void MoviePlayer__NoOpSlot54(void) {}

/* pullFrame (+0x058): VLC-decode the stream's next frame into the other
 * frame buffer; 1 when there is none yet. The stream's end stops it. */
s32 MoviePlayer__PullFrame(MoviePlayer *self) {
    u32 *data;
    s32 size;
    s32 r;

    if (self->streamEnded == 0) {
        r = self->stream->methods->getNextFrame(self->stream, &data, &size, MOVIE_FRAME_TRIES);
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

void MoviePlayer__NoOpSlot5C(void) {}

/* drawStrip (+0x060): upload the decoded strip and ask the MDEC for the
 * next, or, past the frame's right edge, mark the frame done. */
void MoviePlayer__DrawStrip(MoviePlayer *self) {
    DrawSystem *ds = GetDrawSystem();

    ds->methods->loadImage(ds, &self->stripRect, self->strip);
    self->stripRect.x += self->stripRect.w;
    if (self->stripRect.x < self->frame.x + self->frame.w) {
        if (self->stripRect.h < MOVIE_SYNC_HEIGHT) {
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

/* pollActive (+0x064), once the movie has finished: with keepActive, stay
 * active, rewinding the stream every MOVIE_KEEP_ACTIVE_POLLS calls (0);
 * otherwise no movie is active any more (1). */
s32 MoviePlayer__PollActive(MoviePlayer *self) {
    if (self->keepActive != 0) {
        if (sMoviePollCounter++ > MOVIE_KEEP_ACTIVE_POLLS) {
            sMoviePollCounter = 1;
            self->methods->rewind(self);
        }
        return 0;
    }
    sActiveMoviePlayer = NULL;
    return 1;
}

/* decodeFrame (+0x068): when active and not finished, hand the pulled
 * frame to the MDEC once the last one is drawn, and pull the next. */
s32 MoviePlayer__DecodeFrame(MoviePlayer *self) {
    MoviePlayer *cur = sActiveMoviePlayer;

    if (cur == self) {
        if (cur->finished == 0) {
            if (cur->haveFrame != 0) {
                MoviePlayer__WaitFrameReady(cur);
                cur->frameDone = 0;
                if (cur->stripRect.h < MOVIE_SYNC_HEIGHT) {
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
} /* MATCHING: no return when another player is active, as retail */

/* The MDEC's DecDCTout callback: drawStrip of sActiveMoviePlayer, when
 * there is one. */
void OnMdecStripDone(void) {
    if (sActiveMoviePlayer != NULL) {
        sActiveMoviePlayer->methods->drawStrip(sActiveMoviePlayer);
    }
}

/* Wait for drawStrip to finish the frame. */
void MoviePlayer__WaitFrameReady(MoviePlayer *self) {
    while (self->frameDone == 0) { /* MATCHING: not volatile, so retail reads it once and spins */
    }
}

/* setAutoPlay (+0x06C). */
void MoviePlayer__SetAutoPlay(MoviePlayer *self, s32 autoPlay) {
    self->autoPlay = autoPlay;
}

MoviePlayerMethods *GetMoviePlayerMethods(void) {
    return &gMoviePlayerMethods;
}
