# ResourceRequest__Set

> Renamed from `SetVec3` on 2026-09-27 (tools/rename.py). Address 0x80026ce8.

> Renamed from `func_80026CE8` on 2026-09-18 (tools/rename.py). Address 0x80026ce8.

**Unit:** GameApplicationFileResource · **Size:** 5 instructions · **Status:** MATCHED (5/5 words)

## What it does

Fills a `ResourceRequest` (`include/GameApplicationFileResource.h`) -- `buffer`, `name`,
`mode` -- and returns the pointer. That is the descriptor the LinkResource,
Tod, TodSet, ModelData and TriggerWorld ctors take (GraphicsResources.c's
`ResourceSource` declares only its first two words).

## Derivation

```
addu  $v0, $a0, $zero
sw    $a1, 0x0($v0)
sw    $a2, 0x4($v0)
jr    $ra
 sw   $a3, 0x8($v0)
```

The leading `addu $v0, $a0, $zero` copies the incoming pointer into the return
register before the stores, which only makes sense if the function's C source
actually returns it — nothing else in the body needs a copy of `$a0` in
`$v0`. Declared accordingly, rather than as `void`, on that positive evidence
(the one caller found, in `asm/nonmatchings/DreamAux/InitDreamAux.s`,
discards the return value, so a `void` guess would have looked equally
plausible from the call site alone — the `addu` in this function's own body is
what settles it):

```c
typedef struct ResourceRequest {
    /* +0x00 */ void *buffer;
    /* +0x04 */ char *name;
    /* +0x08 */ s32 mode;
} ResourceRequest;

ResourceRequest *ResourceRequest__Set(ResourceRequest *this, void *buffer, char *name, s32 mode) {
    this->buffer = buffer;
    this->name = name;
    this->mode = mode;
    return this;
}
```

Only the first three words are known; there may be a fourth field the struct
doesn't yet claim (this function simply never touches it).

## Proposed learning

A leaf function that copies its first argument into `$v0` before doing
anything else, with no other use for that copy, is returning the pointer —
even when the one caller you can find ignores the result. Check the
function's OWN body for the `addu $v0, $a0, ...` idiom before trusting a
caller's ignored return value as evidence for `void`.

## Naming

