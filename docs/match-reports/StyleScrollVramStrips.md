# StyleScrollVramStrips -- MATCHED, round 46 (2026-09-15)

> Renamed from `DrawStyleTables` on 2026-09-26 (tools/rename.py). Address 0x80055a24.

> Renamed from `func_80055A24` on 2026-09-23 (tools/rename.py). Address 0x80055a24.

Unit `ObjMStyleActor`. **25/25 words, byte-exact.** Second build (one lever).

## What it was

Fresh ground, carved round 45, never attempted. No blockers.

## Derivation

```
/* 46224 80055A24 E8FFBD27 */  addiu $sp, $sp, -0x18
/* 46228 80055A28 6404838F */  lw    $v1, %gp_rel(sStyleStage)($gp)
/* 4622C 80055A2C 02000234 */  ori   $v0, $zero, 0x2
/* 46230 80055A30 07006214 */  bne   $v1, $v0, .L80055A50
/* 46238 80055A38 0880043C */  lui   $a0, %hi(gStyleStripRectA)
/* 4623C 80055A3C 44748424 */  addiu $a0, $a0, %lo(gStyleStripRectA)
/* 46240 80055A40 0880063C */  lui   $a2, %hi(gStyleStripScratchA)
/* 46244 80055A44 5074C624 */  addiu $a2, $a2, %lo(gStyleStripScratchA)
/* 46248 80055A48 9C560108 */  j     .L80055A70
/* 4624C 80055A4C 01000534 */   ori  $a1, $zero, 0x1
.L80055A50:
/* 46250 80055A50 FDFF6224 */  addiu $v0, $v1, -0x3
/* 46254 80055A54 0300422C */  sltiu $v0, $v0, 0x3
/* 46258 80055A58 07004010 */  beqz  $v0, .L80055A78
/* 4625C 80055A5C 01000534 */   ori  $a1, $zero, 0x1
/* 46260 80055A60 0880043C */  lui   $a0, %hi(gStyleStripRectB)
/* 46264 80055A64 5C748424 */  addiu $a0, $a0, %lo(gStyleStripRectB)
/* 46268 80055A68 0880063C */  lui   $a2, %hi(gStyleStripScratchB)
/* 4626C 80055A6C 6874C624 */  addiu $a2, $a2, %lo(gStyleStripScratchB)
.L80055A70:
/* 46270 80055A70 89ED000C */  jal   RotateVramRectRight
.L80055A78:
...
jr $ra
```

`sStyleStage` is a plain `s32` (already established as such in
`ObjMStyleActor.c`, `RegisterStyleConfig`). `(sStyleStage - 3)` cast to unsigned and
compared `< 3` is the standard idiom for a closed range test, matching
retail's `sltiu` exactly. `RotateVramRectRight` is a not-yet-carved,
still-`INCLUDE_ASM` function in `asm/psyq_2bb9c.s` (a 4-argument draw-style
routine reading 12-byte tuples through `a0`/`a2` a variable number of times
via `a1`); this call always passes `a1 = 1`, so only the calling shape
matters here, declared loosely as `void RotateVramRectRight(void *arg0, s32 arg1,
void *arg2);`.

```c
extern s32 sStyleStage;
extern void RotateVramRectRight(void *arg0, s32 arg1, void *arg2);
extern s32 gStyleStripRectA[];
extern s32 gStyleStripScratchA[];
extern s32 gStyleStripRectB[];
extern s32 gStyleStripScratchB[];

void StyleScrollVramStrips(void) {
    void *a0, *a2;
    s32 a1;

    if (sStyleStage == 2) {
        a0 = gStyleStripRectA;
        a2 = gStyleStripScratchA;
        a1 = 1;
    } else if ((u32) (sStyleStage - 3) < 3) {
        a1 = 1;
        a0 = gStyleStripRectB;
        a2 = gStyleStripScratchB;
    } else {
        return;
    }
    RotateVramRectRight(a0, a1, a2);
}
```

**First attempt passed `1` as a literal straight into the call
(`RotateVramRectRight(a0, 1, a2);`) and missed by 3 words**, all three the SAME
kind of diff: the literal got materialized once, right at the call site
(the `jal`'s own delay slot), where retail instead sets it inside EACH
branch, in each branch's own tail instruction (the unconditional `j`'s delay
slot in the first branch, the `beqz`'s delay slot in the second).
Introducing an explicit `s32 a1;` local and assigning `a1 = 1;` inside each
branch (matching each branch's own internal order -- last in the first
branch, first in the second) reproduced retail's scheduling exactly on the
next build.

### Proposed learning

**A call argument that is the SAME literal constant on every path is not
evidence that the source passes it as a literal at the call site.** When a
literal-at-call-site attempt places the constant load in one spot but
retail has it duplicated inside each branch (at each branch's own tail
instruction), replacing the literal with an explicit local variable assigned
inside each branch is the fix -- it lets the scheduler place each
branch's `ori` where that branch's own tail already sits, rather than
collapsing all paths' constant into one shared load ahead of the merged
call.

## Naming

**`StyleScrollVramStrips`, tier B.**

Selects one of two 12-byte-tuple table pairs by `sStyleStage` (`== 2`, or
`3..5`) and forwards them to `RotateVramRectRight`, a not-yet-carved routine this
unit's OWN header comment (round 45) already characterizes as "a 4-argument
draw-style routine reading 12-byte tuples through a0/a2" -- that
characterization, on file before this naming pass, is the evidence for
"Draw" rather than a guess made now. Called as the last step of `TickStyle`
every frame. MATCHED, 25/25, second build (one lever: hoist the shared `a1
= 1` literal into each branch).

## Round 93 polish (delta, track 7)

### Naming

**Renamed from `DrawStyleTables`, tier B.** The callee, RotateVramRectRight (TimImage.c), does three DrawSystem moveImage calls per step: the rect's rightmost column to the scratch point, the rest one pixel right, the scratch column back to the rect's left edge -- a one-column rotation of a VRAM rectangle. This function does one step per tick on stage 2 (rect 0,496 248x8 via scratch 256,496) and stages 3 to 5 (rect 0,504 via 256,504). Drawing is not what it does; the "12-byte tuple" reading is `DrawRect`. What the strips hold is not established.

| old | new | tier | evidence |
| --- | --- | --- | --- |
| `D_80087444`/`D_80087450` | `gStyleStripRectA`/`gStyleStripScratchA` | A | DrawRect {0, 0x1F0, 248, 8} and {256, 0x1F0, 1, 8}: RotateVramRectRight's rect and scratch for stage 2. |
| `D_8008745C`/`D_80087468` | `gStyleStripRectB`/`gStyleStripScratchB` | A | the same at y 0x1F8, for stages 3..5. |

Locals `rect`, `count`, `scratch`.
