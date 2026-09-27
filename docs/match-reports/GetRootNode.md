# GetRootNode — MATCHED

> Renamed from `Unk18Obj__GetTail` on 2026-09-25 (tools/rename.py). Address 0x8003f25c.

> Renamed from `func_8003F25C` on 2026-09-23 (tools/rename.py). Address 0x8003f25c.

Unit: `Task`. Round 14, runner delta. 12/12 words, full match (3
real attempts).

## Signature

```c
Unk18Obj *GetRootNode(Unk18Obj *self);
```

Not a `gViewportMethods` vtable slot (not present in `tools/classtable.py`'s
output for that table) — called directly by symbol elsewhere.

## What it does

Walks `self->unkC` (cast through to `Unk18Obj *`, a self-referential
"next" field per this unit's existing per-call-site typing convention —
`unkC` is otherwise typed `GenericObj *` from round 13's `Viewport__AddChild`)
to find the tail of a singly-linked list rooted at `self`. Returns `self`
itself unchanged if `self->unkC` is already `NULL`.

```c
Unk18Obj *GetRootNode(Unk18Obj *self) {
    while (self->unkC != NULL) {
        self = (Unk18Obj *)self->unkC;
    }
    return self;
}
```

## What the first two attempts got wrong

The disassembly's own shape (an initial guard, `if (self->unkC == 0) return
self;`, followed by a loop that ALWAYS advances once before its own
condition check) reads naturally as a `do`/`while` with an upfront guard —
that was the first attempt, and it compiled to an EXTRA `j` at the very top
(jumping past the loop's first, unconditional advance straight to the
loop's own condition test), one word longer than retail and consequently
not aligned with it at all past word 0. A second attempt spelling the same
thing with explicit `goto`/labels produced BIT-IDENTICAL output to the
`do`/`while` version — consistent with this round's `SceneNode__TryAttachNearby`
finding (previous unit) that `goto` does not reliably escape a GCC 2.6.3
merge/rotation decision once the compiler has already decided on one.

**The actual fix was to recognize the guard as redundant and drop it
entirely.** `self->unkC == NULL` on entry and a zero-iteration `while` loop
produce the identical `return self` — so `while (self->unkC != NULL) self
= self->unkC; return self;` is behaviorally identical to the
guard-plus-do-while version, and it is what retail's own instruction
sequence actually reproduces: GCC's own loop-rotation pass turns a plain
`while` into "test once up front, then do/while a body that re-tests",
which is precisely retail's shape. Attempt 1 essentially wrote the
ROTATED FORM by hand (duplicating the guard test explicitly) and gave GCC
nothing to rotate, so it added its OWN redundant jump on top.

## Proposed learning

**When a loop's own natural zero-iteration behavior already gives the same
result as a separately-written upfront guard, write the plain loop and let
GCC's own rotation produce the guard — do not hand-write the rotated
form.** A hand-written guard-plus-do-while is not simply "the same shape
retail's assembly shows" even when the disassembly visually looks like
one; GCC gets to that shape by ROTATING a `while`, and handing it an
already-rotated source can make it add a further redundant jump on top
rather than recognizing the guard as already covered by the loop.

## Naming

`Unk18Obj__GetTail` -- tier A. Walks `self->unkC` while non-NULL to find the tail of a singly-linked chain rooted at `self`; a pure leaf whose mechanics (list-tail walk) are its whole purpose.

## Track 4 (2026-09-25, round 85, bravo)

Renamed from `Unk18Obj__GetTail`. Not a Viewport method: it is in no table, and its one caller, Viewport__Update, passes it the view node, a SceneNode. The chain it walks is +0x00C, SceneNode's `parent` (DrawNode's `parent` too), so it returns the top of the node's hierarchy: renamed GetRootNode, typed `SceneNode *GetRootNode(SceneNode *node)`, byte-identical. The Viewport class is unified in `include/Viewport.h` (round 85).
