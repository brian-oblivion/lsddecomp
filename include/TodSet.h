#ifndef TODSET_H
#define TODSET_H

#include "Tod.h"

/*
 * TodSet -- a Tod subclass (class id 0x14F03, method table D_8006F590) over
 * a buffer holding several TOD animations: a counted offset table, one Tod
 * per entry, then the packet data. Methods in src/code_33808.c; no
 * subclasses. Its parent is its id parent: TodSet__TodSet's first call is
 * GetTodMethods()->ctor.
 *
 * The name is round 83's, kept on this evidence: TodSet__BuildTods (its
 * +0x064) makes one Tod per entry of the buffer's counted offset table
 * (New_Tod over buffer + entries[i]) and stores each back into the table's
 * own word; TodSet__Finalize releases that array (ReleaseBasicClassArray);
 * TodSet__ScanPackets (+0x078) runs Tod's +0x07C scanner over the data past
 * the counted array. ModelData builds one over its buffer past +0x0C
 * (ModelData__BuildResources: New_TodSet) and forwards its TOD packet scans
 * to it (ModelData.todSet, still declared `Class6D430 *` in
 * include/ModelData.h).
 *
 * NO OWN SLOTS: the table is Tod's 0x84 bytes, with +0x008, +0x00C, +0x064
 * and +0x078 overridden (`classtable.py D_8006F590 --vs D_8006F240`).
 * +0x064 keeps the inherited name `setFlag` and its void type; the occupant
 * TodSet__BuildTods returns s32 (0 when every Tod was built), and the ctor
 * casts the call, as ModelData__ModelData casts its own. +0x078 is
 * Class6D430's slot78 (NULL there); TodSet__ScanPackets occupies it, as
 * Tod__ScanPackets does in Tod's table.
 *
 * NO OWN FIELDS: the object is 0x2C bytes (New_TodSet), Tod's size.
 *
 * The ctor returns self or NULL (New_TodSet tests it), but TOD_SLOTS
 * declares +0x008 returning void, as Tod's own ctor does; the allocator
 * reaches it through code_33808.c's unprototyped Ctor33808 view, as every
 * allocator in that unit does. The descriptor is code_33808.c's Src6F240;
 * only the tag is declared here, as include/Tod.h does.
 */

struct Src6F240;

typedef struct TodSet TodSet;
typedef struct TodSetMethods TodSetMethods;

struct TodSetMethods {
    TOD_SLOTS(TodSet, (TodSet *self, struct Src6F240 *src));
};

struct TodSet {
    TOD_FIELDS(TodSetMethods);
};

extern TodSetMethods D_8006F590;
extern TodSetMethods *GetTodSetMethods(void);

TodSet *New_TodSet(struct Src6F240 *src);
void *TodSet__TodSet(TodSet *self, struct Src6F240 *src);
void TodSet__Finalize(TodSet *self);
s32 TodSet__BuildTods(TodSet *self);
u8 TodSet__ScanPackets(TodSet *self, u8 *out, u32 *sel);

#endif
