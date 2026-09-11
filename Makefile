# LSD: Dream Emulator (PSX / SLPS-01556) — matching decompilation.
#
# DO NOT RUN `make` DIRECTLY to build the executable. `./build-and-verify.sh`
# is the canonical entry point and the only thing that checks the result
# against the retail bytes; a build that is not verified tells you nothing.
# `make extract` is the one target meant to be run by hand.

GAME_ID   := slps01556
GAME      := lsdde
TARGET    := SLPS_015.56

# --- toolchain -------------------------------------------------------------
# Everything is local to the repo, built or fetched by tools/setup.sh. Nothing
# here comes from $PATH: a host binutils that happens to be installed is how
# two machines silently build different bytes.
CROSS     := tools/binutils/bin/mipsel-linux-gnu-
AS        := $(CROSS)as
LD        := $(CROSS)ld
NM        := $(CROSS)nm
OBJCOPY   := $(CROSS)objcopy
OBJDUMP   := $(CROSS)objdump

# GCC 2.6.3, the Psy-Q compiler ("Sony Playstation" is in cc1's own banner).
# `cpp` is 2.6.3's preprocessor too — a modern cpp expands differently enough
# to move code.
GCC_DIR   := tools/gcc263
CPP       := $(GCC_DIR)/cpp
CC1       := $(GCC_DIR)/cc1

PYTHON    := .venv/bin/python3
SPLAT     := $(PYTHON) -m splat split
MASPSX    := $(PYTHON) tools/maspsx/maspsx.py

# --- flags -----------------------------------------------------------------
# NOTE `-fno-builtin` is a cc1 flag only; 2.6.3's cpp rejects it outright.
CPP_FLAGS  := -Iinclude -Iinclude/psyq -undef -Wall -lang-c -nostdinc
CPP_FLAGS  += -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx
CPP_FLAGS  += -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL
CPP_FLAGS  += -D_LANGUAGE_C -DLANGUAGE_C

CC_FLAGS   := -mips1 -mcpu=3000 -quiet -Wall -fno-builtin -mno-abicalls
CC_FLAGS   += -funsigned-char -G0 -O2

MASPSX_FLAGS := --aspsx-version=2.34 --dont-force-G0 --expand-div --addiu-at

AS_FLAGS   := -Iinclude -Iinclude/psyq -march=r3000 -mtune=r3000 -EL
AS_FLAGS   += -no-pad-sections -G0 -O2

LD_FLAGS   := --no-check-sections -nostdlib -EL

# --- files -----------------------------------------------------------------
BUILD_DIR := build
CONFIG    := config

# `find`, not `wildcard`: wildcard has no recursive form, so the moment src/
# grows a subdirectory it silently returns fewer units and every count derived
# from it drops without a word.
S_FILES  := $(shell find asm -name '*.s' -not -path 'asm/nonmatchings/*' 2>/dev/null)
C_FILES  := $(shell find src -name '*.c' 2>/dev/null)
# Prebuilt Psy-Q library objects (splat `o` segments). lib/ is produced from
# the user's own SDK disc by tools/setup.sh and is gitignored; the linker
# script names them under build/lib/, so they are mirrored there.
LIB_FILES := $(shell find lib -name '*.o' 2>/dev/null)
O_FILES  := $(foreach f,$(S_FILES),$(BUILD_DIR)/$(f).o) \
            $(foreach f,$(C_FILES),$(BUILD_DIR)/$(f).o) \
            $(foreach f,$(LIB_FILES),$(BUILD_DIR)/$(f))

ELF      := $(BUILD_DIR)/$(GAME).elf
EXE      := $(BUILD_DIR)/$(TARGET)

.PHONY: all build check extract clean format expected diff-init progress
.DEFAULT_GOAL := all

all: build check

build: $(EXE)

check:
	sha1sum --check build.sha1

# --- link ------------------------------------------------------------------
$(EXE): $(ELF)
	$(OBJCOPY) -O binary $< $@

# psyq-objects.ld goes FIRST: it claims the Sony objects' bss sections (NOLOAD,
# pinned) before the splat script's trailing /DISCARD/ would swallow them.
$(ELF): $(O_FILES) $(GAME).ld $(CONFIG)/psyq-objects.ld
	@mkdir -p $(dir $@)
	$(LD) -o $@ \
		-Map $(BUILD_DIR)/$(GAME).map \
		-T $(CONFIG)/psyq-objects.ld \
		-T $(GAME).ld \
		-T $(CONFIG)/undefined_syms_auto.$(GAME_ID).$(GAME).txt \
		-T $(CONFIG)/undefined_funcs_auto.$(GAME_ID).$(GAME).txt \
		$(LD_FLAGS)

# --- compile ---------------------------------------------------------------
$(BUILD_DIR)/lib/%.o: lib/%.o
	@mkdir -p $(dir $@)
	cp $< $@

$(BUILD_DIR)/%.s.o: %.s
	@mkdir -p $(dir $@)
	$(AS) $(AS_FLAGS) -o $@ $<

# The Psy-Q pipeline. maspsx sits between cc1 and gas because Sony's ASPSX
# assembler expanded macros (div, li, ...) differently from GNU as, and those
# expansions are part of the retail bytes.
#
# A unit's .c depends on every .s it INCLUDE_ASMs, so re-extracting or matching
# a function rebuilds the unit.
#
# IT ALSO DEPENDS ON EVERY HEADER, coarsely and on purpose. Without this a
# header edit does not rebuild anything, `make` reports success, and the next
# funcdiff scores a struct layout you changed minutes ago against an object
# that never saw it -- a stale number that is plausible and self-consistent,
# which is the worst kind. A full build is a few seconds; correctness is
# cheaper than the fine-grained version here.
HEADERS := $(wildcard include/*.h include/*.inc include/psyq/*.H \
                      include/psyq/SYS/*.H)

.SECONDEXPANSION:
$(BUILD_DIR)/%.c.o: %.c $(HEADERS) $$(wildcard asm/nonmatchings/$$(notdir $$*)/*.s)
	@mkdir -p $(dir $@)
	$(CPP) $(CPP_FLAGS) $< | $(CC1) $(CC_FLAGS) | $(MASPSX) $(MASPSX_FLAGS) | \
		$(AS) $(AS_FLAGS) -o $@

# --- extract ---------------------------------------------------------------
# splat NEVER deletes what it stops generating, and asm/ is gitignored, so
# orphaned .s files accumulate in a working checkout forever. They corrupt
# every tool that treats the nonmatchings tree as ground truth (ownership,
# hazard scans, retail-side instruction counts). Wiping first makes the tree
# mean what it looks like it means.
extract:
	rm -rf asm/nonmatchings
	$(SPLAT) $(CONFIG)/splat.$(GAME_ID).$(GAME).yaml

# --- housekeeping ----------------------------------------------------------
clean:
	rm -rf $(BUILD_DIR) asm $(GAME).ld
	rm -f $(CONFIG)/undefined_syms_auto.*.txt $(CONFIG)/undefined_funcs_auto.*.txt

format:
	clang-format -i $$(find src include -name '*.c' -o -name '*.h' | grep -v include/psyq)

expected: check
	rm -rf expected/build && mkdir -p expected && cp -r $(BUILD_DIR) expected/

progress:
	$(PYTHON) tools/progress.py

SHELL = /bin/bash -e -o pipefail
