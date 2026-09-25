#ifndef SVMVOICE_H
#define SVMVOICE_H

#include "common.h"

/*
 * SvmVoice -- one record of libsnd's per-voice table _svm_voice.
 *
 * Sony's, not the game's. libsnd/vmanager.o (Psy-Q disc 3.5) defines the
 * bss block _svm_sreg_buf +0x000, _svm_sreg_dirty +0x180, _svm_voice
 * +0x198, _svm_envx_ptr +0x678, _svm_envx_hist +0x67C. Anchored at
 * 0x8008D7F0 every one of them lands, so _svm_voice = 0x8008D988 and its
 * 0x4E0 bytes are 24 voices x 0x34 (ending exactly at _svm_envx_ptr).
 *
 * The type name is derived from Sony's VARIABLE name, because Sony's own
 * struct tag is unknown: no libsnd internal header ships on any SDK disc.
 * For the same reason every field is named by its OFFSET only. Per
 * FINISHING-PLAN's Sony rule (track 3, "Naming rules") NO game name goes
 * on any field of this struct: only libsnd functions read it. A comment
 * describing a field's MECHANICS is fine and is kept below.
 *
 * Field types are what the accessors need (track 4b step 1): the width
 * every accessor agrees on, and the signedness most of them read. An
 * accessor that needs the other signedness casts at the site, or keeps a
 * unit-local view where a cast changes bytes (each such view names the
 * accessor whose bytes need it).
 *
 * Earlier rounds split this table into one splat symbol per field
 * (D_8008D98A, D_8008D98C, ... at a 0x34 stride) and gave twelve of them
 * game names; round 86 (alpha, track 2) merged them back into this one
 * table. splat still prints the per-address auto-symbols in asm/ (the
 * table lies past the global segment's vram range, so splat never hands
 * _svm_voice's size to spimdisasm); the linker resolves both spellings to
 * the same bytes.
 */
typedef struct SvmVoice {
    s16 unk00; /* +0x00 -- 0xFF when free; SsUtKeyOff/SsUtKeyOffV compare and clear it */
    s16 unk02; /* +0x02 -- SsUtAllKeyOff/SpuVmInit set it to 0x18; key-on clears it; SpuVmAlloc ages it */
    s16 unk04; /* +0x04 -- cleared on every key-off path */
    s16 unk06; /* +0x06 -- SpuVmAlloc's secondary allocation key */
    s16 unk08; /* +0x08 -- level scaled by SpuVmSetVol (x vol / 127) */
    u8 unk0A;  /* +0x0A -- byte, 0x40 at init; SpuVmKeyOn stores its 6th argument */
    u8 pad0B;
    s16 unk0C; /* +0x0C -- note; SePitchBend adds the bend to it */
    s16 unk0E; /* +0x0E -- 0x21 after SsUtKeyOn/SsUtKeyOnV, 0xFF when free */
    s16 unk10; /* +0x10 -- index into D_8008E968 (read as a byte by SePitchBend) */
    s16 unk12; /* +0x12 -- compared against a key-off argument */
    s16 unk14; /* +0x14 -- index into D_8008E978 (read as a byte by SePitchBend), 0xFF when free */
    s16 unk16; /* +0x16 -- compared against a key-off argument */
    s16 unk18; /* +0x18 -- SpuVmAlloc's priority, loaded from D_8008EA1B */
    u8 pad1A;
    u8 unk1B;  /* +0x1B -- byte state: 1 keyed on, 2 noise, 0 off */
    /* +0x1C..+0x26: the SeAutoVol/SetAutoVol ramp. */
    s16 unk1C; /* +0x1C -- nonzero while the ramp runs; SeAutoVol sets it, SetAutoVol clears it when +0x24 reaches +0x26 */
    s16 unk1E; /* +0x1E -- per-tick increment/decrement applied to +0x24 */
    s16 unk20; /* +0x20 -- ticks between steps (0 = every tick) */
    s16 unk22; /* +0x22 -- countdown to the next step, reloaded from +0x20 */
    s16 unk24; /* +0x24 -- running ramp value, SeAutoVol's "from" */
    s16 unk26; /* +0x26 -- value the ramp clamps to, SeAutoVol's "to" */
    /* +0x28..+0x32: the SeAutoPan/SetAutoPan ramp, same mechanics. */
    s16 unk28; /* +0x28 -- nonzero while the ramp runs; SeAutoPan sets it, SetAutoPan clears it when +0x30 reaches +0x32 */
    s16 unk2A; /* +0x2A -- per-tick increment/decrement applied to +0x30 */
    s16 unk2C; /* +0x2C -- ticks between steps (0 = every tick) */
    s16 unk2E; /* +0x2E -- countdown to the next step, reloaded from +0x2C */
    s16 unk30; /* +0x30 -- running ramp value, SeAutoPan's "from" */
    s16 unk32; /* +0x32 -- value the ramp clamps to, SeAutoPan's "to" */
} SvmVoice; /* 0x34 */

extern SvmVoice _svm_voice[]; /* 24 voices */

#endif
