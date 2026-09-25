/*
 * code_2a0e0 -- GAME code carved from psyq_2a0e0 on 2026-09-25 (FINISHING-PLAN
 * revision 18). 0x2A0E0..0x2A878 (vram 0x800398E0..0x8003A078). It was counted
 * as Psy-Q SDK by segment name; tools/gameinsdk.py measured it as game (a call
 * into game code, a method-table entry beside game methods, or contiguity with
 * those, and no Sony fingerprint). What it holds: 12 methods of D_8006E48C,
 * which call New_VabStreamObj; the yaml had called this gap "the game's own
 * libspu build".
 *
 * Nothing here is matched yet: every function is fresh track-1 ground.
 */
#include "common.h"
#include "BasicClass.h"

/* Local view of D_8006E48C's objects: a SEQ player. Fields named from the
 * libsnd calls they feed. */
typedef struct SeqObj SeqObj;
typedef struct SeqObjMethods SeqObjMethods;

struct SeqObjMethods {
    BASICCLASS_SLOTS(SeqObj, (SeqObj *self));
    /* +0x040 */ void (*update)(SeqObj *self, s32 arg1, s32 arg2); /* func_80039B90 */
    /* +0x044 */ void (*play)(SeqObj *self);                       /* func_80039CBC */
};

struct SeqObj {
    BASICCLASS_FIELDS(SeqObjMethods);
    /* +0x00C */ void *unkC;
    /* +0x010 */ void *unk10;
    /* +0x014 */ s16 seqId;
    /* +0x016 */ u8 pad16[0x1A - 0x16];
    /* +0x01A */ u16 state;
    /* +0x01C */ u16 paused;
    /* +0x01E */ u16 playing;
    /* +0x020 */ s32 unk20;
};

/* libsnd (LIBSND.H) */
extern void SsSeqPlay(short, char, short);
extern void SsSeqPause(short);
extern void SsSeqReplay(short);
extern void SsSeqStop(short);
extern void SsSeqSetVol(short, short, short);
extern void SsSeqSetCrescendo(short, short, long);
extern void SsSeqClose(short);

extern s32 func_8002CC28(void);
s32 func_80039C04(SeqObj *self);

extern SeqObjMethods D_8006E48C;
extern s32 D_8008A8D8;
extern u8 D_8008DF38[];

INCLUDE_ASM("asm/nonmatchings/code_2a0e0", func_800398E0);
INCLUDE_ASM("asm/nonmatchings/code_2a0e0", func_8003995C);
INCLUDE_ASM("asm/nonmatchings/code_2a0e0", func_80039A34);
INCLUDE_ASM("asm/nonmatchings/code_2a0e0", func_80039B04);
void func_80039B90(SeqObj *self, s32 arg1, s32 arg2) {
    if (arg2 == 2 && self->state == 1 && func_80039C04(self) && self->unk20 != 0) {
        self->methods->play(self);
    }
}
INCLUDE_ASM("asm/nonmatchings/code_2a0e0", func_80039C04);
void func_80039CBC(SeqObj *self) {
    if (self->playing == 0) {
        SsSeqSetVol(self->seqId, 0x34, 0x34);
        SsSeqPlay(self->seqId, 1, 0);
        self->playing = 1;
    }
}
void func_80039D14(SeqObj *self) {
    if (self->playing != 0) {
        SsSeqStop(self->seqId);
        SsSeqClose(self->seqId);
        self->playing = 0;
        self->state = 0;
    }
}
void func_80039D68(SeqObj *self) {
    if (self->paused == 0) {
        SsSeqPause(self->seqId);
        self->paused = 1;
    }
}
void func_80039DB0(SeqObj *self) {
    if (self->paused != 0) {
        SsSeqReplay(self->seqId);
        self->paused = 0;
    }
}
void func_80039DF4(SeqObj *self, s16 left, s16 right) {
    SsSeqSetVol(self->seqId, left, right);
}
void func_80039E24(SeqObj *self, s16 vol, s32 scale) {
    SsSeqSetCrescendo(self->seqId, vol, func_8002CC28() * scale);
}
INCLUDE_ASM("asm/nonmatchings/code_2a0e0", func_80039E7C);
INCLUDE_ASM("asm/nonmatchings/code_2a0e0", func_80039F64);
SeqObjMethods *func_8003A04C(void) {
    return &D_8006E48C;
}
s32 func_8003A05C(void) {
    return D_8008A8D8;
}
void *func_8003A068(void) {
    return &D_8008DF38;
}
