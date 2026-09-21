# GetCdFileSize -- MATCHED (11/11 words)

> Renamed from `func_80028A50` on 2026-09-21 (tools/rename.py). Address 0x80028a50.

Unit: `code_179d8_h`. Runner: echo, round 17 (second assignment).

## Result

```c
typedef struct ObjA34_179D8H {
    u8 pad0[0x0C];
    s32 unk0C;
    u8 pad10[0x1C - 0x10];
    u32 unk1C;
} ObjA34_179D8H;

s32 GetCdFileSize(ObjA34_179D8H *self) {
    u32 result;

    if (self->unk0C == 0) {
        result = 0;
    } else {
        result = ((self->unk1C >> 11) + 1) << 11;
    }
    return result;
}
```

Byte-exact, 11/11 words.

## Notes

Rounds `self->unk1C` up to the next 0x800 (2048)-byte boundary when
`self->unk0C != 0`, else returns 0 -- reads like a CD-ROM sector-aligned
buffer-size computation, consistent with this unit's overall CD theme
(`CdStatus`, `strcpy`/`strstr` inherited names).

`self->unk1C` is `u32` (not `s32`): retail's shift is `srl` (logical), not
`sra`. A first attempt with the field left `s32` and an explicit `(u32)`
cast at the read site got the shift right but left a DIFFERENT residue (see
below); retyping the field itself was cleaner and is arguably the more
honest reading anyway -- nothing suggests this quantity is ever negative.

**`ObjA34_179D8H` is a local type, not an extension of
`code_171e0.h`'s `Class6D430`**, even though the offsets coincide
suspiciously well: `self->unk0C` lines up with `Class6D430::unk0C`
(already named there, "saved/restored around the unk10 (re)alloc"), and
`self->unk1C` falls inside that struct's `pad18[0x20-0x18]` gap (explicitly
documented there as "unknown, 8 bytes"). Extending the shared header would
require splitting that padding without shifting anything after it, which is
mechanically safe -- but `code_171e0.h` is OUT OF UNIT (shared with
`code_171e0.c`, not this unit) and this round's rules are explicit that nothing
outside the assigned unit + its reports gets edited. Kept as this unit's own
narrower local reading instead, per the project's multiple-independent-
local-views convention. Worth flagging for the head: if this coincidence
holds up under more scrutiny, `code_171e0.h`'s owner may want to fold
`unk1C` in properly.

**Residue and the fix, worth having as a general note.** First attempt used
the by-then-familiar "pre-zero an accumulator, conditionally overwrite it"
shape:

```c
u32 result = 0;
if (self->unk0C != 0) {
    result = ((self->unk1C >> 11) + 1) << 11;
}
return result;
```

This scored 8/11: correct control flow and correct instructions, but
`result` landed in `$v1` with a trailing `move $v0,$v1` where retail computes
directly into `$v0` with no extra move at all. This is the OPPOSITE of
`VabStreamObj__OnBodyReady`'s residue (that function's retail asm genuinely DOES thread
a `$v1` accumulator through to a final `move`), so "pre-zero, conditionally
override" is not a universal idiom -- it happens to match retail sometimes
and not others, and the two cases are only distinguishable by trying both.
Switching to an explicit `if (...) { result = A; } else { result = B; }` --
both branches assigning, no pre-initialization -- put `result` in `$v0`
throughout and matched immediately. A genuine early-return
(`if (cond) return 0; return computed;`) was also tried and was WORSE
(6/11): it introduced an extra jump, so this function's trivial (frameless)
epilogue does NOT get shared the way `InitSoundCueSet`'s did in the other unit
this round -- the "does an early return share the tail" question is
apparently per-function, not something to assume either way.

### Proposed learning

Three visually similar ways to write "compute a value in one of two
branches, then return it" -- pre-zero-then-override, explicit if/else with
both arms assigning, and early-return -- are NOT interchangeable to GCC
2.6.3 at `-O2`, and which one matches is not predictable from the C alone.
When a boolean-guarded return has a residue that is ONLY a register
identity difference (same instructions, same values, `$v1`+`move` vs a bare
`$v0`) with no wrong branch targets, try the sibling forms cheaply -- one
of the three is usually right, and testing all three (as here) costs three
build-and-verify cycles, far less than diagnosing the register allocator's
reasoning by hand. This is now the SECOND time in two units this exact
family of shapes has needed exactly this kind of trial (`VabStreamObj__OnBodyReady`
needed nested-if/single-exit over combined-`&&`/early-return; this one
needed if/else over pre-zero), which makes it worth trying all three
sibling forms as a matter of course before spending time on manual RTL
reasoning.
