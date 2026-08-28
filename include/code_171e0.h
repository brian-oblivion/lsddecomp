#ifndef CODE_171E0_H
#define CODE_171E0_H

#include "common.h"

/* Method table (25 slots per tools/classtable.py) for the class whose
 * constructor caller is new_class_6d3c8 (src unit code_1677c). Not yet named
 * or typed field-by-field -- only its address is needed here, by
 * func_800269E0, which hands it to new_class_6d3c8 so the constructor slot
 * (+0x008, func_80025FDC) can be fetched and called indirectly. See
 * CLAUDE.md's "Writing a class method" for the +0x008 constructor-slot
 * convention. */
extern s32 D_8006D3C8[];

/* Some class instance (a slot of D_8006D430's table, going by
 * classtable.py) with at least one flag word at offset 0x24, OR'd with 1 by
 * func_80026C88. Layout before that offset is unknown. */
typedef struct UnkFlagsObj_171e0 {
    u8 pad0[0x24];
    s32 unknown_value_0x24;
} UnkFlagsObj_171e0;

/* Method table (header 0x00000003) for a second class. Several of this
 * unit's own functions are its slots: func_800269F0 (slot 0, offset +0x004),
 * func_80026A50 (slot 1 / +0x008, i.e. its constructor by the same
 * convention), func_80026AB4 (slot 2 / +0x00C), plus func_80026B08,
 * func_80026C20, func_80026C80 and func_80026C88 further down the table.
 * func_80026C9C returns its address the same way func_800269E0 returns
 * D_8006D3C8's. */
extern s32 D_8006D430[];

#endif
