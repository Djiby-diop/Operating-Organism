#include "acpi.h"
#include <string.h>

AcpiRsdp* acpi_find_rsdp(void) {
    // 1. Scan EBDA or BIOS ROM Area
    // The RSDP is usually in 0xE0000 to 0xFFFFF
    uint8_t *ptr = (uint8_t*)0xE0000;
    
    while ((uintptr_t)ptr < 0x100000) {
        if (memcmp(ptr, "RSD PTR ", 8) == 0) {
            // Checksum validation would go here
            return (AcpiRsdp*)ptr;
        }
        ptr += 16; // RSDP is always aligned to 16 bytes
    }
    
    return NULL;
}

void* acpi_find_table(AcpiRsdp *rsdp, const char *signature) {
    if (!rsdp) return NULL;
    
    AcpiSdtHeader *rsdt = (AcpiSdtHeader*)(uintptr_t)rsdp->rsdt_addr;
    int entries = (rsdt->length - sizeof(AcpiSdtHeader)) / 4;
    uint32_t *entry_ptr = (uint32_t*)(rsdt + 1);
    
    for (int i = 0; i < entries; i++) {
        AcpiSdtHeader *table = (AcpiSdtHeader*)(uintptr_t)entry_ptr[i];
        if (memcmp(table->signature, signature, 4) == 0) {
            return (void*)table;
        }
    }
    
    return NULL;
}
