# Class6D3C8__Class6D3C8

> Renamed from `func_80025FDC` on 2026-09-24 (tools/rename.py). Address 0x80025fdc.

**Unit:** code_1677c · **Size:** 50 instructions (0xC8 bytes) · **Status:** MATCHED (50/50 words, whole-image SHA1 green)

## What it does

The constructor for the class whose method table is `D_8006D3C8`
(`Class6D3C8`, slot `+0x008`): runs the intermediate base class's own
constructor through its ctor slot, installs this class's own vtable, stores
the ctor argument, loads a 3D model ("ETC\DREAME5.TMD"), builds this
object's owned `DreamSys` from the loaded model, makes one call into a
not-yet-understood `DreamSys` vtable slot (`+0x228`), then finally invokes
its own `slot40` (`Class6D3C8__SetDayFromTickCount`, the day-cursor advance already matched in
this unit).

## Derivation

```
addiu $sp, $sp, -0x30
sw    $s0, 0x20($sp)
addu  $s0, $a0, $zero        ; s0 = self
sw    $s1, 0x24($sp)
sw    $ra, 0x28($sp)
jal   GetApplicationMethods           ; -> &D_8006E4F0 (intermediate base table)
 addu $s1, $a1, $zero          ; s1 = arg
lw    $a1, 0x0($s1)             ; a1 = arg->unk00
lw    $v0, 0x8($v0)              ; base table's ctor slot
nop
jalr  $v0                         ; base_ctor(self, arg->unk00), return discarded
 addu $a0, $s0, $zero
jal   GetClass6D3C8Methods                ; -> &D_8006D3C8 (own vtable)
 nop
sw    $v0, 0x0($s0)                 ; self->methods = own vtable
jal   func_80048CF0                  ; reads an unnamed small-data global
 sw   $s1, 0x20($s0)                   ; self->arg = arg
jal   func_800270AC                     ; stores its arg into another unnamed global
 addu $a0, $v0, $zero
addiu $a0, $sp, 0x10                     ; &local request buffer
lui   $v0, %hi(sModelPathDreamE5)
addiu $v0, $v0, %lo(sModelPathDreamE5)            ; &"ETC\DREAME5.TMD"
sw    $zero, 0x10($sp)                      ; req.type = 0
jal   New_LinkResource                           ; loads the model, returns a handle
 sw   $v0, 0x14($sp)                            ; req.path = &sModelPathDreamE5
addu  $a0, $v0, $zero
addu  $a1, $zero, $zero
jal   New_DreamSys                               ; New_DreamSys(handle, 0, 0)
 addu $a2, $zero, $zero
sw    $v0, 0x28($s0)                              ; self->dreamSys = ...
lw    $a0, 0x28($s0)                               ; reload self->dreamSys
sw    $zero, 0x24($s0)                              ; self->unk24 = 0
lw    $v0, 0x0($a0)                                  ; dreamSys->vt
lw    $a1, 0x14($s1)                                  ; a1 = arg->unk14
lw    $v0, 0x228($v0)                                  ; vt->func_228
nop
jalr  $v0                                               ; dreamSys->vt->func_228(dreamSys, arg->unk14)
 nop
lw    $v0, 0x0($s0)                                      ; reload self->methods
nop
lw    $v0, 0x40($v0)                                      ; own vtable's slot40
nop
jalr  $v0                                                  ; self->methods->slot40(self)
 addu $a0, $s0, $zero
... epilogue (restore ra,s1,s0; sp += 0x30) ...
```

Written as:

```c
void Class6D3C8__Class6D3C8(Class6D3C8 *self, Class6D3C8CtorArgs *arg) {
    LoadModelRequest req;

    GetApplicationMethods()->ctor(self, arg->unk00);
    self->methods = GetClass6D3C8Methods();
    self->arg = arg;
    func_800270AC(func_80048CF0());
    req.type = 0;
    req.path = sModelPathDreamE5;
    self->dreamSys = New_DreamSys(New_LinkResource(&req), 0, 0);
    self->unk24 = 0;
    self->dreamSys->vt->func_228(self->dreamSys, arg->unk14);
    self->methods->slot40(self);
}
```

