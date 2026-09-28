# ApplyMatrixLV — extern-arity finding (round 59)

Not a match attempt. `ApplyMatrixLV` is Sony's libgte routine at `0x80015618`,
linked from an SDK object; no C is ever written for it. This file exists only
to record the round-59 extern review.

## Extern arity (round 59)

**Verdict: arity-ok idiom.** Both declarations are correct about their own
translation unit and neither may be changed.

**Callee evidence** (`objdump -d build/lsdde.elf`, `0x80015618`): the body
loads the matrix through `$a0` (`lw t0..t4, 0/4/8/12/16(a0)` into `ctc2
$0..$4`), the input vector through `$a1` (`lw t0,0(a1)`, `lw t1,4(a1)`,
`lw t2,8(a1)`), and writes the result through `$a2`. Three real argument
registers, all read before written — the standard
`ApplyMatrixLV(MATRIX *m, VECTOR *v0, VECTOR *v1)` signature.

**Why the two declarations disagree.** `include/class_3bb8c.h`'s 3-parameter
prototype is the true signature; `class_39e08.c:105` calls it with three
arguments. `include/SceneNode.h`'s `extern void ApplyMatrixLV();` is round
19's *deliberate* unprototyped form: `ApplyMatrixToLVArray` (src/SceneNode.c)
must contain both a live 3-argument call and an unreachable
`if (0) { ApplyMatrixLV(m, src, dst, 0, 0, 0); }`, because GCC 2.6.3 sizes the
outgoing-argument area from every call expression's argument count during RTL
expansion, before dead-branch elimination — that 6-argument call is what
produces retail's 24-byte reservation. An ANSI prototype in that unit would
make the 6-argument call a compile error. Full derivation in
`docs/match-reports/ApplyMatrixToLVArray.md`.

So the "conflict" is not a disagreement about the callee at all: one
declaration states the signature, the other deliberately states nothing.

**Declaration sites changed:** none of the arities. `/* arity-ok: ... */`
markers added to

- `include/class_3bb8c.h` — `extern void ApplyMatrixLV(QueryTemplate866E8 *arg0, s32 *arg1, s32 *arg2);`
- `include/SceneNode.h` — `extern void ApplyMatrixLV();`

Oracle green (`build exit=0`, `OK: build matches retail`) after the edit; a
comment moves zero bytes.

## Round 94 (track 6)

`include/class_3bb8c.h`'s prototype is gone: its one caller,
`StageMap__ComputeFootprintFromRotation` (src/class_39e08.c), takes Sony's
`VECTOR *ApplyMatrixLV(MATRIX *m, VECTOR *v0, VECTOR *v1)` from `<libgte.h>`.
