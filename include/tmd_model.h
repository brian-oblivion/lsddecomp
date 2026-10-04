/**
 * @file tmd_model.h
 * @brief TmdModel, one object of a TMD model file, its method table, the TMD
 *        layouts it reads, and the bounding-box and hull types its collision
 *        helpers use.
 */
#ifndef TMD_MODEL_H
#define TMD_MODEL_H

#include "basic_class.h"

typedef struct TmdModel TmdModel;
typedef struct TmdModelMethods TmdModelMethods;

/** TmdModel's class id (gTmdModelMethods word +0x000). A single nibble, so
 * `(header & CLASS_ID_ROOT_MASK) == TMDMODEL_CLASS_ID` is its is-kind-of test
 * (SceneNode's addChild and removeChild, which link or unlink the model). */
#define TMDMODEL_CLASS_ID 0x9

/** @brief A 6-byte vector of three s16s, 2-byte aligned. */
typedef struct TmdVec3 {
    s16 x; /**< x */
    s16 y; /**< y */
    s16 z; /**< z */
} TmdVec3;

/** @brief One vertex of an object's vertex list: libgte's SVECTOR, 8 bytes. */
typedef struct TmdVertex {
    s16 x;   /**< x */
    s16 y;   /**< y */
    s16 z;   /**< z */
    s16 pad; /**< unused */
} TmdVertex;

/** @brief An axis-aligned bounding box. */
typedef struct TmdBox {
    /* +0x000 */ TmdVec3 min; /**< the smallest x, y and z */
    /* +0x006 */ TmdVec3 max; /**< the largest x, y and z */
} TmdBox;

#define HULL_FACE_CORNERS 4 /**< corners per face of a TmdHull box */
#define HULL_BOX_CORNERS 8  /**< corners per box: two faces */

/** @brief A counted list of boxes, each as its eight corners: v[0..3] the
 *         min-z face, v[4..7] the max-z face. TmdModel__GetHull writes a list
 *         of one. */
typedef struct TmdHull {
    /* +0x000 */ s32 count;                   /**< the number of boxes */
    /* +0x004 */ TmdVec3 v[HULL_BOX_CORNERS]; /**< the first box's corners; further boxes follow */
} TmdHull;

/** @brief One TMD primitive: a 4-byte header, then u16 words, the vertex
 *         indices among them at positions that depend on the packet type
 *         (TmdModel__NextPrimitive). */
typedef struct TmdPrim {
    /* +0x000 */ u8 olen;   /**< the GPU packet's length in words */
    /* +0x001 */ u8 ilen;   /**< the TMD packet's length in words */
    /* +0x002 */ u8 flag;   /**< GsTMDFlag* bits (light, one or two faces, gradation) */
    /* +0x003 */ u8 mode;   /**< the GPU command code (GPU_COM_*), plus the semi-transparency bit */
    /* +0x004 */ u16 h[20]; /**< the packet's body */
} TmdPrim;

/** @name A mapped TMD's lists
 * GsMapModelingData turns the offsets in a TMD's object table into
 * references to the object's lists, in the file's own 32-bit words. On the
 * PS1 those are addresses. A host's addresses may not fit the words, so
 * psyz maps them to offsets from the object's own entry (psyz's GsTMDAddr),
 * at both widths. TMD_LIST is a list field's type and TMD_LIST_ADDR reads
 * one, either way. @{ */
#ifdef HOST_BUILD
#define TMD_LIST(Type) s32 /**< an offset from the entry */
/** the address of `obj`'s list `field` */
#define TMD_LIST_ADDR(obj, field) ((void *)((u8 *)(obj) + (obj)->field))
#else
#define TMD_LIST(Type) Type * /**< an address */
/** the address of `obj`'s list `field` */
#define TMD_LIST_ADDR(obj, field) ((obj)->field)
#endif
/** @} */

/** @brief One entry of a TMD's object table, 0x1C bytes, as
 *         GsMapModelingData leaves it. */
typedef struct TmdObject {
    /* +0x000 */ TMD_LIST(TmdVertex) verts; /**< the vertex list (TMD_LIST_ADDR) */
    /* +0x004 */ s32 nverts;                /**< its length */
    /* +0x008 */ TMD_LIST(void) normals;    /**< the normal list (TMD_LIST_ADDR) */
    /* +0x00C */ s32 nnormals;              /**< its length */
    /* +0x010 */ TMD_LIST(TmdPrim) prims;   /**< the primitive list (TMD_LIST_ADDR) */
    /* +0x014 */ u32 nprims;                /**< its length */
    /* +0x018 */ s32 scale;                 /**< the TMD scale exponent */
} TmdObject;

