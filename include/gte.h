#ifndef GTE_H
#define GTE_H

/*
 * GNU-syntax GTE inline macros.
 *
 * These are the Psy-Q `gte_*` macros the game's source actually called. The
 * SDK's own include/psyq/inline.h cannot be used through this pipeline: it is
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
 * conventions are cross-checked against include/psyq/gtenom.h, which
 * documents the same assignments for the ASPSX macro forms.
 *
 * Macro NAMES follow include/psyq/inline.h exactly, so a reader with the SDK
 * manual can look one up. When retail shows a GTE instruction this file does
 * not cover yet, add the macro here under the SDK's name rather than
 * open-coding the instruction at the call site (docs/MATCHING-GUIDE.md,
 * step 2). Everything around the macros is ordinary C: TransformAndCullPoly in
 * src/graphics/TmdRenderer.c is the worked example of a branching function over
 * eight of them, and it was carried as a whole-function __asm__ for twenty
 * rounds before anyone checked.
 */

/* Load vertex 0 / all three vertices into the GTE input registers. */
/* clang-format off */
#define gte_ldv0(r1) \
    __asm__ volatile ( \
        "lwc2 $0, 0x0(%0)\n\t" \
        "lwc2 $1, 0x4(%0)" \
        : : "r" (r1))
/* clang-format on */

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

/* Store the SXY FIFO into one primitive's vertex slots. The offsets are the
 * Psy-Q primitive layouts: _g3 and _ft3 coincide because POLY_G3's per-vertex
 * RGB and POLY_FT3's per-vertex UV are both 4 bytes wide. */
/* clang-format off */
#define gte_stsxy3_f3(r1) \
    __asm__ volatile ( \
        "swc2 $12, 0x8(%0)\n\t" \
        "swc2 $13, 0xc(%0)\n\t" \
        "swc2 $14, 0x10(%0)" \
        : : "r" (r1) : "memory")
/* clang-format on */

/* clang-format off */
#define gte_stsxy3_g3(r1) \
    __asm__ volatile ( \
        "swc2 $12, 0x8(%0)\n\t" \
        "swc2 $13, 0x10(%0)\n\t" \
        "swc2 $14, 0x18(%0)" \
        : : "r" (r1) : "memory")
/* clang-format on */

/* clang-format off */
#define gte_stsxy3_ft3(r1) \
    __asm__ volatile ( \
        "swc2 $12, 0x8(%0)\n\t" \
        "swc2 $13, 0x10(%0)\n\t" \
        "swc2 $14, 0x18(%0)" \
        : : "r" (r1) : "memory")
/* clang-format on */

/* clang-format off */
#define gte_stsxy3_gt3(r1) \
    __asm__ volatile ( \
        "swc2 $12, 0x8(%0)\n\t" \
        "swc2 $13, 0x14(%0)\n\t" \
        "swc2 $14, 0x20(%0)" \
        : : "r" (r1) : "memory")
/* clang-format on */

/* Single SXY2 store, and the SZ FIFO reads. */
/* clang-format off */
#define gte_stsxy2(r1) \
    __asm__ volatile ("swc2 $14, 0x0(%0)" : : "r" (r1) : "memory")
/* clang-format on */

/* clang-format off */
#define gte_stsz3(r1, r2, r3) \
    __asm__ volatile ( \
        "swc2 $17, 0x0(%0)\n\t" \
        "swc2 $18, 0x0(%1)\n\t" \
        "swc2 $19, 0x0(%2)" \
        : : "r" (r1), "r" (r2), "r" (r3) : "memory")
/* clang-format on */

/* clang-format off */
#define gte_stsz4(r1, r2, r3, r4) \
    __asm__ volatile ( \
        "swc2 $16, 0x0(%0)\n\t" \
        "swc2 $17, 0x0(%1)\n\t" \
        "swc2 $18, 0x0(%2)\n\t" \
        "swc2 $19, 0x0(%3)" \
        : : "r" (r1), "r" (r2), "r" (r3), "r" (r4) : "memory")
