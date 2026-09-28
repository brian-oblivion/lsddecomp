# GetGameApplicationMethods

> Renamed from `GetClass6D3C8Methods` on 2026-09-26 (tools/rename.py). Address 0x800269e0.

> Renamed from `func_800269E0` on 2026-09-18 (tools/rename.py). Address 0x800269e0.

**Unit:** game_shell · **Size:** 4 instructions · **Status:** MATCHED (4/4 words)

## What it does

Returns the address of `gGameApplicationMethods`, a 25-slot hand-rolled-class method table
(per `tools/classtable.py`, header word `0x00001F60`). It is a "get the method
table" accessor, the same shape as `GetDreamSysMethods` in `include/dream_sys.h`.

## Derivation

```
lui   $v0, %hi(gGameApplicationMethods)
jr    $ra
 addiu $v0, $v0, %lo(gGameApplicationMethods)
```

Just an address computation, no load — this is `&gGameApplicationMethods`, not
`*gGameApplicationMethods`. Confirmed by its one caller, `New_GameApplication` in
`asm/nonmatchings/game_shell/New_GameApplication.s`: it calls this function, then
does `lw $v0, 0x8($v0)` on the result and `jalr`s that — fetching the
constructor slot (`+0x008`, `GameApplication__GameApplication`) from the table this function
returned, exactly the "allocate, get methods, call ctor slot" idiom from
CLAUDE.md's "Writing a class method".

```c
extern s32 gGameApplicationMethods[];

void *GetGameApplicationMethods(void) {
    return gGameApplicationMethods;
}
```

`gGameApplicationMethods` is declared `s32[]` (not typed as the owning class's vtable
struct) because that struct doesn't exist yet — the table itself still lives
in `asm/data/57070.data.s` as raw words, owned by neither this unit nor any
carved one yet. Whoever carves that data slot should replace this `extern`
with a proper vtable-typed one.

## Proposed learning

A function that only does `lui/addiu` to a symbol with no `lw`/`sw` around it
is returning `&symbol`, not a value read from it — worth checking who calls it
before guessing a dereferencing signature.

## Naming

Round 52 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_800269E0` | `GetGameApplicationMethods` | A |

**Evidence.** A two-instruction address-of returning `&gGameApplicationMethods`. `GameApplication`
is already an established type name in `src/app/game_shell.c` (that unit's own
functions are typed against it), and the "return my own vtable" shape is
already named twice in this project (`GetCdDriverMethods`,
`GetFileResourceMethods`, this same round). Pure leaf whose mechanics are its
purpose.

## Track 4 (2026-09-26, round 88)

Retyped to `GameApplicationMethods *GetGameApplicationMethods(void)`, returning
`&gGameApplicationMethods`; both are declared once, in `include/GameApplication.h`
(`include/data_source.h`'s `extern s32 gGameApplicationMethods[]` view is deleted, and
New_GameApplication no longer casts the result). Image byte-identical.
