/**
 * @file trigger_world.h
 * @brief TriggerWorld, the ModelData over a buffer of several model files,
 *        and its method table.
 */
#ifndef TRIGGER_WORLD_H
#define TRIGGER_WORLD_H

#include "model_data.h"

struct ResourceSource;

typedef struct TriggerWorld TriggerWorld;
typedef struct TriggerWorldMethods TriggerWorldMethods;

/**
 * @brief TriggerWorld's method table, gTriggerWorldMethods: ModelData's slots,
 *        then one of its own.
 *
 * It overrides +0x008 ctor (TriggerWorld__TriggerWorld), +0x00C finalize
 * (TriggerWorld__Finalize), +0x064 onRequestDone (TriggerWorld__Load),
 * +0x078 processBuffer (TriggerWorld__BuildResources, which returns s32; the
 * callers cast it) and +0x07C releaseResources
 * (TriggerWorld__ReleaseResources). ModelData's +0x080/+0x084 forwarders to a
 * TodSet are inherited unchanged, although this class never builds one.
 */
struct TriggerWorldMethods {
    MODELDATA_SLOTS(TriggerWorld, (TriggerWorld * self, struct ResourceSource *src));
    /* +0x088 */ ModelData *(*getModelData)(TriggerWorld *self, u32 index); /**< @see TriggerWorld__GetModelData */
};

/**
 * @brief A dream trigger group's models (class id 0x15F03): one buffer
 *        holding a counted table of offsets, one ModelData per entry, then the
 *        data.
 *
 * Building it replaces each table entry with a ModelData made over it (not
 * owning its resources). Parent ModelData (its ctor chains to ModelData's,
 * with `owns` 0, so ModelData's own build and release never act on it); no
 * subclasses. Methods in src/graphics/graphics_resources.c. The object is 0x3C
 * bytes (New_TriggerWorld).
 *
 * Its one user, FireDreamAuxTriggerEntries (src/world/dream_aux.c), builds
 * one over a trigger group's buffer; ProcessDreamAuxTriggerRecord passes
 * getModelData(record->parity) on to New_Entity, and
 * TodActor__AcquireModelData borrows it as the entity's ModelData.
 */
struct TriggerWorld {
    MODELDATA_FIELDS(TriggerWorldMethods);
    /* +0x038 */ s32 modelDataCount; /**< the ModelData objects BuildResources made into the buffer's table; ReleaseResources releases that many and zeroes it */
};

/** TriggerWorld's method table. */
extern TriggerWorldMethods gTriggerWorldMethods;

/**
 * @brief Returns TriggerWorld's method table.
 * @return &gTriggerWorldMethods.
 */
extern TriggerWorldMethods *GetTriggerWorldMethods(void);

/**
 * @brief Allocates a TriggerWorld from the pool and constructs it.
 * @param src The descriptor: a buffer to adopt, else a file name to request.
 * @return The new object, or NULL when the pool is exhausted or an adopted
 *         buffer's models cannot be built (the object is then freed).
 */
TriggerWorld *New_TriggerWorld(struct ResourceSource *src);

/**
 * @brief Constructor (slot +0x008): ModelData's, not owning; then, with an
 *        adopted buffer, builds the ModelData array at once.
 * @param self The object to construct.
 * @param src  The descriptor.
 * @return self, or NULL when building the array fails.
 */
void *TriggerWorld__TriggerWorld(TriggerWorld *self, struct ResourceSource *src);

/**
 * @brief Finalizer (slot +0x00C): releaseResources, then ModelData's
 *        finalizer.
 * @param self The object being destroyed.
 */
void TriggerWorld__Finalize(TriggerWorld *self);

/**
 * @brief Slot +0x064 (onRequestDone): BuildResources.
 * @param self The object, its buffer loaded.
 */
void TriggerWorld__Load(TriggerWorld *self);

/**
 * @brief Slot +0x078: builds a ModelData over each of the buffer's
 *        sub-blocks, storing it in place of that entry's offset.
 * @param self The object, its buffer loaded.
 * @return 0 when every ModelData was built; 1 when one fails, those built so
 *         far then released.
 */
s32 TriggerWorld__BuildResources(TriggerWorld *self);

/**
 * @brief Slot +0x07C: releases the ModelData objects built so far and zeroes
 *        the count.
 * @param self The object.
 */
void TriggerWorld__ReleaseResources(TriggerWorld *self);

/**
 * @brief Slot +0x088: one of the built ModelData objects.
 * @param self  The object.
 * @param index The entry's index in the buffer's table.
 * @return The ModelData, or NULL when `index` is past the table's count.
 */
ModelData *TriggerWorld__GetModelData(TriggerWorld *self, u32 index);

#endif
