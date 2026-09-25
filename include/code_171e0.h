#ifndef CODE_171E0_H
#define CODE_171E0_H

#include "common.h"
#include "BasicClass.h"

/* Method table (25 slots per tools/classtable.py) for the class whose
 * constructor caller is New_Class6D3C8 (src unit code_1677c). Not yet named
 * or typed field-by-field -- only its address is needed here, by
 * GetClass6D3C8Methods, which hands it to New_Class6D3C8 so the constructor slot
 * (+0x008, Class6D3C8__Class6D3C8) can be fetched and called indirectly. See
 * CLAUDE.md's "Writing a class method" for the +0x008 constructor-slot
 * convention. */
extern s32 D_8006D3C8[];

/* Some class instance (a slot of D_8006D430's table, going by
 * classtable.py) with at least one flag word at offset 0x24, OR'd with 1 by
 * Class6D430__SetFlag. The full layout is derived further down this file, once
 * the method table type it needs (Class6D430Methods) is declared. */
typedef struct Class6D430 Class6D430;

/* Method table (header 0x00000003) for a second class. Several of this
 * unit's own functions are its slots: DestroyChained (slot 0, offset +0x004),
 * Class6D430__Class6D430 (slot 1 / +0x008, i.e. its constructor by the same
 * convention), Class6D430__Destroy (slot 2 / +0x00C), plus Class6D430__AllocBuffer,
 * Class6D430__FreeBuffer, NoOp and Class6D430__SetFlag further down the table.
 * GetClass6D430Methods returns its address the same way GetClass6D3C8Methods returns
 * D_8006D3C8's. */
extern s32 D_8006D430[];

/* A 3-word vector-like object, written wholesale by SetVec3. Only the
 * first three words are touched; nothing here says whether a further field
 * follows. */
typedef struct Vec3_171e0 {
    s32 x;
    s32 y;
    s32 z;
} Vec3_171e0;

extern void *BMemPMgrAlloc(s32 size); /* one arg confirmed by New_Class6D3C8.md (code_1677c) */
extern void BMemPMgrFree(void *arg);
extern s32 strlen(char *s);

/* D_8006D430's own vtable, laid out by the same convention BasicClass uses
 * (header, an own-class slot at +0x004, ctor at +0x008, dtor at +0x00C --
 * confirmed by dumping the table's raw words directly from disk/SLPS_015.56
 * rather than trusting classtable.py's null-slot elision). +0x010..+0x03C
 * are the 13 slots inherited verbatim from BasicClass (BasicClass__func_*),
 * +0x03C..+0x040 and +0x044..+0x054 (bar +0x058..+0x064) are genuinely null
 * at this level -- i.e. this class declares slots it never itself
 * implements, for a subclass to override; Class6D430__AllocBuffer calls four of them
 * (+0x044/+0x048/+0x04C/+0x054) polymorphically without knowing or caring
 * that they resolve to nothing at this level. Only the slots this unit's
 * queued functions actually dispatch through are typed; the rest are
 * skipped rather than padded out to +0x0B0, since nothing here ever
 * instantiates this struct by value. */
typedef struct Class6D430Methods Class6D430Methods;
struct Class6D430Methods {
    /* +0x00 */ s32 header;
    /* +0x04 */ void *(*ownDtorChain)(Class6D430 *self); /* DestroyChained */
    /* +0x08 */ void (*ctor)(Class6D430 *self);          /* Class6D430__Class6D430 */
    /* +0x0C */ void *(*dtor)(Class6D430 *self);         /* Class6D430__Destroy */
    /* +0x10 */ u8 pad10[0x44 - 0x10];      /* inherited BasicClass slots + null slots */
    /* +0x44 */ void (*open)(Class6D430 *self, s32 arg1, s32 arg2, s32 arg3);
    /* +0x48 */ void (*close)(Class6D430 *self);
    /* +0x4C */ s32 (*seek)(Class6D430 *self, s32 arg1, s32 arg2);
    /* +0x50 */ u8 pad50[0x54 - 0x50];      /* null slot */
    /* +0x54 */ void (*read)(Class6D430 *self, void *arg1, s32 arg2);
    /* +0x58 */ u8 pad58[0x5C - 0x58];      /* Class6D430__AllocBuffer's own slot, unused here */
    /* +0x5C */ void *(*freeBuffer)(Class6D430 *self);       /* Class6D430__FreeBuffer, dispatched
                                                                  * indirectly even though this
                                                                  * class's own copy is known */
};

/* An instance of the D_8006D430 class. Class6D430's own name and the
 * flags field (formerly unknown_value_0x24) predate this unit's work (see
 * Class6D430__SetFlag); fields below it are new, derived from Class6D430__Class6D430 (the ctor, which
 * zeroes them), Class6D430__AllocBuffer/Class6D430__FreeBuffer (isOpen/buffer/bufferSize/freeGuard) and
 * Class6D430__CopyFields (the unk40..unk74 block, a field-by-field copy -- retail
 * copies +0x40..+0x58 then jumps a 0xC-byte gap to +0x68..+0x74, so that gap
 * is left unnamed rather than guessed at). */
struct Class6D430 {
    /* +0x00 */ Class6D430Methods *methods;
    /* +0x04 */ u8 pad04[0x0C - 0x04];       /* BasicClass instance fields; owned elsewhere */
    /* +0x0C */ s32 isOpen;                   /* saved/restored around the buffer (re)alloc */
    /* +0x10 */ void *buffer;                 /* resource from BMemPMgrAlloc; freed via BMemPMgrFree */
    /* +0x14 */ s32 bufferSize;                   /* buffer's allocation size */
    /* +0x18 */ u8 pad18[0x20 - 0x18];       /* unknown, 8 bytes */
    /* +0x20 */ u16 freeGuard;                   /* nonzero blocks the buffer free in Class6D430__FreeBuffer */
    /* +0x22 */ u16 pendingRequests;
    /* +0x24 */ s32 flags;                   /* bit 0 set by Class6D430__SetFlag; only bit
                                                  established so far, meaning unknown */
    /* +0x28 */ u16 inQueueDispatch;
    /* +0x2A */ u16 unk2A;
    /* +0x2C */ u8 pad2C[0x40 - 0x2C];       /* unknown, 0x14 bytes */
    /* +0x40 */ s32 unk40;
    /* +0x44 */ s32 unk44;
    /* +0x48 */ s32 unk48;
    /* +0x4C */ s32 unk4C;
    /* +0x50 */ s32 unk50;
    /* +0x54 */ s32 unk54;
    /* +0x58 */ s32 unk58;
    /* +0x5C */ u8 pad5C[0x68 - 0x5C];       /* unknown, 0xC bytes; NOT copied by Class6D430__CopyFields */
    /* +0x68 */ s32 unk68;
    /* +0x6C */ s32 unk6C;
    /* +0x70 */ s32 unk70;
    /* +0x74 */ s32 unk74;
};

extern void *GetClass6D430Methods(void); /* returns &D_8006D430, see src/code_171e0.c */

char *strcat(char *dest, char *src);

#endif
