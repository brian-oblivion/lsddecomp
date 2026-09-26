#ifndef TMDMODEL_H
#define TMDMODEL_H

#include "BasicClass.h"

/*
 * TmdModel -- one object of a TMD model file (class id 0x9, method table
 * gTmdModelMethods): a BasicClass subclass with no subclasses of its own.
 * Methods in src/code_fa50.c.
 *
 * The object is built on ONE entry of a TMD's object table (New_TmdModel's
 * argument; LinkResource__BuildModels, code_33808, builds one per entry of a
 * loaded TMD). `data` is that entry minus 0xC, i.e. the file header when the
 * entry is the first one; GetObject indexes the table from there and
 * MapModelingData hands `&data->flags` to GsMapModelingData. Class6B5CC's
 * `model` (+0x020) holds one: Class6B5CC__LinkModel links `object` into its
 * GsDOBJ2 (GsLinkObject4(data->objects, ...), GsDOBJ2.tmd = object).
 *
 * Besides its table the class has non-virtual methods Class6B5CC's collision
 * code calls directly: a bounding box over the object's vertices
 * (ComputeBounds, GetHull), a shared bounds buffer holding
 * gTmdModelBoundsCount boxes (always 1, set by the ctor), and a segment cast
 * against every face (RaycastFaces, walking primitives with NextPrimitive).
 *
 * The object is 0x24 bytes (New_TmdModel's allocation). Slots +0x040..+0x04C
 * are named for their occupants; no caller of them has been found in C.
 */

typedef struct TmdModel TmdModel;
typedef struct TmdModelMethods TmdModelMethods;

/* A 6-byte all-s16 vector (alignment 2: whole-value copies are lwl/lwr). */
typedef struct TmdVec3 {
    s16 x, y, z;
} TmdVec3;

/* One vertex of an object's vertex list: libgte's SVECTOR, 8 bytes. */
typedef struct TmdVertex {
    s16 x, y, z, pad;
} TmdVertex;

/* An axis-aligned bounding box. */
typedef struct TmdBox {
    /* +0x000 */ TmdVec3 min;
    /* +0x006 */ TmdVec3 max;
} TmdBox;

/* A counted list of boxes' eight corners; TmdModel__GetHull writes a list of
 * one: v[0..3] the min-z face, v[4..7] the max-z face. */
typedef struct TmdHull {
    /* +0x000 */ s32 count;
    /* +0x004 */ TmdVec3 v[8];
} TmdHull;

/* One TMD primitive: a 4-byte header, then u16 words (vertex indices among
 * them, at mode-dependent positions; TmdModel__NextPrimitive). */
typedef struct TmdPrim {
    /* +0x000 */ u8 olen;
    /* +0x001 */ u8 ilen;
    /* +0x002 */ u8 flag;
    /* +0x003 */ u8 mode;
    /* +0x004 */ u16 h[20];
} TmdPrim;

/* One entry of a TMD's object table, 0x1C bytes. */
typedef struct TmdObject {
    /* +0x000 */ TmdVertex *verts;
    /* +0x004 */ s32 nverts;
    /* +0x008 */ void *normals;
    /* +0x00C */ s32 nnormals;
    /* +0x010 */ TmdPrim *prims;
    /* +0x014 */ u32 nprims;
    /* +0x018 */ s32 scale;
} TmdObject;

/* A TMD file: three header words, then the object table. */
typedef struct TmdFile {
    /* +0x000 */ u32 id;
    /* +0x004 */ u32 flags; /* GsMapModelingData's argument points here */
    /* +0x008 */ u32 nobj;
    /* +0x00C */ TmdObject objects[1];
} TmdFile;

/* The four words slot +0x040 copies in; not read anywhere in C. */
typedef struct TmdModelQuad {
    s32 w[4];
} TmdModelQuad;

struct TmdModelMethods {
    BASICCLASS_SLOTS(TmdModel, (TmdModel * self, TmdObject *object));
    /* +0x040 */ void (*setQuad)(TmdModel *self, TmdModelQuad *src); /* TmdModel__SetQuad */
    /* +0x044 */ void (*mapModelingData)(TmdModel *self);            /* TmdModel__MapModelingData */
    /* +0x048 */ TmdObject *(*getObject)(TmdModel *self, s32 i);     /* TmdModel__GetObject */
    /* +0x04C */ void (*slot4C)(void); /* TmdModel__func_8001F37C, empty */
};

struct TmdModel {
    BASICCLASS_FIELDS(TmdModelMethods);
    /* +0x00C */ TmdFile *data; /* object - 0xC: the file header when object is the first entry */
    /* +0x010 */ TmdObject *object; /* New_TmdModel's argument */
    /* +0x014 */ TmdModelQuad quad; /* setQuad's copy */
};

extern TmdModelMethods gTmdModelMethods;
extern TmdModelMethods *Get_vtable_TmdModel(void); /* returns &gTmdModelMethods */

extern s32 gTmdModelBoundsCount; /* boxes in the bounds buffer: 1, set by the ctor */

TmdModel *New_TmdModel(TmdObject *object);

/* The occupants of its own table. */
void TmdModel__TmdModel(TmdModel *self, TmdObject *object);
void TmdModel__SetQuad(TmdModel *self, TmdModelQuad *src);
void TmdModel__MapModelingData(TmdModel *self);
TmdObject *TmdModel__GetObject(TmdModel *self, s32 i);
void TmdModel__func_8001F37C(void);

/* Non-virtual methods, called directly. */
void TmdModel__InitBoundsCount(TmdModel *self);
s32 TmdModel__GetBoundsCount(TmdModel *self);
void TmdModel__ComputeBounds(TmdModel *self, TmdBox *box);
void TmdModel__UpdateBoundsBuffer(TmdModel *self);
TmdBox *TmdModel__GetBoundsBuffer(TmdModel *self, s32 i);
void TmdModel__GetHull(TmdModel *self, TmdHull *out);
s32 TmdModel__RaycastFaces(TmdModel *self, s32 *best, TmdVec3 *hitOut, s32 *height, TmdVec3 *origin,
                           TmdVec3 *end);
TmdPrim *TmdModel__NextPrimitive(TmdModel *self, TmdPrim *p, s32 *n, TmdVec3 *out, u32 *count);

#endif
