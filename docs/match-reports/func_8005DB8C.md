# func_8005DB8C

**Unit:** Entity · **Size:** 25 instructions · **Status:** MATCHED (25/25 words, whole-image build verified byte-exact)

## What it does

Calls a still-uncarved function, `func_8002CC84(this->unk58, this->unk9C)`
(the same two-argument shape `func_8005D6D4` uses with `func_8002CD08` — see
that report), then calls this entity's own vtable slots `+0x130` and
`+0x114` (both no-argument), and clears `this->unkF8`.

## Derivation

```
lw   $a0, 0x58($s0)
jal  func_8002CC84
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
    func_8002CC84(this->unk58, this->unk9C);
    this->methods->slot130(this);
    this->methods->slot114(this);
    this->unkF8 = 0;
}
```

`func_8002CC84` declared `extern void func_8002CC84(s32 arg0, void *arg1);`
in `Entity.h`, same rationale as `func_8002CD08` in `func_8005D6D4.md`.
`slot130`/`slot114` typed `void (*)(Entity *self)` in `EntityMethods`.

## Attempt log

Matched on the first attempt.

## Proposed learning

None new beyond what's already in `func_8005D6D4.md` about the
`func_8002CD08`/`func_8002CC84` pairing.
