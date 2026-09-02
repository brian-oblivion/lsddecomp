/* code_2cc8c -- first 20-function slice of the 0x2CC8C block (153 functions
 * total; the remainder is the code_2cc8c_b asm segment).
 *
 * Carved in round 10 because this block has the LOWEST toolchain-blocker
 * density of any uncarved segment: 1 of the 133 remaining functions, and
 * 2 of these 20.
 *
 * Those 2 are func_8003C48C and func_8003C63C, and they are BLOCKED by
 * `addiu_at` (docs/research/addiu-at-blocker.md). The carve comment here
 * originally claimed the opposite -- that their `%lo(jtbl_*)` hits were
 * ordinary switch jump tables and so not the indexed-global blocker. That
 * was wrong and was retracted the same round: cc1 emits the same generic
 * pseudo-op for a switch table as for an indexed global (`lw $2,$L13($2)`
 * vs `lbu $2,D_x($4)`), and maspsx folds both identically. Do not re-derive
 * this; see the note under "Open toolchain blockers" in CLAUDE.md.
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
