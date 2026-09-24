# Entity__MoodCue22

> Renamed from `func_8005F1A8` on 2026-09-24 (tools/rename.py). Address 0x8005f1a8.

**Unit:** Entity_c · **Size:** 11 words · **Status:** MATCHED (11/11 words,
whole-image build verified byte-exact)

## What it does

`(Entity *this) -> void`. A one-line wrapper: `Class6B5CC__FaceTarget(this,
this->unk94, 1, 0, 0);` -- the exact same call shape `Entity__MoodCue01`/
`Entity__MoodCue12`/`Entity__MoodCue11` (all in `Entity_b.c`) already established.

## Final C

```c
void Entity__MoodCue22(Entity *this) {
    Class6B5CC__FaceTarget(this, this->unk94, 1, 0, 0);
}
```

## Attempt log

Matched on the first attempt. This is the first function in `Entity_c.c`;
added `#include "Entity.h"` to the top of the file (it previously only
included `common.h`, since every function was still `INCLUDE_ASM`).

## Proposed learning

None new -- straightforward reuse of an already-established call shape from
`Entity_b`.

## Naming

Why `MoodCueNN`: the function's address sits in `gEntityMoodHandlerTable` row 22 (`asm/data/79528.data.s`, base 0x80089EB0, stride 0x10; row = (slot address - 0x80089EB0) / 0x10), read directly off the table (this unit's own row assignment, round 78). Tier B: the row-to-function mapping is a compiler fact, not a guess, but which dream object or mood state each row represents is not established -- the row number is kept decimal and zero-padded so the names sort in table order, same convention as Entity_b/d/e/g.
