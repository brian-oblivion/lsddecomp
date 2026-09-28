# DecodeTodPacketWord -- MATCHED (14/14 words)

> Renamed from `func_8004416C` on 2026-09-25 (tools/rename.py). Address 0x8004416c.

Round 82, runner echo (graphics_resources session, echo #6), 2026-09-25. Unit `graphics_resources`.
Byte-exact on the FIRST build; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 14/14 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

Decodes one packet word: `*out0 = v; *out1 = (v >> 16) & 0xF; *out2 = (v >> 20) & 0xF; *out3 = v >> 24; return acc + 1;` out2/out3 are the o32 5th/6th arguments (stack +0x10/+0x14), and self's a0 is reused for out2. Same signature as the `decodeTodPacket` slot in src/world/TodActor.c.

Table slot (`tools/classtable.py`): gTodMethods +0x080 and gTodSetMethods +0x080.

## Source

The unit-local view `DataSrc33808` (a FileResource subclass built with the unified
`FILERESOURCE_SLOTS`/`FILERESOURCE_FIELDS` macros, plus `slot7C`/`slot80`, and own
fields +0x2C..+0x38) and `SubBlockTable` sit at the top of `src/graphics/graphics_resources.c`.
Slot +0x078 is `void *slot78` in the unified macro, so calls cast it.

```c
/* gTodMethods/gTodSetMethods +0x080: decode one packet word -- the low byte, then
 * the two nibbles at bits 16 and 20, then the top byte -- and return the
 * pointer past it. */
u32 *DecodeTodPacketWord(DataSrc33808 *self, u32 *acc, u8 *out0, u8 *out1, u8 *out2, u8 *out3) {
    u32 v = *acc;

    *out0 = v;
    *out1 = (v >> 16) & 0xF;
    *out2 = (v >> 20) & 0xF;
    *out3 = v >> 24;
    return acc + 1;
}
```

## Notes

- No shared header was edited. `FileResource.h`, `scene_node.h`, `basic_class.h` are
  included; prototypes for other units' functions (GetActiveDataSourceMethods,
  ReleaseBasicClassArray, BMemPMgrFree) are local to the unit.
- Types of arguments and returns are readings of the registers used, not proven.

## Naming

- **DecodeTodPacketWord**, tier A. Free function occupying slot80, shared between Tod and TodSet: decodes one packet word into value/type/sub-type/length fields.

## Track 4 (2026-09-26, round 86, charlie)

`self` is now `Tod *` (include/Tod.h), and the function is Tod's +0x080 `decodePacketWord` slot (TodSet inherits it). Bytes unchanged.

## Round 93 polish (charlie, track 7)

### Naming

| old | new | tier | evidence |
| --- | --- | --- | --- |
| `acc`, `out0`..`out3`, `v` | `packet`, `objId`, `type`, `flag`, `len`, `word` | A | Sony's TOD packet header: id bits 0..15, type 16..19, flag 20..23, length in words 24..31 |
| `16`, `20`, `24`, `0xF` | `TOD_PACKET_TYPE_SHIFT`, `_FLAG_SHIFT`, `_LEN_SHIFT`, `TOD_PACKET_NIBBLE` (include/Tod.h) | A | the same layout |
