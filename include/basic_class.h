#ifndef BASIC_CLASS_H
#define BASIC_CLASS_H

#include "common.h"

/**
 * @file basic_class.h
 * @brief BasicClass, the root of the game's class framework, and the macros
 * every other class header expands to inherit from it.
 *
 * Declares the object and method-table layout every class starts with
 * (BASICCLASS_FIELDS, BASICCLASS_SLOTS), BasicClass's own methods, and the
 * pool-backed list primitives its child and parent-reference lists use. The
 * methods are defined in src/app/bmem_pmgr.c. BasicClass's documentation
 * below also describes how every class header in include/ is laid out.
 */

typedef struct BasicClass BasicClass;
typedef struct BasicClassMethods BasicClassMethods;
typedef struct BasicClassListNode BasicClassListNode;

/** The one notifyParents event the base class defines: finalize sends it, and
 * the base onNotify answers it by dropping the sender from its children.
 * Subclasses number their own events (dream_sys.h, entity.h). */
enum { BASICCLASS_EVENT_FINALIZED = 1 };

/** A class id's lowest nibble: which direct subclass of BasicClass (id 0x0)
 * a class is or derives from. `(header & CLASS_ID_ROOT_MASK) == <CLASS>_CLASS_ID`
 * is the is-kind-of test for a class one level below BasicClass. */
#define CLASS_ID_ROOT_MASK 0xF

/** One node of a BasicClass child or parent-reference list. 8 bytes: the
 * size PushBasicClassListNode asks the pool allocator for. */
struct BasicClassListNode {
    /* +0x000 */ BasicClassListNode *next; /**< the next node; NULL ends the list */
    /* +0x004 */ BasicClass *value;        /**< the object this node holds */
};

/** The fifteen slots every method table starts with, for a class whose ctor
 * returns nothing. Each slot names its occupant in BasicClass's own table; a
 * subclass's table replaces the ones it overrides. */
#define BASICCLASS_SLOTS(Self, CtorParams) BASICCLASS_SLOTS_R(Self, void, CtorParams)

/** The same fifteen slots with the ctor's RETURN type as a parameter too: a
 * class whose constructor returns self or NULL (SceneNode and everything
 * below it: New_SceneNode tests `ctor(obj) != NULL`) passes `void *`.
 * BasicClass's own ctor returns nothing, so BASICCLASS_SLOTS passes `void`. */
/* clang-format off */
#define BASICCLASS_SLOTS_R(Self, CtorRet, CtorParams)                                              \
    /* +0x000 */ s32 header; /**< class id; `(header & mask) == id` is is-kind-of */                 \
    /* +0x004 */ void *(*release)(Self *self);                 /**< @see BasicClass__Release: finalize, free self, NULL */ \
    /* +0x008 */ CtorRet (*ctor) CtorParams;                   /**< @see BasicClass__BasicClass */        \
    /* +0x00C */ void (*finalize)(Self *self);                 /**< @see BasicClass__Finalize: notifyParents(1), removeAllChildren, clearParentRefs */ \
    /* +0x010 */ void (*addChild)(Self *self, BasicClass *child);      /**< @see BasicClass__AddChild */  \
    /* +0x014 */ void (*removeChild)(Self *self, BasicClass *child);   /**< @see BasicClass__RemoveChild */ \
    /* +0x018 */ void (*removeAllChildren)(Self *self);        /**< @see BasicClass__RemoveAllChildren */ \
    /* +0x01C */ void (*getNextChild)(Self *self, BasicClass **outChild, BasicClassListNode **cursor); /**< @see BasicClass__GetNextChild */ \
    /* +0x020 */ s32 (*addParentRef)(Self *self, BasicClass *parent);  /**< @see BasicClass__AddParentRef */ \
    /* +0x024 */ void (*removeParentRef)(Self *self, BasicClass *parent); /**< @see BasicClass__RemoveParentRef */ \
    /* +0x028 */ void (*clearParentRefs)(Self *self);          /**< @see BasicClass__ClearParentRefs */   \
    /* +0x02C */ void (*getNextParentRef)(Self *self, BasicClass **outParent, BasicClassListNode **cursor); /**< @see BasicClass__GetNextParentRef */ \
    /* +0x030 */ void (*notifyParents)(Self *self, s32 event); /**< @see BasicClass__NotifyParents */     \
    /* +0x034 */ void (*slot34)(void);                         /**< @see BasicClass__NoOpSlot34, empty; never overridden, never called */ \
    /* +0x038 */ void (*onNotify)(Self *self, void *sender, s32 event); /**< @see BasicClass__OnNotify */  \
    /* +0x03C */ void *slot3C                                  /**< NULL in every class's table */
