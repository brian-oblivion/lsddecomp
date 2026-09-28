#ifndef INCLUDE_ASM_H
#define INCLUDE_ASM_H

/**
 * @file
 * @brief The two macros that splice generated disassembly into a unit, for
 *        a function or a rodata block not yet written as C.
 *
 * The function macro assembles `FOLDER/NAME.s` into .text with the assembler
 * temporary and instruction reordering left to the file; the rodata macro
 * assembles it into .rodata and returns to .text. Both expand to nothing
 * when the unit is preprocessed for a tool that reads the C alone (the
 * M2CTX define, or its source-mutation sibling tested below). The file-scope block includes
 * the assembler macro file the spliced files need.
 */

#if !defined(M2CTX) && !defined(PERMUTER)

#ifndef INCLUDE_ASM
#define INCLUDE_ASM(FOLDER, NAME) \
    __asm__( \
        ".section .text\n" \
        "    .set noat\n" \
        "    .set noreorder\n" \
        "    .include \"" FOLDER "/" #NAME ".s\"\n" \
        "    .set reorder\n" \
        "    .set at\n" \
    )
#endif
#ifndef INCLUDE_RODATA
#define INCLUDE_RODATA(FOLDER, NAME) \
    __asm__( \
        ".section .rodata\n" \
        "    .include \"" FOLDER "/" #NAME ".s\"\n" \
        ".section .text" \
    )
#endif

#if INCLUDE_ASM_USE_MACRO_INC
__asm__(".include \"include/macro.inc\"\n");
#else
__asm__(".include \"include/labels.inc\"\n");
#endif

#else

#ifndef INCLUDE_ASM
#define INCLUDE_ASM(FOLDER, NAME)
#endif
#ifndef INCLUDE_RODATA
#define INCLUDE_RODATA(FOLDER, NAME)
#endif

#endif /* the spliced or the empty macros */

#endif /* INCLUDE_ASM_H */