/* clang-format on */

/* Quad variants. The first three stores are identical to the tri form -- a
 * POLY_F4 is a POLY_F3 with a fourth vertex appended -- so these differ from
 * the _f3/_g3/_ft3/_gt3 macros only in name. The fourth vertex is stored
 * separately with gte_stsxy2() after a second transform, at the xy3 offset:
 * F4 +0x14, G4/FT4 +0x20, GT4 +0x2c. */
#define gte_stsxy3_f4(r1) gte_stsxy3_f3(r1)
#define gte_stsxy3_g4(r1) gte_stsxy3_g3(r1)
#define gte_stsxy3_ft4(r1) gte_stsxy3_ft3(r1)
#define gte_stsxy3_gt4(r1) gte_stsxy3_gt3(r1)

/* Perspective transform, single vertex. binutils here has no mnemonic for the
 * GTE "cofun" ops, so the encoding is emitted as a raw word; the two leading
 * nops are part of Sony's macro, covering the GTE's load latency. */
/* clang-format off */
#define gte_rtps() \
    __asm__ volatile ( \
        "nop\n\t" \
        "nop\n\t" \
        ".word 0x4A180001")
/* clang-format on */

/* The other GTE "cofun" ops this codebase runs, same raw-word convention. */
/* clang-format off */
#define gte_rtpt() \
    __asm__ volatile ( \
        "nop\n\t" \
        "nop\n\t" \
        ".word 0x4A280030")
/* clang-format on */

/* clang-format off */
#define gte_nclip() \
    __asm__ volatile ( \
        "nop\n\t" \
        "nop\n\t" \
        ".word 0x4B400006")
/* clang-format on */

/* clang-format off */
#define gte_avsz3() \
    __asm__ volatile ( \
        "nop\n\t" \
        "nop\n\t" \
        ".word 0x4B58002D")
/* clang-format on */

/* Local-matrix / IR-vector multiply-add: mvmva(sf=1, mx=0 "local matrix",
 * v=3 "long IR vector", cv=3 "none", lm=0). Confirmed against retail, not
 * against include/psyq/inline.h's gte_llir() -- that header's `.word
 * 0x0000133f/0x133e/0x133e` is the ASPSX macro-CALL encoding (only Sony's
 * assembler expands it, see the file banner above), and is a different
 * value from the actual COP2 cofun word. The word below is retail's own
 * three occurrences in SortTmdObject (splat already decodes them as
 * `mvmva 1, 0, 3, 3, 0`), and it fails to assemble under the pinned `as`
 * with a bare "mvmva" mnemonic (Error: unrecognized opcode) -- confirmed
 * with the reproducer in CLAUDE.md's "Escalate, do not experiment", so this
 * is the raw-word case, same as gte_rtps/gte_rtpt/gte_nclip/gte_avsz3 above.
 * Kept under Sony's own macro name (INLINE.H's naming scheme: "ll" = local
 * matrix, "ir" = IR vector, no tr/bk/fc suffix = cv=3/none) since the field
 * VALUES match even though the encoding form does not. */
/* clang-format off */
#define gte_llir() \
    __asm__ volatile ( \
        "nop\n\t" \
        "nop\n\t" \
        ".word 0x4A49E012")
/* clang-format on */

/* Depth-cue / color-lookup cofun ops used by SortTmdObject's per-face
 * dispatch (one per PS1 GPU primitive flavor: NCDS for flat-shaded,
 * DPCS/DPCT for depth-cued single/triple). Same raw-word convention as the
 * transform ops above -- the pinned `as` rejects all three mnemonics
 * outright ("unrecognized opcode"), confirmed the same way. */
/* clang-format off */
#define gte_ncds() \
    __asm__ volatile ( \
        "nop\n\t" \
        "nop\n\t" \
        ".word 0x4AE80413")
/* clang-format on */

