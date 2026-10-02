#ifndef GTE_H
#define GTE_H

/**
 * @file gte.h
 * @brief The Psy-Q `gte_*` macros the game calls, for the PlayStation's
 *        geometry coprocessor (the GTE, coprocessor 2).
 *
 * The GTE's registers are coprocessor registers: 32 data registers and 32
 * control registers, numbered apart from the general registers, and the
 * instructions that move them (lwc2/swc2, mfc2/mtc2, cfc2/ctc2) and run its
 * operations have no C spelling. Each macro is therefore an `__asm__
 * volatile` block, and the only way it can name a C value is through an
 * operand constraint: every pointer a macro reads or writes through is an
 * `"r"` input. A macro that stores to memory lists `"memory"` as a clobber;
 * one that moves data through general registers lists those registers too.
 * No macro names a general register for any other purpose.
 *
 * A `$n` inside a block is a COP2 register: data 0 VXY0, 1 VZ0, 2 VXY1,
 * 3 VZ1, 4 VXY2, 5 VZ2, 6 RGB, 7 OTZ, 8 IR0, 9-11 IR1-IR3, 12-14 SXY0-SXY2,
 * 16-19 SZ0-SZ3, 20-22 RGB0-RGB2, 24 MAC0; control 0-4 the rotation matrix,
 * 5-7 the translation vector, 31 FLAG. include/psyq/gtenom.h names the same
 * assignments.
 *
 * Names follow Sony's include/psyq/inline.h, so the SDK manual describes
 * each one. Code around the macros is ordinary C; a GTE operation this file
 * lacks is added here under Sony's name, not written out at a call site.
 *
 * The host build (HOST_BUILD) has no COP2: it takes the same macros from
 * psyz's <libgte.h>, which runs them on psyz's GTE emulation, and this file
 * defines none of its own.
 */

#ifdef HOST_BUILD
#include <libgte.h>
#else

/**
 * @brief Loads vertex 0: the SVECTOR at `r1` into VXY0/VZ0 (data 0, 1).
 * Operand: `r1` by "r"; no clobbers (a load).
 */
/* clang-format off */
#define gte_ldv0(r1) \
    __asm__ volatile ( \
        "lwc2 $0, 0x0(%0)\n\t" \
        "lwc2 $1, 0x4(%0)" \
        : : "r" (r1))
/* clang-format on */

/**
 * @brief Loads vertices 0, 1 and 2: the SVECTORs at `r1`, `r2` and `r3`
 * into VXY0/VZ0, VXY1/VZ1 and VXY2/VZ2 (data 0 to 5), the input of
 * gte_rtpt(). Operands: three pointers by "r"; no clobbers.
 */
/* clang-format off */
#define gte_ldv3(r1, r2, r3) \
    __asm__ volatile ( \
        "lwc2 $0, 0x0(%0)\n\t" \
        "lwc2 $1, 0x4(%0)\n\t" \
        "lwc2 $2, 0x0(%1)\n\t" \
        "lwc2 $3, 0x4(%1)\n\t" \
        "lwc2 $4, 0x0(%2)\n\t" \
        "lwc2 $5, 0x4(%2)" \
        : : "r" (r1), "r" (r2), "r" (r3))
/* clang-format on */

/**
 * @brief Stores the screen-XY FIFO (SXY0-SXY2, data 12-14) into a POLY_F3's
 * vertices, x0y0 +0x08, x1y1 +0x0C, x2y2 +0x10.
 *
 * The four `gte_stsxy3_<prim>` macros take one primitive base and use its
 * Psy-Q layout's vertex offsets; gte_stsxy3() takes three separate
 * destinations. Operand: `r1` by "r"; clobbers memory, as every store here
 * does.
 */
/* clang-format off */
#define gte_stsxy3_f3(r1) \
    __asm__ volatile ( \
        "swc2 $12, 0x8(%0)\n\t" \
        "swc2 $13, 0xc(%0)\n\t" \
        "swc2 $14, 0x10(%0)" \
        : : "r" (r1) : "memory")
/* clang-format on */

/**
 * @brief Stores SXY0-SXY2 into a POLY_G3's vertices, +0x08, +0x10, +0x18
 * (each vertex follows its RGB). Operand: `r1` by "r"; clobbers memory.
 */
