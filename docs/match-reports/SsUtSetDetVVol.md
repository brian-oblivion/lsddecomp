# SsUtSetDetVVol — MATCHED, 31/31 words byte-exact (round 38)

> Renamed from `func_80031CF0` on 2026-09-23 (tools/rename.py). Address 0x80031cf0.

Unit: `src/psyq/libsnd_ut_cp_ut_cadsr_ut_vvol_ut_autov_ut_autop.c` · vram `0x80031CF0` · file `0x224F0-0x2256C` · 31 words.

Closed in round 38. Whole-image SHA1 green; `funcdiff` 31/31 with zero
out-of-range bytes. The function was **never permuter-searched** and did not
need to be — it was assigned precisely because the corrected never-searched
screen surfaced it, and it fell to hand analysis in one sitting.

## Provenance of this report

Runner charlie derived the body and reached 31/31, then died to an
infrastructure error (org API 403) before committing or writing this report.
The head salvaged the working tree, re-verified the match independently in the
worktree, **reworked the body** (see below), and committed. The derivation is
charlie's; the simplification and this report are the head's.

## The C

```c
s32 SsUtSetDetVVol(s16 idx, s16 p1, s16 p2)
{
    /* Retail reserves an 8-byte frame it never touches. Only an unused local
     * ARRAY of that size reproduces it -- a scalar is register-allocated and
     * eliminated, and a 4-byte array reserves the wrong amount. */
    s32 unused[2];

    if ((u16) idx < 0x18) {
        _svm_sreg_buf[idx].unk2 = p2;
        _svm_sreg_dirty[idx] |= 3;
        _svm_sreg_buf[idx].unk0 = p1;
        return 0;
    }
    return -1;
}
```

A bounds-checked setter over the same 16-byte-stride SPU voice-parameter
table (`_svm_sreg_buf`) and per-voice flag byte (`_svm_sreg_dirty`) that this unit's
already-matched `SsUtGetDetVVol` reads. Note the store ORDER: retail writes
`unk2` first, then OR's the flag byte, then writes `unk0`. Writing the two
`_svm_sreg_buf` fields adjacently does not reproduce it.

## The residue that mattered: an unused stack frame, and how to reserve one

Retail opens with `addiu $sp, $sp, -0x8` and closes with `addiu $sp, $sp, 0x8`
and **never stores to the frame**. Nothing in the function's visible behaviour
needs a frame at all.

Charlie's first matching body reserved it with a dead-code hack:

```c
    s32 dead[2];

    if (0) {
        dead[0] = 1;
    }
```

That reaches 31/31, but it is a transcribed candidate rather than C anyone
would write, and left standing it becomes the precedent for the next unused
frame. The head measured whether the `if (0)` was load-bearing. **It is not.**

| variant | bytes | whole-image build | score |
| --- | --- | --- | --- |
| `s32 dead[2];` + `if (0) { dead[0] = 1; }` | 8 | green | 31/31 |
| **`s32 unused[2];` alone** | **8** | **green** | **31/31** |
| `s16 unused[4];` alone | 8 | green | 31/31 |
| `s32 unused[1];` alone | 4 | RED | — |
| `s32 unused;` (scalar) | 4 | RED | — |
| `s32 d0; s32 d1;` (two scalars) | 8 | RED | — |
| no local at all | 0 | RED | — |

All six measured against the whole-image oracle, one variant per build.

**The discriminator is ARRAY-vs-SCALAR and then SIZE, not usedness.** An
unused local *array* reserves stack frame space equal to its own size under
GCC 2.6.3 at `-O2`; an unused *scalar* does not, however many of them you
declare, because it is register-allocated and then eliminated. The `if (0)`
guard does nothing — it neither rescues a scalar (two-scalar row is red with
the array form's exact byte count) nor is needed by an array.

### Proposed learning

**"GCC 2.6.3 dead-code-eliminates an unused local" is true of SCALARS and
false of ARRAYS, and the difference is the lever for the unused-frame residue
class.** That sentence is currently written down without the qualifier —
`docs/match-reports/vmNoiseOn2.md`'s round 35 update reaches it from a
scalar experiment and states it unconditionally. Measured here both ways: a
scalar (or two) leaves no frame, an array of the right size leaves exactly
that frame.

So an unused-frame residue is not a toolchain wall and is not a reason to
reach for `__asm__`. It is a **size measurement**: read the `addiu $sp, $sp,
-N` off the prologue and declare an unused local array of exactly N bytes.
Getting N wrong fails the whole-image build outright rather than scoring
low, so the oracle tells you immediately.

This is directly actionable on the live queue: `FlagLargePolyForDivide`
(`tmd_renderer`) is filed as *"unused-frame placement residue, 53/70 words"*
and has never been tried against this lever.

## Track 2 (round 86, 2026-09-26, alpha)

**_svm_sreg_buf / _svm_sreg_dirty (same round).** `D_8008D7F0` (0x180 bytes, 24 voices x 0x10, halfwords at +0x0..+0xA spelled `D_8008D7F0`..`D_8008D7FA` by splat) is Sony's `_svm_sreg_buf` and `D_8008D970` (24 bytes) is `_svm_sreg_dirty`: libsnd/vmanager.o bss +0x000 and +0x180, anchored at 0x8008D7F0. Both are in the symbols file; the record type is `SvmSreg` in `include/SvmData.h` (fields by offset). `src/` spelling only: `_svm_sreg_buf[idx].unk0/unk2` (was a unit-local `Rec16D7F0` view of `D_8008D7F0`) and `_svm_sreg_dirty[idx]`. Byte-exact.

## Round 97 (bravo, track 6)

The definition now takes <libsnd.h>'s prototype: return type `s32` became `s16` (Sony's `short`). Zero bytes.
The in-source comment on `s32 unused[2]` shrank to a MATCHING line; its
content was: retail reserves an 8-byte frame it never touches, and only an
unused local ARRAY of that size reproduces it -- a scalar is register-allocated
and eliminated, and a 4-byte array reserves the wrong amount. SsUtSetVVol uses
the same idiom.
