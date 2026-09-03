# func_80017CFC

**Unit:** code_8220 · **Size:** 109 instructions · **Status:** BLOCKED, not attempted

This is a **stub report**, filed so `tools/progress.py` stops counting this
function as fresh ground and staffing a runner onto it. It records a routing
decision, not an attempt.

## Why it is blocked — gp-relative

```
lw $t0, %gp_rel(D_8008A818)($gp)
```

One `%gp_rel` reference, to `D_8008A818` (see `func_80017A9C.md`,
`func_80017AC8.md`, `func_80017B34.md` — same global). This is the
**gp-relative addressing blocker**, `docs/research/gp-relative-blocker.md`:
the pinned `-G0` pipeline emits the two-instruction absolute (`lui`+`lw`)
form where retail has the one-instruction `$gp`-relative form. The `-G`
experiment was run on 2026-08-29 with operator authorisation and REJECTED —
a clean non-zero-`-G` rebuild damages 19148 bytes, and `-G4`/`-G8` damage
identically, ruling out the size threshold as the mechanism. The pin stays
at `-G0`.

## What it does — this is the pool `free`, one-argument, already
## established project-wide

`func_80017CFC(void *ptr)` — the generic pool deallocator, mirror of
`func_80017B34` (see that report). **Already declared, one-argument,
returning `void *`, in `include/code_55dd4.h`** (`extern void
*func_80017CFC(void *ptr);`) and used there as `self->unk74 =
func_80017CFC(self->unk74);` — the "free and null out the field in one
statement" idiom, which only makes sense if this function reliably returns
`NULL`. Other units (`include/code_171e0.h`, `include/Entity.h`) instead
declare it `void`, discarding the return; both are compatible callers of
the same underlying function per this project's "multiple independent
views" convention (see DECOMPILATION_LEARNINGS). This unit's own
`BasicClass__func_17eb0` (matched) is consistent with the `void *`,
always-`NULL` reading: it calls `func_80017CFC(self)` and then
unconditionally overwrites `$v0` with `0` before returning, regardless of
what this function itself would have returned.

Internally: same critical-section-lock shape as `func_80017B34`
(`func_8001844C(1)`/`func_8001844C(0)`), then walks back from `ptr` to the
block's header (`ptr-4`), and depending on the header's high bit
(`$a0 bgez`, i.e. whether this looks like an in-use vs. free-adjacent
block) either coalesces with an adjacent free block (updating the free
list's prev/next links via the pool struct's `$t0`-based head/tail fields
at `+0xC`/`+8`, same fields `func_80017B34` uses) or inserts the freed block
as a new standalone free-list entry. The size masking (`0xFFFFFFF`) and
flag bits match `func_80017B34`'s header format exactly, confirming both
functions share one packed-header free-list design.

## Do not re-derive this

Both blockers are already escalated with reproducers and corpus censuses
attached. Do not spend attempts here, do not propose a toolchain change, and
do not classify a residue from this construct as a scheduling or delay-slot
choice. Check cheaply before attempting any function:

```sh
grep -n 'gp_rel' asm/nonmatchings/<unit>/<func>.s
grep -n 'addiu *\$at, *\$at, *%lo' asm/nonmatchings/<unit>/<func>.s
```
