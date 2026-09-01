# func_8004ADD0

**Unit:** class_3ac78 · **Size:** 2 instructions · **Status:** MATCHED (2/2 words)

## What it does

A one-field pointer setter -- `self->unkE8 = arg1;`, a plain
`sw $a1, 0xE8($a0)` with no other work.

## The C

```c
void func_8004ADD0(Class866E8 *self, void *arg1)
{
	self->unkE8 = arg1;
}
```

`+0xE8` is read elsewhere in this unit (`func_8004ADD8`, still
`INCLUDE_ASM`) as a linked-list-style head pointer -- typed `void *` here
since this unit's queued work for this round doesn't need more precision
than that.

## Provenance

round 2026-09-01, runner charlie, unit class_3ac78 (first pass, unit carved
this round).
