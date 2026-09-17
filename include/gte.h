#ifndef GTE_H
#define GTE_H

/*
 * GNU-syntax GTE inline macros.
 *
 * These are the Psy-Q `gte_*` macros the game's source actually called. The
 * SDK's own include/psyq/INLINE.H cannot be used through this pipeline: it is
 * the ASPSX flavour, which loads the base pointer into $12 and then emits
 * Sony macro-call words (`.word 0x0000227f`) that only Sony's assembler
 * expands. maspsx passes those through and gas emits the literal word, so the
 * bytes do not assemble to anything. Retail's own bytes confirm the game was
 * not built that way either -- there is no `move $12` and the base register is
 * the incoming argument, which is the shape of the GCC-oriented macro set.
 *
 * $N inside these blocks is a COP2 DATA register, a numbering disjoint from
 * the GPRs: 0 VXY0, 1 VZ0, 2 VXY1, 3 VZ1, 4 VXY2, 5 VZ2, 7 OTZ, 8 IR0,
 * 12 SXY0, 13 SXY1, 14 SXY2, 16 SZ0, 17 SZ1, 18 SZ2, 19 SZ3, 24 MAC0.
 * `cfc2 ..., $31` reads COP2 CONTROL register 31, FLAG. The register
 * conventions are cross-checked against include/psyq/GTENOM.H, which
 * documents the same assignments for the ASPSX macro forms.
 *
 * Macro NAMES follow include/psyq/INLINE.H exactly, so a reader with the SDK
 * manual can look one up. When retail shows a GTE instruction this file does
 * not cover yet, add the macro here under the SDK's name rather than
 * open-coding the instruction at the call site (docs/MATCHING-GUIDE.md,
 * step 2). Everything around the macros is ordinary C: TransformAndCullPoly in
 * src/code_8220_b.c is the worked example of a branching function over
 * eight of them, and it was carried as a whole-function __asm__ for twenty
 * rounds before anyone checked.
 */

/* Load vertex 0 / all three vertices into the GTE input registers. */
#define gte_ldv0(r1) \
    __asm__ volatile ( \
        "lwc2 $0, 0x0(%0)\n\t" \
        "lwc2 $1, 0x4(%0)" \
        : : "r" (r1))

#define gte_ldv3(r1, r2, r3) \
    __asm__ volatile ( \
        "lwc2 $0, 0x0(%0)\n\t" \
        "lwc2 $1, 0x4(%0)\n\t" \
        "lwc2 $2, 0x0(%1)\n\t" \
        "lwc2 $3, 0x4(%1)\n\t" \
        "lwc2 $4, 0x0(%2)\n\t" \
        "lwc2 $5, 0x4(%2)" \
        : : "r" (r1), "r" (r2), "r" (r3))

/* Store the SXY FIFO into one primitive's vertex slots. The offsets are the
 * Psy-Q primitive layouts: _g3 and _ft3 coincide because POLY_G3's per-vertex
 * RGB and POLY_FT3's per-vertex UV are both 4 bytes wide. */
#define gte_stsxy3_f3(r1) \
    __asm__ volatile ( \
        "swc2 $12, 0x8(%0)\n\t" \
        "swc2 $13, 0xc(%0)\n\t" \
        "swc2 $14, 0x10(%0)" \
        : : "r" (r1) : "memory")

#define gte_stsxy3_g3(r1) \
    __asm__ volatile ( \
        "swc2 $12, 0x8(%0)\n\t" \
        "swc2 $13, 0x10(%0)\n\t" \
        "swc2 $14, 0x18(%0)" \
        : : "r" (r1) : "memory")

#define gte_stsxy3_ft3(r1) \
    __asm__ volatile ( \
        "swc2 $12, 0x8(%0)\n\t" \
        "swc2 $13, 0x10(%0)\n\t" \
        "swc2 $14, 0x18(%0)" \
        : : "r" (r1) : "memory")

#define gte_stsxy3_gt3(r1) \
    __asm__ volatile ( \
        "swc2 $12, 0x8(%0)\n\t" \
        "swc2 $13, 0x14(%0)\n\t" \
        "swc2 $14, 0x20(%0)" \
        : : "r" (r1) : "memory")

/* Single SXY2 store, and the SZ FIFO reads. */
#define gte_stsxy2(r1) \
    __asm__ volatile ("swc2 $14, 0x0(%0)" : : "r" (r1) : "memory")

#define gte_stsz3(r1, r2, r3) \
    __asm__ volatile ( \
        "swc2 $17, 0x0(%0)\n\t" \
        "swc2 $18, 0x0(%1)\n\t" \
        "swc2 $19, 0x0(%2)" \
        : : "r" (r1), "r" (r2), "r" (r3) : "memory")

#define gte_stsz4(r1, r2, r3, r4) \
    __asm__ volatile ( \
        "swc2 $16, 0x0(%0)\n\t" \
        "swc2 $17, 0x0(%1)\n\t" \
        "swc2 $18, 0x0(%2)\n\t" \
        "swc2 $19, 0x0(%3)" \
        : : "r" (r1), "r" (r2), "r" (r3), "r" (r4) : "memory")

/* Quad variants. The first three stores are identical to the tri form -- a
 * POLY_F4 is a POLY_F3 with a fourth vertex appended -- so these differ from
 * the _f3/_g3/_ft3/_gt3 macros only in name. The fourth vertex is stored
 * separately with gte_stsxy2() after a second transform, at the xy3 offset:
 * F4 +0x14, G4/FT4 +0x20, GT4 +0x2c. */
