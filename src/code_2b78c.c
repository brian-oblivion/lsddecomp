/*
 * code_2b78c -- GAME code carved from psyq_2b78c on 2026-09-25 (FINISHING-PLAN
 * revision 18). 0x2B78C..0x2BA1C (vram 0x8003AF8C..0x8003B21C). It was counted
 * as Psy-Q SDK by segment name; tools/gameinsdk.py measured it as game (a call
 * into game code, a method-table entry beside game methods, or contiguity with
 * those, and no Sony fingerprint). What it holds: methods of D_8006E4F0 and
 * D_8006D3C8, calling SetActiveDataSource; the yaml had called this gap "the
 * game's own libsnd build".
 *
 * Round 81 (delta): all seven functions matched. They are the whole of one
 * class, D_8006E4F0 (local view Class6E4F0 below): its ctor, empty dtor,
 * four own slots and the table getter.
 */
#include "common.h"
#include "BasicClass.h"

/*
 * The class whose method table is D_8006E4F0 (class id 0x60, 19 slots,
 * `classtable.py 0x8006E4F0 --vs 0x8006B58C`): BasicClass's slots with +0x008
 * and +0x00C overridden, plus four of its own at +0x040..+0x04C. Its only
 * known subclass is Class6D3C8 (include/Class6D3C8.h, which carries its own
 * view of this table as MiddleClassMethods), and +0x04C drives slots
 * +0x050..+0x064 that only the subclass's table fills. This is this unit's
 * local view; no field is named past what these seven bodies show.
 */
typedef struct Class6E4F0 Class6E4F0;
typedef struct Class6E4F0Methods Class6E4F0Methods;

/* A {width, height} pair: the ctor's default is D_8008A8E0 = {320, 240}. */
typedef struct ScreenDims {
    s32 w;
    s32 h;
} ScreenDims;

/* The 0x14-byte block func_8003B044 allocates. */
typedef struct Class6E4F0Aux {
    void *source; /* +0x00 func_8003B044's a1 */
    s32 arg;      /* +0x04 func_8003B044's a2 */
    s32 unk08;
    s32 unk0C;
    s32 unk10;
} Class6E4F0Aux;

/* func_8003B044's a1: an object dispatched through its own +0x044 slot. */
typedef struct Class6E4F0SourceMethods {
    u8 pad00[0x44];
    void (*slot44)(void *self, ScreenDims *dims, s32 arg); /* +0x044 */
} Class6E4F0SourceMethods;

typedef struct Class6E4F0Source {
    Class6E4F0SourceMethods *methods;
} Class6E4F0Source;

struct Class6E4F0Methods {
    BASICCLASS_SLOTS(Class6E4F0, (Class6E4F0 *self, s32 source));
    /* +0x040 */ void (*setDims)(Class6E4F0 *self, ScreenDims *dims, s32 arg); /* func_8003B02C */
    /* +0x044 */ void (*init)(Class6E4F0 *self, Class6E4F0Source *source, s32 arg); /* func_8003B044 */
    /* +0x048 */ void (*slot48)(Class6E4F0 *self);                             /* func_8003B108, empty */
    /* +0x04C */ void (*run)(Class6E4F0 *self);                                /* func_8003B110 */
    /* +0x050.. the subclass's slots, called by func_8003B110 */
    /* +0x050 */ void (*slot50)(Class6E4F0 *self);
    /* +0x054 */ void (*slot54)(Class6E4F0 *self);
    /* +0x058 */ s32 (*slot58)(Class6E4F0 *self);
    /* +0x05C */ void (*slot5C)(Class6E4F0 *self);
    /* +0x060 */ s32 (*slot60)(Class6E4F0 *self);
    /* +0x064 */ void (*slot64)(Class6E4F0 *self);
};

struct Class6E4F0 {
    BASICCLASS_FIELDS(Class6E4F0Methods);
    /* +0x00C */ ScreenDims dims;
    /* +0x014 */ s32 dimsArg;
    /* +0x018 */ s32 initialized;
    /* +0x01C */ Class6E4F0Aux *aux;
};

extern Class6E4F0Methods D_8006E4F0;
extern s32 D_8008A8DC;           /* CdInit has been called */
extern ScreenDims D_8008A8E0;    /* {320, 240} */

Class6E4F0Methods *func_8003B20C(void);

/* Psy-Q LIBCD.H / LIBSND.H / LIBGS.H prototypes. */
extern int CdInit(void);
extern void SsInit(void);
extern void GsInit3D(void);

extern void SetActiveDataSource(s32 arg0); /* code_171e0 */
extern void func_80020C68(void *arg);      /* code_10ee0, stores its arg to a gp global */
extern void *BMemPMgrAlloc(s32 size);

INCLUDE_ASM("asm/nonmatchings/code_2b78c", func_8003AF8C);

void func_8003B024(Class6E4F0 *self) {
}

INCLUDE_ASM("asm/nonmatchings/code_2b78c", func_8003B02C);
INCLUDE_ASM("asm/nonmatchings/code_2b78c", func_8003B044);

void func_8003B108(Class6E4F0 *self) {
}

INCLUDE_ASM("asm/nonmatchings/code_2b78c", func_8003B110);

Class6E4F0Methods *func_8003B20C(void) {
    return &D_8006E4F0;
}
