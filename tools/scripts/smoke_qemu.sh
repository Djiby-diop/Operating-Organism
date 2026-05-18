#!/bin/bash
# OO QEMU smoke test — Phase 6 verification
OVMF=/usr/share/OVMF/OVMF_CODE.fd
# Try alternate OVMF locations
[ -f "$OVMF" ] || OVMF=/usr/share/ovmf/OVMF.fd
[ -f "$OVMF" ] || OVMF=/usr/share/edk2/ovmf/OVMF_CODE.fd
[ -f "$OVMF" ] || OVMF=$(find /usr -name "OVMF_CODE*.fd" 2>/dev/null | head -1)
[ -f "$OVMF" ] || OVMF=$(find /usr -name "OVMF.fd" 2>/dev/null | head -1)

IMG=/mnt/c/Users/djibi/OneDrive/Bureau/baremetal/llm-baremetal/llm-baremetal-boot.img
UART=/tmp/oo_smoke_uart.log
TIMEOUT=90

echo "[smoke] OVMF: $OVMF"
echo "[smoke] IMG : $IMG"
echo "[smoke] qemu: $(which qemu-system-x86_64 2>/dev/null || echo NOT_FOUND)"

if [ ! -f "$OVMF" ]; then
    echo "[FAIL] OVMF not found — trying to install..."
    sudo apt-get install -y ovmf 2>&1 | tail -3
    OVMF=$(find /usr -name "OVMF_CODE*.fd" 2>/dev/null | head -1)
    [ -f "$OVMF" ] || { echo "[FAIL] OVMF still not found. Aborting."; exit 1; }
fi

if [ ! -f "$IMG" ]; then
    echo "[FAIL] Boot image not found: $IMG"; exit 1
fi

if ! command -v qemu-system-x86_64 &>/dev/null; then
    echo "[FAIL] qemu-system-x86_64 not found — trying to install..."
    sudo apt-get install -y qemu-system-x86 2>&1 | tail -3
fi

rm -f "$UART"
echo "[smoke] Booting OO in QEMU (timeout ${TIMEOUT}s)..."

timeout $TIMEOUT qemu-system-x86_64 \
  -machine q35,accel=tcg \
  -cpu max \
  -m 4096 \
  -drive "if=pflash,format=raw,readonly=on,file=$OVMF" \
  -drive "format=raw,file=$IMG" \
  -serial "file:$UART" \
  -nographic -vga none \
  -no-reboot 2>/tmp/qemu_err.log
QEMU_EXIT=$?

echo ""
echo "[smoke] QEMU exit code: $QEMU_EXIT (124=timeout, 0=clean exit)"
echo ""
echo "=== UART output (last 50 lines) ==="
if [ -f "$UART" ]; then
    tail -50 "$UART"
else
    echo "(no UART output captured)"
fi
echo ""
echo "=== QEMU stderr ==="
cat /tmp/qemu_err.log 2>/dev/null | head -20

# Checks
echo ""
echo "=== SMOKE CHECK RESULTS ==="
PASS=0; FAIL=0

check() {
    local label="$1"; local pattern="$2"
    if grep -q "$pattern" "$UART" 2>/dev/null; then
        echo "  [PASS] $label"
        PASS=$((PASS+1))
    else
        echo "  [FAIL] $label (pattern: $pattern)"
        FAIL=$((FAIL+1))
    fi
}

check "UEFI boot started"      "UEFI\|EFI\|EDK"
check "OO kernel loaded"       "OO\|soma\|llama\|OO>"
check "Memory init"            "zone\|mem\|MEM\|alloc"
check "Boot mark"              "boot_mark\|phase\|PHASE\|init"

echo ""
if [ $FAIL -eq 0 ]; then
    echo "[RESULT] ALL $PASS checks PASSED ✅"
else
    echo "[RESULT] $PASS passed, $FAIL FAILED ❌"
fi
