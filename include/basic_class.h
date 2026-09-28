#ifndef BASIC_CLASS_H
#define BASIC_CLASS_H

#include "common.h"

/*
 * BasicClass -- the root of the game's hand-rolled class framework
 * (docs/research/class-framework.md), class id 0x0, method table gBasicClassMethods.
 * Methods live in src/app/BMemPMgr.c.
 *
 * Every class derives from it: word +0x000 of each method table is a
 * hierarchical class id (each nibble above the lowest non-zero one is one more
 * level of derivation; `python3 tools/typeviews.py --tree`), and every table
 * starts with these fifteen slots, replaced where the subclass overrides them.
 *
 * BasicClass keeps two singly-linked lists of other BasicClass objects, built
 * from pool-allocated 8-byte BasicClassListNodes:
 *  - `children`: objects added with addChild/removeChild. Adding one also
 *    registers `self` in the child's own `parentRefs` (child->methods->
 *    addParentRef(child, self)), so a child can find every object holding it.
 *  - `parentRefs`: that back-reference list. notifyParents(self, event) walks
 *    it and calls each parent's onNotify(parent, self, event); the base
 *    onNotify drops the sender from its children on event 1 (finalize sends 1).
 *
 * HOW A CLASS IS DECLARED (this header is the model).
 * One header per class, named for the class, holding exactly: the object
 * struct, the method-table struct, the table's extern and getter, and the
 * prototypes of the class's own methods; a class that HAS subclasses also
 * defines a FIELDS and a SLOTS macro. A subclass inherits by expanding its
 * parent's macros first, so every accessor stays flat (`this->children`,
 * `this->methods->addChild(this, c)`) at any depth and cc1 sees exactly the
 * layout it saw when each function matched:
 *
 *     struct PadMethods {
 *         BASICCLASS_SLOTS(Pad, (Pad *self, void *arg1, s32 port));
 *         void (*init)(Pad *self, s32 port);                (+0x040 onward: Pad's own)
 *     };
 *     struct Pad {
 *         BASICCLASS_FIELDS(PadMethods);
 *         u16 port;                                     (+0x00C onward: Pad's own)
 *     };
 *
 * SLOTS takes the class's own type, so an inherited slot's `self` is typed as
 * the subclass, and the constructor's parameter list, because +0x008 is the
 * one slot whose signature every class redefines. Calling the BASE class's
 * implementation goes through the base table and upcasts:
 * `GetBasicClassMethods()->finalize((BasicClass *)self)`. Nothing else in a
 * slot differs between a class and its subclasses: an override that needed a
 * different return type or parameter list would be a different slot. The one
 * exception is the ctor's return type, for the same reason as its parameters:
 * BASICCLASS_SLOTS_R takes it (include/SceneNode.h, whose ctor returns self).
 */

typedef struct BasicClass BasicClass;
typedef struct BasicClassMethods BasicClassMethods;
typedef struct BasicClassListNode BasicClassListNode;

/* The one notifyParents event the base class defines: finalize sends it, and
 * the base onNotify answers it by dropping the sender from its children.
 * Subclasses number their own events (DreamSys.h, entity.h). */
enum { BASICCLASS_EVENT_FINALIZED = 1 };

/* A class id's lowest nibble: which direct subclass of BasicClass (id 0x0)
 * a class is or derives from. `(header & CLASS_ID_ROOT_MASK) == <CLASS>_CLASS_ID`
 * is the is-kind-of test for a class one level below BasicClass. */
#define CLASS_ID_ROOT_MASK 0xF

/* One node of either list. 8 bytes: the size PushBasicClassListNode asks the
 * pool allocator for. */
struct BasicClassListNode {
    /* +0x000 */ BasicClassListNode *next;
    /* +0x004 */ BasicClass *value;
};

/* The fifteen slots every method table starts with. Occupants in BasicClass's
 * own table named at each slot; `tools/classtable.py <table> --vs gBasicClassMethods`
 * lists a subclass's overrides. */
#define BASICCLASS_SLOTS(Self, CtorParams) BASICCLASS_SLOTS_R(Self, void, CtorParams)

/* The same fifteen slots with the ctor's RETURN type as a parameter too: a
 * class whose constructor returns self or NULL (SceneNode and everything
 * below it: New_SceneNode tests `ctor(obj) != NULL`) passes `void *`.
 * BasicClass's own ctor returns nothing, so BASICCLASS_SLOTS passes `void`. */
