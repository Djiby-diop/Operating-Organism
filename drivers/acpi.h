#pragma once

#include <stdint.h>

/**
 * ACPI Parser — Machine Topology Discovery
 */

typedef struct {
    char     signature[8];
    uint8_t  checksum;
    char     oem_id[6];
    uint8_t  revision;
    uint32_t rsdt_addr;
} __attribute__((packed)) AcpiRsdp;

typedef struct {
    char     signature[4];
    uint32_t length;
    uint8_t  revision;
    uint8_t  checksum;
    char     oem_id[6];
    char     oem_table_id[8];
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
} __attribute__((packed)) AcpiSdtHeader;

typedef struct {
    AcpiSdtHeader header;
    uint32_t      lapic_addr;
    uint32_t      flags;
    // Entries follow...
} __attribute__((packed)) AcpiMadt;

/**
 * Scans memory for the RSDP signature ("RSD PTR ").
 */
AcpiRsdp* acpi_find_rsdp(void);

/**
 * Finds a specific table (e.g. "APIC") in the RSDT.
 */
void* acpi_find_table(AcpiRsdp *rsdp, const char *signature);
