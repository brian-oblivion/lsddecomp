> Renamed from `func_8004EDC0` on 2026-09-20 (tools/rename.py). Address 0x8004edc0.

# TaskObjF__TryReadMemcardFile

**Unit:** class_3bb8c_f · **Size:** 56 words (0xE0) · **Status:** MATCH

## What it does

`s32 TaskObjF__TryReadMemcardFile(TaskObjF *self, char *suffix, void *outBuf, s32
outSize)`. Builds a memory-card path via `BuildMemcardPath` (this unit,
matched: `self->unk0C` selects the "bu00:"/"bu10:" device, `suffix` is the
filename), opens it (`func_80050938(path, 1)`), and on success reads a
0x80-byte header into a freshly-allocated scratch buffer, computes a seek
offset from the header's byte 2 (`(header[2] << 7) - 0x780`), seeks,
reads `outSize` bytes into the caller's `outBuf`, and closes the handle.
Returns 0 if the open failed, 1 otherwise. `TaskObjF__ReadMemcardFile` (this unit,
matched) is the bounded-retry wrapper around this function.

Flagged predicted-hard by the register census (5 distinct callee-saved
registers) — matched, needing two levers.

## Levers

1. **Use `BuildMemcardPath`'s own RETURN VALUE as the path argument to
   `func_80050938`, don't re-derive the address from the local buffer
   variable.** `BuildMemcardPath` returns the same pointer it was given
   (like `strcat`), so `path = BuildMemcardPath(&pathBuf, ...); open(path,
   1);` reuses the value already sitting in `$v0` after the call. Calling
   `open(pathBuf, 1)` directly (recomputing `&pathBuf` as `$sp+0x10`)
   costs one extra `addiu` — this class of lever ("prefer the call's own
   return over re-deriving an equal value") is now confirmed twice in
   this unit family (also see `BuildMemcardPath`'s own report on the
   opposite direction: computing the pointer once, not per-branch).
2. **The seek-offset arithmetic must be ONE fully-resolved expression
   BEFORE the intervening `free()` call, not split across it.** The raw
   disassembly interleaves the shift and the `-0x780` subtract around the
   `func_80017CFC(hdr)` free call (the shift lands before the call, the
   subtract in the call's own delay slot) — reproducing that exact
   interleaving from C proved unnecessary and actively wrong: writing
   `seekPos = raw << 7; free(hdr); seekPos -= 0x780;` (matching the
   asm's own physical layout literally) left the function 1-2 words
   SHORT under every ordering tried, because GCC re-derives the shift
   result fresh right next to its use instead of keeping it alive across
   the call in a subtract-only delay slot. Computing the WHOLE expression
   first — `seekPos = (raw << 7) - 0x780;` — then calling `free(hdr)`
   afterward reproduces retail's exact interleaving on its own (GCC's own
   scheduler moves the independent `free()` call and constant-folds/
   schedules the arithmetic around it identically). The general
   takeaway: don't transcribe an asm interleaving around an unrelated
   call literally — write the source-level DATA DEPENDENCY (one
   expression, computed once) and let the scheduler interleave it.

## Header additions (`include/class_3bb8c.h`, additive only)

- `TaskObjF::unk0C` carved out of the existing `pad04[0x14-0x04]` gap (now
  `pad04[0x0C-0x04]` + `unk0C` (s32) + `pad10[0x14-0x10]`) — no other
  offset moves.
- `extern s32 func_80050938(char *path, s32 mode);`
  `extern s32 func_80050928(s32 handle, void *buf, s32 size);`
  `extern s32 func_800508E8(s32 handle, s32 pos, s32 whence);`
  `extern s32 func_800508F8(s32 handle);` — a CD-stream open/read/seek/
  close family, all **externs for functions outside this unit** (no other
  header in the project declares them yet; typed purely from these call
  sites' own register usage).

See `TaskObjF__ForEachEvent`'s report for the `TaskObjF` class context.
