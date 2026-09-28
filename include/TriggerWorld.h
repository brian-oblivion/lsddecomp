#ifndef TRIGGERWORLD_H
#define TRIGGERWORLD_H

#include "ModelData.h"

/*
 * TriggerWorld -- a ModelData subclass (class id 0x15F03, method table
 * gTriggerWorldMethods) over a buffer holding several model files: a counted offset
 * table, one ModelData per entry, then the data. Methods in
 * src/graphics/GraphicsResources.c; no subclasses. Its parent is its id parent:
 * TriggerWorld__TriggerWorld's first call is GetModelDataMethods()->ctor
 * (with a third argument 0, so ModelData's own resource build and release
 * never act on it).
 *
 * What its own methods do: TriggerWorld__BuildResources (+0x078) makes one
 * ModelData per entry of the buffer's counted offset table (New_ModelData
 * over buffer + entries[i], not owning) and stores each back into the
 * table's own word, counting them at +0x038; TriggerWorld__ReleaseResources
 * (+0x07C) releases that array (ReleaseBasicClassArray); and
 * TriggerWorld__GetModelData (+0x088) returns entry `index`, 0 out of range.
 * Its one outside user, DreamAux's FireDreamAuxTriggerEntries, builds one
 * over a trigger group's buffer, and ProcessDreamAuxTriggerRecord passes
 * getModelData(record->parity) on as New_Entity's descriptor word +0x00C,
 * which TodActor__AcquireModelData borrows as the entity's ModelData.
 *
 * SLOTS (`classtable.py gTriggerWorldMethods --vs gModelDataMethods`: 34 against 33): the
 * overrides are +0x008 (ctor), +0x00C (TriggerWorld__Finalize), +0x064
 * (onRequestDone: TriggerWorld__Load, which only runs +0x078), +0x078 (FileResource's
 * slot78: TriggerWorld__BuildResources, s32, as ModelData__BuildResources
 * there; callers cast it) and +0x07C (releaseResources:
 * TriggerWorld__ReleaseResources). +0x080/+0x084 are ModelData's forwarders
 * to its todSet, inherited unchanged although this class never builds one.
 * One own slot, +0x088.
 *
 * FIELDS: one own field, +0x038. The object is 0x3C bytes (New_TriggerWorld);
 * ModelData's is 0x38.
 *
 * The ctor returns self or NULL (New_TriggerWorld tests it), but
 * MODELDATA_SLOTS declares +0x008 returning void, as FileResource's own ctor
 * does; the allocator reaches it through GraphicsResources.c's unprototyped
 * UnprototypedCtorTable view, as every allocator in that unit does. The descriptor is
 * include/FileResource.h's ResourceSource ({buffer to adopt, file name to request}).
 */

struct ResourceSource;

typedef struct TriggerWorld TriggerWorld;
typedef struct TriggerWorldMethods TriggerWorldMethods;

struct TriggerWorldMethods {
    MODELDATA_SLOTS(TriggerWorld, (TriggerWorld * self, struct ResourceSource *src));
    /* +0x088 */ ModelData *(*getModelData)(TriggerWorld *self, u32 index); /* TriggerWorld__GetModelData */
};

struct TriggerWorld {
    MODELDATA_FIELDS(TriggerWorldMethods);
    /* +0x038 */ s32 modelDataCount; /* ModelData objects BuildResources made into the buffer's table; ReleaseResources releases that many and zeroes it */
};

extern TriggerWorldMethods gTriggerWorldMethods;
extern TriggerWorldMethods *GetTriggerWorldMethods(void);

TriggerWorld *New_TriggerWorld(struct ResourceSource *src);
void *TriggerWorld__TriggerWorld(TriggerWorld *self, struct ResourceSource *src);
void TriggerWorld__Finalize(TriggerWorld *self);
void TriggerWorld__Load(TriggerWorld *self);
s32 TriggerWorld__BuildResources(TriggerWorld *self);
void TriggerWorld__ReleaseResources(TriggerWorld *self);
ModelData *TriggerWorld__GetModelData(TriggerWorld *self, u32 index);

#endif
