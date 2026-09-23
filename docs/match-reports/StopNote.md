# StopNote -- MATCHED (131/131 words)

> Renamed from `func_800300D0` on 2026-09-20 (tools/rename.py). Address 0x800300d0.

Unit: `src/code_179d8_m.c`. Round 24, runner bravo.

## Result

Byte-exact. `./build-and-verify.sh` green (`build exit=0`, whole-image SHA1
matches). `funcdiff.py`: `131/131 words match (file 0x208D0-0x20ADC)`, no
out-of-range drift.

## Final C

```c
typedef struct {
    s16 unk0; /* +0x0 */
    u8 pad2[0x34 - 0x2];
} Rec34S16;
extern Rec34S16 D_8008D994[];
extern Rec34S16 D_8008D996[];
extern Rec34S16 D_8008D99A[];
extern Rec34S16 D_8008D99E[];
extern Rec34S16 D_8008D988[];

extern u16 D_80090C60;
extern u16 D_80090C64;
extern u16 D_8008E228;
extern u16 D_8008E22C;

u8 StopNote(s16 a0, s16 a1, s16 a2, u16 a3) {
    u8 i;
    u8 count;

    count = 0;
    for (i = 0; i < D_8008E9D0; i++) {
        if (D_8008D994[i].unk0 != a3) {
            continue;
        }
        if (D_8008D99A[i].unk0 != a2) {
            continue;
        }
        if (D_8008D996[i].unk0 != a0) {
            continue;
        }
        if (D_8008D99E[i].unk0 != a1) {
            continue;
        }
        if (D_8008D988[i].unk0 == 0xFF) {
            D_8008D9A3[i].unk0 = 0;
            D_8008D98C[i].unk0 = 0;
            D_8006DAD4->unk194 = 0;
            D_8006DAD4->unk196 = 0;
        } else {
            u16 chan;
            u16 lowMask;
            u16 highMask;

            D_8008EA26 = i;
            chan = D_8008EA26;
            if (chan < 0x10) {
                lowMask = 1 << chan;
                highMask = 0;
            } else {
                lowMask = 0;
                highMask = 1 << (chan - 0x10);
            }
            D_8008D9A3[chan].unk0 = 0;
            D_8008D98C[chan].unk0 = 0;
            D_8008D988[chan].unk0 = 0;
            D_80090C60 |= lowMask;
            D_80090C64 |= highMask;
            D_8008E228 &= ~D_80090C60;
            D_8008E22C &= ~D_80090C64;
        }
        count++;
    }
    return count;
}
```

## Shape

Scan every record `i` in `[0, D_8008E9D0)` for one whose four key fields
(`D_8008D994`, `D_8008D99A`, `D_8008D996`, `D_8008D99E` -- s16 fields of the
same 0x34-stride record family `code_179d8_j.c` documents as
`Rec34D994`, redeclared here as a local `Rec34S16`) match the caller's
`(a3, a2, a0, a1)`. For every match, do one of two cleanups depending on
`D_8008D988[i]`:

- **`== 0xFF`:** the same "type A" reset `StopNote`'s neighbours use
  -- clear `D_8008D9A3[i]`, `D_8008D98C[i]`, and the current object's
  `unk194`/`unk196` fields (`D_8006DAD4`, both already declared for
  `ClearNoiseVoices`).
