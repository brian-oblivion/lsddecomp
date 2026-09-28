# FileResource__LoadFile

> Renamed from `Class6D430__LoadFile` on 2026-09-26 (tools/rename.py). Address 0x80026b08.

> Renamed from `FileResource__AllocBuffer` on 2026-09-25 (tools/rename.py). Address 0x80026b08.

> Renamed from `func_80026B08` on 2026-09-18 (tools/rename.py). Address 0x80026b08.

**Unit:** GameApplicationFileResource · **Size:** 70 instructions · **Status:** MATCHED (70/70 words, whole-image build verified byte-exact)

## What it does

`gFileResourceMethods`'s vtable slot `+0x058`. A lazy (re)allocation routine: if
`this->unk10` is already set, it's a no-op; otherwise it calls four more of
this class's own slots (`+0x044`, `+0x04C` twice, `+0x054`) — all four are
null at `gFileResourceMethods`'s own level (verified by reading the table's raw words
directly out of `disk/SLPS_015.56`; see `include/GameApplicationFileResource.h`), so they only
resolve to real code for whichever subclass overrides them — sizes a new
allocation via `BMemPMgrAlloc`, and on success installs the new pointer/size
into `this->unk10`/`this->unk14` and restores a temporarily-zeroed field
(`this->unk0C`); on failure it releases a null pointer via `BMemPMgrFree`
and still calls slot `+0x048`.

## Derivation

```
lw    $v0, 0x10($s0)
bnez  $v0, END                 ; if (this->unk10 != NULL) return;
 li   $a2, 1
lw    $s3, 0xC($s0)            ; save this->unk0C
lw    $v0, 0(s0)
sw    $zero, 0xC($s0)          ; this->unk0C = 0
lw    $v0, 0x44($v0)
jalr  $v0                       ; this->methods->slot44(this, arg1, 1, 0)
 li   $a3, 0
...
lw    $v0, 0x4C($v0)
jalr  $v0                       ; size = this->methods->slot4C(this, 0, 2)
 li   $a2, 2
move  $s2, $v0                  ; s2 = size
jal   BMemPMgrAlloc             ; BMemPMgrAlloc(size)  -- ONE ARGUMENT (see below)
 move $a0, $s2
move  $s1, $v0                  ; s1 = newRes
beqz  $s1, FAIL
 ...
; success path: slot4C(this,0,0), slot54(this,newRes,size), slot48(this)
sw    $s1, 0x10($s0)            ; this->unk10 = newRes
sw    $s2, 0x14($s0)            ; this->unk14 = size
j     END
 sw   $s3, 0xC($s0)             ; this->unk0C = saved value
FAIL:
jal   BMemPMgrFree              ; BMemPMgrFree(0)  -- literal NULL, not `this`
 move $a0, $zero
...slot48(this)
END: epilogue
```

## Final C

```c
void FileResource__LoadFile(FileResource *this, s32 arg1) {
    s32 savedUnk0C;
    s32 size;
    void *newRes;

    if (this->unk10 != NULL) {
        return;
    }
    savedUnk0C = this->unk0C;
    this->unk0C = 0;
    this->methods->slot44(this, arg1, 1, 0);
    size = this->methods->slot4C(this, 0, 2);
    newRes = BMemPMgrAlloc(size);
    if (newRes != NULL) {
        this->methods->slot4C(this, 0, 0);
        this->methods->slot54(this, newRes, size);
        this->methods->slot48(this);
        this->unk10 = newRes;
        this->unk14 = size;
        this->unk0C = savedUnk0C;
    } else {
        BMemPMgrFree(NULL);
        this->methods->slot48(this);
    }
}
```

## Attempt log — the real find

First attempt (`BMemPMgrAlloc(size, 0)`, matching the 2-argument signature
already on file in `include/Pad.h`) compiled and linked, but produced
a single-instruction shape mismatch right at the allocator call: retail's
delay slot for `jal BMemPMgrAlloc` is just `move a0,s2`; mine additionally
emitted a **separate, non-delay-slot** `move a0,s2` plus a redundant
`move a1,zero` in the delay slot — i.e. two wrong instructions from supplying
an argument retail's call site never sets up at all.

