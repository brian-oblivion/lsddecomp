# ServiceStyleCueIfNear -- MATCHED, round 46 (2026-09-15)

> Renamed from `StopStyleCueIfNear` on 2026-09-26 (tools/rename.py). Address 0x8005582c.

> Renamed from `func_8005582C` on 2026-09-23 (tools/rename.py). Address 0x8005582c.

Unit `ObjMStyleActor`. **18/18 words, byte-exact.** First build.

## What it was

Fresh ground, carved round 45, never attempted.

## Derivation

```
/* 4602C 8005582C E8FFBD27 */  addiu $sp, $sp, -0x18
/* 46030 80055830 1000B0AF */  sw    $s0, 0x10($sp)
/* 46034 80055834 1400BFAF */  sw    $ra, 0x14($sp)
/* 46038 80055838 1D56010C */  jal   IsStyleCueNear
/* 4603C 8005583C 21808000 */   addu $s0, $a0, $zero
/* 46040 80055840 07004010 */  beqz  $v0, .L80055860
/* 46044 80055844 21100000 */   addu $v0, $zero, $zero
/* 46048 80055848 7404828F */  lw    $v0, %gp_rel(sStyleSceneRefs)($gp)
/* 46050 80055850 0000448C */  lw    $a0, 0x0($v0)
/* 46054 80055854 42B3000C */  jal   ServiceSoundCueSet
/* 46058 80055858 14000526 */   addiu $a1, $s0, 0x14
/* 4605C 8005585C 01000234 */  ori   $v0, $zero, 0x1
.L80055860:
...
jr $ra
```

The second parameter (`$a1`) is never read before the tail call to
`IsStyleCueNear`, so it passes straight through unmodified -- this function's
own second argument. `IsStyleCueNear` (31w, not attempted this round, still
`INCLUDE_ASM`) is only forward-declared here for its calling shape, taking
this unit's `ObjN14 *` plus a second opaque pointer and returning an `s32`
flag. On a nonzero result this calls `ServiceSoundCueSet` exactly the way
`FlushStyleCue` calls its sibling `FlushSoundCueSet` -- same `sStyleSceneRefs`
dereference for `self`, same `&arg0->unk14` embedded sub-object -- and
returns 1; otherwise returns 0 (retail's own `ServiceSoundCueSet` return value,
if any, is discarded and overwritten by the explicit `ori $v0,$zero,0x1`
right after the call, consistent with `ServiceSoundCueSet`'s existing
`void`-returning declaration in `include/Entity.h`/`include/DreamSys.h`).

```c
extern s32 IsStyleCueNear(ObjN14 *arg0, void *arg1);
extern void ServiceSoundCueSet(s32 arg0, void *arg1);

s32 ServiceStyleCueIfNear(ObjN14 *arg0, void *arg1) {
    if (IsStyleCueNear(arg0, arg1) != 0) {
        ServiceSoundCueSet(*(s32 *) sStyleSceneRefs, &arg0->unk14);
        return 1;
    }
    return 0;
}
```

### Proposed learning

None -- straightforward given `FlushStyleCue`'s already-established
`sStyleSceneRefs`/`FlushSoundCueSet` shape to mirror onto `ServiceSoundCueSet`.

## Naming

**`ServiceStyleCueIfNear`, tier B.**

If `IsStyleCueNear` reports true, calls `ServiceSoundCueSet` (an established
sibling of `FlushSoundCueSet`, same `(s32, void *)` shape, still unnamed
itself -- not this unit's to name, per track 3's "do not rename a Sony
symbol" analog for a cross-unit function this unit only calls) on the slot's
`cueSet`, and returns 1; otherwise returns 0. `ServiceSoundCueSet` is read here
as "stop" by the established pairing with `FlushSoundCueSet`/`InitSoundCueSet`
(init/flush/stop triad) documented in `include/Entity.h`/`include/DreamSys.h`,
which is cross-unit evidence, not a single-call-site guess. MATCHED,
18/18, first build.

## Round 93 polish (delta, track 7)

### Naming

**Renamed from `StopStyleCueIfNear`, tier A.** While the slot is within range (IsStyleCueNear) it calls ServiceSoundCueSet on the slot's cue set -- the once-per-tick service pass (SoundCueSet.h) -- and returns 1; it stops nothing. The stop is TickStyle's FlushStyleCue on a 0 return. Parameters `slot`, `target`, `unused`.