/** @brief A TMD file: three header words, then the object table. */
typedef struct TmdFile {
    /* +0x000 */ u32 id;               /**< the TMD id word */
    /* +0x004 */ u32 flags;            /**< the flags word; GsMapModelingData takes its address */
    /* +0x008 */ u32 nobj;             /**< the number of objects */
    /* +0x00C */ TmdObject objects[1]; /**< the object table */
} TmdFile;

/** @brief The four words setQuad copies in; nothing reads them. */
typedef struct TmdModelQuad {
    s32 w[4]; /**< the words */
} TmdModelQuad;

/**
 * @brief TmdModel's method table, gTmdModelMethods: BasicClass's slots with
 *        the ctor overridden (TmdModel__TmdModel), then four of its own, for
 *        which no caller is known.
 */
struct TmdModelMethods {
    BASICCLASS_SLOTS(TmdModel, (TmdModel * self, TmdObject *object));
    /* +0x040 */ void (*setQuad)(TmdModel *self, TmdModelQuad *src); /**< @see TmdModel__SetQuad */
    /* +0x044 */ void (*mapModelingData)(TmdModel *self); /**< @see TmdModel__MapModelingData */
    /* +0x048 */ TmdObject *(*getObject)(TmdModel *self, s32 i); /**< @see TmdModel__GetObject */
    /* +0x04C */ void (*slot4C)(void);                           /**< @see TmdModel__NoOpSlot4C */
};

/**
 * @brief One object of a TMD file (class id 0x9), built on one entry of the
 *        file's object table.
 *
 * Besides its table it has direct methods SceneNode's collision code calls:
 * a bounding box over the object's vertices (ComputeBounds, GetHull), a
 * shared bounds buffer, and a segment cast against every face
 * (RaycastFaces, walking primitives with NextPrimitive).
 *
 * Parent BasicClass; no subclasses. Methods in src/graphics/tmd_model.c. The
 * object is 0x24 bytes (New_TmdModel). LinkResource__BuildModels builds one
 * per object of a loaded TMD; SceneNode's `model` holds one, and
 * SceneNode__LinkModel links `object` into its GsDOBJ2.
 */
struct TmdModel {
    BASICCLASS_FIELDS(TmdModelMethods);
    /* +0x00C */ TmdFile *data; /**< `object` less the file header's size: the file itself when `object` is the first entry */
    /* +0x010 */ TmdObject *object; /**< the object table entry, New_TmdModel's argument */
    /* +0x014 */ TmdModelQuad quad; /**< setQuad's copy */
};

/** TmdModel's method table. */
extern TmdModelMethods gTmdModelMethods;

/**
 * @brief Returns TmdModel's method table.
 * @return &gTmdModelMethods.
 */
extern TmdModelMethods *GetTmdModelMethods(void);

/**
 * @brief Allocates a TmdModel from the pool and constructs it over a TMD
 *        object.
 * @param object One entry of a TMD file's object table.
 * @return The new model, or NULL when the pool is exhausted.
 */
TmdModel *New_TmdModel(TmdObject *object);

/**
 * @brief Constructor (slot +0x008): BasicClass's, then keeps `object` and the
 *        file header in front of it, and sets the bounds count to 1.
 * @param self   The object to construct.
 * @param object One entry of a TMD file's object table.
 */
void TmdModel__TmdModel(TmdModel *self, TmdObject *object);

/**
 * @brief Slot +0x040: copies four words into `quad`.
 * @param self The model.
 * @param src  The words to copy.
 */
void TmdModel__SetQuad(TmdModel *self, TmdModelQuad *src);

/**
 * @brief Slot +0x044: GsMapModelingData over the model's TMD file.
 * @param self The model.
 */
void TmdModel__MapModelingData(TmdModel *self);

/**
 * @brief Slot +0x048: one entry of the file's object table.
 * @param self The model.
 * @param i    The entry's index; not range-checked.
 * @return The object table entry.
 */
TmdObject *TmdModel__GetObject(TmdModel *self, s32 i);

