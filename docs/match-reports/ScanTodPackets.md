# ScanTodPackets -- MATCHED (98/98 words)

> Renamed from `func_80043FE4` on 2026-09-25 (tools/rename.py). Address 0x80043fe4.

Round 82, runner echo (code_33808 session, echo #9), 2026-09-25. Unit `code_33808`.
Byte-exact; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 98/98 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

Walk a packet stream: the u16 at data +2 is the packet count, packets start at data +8. Each packet is decoded by DecodeTodPacketWord (the tables' +0x080) into value / type / sub-type / length-in-words. Type 8 sub-type 0: count it and append the value to `out` when given. Type 2: with `out` and `sel`, when the packet's halfword at +4 equals `*sel`, search the values appended so far (rewinding `out` by the count) for this packet's value and keep its index; without `out`, count it. The index / count is stored through `sel` when given; returns the number appended (u8).

Table slot (`tools/classtable.py`): D_8006F240 +0x07C and D_8006F590 +0x07C (both tables share it; Tod__ScanPackets / TodSet__ScanPackets call it through slot +0x07C with the data past the buffer header).

## Source

The unit-local views `DataSrc33808` (FileResource subclass via the unified
`FILERESOURCE_SLOTS`/`FILERESOURCE_FIELDS` macros plus `slot7C`/`slot80` and own
fields +0x2C..+0x38), `Ctor33808`, `CountedBuf33808` and `Req44858` sit at the
top of / earlier in `src/code_33808.c`.

```c
/* D_8006F240/D_8006F590 +0x07C: walk the packet words after the u16 count
 * at data +2 (from data +8), each decoded by +0x080 into a value, a type, a
 * sub-type and a length in words. Type 8 sub-type 0 appends the value to
 * `out` (when given) and counts it; type 2 either, with `out`, looks the
 * value up among the ones appended so far when its halfword at +4 matches
 * `*sel` -- keeping its index -- or, without `out`, counts it. The index /
 * count goes back through `sel`; returns the number appended. */
u8 ScanTodPackets(DataSrc33808 *self, u8 *out, u32 *sel, u32 *data) {
    u8 value;
    u8 type;
    u8 sub;
    u8 len;
    u32 n;
    u32 i;
    s32 j;
    u8 cnt;
    s32 found;

    n = ((u16 *)data)[1];
    data += 2;
    i = 0;
    cnt = 0;
    found = 0;
    for (; i < n; i++) {
        DecodeTodPacketWord(self, data, &value, &type, &sub, &len);
        if (type == 8 && sub == 0) {
            cnt++;
            if (out != NULL) {
                *out++ = value;
            }
        } else if (type == 2) {
            if (out != NULL) {
                if (sel != NULL && ((u16 *)data)[2] == *sel) {
                    for (j = 0, out -= cnt; j < cnt; j++) {
                        if (*out++ == value) {
                            found = j;
                            break;
                        }
                    }
                }
            } else if (sel != NULL) {
                found++;
            }
        }
        data += len;
    }
    if (sel != NULL) {
        *sel = found;
    }
    return cnt;
}
```

## Notes

About twenty builds plus one bounded permuter search (Gate 3: check 1 scaffold compiled and scored 200; check 2 `--debug --stack-diffs` insertions 1 / deletions 1; check 3 funcdiff on the same body `insertions 0 / deletions 0 ... positional skeleton diffs 1` -- the permuter's differ counts the one opcode replacement blez->beqz as 1+1, funcdiff as one positional replacement: same single residue, AGREE; search `-j 4 --stop-on-zero --best-only`, killed after ~31000 iterations once the hand lever below matched, best 100 was a semantics-changing candidate that hoisted `out -= cnt` out of the branch, never zero). The residue from build ~5 on was ONE instruction: the inner search loop's entry test, retail `blez` on the u8 count, mine `beqz` (cc1 knew the zero-extended count is non-negative and turned `0 < cnt` into `cnt != 0`). What matches is moving the rewind into the for-init as a comma expression: `for (j = 0, out -= cnt; j < cnt; j++)`. Tried and still `beqz` (97/98): `j < (s32)cnt`, `cnt > j`, a while loop, `j <= cnt - 1` (worse), `j - cnt < 0` (worse), `(s8)cnt` (worse), u8/s16 `j` (frame changed), an s32 `cnt` with `(u8)` casts (register swap), a separate `k = cnt` (set once or twice: extra move, still beqz), `out -= (k = cnt)`, an explicit `if (cnt > 0) do {} while` (29/98). Earlier levers: the `data` parameter itself is the walking pointer (`u32 *data; data += 2;`) -- a separate `p` cost a register and a save order (35/98 -> 49/98); the loop bound read as `cnt` directly rather than through an s32 `k` (49 -> 97/98).

### Proposed learning

A loop pre-test that retail does as a SIGNED compare (`blez`) on a value cc1 can prove non-negative (a u8 variable) while cc1 emits `beqz`: put a side-effecting statement into the for-init as a comma expression (`for (j = 0, out -= cnt; j < cnt; j++)`); with the statement before the loop cc1 simplifies the test.

## Naming

- **ScanTodPackets**, tier A. Free function occupying slot7C, shared between Tod and TodSet: walks the TOD packet stream counting/looking up type-8/type-2 packets; the packet shape matches include/code_55dd4.h's TOD-packet description exactly.

## Track 4 (2026-09-26, round 86, charlie)

`self` is now `Tod *` (include/Tod.h), and the function is Tod's +0x07C `scanTodPackets` slot (TodSet inherits it). The forward prototype of DecodeTodPacketWord above it moved into the header. Bytes unchanged.
