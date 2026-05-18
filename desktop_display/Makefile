# SOMA Desktop -- Bare-Metal UEFI Build
ARCH     = x86_64
CC       = gcc
OBJCOPY  = objcopy

MULTIARCH    ?= $(shell $(CC) -print-multiarch 2>/dev/null)
EFI_LIBDIR_CANDIDATES := /usr/lib /usr/lib/$(MULTIARCH)

EFI_LDS    := $(firstword $(wildcard $(addsuffix /elf_$(ARCH)_efi.lds,$(EFI_LIBDIR_CANDIDATES))))
EFI_CRT0   := $(firstword $(wildcard $(addsuffix /crt0-efi-$(ARCH).o,$(EFI_LIBDIR_CANDIDATES))))
EFI_LIBDIR := $(firstword $(foreach d,$(EFI_LIBDIR_CANDIDATES),$(if $(wildcard $(d)/libgnuefi.a),$(d),)))

CFLAGS = \
  -ffreestanding \
  -fno-stack-protector \
  -fpic \
  -fshort-wchar \
  -mno-red-zone \
  -I/usr/include/efi \
  -I/usr/include/efi/$(ARCH) \
  -DEFI_FUNCTION_WRAPPER \
  -O2 -msse2 -Wall -Wextra -Wno-unused-parameter -DUEFI_BUILD=1

LDFLAGS = \
  -nostdlib -znocombreloc \
  -T$(EFI_LDS) \
  -shared -Bsymbolic \
  -L$(EFI_LIBDIR) -lgnuefi -lefi

.PHONY: all clean

all: soma.efi

oo_hud_final.o: oo_hud_final.c oo_soma_bridge.h
	$(CC) $(CFLAGS) -c $< -o $@

soma.so: $(EFI_CRT0) oo_hud_final.o
	$(CC) $(LDFLAGS) $^ -o $@

soma.efi: soma.so
	$(OBJCOPY) -j .text -j .sdata -j .data -j .dynamic -j .dynsym -j .rel -j .rela -j .reloc --target=efi-app-$(ARCH) $^ $@
	@echo "Built: soma.efi"

clean:
	rm -f *.o *.so soma.efi