/** @brief Slot +0x04C: does nothing. */
void TmdModel__NoOpSlot4C(void);

/**
 * @brief Sets the shared bounds buffer's box count to 1.
 * @param self The model (unused).
 */
void TmdModel__InitBoundsCount(TmdModel *self);

/**
 * @brief Returns the shared bounds buffer's box count.
 * @param self The model (unused).
 * @return The count, 1 once any model is built.
 */
s32 TmdModel__GetBoundsCount(TmdModel *self);

/**
 * @brief Computes the axis-aligned box around the object's vertices.
 * @param self The model; its object must have at least one vertex.
 * @param box  Receives the box.
 */
void TmdModel__ComputeBounds(TmdModel *self, TmdBox *box);

/**
 * @brief Computes the object's box into the shared bounds buffer.
 * @param self The model.
 */
void TmdModel__UpdateBoundsBuffer(TmdModel *self);

/**
 * @brief Returns the shared bounds buffer.
 * @param self The model (unused).
 * @param i    Unused: the buffer's first box is always returned.
 * @return The buffer's first box.
 */
TmdBox *TmdModel__GetBoundsBuffer(TmdModel *self, s32 i);

/**
 * @brief Writes the object's bounding box as a hull of one box, its eight
 *        corners.
 * @param self The model.
 * @param out  Receives the hull; its count is set to 1.
 */
void TmdModel__GetHull(TmdModel *self, TmdHull *out);

/** TmdModel__RaycastFaces' `*best` before any face is hit: the largest s32. */
#define DIST_NONE 0x7FFFFFFF

/**
 * @brief Casts the segment origin..end against every triangle and quad of the
 *        object and keeps the nearest hit.
 *
 * A face is hit when the segment crosses its plane (through its first three
 * vertices) no farther than `end`, inside the face's bounding box grown by 24
 * on each side.
 * @param self   The model.
 * @param best   Receives the nearest hit's distance from `origin`, DIST_NONE
 *               when nothing is hit.
 * @param hitOut Receives the nearest hit point.
 * @param height Receives the hit point's height above the bottom of the hit
 *               face's box, or NULL.
 * @param origin The segment's start.
 * @param end    The segment's end.
 * @return 1 when any face was hit, else 0.
 */
s32 TmdModel__RaycastFaces(TmdModel *self, s32 *best, TmdVec3 *hitOut, s32 *height, TmdVec3 *origin,
                           TmdVec3 *end);

/**
 * @brief Walks the object's primitive list one packet per call.
 * @param self  The model.
 * @param p     The previous call's result; ignored when `*count` is 0.
 * @param n     Receives the face's vertex count: 3, 4, or 0 for a packet type
 *              it does not know.
 * @param out   Receives the face's vertices (up to four).
 * @param count The cursor: 0 starts at the first packet; advanced by one.
 * @return The packet after this one, or NULL past the end of the list.
 */
TmdPrim *TmdModel__NextPrimitive(TmdModel *self, TmdPrim *p, s32 *n, TmdVec3 *out, u32 *count);

/**
 * @brief Moves the first primitive's CLUT id by the id of VRAM position
 *        (xy[0], xy[1]): x / 16 + y * 64 added to it.
 * @param self The model; its first primitive must be textured.
 * @param xy   The VRAM position, two s32s.
 */
void TmdModel__AddFirstPrimClut(TmdModel *self, s32 *xy);

/**
 * @brief Points the first primitive's CLUT id at the CLUT at VRAM
 *        (xy[0], xy[1]). SetStyleEffectSources (src/world/style_effect.c)
 *        calls it.
 * @param self The model; its first primitive must be textured.
 * @param xy   The CLUT's VRAM position, two s16s.
 */
void TmdModel__SetFirstPrimClut(TmdModel *self, s16 *xy);

/**
 * @brief Turns each box of a hull a quarter turn and offsets one of its
 *        faces. Not a method; the hull is the kind SceneNode's getModelHull
 *        fills for Actor__NotifyMove.
 * @param h     The hull.
 * @param turn  Nonzero to swap the boxes' faces a quarter turn and offset x;
 *              zero to offset z only.
 * @param back  Zero to offset the min-z face (v[0..3]), nonzero the max-z
 *              face (v[4..7]).
 * @param delta The offset added to that face's corners.
 */
void RotateAndOffsetHullList(TmdHull *h, s32 turn, s32 back, s32 delta);

#endif