/* clang-format off */
#define gte_stsxy3_g3(r1) \
    __asm__ volatile ( \
        "swc2 $12, 0x8(%0)\n\t" \
        "swc2 $13, 0x10(%0)\n\t" \
        "swc2 $14, 0x18(%0)" \
        : : "r" (r1) : "memory")
/* clang-format on */

/**
 * @brief Stores SXY0-SXY2 into a POLY_FT3's vertices, +0x08, +0x10, +0x18:
 * the offsets of gte_stsxy3_g3(), because a POLY_FT3 vertex's UV is 4 bytes
 * wide like a POLY_G3 vertex's RGB. Operand: `r1` by "r"; clobbers memory.
 */
/* clang-format off */
#define gte_stsxy3_ft3(r1) \
    __asm__ volatile ( \
        "swc2 $12, 0x8(%0)\n\t" \
        "swc2 $13, 0x10(%0)\n\t" \
        "swc2 $14, 0x18(%0)" \
        : : "r" (r1) : "memory")
/* clang-format on */

/**
 * @brief Stores SXY0-SXY2 into a POLY_GT3's vertices, +0x08, +0x14, +0x20.
 * Operand: `r1` by "r"; clobbers memory.
 */
/* clang-format off */
#define gte_stsxy3_gt3(r1) \
    __asm__ volatile ( \
        "swc2 $12, 0x8(%0)\n\t" \
        "swc2 $13, 0x14(%0)\n\t" \
        "swc2 $14, 0x20(%0)" \
        : : "r" (r1) : "memory")
/* clang-format on */

/**
 * @brief Stores SXY2 (data 14), the last projected vertex, at `r1`: a quad's
 * fourth vertex after a second transform. Operand: `r1` by "r"; clobbers
 * memory.
 */
/* clang-format off */
#define gte_stsxy2(r1) \
    __asm__ volatile ("swc2 $14, 0x0(%0)" : : "r" (r1) : "memory")
/* clang-format on */

/**
 * @brief Stores the screen-Z FIFO's last three entries, SZ1-SZ3 (data
 * 17-19), at `r1`, `r2` and `r3`. Operands: three pointers by "r"; clobbers
 * memory.
 */
/* clang-format off */
#define gte_stsz3(r1, r2, r3) \
    __asm__ volatile ( \
        "swc2 $17, 0x0(%0)\n\t" \
        "swc2 $18, 0x0(%1)\n\t" \
        "swc2 $19, 0x0(%2)" \
        : : "r" (r1), "r" (r2), "r" (r3) : "memory")
/* clang-format on */

/**
 * @brief Stores all four screen-Z FIFO entries, SZ0-SZ3 (data 16-19), at
 * `r1` to `r4`. Operands: four pointers by "r"; clobbers memory.
 */
/* clang-format off */
#define gte_stsz4(r1, r2, r3, r4) \
    __asm__ volatile ( \
        "swc2 $16, 0x0(%0)\n\t" \
        "swc2 $17, 0x0(%1)\n\t" \
        "swc2 $18, 0x0(%2)\n\t" \
        "swc2 $19, 0x0(%3)" \
        : : "r" (r1), "r" (r2), "r" (r3), "r" (r4) : "memory")
/* clang-format on */

/** @brief Stores SXY0-SXY2 into a POLY_F4's first three vertices: a quad
 * begins as its triangle, so this is gte_stsxy3_f3(). The fourth vertex
 * (+0x14) is stored with gte_stsxy2() after a second transform. */
#define gte_stsxy3_f4(r1) gte_stsxy3_f3(r1)
/** @brief Stores SXY0-SXY2 into a POLY_G4's first three vertices: a quad
 * begins as its triangle, so this is gte_stsxy3_g3(). The fourth vertex
 * (+0x20) is stored with gte_stsxy2() after a second transform. */
#define gte_stsxy3_g4(r1) gte_stsxy3_g3(r1)
/** @brief Stores SXY0-SXY2 into a POLY_FT4's first three vertices: a quad
 * begins as its triangle, so this is gte_stsxy3_ft3(). The fourth vertex
 * (+0x20) is stored with gte_stsxy2() after a second transform. */
#define gte_stsxy3_ft4(r1) gte_stsxy3_ft3(r1)
/** @brief Stores SXY0-SXY2 into a POLY_GT4's first three vertices: a quad
 * begins as its triangle, so this is gte_stsxy3_gt3(). The fourth vertex
 * (+0x2C) is stored with gte_stsxy2() after a second transform. */
#define gte_stsxy3_gt4(r1) gte_stsxy3_gt3(r1)

