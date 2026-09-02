/* code_2cc8c -- first 20-function slice of the 0x2CC8C block (153 functions
 * total; the remainder is the code_2cc8c_b asm segment).
 *
 * Carved in round 10 because this block has the LOWEST toolchain-blocker
 * density of any uncarved segment: 1 of the 133 remaining functions, and
 * 0 of these 20. (func_8003C48C and func_8003C63C match the `addiu $at`
 * screening grep, but only via `%lo(jtbl_*)` -- an ordinary switch jump
 * table, not the indexed-global blocker. See docs/research/addiu-at-blocker.md.)
 *
 * Shape: this is class-framework code. Objects carry their method table at
 * offset 0 (`lw $v1, 0x0($a0)` then `lw $v0, 0xNN($v1)` then `jalr`), so
 * resolve slots with tools/classtable.py rather than by counting. The first
 * two functions are switch dispatchers over a small event/message code.
 *
 * No header of its own yet -- add include/code_2cc8c.h when a struct emerges.
 */

#include "common.h"

INCLUDE_ASM("asm/nonmatchings/code_2cc8c", func_8003C48C);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c", func_8003C51C);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c", func_8003C63C);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c", func_8003C794);

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
