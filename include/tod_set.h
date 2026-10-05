/**
 * @file tod_set.h
 * @brief TodSet, the Tod over a buffer of several TOD animations, and its
 *        method table.
 */
#ifndef TOD_SET_H
#define TOD_SET_H

#include "tod.h"

struct ResourceSource;

typedef struct TodSet TodSet;
typedef struct TodSetMethods TodSetMethods;

/** @brief A TodSet's or TriggerWorld's (include/trigger_world.h) buffer: a
 * word, a count, then that many offsets from the buffer's start, which
 * BuildTods / BuildResources overwrite with the objects built over them. */
typedef struct SubBlockTable {
    /* +0x00 */ u8 pad0[4];
    /* +0x04 */ u32 count; /**< how many sub-blocks follow */
    /* +0x08 */ s32 entries[1]; /**< each sub-block's offset, then the object built over it (SUBBLOCK_OBJ) */
} SubBlockTable;

/* The file's layout: the table stays 32-bit words at every width. */
COMPILE_ASSERT(offsetof(SubBlockTable, count) == 0x04, SubBlockTable_count);
COMPILE_ASSERT(offsetof(SubBlockTable, entries) == 0x08, SubBlockTable_entries);
COMPILE_ASSERT(sizeof(((SubBlockTable *)0)->entries[0]) == 4, SubBlockTable_entry_size);

/** @name A sub-block's object
 * The object built over a sub-block replaces its offset in the file's own
 * 32-bit word: its address on the PS1. A host's addresses may not fit the
 * word, so a host stores the object's offset from the word itself (the
 * BMemPMgr pool holds both), 0 for NULL. @{ */
#ifdef HOST_BUILD
/** stores `obj` in the table word `word` */
#define SUBBLOCK_SET_OBJ(word, obj) ((word) = SubBlockWordFor(&(word), (obj)))
/** the object table word `word` holds, as a void * */
#define SUBBLOCK_OBJ(word) ((word) != 0 ? (void *)((u8 *)&(word) + (word)) : NULL)
#else
/** stores `obj` in the table word `word` */
#define SUBBLOCK_SET_OBJ(word, obj) ((word) = (s32)(obj))
/** the object table word `word` holds, as a void * */
#define SUBBLOCK_OBJ(word) ((void *)(word))
#endif
/** @} */

#ifdef HOST_BUILD
/**
 * @brief What a host's table word at `word` holds for `obj`: its offset
 *        from the word, or 0 for NULL (SUBBLOCK_SET_OBJ). Defined in
 *        src/graphics/tod_set.c.
 * @param word The table word.
 * @param obj  The object, or NULL.
 * @return The word's value.
 */
extern s32 SubBlockWordFor(s32 *word, void *obj);

/**
 * @brief Releases the objects in `count` table words and clears them, as
 *        ReleaseBasicClassArray does with an array of pointers (the PS1's
 *        spelling of this). Defined in src/graphics/tod_set.c.
 * @param entries The first table word.
 * @param count   How many.
 */
extern void ReleaseSubBlockObjects(s32 *entries, s32 count);
#else
/** Releases the objects in `count` table words and clears them: the words
 * are pointers here, so this is ReleaseBasicClassArray. */
#define ReleaseSubBlockObjects(entries, count) \
    ReleaseBasicClassArray((BasicClass **)(entries), (count))
#endif

/** TodSet's class id (gTodSetMethods word +0x000). */
#define TODSET_CLASS_ID 0x14F03

/**
 * @brief TodSet's method table, gTodSetMethods: Tod's slots, with no new ones.
 *
 * It overrides +0x008 ctor (TodSet__TodSet), +0x00C finalize
 * (TodSet__Finalize), +0x064 onRequestDone (TodSet__BuildTods, which returns
 * s32; the ctor casts the call) and +0x078 processBuffer
 * (TodSet__ScanPackets).
 */
struct TodSetMethods {
    TOD_SLOTS(TodSet, (TodSet * self, struct ResourceSource *src));
};

/**
 * @brief A set of TOD animations (class id 0x14F03): one buffer holding a
 *        counted table of offsets, one Tod per entry, then the packet data.
 *
 * Building it replaces each table entry with the Tod made over it. Parent Tod
 * (its ctor chains to Tod's first); no subclasses, and no fields of its own:
 * the object is 0x2C bytes (New_TodSet). Methods in
 * src/graphics/tod_set.c. ModelData__BuildResources builds one over
 * a MOM file's TODs, and ModelData forwards its packet scans to it.
 */
struct TodSet {
    TOD_FIELDS(TodSetMethods);
};

/** TodSet's method table. */
extern TodSetMethods gTodSetMethods;

/**
 * @brief Returns TodSet's method table.
 * @return &gTodSetMethods.
 */
extern TodSetMethods *GetTodSetMethods(void);

/**
 * @brief Allocates a TodSet from the pool and constructs it.
 * @param src The descriptor: a buffer to adopt, else a file name to request.
 * @return The new object, or NULL when the pool is exhausted or an adopted
 *         buffer's Tods cannot be built (the object is then freed).
 */
TodSet *New_TodSet(struct ResourceSource *src);

/**
 * @brief Constructor (slot +0x008): Tod's, then, with an adopted buffer,
 *        builds its Tods at once.
 * @param self The object to construct.
 * @param src  The descriptor.
 * @return self, or NULL when building the Tods fails.
 */
void *TodSet__TodSet(TodSet *self, struct ResourceSource *src);

/**
 * @brief Finalizer (slot +0x00C): releases the Tods in the buffer's table,
 *        then Tod's finalizer.
 * @param self The object being destroyed.
 */
void TodSet__Finalize(TodSet *self);

/**
 * @brief Slot +0x064 (onRequestDone): builds a Tod over each of the buffer's
 *        sub-blocks, storing it in place of that entry's offset.
 * @param self The object, its buffer loaded.
 * @return 0 when every Tod was built; 1 when one fails, those built so far
 *         then released.
 */
s32 TodSet__BuildTods(TodSet *self);

/**
 * @brief Slot +0x078: scanTodPackets over the first frame of the TOD that
 *        follows the counted table.
 * @param self  The object.
 * @param out   As ScanTodPackets'.
 * @param tmdId As ScanTodPackets'.
 * @return The number of object-create packets in that frame.
 */
u8 TodSet__ScanPackets(TodSet *self, u8 *out, u32 *tmdId);

#endif