/* clang-format on */

/** The object fields every class starts with. `Methods` is the class's own
 * method-table type, so `self->methods->slot` is typed for the subclass. */
/* clang-format off */
#define BASICCLASS_FIELDS(Methods)                                                                 \
    /* +0x000 */ Methods *methods;              /**< the class's method table */                   \
    /* +0x004 */ BasicClassListNode *children;  /**< objects added with addChild */                \
    /* +0x008 */ BasicClassListNode *parentRefs /**< objects holding this one as a child */
/* clang-format on */

/** BasicClass's method table: the fifteen base slots and nothing else. */
struct BasicClassMethods {
    BASICCLASS_SLOTS(BasicClass, (BasicClass * self));
};

/**
 * BasicClass -- the root of the game's class framework: class id 0x0, method
 * table gBasicClassMethods, no parent. Its methods are in src/app/bmem_pmgr.c.
 *
 * Every class derives from it. Word +0x000 of each method table is a
 * hierarchical class id: each nibble above the lowest non-zero one is one
 * more level of derivation (the parent of 0x1F234 is 0xF234, then 0x234,
 * 0x34, 0x4 and 0x0). Every table starts with the fifteen slots of
 * BASICCLASS_SLOTS, replaced where a subclass overrides them.
 *
 * A BasicClass keeps two singly-linked lists of other BasicClass objects,
 * built from pool-allocated 8-byte BasicClassListNodes:
 *  - `children`: objects added with addChild/removeChild. Adding one also
 *    registers `self` in the child's own `parentRefs`
 *    (child->methods->addParentRef(child, self)), so a child can find every
 *    object holding it.
 *  - `parentRefs`: that back-reference list. notifyParents(self, event) walks
 *    it and calls each parent's onNotify(parent, self, event); the base
 *    onNotify drops the sender from its children on BASICCLASS_EVENT_FINALIZED,
 *    which finalize sends.
 *
 * Lifecycle: a subclass's allocator takes the object from the BMemPMgr pool
 * and runs its ctor, which chains to its parent's ctor first
 * (`GetBasicClassMethods()->ctor((BasicClass *)self)` at the root) and then
 * installs its own table. `release` runs finalize, returns the memory to the
 * pool and returns NULL, so `obj = obj->methods->release(obj)` clears the
 * caller's pointer.
 *
 * HOW A CLASS IS DECLARED (this header is the model). One header per class,
 * named for the class, holds exactly: the object struct, the method-table
 * struct, the table's extern and getter, and the prototypes of the class's
 * own methods; a class that HAS subclasses also defines a FIELDS and a SLOTS
 * macro. A subclass inherits by expanding its parent's macros first, so every
 * accessor stays flat (`self->children`, `self->methods->addChild(self, c)`)
 * at any depth:
 *
 * @code
 *     struct PadMethods {
 *         BASICCLASS_SLOTS(Pad, (Pad *self, s32 mode, s32 port));
 *         void (*init)(Pad *self, s32 port);    (+0x040 onward: Pad's own)
 *     };
 *     struct Pad {
 *         BASICCLASS_FIELDS(PadMethods);
 *         u16 port;                             (+0x00C onward: Pad's own)
 *     };
 * @endcode
 *
 * SLOTS takes the class's own type, so an inherited slot's `self` is typed as
 * the subclass, and the constructor's parameter list, because +0x008 is the
 * one slot whose signature every class redefines. Calling the BASE class's
 * implementation goes through the base table and upcasts:
 * `GetBasicClassMethods()->finalize((BasicClass *)self)`. Nothing else in a
 * slot differs between a class and its subclasses: an override that needed a
 * different return type or parameter list would be a different slot. The one
 * exception is the ctor's return type, for the same reason as its parameters:
 * BASICCLASS_SLOTS_R takes it (include/scene_node.h, whose ctor returns self).
 */
struct BasicClass {
    BASICCLASS_FIELDS(BasicClassMethods);
};

/** BasicClass's own method table. */
extern BasicClassMethods gBasicClassMethods;

/** @brief BasicClass's method-table getter; subclasses reach the base
 * implementations through it.
 * @return &gBasicClassMethods */
extern BasicClassMethods *GetBasicClassMethods(void);

