# D8006ED4C__SetPosition -- MATCHED (11/11 words), round 82

> Renamed from `func_80041E2C` on 2026-09-25 (tools/rename.py). Address 0x80041e2c.

Round 82, runner alpha (third re-staffed slot of the round). Unit `src/code_322b4.c`. Fresh ground (carved in FINISHING-PLAN revision 18), no prior attempt, no report before this one.

- **Where:** D_8006EC74 and D_8006ED4C slot +0x0BC (`tools/classtable.py`).
- **What:** If the word at +0x00C is non-zero, copies two words from the argument to +0x0A0/+0x0A4. Retail moves both args to `$a2`/`$a3` first and does `lw,lw,sw,sw`: a whole-struct assignment of an 8-byte `{s32 a,b;}` (`Pair_322b4`). Uses the unit-local `SpriteView_322b4` view (methods at +0x000, `unkC` +0x00C, GsSPRITE attribute +0x064, u/v +0x072/+0x073, 8-byte `Pair_322b4` +0x0A0, cell byte +0x0A8), added this session.
- **Result:** byte-exact; 11/11 words, 0 insertions / 0 deletions, whole-image SHA1 green (`./build-and-verify.sh` OK). First build.
- **Types:** local views and prototypes live in the unit; no shared header was touched.

## Source

```c
/* D_8006EC74 and D_8006ED4C slot +0x0BC. */
void D8006ED4C__SetPosition(SpriteView_322b4 *self, Pair_322b4 *src) {
    if (self->unkC != 0) {
        self->unkA0 = *src;
    }
}
```
