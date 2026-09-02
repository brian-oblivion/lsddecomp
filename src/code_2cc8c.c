/* code_2cc8c -- first 20-function slice of the 0x2CC8C block (153 functions
 * total; the remainder is the code_2cc8c_b asm segment).
 *
 * Carved in round 10 on the belief that this block had the LOWEST
 * toolchain-blocker density of any uncarved segment. That belief was
 * WRONG for two of the 20: `func_8003C48C` and `func_8003C63C` both hit
 * the `addiu $at, $at, %lo` screening grep on their jump-table dispatch
 * (`%lo(jtbl_*)`), and a `jtbl_*` symbol there is NOT a safe exception --
 * measured through the pinned pipeline, maspsx folds a jump-table address
 * resolution exactly like an indexed data-global one, with no way to tell
 * them apart below cc1. Both are genuinely `addiu_at`-blocked; see
 * docs/research/addiu-at-blocker.md and each function's own
 * docs/match-reports/*.md for the reproducer detail. Of the OTHER 18, none
 * hit either open blocker.
 *
 * Shape: this is class-framework code. Objects carry their method table at
 * offset 0 (`lw $v1, 0x0($a0)` then `lw $v0, 0xNN($v1)` then `jalr`), so
 * resolve slots with tools/classtable.py rather than by counting. The
 * class is `Obj86B60` (include/code_2cc8c.h), named after its base method
 * table D_80086B60 (78 slots; a derived override table also exists at
 * D_80087AAC, 73 slots -- see the header's own comment). The first two
 * functions are switch dispatchers over a small event/message code -- both
 * blocked, see above.
 */

#include "common.h"
#include "code_2cc8c.h"

INCLUDE_ASM("asm/nonmatchings/code_2cc8c", func_8003C48C);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c", func_8003C51C);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c", func_8003C63C);

void func_8003C794(Obj86B60 *self, s32 a1)
{
    self->unk40 = a1;
    if (a1 >= 0) {
        self->unk40 = a1 * 20;
    }
}

INCLUDE_ASM("asm/nonmatchings/code_2cc8c", func_8003C7B4);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c", func_8003C7F4);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c", func_8003C858);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c", func_8003C8D0);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c", func_8003C944);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c", func_8003C9B0);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c", func_8003CA1C);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c", func_8003CA94);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c", func_8003CAEC);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c", func_8003CAF8);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c", func_8003CB30);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c", func_8003CB68);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c", func_8003CBB8);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c", func_8003CBC0);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c", func_8003CC2C);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c", func_8003CCDC);
