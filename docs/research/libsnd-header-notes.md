# libsnd headers: notes moved out of the API docs

Track 12 (round 106, area-psyq-shared) rewrote `include/libsnd_internal.h`,
`include/ss_score.h` and `include/svm_data.h` as Doxygen API documentation.
Text that said how a declaration was derived or kept for its bytes, rather
than what it declares, and that belongs to no single function's report,
moved here verbatim. A `MATCHING:` note that justifies one spelling now also
stands as one line in the `.c` beside the code it justifies (listed below).

## History (source comments moved in track 12, round 106)

### include/libsnd_internal.h

The declaration of SpuVmAlloc (now documented in prose on the prototype,
with an `arity-ok` note on the line itself):

```c
/* MATCHING: declared without a parameter list. The voice manager's own
 * calls pass an argument (0 or 0xFF) and SsUtKeyOn/SsUtKeyOnV pass none;
 * the definition reads no argument register. */
```

Above SpuVmSetSeqVol (the first sentence is now in the file doc; the
MATCHING note is one line at Snd_crescendo's first call in
`src/psyq/libsnd_cres.c`):

```c
/* A sequence is named by one packed number, seqSepNo: the SEQ/SEP access in
 * the low byte, the sequence in the high byte. MATCHING: SpuVmSetSeqVol's
 * volumes are u16; Snd_crescendo's calls mask them with andi 0xFFFF. */
```

The _svm_cur block's opening and closing lines (the build detail, and which
unit declares the tone which way; each unit's own MATCHING line on its
declaration of the tone says why):

```c
 * _svm_cur (pinned in config/psyq-objects.ld, 0x20 bytes): the key-on
 ...
 * one above. +0x0C, the tone, is declared by each unit that reads it:
 * libsnd_vmanager.c reads it plain, libsnd_vm_vol_ut_key_ut_keyv.c volatile.
```

The voice byte at `_svm_cur + 0x1A` (the qualifier's reason is one line in
`src/psyq/libsnd_vmanager.c`, at SpuVmKeyOff's store and reload):

```c
/* +0x1A: the voice being keyed. MATCHING: volatile; the key paths store it
 * and read it straight back, and without the qualifier cc1 drops the
 * reload. */
extern volatile u16 D_8008EA26;
```

### include/ss_score.h

From the banner: "_ss_score (pinned in config/psyq-objects.ld) is an array
of pointers, one per open SEQ/SEP access". The field at +0x98,
fadeTicksLeft, said "Snd_setvol_data stores v_time (in unk94 too)"; there is
no unk94 field (+0x94 is padding), so the doc now says "(and at +0x94,
undeclared)".

### include/svm_data.h

From the banner. The MATCHING note is one line above SePitchBend in
`src/psyq/libsnd_vmanager.c`:

```c
 * site, (u16)v.note and (u8)v.progIndex. MATCHING: an address cast,
 * *(u8 *)&v.progIndex, grows SePitchBend's frame by 8 bytes. SpuVmFlush walks
 * envx as u16 for its pointer stride.
 *
 * asm/ names this table's fields by address (D_8008D98A, D_8008D98C, ...
 * at a 0x34 stride); the linker resolves those and this struct to the same
 * bytes.
```

From the SpuRegs block: "Units declare _svm_sreg themselves, each with the
pointee spelling its bodies match against."
