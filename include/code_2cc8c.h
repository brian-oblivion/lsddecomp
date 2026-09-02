#ifndef CODE_2CC8C_H
#define CODE_2CC8C_H

#include "common.h"

/*
 * The class whose method table is D_80086B60 (78 slots, base) with a
 * derived override table at D_80087AAC (73 slots) -- resolved with
 * tools/classtable.py D_80086B60 / D_80086B60 --vs D_80087AAC. No FirecatFG
 * name survives, so the struct is named `Obj86B60` after the base table's
 * address and fields are named by offset until real names are known. Per
 * this project's established multiple-independent-local-views convention
 * (see e.g. class_3bb8c.h's own header comment), this is THIS unit's own
 * view -- no other unit includes this header.
 *
 * Both tables agree on every slot this unit's 18 attempted functions
 * touch, so the base/derived split does not matter for this file; only
 * slots 0x090/0x094 (unused here) and a few 0x0D8+ slots differ between
 * them. See src/code_2cc8c.c's own header comment for the carve
 * provenance and the two functions (func_8003C48C, func_8003C63C) this
 * unit could not attempt (addiu_at-blocked dense switches).
 *
 * Only the slots/fields this unit's functions actually touch are given
 * concrete types; everything else stays opaque padding.
 */
typedef struct Obj86B60 Obj86B60;
typedef struct Obj86B60Methods Obj86B60Methods;
typedef struct Unk4CObj Unk4CObj;
typedef struct Unk48Obj Unk48Obj;
typedef struct Unk48ObjMethods Unk48ObjMethods;
typedef struct Unk78Obj Unk78Obj;
typedef struct Unk78ObjMethods Unk78ObjMethods;

/*
 * self->unk4C's pointee ("target"). Established from six independent
 * functions all agreeing:
 *  - func_8003C7F4/func_8003C858/func_8003C8D0/func_8003C944/func_8003C9B0
 *    each use it ONLY as a null/non-null gate (never dereferenced), so it
 *    is at minimum a pointer.
 *  - func_8003CA1C dereferences +0x00C (s32, compared directly against
 *    self->unk58) and +0x024 (a pointer to a word-pointer array, indexed
 *    by self->unk58 and null-checked -- `target->unk24[self->unk58]`).
 *  - func_8003C63C (case a1==5, STALL -- see that function's report for
 *    why it was never attempted as a whole; this field use was read off
 *    the disassembly independently of attempting the function) reads
 *    +0x008 (s32, forwarded as an argument) and takes the ADDRESS of
 *    +0x010 (passed as a 3-byte colour-ish buffer to self->methods->slotE4,
 *    the same slot func_8003CC2C feeds a locally-built 3-byte buffer to).
 */
struct Unk4CObj {
    u8 pad000[0x008];
    s32 unk8;           /* +0x008, OBSERVED: func_8003C63C (not attempted) */
    s32 unkC;            /* +0x00C, OBSERVED: func_8003CA1C */
    u8 unk10[3];          /* +0x010, INFERRED 3-byte colour buffer read by
                              address only (func_8003C63C, not attempted) */
    u8 pad13[0x024 - 0x013];
    void **unk24;         /* +0x024, OBSERVED: func_8003CA1C, word-pointer
                              array indexed by self->unk58 */
};

/* self->unk48's pointee ("child"). A DIFFERENT class from Obj86B60 -- its
 * own vtable slot +0x080 takes FOUR args (self, a1, a2, a3), whereas
 * Obj86B60's OWN slot +0x080 (func_8003C944) takes none beyond self. Only
 * func_8003C7B4 touches it, so only that one slot is modelled. */
struct Unk48ObjMethods {
    u8 pad000[0x080];
    void (*slot80)(Unk48Obj *self, s32 a1, s32 a2, s32 a3); /* +0x080 */
};
struct Unk48Obj {
    Unk48ObjMethods *methods; /* +0x000 */
};

/* self->unk78's pointee. Only func_8003CC2C touches it, calling one slot
 * with a literal 1 and the same 3-byte colour buffer func_8003CC2C builds
 * for its own self->methods->slotE4 call just above. */
struct Unk78ObjMethods {
    u8 pad000[0x0B8];
    void (*slotB8)(Unk78Obj *self, s32 a1, u8 *buf); /* +0x0B8 */
};
struct Unk78Obj {
    Unk78ObjMethods *methods; /* +0x000 */
};

