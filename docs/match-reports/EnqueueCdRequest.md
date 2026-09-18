> Renamed from `func_800282AC` on 2026-09-17 (tools/rename.py). Address 0x800282ac.

# EnqueueCdRequest — MATCHED (32/32 words)

Round 45, runner echo (second sitting), `src/code_179d8_q.c`.

## Result

Byte-exact on the first attempt.

The body below is the round-51 source, after track-3 naming. The
derivation notes that follow were written in round 45 against the same
code under its `unk` names; only names changed, the image is
byte-identical, and the `## Naming` section at the end of this report
carries the evidence for each one.

```c
/* The same 0x24-byte queue node CdRequest_D70 above is a view of, from the
 * writing side: AllocCdRequestNode (code_179d8_r) allocates one and links it onto
 * D_8008A894, and only the fields this call site writes are typed here
 * (padded to their offsets, per this unit's convention). `op` takes the
 * CD_OP_* values, `fileIndex` is FindCdFileIndex's index into gFileTable (0
 * when the op does not name a file), and param0/param1 are the two per-op
 * arguments code_179d8_s passes through: a byte count and a flag for op 4, a
 * buffer and a size for op 5. */
typedef struct CdRequest_282AC CdRequest_282AC;
struct CdRequest_282AC {
    u8 pad00[0x08];
    /* +0x08 */ s32 op;
    /* +0x0C */ s32 owner;
    /* +0x10 */ s32 fileIndex;
    /* +0x14 */ s32 param0;
    /* +0x18 */ s32 param1;
};
extern CdRequest_282AC *AllocCdRequestNode(void); /* code_179d8_r: alloc + link */

typedef struct Obj6D4E8_282AC Obj6D4E8_282AC;
struct Obj6D4E8_282AC {
    u8 pad00[0x22];
    /* +0x22 */ u16 pendingRequests;
    /* +0x24 */ s32 flags;
};

/* The store order below is retail's own (+0x08, +0x14, +0x0C, +0x10, +0x18),
 * not ascending offset -- see the match report: this compiler keeps
 * statement order for these, so the statements are in retail's order. */
void EnqueueCdRequest(Obj6D4E8_282AC *owner, s32 fileIndex, s32 op,
                      s32 param0, s32 param1)
{
    CdRequest_282AC *entry = AllocCdRequestNode();

    entry->op = op;
    entry->param0 = param0;
    entry->owner = (s32)owner;
    entry->fileIndex = fileIndex;
    entry->param1 = param1;

    owner->pendingRequests++;
    owner->flags = 0;
    StartCdService();
}
```

## Derivation

Five-argument function (four in registers, a fifth on the caller's stack at
`0x38($sp)` after this function's own `-0x28` prologue adjustment —
standard o32 stack-arg slot). Allocates/links a list node via
`AllocCdRequestNode` (foxtrot's `code_179d8_r`, still `INCLUDE_ASM` there —
declared `extern` here per the cross-unit convention already established by
`code_179d8_h.c` and `ServiceCdDriver`'s report) and fills five of its
fields with the incoming parameters. The store order to the new entry
(`+0x08, +0x14, +0x0C, +0x10, +0x18`) is NOT ascending-offset — it's
`arg2, arg3, arg0, arg1, arg4` in that literal order — and reproducing it
required writing the assignment *statements* in that same order; GCC
2.6.3 preserves store-to-store order for what look like independent
struct-field writes here rather than reordering them by offset. Then
increments a `u16` counter on `arg0` at `+0x22` and clears a `s32` at
`+0x24`, then tail-calls the already-matched `StartCdService` (void, no
args). GCC scheduled the counter's store-back into `StartCdService`'s call
delay slot on its own; writing the natural `owner->pendingRequests++; owner->flags = 0;
StartCdService();` statement order was sufficient — no manual reordering
needed to reproduce that scheduling choice.

### Proposed learning

**Struct-field-store order in this compiler's output is statement order,
not offset order.** When a run of consecutive stores targets non-ascending
offsets of the same base pointer, don't "clean up" the C by writing the
assignments in offset order — transcribe the disassembly's literal store
sequence into statement order instead. Matches the class of learning
already on file for do-while retry loops and pointer-walk loops: this
compiler is comparatively deferential to source order except where genuine
scheduling freedom exists (as it exercised here, moving one store into an
unrelated call's delay slot).

## Naming

Round 51 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_800282AC` | `EnqueueCdRequest` | A |

**Evidence.** Allocates and links a node via `AllocCdRequestNode` (code_179d8_r,
which appends to the `D_8008A894` list), fills five of its fields from the
parameters, bumps the requesting object's pending count, clears its flags and
calls `StartCdService`. Every caller is a class method taking its
asynchronous path (`code_179d8_s` at op 2/3/4/5/7,
`Class6D4E8__RequestLoadFile` at op 7). Append a request and make sure the
service runs: tier A.

**Parameter and field names established here**, all local to this `.c`:

| field | name | tier | evidence |
| --- | --- | --- | --- |
| `+0x08` | `op` | A | the five call sites pass 2, 3, 4, 5, 7 -- one constant per class method, and `func_80027A24` switches on it when it drains the queue |
| `+0x0C` | `owner` | A | the requesting object; `Class6D4E8__CancelRequests` matches on it to cancel one object's requests |
| `+0x10` | `fileIndex` | A | `FindCdFileIndex`'s return -- an index into `gFileTable` -- at the two ops that name a file, 0 at the others |
| `+0x14` | `param0` | B | the op's first extra argument: `arg2` for op 2, a byte count for op 4, a buffer for op 5 |
| `+0x18` | `param1` | B | the op's second extra argument, same call sites |

`param0`/`param1` are tier B deliberately: they are per-op arguments with a
different meaning in each op, so any more specific name would be true of one
call site and false of three.

## Proposed field names

**APPLIED by the head at merge, round 52** -- all four fields, both types
and all five vtable slots below are now in the tree, each one applied
separately with `./build-and-verify.sh` green and byte-exact after it. One
mis-hit had to be resolved by receiver type: `src/code_179d8_h.c:143`
accesses `pendingGeneration` on a `Class6D430 *self`, while the same file's
lines 97/174/175/182 are its OWN `ObjA34_179D8H::unk0C` and were left alone.
The compiler named that mis-hit (`structure has no member named 'unk0C'`),
which is the procedure working in the direction where it can work.

| unit | type | field | proposed | tier | evidence |
| --- | --- | --- | --- | --- | --- |
| code_179d8_s | `Node8008A894` | `unk8` | `op` | A | as above |
| code_179d8_s | `Node8008A894` | `unkC` | `owner` | A | as above |
| code_179d8_s | `Node8008A894` | `unk10` | `fileIndex` | A | as above |
| code_179d8_s | `Node8008A894` | `unk14`/`unk18` | `param0`/`param1` | B | as above |
| code_179d8_r | `Node8008A894` | `unk4` | *(no proposal)* | C | this unit never touches `+0x04`; only `AllocCdRequestNode` zeroes it |
