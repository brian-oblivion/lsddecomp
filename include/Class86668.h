#ifndef CLASS86668_H
#define CLASS86668_H

#include "IntermediateBase.h"

/*
 * Class86668 -- class id 0x230, method table gClass86668Methods: the
 * IntermediateBase subclass that adds a frame timeout and a sound object to
 * IntermediateBase's run-one-job-to-a-result. Methods in src/class_39e08.c
 * (New_Class86668 through Class86668__SetTimeout) and src/class_3ac78.c
 * (Class86668__PlaySound, GetClass86668Methods). The object is 0x38 bytes
 * (New_Class86668). Two classes derive from it, each ctor calling
 * Class86668__Class86668 first (`typeviews.py --tree`): gClass865C8Methods
 * (0x1F230, Class865C8, class_39e08) and gObjMMethods (0x2F230, ObjM,
 * class_3bb8c_k/_l/_m).
 *
 * Construction, ctor(soundBankPath, sound): the base ctor, then `sound` =
 * New_VabStreamObj(soundBankPath) when a path is given (Class865C8 passes
 * GetSoundEffectDir()) or the caller's object (ObjM passes Class865C8's), and
 * resetCounters, which here is Class86668__CancelTimeout: setTimeout(-1).
 * Finalize releases `sound` only when the ctor made it, then runs the base
 * finalize. playSound(tone) is the sound's +0x080, VabStreamObj__PlayTone,
 * with 0x7F, 0x7F (TaskCore__PlaySound's shape; ObjM calls the same object's
 * +0x088/+0x08C, VabStreamObj's Mute/Unmute).
 *
 * The timeout. setTimeout(n) stores n * 20 frames, or n itself when
 * negative. update (Class86668__CheckTimeout) runs the base update, which
 * counts frames, then calls setState(4) once frameCounter passes
 * timeoutFrames as UNSIGNED, so -1 never fires. setState
 * (Class86668__SetState) runs the base setState and, for 4, sets result = 1
 * and calls onState4. init (Class86668__Init) zeroes `result`, runs the base
 * init and returns `result`: Class6D3C8__PollStatusObj, through Class865C8,
 * switches on it (2 and 3 are Class865C8's own codes).
 *
 * Slots +0x074..+0x07C are NULL in this class's own table (the table is 0x80
 * bytes, its last three words zero; classtable.py stops at the last
 * non-NULL slot). +0x07C is the one this class calls.
 *
 * IntermediateBase's onInit (+0x04C) is (self, s32, s32, s32). Of this
 * class's two subclasses, ObjM__InitStyleAndWorld takes three arguments
 * after self and Class865C8__OnInit takes self alone; neither is retyped
 * here.
 */

typedef struct Class86668 Class86668;
typedef struct Class86668Methods Class86668Methods;

/* clang-format off */
#define CLASS86668_SLOTS(Self, CtorParams)                                                         \
    INTERMEDIATEBASE_SLOTS(Self, CtorParams);                                                      \
    /* +0x06C */ void (*setTimeout)(Self *self, s32 timeout); /* Class86668__SetTimeout: timeoutFrames = timeout * 20 (negative: kept, never fires) */ \
    /* +0x070 */ void (*playSound)(Self *self, s32 tone);     /* Class86668__PlaySound: sound's PlayTone(tone, 0x7F, 0x7F) */ \
    /* +0x074 */ void (*slot74)(Self *self);                  /* NULL; ObjM__TogglePause */ \
    /* +0x078 */ void *slot78;                                /* NULL in all three tables */     \
    /* +0x07C */ void (*onState4)(Self *self)                 /* NULL; setState(4) calls it; empty in both subclasses */
/* clang-format on */

/* clang-format off */
#define CLASS86668_FIELDS(Methods)                                                                 \
    INTERMEDIATEBASE_FIELDS(Methods);                                                              \
    /* +0x028 */ s32 result;          /* init zeroes it and returns it; setState(4) sets 1 */      \
    /* +0x02C */ s32 timeoutFrames;   /* setTimeout; update: frameCounter past it (unsigned) is setState(4) */ \
    /* +0x030 */ char *soundBankPath; /* the ctor's; nonzero: finalize releases `sound` */         \
    /* +0x034 */ BasicClass *sound    /* New_VabStreamObj(soundBankPath) or the ctor's own; the object is 0x38 bytes */
/* clang-format on */

struct Class86668Methods {
    CLASS86668_SLOTS(Class86668, (Class86668 * self, char *soundBankPath, BasicClass *sound));
};

struct Class86668 {
    CLASS86668_FIELDS(Class86668Methods);
};

extern Class86668Methods gClass86668Methods;
extern Class86668Methods *GetClass86668Methods(void); /* returns &gClass86668Methods */

Class86668 *New_Class86668(char *soundBankPath, BasicClass *sound);
void Class86668__Class86668(Class86668 *self, char *soundBankPath, BasicClass *sound);
void Class86668__Finalize(Class86668 *self);
void Class86668__CancelTimeout(Class86668 *self);
s32 Class86668__Init(Class86668 *self, IntermediateBaseInitArgs *args, s32 mode);
void Class86668__Deinit(Class86668 *self);
void Class86668__NoOpSlot58(void);
void Class86668__CheckTimeout(Class86668 *self, BasicClass *sender, s32 event);
void Class86668__SetState(Class86668 *self, s32 state);
void Class86668__SetTimeout(Class86668 *self, s32 timeout);
void Class86668__PlaySound(Class86668 *self, s32 tone);

#endif
