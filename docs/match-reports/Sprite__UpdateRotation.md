# Sprite__UpdateRotation -- MATCHED (39/39 words), round 82

Round 82, runner alpha (fifth slot on code_322b4). Unit `src/code_322b4.c`. Fresh ground, no prior body attempt.

- **Where:** gSpriteMethods slot +0x044 (updateRotation) (`tools/classtable.py`).
- **What:** Takes table[2] (the Ratio16 z entry) as a quotient, angle = ((whole / frac) << 12) + ((whole % frac) << 12) / frac -- degrees in 4096ths -- and sets or adds it to sprite.rotate (+0x084). First build.
- **Result:** byte-exact; 39/39 words, 0 insertions / 0 deletions, whole-image SHA1 green (`./build-and-verify.sh` OK). First build.
- **Types:** typed through the UNIFIED `Sprite` and SceneNode.h's `Ratio16`, unchanged; matches the Sprite.h prototype.

## Source

```c
/* gSpriteMethods slot +0x044 (updateRotation): table[2] as a fraction of
 * degrees, in 4096ths; set or add to the GsSPRITE's rotate. */
void Sprite__UpdateRotation(Sprite *self, s32 set, Ratio16 *table) {
    s32 angle;

    angle = ((table[2].whole / table[2].frac) << 12) + ((table[2].whole % table[2].frac) << 12) / table[2].frac;
    if (set) {
        self->sprite.rotate = angle;
    } else {
        self->sprite.rotate += angle;
    }
}
```

## Track 7 (round 99, charlie)

`<< 12` -> `<< FIX12_SHIFT` (include/common.h, added this round: `ONE == 1 << FIX12_SHIFT`, token-identical to GraphicsResources.c's local definition). Byte-exact.