/**
 * @brief RTPS: perspective-transforms vertex 0 into SXY2/SZ3.
 *
 * The GTE operations (rtps and every macro after it that takes no operand)
 * are emitted as their raw COP2 instruction word, preceded by two no-op
 * instructions that cover the GTE's load latency, as Sony's macros are.
 * No operands, no clobbers.
 */
/* clang-format off */
#define gte_rtps() \
    __asm__ volatile ( \
        "nop\n\t" \
        "nop\n\t" \
        ".word 0x4A180001")
/* clang-format on */

/**
 * @brief RTPT: perspective-transforms vertices 0-2 into SXY0-SXY2 and
 * SZ1-SZ3. No operands, no clobbers.
 */
/* clang-format off */
#define gte_rtpt() \
    __asm__ volatile ( \
        "nop\n\t" \
        "nop\n\t" \
        ".word 0x4A280030")
/* clang-format on */

/**
 * @brief NCLIP: the outer product of SXY0-SXY2 into MAC0 (read back with
 * gte_stopz()): its sign is the triangle's winding, for back-face culling.
 * No operands, no clobbers.
 */
/* clang-format off */
#define gte_nclip() \
    __asm__ volatile ( \
        "nop\n\t" \
        "nop\n\t" \
        ".word 0x4B400006")
/* clang-format on */

/**
 * @brief AVSZ3: the average of SZ1-SZ3, scaled, into OTZ (read back with
 * gte_stotz()): an ordering-table depth. No operands, no clobbers.
 */
/* clang-format off */
#define gte_avsz3() \
    __asm__ volatile ( \
        "nop\n\t" \
        "nop\n\t" \
        ".word 0x4B58002D")
/* clang-format on */

/**
 * @brief MVMVA with the rotation matrix and the IR vector, no translation,
 * shifted by 12: IR1-IR3 := rotation * IR. Sony's rtir; its llir uses the
 * light matrix instead. No operands, no clobbers.
 */
/* clang-format off */
#define gte_rtir() \
    __asm__ volatile ( \
        "nop\n\t" \
        "nop\n\t" \
        ".word 0x4A49E012")
/* clang-format on */

/**
 * @brief NCDS: normal colour depth-cue for vertex 0's normal and the
 * colour RGB (data 6); the result is RGB2. No operands, no clobbers.
 */
/* clang-format off */
#define gte_ncds() \
    __asm__ volatile ( \
        "nop\n\t" \
        "nop\n\t" \
        ".word 0x4AE80413")
/* clang-format on */

/**
 * @brief DPCS: depth-cues the single colour RGB (data 6) by IR0 into RGB2.
 * No operands, no clobbers.
 */
/* clang-format off */
#define gte_dpcs() \
    __asm__ volatile ( \
        "nop\n\t" \
        "nop\n\t" \
        ".word 0x4A780010")
/* clang-format on */

/**
 * @brief DPCT: depth-cues the three colours RGB0-RGB2 by IR0, back into
 * RGB0-RGB2.
 * No operands, no clobbers.
 */
/* clang-format off */
#define gte_dpct() \
    __asm__ volatile ( \
        "nop\n\t" \
        "nop\n\t" \
        ".word 0x4AF8002A")
/* clang-format on */

/**
 * @brief Loads the colour at `r1` into RGB (data 6), the colour input of the
 * depth-cue and normal-colour operations.
 *
 * The colour macros move RGB (data 6) and the three colour outputs
 * RGB0-RGB2 (data 20-22). Each pointer form reads or writes offset 0 of its
 * operand, so a caller passes the colour's own address (`prim + 0x4`), not a
 * base and an offset. Operand: `r1` by "r"; no clobbers.
 */
/* clang-format off */
#define gte_ldrgb(r1) \
    __asm__ volatile ("lwc2 $6, 0x0(%0)" : : "r" (r1))
/* clang-format on */

/**
 * @brief Loads the colours at `r1`, `r2` and `r3` into RGB0-RGB2, and `r3`'s
 * again into RGB. Operands: three pointers by "r"; no clobbers.
 */
/* clang-format off */
#define gte_ldrgb3(r1, r2, r3) \
    __asm__ volatile ( \
        "lwc2 $20, 0x0(%0)\n\t" \
        "lwc2 $21, 0x0(%1)\n\t" \
        "lwc2 $22, 0x0(%2)\n\t" \
        "lwc2 $6, 0x0(%2)" \
        : : "r" (r1), "r" (r2), "r" (r3))