- **otherwise:** a "channel" reset: stash `i` into the "currently selected
  channel" scratch global `D_8008EA26`, re-read it, clear `D_8008D9A3`/
  `D_8008D98C`/`D_8008D988` at the CHANNEL index (not `i` -- same value in
  practice, but a fresh read, matching the project's documented idiom),
  then update the two 16-channel bitmask pairs `code_179d8_j.c` already
  documents (`D_80090C60`/`D_8008E228` for channels 0-15,
  `D_80090C64`/`D_8008E22C` for channels 16-31): OR the appropriate mask
  into the "channel" word, then AND-NOT the freshly updated channel word
  into the "active" word. **Both pairs are touched unconditionally on
  every call** -- whichever mask doesn't apply to this channel's half is
  simply `0`, a no-op OR/AND-NOT, but the instructions run either way.

Either way, increment `count`; the loop does not stop at the first match
-- it keeps scanning to the end and returns the total number of records
reset.

## Two levers, both needed

1. **`D_8008EA26` genuinely needs `volatile`, and this function is what
   proves it (see the promoted declaration and its comment, moved above
   `SpuVmNoiseOnWithAdsr` since this function is the third user in ROM order).**
   Every earlier user of this symbol (`SpuVmNoiseOn`, `SpuVmNoiseOnWithAdsr`)
   only ever read it back through a BYTE-narrowing pointer cast
   (`*(u8 *)&D_8008EA26`), and a plain non-volatile declaration was
   sufficient there. This function is the first to read it back at its
   OWN declared width (`chan = D_8008EA26;`, no cast) -- and here, WITHOUT
   `volatile`, this compiler proves algebraically that `chan == i` (since
   `i` is `u8`, provably 0..255, and storing then reloading a value that
   narrow through a wider `u16` slot is lossless) and elides the reload
   entirely, producing `move v1, v0` instead of retail's genuine
   `sh`-then-`lhu` pair. Confirmed in isolation through the pinned
   pipeline: a minimal `u8 i; D_8008EA26 = i; chan = D_8008EA26;` reproduces
   the same elision without `volatile`, and reproduces retail's real
   store-then-reload with it.

   **Getting `volatile` right took a wrong turn first.** The natural
   instinct -- forcing the reload via a pointer cast, `*(volatile u16
   *)&D_8008EA26` on a non-volatile object -- reproduces the SAME failure
   mode `SpuVmNoiseOn`'s report already found for a byte-width volatile
   pointer cast: it defeats the compiler's addressing-mode fold, emitting
   `lui`/`addiu`/`lhu` (compute the pointer, then dereference) instead of
   retail's compact `lui`/`lhu %lo(sym)(reg)` two-instruction form.
   Confirmed in isolation (see below): the fold-breaking behavior is tied
   to the POINTER TYPE carrying `volatile`, not to whether the underlying
   OBJECT is volatile. Declaring the OBJECT `volatile u16` and accessing it
   through a PLAIN (non-volatile-qualified) `u8 *`/`u16` reference folds
   fine either way:

   | access | object volatile? | pointer volatile? | folds to compact form? |
   | --- | --- | --- | --- |
   | `chan = D_8008EA26;` (plain scalar) | no | n/a | yes, but elides the reload entirely (wrong) |
   | `chan = D_8008EA26;` (plain scalar) | **yes** | n/a | **yes, and reload happens (right)** |
   | `*(volatile u16 *)&D_8008EA26` | no | yes | no -- 3 extra instructions |
   | `*(u8 *)&D_8008EA26` (SpuVmNoiseOn's read) | **yes** | no | **yes** (unaffected by the object's own volatility) |

   So the one-line fix, once isolated, is simply: promote the shared
   `D_8008EA26` declaration to `extern volatile u16 D_8008EA26;` (matching
   `code_179d8_j.c`'s own declaration for this symbol) and read it plainly
   everywhere. `SpuVmNoiseOn`/`SpuVmNoiseOnWithAdsr`'s byte-cast reads were
   re-verified to still match after this change -- their `*(u8 *)&...`
   pattern is unaffected by the object gaining `volatile`, because the
   pointer type itself was never volatile-qualified.

2. **A pure register-identity residue in the bitmask-update tail, fixed by
   INTERLEAVING two independent statement pairs rather than grouping
   them.** Grouped as `D_80090C60 |= lowMask; D_8008E228 &= ~D_80090C60;
   D_80090C64 |= highMask; D_8008E22C &= ~D_80090C64;` (each OR immediately
   followed by its own AND-NOT), the function matched everywhere EXCEPT a
   pure `$a1`/`$a2` register swap across ~12 words, both in the earlier
   mask-selection `if`/`else` and in the four bitmask-update instructions
   themselves -- same instructions, same operands conceptually, wrong
   register throughout (word-match capped at 105/131 with zero out-of-range
   drift, a textbook register-identity shape). Neither reordering the two
   locals' DECLARATIONS, nor reordering the ASSIGNMENTS inside each `if`/
   `else` arm, nor swapping which GLOBAL PAIR's block came first, moved it
   (one reordering attempt made it worse, 104/131, by additionally
   swapping which physical global each mask fed). What worked:
   INTERLEAVING the two ORs before either AND-NOT --
   `D_80090C60 |= lowMask; D_80090C64 |= highMask; D_8008E228 &=
   ~D_80090C60; D_8008E22C &= ~D_80090C64;` -- closed the whole residue on
   the first try after the grouped form was ruled out.

### Proposed learning

**A same-value, same-shape register-identity swap across two 16-bit
bitmask fields is reachable by INTERLEAVING their update statements
(computing both new values before consuming either) rather than writing
each field's read-modify-write pair back-to-back.** This is a distinct
axis from the ones DECOMPILATION_LEARNINGS already lists for this residue
class (declaration order, statement order within a branch, whole-block
order) -- all three of those were tried here and failed or worsened it
before interleaving worked. Worth trying early on a similar "two
independent OR/AND-NOT accumulator pairs" shape before spending attempts
on the already-documented but here-ineffective axes.

**`volatile` on a pointer TYPE and `volatile` on the underlying OBJECT are
different levers with opposite failure modes, and the existing project note
(`SpuVmNoiseOn`'s report) only demonstrated one side of this.** That report
established "a volatile pointer cast breaks the addressing fold"; this
function needed the complementary fact -- "a volatile OBJECT read through a
plain non-volatile pointer/cast does NOT break the fold, and is the
correct way to force a redundant-store-elimination-defeating reload when
the value's narrow range would otherwise let the compiler prove the reload
unnecessary." Both facts belong together for the next runner who hits this
symbol or one like it.

## Naming

**StopNote** (was `func_800300D0`) -- Tier A. Same evidence as StartNote:
`code_179d8_k.c`'s `func_800344FC` calls this in its zero-velocity branch
(the MIDI note-off convention) and StartNote in the nonzero-velocity
branch of the same switch; `code_179d8_j.c`'s `SpuVmSeKeyOff` wraps this
with the same fixed leading identity constant `code_179d8_j.c`'s
`SpuVmSeKeyOn` uses to wrap StartNote. Scans every voice for one whose
four identity fields match the caller's, and releases it (a no-op
bookkeeping clear if it was never actually keyed on, per the
`D_8008D988[i].unk0 == 0xFF` branch; otherwise the real SPU-mask
deallocation), returning the count released.
