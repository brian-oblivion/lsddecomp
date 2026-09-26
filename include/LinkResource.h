#ifndef LINKRESOURCE_H
#define LINKRESOURCE_H

#include "Class6D430.h"

/*
 * LinkResource -- a Class6D430 data source (class id 0xD03, method table
 * gLinkResourceMethods) over one loaded TMD file: it builds one TmdModel
 * (include/TmdModel.h) per object of the TMD and hands them out by index.
 * Methods in src/code_33808.c; no subclasses.
 *
 * The name is round 20's, from class_3bb8c.c's local view of the object
 * Class866E8__LoadElementResources stores in a Class6D940's `linkResource`;
 * it is kept on this evidence: the class's own methods map the file's TMD
 * (LinkResource__MapModel: GsMapModelingData(&file->flags)) and build and
 * return the TmdModel objects the callers LINK -- code_55dd4.c's TOD
 * model-id packet passes getModel's result to Class6B5CC__LinkModel, and
 * class_3bb8c.c links the TmdObject behind it with GsLinkObject4.
 *
 * Holders: ModelData's `linkResource` (ModelData__BuildResources, over the
 * TMD sub-block of a MOM file), Class6D940's `linkResource`
 * (Class866E8__LoadElementResources, the element's models), Class865C8's
 * `dreamerTmd` ("ETC\DREAMER.TMD") and DreamSys's ctor argument
 * (Class6D3C8__Class6D3C8, "ETC\DREAME5.TMD": DreamSys__DreamSys adds
 * getModel(0) as its child).
 *
 * Its parent ctor is the active driver's (GetActiveDataSourceMethods()->ctor,
 * chosen at run time; see include/ModelData.h), so the fields below assume
 * Class6D430's own 0x2C-byte layout. The object is 0x30 bytes
 * (New_LinkResource).
 *
 * Inherited slots keep Class6D430's names and types; this table's occupants
 * differ from them in two places, and the callers cast:
 *   +0x008 ctor: LinkResource__LinkResource returns self, or NULL when the
 *          buffer it adopted fails to build (New_LinkResource tests it,
 *          through code_33808.c's unprototyped Ctor33808 view).
 *   +0x064 setFlag: LinkResource__BuildModels(self), s32: 1 when an
 *          allocation fails, else 0 after the active driver's setFlag.
 *   +0x078 slot78 (NULL in Class6D430): LinkResource__MapModel(self).
 *
 * The ctor's descriptor is code_33808.c's Src6F240 ({buffer to adopt, file
 * name to request}); only the tag is declared here. The callers outside
 * code_33808 build it in their own 0x10-byte request types and cast.
 */

struct Src6F240;
struct TmdModel;
struct TmdObject;

typedef struct LinkResource LinkResource;
typedef struct LinkResourceMethods LinkResourceMethods;

struct LinkResourceMethods {
    CLASS6D430_SLOTS(LinkResource, (LinkResource * self, struct Src6F240 *src));
    /* +0x07C */ struct TmdObject *(*getTmdObject)(LinkResource *self, s32 index); /* LinkResource__GetTmdObject */
    /* +0x080 */ struct TmdModel *(*getModel)(LinkResource *self, s32 index); /* LinkResource__GetModel */
    /* +0x084 */ void (*slot84)(void); /* LinkResource__NoOp */
};

struct LinkResource {
    CLASS6D430_FIELDS(LinkResourceMethods);
    /* +0x02C */ struct TmdModel **models; /* NULL-ended, one per TMD object (BuildModels); released and freed by Finalize */
};

/* The occupants of +0x064 and +0x078 as the ctor and BuildModels call them
 * (no code: a function-pointer cast). */
typedef s32 (*LinkResourceBuildModelsFn)(LinkResource *self);
typedef void (*LinkResourceMapModelFn)(LinkResource *self);

extern LinkResourceMethods gLinkResourceMethods;
extern LinkResourceMethods *GetLinkResourceMethods(void);

LinkResource *New_LinkResource(struct Src6F240 *src);
void *LinkResource__LinkResource(LinkResource *self, struct Src6F240 *src);
void LinkResource__Finalize(LinkResource *self);
s32 LinkResource__BuildModels(LinkResource *self);
void LinkResource__MapModel(LinkResource *self);
struct TmdObject *LinkResource__GetTmdObject(LinkResource *self, s32 index);
struct TmdModel *LinkResource__GetModel(LinkResource *self, s32 index);
void LinkResource__NoOp(void);

#endif
