# func_80056BBC -- STALL (86/87 words, one-instruction dead-store residue)

Unit `class_3bb8c_s`. `self` is the owning `LinkNode`. The carve-time census
flagged this function's only screen hit as `addiu_at`, which was RESOLVED in
round 21 (see `docs/research/addiu-at-blocker.md`) -- it is NOT why this
stalled. This is a genuine residue, not a toolchain blocker.

## Classification

A rand()-driven message/attach dispatcher over `self->arr84[1]`/`arr84[2]`,
plus a call into the still-`gp_rel`-blocked `func_80056D18` (left
`INCLUDE_ASM`, not edited) with a table pointer picked by a `rand() % 2`
parity roll used again later for a second table choice.

Two real defects were found and fixed during this attempt (both by
comparing `tools/asm-differ/diff.py` output against retail instruction by
instruction):

1. **Scheduling: the "reload child->methods" needs to happen BEFORE the
   branch, not after.** The first draft loaded `child->methods` fresh at the
   point of the `slotB8` call (after the branch merge), leaving the earlier
   `bnez self->unk78` branch's load-delay slot (after `lw a1,0x78(s1)`) with
   nothing useful to fill -> a real `nop`, plus a second, separate reload of
   `child->methods` later where retail reused the one register. Explicitly
   caching `LinkNodeMethods *m = child->methods;` right after the
   `func_800573A8` call (BEFORE computing the `arg` ternary that gates the
   branch) let the compiler's own scheduler hoist that load into the earlier
   delay slot exactly as retail does. This closed 2 of the then-3 residual
   words (from a base score of 705 down to 265 on the permuter's own
   scorer, `--stack-diffs`).
2. **Ternary branch sense, and dropping a shared temp for a tail call.** The
   `(parity == 0) ? D_80087874 : D_8008785C` selector had the branch sense
   backwards relative to retail's default/override shape (retail defaults to
   `D_80087874` and overrides to `D_8008785C` only when `parity != 0` --
   flipping the ternary's arm order to `(parity != 0) ? D_8008785C :
   D_80087874` fixed the branch AND its target address, both of which showed
   as diffs before this). Separately, holding `self->arr84[2]` in a shared
   `child` local for the final `slot60` call put it in a saved register
   (`$s0`) retail does not use there (retail uses `$a0`/`$v0` directly, since
   nothing else needs that value preserved past a call) -- inlining
   `self->arr84[2]->methods->slot60(self->arr84[2], 0)` directly (no local)
   fixed the register identity and an insertion/deletion pair the permuter's
   scorer flagged.

## Residue

After both fixes the ONLY remaining diff is a single retail instruction this
C does not produce:

```
/* 4741C 8005671C 01000534 */   ori  $a1, $zero, 0x1
```

This sits as the delay-slot filler for the branch testing `self->unk70 < 2`
(`bnez v0, .L80056C78` at `0x80056C18`), and it is NEVER READ on either the
branch-taken path (`func_80056BBC`'s "< 2" body, which sets `$a1` itself
before its own uses) or the fallthrough path (the ">= 2" body, which
overwrites `$a1` with `D_80087880`'s address in the very next instruction).
It is a genuine dead store in retail's own compiled output, matching
`docs/MATCHING-GUIDE.md`'s documented "redundant move" residue class
verbatim (same symptom as `new_class_6d3c8`/`strcat`/the round-16 third
instance).

Confirmed with `tools/asm-differ/diff.py func_80056BBC`: every other
instruction in the function lines up 1:1 with retail once this one line is
accounted for; everything downstream of it in the diff differs only by the
constant 4-byte address shift this one missing instruction causes (each
`%lo`/`jal` immediate off by exactly 4), not by any independent defect.

## Permuter

Set up per `docs/MATCHING-GUIDE.md`'s permuter section
(`tools/setup-permuter.sh func_80056BBC <seed>`). Base score with
`--stack-diffs`: **265** (1 register difference, 1 reordering, 0 insertions,
2 deletions -- consistent with exactly one missing instruction). Ran
`-j 6 --stop-on-zero --best-only` for ~8000 iterations (60s wall-clock
timeout) without finding a zero -- unlike the three already-documented
instances of this class, which closed in under 400 iterations each. This one
did not reproduce inside the attempt budget; **not asserting it is
permuter-exhausted**, just that it did not close in this session. A longer
unattended run (`--stop-on-zero`, no wall-clock cap) is the natural next
step if someone picks this back up.

## Best-reached body (restored to `INCLUDE_ASM`, not left in `src/`)

```c
#if 0
typedef struct LinkNode LinkNode;

