#ifndef MODELDATA_H
#define MODELDATA_H

#include "FileResource.h"
#include "TodSet.h"

/*
 * ModelData -- a FileResource data source (class id 0x5F03, method table
 * gModelDataMethods) that splits one loaded file into a LinkResource (gLinkResourceMethods,
 * an array of TMD models) and a TodSet (gTodSetMethods, an array of TOD
 * animations), and forwards TOD packet scanning to the TodSet. Methods in
 * src/graphics/GraphicsResources.c; one subclass, TriggerWorld (gTriggerWorldMethods, 0x15F03), whose
 * ctor calls this class's first (TriggerWorld__TriggerWorld:
 * GetModelDataMethods()->ctor(self, arg, 0)).
 *
 * What its own methods do: build a
 * TMD model source and a TOD set over one buffer (ModelData__BuildResources:
 * New_LinkResource over the sub-block at the buffer's third word, New_TodSet
 * over the buffer past +0x0C) and forward TOD packet decoding to the set
 * (+0x080/+0x084); its outside users, TodActor (TodActor.modelData,
 * `tmd`/`tods`) and DreamAux (InitDreamAux's MOM files), hold it as the
 * model and animation data of an actor.
 *
 * PARENT BY CTOR CHAIN, NOT BY ID. The id 0x5F03 puts it under TimBlockSrc
 * (0xF03), but ModelData__ModelData's first call is
 * GetActiveDataSourceMethods()->ctor, as TimBlockSrc__TimBlockSrc's is: it is
 * TimBlockSrc's sibling under FileResource, and carries none of TimBlockSrc's
 * layout. That parent ctor is chosen at RUN TIME: the CD driver's
 * (CdDriver__CdDriver, which chains to FileResource__FileResource; its
 * object is 0x2C bytes, New_CdDriver) or, while sActiveDataSource is
 * DATASOURCE_NULL, the null driver's (NullDriver__NullDriver, an empty body).
 * The fields below assume FileResource's own 0x2C-byte layout, which is the
 * CD driver's whole object: ModelData's own fields start at +0x02C.
 *
 * The ctor returns self or NULL (New_ModelData tests it), but
 * FILERESOURCE_SLOTS declares +0x008 returning void, as FileResource's own ctor
 * does; the allocators reach the ctor through GraphicsResources.c's unprototyped
 * UnprototypedCtorTable view instead.
 *
 * The ctor's descriptor is include/FileResource.h's ResourceSource ({buffer
 * to adopt, file name to request}).
 */

struct ResourceSource;

typedef struct ModelData ModelData;
typedef struct ModelDataMethods ModelDataMethods;

/* +0x078 is FileResource's processBuffer (NULL there): this table's occupant is
 * ModelData__BuildResources(self), s32, 0 when both sources exist; the
 * callers cast it (an inherited slot keeps the parent's name). */
/* clang-format off */
#define MODELDATA_SLOTS(Self, CtorParams)                                                          \
    FILERESOURCE_SLOTS(Self, CtorParams);                                                            \
    /* +0x07C */ void (*releaseResources)(Self *self);          /* ModelData__ReleaseResources */  \
    /* +0x080 */ u8 (*scanPackets)(Self *self, u8 *out, u32 *tmdId); /* ModelData__ForwardScanPackets: todSet's +0x078 */ \
    /* +0x084 */ void *(*decodePacketWord)(Self *self, u32 *packet, u8 *objId, u8 *type, u8 *flag, u8 *len) /* ModelData__ForwardDecodePacketWord: todSet's +0x080 */
/* clang-format on */

/* clang-format off */
#define MODELDATA_FIELDS(Methods)                                                                  \
    FILERESOURCE_FIELDS(Methods);                                                                    \
    /* +0x02C */ struct LinkResource *linkResource; /* New_LinkResource (include/LinkResource.h); released by ReleaseResources */ \
    /* +0x030 */ TodSet *todSet;       /* New_TodSet (gTodSetMethods); +0x080/+0x084 forward to it */ \
    /* +0x034 */ s32 ownsResources         /* the ctor's third argument: New_ModelData 1, TriggerWorld 0; BuildResources and ReleaseResources act only while it is set. The object is 0x38 bytes (New_ModelData): TriggerWorld's own fields start at +0x038 */
/* clang-format on */

struct ModelDataMethods {
    MODELDATA_SLOTS(ModelData, (ModelData * self, struct ResourceSource *src, s32 owns));
};

struct ModelData {
    MODELDATA_FIELDS(ModelDataMethods);
};

extern ModelDataMethods gModelDataMethods;
extern ModelDataMethods *GetModelDataMethods(void);

ModelData *New_ModelData(struct ResourceSource *src);
void *ModelData__ModelData(ModelData *self, struct ResourceSource *src, s32 owns);
void ModelData__Finalize(ModelData *self);
void ModelData__Load(ModelData *self);
s32 ModelData__BuildResources(ModelData *self);
void ModelData__ReleaseResources(ModelData *self);
u8 ModelData__ForwardScanPackets(ModelData *self, u8 *out, u32 *tmdId);
void *ModelData__ForwardDecodePacketWord(ModelData *self, u32 *packet, u8 *objId, u8 *type,
                                         u8 *flag, u8 *len);

#endif
