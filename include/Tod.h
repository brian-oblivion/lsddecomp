#ifndef TOD_H
#define TOD_H

#include "Class6D430.h"

/*
 * Tod -- a Class6D430 data source (class id 0x4F03, method table D_8006F240)
 * over one TOD animation's packet stream. Methods in src/code_33808.c; one
 * subclass, TodSet (D_8006F590, 0x14F03), whose ctor calls this class's
 * first (TodSet__TodSet: GetTodMethods()->ctor(self, arg)) and whose
 * TodSet__BuildTods makes one Tod per sub-block of its buffer (New_Tod).
 *
 * The name is round 83's, kept on this evidence: its own methods walk
 * the buffer's packet words (ScanTodPackets, from buffer +8 with the u16
 * packet count at +2 before it) decoding each into a low byte, the nibbles
 * at bits 16 and 20 and a top-byte length in words (DecodeTodPacketWord);
 * ModelData forwards its TOD packet scans to the TodSet it holds
 * (ModelData__ForwardScanPackets: todSet's +0x078), and Class65650 plays
 * TODs through that ModelData.
 *
 * PARENT BY CTOR CHAIN, NOT BY ID. The id 0x4F03 puts it under TimBlockSrc
 * (0xF03), but Tod__Tod's first call is GetActiveDataSourceMethods()->ctor,
 * as TimBlockSrc__TimBlockSrc's is: it is TimBlockSrc's sibling under
 * Class6D430 and carries none of TimBlockSrc's layout (include/TimBlockSrc.h).
 *
 * NO OWN FIELDS. The object is 0x2C bytes (New_Tod), Class6D430's own size;
 * TodSet's is 0x2C too (New_TodSet). Everything a Tod reads is in the
 * adopted or loaded `buffer`.
 *
 * +0x078 is Class6D430's slot78 (NULL there): this table's occupant is
 * Tod__ScanPackets(self, out, sel), u8, which runs +0x07C over the buffer
 * past its first two words (TodSet's occupant runs it past its counted
 * array); ModelData__ForwardScanPackets casts it (an inherited slot keeps
 * the parent's name).
 *
 * The ctor's descriptor is code_33808.c's Src6F240 ({buffer to adopt, file
 * name to request}); only the tag is declared here, as include/ModelData.h
 * does. The allocators reach the ctor through code_33808.c's unprototyped
 * Ctor33808 view.
 */

struct Src6F240;

typedef struct Tod Tod;
typedef struct TodMethods TodMethods;

/* Both own slots are typed as their occupants. +0x07C returns u8: cc1 still
 * emits Tod__ScanPackets' trailing `andi 0xFF` over a u8 slot, so the bytes
 * do not need the s32 the unit-local view had (round 86). */
#define TOD_SLOTS(Self, CtorParams)                                                                \
    CLASS6D430_SLOTS(Self, CtorParams);                                                            \
    /* +0x07C */ u8 (*scanTodPackets)(Self *self, u8 *out, u32 *sel, u32 *data); /* ScanTodPackets, in both tables */ \
    /* +0x080 */ u32 *(*decodePacketWord)(Self *self, u32 *acc, u8 *out0, u8 *out1, u8 *out2, u8 *out3) /* DecodeTodPacketWord, in both tables */

#define TOD_FIELDS(Methods)                                                                        \
    CLASS6D430_FIELDS(Methods) /* no own fields: the object is 0x2C bytes (New_Tod), and so is TodSet's (New_TodSet) */

struct TodMethods {
    TOD_SLOTS(Tod, (Tod *self, struct Src6F240 *src));
};

struct Tod {
    TOD_FIELDS(TodMethods);
};

extern TodMethods D_8006F240;
extern TodMethods *GetTodMethods(void);

Tod *New_Tod(struct Src6F240 *src);
void Tod__Tod(Tod *self, struct Src6F240 *src);
void Tod__Finalize(Tod *self);
u8 Tod__ScanPackets(Tod *self, u8 *out, u32 *sel);
u8 ScanTodPackets(Tod *self, u8 *out, u32 *sel, u32 *data);
u32 *DecodeTodPacketWord(Tod *self, u32 *acc, u8 *out0, u8 *out1, u8 *out2, u8 *out3);

#endif
