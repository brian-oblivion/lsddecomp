# GTE macros: notes moved out of include/gte.h

Track 12 (round 106, area-psyq-shared) rewrote `include/gte.h` as API
documentation of its `gte_*` macros. The header keeps what each macro does,
which COP2 registers it moves, which operands it names through an asm
constraint and which clobbers it lists. Everything else its comments said
(how the macro set was chosen, what each form was checked against, the
traps, and pointers to other documents) is kept here verbatim, one section
per comment, headed by the macros that comment sat above.

## History (source comments moved in track 12, round 106)

### The file banner

```c
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
 * src/graphics/tmd_renderer.c is the worked example of a branching function over
 * eight of them.
 */
```

### `gte_ldv0`, `gte_ldv3`

```c
/* Load vertex 0 / all three vertices into the GTE input registers. */
```

### `gte_stsxy3_f3`, `gte_stsxy3_g3`, `gte_stsxy3_ft3`, `gte_stsxy3_gt3`

```c
/* Store the SXY FIFO into one primitive's vertex slots. The offsets are the
 * Psy-Q primitive layouts: _g3 and _ft3 coincide because POLY_G3's per-vertex
 * RGB and POLY_FT3's per-vertex UV are both 4 bytes wide. */
```

### `gte_stsxy2`, `gte_stsz3`, `gte_stsz4`

```c
/* Single SXY2 store, and the SZ FIFO reads. */
```

### `gte_stsxy3_f4`, `gte_stsxy3_g4`, `gte_stsxy3_ft4`, `gte_stsxy3_gt4`

```c
/* Quad variants. The first three stores are identical to the tri form -- a
 * POLY_F4 is a POLY_F3 with a fourth vertex appended -- so these differ from
 * the _f3/_g3/_ft3/_gt3 macros only in name. The fourth vertex is stored
 * separately with gte_stsxy2() after a second transform, at the xy3 offset:
 * F4 +0x14, G4/FT4 +0x20, GT4 +0x2c. */
```

### `gte_rtps`

```c
/* Perspective transform, single vertex. binutils here has no mnemonic for the
 * GTE "cofun" ops, so the encoding is emitted as a raw word; the two leading
 * nops are part of Sony's macro, covering the GTE's load latency. */
```

### `gte_rtpt`, `gte_nclip`, `gte_avsz3`

```c
/* The other GTE "cofun" ops this codebase runs, same raw-word convention. */
```

### `gte_llir`

```c
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
```

### `gte_ncds`, `gte_dpcs`, `gte_dpct`

```c
/* Depth-cue / color-lookup cofun ops used by SortTmdObject's per-face
 * dispatch (one per PS1 GPU primitive flavor: NCDS for flat-shaded,
 * DPCS/DPCT for depth-cued single/triple). Same raw-word convention as the
 * transform ops above -- the pinned `as` rejects all three mnemonics
 * outright ("unrecognized opcode"), confirmed the same way. */
```

### `gte_ldrgb`, `gte_ldrgb3`, `gte_ldrgb3c`, `gte_strgb`, `gte_strgb3`, `gte_strgb3_g3`

```c
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
```

### `gte_ReadRotMatrix`, `gte_SetRotMatrix`, `gte_ldclmv`, `gte_stclmv`

```c
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
```

### `gte_stflg`

```c
/* Read the GTE FLAG control register ($31 of COP2 control), keep only bit 18
 * (0x40000, the SZ3/OTZ saturation flag Sony's macro tests), store it. This
 * is the one macro that uses GPR scratch, and $12/$13 are exactly the GPRs
 * retail shows, so naming them as clobbers here is correct rather than the
 * "wrong clobbers" trap CLAUDE.md warns about for COP2-only blocks. */
```

### `gte_stopz`, `gte_stdp`, `gte_stotz`

```c
/* Single-register result stores: MAC0 (the nclip outer product, "opz"),
 * IR0 (the depth-cue interpolation factor, "dp"), OTZ (the avsz3 average). */
```

### `gte_stsxy3`

```c
/* SXY FIFO to three separate destinations (contrast the _f3/_g3/... forms,
 * which take one primitive base and use its layout's offsets). */
```
