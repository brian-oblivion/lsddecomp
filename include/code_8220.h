#ifndef CODE_8220_H
#define CODE_8220_H

#include "common.h"

/* The BasicClass type, its method table and its list primitives are
 * include/BasicClass.h (FINISHING-PLAN track 4: one header per class).
 * This header keeps the rest of the code_8220 units' declarations: the
 * pool allocator, the prim-setup and GTE helpers. */
#include "BasicClass.h"

/*
 * BMemBlockHdr -- a single free-list node inside a BMemPMgr's pool area.
 * `sizeAndFlags` packs the block's byte size into the low 28 bits and
 * flag bits into the high 4 (0x40000000 = free); `prev`/`next` link the
 * pool's doubly-linked free list. Derived from SetupBMemPMgrFreeList (round 45)
 * and reused by BMemPMgrAlloc/BMemPMgrFree's still-undecoded bodies,
 * which walk this same list via BMemPMgr's freeListStart/freeListEnd.
 */
typedef struct BMemBlockHdr BMemBlockHdr;

struct BMemBlockHdr {
    /* +0x000 */ u32 sizeAndFlags;
    /* +0x004 */ BMemBlockHdr *prev;
    /* +0x008 */ BMemBlockHdr *next;
};

/*
 * bMemPMgr -- BMemPMgrInit's own pool-header object. `freeListHead`/
 * `poolSize` are the two fields BMemPMgrInit itself writes; the three
 * below them (round 45, SetupBMemPMgrFreeList) round out the pool's free-list
 * bookkeeping. What remains opaque is the pool AREA itself (poolSize +
 * 0x20 bytes total, starting at `freeListHead`), walked as a chain of
 * BMemBlockHdr nodes rather than through any field of this struct.
 */
typedef struct BMemPMgr BMemPMgr;

struct BMemPMgr {
    /* +0x000 */ void *freeListHead; /* set to `self + 0x1C` by BMemPMgrInit; the pool's first free-list node */
    /* +0x004 */ s32 poolSize;
    /* +0x008 */ BMemBlockHdr *freeListStart; /* free list head, SetupBMemPMgrFreeList/B34/CFC */
    /* +0x00C */ BMemBlockHdr *freeListEnd;   /* free list tail, same trio */
    /* +0x010 */ s32 unk10; /* set to 1 by SetupBMemPMgrFreeList; not yet read by any decoded function */
};

/* The generic pool allocator/free pair, established already by
 * include/DreamSys.h, include/Entity.h etc. --
 * all single-argument, and every one of those ~15 headers declares its
 * own full ANSI prototype (`s32 size` / `void *ptr`), per this project's
 * multiple-independent-local-views convention -- none of them get their
 * declaration from this header.
 *
 * THIS header, uniquely, declares both with UNSPECIFIED parameters
 * (empty parens). Round 45 (BMemPMgrAlloc/BMemPMgrFree, matched): each
 * function's own BODY genuinely reads a second argument ($a1, a fallback
 * pool pointer used only when the global default pool gDefaultBMemPMgr is
 * unset -- dead in practice at every decoded call site, confirmed by
 * SetupBMemPMgrFreeList/A9C setting that global before either is ever called).
 * Both are therefore DEFINED in code_8220.c with an old-style
 * (K&R identifier-list) parameter list, which is the only way to expose
 * that second parameter to their own bodies without contradicting the
 * ~15 external single-argument prototypes OR this same unit's own
 * single-argument call sites (PushBasicClassListNode's `BMemPMgrAlloc(0x8)`,
 * RemoveBasicClassListNode's `BMemPMgrFree(node)`) that appear LATER in
 * code_8220.c. A K&R-style definition does not install a prototype, so
 * those later 1-argument calls stay uncheck-and-compile clean; an
 * unspecified-parameter declaration here does the same for everything
 * before the definition. Do not "fix" this back to a full prototype --
 * that reintroduces the conflict this was written to route around. */
extern void *BMemPMgrAlloc(); /* arity-ok: re-measured round 59 -- the body really does read $a1 -- `move s1,a1` at 0x80017B40, consumed as `move t0,s1` at 0x80017B68 only when the gp default pool is unset. The ~22 one-parameter declarations elsewhere are right about THEIR call sites (retail emits $a0 only, e.g. `move a0,s2` at 0x80026B74); this unprototyped pair is required by the K&R definitions in code_8220.c. */
extern void *BMemPMgrFree(); /* arity-ok: re-measured round 59, same -- `move s1,a1` at 0x80017D0C, consumed as `move t0,s1` at 0x80017D2C on the unset-default-pool path. */

