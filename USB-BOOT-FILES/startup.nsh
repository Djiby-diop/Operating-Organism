@echo -off
# OO startup.nsh — UEFI Shell autorun
# Executed automatically by UEFI firmware when a UEFI Shell is the bootloader,
# or manually from the UEFI Shell prompt.
echo.
echo  =========================================
echo   OO - Operating Organism  v0.3
echo   Bare-metal LLM + Biological OS kernel
echo  =========================================
echo.
echo  Loading OO kernel from EFI partition...
echo.
# Try primary EFI binary
if exist fs0:\EFI\BOOT\BOOTX64.EFI then
  fs0:\EFI\BOOT\BOOTX64.EFI
  goto done
endif
# Fallback: try other filesystems
if exist fs1:\EFI\BOOT\BOOTX64.EFI then
  fs1:\EFI\BOOT\BOOTX64.EFI
  goto done
endif
echo  [ERROR] BOOTX64.EFI not found on fs0: or fs1:
echo  Check USB structure: EFI\BOOT\BOOTX64.EFI must exist.
echo.
:done
