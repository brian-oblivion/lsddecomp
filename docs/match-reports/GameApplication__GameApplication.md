# GameApplication__GameApplication

> Renamed from `Class6D3C8__Class6D3C8` on 2026-09-26 (tools/rename.py). Address 0x80025fdc.

> Renamed from `func_80025FDC` on 2026-09-24 (tools/rename.py). Address 0x80025fdc.

**Unit:** game_shell · **Size:** 50 instructions (0xC8 bytes) · **Status:** MATCHED (50/50 words, whole-image SHA1 green)

## What it does

The constructor for the class whose method table is `gGameApplicationMethods`
(`GameApplication`, slot `+0x008`): runs the intermediate base class's own
constructor through its ctor slot, installs this class's own vtable, stores
the ctor argument, loads a 3D model ("ETC\DREAME5.TMD"), builds this
object's owned `DreamSys` from the loaded model, makes one call into a
not-yet-understood `DreamSys` vtable slot (`+0x228`), then finally invokes
its own `slot40` (`GameApplication__SeedRandom`, the day-cursor advance already matched in
this unit).

## Derivation

```
addiu $sp, $sp, -0x30
sw    $s0, 0x20($sp)
addu  $s0, $a0, $zero        ; s0 = self
sw    $s1, 0x24($sp)
sw    $ra, 0x28($sp)
jal   GetApplicationMethods           ; -> &gApplicationMethods (intermediate base table)
 addu $s1, $a1, $zero          ; s1 = arg
lw    $a1, 0x0($s1)             ; a1 = arg->unk00
lw    $v0, 0x8($v0)              ; base table's ctor slot
nop
jalr  $v0                         ; base_ctor(self, arg->unk00), return discarded
 addu $a0, $s0, $zero
jal   GetGameApplicationMethods                ; -> &gGameApplicationMethods (own vtable)
 nop
sw    $v0, 0x0($s0)                 ; self->methods = own vtable
jal   GetDefaultDataDirectory                  ; reads an unnamed small-data global
 sw   $s1, 0x20($s0)                   ; self->arg = arg
jal   SetDataDirectory                     ; stores its arg into another unnamed global
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
void GameApplication__GameApplication(GameApplication *self, GameApplicationConfig *arg) {
    LoadModelRequest req;

    GetApplicationMethods()->ctor(self, arg->unk00);
    self->methods = GetGameApplicationMethods();
    self->arg = arg;
    SetDataDirectory(GetDefaultDataDirectory());
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

- `include/GameApplication.h`: added `GameApplicationConfig` (the ctor's `arg`
  parameter type — only `+0x00` and `+0x14` are read here, observed against
  the one call site's data, `asm/main.s`'s `sGameApplicationConfig` global:
  `{0x13, 0, 1, 1, 1, 1}`), added `LoadModelRequest`, retyped
  `MiddleClassMethods.ctor` and `GameApplicationMethods.ctor`/`.slot40` from
  opaque `void *` to real callable signatures now that this function
  exercises them, retyped `GameApplication::arg` and `::dreamSys` from `void *`
  to their real pointer types, and declared the small externs this function
  needed (`GetGameApplicationMethods`, `sModelPathDreamE5`, `GetDefaultDataDirectory`, `SetDataDirectory`,
  `New_LinkResource`).
- `include/dream_sys.h`: extended `struct vtable_DreamSys` past its
  previously-documented end (`0x21c`) with 3 padding words and a new named
  slot at `+0x228` (`func_228`), discovered purely from this call site —
  nothing in `DreamSys`'s own unit references it yet. Added the
  `New_DreamSys` prototype (still `INCLUDE_ASM` in `src/world/dream_sys.c`; this
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

**`GameApplication__GameApplication` -- tier A.** Convention `Class__Class` for a
constructor (compare `DayTask__DayTask`, `BasicClass__BasicClass`,
`TodActor__TodActor`, etc. -- `grep -rnP '(\w+)__\1\(' src/*.c`). Evident
from the body itself: dispatched through `GameApplicationMethods.ctor`
(vtable slot +0x008), calls the base class's own ctor slot first, then
installs this class's own vtable pointer -- the base-constructor-through-
slot+8 shape documented in `docs/research/class-framework.md`.

## Track 4

**2026-09-25, round 84 (echo).** The parent class is declared once, in
`include/application.h`; the base-ctor call is
`GetApplicationMethods()->ctor((Application *)self, arg->unk00)`, an upcast
that emits no code. Bytes unchanged.

## Track 6 (round 93, echo)

Class `Class6D3C8` renamed `GameApplication` (`tools/renametype.py
Class6D3C8 GameApplication`), table `D_8006D3C8` renamed
`gGameApplicationMethods` (`tools/rename.py`). **Tier A.** It is the only
Application subclass and the one object main() builds (`New_GameApplication(
&sGameApplicationConfig)` into `sGameApplication`, then initSystems and the
never-returning runMainLoop); its ctor builds and keeps the game's DreamSys,
and its own methods are exactly the six hooks Application's main loop calls
(intro logos, weekly stream, GraphRoom poll, DayTask run, cinematic and
follow-up streams). Main and Application's runMainLoop, its two callers, agree.
The name claims only "the game's application object", which is what those
callers make it; it does not name what the sequence is for.

Types renamed with it:

- `Class6D3C8CtorArgs` -> `GameApplicationConfig` (`renametype.py
  GameApplicationCtorArgs GameApplicationConfig --any-stem`, after the family
  rename), with `gClass6D3C8CtorArgs` -> `sGameApplicationConfig`. Tier A: one
  static instance, `{0x13, 0, 1, 1, 1, 1}`, whose words are the data source and
  on/off switches the hooks test (`playStreams`, `showIntroLogos`,
  `pollGraphRoom`). The field that keeps it, `ctorArgs`, is now `config`
  (header edit; the compiler listed 8 accessors, all in game_shell.c).
- `Class6D3C8SetDayFn` -> `GameApplicationSeedRandomFn`, following its occupant
  `GameApplication__SeedRandom` (see that report).
- `Class6D3C8InitSystemsFn` -> `GameApplicationInitSystemsFn`,
  `Class6D3C8Methods` -> `GameApplicationMethods`: the family rename.
- Header guard `CLASS_6D3C8_H` -> `GAMEAPPLICATION_H` by hand: renametype.py's
  upper-case pattern is `CLASS6D3C8`, so an underscored guard is invisible to it.

Kept: `GameApplicationConfig::unk04` (DayTask's ctor passes `unk04 == 0` to
SetActiveDataSourceDriverMode; the driver mode's meaning is not established)
and `unk14` (handed to DreamSys__GetSetConfigOption, which stores a value >= 0 at the
still-unnamed DreamSys +0x924). No Sony type applies: the class's fields are a
config pointer, a flag and a DreamSys pointer.

renametype.py rewrote the old class name inside this and the sibling reports'
history prose (pending an operator decision; not undone by hand).

Facts the header banner carried, kept here: `classtable.py
gGameApplicationMethods --vs gApplicationMethods` shares every slot but
+0x008/+0x040/+0x044 and the six +0x050..+0x064 slots Application leaves
NULL (25 slots, i.e. Application's 0x68-byte table fully populated); the
object size 0x2C is New_GameApplication's BMemPMgrAlloc(0x2C); the config
instance is in asm/data/57028.data.s; the previous banner kept the table's
address as the name because "nothing yet names what the class IS beyond the
object main runs" -- the pass above is the reading that settles it at the
level the name claims.

## Track 6 (round 96, delta): the request local is ResourceSourceRequest

src/app/game_shell.c `LoadModelRequest`, `{ s32 type; const char *path; s32 unk08; s32 unk0C; }`, is the same
0x10-byte record as DayTaskStageMap.c's and the third caller's: the body writes
`type = 0` (ResourceSource's NULL `buffer`: no buffer to adopt) and `path`
(its `name`: the file to request) and passes it to New_LinkResource. It
retired onto include/FileResource.h's `ResourceSourceRequest` (a
`ResourceSource src` then 8 bytes of padding; tier A: the fields are the
ctor's own descriptor, read by LinkResource__LinkResource as `src->buffer`
and `src->name`). The body now writes `req.src.buffer = NULL` and
`req.src.name` and passes `&req.src` with no cast; byte-exact. The 0x10
size is kept (8 changes the frame, measured on StageMap__PopulateSlotCells).

## Track 6 (round 97, alpha): the request local is ResourceRequest

`ResourceSourceRequest` is deleted. The local is now `ResourceRequest req;`
(include/FileResource.h, 0x0C) with `mode` left unset; the body is
unchanged (`req.src...` writes, `&req.src` to New_LinkResource). A plain
`ResourceSource` (8 bytes) was measured to shrink this function's frame by
8 and move every callee-save slot, and an unused pad local is dropped by
cc1, so ResourceRequest is the smallest existing type that keeps the frame.
Byte-exact. Table and details: ResourceRequest__Set.md, round 97 second job.

## Track 7 polish (round 100, echo)

Body changes, all byte-identical: parameter arg -> config; the DREAME5 path extern is `char []` so the `(char *)` cast is gone (ResourceSource's name is char *).

### History: code_1677c.c comments before the round-100 polish

Moved here from the source, verbatim (names as they stood then, where the tools had not already rewritten them).

```c
/* Constructs a GameApplication instance: runs the intermediate base class's own
 * constructor (through its ctor slot), installs this class's own vtable,
 * stores the ctor argument, loads the "ETC\DREAME5.TMD" model, builds this
 * object's owned DreamSys from it, dispatches one DreamSys init call, then
 * runs this class's own slot40 (GameApplication__SeedRandom) once. */

extern const char sModelPathDreamE5[]; /* "ETC\DREAME5.TMD", asm/data/FA4.rodata.s */
```

## Track 10 (2026-09-28, round 104, alpha)

`GameApplicationConfig::unk14` is `dreamSysConfigOption`, the name charlie gave the DreamSys word it lands in (DreamSys +0x924, `configOption`, set through slot228). Tier B: stored once here (sGameApplicationConfig passes 1); no code reads the word back. Proposed, not applied (dream_sys.h is world's): slot228 -> `getSetConfigOption`.

## Track 10 (2026-09-28, round 104, echo)

DreamSysMethods `slot228` -> `getSetConfigOption`, the slot's occupant being DreamSys__GetSetConfigOption (`classtable.py gDreamSysMethods`, +0x228); the ctor's call now reads `methods->getSetConfigOption`, and GameApplicationConfig::dreamSysConfigOption's comment names the slot, its occupant and the field it stores to. Byte-identical.
