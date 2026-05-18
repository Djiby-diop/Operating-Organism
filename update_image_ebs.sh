#!/bin/bash
set -e

IMG="/mnt/c/Users/djibi/OneDrive/Bureau/baremetal/llm-baremetal/llm-baremetal-boot.img"
EFI="/mnt/c/Users/djibi/OneDrive/Bureau/baremetal/llm-baremetal/llama2.efi"
MTRC="/tmp/mtoolsrc_ebs"

printf 'mtools_skip_check=1\ndrive z: file="%s" offset=1048576\n' "$IMG" > "$MTRC"
export MTOOLSRC="$MTRC"

echo "=== Files in image ==="
mdir z:/ 2>&1 | head -10

printf '/ebs_status\n/ebs_prepare\n/ebs_status\n/ebs_mmap\n/shutdown\n' > /tmp/ebs_autorun.txt

mcopy -o "$EFI" z:/EFI/BOOT/BOOTX64.EFI && echo '[OK] EFI updated'
mcopy -o /tmp/ebs_autorun.txt z:/llmk-autorun.txt && echo '[OK] Autorun written'

echo "=== Updated files ==="
mdir z:/ 2>&1 | head -10