/* clang-format on */

/**
 * @brief Loads three consecutive colours at `r1` (+0x0, +0x4, +0x8) into
 * RGB0-RGB2, and the third again into RGB. Operand: `r1` by "r"; no
 * clobbers.
 */
/* clang-format off */
#define gte_ldrgb3c(r1) \
    __asm__ volatile ( \
        "lwc2 $20, 0x0(%0)\n\t" \
        "lwc2 $21, 0x4(%0)\n\t" \
        "lwc2 $22, 0x8(%0)\n\t" \
        "lwc2 $6, 0x8(%0)" \
        : : "r" (r1))
/* clang-format on */

/**
 * @brief Stores RGB2 (data 22), the single-colour result, at `r1`. Operand:
 * `r1` by "r"; clobbers memory.
 */
/* clang-format off */
#define gte_strgb(r1) \
    __asm__ volatile ("swc2 $22, 0x0(%0)" : : "r" (r1) : "memory")
/* clang-format on */

/**
 * @brief Stores RGB0-RGB2 at `r1`, `r2` and `r3`: how the game writes a
 * POLY_GT3 or POLY_GT4's colours (+0x4, +0x10, +0x1C), which have no
 * one-pointer form here. Operands: three pointers by "r"; clobbers memory.
 */
/* clang-format off */
#define gte_strgb3(r1, r2, r3) \
    __asm__ volatile ( \
        "swc2 $20, 0x0(%0)\n\t" \
        "swc2 $21, 0x0(%1)\n\t" \
        "swc2 $22, 0x0(%2)" \
        : : "r" (r1), "r" (r2), "r" (r3) : "memory")
/* clang-format on */

/**
 * @brief Stores RGB0-RGB2 into a POLY_G3 or POLY_G4's colours, +0x4, +0xC,
 * +0x14 from `r1`. Operand: `r1` by "r"; clobbers memory.
 */
/* clang-format off */
#define gte_strgb3_g3(r1) \
    __asm__ volatile ( \
        "swc2 $20, 0x4(%0)\n\t" \
        "swc2 $21, 0xC(%0)\n\t" \
        "swc2 $22, 0x14(%0)" \
        : : "r" (r1) : "memory")
/* clang-format on */

/**
 * @brief Saves the whole current MATRIX (control 0-7: the 3x3 rotation and
 * the translation vector, 0x20 bytes) to `r1`.
 *
 * The four matrix and column macros are the ones that move data through
 * general registers (cfc2/ctc2, mtc2/mfc2 go by way of a GPR), so they list
 * general registers 12, 13 and 14 as clobbers alongside the pointer operand.
 * Operand: `r1` by "r"; clobbers general registers 12-14 and memory.
 */
/* clang-format off */
#define gte_ReadRotMatrix(r1) \
    __asm__ volatile ( \
        "cfc2 $12, $0\n\t" \
        "cfc2 $13, $1\n\t" \
        "sw $12, 0x0(%0)\n\t" \
        "sw $13, 0x4(%0)\n\t" \
        "cfc2 $12, $2\n\t" \
        "cfc2 $13, $3\n\t" \
        "cfc2 $14, $4\n\t" \
        "sw $12, 0x8(%0)\n\t" \
        "sw $13, 0xC(%0)\n\t" \
        "sw $14, 0x10(%0)\n\t" \
        "cfc2 $12, $5\n\t" \
        "cfc2 $13, $6\n\t" \
        "cfc2 $14, $7\n\t" \
        "sw $12, 0x14(%0)\n\t" \
        "sw $13, 0x18(%0)\n\t" \
        "sw $14, 0x1C(%0)" \
        : : "r" (r1) : "$12", "$13", "$14", "memory")
/* clang-format on */

/**
 * @brief Restores the 3x3 rotation (control 0-4) from `r1`, a MATRIX; the
 * translation vector is left as it is, so it is not the inverse of
 * gte_ReadRotMatrix(). Operand: `r1` by "r"; clobbers general registers
 * 12-14.
 */
