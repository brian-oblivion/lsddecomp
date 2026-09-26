# FileResource__Finalize

> Renamed from `Class6D430__Finalize` on 2026-09-26 (tools/rename.py). Address 0x80026ab4.

> Renamed from `FileResource__Destroy` on 2026-09-25 (tools/rename.py). Address 0x80026ab4.

> Renamed from `func_80026AB4` on 2026-09-18 (tools/rename.py). Address 0x80026ab4.

**Unit:** code_171e0 · **Size:** 21 instructions · **Status:** MATCHED (21/21 words, whole-image build verified byte-exact)

## What it does

This class's own destructor — `gFileResourceMethods`'s vtable slot `+0x00C`, called
`FileResource__Release` and `Pad__Finalize` (in `class_16334`, a different class'
`dtor`, same convention) alike. It calls two more of its own slots in turn:
`+0x048` (unimplemented/null at this level — a subclass-provided hook,
`slot48`) and then `+0x05C`, which happens to resolve to `FileResource__FreeBuffer` *at
this class's own level*, but is still dispatched indirectly through the
table, never called by name, since retail's bytes are `jalr`, not `jal`.

## Derivation

```
lw    $v0, 0x0($s0)      ; v0 = this->methods
lw    $v0, 0x48($v0)     ; v0 = methods->slot48
jalr  $v0                ; this->methods->slot48(this) -- return discarded
lw    $v0, 0x0($s0)      ; reload this->methods
lw    $v0, 0x5C($v0)     ; v0 = methods->slot5C
jalr  $v0                ; return this->methods->slot5C(this)
 addu $a0, $s0, $zero
```

No explicit `addu v0,zero,zero` after the last call — `$v0` carries straight
through from the final `jalr` into the epilogue, so per CLAUDE.md's
"one-line tail-call" guidance this is written as `return ...(...)`, not a
bare call-then-return-something-else.

## Final C

```c
void *FileResource__Finalize(FileResource *this) {
    this->methods->slot48(this);
    return this->methods->slot5C(this);
}
```

## Attempt log

Two attempts. The first (correct call sequence) showed 19/21 in-range, with
both mismatches at `lw $v0, N($v0)` instructions where `N` was wrong (`0x14`
and `0x20` instead of `0x48` and `0x5C`) — not a control-flow or register
problem at all, but a **struct-layout bug**: `FileResourceMethods` in
`include/code_171e0.h` declared `slot44`/`slot48`/`slot4C`/`slot54`/`slot5C`
immediately after `dtor` with no padding, so the C struct actually placed
`slot44` at byte offset `0x10`, not the intended `0x44` — the `/* +0x44 */`
comments were fiction the compiler never saw. Adding explicit `u8 padN[...]`
members to close the gaps (`+0x10`..`+0x44`, `+0x50`..`+0x54`, `+0x58`..`+0x5C`)
fixed every affected function in the unit at once (this one, and
`FileResource__LoadFile` below) on the next build. This is a sharper restatement of
CLAUDE.md's "name the field, don't do raw pointer arithmetic" — the risk runs
the other way too: naming fields with offset *comments* that aren't backed by
real padding is silently worse than pointer arithmetic, because the mistake
is invisible in a source diff and only shows up as a wrong immediate in the
generated `lw`.

## Head broadcast levers — applicability

- **goto-vs-return:** not applicable, no branches at all.
- **loop-invariant hoisting:** not applicable, no loop.
- **prologue store-order barrier:** not applicable.

## Proposed learning

See "Attempt log" above — promoted to a project-wide note: **a struct field
declared with an `/* +0xNN */` offset comment is not actually at that offset
unless the preceding fields' sizes (or explicit padding) add up to it.**
`funcdiff`/`asm-differ` catches this immediately as an `lw`/`sw` immediate
mismatch with the *same instruction shape* (same opcode, same registers,
wrong constant) rather than a structural residue — that specific fingerprint
(correct opcode/registers, wrong small immediate, on a `$v0`-relative load
right after a `this->methods` load) should be checked against the header's
struct layout *before* being treated as a real residue class.

## Naming

Round 52 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_80026AB4` | `FileResource__Finalize` | B |

**Track 4 (2026-09-25, FileResource unification): renamed from `FileResource__Destroy`, tier A.** Occupies BasicClass's `finalize` slot (+0x00C): `close` then `freeBuffer`. Now `void`, like the base slot; the old `void *` returned `freeBuffer`'s (void) result and was byte-identical either way.

**Evidence.** The class's own destructor, `+0x00C` slot (the `dtor` field
already named in `FileResourceMethods`). Calls the subclass-overridable
hook `slot48` (null at this level) then tail-calls `slot5C`
(`FileResource__FreeBuffer` at this class's own level, though dispatched
indirectly). Mechanics known (release the hook, then the buffer); whether
this specific sequencing has a game-visible purpose beyond "the dtor" is
not established.

## Proposed vtable slot names

| slot | proposed name | tier | evidence |
| --- | --- | --- | --- |
| `+0x05C` (`slot5C`) | `freeBuffer` | B | Resolves to `FileResource__FreeBuffer` at this class's own level (confirmed by dumping `gFileResourceMethods`'s raw words), dispatched indirectly. Per the project's "vtable slots named like the method they dispatch to" convention. Proposed rather than renamed only because the slot's declaration lives in the shared, cross-unit-exposed `FileResourceMethods`. |

See `FileResource__LoadFile.md` for `+0x048`'s proposal (`onBufferChanged`),
also dispatched from this function. Posted to the broadcast.