#define gte_stsxy3_f4(r1)  gte_stsxy3_f3(r1)
#define gte_stsxy3_g4(r1)  gte_stsxy3_g3(r1)
#define gte_stsxy3_ft4(r1) gte_stsxy3_ft3(r1)
#define gte_stsxy3_gt4(r1) gte_stsxy3_gt3(r1)

/* Perspective transform, single vertex. binutils here has no mnemonic for the
 * GTE "cofun" ops, so the encoding is emitted as a raw word; the two leading
 * nops are part of Sony's macro, covering the GTE's load latency. */
#define gte_rtps() \
    __asm__ volatile ( \
        "nop\n\t" \
        "nop\n\t" \
        ".word 0x4A180001")

/* The other GTE "cofun" ops this codebase runs, same raw-word convention. */
#define gte_rtpt() \
    __asm__ volatile ( \
        "nop\n\t" \
        "nop\n\t" \
        ".word 0x4A280030")

#define gte_nclip() \
    __asm__ volatile ( \
        "nop\n\t" \
        "nop\n\t" \
        ".word 0x4B400006")

#define gte_avsz3() \
    __asm__ volatile ( \
        "nop\n\t" \
        "nop\n\t" \
        ".word 0x4B58002D")

/* Local-matrix / IR-vector multiply-add: mvmva(sf=1, mx=0 "local matrix",
 * v=3 "long IR vector", cv=3 "none", lm=0). Confirmed against retail, not
 * against include/psyq/INLINE.H's gte_llir() -- that header's `.word
 * 0x0000133f/0x133e/0x133e` is the ASPSX macro-CALL encoding (only Sony's
 * assembler expands it, see the file banner above), and is a different
 * value from the actual COP2 cofun word. The word below is retail's own
 * three occurrences in func_80018464 (splat already decodes them as
 * `mvmva 1, 0, 3, 3, 0`), and it fails to assemble under the pinned `as`
 * with a bare "mvmva" mnemonic (Error: unrecognized opcode) -- confirmed
 * with the reproducer in CLAUDE.md's "Escalate, do not experiment", so this
 * is the raw-word case, same as gte_rtps/gte_rtpt/gte_nclip/gte_avsz3 above.
 * Kept under Sony's own macro name (INLINE.H's naming scheme: "ll" = local
 * matrix, "ir" = IR vector, no tr/bk/fc suffix = cv=3/none) since the field
 * VALUES match even though the encoding form does not. */
#define gte_llir() \
    __asm__ volatile ( \
        "nop\n\t" \
        "nop\n\t" \
        ".word 0x4A49E012")

/* Depth-cue / color-lookup cofun ops used by func_80018464's per-face
 * dispatch (one per PS1 GPU primitive flavor: NCDS for flat-shaded,
 * DPCS/DPCT for depth-cued single/triple). Same raw-word convention as the
 * transform ops above -- the pinned `as` rejects all three mnemonics
 * outright ("unrecognized opcode"), confirmed the same way. */
#define gte_ncds() \
    __asm__ volatile ( \
        "nop\n\t" \
        "nop\n\t" \
        ".word 0x4AE80413")

#define gte_dpcs() \
    __asm__ volatile ( \
        "nop\n\t" \
        "nop\n\t" \
        ".word 0x4A780010")

#define gte_dpct() \
    __asm__ volatile ( \
        "nop\n\t" \
        "nop\n\t" \
        ".word 0x4AF8002A")

/* Read the GTE FLAG control register ($31 of COP2 control), keep only bit 18
 * (0x40000, the SZ3/OTZ saturation flag Sony's macro tests), store it. This
 * is the one macro that uses GPR scratch, and $12/$13 are exactly the GPRs
 * retail shows, so naming them as clobbers here is correct rather than the
 * "wrong clobbers" trap CLAUDE.md warns about for COP2-only blocks. */
#define gte_stflg(r1) \
    __asm__ volatile ( \
        "cfc2 $12, $31\n\t" \
        "addi $13, $zero, 0x4\n\t" \
        "sll $13, $13, 16\n\t" \
        "and $12, $12, $13\n\t" \
        "sw $12, 0x0(%0)" \
        : : "r" (r1) : "$12", "$13", "memory")

/* Single-register result stores: MAC0 (the nclip outer product, "opz"),
 * IR0 (the depth-cue interpolation factor, "dp"), OTZ (the avsz3 average). */
#define gte_stopz(r1) \
    __asm__ volatile ("swc2 $24, 0x0(%0)" : : "r" (r1) : "memory")

#define gte_stdp(r1) \
    __asm__ volatile ("swc2 $8, 0x0(%0)" : : "r" (r1) : "memory")

#define gte_stotz(r1) \
    __asm__ volatile ("swc2 $7, 0x0(%0)" : : "r" (r1) : "memory")

/* SXY FIFO to three separate destinations (contrast the _f3/_g3/... forms,
 * which take one primitive base and use its layout's offsets). */
#define gte_stsxy3(r1, r2, r3) \
    __asm__ volatile ( \
        "swc2 $12, 0x0(%0)\n\t" \
        "swc2 $13, 0x0(%1)\n\t" \
        "swc2 $14, 0x0(%2)" \
        : : "r" (r1), "r" (r2), "r" (r3) : "memory")

#endif
