# func_8005DB8C

**Unit:** Entity · **Size:** 25 instructions · **Status:** MATCHED (25/25 words, whole-image build verified byte-exact)

## What it does

Calls a still-uncarved function, `FlushSoundCueSet(this->unk58, &this->unk9C)`
(the same two-argument shape `func_8005D6D4` uses with `func_8002CD08` — see
that report), then calls this entity's own vtable slots `+0x130` and
`+0x114` (both no-argument), and clears `this->unkF8`.

**Updated in round 2026-09-01 (runner bravo, Entity 11-function pass):**
`this->unk9C` changed from a `u8[]` array to a plain `s32` (see
`func_8005D6D4.md`'s update note); this call site's array-decay
`this->unk9C` became an explicit `&this->unk9C`, same compiled address.
Re-verified byte-exact.

## Derivation

```
lw   $a0, 0x58($s0)
jal  FlushSoundCueSet
 addiu $a1, $s0, 0x9C
lw   $v0, 0x0($s0)
lw   $v0, 0x130($v0)
jalr $v0                     ; this->methods->slot130(this)
 move $a0, $s0
lw   $v0, 0x0($s0)
lw   $v0, 0x114($v0)
jalr $v0                     ; this->methods->slot114(this)
 move $a0, $s0
sw   $zero, 0xF8($s0)        ; this->unkF8 = 0
```

## Final C

```c
void func_8005DB8C(Entity *this) {
    FlushSoundCueSet(this->unk58, &this->unk9C);
    this->methods->slot130(this);
    this->methods->slot114(this);
    this->unkF8 = 0;
}
```

`FlushSoundCueSet` declared `extern void FlushSoundCueSet(s32 arg0, void *arg1);`
in `Entity.h`, same rationale as `func_8002CD08` in `func_8005D6D4.md`.
`slot130`/`slot114` typed `void (*)(Entity *self)` in `EntityMethods`.

## Attempt log

Matched on the first attempt.

## Proposed learning

None new beyond what's already in `func_8005D6D4.md` about the
`func_8002CD08`/`FlushSoundCueSet` pairing.
