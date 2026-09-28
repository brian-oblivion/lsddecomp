# GraphRoom__Reset -- MATCHED (26/26)

> Renamed from `GraphRoomObj__InitDisplay` on 2026-09-26 (tools/rename.py). Address 0x80058078.

> Renamed from `func_80058078` on 2026-09-24 (tools/rename.py). Address 0x80058078.

Unit: `src/class_3bb8c_t.c`. Class: `gGraphRoomMethods`, own vtable slot `+0x040`
(resolved via `tools/classtable.py gGraphRoomMethods`).

## Signature

```c
void GraphRoom__Reset(D_80087AACObj *self);
```

## Body

```c
extern char sGraphTimPath[];

void GraphRoom__Reset(D_80087AACObj *self) {
    self->unk_0x84 = 5;
    self->unk_0x2C = 0x190;
    self->methods->slotD4(self, sGraphTimPath, 0);
    self->methods->slot6C(self, 0xA);
}
```

Uses `sGraphTimPath` (the `"ETC\HGRAPH.TIM"` string, one of the two
standalone strings this unit's own header comment names -- resolved by
symbol, declared locally as `extern char sGraphTimPath[];` since it is not
referenced by any other unit). Names `D_80087AACMethods::slot6C/slotD4`
and `D_80087AACObj::unk_0x2C/unk_0x84` (additive extensions).

## Verify

```
./build-and-verify.sh   # build exit=0, OK: build matches retail
tools/funcdiff.py GraphRoom__Reset   # 26/26
```

## Naming (round 75, track 3)

**`GraphRoom__Reset`** -- tier B. Own vtable slot +0x040. Loads
the literal texture string `"ETC\HGRAPH.TIM"` through the renamed
`loadTexture` slot (+0x0D4) -- this call site is the primary evidence for
the whole class being named `GraphRoomObj` (a "Graph"-labelled texture
resource). The rest of the body (setting `unk_0x84`/`unk_0x2C`, a further
`slot6C(self, 0xA)` call) is setup whose specific purpose isn't
established past "display init", hence "InitDisplay" rather than a more
specific verb.

## Track 4 (2026-09-26, round 87, alpha): renamed `GraphRoomObj__InitDisplay` -> `GraphRoom__Reset`

The class is unified in `include/GraphRoom.h` (class id 0x2F130, table `gGraphRoomMethods`, parent TaskCore; `tools/classtable.py gGraphRoomMethods --vs gTaskCoreMethods`). The class name drops round 75's `Obj` suffix (FINISHING-PLAN track 4 step 2); every function prefix moved with it. The unit's GraphRoomObj/GraphRoomMethods views are deleted and `self` is `GraphRoom *`; inherited fields and slots carry TaskCore's and IntermediateBase's names. Zero bytes changed (build-and-verify OK, typeviews --warnings 0 new). Named for its slot, track 4 step 6: +0x040 is IntermediateBase's `resetCounters`, TaskCore__Reset in the parent table, and the ctor's last call, which is how this body runs. It sets fadeRate (+0x084, was unk_0x84) 5 and unk2C 0x190 (TaskCore's reset sets 9 and 0x12C), then setSubHandle (+0x0D4, was `loadTexture`) with "ETC\HGRAPH.TIM" and setFrameBound (+0x06C, was slot6C) 10. It does not up-call TaskCore__Reset.

## Track 7 (2026-09-27, round 97, delta)

- **Naming: `D_80011778` -> `sGraphTimPath`** (tier A): the rodata string `"ETC\HGRAPH.TIM"`, Reset's path for `setSubHandle`; its only user.
- Constants in decimal: `unk2C = 400` (the 0x190; TaskCore's reset writes 300, class_3bb8c_c writes 400; it becomes Viewport's `unk44`, one factor of each buffer's packet area), `setFrameBound(10)` (frameBound = 10 * 20). `setSubHandle`'s handle is `NULL`. Zero bytes changed.
- Left: `unk2C` is TaskCore's field, accessed in Task.c and class_3bb8c_c.c too: a name is proposed to the head rather than applied.
