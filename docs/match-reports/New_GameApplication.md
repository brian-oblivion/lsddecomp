# New_GameApplication

> Renamed from `New_Class6D3C8` on 2026-09-26 (tools/rename.py). Address 0x80025f7c.

> Renamed from `new_class_6d3c8` on 2026-09-24 (tools/rename.py). Address 0x80025f7c.

**Unit:** code_1677c · **Size:** 24 words (0x60 bytes) ·
**Status: MATCHED 24/24**, whole-image SHA1 green. Closed by the head in
round 8 (2026-09-02) with the project's first permuter run.

> **This report is kept in full, including three rounds of negative results
> that are now superseded.** They are what made the permuter case, and the
> class of residue they map out is real; only the conclusion "no source-level
> lever exists" was wrong. Read the RESOLUTION section first — everything below
> it under "Derivation", "Residue" and "Round-3 follow-up" is the state of
> knowledge BEFORE the fix, preserved deliberately.

## RESOLUTION — the matching form

```c
GameApplication *New_GameApplication(GameApplicationCtorArgs *arg) {
    GameApplication *self = BMemPMgrAlloc(0x2C);

    if (self != 0) {
        ((GameApplicationMethods *)GetGameApplicationMethods())->ctor(self, arg);
        return self;
    }
}
```

**The `return` moves INSIDE the `if`, and the null path falls off the end of
a non-void function.** That is the whole fix, and it is why every reshaping
attempt in the three rounds below failed: they all kept a `return` on the
null path, and *any* `return` there costs the instruction. `BMemPMgrAlloc`
already left the null in `$v0`, so the original source never had to restate
it — which is exactly why retail's `beqz` delay slot is a bare `nop`.

The residue was never a delay-slot *filler* problem. It was a redundant
value materialisation: our C asked for a value retail's C never asked for.
The delay slot was only where the compiler happened to put it.

**Measured against retail, every neighbouring spelling** (harness: the pinned
pipeline into `mipsel-linux-gnu-objdump`, instruction-text diff against the
assembled retail `.s`):

| source form | diff |
| --- | --- |
| `if (p != 0) { ctor(); return p; }` — no return on the null path | **0** |
| `if (p == 0) return 0;` then body, `return p` | 4 (27 insns: second epilogue) |
| `if (p == 0) return p;` then body, `return p` | 4 |
| `if (p != 0) ctor(); return p;` (the old 23/24) | 2 |
| `if (p != 0) { ctor(); return p; } return 0;` | 2 |
| `if (p != 0) { ctor(); return p; } return p;` | 2 |

Only the fall-through form reaches zero. GCC 2.6.3 warns *"control reaches
end of non-void function"* on it; the warning is correct about the C, and the
bytes are what say the original source had it anyway. **Do not "fix" the
warning** — doing so un-matches the function.

### How it was found, and what that cost

`tools/decomp-permuter` had never been run on this project (MATCHING-GUIDE
said "Not yet set up"). The head set it up this round against this exact
function — the target MATCHING-GUIDE had already nominated — and it reached
score 0 at **iteration 47**, in well under a minute of search. Three rounds
of careful manual reshaping, 20+ recorded attempts, and a root-cause
hypothesis read against GCC's `reorg.c` had not found it.

The transferable lesson is not about `reorg.c`. It is that **a residue
described as "the compiler chose a different delay-slot filler" can actually
be "our source computes one more value than the original did"** — and those
two look identical in a diff, because the surplus value has to go *somewhere*
and a free delay slot is where the scheduler puts it. The hypothesis below
was mechanically detailed and confidently argued, and it pointed away from
the fix.