Cross-checked against `docs/match-reports/New_GameApplication.md` (a different
unit, `GameApplicationFileResource`), which independently derived `BMemPMgrAlloc(0x2C)` — one
argument — for the same function. **The two-argument signature in
`include/Pad.h` (`s32 size, s32 zone`) is wrong**; it was never
exercised against a call site where the phantom second argument's register
happened to differ from whatever was already sitting in `$a1`, so the bug was
invisible there. `include/GameApplicationFileResource.h` now declares the one-argument form
locally with a comment pointing at `New_GameApplication.md` as the confirming
evidence; `Pad.h` is out of this unit's scope to fix, but is flagged
below as a proposed learning / spawn candidate.

Fixing the call to one argument made this function match immediately (was
previously the single largest word-count in this unit's queue and the
worst-drifted function in the build before the fix — `70/70` after).

## Head broadcast levers — applicability

- **goto-vs-return:** not applicable, void return, no differing return
  values on any path (all paths `return;` with no value).
- **loop-invariant hoisting:** not applicable, no loop.
- **prologue store-order barrier:** not applicable, no store-order residue
  observed once the argument-count bug was fixed.

## Proposed learning

**FALSIFIED, round 59 (head, extern review).** The claim below is wrong
about the callee. `BMemPMgrAlloc` genuinely reads TWO argument registers:
`addu $s1, $a1, $zero` at `0x80017B44` saves the incoming `$a1` before
anything writes it, and it is consumed as `addu $t0, $s1, $zero` at
`0x80017B68` on the path taken when the `$gp` default pool is unset. What
the two units below measured is that THEIR OWN call sites pass one argument
and retail emits nothing for a second -- true, and the reason
`include/Pad.h:74`'s one-parameter declaration is byte-correct for
them. It is a fact about those call sites, not about the callee's arity.
See `docs/match-reports/BMemPMgrAlloc.md`, `## Extern arity (round 59)`.

**`BMemPMgrAlloc` takes one argument (`size`), not two.**
`include/Pad.h:74`'s `extern void *BMemPMgrAlloc(s32 size, s32 zone);`
should be corrected to `extern void *BMemPMgrAlloc(s32 size);` — confirmed
independently in two units (`New_GameApplication` in `GameApplicationFileResource`, and this
function). Left unfixed for now since `Pad.h` is outside this unit's
scope; flagged for a spawned follow-up.

## Naming