/* BMemPMgr setup, gp_rel-blocked (docs/research/gp-relative-blocker.md).
 * Called only by BMemPMgrInit in this unit. Genuinely ONE argument: its
 * own body's $a1 is a fallback pool pointer (defaulting to $a0/self) used
 * only when the global default pool gDefaultBMemPMgr is unset, and
 * BMemPMgrInit's call site never sets $a1 before the `jal` -- confirmed
 * by objdump: declaring a second parameter here forces the caller to
 * materialise a spurious `move a1,s1`, one word too many. */
extern void SetupBMemPMgrFreeList(BMemPMgr *pool);

/* The default-pool global itself (see the comment above). Setter is
 * SetDefaultBMemPMgr(BMemPMgr *pool), a one-line `gDefaultBMemPMgr = pool;`. Not yet
 * called from any carved C -- BMemPMgrInit never calls it, so whoever
 * establishes the game's one default pool is still asm. */
extern BMemPMgr *gDefaultBMemPMgr;

/* Pool allocator/free critical-section flag, code_8220_b (setter
 * SetBMemPMgrBusy, getter GetBMemPMgrBusy). BMemPMgrAlloc/BMemPMgrFree in
 * THIS unit bracket their free-list walk with SetBMemPMgrBusy(1) on entry
 * and SetBMemPMgrBusy(0) on exit -- an enter/exit pair, not a real lock
 * (no busy-wait or check on entry visible in either caller). */
extern s32 gBMemPMgrBusy;
extern void SetBMemPMgrBusy(s32 val);
extern s32 GetBMemPMgrBusy(void);

/* The Psy-Q declarations that used to sit here (func_80011D34 is malloc,
 * func_80011F68 is free, func_80012C20 is printf) moved into src/code_8220.c
 * when the SDK objects were linked. They are deliberately NOT shared:
 * printf is variadic and five units each declare the argument shape their
 * own call site passes, and malloc/free now have Sony's real names, so a
 * copy in this header would collide with <malloc.h> in whichever sibling
 * unit includes the SDK header first. See CLAUDE.md, "To include/ has one
 * exception". */

/* The "bMemPMgr = %p, poolSize = %ld in BMemPMgrInit\n" format string,
 * asm/data/A8C.rodata.s. */
extern const char D_8001028C[];

/* Global boolean flag read by SetupPrimCode, asm/data (bss/data, not yet
 * carved). Read by SetupPrimCode, written by func_80018464 (both code_8220_b)
 * from bit 6 of the drawn object's flags word; SetupPrimCode ORs it into bit
 * 0x1 of the GPU command byte, which is the shade-texture bit Psy-Q's
 * SetShadeTex() sets. PROPOSED RENAME (round 51, tier B): gShadeTex.
 * tools/rename.py cannot do it -- "resolves to 0x8008e248, outside the
 * image", because it is bss past the image end -- so the head applies it. */
extern s32 D_8008E248;

/* GTE transform/clip/OT-bucket routine, this unit (code_8220_b; ordinary C
 * over the include/gte.h macros -- see docs/match-reports/TransformAndCullPoly.md;
 * the .c holds its local struct views of both arguments). arg1 is the
 * per-object draw context (OT base +0x0, OT shift +0x4, culled-flag +0x78,
 * SXY0-2 cache +0x60/0x64/0x68, computed OT bucket pointer +0x30, ...);
 * arg0 is the Psy-Q GPU primitive being filled in, and its only touched
 * field is the P_TAG length byte at +0x3, re-stamped from the copy
 * SetupPrimCode cached at arg1->0x14. Returns 0 on success (OT bucket
 * computed and stored), 1 if the primitive was culled/degenerate. Declared
 * here because its two callers in this unit (ProjectTriFace,
 * ProjectQuadFace) are earlier in ROM order and so precede its own
 * definition in the .c file. */
extern s32 TransformAndCullPoly(void *arg0, void *arg1);