/* clang-format off */
#define gte_dpcs() \
    __asm__ volatile ( \
        "nop\n\t" \
        "nop\n\t" \
        ".word 0x4A780010")
/* clang-format on */

/* clang-format off */
#define gte_dpct() \
    __asm__ volatile ( \
        "nop\n\t" \
        "nop\n\t" \
        ".word 0x4AF8002A")
/* clang-format on */

/* RGB load/store. COP2 data register 6 is RGB (the colour INPUT the
 * depth-cue and normal-colour ops read); 20/21/22 are RGB0/RGB1/RGB2, the
 * three colour OUTPUTS. These are Sony's own operand shapes, read off
 * include/psyq/inline.h and confirmed instruction-for-instruction against
 * retail in SortTmdObject:
 *
 *   gte_ldrgb(p)          one pointer,   1 op
 *   gte_ldrgb3(p0,p1,p2)  three pointers, 4 ops (the 4th reloads p2 into RGB)
 *   gte_ldrgb3c(p)        one pointer "contiguous", 4 ops, same 4th load
 *   gte_strgb(p)          one pointer,   1 op
 *   gte_strgb3(p0,p1,p2)  three pointers, 3 ops
 *   gte_strgb3_g3(p)      one pointer,   3 ops, POLY_G3/G4 colour offsets
 *
 * INLINE.H is the ASPSX flavour and its macro-call words say nothing about
 * the offsets (see the file banner above), but the OPERAND COUNT and the OP
 * COUNT are readable there and they decide the bytes -- which is why these
 * forms are not interchangeable. The pointer forms take their address at
 * offset 0x0, so a call site spelling `gte_strgb(prim + 0x4)` makes GCC
 * materialise the sum with its own `addiu`. That is exactly what retail
 * shows, fifteen times over in SortTmdObject. Open-coding the same store as
 * `swc2 $22, 0x4(%0)` on the base pointer drops the addiu, and the function
 * then assembles short by one word per call site.
 *
 * The _g3 suffix is Sony's: POLY_G3 and POLY_G4 carry their three RGBs at
 * +0x4/+0xC/+0x14, close enough to index from one pointer. POLY_GT3 and
 * POLY_GT4 space theirs at +0x4/+0x10/+0x1C, and retail reaches those with
 * the generic three-pointer gte_strgb3() rather than Sony's own
 * gte_strgb3_gt3(), so only the form actually observed is spelled here. */
/* clang-format off */
#define gte_ldrgb(r1) \
    __asm__ volatile ("lwc2 $6, 0x0(%0)" : : "r" (r1))
/* clang-format on */

/* clang-format off */
#define gte_ldrgb3(r1, r2, r3) \
    __asm__ volatile ( \
        "lwc2 $20, 0x0(%0)\n\t" \
        "lwc2 $21, 0x0(%1)\n\t" \
        "lwc2 $22, 0x0(%2)\n\t" \
        "lwc2 $6, 0x0(%2)" \
        : : "r" (r1), "r" (r2), "r" (r3))
/* clang-format on */

/* clang-format off */
#define gte_ldrgb3c(r1) \
    __asm__ volatile ( \
        "lwc2 $20, 0x0(%0)\n\t" \
        "lwc2 $21, 0x4(%0)\n\t" \
        "lwc2 $22, 0x8(%0)\n\t" \
        "lwc2 $6, 0x8(%0)" \
        : : "r" (r1))
/* clang-format on */

/* clang-format off */
#define gte_strgb(r1) \
    __asm__ volatile ("swc2 $22, 0x0(%0)" : : "r" (r1) : "memory")
/* clang-format on */

/* clang-format off */
#define gte_strgb3(r1, r2, r3) \
    __asm__ volatile ( \
        "swc2 $20, 0x0(%0)\n\t" \
        "swc2 $21, 0x0(%1)\n\t" \
        "swc2 $22, 0x0(%2)" \
        : : "r" (r1), "r" (r2), "r" (r3) : "memory")
