# AcceptGridElem -- MATCHED (15/15)

> Renamed from `DreamSys__AcceptGridElem` on 2026-09-25 (tools/rename.py). Address 0x80057b54.

> Renamed from `func_80057B54` on 2026-09-19 (tools/rename.py). Address 0x80057b54.

Unit: `src/class_3bb8c_p.c`. Class: `DreamSys` family (called only from
this unit's own `Actor__ScanGridWindow`, still queued at time of writing, twice, as
`AcceptGridElem(elemOrHead, arg1, arg2)`). Not itself a vtable slot in any
of the class tables reachable from this unit's addresses.

## Signature

```c
void *AcceptGridElem(void *arg0, void *arg1, void *arg2);
```

`arg1`/`arg2` are never read by the body -- present only to match the
caller's 3-argument calling convention.

## Body

```c
void *AcceptGridElem(void *arg0, void *arg1, void *arg2) {
    if (arg0 != NULL) {
        if (SceneNode__RaycastVertical() != 0) {
            return arg0;
        }
    }
    return NULL;
}
```

`SceneNode__RaycastVertical` is declared locally (`extern s32 SceneNode__RaycastVertical(void);`) --
it is matched/queued in a different unit (`src/SceneNode.c`), so its
prototype belongs here, not in a shared header.

## Shape note

Written as NESTED `if`s rather than `if (arg0 != NULL && SceneNode__RaycastVertical())`.
The combined-condition form compiles to an extra `move v1,v0` before the
`bnez` and shifts the whole function 4 bytes long (a spurious second
`return NULL;` merge point reached via a different control path than
retail's, which falls through to the shared `move v0,zero` from the INNER
check only). Confirmed by direct comparison: combined form gives 6/15
words with the size drift warning; nested form gives 15/15 exactly.

## Naming

**`AcceptGridElem` -- tier A.** A pure predicate/leaf: returns
`arg0` unchanged if it is non-NULL AND the global gate `SceneNode__RaycastVertical()`
is non-zero, else `NULL`. Mechanics ARE the purpose here (an accept/reject
test), matching CLAUDE.md's tier-A carve-out for "a pure leaf whose
mechanics ARE its purpose (a getter, a clamp, a list push)". `arg1`/`arg2`
are not named by this name, but they are NOT dead: see the round-57 head
correction below.

## Verify

```
./build-and-verify.sh   # build exit=0, OK: build matches retail
tools/funcdiff.py AcceptGridElem   # 15/15
```

### Proposed learning

**A guard clause folded into `&&` with the following check is not always
equivalent to writing it as a nested `if`, even when both are logically a
single early return.** `if (a && b) return x; return y;` and
`if (a) { if (b) return x; } return y;` can compile to different merge
points for the "no" path -- one funnels through an extra register copy to
reach a single shared zero-setter, the other lets the inner check fall
through directly. When a combined condition scores short by exactly a
handful of words with everything downstream shifted, try splitting it into
nested `if`s before suspecting anything else.


## Round 57 (head, at merge) -- the prototype was wrong and the oracle could not see it

This unit declared `extern s32 SceneNode__RaycastVertical(void);` and called it with no
arguments. In the SAME round, `SceneNode` matched `SceneNode__RaycastVertical` and
established its real signature: `s32 SceneNode__RaycastVertical(SceneNodeObj *self,
s32 *arg1, s32 *arg2)`, a SceneNode method. The two readings met at merge.

`arg1`/`arg2` were therefore never "unused parameters that exist to match
the calling convention" -- they are FORWARDED. `AcceptGridElem(arg0, arg1,
arg2)` receives them in `$a0`-`$a2` and `SceneNode__RaycastVertical` reads them from the
same registers, so the zero-argument call compiled byte-identically while
saying something false about the code. Rewritten as
`SceneNode__RaycastVertical(arg0, arg1, arg2)` against the real prototype: **byte-exact,
whole-image SHA1 green**, so this is a readability fix with no codegen
component.

**Why it matters beyond this function.** This is the failure mode
FINISHING-PLAN track 2 names for SDK identification -- "the byte oracle
cannot see a wrong signature" -- occurring in ordinary game code, between
two units, and surviving because the callee was `INCLUDE_ASM` (no C
prototype existed to conflict with) until round 57. A wrong `extern` in one
unit is invisible to every check this project runs until some other unit
declares the same symbol correctly and the two land in one translation
unit as `conflicting types`.

### Proposed learning

**When a function leaves the queue by MATCHING, its new C signature is
evidence about every OTHER unit that declares it `extern`.** Those `extern`
lines were written against disassembly, with no prototype to check them,
and a wrong one is byte-invisible whenever the arguments already sit in the
right registers -- which is exactly the common case for a forwarding leaf.
Discriminator: a local `extern ... (void)` (or any arity) for a symbol some
other unit now DEFINES. Sweep with
`grep -rn 'extern .*func_' src/` against the matched set after any round
that closes functions. (a docs/match-reports/AcceptGridElem.md
and docs/match-reports/SceneNode__RaycastVertical.md, round 57)

## Track 4 (2026-09-25, round 82, delta)

Renamed from `DreamSys__AcceptGridElem`. Helper of Actor__ScanGridWindow. No self parameter, so no class prefix. The class (id 0x34, table `gActorMethods`, formerly `D_800878D4`) is unified as `Actor` in `include/Actor.h`: a SceneNode subclass and the base of TodActor/Entity, DreamSys and StyleEffect. Any source block above is the pre-unification spelling; the live body in `src/class_3bb8c_p.c` takes the unified types and field/slot names, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 7 (2026-09-27, round 96, bravo)

Comments quoted below are verbatim as the file stood before this round's
comment pass, i.e. with this round's renames already applied (the
`LinkQueryBuf` one as it stood before step 2).

- Parameters `arg0`/`arg1`/`arg2` -> `cell`/`offset`/`pos`, and the local
  extern of SceneNode__RaycastVertical names its `out`/`target` the same
  way: it writes the hit less the ray's start into `offset`, and `pos` is
  the actor's `coord2->tx` (Actor__FindNearbyLink). Types unchanged.

The extern's comment, verbatim:

```c
/* The real signature, established when SceneNode matched this function in
 * round 57: it is a SceneNode method taking (self, out, target). This unit
 * had long declared it `(void)` and called it with no arguments, which is
 * byte-identical here only because arg0-arg2 are already in $a0-$a2 -- the
 * byte oracle cannot see a wrong prototype. Spelled out so the forwarding is
 * visible; verified byte-exact. */
```
