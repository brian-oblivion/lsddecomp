# GetCdFileSize -- MATCHED (11/11 words)

> Renamed from `func_80028A50` on 2026-09-21 (tools/rename.py). Address 0x80028a50.

Unit: `CdDriver`. Runner: echo, round 17 (second assignment).

## Result

```c
typedef struct ObjA34_179D8H {
    u8 pad0[0x0C];
    s32 isOpen;
    u8 pad10[0x1C - 0x10];
    u32 size;
} ObjA34_179D8H;

s32 GetCdFileSize(ObjA34_179D8H *self) {
    u32 result;

    if (self->isOpen == 0) {
        result = 0;
    } else {
        result = ((self->size >> 11) + 1) << 11;
    }
    return result;
}
```

Byte-exact, 11/11 words. (Fields renamed round 64: `unk0C` -> `isOpen`,
`unk1C` -> `size`; see `## Naming` below.)

## Notes

Rounds `self->size` up to the next 0x800 (2048)-byte boundary when
`self->isOpen != 0`, else returns 0 -- reads like a CD-ROM sector-aligned
buffer-size computation, consistent with this unit's overall CD theme
(`CdStatus`, `strcpy`/`strstr` inherited names).

`self->size` is `u32` (not `s32`): retail's shift is `srl` (logical), not
`sra`. A first attempt with the field left `s32` and an explicit `(u32)`
cast at the read site got the shift right but left a DIFFERENT residue (see
below); retyping the field itself was cleaner and is arguably the more
honest reading anyway -- nothing suggests this quantity is ever negative.

**`ObjA34_179D8H` is a local type, not an extension of
`GameApplicationFileResource.h`'s `FileResource`**, even though the offsets coincide
suspiciously well: `self->isOpen` (offset +0x0C) lines up with
`FileResource::pendingGeneration` (same offset, "saved/restored around the
buffer (re)alloc"), and `self->size` falls inside that struct's
`pad18[0x20-0x18]` gap (explicitly documented there as "unknown, 8 bytes").
Extending the shared header would require splitting that padding without
shifting anything after it, which is mechanically safe -- but
`GameApplicationFileResource.h` is OUT OF UNIT (shared with `GameApplicationFileResource.c` and
`cd_driver.c`, neither this unit) and the rule is explicit that nothing
outside the assigned unit + its reports gets edited. Kept as this unit's own
narrower local reading instead, per the project's multiple-independent-
local-views convention. Worth flagging for the head: if this coincidence
holds up under more scrutiny, `GameApplicationFileResource.h`'s owner may want to fold
`size` in properly -- though round 64 notes the semantic mismatch this
would need to resolve first (see `## Naming` below).

**Residue and the fix, worth having as a general note.** First attempt used
the by-then-familiar "pre-zero an accumulator, conditionally overwrite it"
shape:

```c
u32 result = 0;
if (self->isOpen != 0) {
    result = ((self->size >> 11) + 1) << 11;
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

## Naming (round 64, runner alpha)

- `func_80028A50` -> `GetCdFileSize`, tier B. `src/cd/cd_driver.c`'s
  `CdDriver__Seek` calls this function directly (ignoring its own `arg1`,
  `arg2`) when CD-async mode is off; its async path, when asked to just
  query size (`arg2 != 0`), does the IDENTICAL `self->unk1C` rounding as a
  fallthrough of the same seek function. Paired with `OpenCdFile`/
  `CloseCdFile`/`ReadCdFile` (also this unit) as an Open/Close/Size/Read
  quad.
- `ObjA34_179D8H::unk0C` -> `isOpen`, `unk1C` -> `size` (tier B, both).
  Evidence is cross-unit: `src/cd/cd_driver.c`'s `Obj80027480` is an
  independent local view of what is very likely the SAME object (see the
  coincidence note above and `CloseCdFile.md`), and its own
  `CdDriver__Open`/`CdDriver__Close`/`CdDriver__Seek` async bodies set/clear
  the identical offsets (`self->unk0C = 1` on a successful CD lookup,
  `self->unk0C = 0` on close, `self->unk1C` fed the identical
  `(x >> 11) + 1) << 11` rounding on a size query) around the identical
  `CdSearchFile`/`CdControl`+`CdSync` sequence this unit's own `OpenCdFile`
  uses. Not derived from this function's body in isolation.
- **Note on the `FileResource::pendingGeneration` coincidence (see `## Notes`
  above): the SEMANTICS now look different, not just the offset.**
  `pendingGeneration` is documented in `GameApplicationFileResource.h` as "saved/restored
  around the buffer (re)alloc" (implying a counter), while this unit's
  reading at the same offset is a plain 0/1 open flag. These could still be
  the same field serving double duty (0 = no generation yet = "closed"),
  but that is now a SPECIFIC claim to verify, not just an offset match --
  flagged here rather than resolved, per the park-rule spirit for anything
  short of direct evidence. Not applied to `GameApplicationFileResource.h` (out of unit).

## Naming (round 99, echo, track 7)

`>> 11`/`<< 11` -> `CD_SECTOR_SHIFT` (`include/cd_driver.h`, 2048-byte
sectors). The result is always one sector more than the whole sectors in
`size`, even when `size` is already a multiple of 2048; `CdDriver__Seek`'s
async path rounds up only when `size & 0x7FF` is nonzero. The source says so
in the function's comment.