/* clang-format on */

/* clang-format off */
#define gte_strgb3_g3(r1) \
    __asm__ volatile ( \
        "swc2 $20, 0x4(%0)\n\t" \
        "swc2 $21, 0xC(%0)\n\t" \
        "swc2 $22, 0x14(%0)" \
        : : "r" (r1) : "memory")
/* clang-format on */

/* Whole-MATRIX moves through the COP2 CONTROL registers, and the column-vector
 * pair. These four are the only macros here that touch GPRs, and $12/$13/$14
 * are exactly the GPRs retail shows -- the same situation gte_stflg() is in
 * below, not the "wrong clobbers" trap, since these instructions really do
 * move through general registers.
 *
 * Sony's include/psyq/inline.h settles which op belongs to which name by
 * operand count and op count even though its macro-call words say nothing
 * about the encodings: gte_ReadRotMatrix is 16 ops, gte_SetRotMatrix 10,
 * gte_ldclmv and gte_stclmv 6 each. Retail's SortTmdObject preamble has
 * blocks of exactly 16, 10, 6, 6 and 10 in that order, so the mapping is not
 * a guess.
 *
 * Note the asymmetry, which is retail's and not a transcription slip:
 * gte_ReadRotMatrix saves control 0..7 -- the 3x3 rotation AND the
 * translation vector, a full Psy-Q MATRIX, 0x20 bytes -- while
 * gte_SetRotMatrix restores only control 0..4, the 3x3. The translation
 * vector is read out and never put back.
 *
 * gte_ldclmv/gte_stclmv move ONE COLUMN of a 3x3 s16 matrix (stride 6) in and
 * out of IR1/IR2/IR3, which is what makes the three-call loop in that
 * preamble a matrix multiply done a column at a time. The halfword loads are
 * `lhu`, unsigned, so the C form is u16. */
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

/* Read the GTE FLAG control register ($31 of COP2 control), keep only bit 18
 * (0x40000, the SZ3/OTZ saturation flag Sony's macro tests), store it. This
 * is the one macro that uses GPR scratch, and $12/$13 are exactly the GPRs
 * retail shows, so naming them as clobbers here is correct rather than the
 * "wrong clobbers" trap CLAUDE.md warns about for COP2-only blocks. */
/* clang-format off */
#define gte_stflg(r1) \
    __asm__ volatile ( \
        "cfc2 $12, $31\n\t" \
        "addi $13, $zero, 0x4\n\t" \
        "sll $13, $13, 16\n\t" \
        "and $12, $12, $13\n\t" \
        "sw $12, 0x0(%0)" \
        : : "r" (r1) : "$12", "$13", "memory")
/* clang-format on */

/* Single-register result stores: MAC0 (the nclip outer product, "opz"),
 * IR0 (the depth-cue interpolation factor, "dp"), OTZ (the avsz3 average). */
/* clang-format off */
#define gte_stopz(r1) \
    __asm__ volatile ("swc2 $24, 0x0(%0)" : : "r" (r1) : "memory")
/* clang-format on */

/* clang-format off */
#define gte_stdp(r1) \
    __asm__ volatile ("swc2 $8, 0x0(%0)" : : "r" (r1) : "memory")
/* clang-format on */

/* clang-format off */
#define gte_stotz(r1) \
    __asm__ volatile ("swc2 $7, 0x0(%0)" : : "r" (r1) : "memory")
/* clang-format on */

/* SXY FIFO to three separate destinations (contrast the _f3/_g3/... forms,
 * which take one primitive base and use its layout's offsets). */
/* clang-format off */
#define gte_stsxy3(r1, r2, r3) \
    __asm__ volatile ( \
        "swc2 $12, 0x0(%0)\n\t" \
        "swc2 $13, 0x0(%1)\n\t" \
        "swc2 $14, 0x0(%2)" \
        : : "r" (r1), "r" (r2), "r" (r3) : "memory")
/* clang-format on */

#endif
