#ifndef PAD_H
#define PAD_H

#include "basic_class.h"

/**
 * @file pad.h
 * @brief Pad, a controller port wrapped as a BasicClass, and the button
 * events it sends to its parents.
 *
 * Declares the class, its event codes and button indices, and its methods,
 * defined in src/app/pad.c.
 */

/** @name Pad events
 * dispatchEvents' event codes: one base per edge plus the button's index in
 * sButtonMasks (PAD_EVENT_PRESSED + PAD_BUTTON_START is Start going down). @{ */
#define PAD_EVENT_HELD 0x02
#define PAD_EVENT_PRESSED 0x12
#define PAD_EVENT_RELEASED 0x22
/** @} */

/** Pad's class id (gPadMethods word +0x000). A single nibble, so
 * `(header & 0xF) == PAD_CLASS_ID` is its is-kind-of test (TextEntry's
 * addChild/removeChild/onNotify). */
#define PAD_CLASS_ID 0x2

/** sButtonMasks' indices. Pad__LoadButtonTable fills it from one fixed
 * table (sDefaultButtonMasks), whose words are libetc's masks in this order. */
enum PadButton {
    PAD_BUTTON_LUP = 0,     /**< PADLup */
    PAD_BUTTON_LDOWN = 1,   /**< PADLdown */
    PAD_BUTTON_LLEFT = 2,   /**< PADLleft */
    PAD_BUTTON_LRIGHT = 3,  /**< PADLright */
    PAD_BUTTON_RUP = 4,     /**< PADRup (triangle) */
    PAD_BUTTON_RDOWN = 5,   /**< PADRdown (cross) */
    PAD_BUTTON_RLEFT = 6,   /**< PADRleft (square) */
    PAD_BUTTON_RRIGHT = 7,  /**< PADRright (circle) */
    PAD_BUTTON_I = 8,       /**< PADi */
    PAD_BUTTON_J = 9,       /**< PADj */
    PAD_BUTTON_SELECT = 10, /**< PADk, PADselect */
    PAD_BUTTON_R1 = 11,     /**< PADl, PADR1 */
    PAD_BUTTON_R2 = 12,     /**< PADm, PADR2 */
    PAD_BUTTON_L1 = 13,     /**< PADn, PADL1 */
    PAD_BUTTON_L2 = 14,     /**< PADo, PADL2 */
    PAD_BUTTON_START = 15,  /**< PADh, PADstart */
    PAD_BUTTON_COUNT = 16   /**< sButtonMasks' length, one event per button per frame at most */
};

typedef struct Pad Pad;
typedef struct PadMethods PadMethods;

/** Pad's method table: BasicClass's slots (ctor and finalize overridden),
 * then six of its own. */
struct PadMethods {
    BASICCLASS_SLOTS(Pad, (Pad * self, s32 mode, s32 port));
    /* +0x040 */ void (*init)(Pad *self, s32 port); /**< @see Pad__Init */
    /* +0x044 */ u32 (*updateMasks)(Pad *self); /**< @see Pad__UpdateMasks: returns the new held mask */
    /* +0x048 */ void (*dispatchEvents)(Pad *self); /**< @see Pad__DispatchEvents */
    /* +0x04C */ void (*slot4C)(void);              /**< Pad__NoOpSlot4C, empty, never called */
    /* +0x050 */ void (*loadButtonTable)(void);     /**< @see Pad__LoadButtonTable */
    /* +0x054 */ void (*slot54)(void);              /**< Pad__NoOpSlot54, empty, never called */
};

/**
 * Pad -- a controller port: class id 0x2, method table gPadMethods, a direct
 * BasicClass subclass. Methods in src/app/pad.c; main() creates the one
 * instance, New_Pad(0, 0).
 *
 * A game-side wrapper around the Psy-Q pad library (<libetc.h>): the first
 * live instance calls PadInit and the last calls PadStop (sPadRefCount). On
 * every DrawSystem VSync event (IntermediateBase__OnDrawSystemEvent)
 * updateMasks turns two consecutive PadRead words into held, pressed and
 * released edge masks, and dispatchEvents turns those into one event code
 * per button (PAD_EVENT_* plus the button's index) and sends each, highest
 * button first, through the inherited notifyParents slot: a Pad's parents
 * receive its button events in their onNotify(parent, pad, event).
 *
 * The object is 0x20 bytes (New_Pad).
 */
struct Pad {
    BASICCLASS_FIELDS(PadMethods);
    /* +0x00C */ u16 port; /**< 0 or 1: the ctor boolifies its argument */
    /* +0x00E */ u8 pad0E[2];
    /* +0x010 */ u32 heldMask;     /**< this frame's PadRead word */
    /* +0x014 */ u32 releasedMask; /**< buttons down last frame and up this one */
    /* +0x018 */ u32 pressedMask;  /**< buttons up last frame and down this one */
    /* +0x01C */ u8 pad1C[4];
};

/** Pad's own method table. */
extern PadMethods gPadMethods;

/** @brief Pad's method-table getter.
 * @return &gPadMethods */
extern PadMethods *GetPadMethods(void);

/** @brief Allocates a Pad from the pool and runs its ctor.
 * @param mode PadInit's mode, used by the first live instance only
 * @param port the controller port; nonzero is port 1
 * @return the new Pad, or NULL when the pool allocation fails */
Pad *New_Pad(s32 mode, s32 port);

/** @brief Constructor: BasicClass's, then this table; the first live
 * instance calls PadInit(mode); then init(port).
 * @param self the object
 * @param mode PadInit's mode
 * @param port the controller port */
void Pad__Pad(Pad *self, s32 mode, s32 port);

/** @brief Finalize: the last live instance calls PadStop; then BasicClass's
 * finalize.
 * @param self the object */
void Pad__Finalize(Pad *self);

/** @brief Selects the port (0 or 1), clears the three masks and loads the
 * button table.
 * @param self the object
 * @param port nonzero selects port 1 */
void Pad__Init(Pad *self, s32 port);

/** @brief Reads the port and derives this frame's edge masks from the last
 * one: releasedMask and pressedMask are the bits that changed, split by
 * their old and new state.
 * @param self the object
 * @return the new held mask */
u32 Pad__UpdateMasks(Pad *self);

/** @brief Sends one event per button with an edge or held bit to the
 * parents (notifyParents), highest button index first: released over
 * pressed over held. Sends nothing when all three masks are 0.
 * @param self the object */
void Pad__DispatchEvents(Pad *self);

/** @brief Copies sDefaultButtonMasks, the fixed table of libetc's masks in
 * PadButton order, into sButtonMasks. */
void Pad__LoadButtonTable(void);

#endif
