#!/bin/bash
IMG='/mnt/c/Users/djibi/OneDrive/Bureau/baremetal/llm-baremetal/llm-baremetal-boot.img'
if [ ! -f "$IMG" ]; then echo "NOT FOUND"; exit 1; fi
echo "Image size: $(stat -c '%s' "$IMG") bytes"
echo "--- fdisk ---"
fdisk -l "$IMG" 2>&1 | head -20
echo "--- mtools EFI/BOOT ---"
mdir -i "${IMG}@@1048576" ::EFI/BOOT/ 2>&1 | head -20
echo "--- mtools root ---"
mdir -i "${IMG}@@1048576" :: 2>&1 | head -20
