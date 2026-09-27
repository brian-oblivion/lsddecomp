# BasicClass__Release

> Renamed from `BasicClass__func_17eb0` on 2026-09-24 (tools/rename.py). Address 0x80017eb0.

**Unit:** BMemPMgr · **Size:** 18 instructions · **Status:** MATCHED (18/18 words)

BasicClass vtable slot `+0x004` — see `BasicClass__BasicClass.md` for the
class's overall design (two pool-allocated linked lists, `children` and
`parentRefs`).

## What it does

The base "release"/destroy method: dispatches the virtual finalize hook
(`self->methods->finalize`, this unit's own `BasicClass__Finalize`, slot
`+0x00C`) to let a subclass tear down its own state, then frees `self`
itself back to the pool via `BMemPMgrFree` (one argument — see that
function's own stub report for why), and always returns `NULL`.

## The C

```c
void *BasicClass__Release(BasicClass *self)
{
    self->methods->finalize(self);
    BMemPMgrFree(self);
    return NULL;
}
```

## Signature: one argument, `void *` return — resolved from the CALL, not
## from the callee alone

Two things had to be gotten right before this compiled to the right shape:

- **One argument, not two.** A first reading assumed a second parameter
  (a "pool" to forward into `BMemPMgrFree`, since `$a1` is never
  explicitly set before that call and per DECOMPILATION_LEARNINGS "a value
  in an argument register that survives is a genuine argument" this looked
  like the documented positive case). It is not — `BMemPMgrFree` is
  already established project-wide as ONE argument (see its stub report),
  and the unread `$a1` here is exactly DECOMPILATION_LEARNINGS' NEGATIVE
  case instead: a register dead at the next call, carrying no real meaning.
  The tell: this function explicitly overwrites `$v0` to `0` with its OWN
  final instruction, right after the `BMemPMgrFree` call and NOT in that
  call's delay slot — so whatever `BMemPMgrFree` itself returns is
  discarded here regardless, which is only sensible if the second register
  it might have depended on was equally irrelevant.
- **`void *`, matching `BMemPMgrFree`'s own return type**, not `void`.
  The explicit `addu $v0,$zero,$zero` after the call is a real, deliberate
  return-value write (not incidental fallthrough), so `return NULL;` is
  correct rather than a bare `return;`.

## Provenance

round 11 (2026-09-03), runner delta, unit code_8220 (fresh carve). Matched
first attempt once `code_8220.h`'s `BasicClass`/`BasicClassMethods` types
existed.

## Naming (round 74)

`BasicClass__Release`, **tier A**: matches vtable slot `+0x004`
(`release`), already documented in `code_8220.h`'s `BasicClassMethods`
comment. Dispatches `finalize` then frees `self` via `BMemPMgrFree`;
always returns `NULL`.
