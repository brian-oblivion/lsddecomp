# DreamSys__func_58968

**Unit:** DreamSys · **Size:** 75 words (0x12C bytes) · **Status:** MATCHED
(75/75 words, whole-image `./build-and-verify.sh` green)

## What it does

`DREAMSYS_METHODS` slot `+0x04C` (this function's own slot, confirmed via
`tools/classtable.py`). Forwards `arg1` through two base-class hooks and the
"link" step, and — while a flashback is in the specific `0xE` link state —
consumes the CURRENT flashback entry's rotation/time-limit and advances the
flashback playback index:

```c
void DreamSys__func_58968(DreamSys *this, DreamSysFunc58968ArgObj *arg1)
{
	s32 local[4];

	arg1->methods->slot0xE4(arg1, local, this, &this->linkCoordinates);
	func_80057C84()->slot4C(this, arg1, local);
	this->vt->slot10(this, arg1);
	if (this->unknwon_int_0x44 == 0xE) {
		FlashbackEntry *entry = &this->storedFlasbacks[this->currentFlashbackIndex];
		this->vt->func_8001CEB4(this, 1, &entry->rotation);
		this->vt->GetSetDreamTimeLimit(this, entry->timeLimit + 4);
		this->currentFlashbackIndex++;
	}
	if (this->unk_0x6c != 0 && this->unk_0x888 != 0) {
		this->vt->func_8001CEB4(this, 1, (void *)this->unk_0x888);
	}
}
```

## The one wrinkle: a field mix-up caught immediately by the byte diff

First attempt reached 72/75 with the ONLY residue being three instructions
reading/writing offset `0x87C` in retail versus `0x878` in the build — every
other word matched, including branch targets, register allocation, and the
tricky `entry->timeLimit + 4` and 36-byte-stride index computation. `0x878`
is `DreamSys::unk_0x878`, a field several already-matched functions use
(`func_8005A33C`, `func_8005A344`, `DreamSys__func_588ec`,
`DreamSys__EndDay`) — so it could not itself be at the wrong offset. The
actual field at `0x87C` is the VERY NEXT declared field,
`DreamSys::currentFlashbackIndex`, which is semantically the right one
anyway: it is the flashback-playback index this function reads to pick the
current `storedFlasbacks` entry and increments afterward — exactly the
`DreamSys__LoadNextFlashback`/`DreamSys__StartDay` field, not the unrelated
`unk_0x878` counter. Swapping the field name (same offset math otherwise)
closed the last 3 words immediately.

## New knowledge

- **`DreamSysBaseMethods` (this unit's local view of the shared `D_800878D4`
  base table) gets a new slot at `+0x04C`: `slot4C`.** Cross-confirmed
  against `code_55dd4.h`'s `D800878D4Methods`, which already names this exact
  slot (same offset in the same shared table) and describes its call shape
  as `(self, arg1, arg2)` — matching this call site's `(this, arg1, &local)`
  exactly.
- **`DreamSysFunc58968ArgObj`**, a new minimal opaque class for this
  function's own `arg1` parameter (same "vtable pointer at offset 0" shape
  as this unit's other opaque views). Its `slot0xE4(self, &local, this,
  &this->linkCoordinates)` return value is discarded. Kept separate from
  `DreamSysCtorArgObj` (the constructor's own opaque `arg1` type) per this
  unit's "multiple independent local views" convention — both are eventually
  forwarded into `vt->slot10`, one directly and one via a derived value, but
  nothing proves they are the same underlying class.
- **`FlashbackEntry::rotation` and `FlashbackEntry::timeLimit` confirmed
  in a NEW caller**, read via real struct field access
  (`&this->storedFlasbacks[i].rotation`, `.timeLimit`) rather than the raw
  `s0+0xE`/`s0+0x1A` pointer arithmetic the disassembly shows — the 36-byte
  stride the disassembly computes (`v0*9<<2`) is exactly `sizeof(FlashbackEntry)`
  as already declared, confirming that struct's layout independently of the
  round that first derived it.

### Proposed learning

**When a residue is "a field at the wrong offset by exactly the size of an
adjacent field," check whether a DIFFERENT, already-declared field belongs
at that address before assuming the current field's offset is wrong** —
especially when the current field is independently confirmed correct by
several already-matched functions. Here the fix was not a struct layout
correction at all, just picking the right one of two adjacent fields.