/* Called by ProjectTriFace/ProjectQuadFace at the end of a face that was not
 * culled. `count` is the face's VERTEX COUNT (3 or 4), not a primitive-kind
 * code: docs/match-reports/UpdatePolyBBoxAndCull.md derives the body, which walks
 * `count` screen-XY pairs from ctx+0x64 to ctx + 0x5C + count*4, tracks the
 * 2D bounding box in +0x70../+0x76 and sets the culled flag at +0x78 when
 * either span reaches 0x101. The older "3 = triangle, 4 = quad" wording here
 * read the right numbers off the call sites for the wrong reason; corrected
 * round 51. `ctx` (named round 77) is the same per-face draw context
 * TransformAndCullPoly above documents -- its SXY0-2 cache and culled flag
 * are exactly the fields this function reads and sets. code_8220_c, round 13. */
extern void UpdatePolyBBoxAndCull(void *ctx, s32 count);

/* Round 13: this unit's own minimal, local view of the Psy-Q GPU primitive
 * tag word -- the same shape as `P_TAG` in include/psyq/libgpu.h, declared
 * locally because that SDK header does not compile standalone under this
 * toolchain (it needs the LIBGTE/RECT chain) and no unit includes it yet.
 *
 * The 24-bit BITFIELD is the load-bearing part. On little-endian MIPS
 * `addr` occupies bits 0..23, so writing it is a read-modify-write that
 * GCC 2.6.3 emits as `& 0xFF000000` on the old word, `& 0x00FFFFFF` on the
 * new value, and an `or` -- which is exactly retail's OT linked-list
 * splice. Hand-writing those masks produces the same VALUE with different
 * register allocation and does NOT match. */
typedef struct OtTag {
    u32 addr : 24;
    u32 len : 8;
} OtTag;

/* FillRVectors3's payload types (round 13). Both are ALL-s16 and that is
 * load-bearing: all-s16 members give alignment 2, which is what makes a
 * whole-struct assignment compile to unaligned lwl/lwr + swl/swr instead of
 * aligned lw/sw. See DECOMPILATION_LEARNINGS, "A struct whose members are
 * all s8/s16 has alignment 2". One stray s32 member and the copy stops
 * matching. */
typedef struct PolyXY8 {
    s16 a;
    s16 b;
    s16 c;
    s16 d;
} PolyXY8;

typedef struct PolyUV4 {
    s16 u;
    s16 v;
} PolyUV4;

/* FillRVectors3's element type. Only the 8-byte payload at +0x000 and the
 * 4-byte payload at +0x010 are touched by that function; the span between
 * is opaque from it alone. */
typedef struct PolyVtx {
    PolyXY8 xy; /* +0x000 */
    u8 pad008[0x010 - 0x008];
    PolyUV4 uv; /* +0x010 */
} PolyVtx;

/* Unaligned struct-field copy helper, code_8220_c (round 13). Takes two
 * 3-element arrays of PolyVtx pointers plus three UV sources. */
extern void FillRVectors3(PolyVtx **dst, PolyVtx **src, PolyUV4 *uv0, PolyUV4 *uv1, PolyUV4 *uv2);

/* Populates a submit table's (`table`, gDivPolygon3/gDivPolygon4):
 * +0x00 an OT/code word (sNdivOverride when sNdivOverrideSet is set, else
 * D_80090C18), +0x04 sDivClipWidth, +0x08 sDivClipHeight -- these three are
 * UNCONDITIONAL (the third rides in the branch's own delay slot in
 * retail); only the two u16 stack args at +0x0C/+0x0E are actually
 * gated on `hasUv1Codes != 0`. +0x10 is an unaligned PolyUV4 copied from
 * `*uv` (same lwl/lwr idiom as FillRVectors3, forced by PolyUV4's
 * alignment-2 all-s16 layout); +0x14 is the plain word at
 * `ctx + 0x30`. MATCHED round 44 after the gp_rel blocker that
 * stalled it at carve time (round 13) was resolved -- see
 * docs/match-reports/FillDivPolygonHeader.md. Declared here so its caller in
 * this unit, SubmitPolyF3, can compile (still INCLUDE_ASM). Parameters
 * named round 77 (alpha): `hasUv1Codes`/`uv1Clut`/`uv1TPage` from the
 * FT3/GT3/FT4/GT4 call sites, which pass 1 plus the primitive's own
 * `+0xE`/`+0x16` or `+0xE`/`+0x1A` fields (POLY_FTn/GTn's CLUT and TPAGE
 * words); F3/G3/F4/G4 pass 0/0/0 and leave +0xC/+0xE untouched. */
