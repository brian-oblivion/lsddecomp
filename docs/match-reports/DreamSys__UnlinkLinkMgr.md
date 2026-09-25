# DreamSys__UnlinkLinkMgr

> Renamed from `func_80058A94` on 2026-09-22 (tools/rename.py). Address 0x80058a94.

**Unit:** DreamSys · **Size:** 29 instructions · **Status:** MATCHED (29/29 words)

## What it does

Vtable slot `+0x050`. Three calls in sequence, each through a different
class's method table:

1. `this->unk_0x4C->methods->slot0xF0(this->unk_0x4C)` -- a call through an
   unidentified object's OWN vtable (not `this->vt`), with itself as the
   sole argument.
2. `this->vt->Actor__RemoveChild(this, this->unk_0x4C)` -- resolved via
   `tools/classtable.py DREAMSYS_METHODS` to vtable offset `+0x014`, shared
   with `Class65650`'s inherited "slot14" (`code_55dd4.h` calls it the
   "'unlink' companion of slot10"). Still `INCLUDE_ASM`; its address
   (`0x80057130`) is below this unit/runner's range.
3. `GetActorMethods()->slot0x50(this)` -- the shared intermediate base class
   table (`gActorMethods`), same one `code_55dd4.h`'s `D800878D4Methods` types
   for `Class65650` (a DreamSys sibling under that base, per
   `docs/research/class-framework.md`).

## The C

```c
void DreamSys__UnlinkLinkMgr(DreamSys *this)
{
	this->unk_0x4C->methods->slot0xF0(this->unk_0x4C);
	this->vt->Actor__RemoveChild(this, this->unk_0x4C);
	GetActorMethods()->slot0x50(this);
}
```

New field/types added to `include/DreamSys.h`:

```c
typedef struct DreamSysUnk4CMethods {
	u8 pad00[0xF0];
	void (*slot0xF0)(void *self);
} DreamSysUnk4CMethods;
typedef struct DreamSysUnk4CObj {
	DreamSysUnk4CMethods *methods;
} DreamSysUnk4CObj;
/* ... inside DreamSys ... */
DreamSysUnk4CObj *unk_0x4C;
```

```c
typedef struct DreamSysBaseMethods {
	u8 pad00[0x50];
	void (*slot0x50)(struct DreamSys *self);
} DreamSysBaseMethods;
extern DreamSysBaseMethods *GetActorMethods(void);
```

## Note: deliberate `struct DreamSys *`, not `DreamSys *`, in `DreamSysBaseMethods`

`DreamSysBaseMethods` is declared ahead of `typedef struct DreamSys {...}
DreamSys;` in the header, so GCC 2.6.3 warns `struct DreamSys' declared
inside parameter list ... probably not what you want` for the `self`
parameter -- it scopes a fresh, distinct `struct DreamSys` tag there. **Do
NOT try to fix this with a forward `typedef struct DreamSys DreamSys;`** --
tried it, and the LATER real typedef then errors with `redefinition of
'DreamSys'` in every OTHER unit that includes this header (surfaced in
`code_1677c.c`, not in `DreamSys.c` itself, because of include-order
differences -- easy to miss if you only rebuild the one file you touched).
The warning is cosmetic: both tags are pointer-compatible at the MIPS ABI
level (a pointer is a pointer, regardless of struct tag), and the whole-image
SHA1 confirms this produces byte-identical output.

## Provenance

round 2026-08-30-b, runner ALPHA, address range
`0x80058774`-`0x8005A1EC`.

## Naming

- **Tier B.** Calls the linkMgr companion's own slot0xF0, then vt->Actor__RemoveChild(this, linkMgr) (the ctor's slot10 buddy-link's own unlink counterpart), then the shared base's own +0x050 slot. Mechanics (undoes the ctor's companion link) are solid; why it is invoked is not.