/* clang-format off */
#define gte_SetRotMatrix(r1) \
    __asm__ volatile ( \
        "lw $12, 0x0(%0)\n\t" \
        "lw $13, 0x4(%0)\n\t" \
        "ctc2 $12, $0\n\t" \
        "ctc2 $13, $1\n\t" \
        "lw $12, 0x8(%0)\n\t" \
        "lw $13, 0xC(%0)\n\t" \
        "lw $14, 0x10(%0)\n\t" \
        "ctc2 $12, $2\n\t" \
        "ctc2 $13, $3\n\t" \
        "ctc2 $14, $4" \
        : : "r" (r1) : "$12", "$13", "$14")
/* clang-format on */

/**
 * @brief Loads one column of a 3x3 s16 matrix at `r1` (three u16 at a
 * 6-byte stride: +0x0, +0x6, +0xC) into IR1-IR3 (data 9-11). With
 * gte_rtir() and gte_stclmv(), three calls multiply a matrix a column at a
 * time. Operand: `r1` by "r"; clobbers general registers 12-14.
 */
/* clang-format off */
#define gte_ldclmv(r1) \
    __asm__ volatile ( \
        "lhu $12, 0x0(%0)\n\t" \
        "lhu $13, 0x6(%0)\n\t" \
        "lhu $14, 0xC(%0)\n\t" \
        "mtc2 $12, $9\n\t" \
        "mtc2 $13, $10\n\t" \
        "mtc2 $14, $11" \
        : : "r" (r1) : "$12", "$13", "$14")
/* clang-format on */

/**
 * @brief Stores IR1-IR3 into one column of a 3x3 s16 matrix at `r1` (+0x0,
 * +0x6, +0xC). Operand: `r1` by "r"; clobbers general registers 12-14 and
 * memory.
 */
/* clang-format off */
#define gte_stclmv(r1) \
    __asm__ volatile ( \
        "mfc2 $12, $9\n\t" \
        "mfc2 $13, $10\n\t" \
        "mfc2 $14, $11\n\t" \
        "sh $12, 0x0(%0)\n\t" \
        "sh $13, 0x6(%0)\n\t" \
        "sh $14, 0xC(%0)" \
        : : "r" (r1) : "$12", "$13", "$14", "memory")
/* clang-format on */

/**
 * @brief Reads the FLAG control register (control 31), keeps bit 18
 * (0x40000, the SZ3/OTZ saturation flag) and stores it at `r1`: Sony's
 * gte_stflg_4, not gte_stflg, which stores the whole register. Operand:
 * `r1` by "r"; clobbers general registers 12 and 13 and memory.
 */
/* clang-format off */
#define gte_stflg_4(r1) \
    __asm__ volatile ( \
        "cfc2 $12, $31\n\t" \
        "addi $13, $zero, 0x4\n\t" \
        "sll $13, $13, 16\n\t" \
        "and $12, $12, $13\n\t" \
        "sw $12, 0x0(%0)" \
        : : "r" (r1) : "$12", "$13", "memory")
/* clang-format on */

/**
 * @brief Stores MAC0 (data 24), gte_nclip()'s outer product, at `r1`.
 * Operand: `r1` by "r"; clobbers memory.
 */
/* clang-format off */
#define gte_stopz(r1) \
    __asm__ volatile ("swc2 $24, 0x0(%0)" : : "r" (r1) : "memory")
/* clang-format on */

/**
 * @brief Stores IR0 (data 8), the depth-cue interpolation factor, at `r1`.
 * Operand: `r1` by "r"; clobbers memory.
 */
/* clang-format off */
#define gte_stdp(r1) \
    __asm__ volatile ("swc2 $8, 0x0(%0)" : : "r" (r1) : "memory")
/* clang-format on */

/**
 * @brief Stores OTZ (data 7), gte_avsz3()'s average depth, at `r1`.
 * Operand: `r1` by "r"; clobbers memory.
 */
/* clang-format off */
#define gte_stotz(r1) \
    __asm__ volatile ("swc2 $7, 0x0(%0)" : : "r" (r1) : "memory")
/* clang-format on */

/**
 * @brief Stores SXY0-SXY2 at three separate destinations `r1`, `r2`, `r3`.
 * Operands: three pointers by "r"; clobbers memory.
 */
/* clang-format off */
#define gte_stsxy3(r1, r2, r3) \
    __asm__ volatile ( \
        "swc2 $12, 0x0(%0)\n\t" \
        "swc2 $13, 0x0(%1)\n\t" \
        "swc2 $14, 0x0(%2)" \
        : : "r" (r1), "r" (r2), "r" (r3) : "memory")
/* clang-format on */

#endif /* !HOST_BUILD */

#endif
