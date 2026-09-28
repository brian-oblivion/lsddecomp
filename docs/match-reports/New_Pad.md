# New_Pad

> Renamed from `func_80025B34` on 2026-09-24 (tools/rename.py). Address 0x80025b34.

**Unit:** Pad · **Size:** 27 words (0x6C bytes) · **Status:** MATCHED
(byte-exact, whole-image `./build-and-verify.sh` green) · Worked by the head in
round 2026-08-30-a.

## What it does

The `New_X` allocator for the Pad class (`gPadMethods`, resolved with
`tools/classtable.py 0x8006D370 --vs 0x8006B58C`). Allocates a 0x20-byte
instance through the game allocator `BMemPMgrAlloc`, and on success calls the
class's own constructor — slot `+0x08` of the table returned by
`GetPadMethods()` — with `(self, arg1, port)`. Returns the instance, or NULL if
the allocation failed.

```c
Pad *New_Pad(void *arg1, s32 port) {
    Pad *self;

    self = BMemPMgrAlloc(0x20);
    if (self == NULL) {
        goto fail;
    }
    GetPadMethods()->ctor(self, arg1, port);
    return self;
fail:
    return NULL;
}
```

## The discriminator: `goto fail` vs `return NULL`

This is the finding worth carrying, because the obvious spelling is one word
too long and *nothing about the residue says so*.

Retail's shape is a single shared epilogue reached two ways, with the NULL
result sunk into the branch's own delay slot:

```
beqz  $s0, .L80025B84
 addu $v0, $zero, $zero      ; delay slot: result = NULL
jal   GetPadMethods
 nop
addu  $a0, $s0, $zero
lw    $v0, 0x8($v0)
addu  $a1, $s1, $zero
jalr  $v0
 addu $a2, $s2, $zero
addu  $v0, $s0, $zero        ; result = self
.L80025B84:                  ; single epilogue
```

Four source shapes were built and scored against that. All four are
control-flow-equivalent; only one is byte-exact.

| shape | words | result |
| --- | --- | --- |
| `if (self == NULL) { return NULL; } ...; return self;` | 28 | 16/27, image red |
| `result = NULL; if (self != NULL) { ctor(); result = self; } return result;` | 30 | 0/27, image red — the extra live variable costs a fourth saved register (`$s3`) |
| `return self != NULL ? (ctor(...), self) : NULL;` | 28 | 16/27, image red |
| **`if (self == NULL) { goto fail; } ...; return self; fail: return NULL;`** | **27** | **27/27, image GREEN** |

The two 28-word forms fail the same way, and it is instructive: GCC 2.6.3 emits
the NULL-return as its own basic block placed *after* the constructor path,
leaves the `beqz` delay slot as `nop`, and then needs a `j` to reach the shared
epilogue from the constructor path — one extra word. Written with an explicit
`goto`, the same CFG comes out with the NULL assignment stolen into the delay
slot and the branch retargeted straight at the epilogue, which is retail
exactly.

So the residue for the wrong shapes is an extra `j` plus a `nop`, and it reads
like a delay-slot scheduling artifact — the kind of thing the guide's residue
list says to try `__asm__("")` against. It is not. It is a block-placement
difference driven by the source's spelling of the early exit, and the lever is
`goto`.

## Relationship to `New_GameApplication`

`docs/match-reports/New_GameApplication.md` is a stall on the *other* `New_X`
variant — the one that returns the allocation unconditionally, where retail
leaves the `beqz` delay slot as a literal `nop` and every shape tried compiled
`move $v0, $s0` into it. That report tried `goto`, but in the
`goto done; ... done: return self;` spelling, i.e. jumping to a shared
`return self`. It did not try the shape that worked here, because here the two
paths return *different* values.

The two are therefore distinct sub-shapes and the fix here does not
automatically transfer. What does transfer is the general lever: **for a
GCC 2.6.3 early exit, `goto` and `return` are not interchangeable at the byte
level**, so on any residue that is an extra `j`/`nop` pair around a null check,
try both spellings before classifying it as a scheduling stall.

## Notes on the header

`include/Pad.h` previously declared `extern void *BMemPMgrAlloc(s32
size, s32 zone);`. The call site sets only `$a0`, never `$a1` — and `$a1` still
holds the incoming `port` at that point, so a genuine second argument would
have had to be set up explicitly. Corrected to one parameter. The two-parameter
form was a hypothesis written while every `New_X` in the tree was still
`INCLUDE_ASM`, so no compiled code had ever exercised it.

### Proposed learning

For a GCC 2.6.3 early exit that returns a *different* value from the main path,
`goto fail; ... fail: return OTHER;` and `return OTHER;` compile differently:
the `return` spelling costs an extra `j` and leaves the branch delay slot as
`nop`, the `goto` spelling sinks the exit value into the delay slot and reaches
one shared epilogue. Try both before spending attempts on scheduling barriers.

## Naming

**Tier A.** `New_X` allocator+ctor-wrapper shape (matches `New_GameApplication`,
`New_DreamSys`, `New_TodActor`, etc. project-wide): allocates the instance
through `BMemPMgrAlloc`, then calls the class's own ctor slot through
`GetPadMethods()`. `Pad` is the class name established for this whole unit
(see `include/Pad.h`'s header comment, confirmed by the direct
`PadInit`/`PadRead`/`PadStop` calls in `Pad__Pad`/`Pad__Finalize`/
`Pad__UpdateMasks`). Only caller: `src/main.c`'s `New_Pad(0, 0)`.

## Round 95 (delta): Sony's declarations

`include/Pad.h` now takes `PadInit`/`PadRead`/`PadStop` from Sony's
`<libetc.h>` instead of local prototypes (`PadInit(void *)` became Sony's
`PadInit(int mode)`). The ctor's first parameter forwards straight to
`PadInit`, so `New_Pad`, `Pad__Pad` and the Pad ctor slot now take
`s32 mode` where the body above says `void *arg1`. Byte-identical; whole-image
SHA1 unchanged.

## History (moved from src/Pad.c, comments pass)

The file's banner carried its edge evidence:

> Edges: Sony objects on both sides, libc2/puts before and libetc/pad
> after, so the file is exactly this unit. tuboundary.py finds no rodata
> crossing; its "a forced boundary lies in this stretch" note is satisfied
> by those Sony edges. Named for its class.