## The one lever that closed it: local struct SIZE, not shape

The first attempt matched every single instruction byte-for-byte except the
frame size (retail `-0x30`, mine `-0x28`) and the three saved-register
offsets that shift with it (`s0`/`s1`/`ra` each 8 bytes lower). `req` was
declared as `{ s32 type; const char *path; }` (8 bytes) and placed at
`sp+0x10`, exactly matching retail's writes (`sw zero, 0x10(sp)` /
`sw v0, 0x14(sp)`) — but retail's saved registers sit at `sp+0x20`/`0x24`/
`0x28`, 8 bytes above where an 8-byte local would put them. Retail reserves
8 bytes of stack that are never written by this function at all.

Padding `LoadModelRequest` out to 4 words (`{ s32 type; const char *path;
s32 unk08; s32 unk0C; }` — the extra two fields never read or written here)
moved the frame to exactly `-0x30` and every remaining diff disappeared.
This is the same idiom as `docs/DECOMPILATION_LEARNINGS.md`'s "a
`padNN[0x..]` access means the struct is missing a field" note, just for a
*stack* local instead of a heap/global struct: **GCC allocates stack space
for a local's declared `sizeof`, not for the bytes it happens to
initialize** — a local that is only partially written is real evidence the
type is bigger than its used prefix, not evidence of two separate locals or
of scheduling. `LoadModelRequest` is likely a "resource load request" struct
shared by other loader call sites elsewhere in the game (all with the same
`{type; path; ...}` shape) that only ever populates its first two words at
this call site; its trailing two words stay type/offset-unknown until
another caller is found that writes them.

## New struct/header knowledge (recorded in `include/`)

- `include/Class6D3C8.h`: added `Class6D3C8CtorArgs` (the ctor's `arg`
  parameter type — only `+0x00` and `+0x14` are read here, observed against
  the one call site's data, `asm/main.s`'s `gClass6D3C8CtorArgs` global:
  `{0x13, 0, 1, 1, 1, 1}`), added `LoadModelRequest`, retyped
  `MiddleClassMethods.ctor` and `Class6D3C8Methods.ctor`/`.slot40` from
  opaque `void *` to real callable signatures now that this function
  exercises them, retyped `Class6D3C8::arg` and `::dreamSys` from `void *`
  to their real pointer types, and declared the small externs this function
  needed (`GetClass6D3C8Methods`, `sModelPathDreamE5`, `func_80048CF0`, `func_800270AC`,
  `New_LinkResource`).
- `include/DreamSys.h`: extended `struct vtable_DreamSys` past its
  previously-documented end (`0x21c`) with 3 padding words and a new named
  slot at `+0x228` (`func_228`), discovered purely from this call site —
  nothing in `DreamSys`'s own unit references it yet. Added the
  `New_DreamSys` prototype (still `INCLUDE_ASM` in `src/DreamSys.c`; this
  is a same-shape cross-unit call as documented in
  `docs/DECOMPILATION_LEARNINGS.md`).

## Proposed learning

A local variable whose stack slot is only partially written, leaving a gap
between its last write and the next saved register, is the *stack-local*
form of the "a `padNN` access means a missing struct field" idiom already
documented for heap/global structs — pad the local's C type out to the full
size the frame implies (found by diffing the frame-size/saved-register
constants against retail) rather than assuming a scheduling residue or a
second hidden local.

## Naming

**`Class6D3C8__Class6D3C8` -- tier A.** Convention `Class__Class` for a
constructor (compare `Class865C8__Class865C8`, `BasicClass__BasicClass`,
`Class65650__Class65650`, etc. -- `grep -rnP '(\w+)__\1\(' src/*.c`). Evident
from the body itself: dispatched through `Class6D3C8Methods.ctor`
(vtable slot +0x008), calls the base class's own ctor slot first, then
installs this class's own vtable pointer -- the base-constructor-through-
slot+8 shape documented in `docs/research/class-framework.md`.

## Track 4

**2026-09-25, round 84 (echo).** The parent class is declared once, in
`include/Application.h`; the base-ctor call is
`GetApplicationMethods()->ctor((Application *)self, arg->unk00)`, an upcast
that emits no code. Bytes unchanged.
