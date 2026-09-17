> Renamed from `func_800282AC` on 2026-09-17 (tools/rename.py). Address 0x800282ac.

# EnqueueCdRequest — MATCHED (32/32 words)

Round 45, runner echo (second sitting), `src/code_179d8_q.c`.

## Result

Byte-exact on the first attempt.

```c
/* func_8002832C (code_179d8_r) allocates and links a 0x24-byte list node;
 * only the fields this call site writes are typed here (padded to their
 * offsets, per this unit's convention). */
typedef struct Entry800282AC Entry800282AC;
struct Entry800282AC {
    u8 pad00[0x08];
    s32 unk08;
    s32 unk0C;
    s32 unk10;
    s32 unk14;
    s32 unk18;
};
extern Entry800282AC *func_8002832C(void); /* code_179d8_r */

typedef struct Self800282AC Self800282AC;
struct Self800282AC {
    u8 pad00[0x22];
    u16 unk22;
    s32 unk24;
};

void EnqueueCdRequest(Self800282AC *arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4)
{
    Entry800282AC *entry = func_8002832C();

    entry->unk08 = arg2;
    entry->unk14 = arg3;
    entry->unk0C = (s32)arg0;
    entry->unk10 = arg1;
    entry->unk18 = arg4;

    arg0->unk22++;
    arg0->unk24 = 0;
    StartCdService();
}
```

## Derivation

Five-argument function (four in registers, a fifth on the caller's stack at
`0x38($sp)` after this function's own `-0x28` prologue adjustment —
standard o32 stack-arg slot). Allocates/links a list node via
`func_8002832C` (foxtrot's `code_179d8_r`, still `INCLUDE_ASM` there —
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
delay slot on its own; writing the natural `arg0->unk22++; arg0->unk24 = 0;
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

**Evidence.** Allocates and links a node via `func_8002832C` (code_179d8_r,
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
| `+0x10` | `fileIndex` | A | `func_800284C4`'s return -- an index into `gFileTable` -- at the two ops that name a file, 0 at the others |
| `+0x14` | `param0` | B | the op's first extra argument: `arg2` for op 2, a byte count for op 4, a buffer for op 5 |
| `+0x18` | `param1` | B | the op's second extra argument, same call sites |

`param0`/`param1` are tier B deliberately: they are per-op arguments with a
different meaning in each op, so any more specific name would be true of one
call site and false of three.

## Proposed field names

| unit | type | field | proposed | tier | evidence |
| --- | --- | --- | --- | --- | --- |
| code_179d8_s | `Node8008A894` | `unk8` | `op` | A | as above |
| code_179d8_s | `Node8008A894` | `unkC` | `owner` | A | as above |
| code_179d8_s | `Node8008A894` | `unk10` | `fileIndex` | A | as above |
| code_179d8_s | `Node8008A894` | `unk14`/`unk18` | `param0`/`param1` | B | as above |
| code_179d8_r | `Node8008A894` | `unk4` | *(no proposal)* | C | this unit never touches `+0x04`; only `func_8002832C` zeroes it |