/*
 * self->methods. Only the slots this unit's functions actually CALL
 * THROUGH (as opposed to slots that simply ARE these functions, entered
 * from elsewhere) are given here -- there is no need to model a slot this
 * unit never itself invokes.
 *
 * slotB0/slotC4 are read as DATA (a raw function-pointer VALUE stashed into
 * self->unk88/self->unk8C by func_8003CAF8/func_8003CB30), never called
 * directly through the vtable in this unit -- their type is inferred from
 * how self->unk88/unk8C are later CALLED (func_8003CBC0/func_8003CCDC).
 *
 * slotAC/slotC0 ARE func_8003CBC0/func_8003CCDC respectively (OBSERVED from
 * classtable.py); func_8003C51C calls them with a caller-forwarded `s32 a1`
 * that their own bodies never read (see those two functions' own reports
 * for the "unused parameter in the callee" idiom this produces).
 */
struct Obj86B60Methods {
    u8 pad000[0x060];
    void (*slot60)(Obj86B60 *self, s32 reason);  /* +0x060, external
                                                      (func_8004D90C) */
    u8 pad064[0x070 - 0x064];
    void (*slot70)(Obj86B60 *self, s32 a1);       /* +0x070, IS
                                                      func_8003C7B4 */
    u8 pad074[0x090 - 0x074];
    void (*slot90)(Obj86B60 *self);               /* +0x090, external
                                                      (func_8004D9D4);
                                                      OBSERVED: func_8003C63C
                                                      (STALL, not attempted --
                                                      read off the
                                                      disassembly only) */
    void (*slot94)(Obj86B60 *self);                /* +0x094, external
                                                       (func_8004DABC);
                                                       OBSERVED:
                                                       func_8003CA1C and
                                                       func_8003C63C (STALL) */
    u8 pad098[0x0AC - 0x098];
    s32 (*slotAC)(Obj86B60 *self, s32 a1);         /* +0x0AC, OBSERVED:
                                                       IS func_8003CBC0 */
    s32 (*slotB0)(Obj86B60 *self);                 /* +0x0B0, IS
                                                       func_8003CC2C; read as
                                                       DATA by func_8003CAF8 */
    u8 pad0B4[0x0C0 - 0xB4];
    s32 (*slotC0)(Obj86B60 *self, s32 a1);         /* +0x0C0, IS
                                                       func_8003CCDC */
    s32 (*slotC4)(Obj86B60 *self);                  /* +0x0C4, external
                                                       (func_8003CD48); read
                                                       as DATA by
                                                       func_8003CB30 */
    u8 pad0C8[0x0E4 - 0xC8];
    void (*slotE4)(Obj86B60 *self, u8 *buf);        /* +0x0E4, external
                                                       (func_8003D9D4);
                                                       OBSERVED:
                                                       func_8003CC2C and
                                                       func_8003C63C (STALL,
                                                       not attempted) */
    void (*slotE8)(Obj86B60 *self);                 /* +0x0E8, external
                                                       (func_8003D3B0);
                                                       OBSERVED: func_8003C9B0 */
    void (*slotEC)(Obj86B60 *self);                 /* +0x0EC, external
                                                       (func_8003D444);
                                                       OBSERVED: func_8003C944 */
    void (*slotF0)(Obj86B60 *self, s32 a1, s32 a2); /* +0x0F0, external
                                                       (func_8004DABC);
                                                       OBSERVED:
                                                       func_8003C63C (STALL,
                                                       not attempted) */
    u8 pad0F4[0x108 - 0xF4];
    void (*slot108)(Obj86B60 *self);                /* +0x108, external
                                                       (func_8003DA10);
                                                       OBSERVED: func_8003CA1C */
    s32 (*slot10C)(Obj86B60 *self);                 /* +0x10C, external
                                                       (func_8003DAD4);
                                                       OBSERVED:
                                                       func_8003C63C (STALL,
                                                       not attempted) */
    s32 (*slot110)(Obj86B60 *self);                 /* +0x110, external
                                                       (func_8003DCAC);
                                                       OBSERVED:
                                                       func_8003C63C (STALL,
                                                       not attempted) */
    void (*slot114)(Obj86B60 *self);                /* +0x114, external
                                                       (func_8003DDC8);
                                                       OBSERVED: func_8003C9B0 */
    s32 (*slot118)(Obj86B60 *self);                 /* +0x118, external
                                                       (func_8003DFA0);
                                                       OBSERVED: func_8003C944 */
};

