# func_8003B9DC

**Unit:** code_2c054 · **Size:** 23 instructions · **Status:** MATCHED (23/23 words)

## What it does

StreamTask's dtor override (method-table slot `+0x00C`). Ticks the
sub-object referenced from `self->unkB4` (a class instance of its own,
method table at its own offset 0; only its slot `+0x004`, a self-only
call, is dispatched here), then chains into the base task class's own
dtor via `func_8003DFBC()->dtor(self)` -- the same "ownDtorChain" shape
already documented in `include/code_171e0.h` (`func_800269F0`).

## The C

```c
void *func_8003B9DC(StreamTask *self) {
    self->unkB4->methods->slot4(self->unkB4);
    return func_8003DFBC()->dtor(self);
}
```

## How it was found

Raw disassembly: `lw $a0, 0xB4($s0)` then two `lw`s to resolve
`a0->methods->slot4` and a `jalr` with `a0` (the sub-object) still
current, followed by `jal func_8003DFBC` (whose own argument is ignored),
`lw $v0, 0xC($v0)` and a final `jalr $v0` with `$a0` explicitly reset to
`self`. `classtable.py D_8006E5F8` places this function at slot `+0x00C`
(the dtor slot, by the class-framework convention documented in
CLAUDE.md); `classtable.py D_8006E730` shows the base class's own dtor at
the same offset is `func_8003C008`.

**Return type is a guess, not evidence.** `void *` was chosen to match
BasicClass's own dtor convention (`void *(*dtor)(void *self)`, per
`include/code_171e0.h`'s `BasicClassMethods171e0`), not because this
function's own disassembly or callers confirm it.

## Provenance

round 2026-09-01, runner alpha, unit code_2c054 (unit's first pass).
