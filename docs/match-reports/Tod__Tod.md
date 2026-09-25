# Tod__Tod -- MATCHED (37/37 words)

> Renamed from `func_80043EE4` on 2026-09-25 (tools/rename.py). Address 0x80043ee4.

Round 82, runner echo (code_33808 session, echo #8), 2026-09-25. Unit `code_33808`.
Byte-exact; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 37/37 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

Constructor: active driver's ctor, install D_8006F240 (GetTodMethods); the argument is a two-word source descriptor { buffer, name }: with a buffer, adopt it (+0x10 = buffer, +0x14 size = 0) and call its own +0x064 (setFlag override); without one, call its own requestLoadFile (+0x06C) with the name.

Table slot (`tools/classtable.py`): D_8006F240 +0x008.

## Source

The unit-local views `DataSrc33808` (Class6D430 subclass via the unified
`CLASS6D430_SLOTS`/`CLASS6D430_FIELDS` macros plus `slot7C`/`slot80` and own
fields +0x2C..+0x38), `Ctor33808` and `CountedBuf33808` sit at the top of
`src/code_33808.c`.

```c
typedef struct Src6F240 {
    /* +0x00 */ void *buffer;
    /* +0x04 */ char *name;
} Src6F240;

void Tod__Tod(DataSrc33808 *self, Src6F240 *src) {
    GetActiveDataSourceMethods()->ctor((Class6D430 *)self);
    self->methods = GetTodMethods();
    if (src->buffer != NULL) {
        self->buffer = src->buffer;
        self->bufferSize = 0;
        self->methods->setFlag(self);
    } else {
        self->methods->requestLoadFile(self, src->name);
    }
}
```

## Notes

First build. The else branch reuses the just-stored table pointer (GCC CSE of `self->methods = getter()`), which the ordinary `self->methods->requestLoadFile` spelling reproduces. The same descriptor is what D_8006F590's ctor (TodSet__TodSet, the subclass) tests with `*arg`.

## Naming

- **Tod__Tod**, tier A. Constructor: adopts a buffer or requests a named file, matching the interface a data source uses for its own resource.