extern void FillDivPolygonHeader(void *table, void *ctx, PolyUV4 *uv, s32 hasUv1Codes, u16 uv1Clut,
                                 u16 uv1TPage);

/* Psy-Q SDK (asm/psyq_rcpolyf3.s, not a carved C unit). Called by
 * SubmitPolyF3 (code_8220_c) with (self, table).
 *
 * Every RCpoly* wrapper in code_8220_c returns the next packet pointer:
 * `arg0 + sizeof(prim)` when it splices the primitive into the OT itself,
 * else its RCpoly* callee's own $v0 (a tail call). Round 50 measured this
 * as a lead; round 75 acted on it and matched SubmitPolyF3 and
 * SubmitPolyF4 with it (calls arm first, then the splice arm). The six
 * other wrappers carry the same filler signature. The Sony functions' own
 * return type has no Psy-Q header prototype and stays `void` here, so the
 * matched wrappers call through a local `void *(*)(void *, void *)` cast
 * (code_8220_c.c). */
extern void RCpolyF3(void *self, void *table);

/* Opaque table, referenced only by ADDRESS (never dereferenced in this
 * unit) and handed to FillDivPolygonHeader/RCpolyF3. asm/data, not yet
 * carved -- real element type unknown. */
extern u8 gDivPolygon3[];

/* Quad-flavored sibling of gDivPolygon3/FillDivPolygonHeader/RCpolyF3,
 * referenced the same way by SubmitPolyF4 (code_8220_c, round 13). */
extern u8 gDivPolygon4[];

/* Unaligned struct-field copy helper (quad flavor: 4 fields, not 3).
 * Extends FillRVectors3 to a 4th vertex: forwards elements 0-2 to it
 * unchanged, then does its own dst[3]->xy = src[3]->xy / dst[3]->uv = *uv3
 * (round 20). */
extern void FillRVectors4(PolyVtx **dst, PolyVtx **src, PolyUV4 *uv0, PolyUV4 *uv1, PolyUV4 *uv2,
                          PolyUV4 *uv3);

/* Psy-Q SDK (asm/psyq_rcpolyf4.s, not a carved C unit). Called by
 * SubmitPolyF4 (code_8220_c) with (self, table) -- quad-flavored sibling
 * of RCpolyF3. */
extern void RCpolyF4(void *self, void *table);

/* Psy-Q SDK (asm/psyq_rcpolyg3.s, not a carved C unit). Called by
 * SubmitPolyG3 (code_8220_c) with (self, table) -- Gouraud-shaded
 * sibling of RCpolyF3/RCpolyF4. */
extern void RCpolyG3(void *self, void *table);

/* Psy-Q SDK (asm/psyq_rcpolyg3.s, same file as RCpolyG3, not a
 * carved C unit). Called by SubmitPolyFT3 (code_8220_c) with
 * (self, table). */
extern void RCpolyFT3(void *self, void *table);

/* Psy-Q SDK (asm/psyq_rcpolyg3.s, same file as RCpolyG3/RCpolyFT3,
 * not a carved C unit). Called by SubmitPolyG4 (code_8220_c) with
 * (self, table) -- Gouraud-shaded quad, quad-flavored sibling of
 * RCpolyG3. */
extern void RCpolyG4(void *self, void *table);

/* Psy-Q SDK (asm/psyq_rcpolyft3.s, not a carved C unit). Called by
 * SubmitPolyFT4 (code_8220_c) with (self, table). */
extern void RCpolyFT4(void *self, void *table);

/* Psy-Q SDK (asm/psyq_rcpolygt3.s, not a carved C unit). Called by
 * SubmitPolyGT3 (code_8220_c) with (self, table). */
extern void RCpolyGT3(void *self, void *table);

/* Psy-Q SDK (asm/psyq_rcpolygt3.s, same file as RCpolyGT3, not a
 * carved C unit). Called by SubmitPolyGT4 (code_8220_c) with (self,
 * table) -- Gouraud-shaded quad, quad-flavored sibling of RCpolyGT3. */
extern void RCpolyGT4(void *self, void *table);

#endif