Round 52 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_80026B08` | `FileResource__LoadFile` | B |

**Track 4 (2026-09-25, FileResource unification): renamed from `FileResource__AllocBuffer`, tier A.** The body opens the file named by its argument (`open(this, name, 1, 0)`), seeks to the end for the size (`seek(this, 0, 2)`), allocates that much, rewinds, reads the whole file into `buffer` and closes: it loads a file. It also occupies +0x058, the slot the CD driver fills with `CdDriver__LoadFile`. The argument is `char *name`, matching `open`'s.

**Evidence.** `+0x058` slot: a lazy (re)allocation routine. No-ops if
`this->unk10` is already set; otherwise sizes and commits a new buffer
through `slot44`/`slot4C`/`slot54`/`slot48` (all null at this class's own
level -- subclass hooks) and `BMemPMgrAlloc`, installing the result into
`unk10`/`unk14` on success. Mechanics fully derived; what the buffer holds
at the base-class level is not (the subclass provides that via the hooks).

## Proposed field names

**APPLIED by the head at merge, round 52** -- all four fields, both types
and all five vtable slots below are now in the tree, each one applied
separately with `./build-and-verify.sh` green and byte-exact after it. One
mis-hit had to be resolved by receiver type: `src/code_179d8_h.c:143`
accesses `pendingGeneration` on a `FileResource *self`, while the same file's
lines 97/174/175/182 are its OWN `ObjA34_179D8H::unk0C` and were left alone.
The compiler named that mis-hit (`structure has no member named 'unk0C'`),
which is the procedure working in the direction where it can work.

`FileResource` (`include/GameApplicationFileResource.h`) is included by `src/code_179d8_h.c`
and `src/code_179d8_q.c` too, and `code_179d8_h.c`'s `FileResource__InstallCdReadDriver`
genuinely reads/writes `self->unk0C` on this exact type (not a same-named
field on a different struct), so per track 3's ownership rule these are
PROPOSED, not renamed:

| field | proposed name | tier | evidence |
| --- | --- | --- | --- |
| `unk0C` | `pendingGeneration` | C | Saved before being zeroed for the duration of the `slot44`/`slot4C` alloc dance, restored on success, left at `0` on failure. Read/write shape of a counter or sequence id, but no read site outside this dance was found to confirm what it counts -- kept speculative (tier C) rather than asserted. |
| `unk10` | `buffer` | B | The lazily-(re)allocated resource itself: obtained from `BMemPMgrAlloc`, released via `BMemPMgrFree`. Mechanics fully known; what the buffer actually holds at this base-class level is not (subclass-specific, via the null hooks). |
| `unk14` | `bufferSize` | B | `unk10`'s allocation size, threaded through the same `slot4C`/`BMemPMgrAlloc` calls. |

Posted to the broadcast for the head to apply (whole-tree replace + oracle,
per FINISHING-PLAN track 3's merge procedure).

## Proposed vtable slot names

`FileResourceMethods`'s slots this function dispatches through are
null at `gFileResourceMethods`'s own level (subclass-provided), and the same
cross-unit exposure applies (`code_179d8_h.c` types objects against this
table too). Proposed, not renamed:

| slot | proposed name | tier | evidence |
| --- | --- | --- | --- |
| `+0x044` (`slot44`) | `configureBuffer` | C | Called as `(this, arg1, 1, 0)` before sizing; `arg1` is caller-supplied (an identifier or key), the two constants look like a mode/flag pair. Mechanics of the call site known, the callee's behaviour (null here) is not. |
| `+0x04C` (`slot4C`) | `bufferControl` | C | Called twice with different second/third args -- `(0, 2)` to obtain `size`, `(0, 0)` after a successful alloc -- reading like a generic opcode-style control method rather than a plain getter. |
| `+0x054` (`slot54`) | `installBuffer` | C | Called as `(this, newRes, size)` right after a successful allocation -- installs/commits the new buffer into whatever subclass-specific bookkeeping exists. |
| `+0x048` (`slot48`) | `onBufferChanged` | C | Called on both the success and failure paths of this function, and also from `FileResource__Finalize` -- a notification/finalize hook rather than part of the alloc logic itself. |

All four are tier C: the CALL SITES are fully derived, but every one of
these slots is null at `gFileResourceMethods`'s own level, so nothing here confirms
what an overriding subclass's implementation actually does. Posted to the
broadcast.

## Extern arity (round 59)

**Verdict: arity-ok idiom.** `src/code_179d8_s.c`'s `(void)` declaration stays.

**Callee evidence** (`0x80026B08`, and the matched definition in
`src/GameApplicationFileResource.c`): the body reads BOTH argument registers before writing
them — `move s0,a0` at entry, and `$a1` is still the incoming `arg1` when it is
forwarded to `this->methods->configureBuffer(this, arg1, 1, 0)` at `0x80026B48`
(only `$a2`/`$a3` are re-set there, with `li a2,0x1` / `move a3,zero`). Two
real arguments.

**Why the `(void)` extern is right anyway.** The one caller,
`CdDriver__LoadFile` (this unit, matched), passes nothing at all:

```
8002780c:  move  s0,a0          <- its own self, only spilled
80027814:  move  s1,a1          <- its own arg1, only spilled
80027834:  jal   80026b08 <FileResource__LoadFile>
80027838:  nop                  <- no argument setup, in retail
```

`$a0` and `$a1` still hold `CdDriver__LoadFile`'s own incoming arguments, which the
callee then consumes. This is the textbook dead-argument idiom: byte-exact
either way, and writing the two arguments out at the call site would change
nothing *only* if the values happened to match — they do here by accident of
register allocation, which is precisely why the declaration must not be
"corrected" to two parameters and the call site must not grow arguments.

**Declaration sites changed:** none (arity unchanged). `/* arity-ok: ... */`
added to `src/code_179d8_s.c:273`. Oracle green.
