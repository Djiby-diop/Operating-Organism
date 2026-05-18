# =============================================================================
# VITAL-BAREMETAL — Makefile (Compilation UEFI x86_64 via WSL/GNU-EFI)
# =============================================================================

ARCH = x86_64
CC = gcc
NASM = nasm

# GNU-EFI paths
MULTIARCH ?= $(shell $(CC) -print-multiarch 2>/dev/null)
EFI_LIBDIR_CANDIDATES := /usr/lib /usr/lib/$(MULTIARCH)

EFI_LDS := $(firstword $(wildcard $(addsuffix /elf_$(ARCH)_efi.lds,$(EFI_LIBDIR_CANDIDATES))))
EFI_CRT0 := $(firstword $(wildcard $(addsuffix /crt0-efi-$(ARCH).o,$(EFI_LIBDIR_CANDIDATES))))
EFI_LIBDIR := $(firstword $(foreach d,$(EFI_LIBDIR_CANDIDATES),$(if $(wildcard $(d)/libgnuefi.a),$(d),)))

ifeq ($(strip $(EFI_LDS)),)
$(error Could not find elf_$(ARCH)_efi.lds (install gnu-efi))
endif
ifeq ($(strip $(EFI_CRT0)),)
$(error Could not find crt0-efi-$(ARCH).o (install gnu-efi))
endif

CFLAGS = -ffreestanding -fno-stack-protector -fpic -fshort-wchar -mno-red-zone \
         -I/usr/include/efi -I/usr/include/efi/$(ARCH) -DEFI_FUNCTION_WRAPPER \
         -Wall -Wextra -O2 -Wno-unused-parameter -Wno-unused-variable \
         -I include \
         -I ../united-baremetal/include \
         -I ../kernel-baremetal/include \
         -I ../swarm-baremetal/include \
         -I ../llm-baremetal/oo-storage/core \
         -DVITAL_BAREMETAL

LDFLAGS = -nostdlib -znocombreloc -T $(EFI_LDS) \
          -shared -Bsymbolic -L$(EFI_LIBDIR) $(EFI_CRT0)

LIBS = -lefi -lgnuefi

C_SOURCES = \
    src/uefi_entry.c \
    src/life_loop.c \
    src/consciousness_fsm.c \
    src/homeostasis_regulator.c \
    src/nociception.c \
    src/energy_metabolism.c \
    src/synapse_network.c \
    src/dream_simulator.c \
    src/epigenetics.c \
    src/aura_visualizer.c \
    src/biotic_entropy.c \
    src/metabolic_forth.c \
    src/swarm_sync.c \
    src/vital_core_unifier.c \
    src/pineal_gland.c \
    src/vital_persistence.c \
    src/oo_stubs.c

ASM_SOURCES = \
    src/ouroboros.asm \
    src/vital_beat.asm

RUST_SOURCES = \
    src/quantum_vault.rs

# Objets pour la bibliothèque (pas de stubs de bus)
RUST_OBJECTS = $(RUST_SOURCES:.rs=.o)
LIB_OBJECTS = $(C_SOURCES:.c=.o) $(ASM_SOURCES:.asm=.o) $(RUST_OBJECTS)

# Objets pour le binaire standalone (avec stubs de bus)
STANDALONE_SOURCES = src/united_bus_stub.c
STANDALONE_OBJECTS = $(STANDALONE_SOURCES:.c=.o)
ALL_OBJECTS = $(LIB_OBJECTS) $(STANDALONE_OBJECTS)

TARGET_SO = vital.so
TARGET = vital.efi
TARGET_LIB = liboo-vital.a

all: $(TARGET) $(TARGET_LIB)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.asm
	$(NASM) -f elf64 $< -o $@

%.o: %.rs
	rustc --crate-type staticlib -C panic=abort -C lto -O --emit obj $< -o $@

src/united_bus_stub.o: src/united_bus_stub.c
	$(CC) $(CFLAGS) -DVITAL_STANDALONE -c $< -o $@

$(TARGET_LIB): $(LIB_OBJECTS)
	ar rcs $(TARGET_LIB) $(LIB_OBJECTS)
	@echo "  $(TARGET_LIB) créé."

$(TARGET_SO): $(ALL_OBJECTS)
	ld $(LDFLAGS) --allow-multiple-definition $(ALL_OBJECTS) -o $(TARGET_SO) $(LIBS)

$(TARGET): $(TARGET_SO)
	objcopy -j .text -j .sdata -j .data -j .dynamic -j .dynsym \
		-j .rel -j .rela -j .reloc --target=efi-app-$(ARCH) $(TARGET_SO) $(TARGET)
	@echo "================================================"
	@echo "  vital.efi compilé avec succès (GNU-EFI) !"
	@ls -lh $(TARGET)
	@echo "================================================"

clean:
	rm -f $(ALL_OBJECTS) $(TARGET_SO) $(TARGET) $(TARGET_LIB)

.PHONY: all clean
