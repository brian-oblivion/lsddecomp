# SceneNode__GetNextAttachedChild

> Renamed from `Class6B5CC__GetNextAttachedChild` on 2026-09-26 (tools/rename.py). Address 0x8001d280.

> Renamed from `func_8001D280` on 2026-09-23 (tools/rename.py). Address 0x8001d280.

**Unit:** code_d294 · **Size:** 47 words · **Status:** MATCHED (47/47 words)

## What it does

`SceneNode` vtable slot `+0x058`, `SceneNode__DetachAttachedChildren`'s (`+0x054`) own call
target -- searches an intrusive list (starting from `self->unk4`, a
BasicClass-owned field this unit reads directly) for the first entry whose
vtable header tag is `4` AND whose own `+0x00C` field equals `self`, using
the generic list-pop helper `GetNextBasicClass` (a DIFFERENT still-uncarved
unit, `TmdRenderer.s`) to advance. On a match, leaves `*entry` pointing at
it and returns. If the list runs out first, sets `*entry = NULL`.

`entry` and `cursor` are the SAME two stack slots `SceneNode__DetachAttachedChildren` passes
by reference on every iteration of its own loop -- `cursor`'s truthiness
is what that caller's loop treats as "keep going", but inside THIS
function it is the actual list-traversal cursor, not a boolean. Two
different, both byte-exact, local readings of the identical memory --
same "per-call-site typing" situation as `GetSceneNodeMethods` (see this unit's
round-1 reports).

## The C

```c
void SceneNode__GetNextAttachedChild(SceneNodeObj *self, GenericObj_d294 **entry, GenericObj_d294 **cursor) {
    s32 tag;

    tag = 4;
    do {
        if (*entry == NULL) {
            *cursor = self->unk4;
        }
        GetNextBasicClass(entry, cursor);
        if (*entry != NULL) {
            if ((((*entry)->methods->header) & 0xF) == tag) {
                if ((*entry)->unkC == self) {
                    return;
                }
            }
        }
    } while (*cursor != NULL);
    *entry = NULL;
}
```

## Provenance

round 11 (2026-09-03), runner charlie, unit code_d294, second pass. Matched on the first build (no
iteration needed) -- the most complex function attempted this round, and
the one whose control-flow was transcribed most literally from the
disassembly's own branch structure (three nested `if`s rather than a
combined `&&`, to keep each branch target lined up with retail's own).
Established `GetNextBasicClass`'s call shape from this unit's own vantage,
`SceneNodeObj::unk4` (split out of the previously-opaque BasicClass field
blob), and `GenericObj_d294::unkC`.

## Naming

Round 71 (alpha). `func_8001D280` -> `SceneNode__GetNextAttachedChild`, **tier A**. Table slot +0x058. Walks self->children (BasicClass +0x004, seeded when *entry is NULL) with GetNextBasicClass and stops at the next child whose tag is 4 (SceneNode family) and whose +0x00C parent pointer is self; sets *entry = NULL when the list ends. A list iterator whose mechanics are its purpose.

## Proposed field names

For the head to apply by type scope. Each one fails to compile in another unit when renamed in the definition, so this unit did not apply it.

- `GenericObj_d294.unkC` -> `parent` (tier A): the same +0x00C parent pointer on the child, compared against self to pick attached children. Accessors: code_d294, code_d294_c, plus a NON_MATCHING body in code_d294_b.

## Track 4 (2026-09-25, round 81, charlie)

The signature is now `(SceneNode *self, SceneNode **entry,
BasicClassListNode **cursor)`. The entries it returns are tag-4 children
whose `parent` is `self`, and SceneNode__DetachAttachedChildren calls their
detachFromParent, so they are SceneNodes. `cursor` is the BasicClass child
list cursor. DetachAttachedChildren's `s32 cont` became that cursor, which
it only tests for NULL. Byte-identical.
