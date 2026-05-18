@echo -off
echo "==> Booting Bot-Baremetal SwarmMind..."
fs0:
cd EFI\BOOT
BOOTX64.EFI
reset