Round 52 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_80026CE8` | `ResourceRequest__Set` | A |

**Evidence.** Stores three words into its first argument and returns the
pointer -- a pure "set and return this" setter. Mechanics are its purpose.

### Track 6 (round 96, alpha)

| was | now | tier |
| --- | --- | --- |
| `Vec3_171e0 {s32 x, y, z}` | `ResourceRequest {void *buffer; char *name; s32 mode}` | A (type), B (`mode`) |
| `SetVec3` | `ResourceRequest__Set` | A |

**Evidence.** Not a vector. All four callers in retail (the only `jal`s to
0x80026CE8) fill it as a ctor descriptor and pass it cast to `ResourceSource`:
`ModelData__BuildResources` (TMD offset in the buffer, name 0, 1) and then
`New_LinkResource`/`New_TodSet`; `TriggerWorld__BuildResources` and
`TodSet__BuildTods` (0, 0, 1, then the buffer per sub-block) into
`New_ModelData`/`New_Tod`; `InitDreamAux` (0, "ETC\\SYMSPY.MOM", 1) into
`New_ModelData`. Every ctor reads `buffer` (adopt it) and, when that is NULL,
`name` (request the file). Nobody reads word +0x08, and every caller passes 1;
`mode` follows the name two callers' own views already use, and is tier B.
Not `LongVec3` (the words are a pointer, a string and a flag) and not Sony's
`VECTOR` (no fourth word is touched).

Several unit-local views of this same descriptor remain, under other names:
`ResourceSource` and `ResourceSourceArgs` (GraphicsResources.c),
`DreamAuxLoadReq` (DreamAux.h), `LoadRequest` (class_39e08.h),
`LoadModelRequest` (GameApplicationFileResource), `ResourceSourceRequest` (class_3bb8c.c). Some are
0x10-byte locals, where the stack slot size may be what matches, so merging
them is a head decision (proposed below), not a rename.

## Proposed (not applied: outside this job's edit set)

- One header for the descriptor (e.g. `include/ResourceSource.h`) holding a
  single definition, retiring `ResourceRequest`, GraphicsResources.c's
  `ResourceSource`/`ResourceSourceArgs`, `DreamAuxLoadReq`, `LoadRequest`,
  `LoadModelRequest` and `ResourceSourceRequest`, with `ResourceRequest__Set`'s
  prototype there. Check each 0x10-byte local keeps its size.
- `include/DreamAux.h`'s comment above `DreamAuxLoadReq` still quotes the
  old body (`this->x=x; ...`) and calls the record "physically the same shape"
  as the vector; it now reads as the same descriptor as `ResourceRequest`.
- `ResourceRequest__Set`'s prototype is not in `include/GameApplicationFileResource.h`: three
  units declare their own (typed to their local view), and DreamAux.h's
  would conflict with it in any file including both.

### Track 6 (round 97, alpha): one definition

`ResourceRequest` now lives in `include/FileResource.h`, beside the
`ResourceSource` it extends, as `{ ResourceSource src; s32 mode; }`, with
`ResourceRequest__Set`'s one prototype under it. `include/GameApplicationFileResource.h` no
longer defines it. The body reads `this->src.buffer = buffer;
this->src.name = name; this->mode = mode;`. Retired onto it:
GraphicsResources.c's `ResourceSourceArgs` (ModelData__BuildResources,
TriggerWorld__BuildResources, TodSet__BuildTods) and include/DreamAux.h's
`DreamAuxLoadReq` (InitDreamAux), along with the local
`ResourceRequest__Set` externs typed to them. Every `(ResourceSource *)&req`
cast became `&req.src`. `mode` stays tier B: every caller passes 1 and no
code reads it. Image byte-identical after every step.

Kept: `ResourceSourceRequest` (FileResource.h, 0x10 bytes; StageMap__PopulateSlotCells,
DayTask__DayTask, GameApplication__GameApplication). Its callers write only
`src.buffer` and never call ResourceRequest__Set. Measured this round:
shrinking its pad to 4 bytes (so it is 0x0C, ResourceRequest's size) still
builds byte-exact, so its size does not tell the two apart. Retiring it onto
`ResourceRequest` is proposed, not applied, because its three units are
outside this job's edit set. `TodActorDesc` (include/code_55dd4.h) now opens
with a `ResourceSource src` but is not a ResourceRequest: see
TodActor__AcquireModelData.md.

### Track 6 (round 97, alpha, second job): ResourceSourceRequest retired

`ResourceSourceRequest` is deleted from include/FileResource.h. Its three
locals are now `ResourceRequest req;` with `mode` never written:

| function | retail frame | words written | passed |
| --- | --- | --- | --- |
| StageMap__PopulateSlotCells | 0x80, req at sp+0x50 | `src.buffer` (sp+0x50) | `&req.src` to New_LinkResource |
| DayTask__DayTask | 0x38, req at sp+0x10 | `src.buffer = NULL`, `src.name` (sp+0x10/0x14) | `&req.src` to New_LinkResource |
| GameApplication__GameApplication | 0x30, req at sp+0x10 | `src.buffer = NULL`, `src.name` (sp+0x10/0x14) | `&req.src` to New_LinkResource |

Plain `ResourceSource` was tried first in all three and fails every one
the same way: the frame shrinks by 8 (0x80 -> 0x78, 0x38 -> 0x30,
0x30 -> 0x28), moving every callee-save slot (132/150, 93/107 and 42/50
words). So the local is more than 8 bytes, and ResourceRequest's 0x0C
rounds to the same frame as the old 0x10. A separate unused pad local
(`ResourceSource src; s32 pad[1];`) does not help: cc1 drops the unused
array and the frame still shrinks by 8, so the only honest spelling that
keeps the frame is an existing type of 9..16 bytes, and ResourceRequest is
the one this descriptor already has. Each local carries a MATCHING line
saying mode is unset and why the type is not ResourceSource. The
ResourceRequest comment in FileResource.h now names the three hand-filled
callers. Image byte-identical after every step.
