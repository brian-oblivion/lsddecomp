# PickSoundBank -- MATCHED (33/33 words)

> Renamed from `PickWeeklyGroup` on 2026-09-27 (tools/rename.py). Address 0x80048d74.

> Renamed from `func_80048D74` on 2026-09-25 (tools/rename.py). Address 0x80048d74.

Round 82, runner echo (second echo session), 2026-09-25. Unit `game_files`.
Byte-exact on the fourth build; whole-image SHA1 green (`./build-and-verify.sh`: `OK: build matches
retail SLPS_015.56`), funcdiff 33/33, 0 insertions / 0 deletions, no
out-of-range drift.

## What it does

Weekly table pick: `r = (u32)SeedAndRandom(0, arg) % 7` (unsigned: retail's multu/mfhi magic 0x24924925), `table = GetSoundBankPaths()` (sSoundBankPaths, words); returns `table[sForcedSoundBank - 1]` when the override global sForcedSoundBank is set, else `table[r]`.

## Source

Declarations it needs are the local views at the top of `src/cd/game_files.c`
(`D_80081940Obj`, `D_80081940Methods`, `FilePathRecord`) and `include/file_resource.h`.

```c
s32 PickSoundBank(s32 arg) {
    u32 r = (u32)SeedAndRandom(0, arg) % 7;
    s32 *table = GetSoundBankPaths();
    s32 *entry;
    s32 index;
    if (sForcedSoundBank != 0) {
        index = sForcedSoundBank - 1;
        entry = &table[index];
    } else {
        entry = &table[r];
    }
    return *entry;
}
```

## Lever (4 builds, 3 misses)

1. `if (D) return table[D-1]; return table[r];` -- two loads, 2 words LONG.
2. `index = D ? D-1 : r; return table[index];` -- the `sll` is shared after the
   join (`addiu v1,a1,-1` into a common `sll`), 2 words SHORT.
3. `if (D) entry = &table[D-1]; else entry = &table[r]; return *entry;` --
   right shape, but `(D-1)*4` folds to `sll; addiu -4`.
4. MATCH: the pointer-per-branch shape with the then-index in a temporary:

   `if (D) { index = D - 1; entry = &table[index]; } else { entry = &table[r]; }`

   The temporary stops the `(x-1)*4 -> x*4-4` fold, and the per-branch
   pointer keeps a separate `sll` in each arm as retail has.

### Proposed learning

"Retail has `addiu -1` BEFORE the `sll` of an index in one arm of an if/else
and a separate `sll` in the other arm": assign the pointer (`p = &t[i]`) in
each arm, with the decremented index in its own variable; a shared index
variable merges the shifts, and `&t[x - 1]` folds the constant after the
shift.

## Notes

- The unit now has a local `D_80081940Methods` view (FILERESOURCE_SLOTS plus
  slots +0x07C..+0x084, +0x084 = LbdFile__ReleaseDataBlock) and the object's
  `pad30[4]` is split into `s16 unk30` (init -1) and `u16 unk32`. Byte-neutral
  for the ten functions matched earlier this round (whole image green).
- `SeedAndRandom`'s local definition gained an unused second parameter
  (`s32 unused`): PickSoundBank passes one in `$a1`, as game_shell's own
  prototype already says. Byte-neutral for SeedAndRandom.
- No shared header was edited. Other units' prototypes for these functions
  (dream_day.h, class_3bb8c.h, game_application.h) are independent and untouched.

## Naming

- **Name:** `PickSoundBank`
- **Tier:** A
- **Evidence:** returns sSoundBankPaths[sForcedSoundBank - 1] or a random one of the seven (`% SOUND_BANK_COUNT`); its one caller, DayTask__DayTask, hands the path to New_WBgm as the VAB base (include/wbgm.h). gForcedWeeklyGroup was renamed sForcedSoundBank with it.

## Naming history

- Round 100 (bravo, polish): renamed from `PickWeeklyGroup` with tools/rename.py, on the record paths and callers above; previous tier B (head review, round 82: was A. The mechanics are this body's; the purpose word comes from the callers' inherited names in code_1677c.c / GameApplication.h, which are themselves hypotheses, so the name is consistent but not established).
