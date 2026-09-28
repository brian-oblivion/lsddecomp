#ifndef LINK_RESOURCE_H
#define LINK_RESOURCE_H

#include "file_resource.h"

/*
 * LinkResource -- a FileResource data source (class id 0xD03, method table
 * gLinkResourceMethods) over one loaded TMD file: it builds one TmdModel
 * (include/tmd_model.h) per object of the TMD and hands them out by index.
 * Methods in src/graphics/graphics_resources.c; no subclasses.
 *
 * What its own methods do: map the file's TMD
 * (LinkResource__MapModel: GsMapModelingData(&file->flags)) and build and
 * return the TmdModel objects the callers LINK -- tod_actor.c's TOD
 * model-id packet passes getModel's result to SceneNode__LinkModel, and
 * dream_day.c links the TmdObject behind it with GsLinkObject4.
 *
 * Holders: ModelData's `linkResource` (ModelData__BuildResources, over the
 * TMD sub-block of a MOM file), PlacementGrid's `linkResource`
 * (StageMap__PopulateSlotCells, the element's models), DayTask's
 * `dreamerTmd` ("ETC\DREAMER.TMD") and DreamSys's ctor argument
 * (GameApplication__GameApplication, "ETC\DREAME5.TMD": DreamSys__DreamSys adds
 * getModel(0) as its child).
 *
 * Its parent ctor is the active driver's (GetActiveDataSourceMethods()->ctor,
 * chosen at run time; see include/model_data.h), so the fields below assume
 * FileResource's own 0x2C-byte layout. The object is 0x30 bytes
 * (New_LinkResource).
 *
 * Inherited slots keep FileResource's names and types; this table's occupants
 * differ from them in two places, and the callers cast:
 *   +0x008 ctor: LinkResource__LinkResource returns self, or NULL when the
 *          buffer it adopted fails to build (New_LinkResource tests it,
 *          through graphics_resources.c's unprototyped UnprototypedCtorTable view).
 *   +0x064 onRequestDone: LinkResource__BuildModels(self), s32: 1 when an
 *          allocation fails, else 0 after the active driver's onRequestDone.
 *   +0x078 processBuffer (NULL in FileResource): LinkResource__MapModel(self).
 *
 * The ctor's descriptor is ResourceSource (include/file_resource.h): a buffer
 * to adopt, else a file name to request.
 */

struct TmdModel;
struct TmdObject;

typedef struct LinkResource LinkResource;
typedef struct LinkResourceMethods LinkResourceMethods;

struct LinkResourceMethods {
    FILERESOURCE_SLOTS(LinkResource, (LinkResource * self, ResourceSource *src));
    /* +0x07C */ struct TmdObject *(*getTmdObject)(LinkResource *self, s32 index); /* LinkResource__GetTmdObject */
    /* +0x080 */ struct TmdModel *(*getModel)(LinkResource *self, s32 index); /* LinkResource__GetModel */
    /* +0x084 */ void (*slot84)(void); /* LinkResource__NoOp */
};

struct LinkResource {
    FILERESOURCE_FIELDS(LinkResourceMethods);
    /* +0x02C */ struct TmdModel **models; /* NULL-ended, one per TMD object (BuildModels); released and freed by Finalize */
};

/* The occupants of +0x064 and +0x078 as the ctor and BuildModels call them
 * (no code: a function-pointer cast). */
typedef s32 (*LinkResourceBuildModelsFn)(LinkResource *self);
typedef void (*LinkResourceMapModelFn)(LinkResource *self);

extern LinkResourceMethods gLinkResourceMethods;
extern LinkResourceMethods *GetLinkResourceMethods(void);

LinkResource *New_LinkResource(ResourceSource *src);
void *LinkResource__LinkResource(LinkResource *self, ResourceSource *src);
void LinkResource__Finalize(LinkResource *self);
s32 LinkResource__BuildModels(LinkResource *self);
void LinkResource__MapModel(LinkResource *self);
struct TmdObject *LinkResource__GetTmdObject(LinkResource *self, s32 index);
struct TmdModel *LinkResource__GetModel(LinkResource *self, s32 index);
void LinkResource__NoOp(void);

#endif
