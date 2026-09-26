#ifndef CLASS_6D3C8_H
#define CLASS_6D3C8_H

#include "Class6E4F0.h"

/*
 * Class6D3C8 -- class id 0x1F60, method table D_8006D3C8 (25 slots), a
 * subclass of Class6E4F0 (include/Class6E4F0.h): its ctor calls
 * GetClass6E4F0Methods()->ctor first, and `classtable.py D_8006D3C8 --vs
 * D_8006E4F0` shares every slot but +0x008/+0x040/+0x044 and the six
 * +0x050..+0x064 slots Class6E4F0 leaves NULL. No class derives from it.
 * Methods in src/code_1677c.c; the table getter is in src/code_171e0.c.
 *
 * It is the game's root object: main() (src/main.c) builds exactly one,
 * New_Class6D3C8(&gClass6D3C8CtorArgs) into gClass6D3C8, then runs its
 * initSystems and runMainLoop. What its own methods do: the ctor loads
 * "ETC\DREAME5.TMD" and builds the owned DreamSys from it; the six slots
 * Class6E4F0's runMainLoop calls load the ASMK/OSD intro logos and their
 * stream (+0x050), start the day-of-week stream (+0x054), poll the
 * GraphRoom tasks against the DreamSys status (+0x058), and run a status
 * object whose code can start the current cinematic (+0x060, +0x064). The
 * name stays the table-address identity: nothing yet names what the class
 * IS beyond "the object main runs".
 *
 * Slots are Class6E4F0's, flat (CLASS6E4F0_SLOTS); that header already names
 * +0x050..+0x064 for this class's occupants. Two overrides take a different
 * parameter list from the slot they fill, so the slot keeps the inherited
 * type and the caller casts (FINISHING-PLAN track 4 step 6):
 *   +0x040 setScreenDims <- Class6D3C8__SetDayFromTickCount (self only);
 *          the ctor calls it through Class6D3C8SetDayFn.
 *   +0x044 initSystems   <- Class6D3C8__InitSystems (no 4th argument);
 *          main calls it through Class6D3C8InitSystemsFn.
 *
 * Object size 0x2C (New_Class6D3C8's BMemPMgrAlloc(0x2C)). +0x000..+0x01F
 * are Class6E4F0's: this class's methods pass the parent's `aux` (+0x01C)
 * as the IntermediateBaseInitArgs of every task they start (it is the
 * 0x14-byte block initSystems allocates: {drawSystem, pad, 0, 0, 0}), and
 * InitSystems tests the parent's `initialized` (+0x018).
 */

typedef struct Class6D3C8 Class6D3C8;
typedef struct Class6D3C8Methods Class6D3C8Methods;

/* The ctor's argument block. One instance, gClass6D3C8CtorArgs =
 * {0x13, 0, 1, 1, 1, 1} (asm/data/57028.data.s), kept by the ctor at
 * self->ctorArgs; each gate below is read `!= 0` by the methods it names. */
typedef struct Class6D3C8CtorArgs {
    /* +0x00 */ s32 dataSource;     /* Class6E4F0's ctor argument (0x13 = the CD driver's class id) */
    /* +0x04 */ s32 unk04;          /* New_Obj865C8's 3rd argument, in PollStatusObj */
    /* +0x08 */ s32 playStreams;    /* gates every StreamTask: StartWeeklyStreamTask, StartGraphRoomStreamTask,
                                     * StartCinematicStream's stream branch, StartStreamTaskWithInit */
    /* +0x0C */ s32 showIntroLogos; /* gates LoadIntroLogoSequence */
    /* +0x10 */ s32 pollGraphRoom;  /* gates PollGraphRoomStatus (0: it returns 2 at once) */
    /* +0x14 */ s32 unk14;          /* the ctor passes it to the DreamSys's func_228 */
} Class6D3C8CtorArgs;

struct Class6D3C8Methods {
    CLASS6E4F0_SLOTS(Class6D3C8, (Class6D3C8 *self, Class6D3C8CtorArgs *args));
};

struct Class6D3C8 {
    CLASS6E4F0_FIELDS(Class6D3C8Methods);
    /* +0x020 */ Class6D3C8CtorArgs *ctorArgs; /* the ctor's argument */
    /* +0x024 */ s32 skipGraphRoomPoll;        /* ctor clears; PollStatusObj sets it on status 3; PollGraphRoomStatus
                                                * skips its New_GraphRoom poll while set, then clears it */
    /* +0x028 */ struct DreamSys *dreamSys;    /* the ctor's New_DreamSys() */
};

/* The two overrides whose parameter lists differ from their slots'. */
typedef void (*Class6D3C8SetDayFn)(Class6D3C8 *self);
typedef void (*Class6D3C8InitSystemsFn)(Class6D3C8 *self, DrawSystem *drawSystem, struct Pad *pad);

extern Class6D3C8Methods D_8006D3C8;
extern Class6D3C8Methods *GetClass6D3C8Methods(void); /* returns &D_8006D3C8 */

Class6D3C8 *New_Class6D3C8(Class6D3C8CtorArgs *args);
void Class6D3C8__Class6D3C8(Class6D3C8 *self, Class6D3C8CtorArgs *args);
void Class6D3C8__SetDayFromTickCount(Class6D3C8 *self);
void Class6D3C8__InitSystems(Class6D3C8 *self, DrawSystem *drawSystem, struct Pad *pad);
void Class6D3C8__LoadIntroLogoSequence(Class6D3C8 *self);
void Class6D3C8__StartLoaderTask(Class6D3C8 *self, const char *path);
s32 Class6D3C8__LoaderTaskDoneCallback(void);
void Class6D3C8__StartWeeklyStreamTask(Class6D3C8 *self);
s32 Class6D3C8__PollGraphRoomStatus(Class6D3C8 *self);
void Class6D3C8__StartGraphRoomStreamTask(Class6D3C8 *self);
void Class6D3C8__NoOpSlot5C(void);
s32 Class6D3C8__PollStatusObj(Class6D3C8 *self);
void Class6D3C8__StartCinematicStream(Class6D3C8 *self);
void Class6D3C8__StartStreamTaskWithInit(Class6D3C8 *self);

#endif
