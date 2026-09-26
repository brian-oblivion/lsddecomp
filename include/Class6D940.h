#ifndef CLASS6D940_H
#define CLASS6D940_H

#include "Class6D430.h"

/*
 * Class6D940 -- a Class6D430 data source (class id 0xE03, method table
 * D_8006D940) over a 20x20 grid of placement records. Methods in
 * src/code_179d8_d.c. No classes derive from it. Its getter is the first
 * entry of gDataSourceClientGetters, so SetActiveDataSource rebinds its
 * interface slots like every other client's.
 *
 * PARENT BY CTOR CHAIN: Class6D940__Class6D940's first call is
 * GetActiveDataSourceMethods()->ctor, and Class6D940__Finalize and
 * Class6D940__SetFlag forward to the active driver's finalize and setFlag,
 * as TimBlockSrc, Tod and ModelData do.
 *
 * Its one user is the grid manager (Class866E8): Class866E8__Class866E8
 * makes one per grid element with New_Class6D940(0), so nothing is
 * loaded; Class866E8__LoadElementResources points `buffer` into an already
 * loaded resource, puts a LinkResource (gLinkResourceMethods) of the element's models
 * in `linkResource`, then calls +0x078 once per cell until it returns 0.
 *
 * +0x078 is Class6D430's slot78 (NULL there): this table's occupant is
 * Class6D940__ResolveEntry(self, placement, cell), s32. It reads cell
 * `cell`'s Class6D940Record (12 bytes each, from buffer +8; cell / 20 is
 * the row, cell % 20 the column), follows `next` for a second record in
 * the same cell, fills a Class6D940Placement and returns the model
 * linkResource's +0x080 (LinkResource__GetModel) gives for the record's
 * model index: 0 past the last cell (400), -1 for an empty record.
 * Class866E8__LoadElementResources calls it through
 * Class6D940ResolveEntryFn, a cast of the inherited slot (no code).
 */

typedef struct Class6D940 Class6D940;
typedef struct Class6D940Methods Class6D940Methods;

/* One record in the buffer, 12 bytes: Class6D940__ResolveEntry steps
 * cell * 12 + 8. */
typedef struct Class6D940Record {
    /* +0x0 */ u8 present;   /* zero: the cell is empty, ResolveEntry returns -1 */
    /* +0x1 */ u8 unk1;      /* -> Class6D940Placement.unk2C */
    /* +0x2 */ u16 model;    /* the index passed to linkResource's +0x080 */
    /* +0x4 */ u8 unk4;      /* -> Class6D940Placement.unk2E */
    /* +0x5 */ u8 rotY;      /* in 0x400 steps: -> Class6D940Placement.rotY */
    /* +0x6 */ s16 y;        /* in 0x800 units: -> Class6D940Placement.y */
    /* +0x8 */ s32 next;     /* buffer offset of the cell's next record, 0 for none */
} Class6D940Record;

/* What Class6D940__ResolveEntry fills in: the caller's stack record, 0x40
 * bytes (Class866E8__LoadElementResources). x and z are the cell's centre,
 * 0x800 units a cell. */
typedef struct Class6D940Placement {
    /* +0x000 */ u8 pad0[0xC];
    /* +0x00C */ s32 x;          /* column * 0x800 + 0x400 */
    /* +0x010 */ s32 y;          /* record y * 0x800 */
    /* +0x014 */ s32 z;          /* row * 0x800 + 0x400 */
    /* +0x018 */ u8 pad18[0x1A - 0x18];
    /* +0x01A */ u16 rotY;       /* record rotY * 0x400 */
    /* +0x01C */ u8 pad1C[0x2C - 0x1C];
    /* +0x02C */ u16 unk2C;      /* record unk1 */
    /* +0x02E */ u16 unk2E;      /* record unk4 */
    /* +0x030 */ s32 chained;    /* 1: this record came from the previous one's `next` */
    /* +0x034 */ s32 next;       /* in: the offset to follow (0 starts at the cell); out: the record's `next` */
    /* +0x038 */ s32 model;      /* record model */
    /* +0x03C */ u8 pad3C[4];
} Class6D940Placement;

typedef s32 (*Class6D940ResolveEntryFn)(Class6D940 *self, Class6D940Placement *placement, s32 cell);

struct Class6D940Methods {
    CLASS6D430_SLOTS(Class6D940, (Class6D940 *self, char *name));
    /* +0x078 is Class6D430's slot78; this table's occupant is
     * Class6D940__ResolveEntry (Class6D940ResolveEntryFn). */
};

struct Class6D940 {
    CLASS6D430_FIELDS(Class6D940Methods);
    /* +0x02C */ Class6D430 *linkResource; /* the models' LinkResource (gLinkResourceMethods); zeroed by the ctor */
    /* +0x030 */ s32 loaded;               /* set by Class6D940__SetFlag; zeroed by the ctor */
};                                         /* 0x34 bytes: New_Class6D940 */

extern Class6D940Methods D_8006D940;
extern Class6D940Methods *GetClass6D940Methods(void);

Class6D940 *New_Class6D940(char *name);
void Class6D940__Class6D940(Class6D940 *self, char *name);
void Class6D940__Finalize(Class6D940 *self);
void Class6D940__SetFlag(Class6D940 *self);
s32 Class6D940__ResolveEntry(Class6D940 *self, Class6D940Placement *placement, s32 cell);

#endif
