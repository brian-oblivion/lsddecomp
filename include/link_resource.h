/**
 * @file link_resource.h
 * @brief LinkResource, the FileResource that turns one loaded TMD file into
 *        an array of TmdModel objects, and its method table.
 */
#ifndef LINK_RESOURCE_H
#define LINK_RESOURCE_H

#include "file_resource.h"

struct TmdModel;
struct TmdObject;

typedef struct LinkResource LinkResource;
typedef struct LinkResourceMethods LinkResourceMethods;

/** LinkResource's class id (gLinkResourceMethods word +0x000). */
#define LINKRESOURCE_CLASS_ID 0xD03

/**
 * @brief LinkResource's method table, gLinkResourceMethods: FileResource's
 *        slots, then three of its own.
 *
 * Inherited slots keep FileResource's names and types. Three occupants differ
 * from them, and the callers cast: +0x008 ctor is LinkResource__LinkResource,
 * which returns self or NULL; +0x064 onRequestDone is
 * LinkResource__BuildModels, which returns s32 (LinkResourceBuildModelsFn);
 * +0x078 processBuffer is LinkResource__MapModel (LinkResourceMapModelFn).
 */
struct LinkResourceMethods {
    FILERESOURCE_SLOTS(LinkResource, (LinkResource * self, ResourceSource *src));
    /* +0x07C */ struct TmdObject *(*getTmdObject)(LinkResource *self,
                                                   s32 index); /**< @see LinkResource__GetTmdObject */
    /* +0x080 */ struct TmdModel *(*getModel)(LinkResource *self, s32 index); /**< @see LinkResource__GetModel */
    /* +0x084 */ void (*slot84)(void); /**< @see LinkResource__NoOp */
};

/**
 * @brief A TMD model source (class id 0xD03): one loaded or adopted TMD file
 *        and one TmdModel per object in it, handed out by index.
 *
 * Parent FileResource, through whichever data-source driver is active when it
 * is built (its ctor chains to GetActiveDataSourceMethods()->ctor), so its
 * fields follow FileResource's own 0x2C bytes. No subclasses. Methods in
 * src/graphics/graphics_resources.c. The object is 0x30 bytes
 * (New_LinkResource).
 *
 * Callers link the models it builds: tod_actor.c's TOD model-id packet passes
 * getModel's result to SceneNode__LinkModel, and dream_day.c links the
 * TmdObject behind one with GsLinkObject4. Holders: ModelData's
 * `linkResource` (over the TMD in a MOM file), PlacementGrid's
 * `linkResource`, DayTask's `dreamerTmd` ("ETC\DREAMER.TMD") and the
 * DreamSys the GameApplication ctor builds over "ETC\DREAME5.TMD".
 */
struct LinkResource {
    FILERESOURCE_FIELDS(LinkResourceMethods);
    /* +0x02C */ struct TmdModel **models; /**< NULL-ended, one per TMD object (BuildModels); released and freed by Finalize */
};

/** @brief LinkResource__BuildModels as the ctor calls it through the
 *         void-typed onRequestDone slot. */
typedef s32 (*LinkResourceBuildModelsFn)(LinkResource *self);

/** @brief LinkResource__MapModel as BuildModels calls it through the
 *         processBuffer slot. */
typedef void (*LinkResourceMapModelFn)(LinkResource *self);

/** LinkResource's method table. */
extern LinkResourceMethods gLinkResourceMethods;

/**
 * @brief Returns LinkResource's method table.
 * @return &gLinkResourceMethods.
 */
extern LinkResourceMethods *GetLinkResourceMethods(void);

/**
 * @brief Allocates a LinkResource from the pool and constructs it.
 * @param src The descriptor: a TMD buffer to adopt, else a file name to
 *            request.
 * @return The new object, or NULL when the pool is exhausted or the models
 *         of an adopted buffer cannot be built (the object is then freed).
 */
LinkResource *New_LinkResource(ResourceSource *src);

/**
 * @brief Constructor (slot +0x008): the active driver's, then either adopts
 *        the descriptor's buffer and builds its models at once, or requests
 *        the named file.
 * @param self The object to construct.
 * @param src  The descriptor, or NULL for neither.
 * @return self, or NULL when building an adopted buffer's models fails.
 */
void *LinkResource__LinkResource(LinkResource *self, ResourceSource *src);

/**
 * @brief Finalizer (slot +0x00C): releases every model, frees the array, then
 *        runs the active driver's finalizer.
 * @param self The object being destroyed.
 */
void LinkResource__Finalize(LinkResource *self);

/**
 * @brief Slot +0x064 (onRequestDone): maps the TMD, then builds a NULL-ended
 *        array of one TmdModel per TMD object and runs the driver's
 *        onRequestDone.
 * @param self The object, its buffer holding the TMD file.
 * @return 0 on success; 1 when an allocation fails, everything built so far
 *         released.
 */
s32 LinkResource__BuildModels(LinkResource *self);

/**
 * @brief Slot +0x078 (processBuffer): GsMapModelingData over the TMD in the
 *        buffer, turning its offsets into addresses.
 * @param self The object, its buffer holding the TMD file.
 */
void LinkResource__MapModel(LinkResource *self);

/**
 * @brief Slot +0x07C: one entry of the TMD's object table.
 * @param self  The object.
 * @param index The TMD object's index; not range-checked.
 * @return The object table entry, inside the buffer.
 */
struct TmdObject *LinkResource__GetTmdObject(LinkResource *self, s32 index);

/**
 * @brief Slot +0x080: one of the built models.
 * @param self  The object.
 * @param index The model's index; not range-checked.
 * @return The TmdModel built over TMD object `index`.
 */
struct TmdModel *LinkResource__GetModel(LinkResource *self, s32 index);

/** @brief Slot +0x084: does nothing. */
void LinkResource__NoOp(void);

#endif
