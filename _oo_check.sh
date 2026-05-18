#!/usr/bin/env bash
set -e
ROOT=/mnt/c/Users/djibi/OneDrive/Bureau/baremetal/llm-baremetal
cd "$ROOT"

CF="-std=c11 -Wall -Wextra -ffreestanding -fno-stack-protector -fpic -fshort-wchar \
    -mno-red-zone -I/usr/include/efi -I/usr/include/efi/x86_64 -DEFI_FUNCTION_WRAPPER \
    -DUEFI_BUILD=1 -O2 -msse2 -DDJIBLAS_DISABLE_CPUID=1 \
    -Icore -Iengine/llama2 -Iengine/gguf -Iengine/djiblas -Iengine/ssm -I."

fail=0

check() {
    local label=$1; shift
    echo -n "  $label ... "
    if gcc $CF "$@" 2>/tmp/oo_err.txt; then
        echo "OK"
    else
        echo "FAIL"
        cat /tmp/oo_err.txt
        fail=1
    fi
}

echo ""
echo "=== OO build check ==="

# New files
check "gguf_kquant.c"        -c engine/gguf/gguf_kquant.c -o /tmp/gguf_kquant.o
check "gguf_infer.c"         -fsyntax-only engine/gguf/gguf_infer.c
check "gguf_portable_stub.c" -fsyntax-only engine/gguf/gguf_portable_stub.c
check "gguf_loader.c"        -fsyntax-only engine/gguf/gguf_loader.c

echo ""
if [ $fail -eq 0 ]; then
    echo "ALL CHECKS PASSED"
else
    echo "SOME CHECKS FAILED"
    exit 1
fi