/*
 * The object itself. Every field below is OBSERVED (not inferred) from at
 * least one of this unit's 18 attempted functions -- see each function's
 * own match report for the specific derivation. Gaps are left as opaque
 * padding; nothing here claims knowledge of bytes no function touched.
 */
struct Obj86B60 {
    Obj86B60Methods *methods;   /* +0x000 */
    u8 pad004[0x01C - 0x004];
    s32 unk1C;                  /* +0x01C, func_8003CC2C (a running count/
                                    frame value multiplied against unk84);
                                    func_8003C63C (STALL) zeroes it on
                                    several message codes */
    u8 pad020[0x020 - 0x020];
    s32 unk20;                  /* +0x020, func_8003C63C (STALL) sets it
                                    to a literal 5 */
    u8 pad024[0x038 - 0x024];
    s32 unk38;                  /* +0x038, func_8003C63C (STALL) sets it
                                    to 1 */
    s32 unk3C;                  /* +0x03C, a mode/state value: func_8003C858
                                    compares ==1, func_8003C8D0 !=1,
                                    func_8003C944/func_8003C9B0 ==1/==2,
                                    func_8003C48C (STALL) gates on !=0,
                                    func_8003C63C (STALL) sets 0/1 */
    s32 unk40;                  /* +0x040, func_8003C794 (setter: raw value
                                    if negative, value*20 if >= 0);
                                    func_8003C51C compared against unk1C */
    u8 pad044[0x048 - 0x044];
    Unk48Obj *unk48;            /* +0x048, func_8003C7B4 only */
    Unk4CObj *unk4C;            /* +0x04C, see Unk4CObj's own comment */
    u8 pad050[0x058 - 0x050];
    s32 unk58;                  /* +0x058, func_8003CA1C: index into
                                    unk4C->unk24[] and compared against
                                    unk4C->unkC */
    u8 pad05C[0x078 - 0x05C];
    Unk78Obj *unk78;             /* +0x078, func_8003CC2C only */
    u8 pad07C[0x084 - 0x07C];
    s32 unk84;                  /* +0x084, func_8003CC2C: multiplied
                                    against unk1C */
    s32 (*unk88)(Obj86B60 *self); /* +0x088, a callback: set (to NULL or
                                    self->methods->slotB0) by func_8003CAF8,
                                    invoked by func_8003CBC0 */
    s32 (*unk8C)(Obj86B60 *self); /* +0x08C, same idiom via slotC4/
                                    func_8003CB30/func_8003CCDC */
    u8 unk90[3];                 /* +0x090, func_8003CB68 (setter, from
                                    a1[0..2]); func_8003CC2C reads it as a
                                    colour base */
    u8 unk93[3];                 /* +0x093, func_8003CB68 (setter, from
                                    a2[0..2]) */
    u8 unk96[3];                 /* +0x096, func_8003CB68 (setter, from
                                    a3[0..2]) */
    u8 unk99[0x09C - 0x099];
    void (*unk9C)(void *ctx);   /* +0x09C, a callback: set by func_8003CAEC,
                                    invoked (with unkA0 as its argument) by
                                    func_8003CA94 */
    void *unkA0;                 /* +0x0A0, set by func_8003CAEC, passed to
                                    unk9C by func_8003CA94 */
};

/*
 * Shared "IntermediateBase" utility class, reached only through
 * func_8003E5C8() (still raw asm elsewhere in the still-uncarved
 * code_2cc8c_b portion of this segment -- not this unit's function to
 * write). Same idiom already established in src/code_2c054.c
 * (TaskUtilMethods) and src/class_39e08.c (IntermediateBaseMethods): each
 * unit that reaches it keeps its own local view, self typed `void *`
 * since it is shared across unrelated classes. Only the two slots this
 * unit's func_8003C51C (and func_8003C63C, STALL) actually reach are
 * modelled.
 */
typedef struct IntermediateBaseMethods IntermediateBaseMethods;
struct IntermediateBaseMethods {
    u8 pad000[0x05C];
    void (*slot5C)(void *self, s32 a1, s32 a2);  /* +0x05C */
    void (*slot60)(void *self, s32 a1);           /* +0x060 */
};

extern IntermediateBaseMethods *func_8003E5C8(void); /* returns &D_8006E878,
                                                          same static table
                                                          as code_2c054.h's
                                                          and class_39e08.h's
                                                          own views */

#endif