/* clang-format off */
#define BASICCLASS_SLOTS_R(Self, CtorRet, CtorParams)                                              \
    /* +0x000 */ s32 header; /* class id; `(header & mask) == id` is is-kind-of */                 \
    /* +0x004 */ void *(*release)(Self *self);                 /* BasicClass__Release: finalize, free self, NULL */ \
    /* +0x008 */ CtorRet (*ctor) CtorParams;                   /* BasicClass__BasicClass */        \
    /* +0x00C */ void (*finalize)(Self *self);                 /* BasicClass__Finalize: notifyParents(1), removeAllChildren, clearParentRefs */ \
    /* +0x010 */ void (*addChild)(Self *self, BasicClass *child);      /* BasicClass__AddChild */  \
    /* +0x014 */ void (*removeChild)(Self *self, BasicClass *child);   /* BasicClass__RemoveChild */ \
    /* +0x018 */ void (*removeAllChildren)(Self *self);        /* BasicClass__RemoveAllChildren */ \
    /* +0x01C */ void (*getNextChild)(Self *self, BasicClass **outChild, BasicClassListNode **cursor); /* BasicClass__GetNextChild */ \
    /* +0x020 */ s32 (*addParentRef)(Self *self, BasicClass *parent);  /* BasicClass__AddParentRef */ \
    /* +0x024 */ void (*removeParentRef)(Self *self, BasicClass *parent); /* BasicClass__RemoveParentRef */ \
    /* +0x028 */ void (*clearParentRefs)(Self *self);          /* BasicClass__ClearParentRefs */   \
    /* +0x02C */ void (*getNextParentRef)(Self *self, BasicClass **outParent, BasicClassListNode **cursor); /* BasicClass__GetNextParentRef */ \
    /* +0x030 */ void (*notifyParents)(Self *self, s32 event); /* BasicClass__NotifyParents */     \
    /* +0x034 */ void (*slot34)(void);                         /* BasicClass__NoOpSlot34, empty; never overridden, never called */ \
    /* +0x038 */ void (*onNotify)(Self *self, void *sender, s32 event); /* BasicClass__OnNotify */  \
    /* +0x03C */ void *slot3C                                  /* NULL in all 59 method tables */
/* clang-format on */

/* The object fields every class starts with. `Methods` is the class's own
 * method-table type, so `this->methods->slot` is typed for the subclass. */
/* clang-format off */
#define BASICCLASS_FIELDS(Methods)                                                                 \
    /* +0x000 */ Methods *methods;                                                                 \
    /* +0x004 */ BasicClassListNode *children;                                                     \
    /* +0x008 */ BasicClassListNode *parentRefs
/* clang-format on */

struct BasicClassMethods {
    BASICCLASS_SLOTS(BasicClass, (BasicClass * self));
};

struct BasicClass {
    BASICCLASS_FIELDS(BasicClassMethods);
};

extern BasicClassMethods gBasicClassMethods;          /* BasicClass's own method table */
extern BasicClassMethods *GetBasicClassMethods(void); /* returns &gBasicClassMethods */

/* BasicClass's methods: the occupants of its own table, in BMemPMgr.c.
 * A subclass reaches them through GetBasicClassMethods(). */
void *BasicClass__Release(BasicClass *self);
void BasicClass__BasicClass(BasicClass *self);
void BasicClass__Finalize(BasicClass *self);
void BasicClass__AddChild(BasicClass *self, BasicClass *child);
void BasicClass__RemoveChild(BasicClass *self, BasicClass *child);
void BasicClass__RemoveAllChildren(BasicClass *self);
void BasicClass__GetNextChild(BasicClass *self, BasicClass **outChild, BasicClassListNode **cursor);
s32 BasicClass__AddParentRef(BasicClass *self, BasicClass *parent);
void BasicClass__RemoveParentRef(BasicClass *self, BasicClass *parent);
void BasicClass__ClearParentRefs(BasicClass *self);
void BasicClass__GetNextParentRef(BasicClass *self, BasicClass **outParent, BasicClassListNode **cursor);
void BasicClass__NotifyParents(BasicClass *self, s32 event);
void BasicClass__NoOpSlot34(void);
void BasicClass__OnNotify(BasicClass *self, void *sender, s32 event);

/* The list primitives, in BMemPMgr.c. */
extern s32 PushBasicClassListNode(BasicClassListNode **head,
                                  BasicClass *value); /* allocate a node, prepend it to *head */
extern void RemoveBasicClassListNode(BasicClassListNode **head,
                                     BasicClass *value); /* unlink and free the node holding value */
extern void GetNextBasicClass(BasicClass **outValue,
                              BasicClassListNode **cursor); /* *outValue = node value (or NULL); advance *cursor */
extern void FreeBasicClassList(BasicClassListNode **head); /* free every node; *head is not cleared */

/* Releases each of `count` objects of `array`. */
extern void ReleaseBasicClassArray(BasicClass **array, s32 count);

#endif