/** @brief Finalizes the object and returns its memory to the pool.
 * @param self the object to destroy
 * @return NULL, for the caller to store over its pointer */
void *BasicClass__Release(BasicClass *self);

/** @brief Constructor: installs gBasicClassMethods and empties both lists.
 * @param self the object to initialize */
void BasicClass__BasicClass(BasicClass *self);

/** @brief Tells every parent the object is going away
 * (BASICCLASS_EVENT_FINALIZED), then removes all children and frees the
 * parent-reference list.
 * @param self the object being finalized */
void BasicClass__Finalize(BasicClass *self);

/** @brief Prepends `child` to the children and, if the node was allocated,
 * registers `self` in the child's parent references.
 * @param self the new parent
 * @param child the object to add */
void BasicClass__AddChild(BasicClass *self, BasicClass *child);

/** @brief Unlinks `child` from the children and drops `self` from the
 * child's parent references.
 * @param self the parent
 * @param child the object to remove */
void BasicClass__RemoveChild(BasicClass *self, BasicClass *child);

/** @brief Removes every child through the removeChild slot, so an override
 * sees each one.
 * @param self the parent */
void BasicClass__RemoveAllChildren(BasicClass *self);

/** @brief Iterates the children: restarts at the first child when *outChild is NULL,
 * then yields the next child (NULL at the end).
 * @param self the parent
 * @param outChild in: NULL to start; out: the next child, or NULL
 * @param cursor iteration state the caller keeps between calls */
void BasicClass__GetNextChild(BasicClass *self, BasicClass **outChild, BasicClassListNode **cursor);

/** @brief Prepends `parent` to the parent-reference list.
 * @param self the child
 * @param parent the object that now holds `self`
 * @return 1 when a node was allocated, 0 when the pool allocation failed */
s32 BasicClass__AddParentRef(BasicClass *self, BasicClass *parent);

/** @brief Unlinks `parent` from the parent-reference list.
 * @param self the child
 * @param parent the object that no longer holds `self` */
void BasicClass__RemoveParentRef(BasicClass *self, BasicClass *parent);

/** @brief Frees every node of the parent-reference list and empties it.
 * @param self the object */
void BasicClass__ClearParentRefs(BasicClass *self);

/** @brief Iterates the parent references, as BasicClass__GetNextChild does
 * the children.
 * @param self the child
 * @param outParent in: NULL to start; out: the next parent, or NULL
 * @param cursor iteration state the caller keeps between calls */
void BasicClass__GetNextParentRef(BasicClass *self, BasicClass **outParent, BasicClassListNode **cursor);

/** @brief Calls each parent's onNotify with `self` as the sender.
 * @param self the sender
 * @param event what happened (BASICCLASS_EVENT_FINALIZED or a subclass's code) */
void BasicClass__NotifyParents(BasicClass *self, s32 event);

/** @brief Slot +0x034: empty; no table overrides it and nothing calls it. */
void BasicClass__NoOpSlot34(void);

/** @brief The receiving half of NotifyParents: on BASICCLASS_EVENT_FINALIZED
 * drops `sender` from the children. Overrides call this first, then act on
 * the sender's class and the event code.
 * @param self the parent being told
 * @param sender the object that sent the event
 * @param event the event code */
void BasicClass__OnNotify(BasicClass *self, void *sender, s32 event);

/** @brief Allocates a list node from the pool and prepends it to *head.
 * @param head the list to grow
 * @param value the object the node holds
 * @return 1 on success, 0 when the pool allocation failed */
extern s32 PushBasicClassListNode(BasicClassListNode **head, BasicClass *value);

/** @brief Unlinks and frees the first node holding `value`, if any.
 * @param head the list to search
 * @param value the object to remove */
extern void RemoveBasicClassListNode(BasicClassListNode **head, BasicClass *value);

/** @brief Yields the node under *cursor and advances it.
 * @param outValue the node's object, or NULL once *cursor is NULL
 * @param cursor the node to read; left at the next one */
extern void GetNextBasicClass(BasicClass **outValue, BasicClassListNode **cursor);

/** @brief Frees every node of a list; *head is left as it was.
 * @param head the list to free */
extern void FreeBasicClassList(BasicClassListNode **head);

/** @brief Releases each of `count` objects and stores release's NULL back
 * over each entry.
 * @param array the objects
 * @param count how many */
extern void ReleaseBasicClassArray(BasicClass **array, s32 count);

#endif