typedef struct LinkNodeMethods {
    u8 pad0[0x44];
    void (*slot44)(LinkNode *self, s32 flag, s32 val);
    void (*slot48)(LinkNode *self, s32 flag, void *arg);
    void (*slot4C)(LinkNode *self, void *arg1, void *arg2);
    u8 pad50[0x60 - 0x50];
    void (*slot60)(LinkNode *self, s32 arg1);
    void (*slot64)(LinkNode *self);
    void (*slot68)(LinkNode *self, s32 arg1);
    u8 pad6C[0xB8 - 0x6C];
    void (*slotB8)(LinkNode *self, void *arg1);
} LinkNodeMethods;

struct LinkNode {
    LinkNodeMethods *methods;
    u8 pad4[0x20 - 0x4];
    s32 unk20;
    u8 pad24[0x54 - 0x24];
    s32 unk54;
    u8 pad58[0x64 - 0x58];
    s32 unk64;
    void *unk68;
    s32 unk6C;
    s32 unk70;
    void *unk74;
    void *unk78;
    LinkNode *arr7C[2];
    LinkNode *arr84[5];
};

typedef struct Vec3S {
    s32 x, y, z;
} Vec3S;

extern s32 rand(void);
extern void func_80056D18(void *self, s32 arg1, s32 arg2, void *arg3);
extern void func_800573A8(void *self, Vec3S *arg1);
extern s32 D_80087844[];
extern s32 D_8008785C[];
extern s32 D_80087868[];
extern s32 D_80087874[];
extern Vec3S D_80087880;

/* 86/87 words -- one dead-store residue, see the classification above. */
void func_80056BBC(LinkNode *self) {
    s32 parity = rand() % 2;
    void *tblOrNull = parity ? NULL : D_80087868;
    s32 one = 1;
    LinkNode *child;
    void *arg;

    func_80056D18(self, 0, 0, tblOrNull);

    if (self->unk70 >= 2) {
        LinkNodeMethods *m;

        child = self->arr84[1];
        D_80087880.x = D_80087844[self->unk70];
        func_800573A8(child, &D_80087880);
        m = child->methods;
        arg = (self->unk78 != NULL) ? self->unk78 : self->unk74;
        m->slotB8(child, arg);
    } else {
        child = self->arr84[1];
        child->methods->slot64(child);
        child->methods->slot68(child, 0);
        child->methods->slot48(child, one, (parity != 0) ? D_8008785C : D_80087874);
    }

    self->arr84[2]->methods->slot60(self->arr84[2], 0);
}
#endif
```

### Proposed learning

**The "reload a shared value before a branch so it can fill an earlier
branch's load-delay slot" lever is real and mechanical, not luck.** Moving an
independent load (here, `child->methods`) to an earlier statement position --
BEFORE a conditional expression that does not depend on it -- let GCC 2.6.3's
own scheduler hoist it into a load-delay slot one branch earlier, closing two
words in one edit. This is a source-level, non-register-pinning lever (pure
statement reordering) worth reaching for before suspecting a residue is
unreachable: check whether an unrelated load sitting right after your
function's near-miss point could instead be moved earlier to fill a delay
slot that currently holds a `nop`.