See the `### Proposed learning

**Superseded by the RESOLUTION above — the permuter run it asked for was made
this round and it closed the function.** The prediction that one source shape
would close the whole class was correct; the shape is `return` inside the
conditional with no return on the other path.

The durable generalisation, promoted to DECOMPILATION_LEARNINGS.md:

1. **A single redundant `move $v0, <reg>` — in a branch delay slot or just
   before the epilogue — where retail has `nop` or nothing means the source
   restates a return value the original never restated.** The value is
   already in `$v0` from a preceding call. Do not reshape control flow;
   remove the restatement, which usually means moving the `return` inside the
   conditional and letting the other path fall off the end of a non-void
   function.
2. **"Different delay-slot filler" and "one surplus value" are
   indistinguishable in a diff**, because a surplus value lands in whatever
   slot is free. Prefer the surplus-value reading first: it has a source-level
   fix, and the scheduling reading does not.
3. `New_X` allocator wrappers (malloc → null check → ctor through slot
   `+0x008` → return the allocation) recur across this game's ~60 classes.
   This form is now the one to write FIRST for every one of them.

## Round-3 follow-up (head-directed, 5-attempt budget, all negative)

The head asked for a targeted follow-up after two other round-3 stalls
turned out to be the SAME one-instruction class (`Pad__DispatchEvents`'s
broadcast #1/#2, and the second instance broadcast #3 adjudicated), with an
explicit instruction to check branch targets FIRST (broadcast #3) before
spending any attempts, since a differing branch TARGET (not just a delay
slot) means the source shape is still wrong — not a compiler-internal
stall.

**Branch-target check, performed first:** this function has exactly one
branch (`beqz $s0, .L80025FC4`), and every C shape tried — including all
of the new ones below — reaches that same target address (confirmed via
`asm-differ`; the only diff line in the best attempts is the delay slot's
content, never the branch's destination). Per broadcast #3's own test, this
means the CFG is right and there is nothing here for a wrong-shape fix to
correct — consistent with the existing diagnosis, not a new lead.

Five attempts spent, all at 23/24 or worse (never better):

1. **`if (self == 0) goto fail; ...; return self; fail: return 0;`** — the
   specific untried spelling the head flagged (goto target returns a
   DIFFERENT value from the fallthrough, unlike the previously-tried
   `goto done; done: return self;` where both paths shared one return).
   Needed one incidental fix along the way: calling
   `GetGameApplicationMethods()->ctor(...)` directly (rather than through an
   intermediate `self->methods = GetGameApplicationMethods();` assignment as in
   `GameApplication__GameApplication`) requires an explicit `(GameApplicationMethods *)` cast,
   since `GetGameApplicationMethods` returns bare `void *` and dereferencing a `void *`
   member is a hard error, not just a warning, in this compiler. **Result:
   23/24, identical residue** (delay slot: retail `nop`, ours
   `move v0,s0`).
2. **Inverted goto direction** (`if (self != 0) goto construct; return 0;
   construct: ...`) — 15/24, WORSE: this shape doesn't merge the two exits
   into one epilogue at all (grows by 2 words, matching the general
   "early return with a separate epilogue" trap documented for
   `GameApplication__PollGraphRoomStatus`).
3. **Bare `__asm__("")` as the very first statement of the function**
   (before the `malloc` call) — the one position broadcast #2 said closed
   an unrelated function's residue, and the existing report here had only
   tried it after the malloc call and inside the `if` body, not before
   everything. Result: 20/24, WORSE — it reordered two PROLOGUE
   instructions (still just order, not register identity, so within the
   CLAUDE.md rule 6 line) without touching the actual residue at all.
4. **`volatile` qualifier on `self`** — forces a reload from memory on
   every use instead of trusting the register. Result: 4/24, much WORSE
   (forces spurious stack traffic throughout, an obviously wrong lever in
   hindsight, but cheap to rule out).

**Conclusion: unchanged from the existing diagnosis.** All three
of `if`/`goto`/temp-variable reshaping (already exhausted before this
round), the goto-with-different-return-value spelling (new this round),
and the pre-malloc `__asm__("")` position (new this round) fail to move
this residue, and the branch target agrees with retail throughout. This is
the SAME one-delay-slot class as `Pad__DispatchEvents`'s two now-adjudicated
instances, just with retail choosing `nop` where those chose a
value-restating `move` — a THIRD confirmed instance of the class, and if
anything a stronger permuter case for it (three independent occurrences
now, not two). The permuter-target recommendation above stands unchanged.

## Naming

**`New_GameApplication` -- already named (pre-round-77), tier A.** The `New_X`
allocator for `GameApplication` (`BMemPMgrAlloc` + dispatch through the class's
own ctor slot). Left as-is this round -- it predates this naming pass and
already follows the project's `New_X` convention, just lowercase/underscore
because it was named before the class's own type name (`GameApplication`) was
established from `classtable.py`. Not renamed to `New_GameApplication` here:
that would be a cosmetic-only rename with no new evidence behind it, and
picking it up is better left to whoever names `GameApplication` itself formally
(this runner did not establish a game-purpose name for the class, only its
`ClassXXXXX`-by-table-address identity).
