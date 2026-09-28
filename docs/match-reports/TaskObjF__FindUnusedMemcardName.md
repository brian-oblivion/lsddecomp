# TaskObjF__FindUnusedMemcardName — MATCH (43/43 words)

> Renamed from `func_8004EADC` on 2026-09-24 (tools/rename.py). Address 0x8004eadc.

**Unit:** class_3bb8c_c (round 14, `Node3bb8cE` class).

## What it does

`char *TaskObjF__FindUnusedMemcardName(Node3bb8cE *self, char *buf, char *middle, char
**entries)`. Walks a NULL-terminated array of candidate string suffixes
(`entries`): for each, builds `buf = middle + entries[i]` (via
`strcpy`/`strcat`) and tests it with `self->methods->slot54(self, 0,
buf)` — a real call through the object's OWN vtable (offset 0), not the
separately-fetched base-class table the rest of this unit dispatches
through. A zero return from `slot54` means "match": return `buf`
immediately. Exhausting the array without a match returns `NULL`.

## Where it stood, and the fix

First attempt (10/43, ~126KB drift) used `if (*entries == NULL) return
NULL; do { ... } while (*entries != NULL);` — a leading guard plus a
do-while. That produces an extra, dead `move a0, s1` instruction right
before the loop body that retail doesn't have, and is the wrong total
length. **The real shape is a plain top-tested `while` loop**, not an
`if`-guarded `do-while` — even though the two are logically equivalent
here (both test `*entries` before ever entering the body), they compile
to different code: the `while` form's test-then-branch-to-end sequence
IS retail's bytes.

```c
char *TaskObjF__FindUnusedMemcardName(Node3bb8cE *self, char *buf, char *middle, char **entries)
{
    while (*entries != NULL) {
        strcpy(buf, middle);
        strcat(buf, *entries);
        if (self->methods->slot54(self, 0, buf) == 0) {
            return buf;
        }
        entries++;
    }
    return NULL;
}
```

### Header/struct change

`Node3bb8cE`'s offset 0 was previously undifferentiated padding
(`pad00[0x00C]`); this function proves it's a real vtable pointer (the
object's OWN methods table, distinct from the base-class table obtained
via `Get_vtable_BasicClass()`). Added `SelfMethods3bb8cE` (local to
`src/class_3bb8c_c.c`) with only the one reached slot, `+0x054`.

### Proposed learning

**An early-exit `if` immediately followed by a semantically-equivalent
loop condition is not interchangeable with a plain top-tested loop**,
even when the guard exactly restates the loop's own condition. This is a
new instance of the project's established "guard clause vs. loop shape"
family (see DECOMPILATION_LEARNINGS' `while`/`do-while`/post-increment
entries) — here the tell was a spurious extra `move` and the wrong total
instruction count, not a branch-target or value mismatch, so it read at
first like "something is subtly wrong with the loop" rather than "this
is a `do-while`, try `while` instead."

## Naming (round 78, track 3)

`func_8004EADC` -> `TaskObjF__FindUnusedMemcardName`. **Tier B.** Sits at `gTaskObjFMethods` +0x058. Walks a NULL-terminated `entries` array of candidate suffixes, builds `buf = middle + entries[i]`, and returns the first candidate for which `self->methods->slot54(self, 0, buf)` reports **not found** (`== 0`, `destBuf` NULL so existence-only). Mechanically this returns the first candidate name that does NOT already exist on the card -- named for that mechanism ("unused"), not for an assumed purpose (e.g. "next free save slot"), which the body alone does not establish; tier B.
