# TimImage__Finalize -- MATCHED (14/14 words), round 81

> Renamed from `func_8003B470` on 2026-09-25 (tools/rename.py). Address 0x8003b470.

Round 81, runner echo. Unit `src/TimImage.c`. Fresh ground, no prior attempt.

- **Where:** TimImage's table (`gTimImageMethods`) slot +0x00C (finalize; `tools/classtable.py D_8006E558`).
- **What:** forwards to the active data-source driver's finalize
  (`GetActiveDataSourceMethods()->finalize(self)`). Written `void`: the
  base's `finalize` slot is `void`, and a byte match of a forwarder says
  nothing about the return type either way.
- **Result:** byte-exact on the first build, whole-image SHA1 green.

## Source

```c
void TimImage__Finalize(TimImage *self) {
    GetActiveDataSourceMethods()->finalize((FileResource *)self);
}
```

## Naming

- **`TimImage__Finalize`**, tier A: dispatched from slot +0x00C
  (`FileResource`'s `finalize` slot), a pure forward to the active
  data-source driver's own finalize -- mechanics are the whole purpose.
