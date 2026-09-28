# New_VariantSprite -- MATCHED (31/31)

> Renamed from `New_Class879C4` on 2026-09-26 (tools/rename.py). Address 0x80057c94.

> Renamed from `New_D800879C4` on 2026-09-26 (tools/rename.py). Address 0x80057c94.

> Renamed from `func_80057C94` on 2026-09-19 (tools/rename.py). Address 0x80057c94.

Unit: `src/class_3bb8c_k.c`. Class: `DreamSys` family -- plain
allocator/constructor wrapper (`New_X` shape per CLAUDE.md's own
description), not a vtable slot itself.

## Signature

```c
void *New_VariantSprite(void *arg1, void *arg2, void *arg3);
```

## Body

```c
void *New_VariantSprite(void *arg1, void *arg2, void *arg3) {
    void *obj = BMemPMgrAlloc(0xA8);
    if (obj != NULL) {
        GetVariantSpriteMethods()->ctor(obj, arg1, arg2, arg3);
        return obj;
    }
    return NULL;
}
```

Textbook `New_X`: allocate `0xA8` bytes (`BMemPMgrAlloc`), null-check, call
the constructor fetched from a table's `+0x008` slot with `(obj, arg1,
arg2, arg3)`, return the allocation (the ctor's own return value is
discarded).

## `GetVariantSpriteMethods` and `gVariantSpriteMethods` are in UNCARVED ground

`GetVariantSpriteMethods` is a plain no-argument getter (`return &gVariantSpriteMethods;`),
confirmed by reading its body directly in `asm/class_3bb8c_q.s` -- the
still-monolithic segment immediately behind this unit
(`class_3bb8c_k.c`'s own file banner already names it as this unit's
successor, `class_3bb8c_q`). `gVariantSpriteMethods` is a 49-slot table
(`tools/classtable.py` header `0x1F44`) whose `+0x008` slot resolves to
THIS unit's own `VariantSprite__VariantSprite` (still queued at the time this was
written; see its own report). Only the ctor slot is typed here
(`D800879C4Methods`, local to this file) -- the rest of that class is out
of this unit's scope (uncarved, belongs to whoever carves
`class_3bb8c_q`).

## Shape note: `return obj;` INSIDE the `if`, not after it

Two variants were rejected before this one:
- `if (obj == NULL) return NULL; ...; return obj;` (guard clause at top):
  scores 19/31 with the function ONE WORD LONGER -- the early return
  needs an explicit jump to a since-relocated epilogue, because GCC placed
  the shared epilogue differently once the guard clause became the
  function's first statement.
- `if (obj != NULL) { ...; } return obj;` (single trailing return, guard
  wraps the body instead): scores 30/31, ONE instruction wrong -- the
  `beqz`'s delay slot materializes `move v0,s0` (reusing the allocation
  pointer register, which happens to equal 0 on the null path) instead of
  retail's explicit `move v0,zero`. Value-equivalent, register-source
  different.

The version that matches puts `return obj;` INSIDE the `if`, with a
separate `return NULL;` as the function's last statement -- this is the
one shape where GCC materializes an explicit zero for the null path's
early exit (reached by a direct branch to the shared epilogue, no jump
needed) while the success path's `return obj;` is a distinct, later `move
v0,s0`.

## Naming

**`New_VariantSprite` -- tier A.** Textbook `New_X` shape per CLAUDE.md's own
convention ("constructors `New_Class`/`Class__Class`"): allocate, null
check, construct, return -- mechanics ARE the purpose. `D800879C4` names
the class this allocates (its own ctor's table, `gVariantSpriteMethods`, with the
underscore dropped per this project's `D800878D4Methods`-style convention
for an as-yet-unnamed class, since the class itself lives in uncarved
ground `class_3bb8c_q.s` this runner cannot rename).

## Verify

```
./build-and-verify.sh   # build exit=0, OK: build matches retail
tools/funcdiff.py New_VariantSprite   # 31/31
```

### Proposed learning

**For a "return obj on success, return NULL on failure" allocator
wrapper, put `return obj;` INSIDE the success `if`-block and a bare
`return NULL;` as the function's trailing statement -- not a guard clause
at the top, and not a single shared `return obj;` after the `if`.** Three
control-flow shapes were tried for the exact same logic:
1. Guard clause (`if (!obj) return NULL;` then unconditional body then
   trailing `return obj;`) grew the function by one word: an extra jump
   appears because the early return no longer lands on the epilogue GCC
   ends up placing.
2. Wrapped body with a single trailing `return obj;` reused the
   allocation-pointer register (`s0`) instead of materializing a fresh
   zero for the null path's branch-delay slot -- value-correct,
   register-source wrong.
3. `if (obj) { ...; return obj; } return NULL;` matched exactly.

This is the opposite of the existing "write an early exit as an inverted
guard clause" learning in `docs/DECOMPILATION_LEARNINGS.md` -- that one is
about `if (cond) { lots } else { return k; }` vs `if (!cond) return k;`
for a DIFFERENT function shape (the trailing code is the "success" body,
not a redundant return of an already-computed pointer). Read both: which
one applies depends on whether the tail after the guard is itself a bare
`return` of a value already computed before the branch (this case) or
further work (that one).

## Track 4 (2026-09-26, round 87, alpha)

Renamed from `New_D800879C4` (tools/rename.py); the class is `VariantSprite`
(`include/VariantSprite.h`). The prototype is now
`VariantSprite *New_VariantSprite(s32 variant, void *arg2, void *texture)`: the
three arguments are forwarded unchanged to the ctor, whose parameters they
name (see `VariantSprite__VariantSprite`'s report), and the result is the object.
The body is unchanged but for the parameter names; the one caller outside
this unit, `StyleEffect__SpawnSprites` (class_3bb8c_k.c), passes its `a2`
as the variant without a cast and casts the result to its `LinkNode *`
view, and its local `extern` of this function is gone. The "UNCARVED
ground" section above is history: the getter and table are
`GetVariantSpriteMethods` / `gVariantSpriteMethods` in class_3bb8c_t.c.
Byte-identical.

## Track 6 (2026-09-26, round 93, bravo)

The class `Class879C4` is now `VariantSprite` (`include/VariantSprite.h`,
`python3 tools/renametype.py Class879C4 VariantSprite`), tier B: the
mechanics are certain and are the whole of what the class adds to Sprite --
`variant` (0 or 1) picks the texture cell the Sprite ctor binds
(`gVariantSpriteCells`) and the CLUT row the reset slot sets
(`gVariantSpriteClutX/Y`). What the sprites are in the game is not
established (their only builder is StyleEffect, kinds 2 and 3, and every
path passes variant 0), which is why it is not tier A. The table, getter,
allocator, methods and the three data tables followed the class name.
The same tool run rewrote `Class879C4` tokens inside this report's older
history prose (the known renametype behaviour pending an operator
decision); those lines were left as the tool wrote them.

Its only caller is `StyleEffect__SpawnSprites`, with variant 0.

## Track 7 (2026-09-27, round 96, bravo)

Comments quoted below are verbatim as the file stood before this round's
comment pass, i.e. with this round's renames already applied (the
`LinkQueryBuf` one as it stood before step 2).

- `BMemPMgrAlloc(0xA8)` became `sizeof(VariantSprite)` (0xA8,
  VariantSprite.h), byte-identical.
- Parameter `arg2` -> `resetArg` (tier B), here, in the ctor and in
  VariantSprite.h's prototypes and CtorParams: it is forwarded to Sprite's
  ctor as its `arg4`, which Sprite__Sprite hands on to reset, and
  Sprite__Reset does not read it; the one caller (StyleEffect__SpawnSprites)
  passes 0.

The function comment, verbatim:

```c
/* VariantSprite (include/VariantSprite.h, track 4, round 87): its allocator and
 * ctor. The other methods are in class_3bb8c_q.c and class_3bb8c_t.c. */
```
