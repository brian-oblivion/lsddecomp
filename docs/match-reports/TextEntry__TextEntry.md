# TextEntry__TextEntry — MATCHED (round 45, 49/49 words)

> Renamed from `Obj86ED0__Obj86ED0` on 2026-09-26 (tools/rename.py). Address 0x80050c14.

> Renamed from `func_80050C14` on 2026-09-24 (tools/rename.py). Address 0x80050c14.

**Unit:** TextEntryItemList · **Size:** 49 words (0xC4 bytes)

Filed as a `gp_rel`-blocked stub in round 14. That blocker was RESOLVED in
round 42 (`--gp-symbols`, pinned in the Makefile). Matched on the first
attempt once rebuilt.

## Derivation

```c
extern s32 strlen(char *s);
extern u8 *gNameCharTable;
extern void TextEntry__ClearChildRefs(Obj86ED0 *self);

void TextEntry__TextEntry(Obj86ED0 *self, char *arg1, s32 arg2)
{
    u8 *p;
    s32 count;

    GetBasicClassMethods()->ctor(self);
    self->methods = GetTextEntryMethods();
    self->unk10 = strlen(arg1);
    self->unk28 = BMemPMgrAlloc(self->unk10 + 4);

    p = gNameCharTable;
    count = 0;
    while (*p != 0) {
        p++;
        count++;
    }
    self->unk14 = count;

    TextEntry__ClearChildRefs(self);
    self->methods->slot40(self, arg1, arg2);
}
```

This is `Obj86ED0`'s own ctor, called through `Obj86ED0Methods::ctor` at
`New_TextEntry`'s allocation site: base ctor first
(`GetBasicClassMethods()->ctor(self)`), then `self->methods` overridden to this
class's own table (`GetTextEntryMethods()`, defined in `TextEntryItemList.c`) — same
shape as `NodeGuardedViewport__NodeGuardedViewport` in `TitleMenuTaskObjF.c`.

`self->unk10 = strlen(arg1)` then `self->unk28 = BMemPMgrAlloc(self->unk10
+ 4)` allocates a name buffer 4 bytes larger than the string. Retail's own
codegen reuses the SAME register across both statements (the store to
`self->unk10` and the `+4` addend both read `strlen`'s still-live return
value rather than reloading `self->unk10` from memory) — ordinary redundant-
load elimination, not something the C needs to spell out by hand.

**One thing worth flagging explicitly:** the byte-counting loop over
`gNameCharTable` is a HAND-WRITTEN `while (*p != 0) { p++; count++; }`, not a
`strlen()` call — this project's `-fno-builtin` means GCC 2.6.3 never turns
a `strlen` call into inline code, so retail's identical-looking inline loop
means the SOURCE itself never called `strlen` for this count. Writing
`count = strlen((char *)gNameCharTable);` here would emit an actual `jal
strlen` instruction retail does not have.

**Header changes** (`include/class_3bb8c.h`, additive):
- `Obj86ED0Methods::slot40` (offset 0x040, `void (*)(Obj86ED0 *, s32, s32)`)
  — the ctor's own tail dispatch, forwarding `arg1`/`arg2` unchanged.
- The `ctor` slot comment updated (STALLED → MATCHED); note added that the
  vtable slot's declared type (`s32 arg1`) doesn't have to match this
  function's own more precise definition (`char *arg1`, since it calls
  `strlen` on it) — a data-table function-pointer field and the function it
  points at are two separate things to the compiler; only the actual
  vtable DATA (not compiled from this C) has to agree with what's really
  there.

`TextEntry__ClearChildRefs` (defined later in this same file, ROM order) needed a
forward `extern` declaration above `TextEntry__TextEntry`, same convention as
`DayTask__StartObjM` in `src/world/DayTaskStageMap.c` — otherwise C89's implicit
`int`-returning declaration would conflict with its real `void` definition
further down.

### Proposed learning

None new beyond what's already documented project-wide (the `-fno-builtin`
/ hand-rolled-`strlen`-loop point is a specific instance of the general
"retail's inline-looking code may really be inline SOURCE, not an
optimized-away call" caution, not a new lever).

## Naming

- `TextEntry__TextEntry` -- tier A. The class's own ctor override (classtable.py gTextEntryMethods +0x008), dispatched only by New_TextEntry. `Class__Class` is this project's established constructor convention.

## Track 4 (2026-09-26, round 87)

Class unified as `TextEntry` (include/TextEntry.h; table gObj86ED0Methods -> gTextEntryMethods, type Obj86ED0 -> TextEntry). The class name is for what its methods do: setText keeps a caller's string buffer and a working copy, the cursor and char methods edit the copy, command 25 writes it back, 23 closes without writing (banner of include/TextEntry.h). Fields renamed from their accessors: unk14 charCount, unk1C charIndex, unk20 altCommands, nameLen textLen, nameBuf textBuf, unk28 editBuf, unk40 cursorSprite (CharSprite *), unk44 textRow, unk48 panelSprite (ScreenSprite *). Zero bytes changed.

## Track 7 (2026-09-27, round 98, bravo)

Parameters `arg1`/`arg2` -> `text`/`mode` (the prototype's names).
`strlen` now comes from Sony's `<strings.h>` (libc2) instead of a local
`extern s32 strlen(char *s)`; the K&R declaration returns int, zero bytes
changed.

Moved here from the comment on `gNameCharTable` in src/ui/TextEntryItemList.c:
"VALUE-of `%gp_rel`, round 45's own local view -- same global as
TextEntryItemList's `gNameCharTable` (a byte lookup table whose length this
function counts by hand rather than via `strlen`, since GCC 2.6.3 with
`-fno-builtin` never turns a `strlen` CALL into inline code -- the inline
loop below has to be literal source, not a call)." The source keeps one
line: `MATCHING: counted inline; strlen(gNameCharTable) would be a call.`
