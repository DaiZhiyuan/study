#include <stdio.h>
#include <stdint.h>

typedef struct {
    uint16_t vendor_id;
    uint16_t device_id;
    uint16_t command;
    uint16_t status;
    uint8_t revision_id;
    uint8_t prog_if;
    uint8_t sub_class;
    uint8_t base_class;
    uint8_t cache_line_size;
    uint8_t latency_timer;
    uint8_t header_type;
    uint8_t bist;
    uint32_t bar0;
    uint32_t bar1;
    uint32_t bar2;
    uint32_t bar3;
    uint32_t bar4;
    uint32_t bar5;
    uint32_t cardbus_cis_pointer;
    uint16_t subsystem_vendor_id;
    uint16_t subsystem_id;
    uint32_t expansion_rom_base_address;
    uint8_t capabilities_pointer;
    uint8_t reserved[3];
    uint32_t reserved2;
    uint8_t interrupt_line;
    uint8_t interrupt_pin;
    uint8_t min_grant;
    uint8_t max_latency;
} PciConfigSpace;

/*
00:03.0 Ethernet controller: Intel Corporation 82540EM Gigabit Ethernet Controller (rev 03)
        Subsystem: Red Hat, Inc. QEMU Virtual Machine
        Physical Slot: 3
        Control: I/O+ Mem+ BusMaster+ SpecCycle- MemWINV- VGASnoop- ParErr- Stepping- SERR+ FastB2B- DisINTx-
        Status: Cap- 66MHz- UDF- FastB2B- ParErr- DEVSEL=fast >TAbort- <TAbort- <MAbort- >SERR- <PERR- INTx-
        Latency: 0
        Interrupt: pin A routed to IRQ 11
        Region 0: Memory at febc0000 (32-bit, non-prefetchable) [size=128K]
        Region 1: I/O ports at c000 [size=64]
        Expansion ROM at feb80000 [disabled] [size=256K]
        Kernel driver in use: e1000
        Kernel modules: e1000

        00: 86 80 0e 10 07 01 00 00 03 00 00 02 00 00 00 00
        10: 00 00 bc fe 01 c0 00 00 00 00 00 00 00 00 00 00
        20: 00 00 00 00 00 00 00 00 00 00 00 00 f4 1a 00 11
        30: 00 00 b8 fe 00 00 00 00 00 00 00 00 0b 01 00 00
 */
const uint8_t pci_data[] = {
    0x86, 0x80, 0x0e, 0x10, 0x07, 0x01, 0x00, 0x00, 0x03, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xbc, 0xfe, 0x01, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf4, 0x1a, 0x00, 0x11,
    0x00, 0x00, 0xb8, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0b, 0x01, 0x00, 0x00
};

uint16_t le_to_native16(const uint8_t *data) {
    return (uint16_t)data[0] | ((uint16_t)data[1] << 8);
}

uint32_t le_to_native32(const uint8_t *data) {
    return (uint32_t)data[0] | ((uint32_t)data[1] << 8) | ((uint32_t)data[2] << 16) | ((uint32_t)data[3] << 24);
}

void parse_command_register(uint16_t command) {
    printf("  [0x04] Command (2Byte): 0x%04X\n", command);
    printf("    IO Space Enable: %s\n", (command & 0x0001) ? "Enabled" : "Disabled");
    printf("    Memory Space Enable: %s\n", (command & 0x0002) ? "Enabled" : "Disabled");
    printf("    Bus Master Enable: %s\n", (command & 0x0004) ? "Enabled" : "Disabled");
    printf("    Special Cycle Enable: %s\n", (command & 0x0008) ? "Enabled" : "Disabled");
    printf("    Memory Write and Invalidate Enable: %s\n", (command & 0x0010) ? "Enabled" : "Disabled");
    printf("    VGA Palette Snoop Enable: %s\n", (command & 0x0020) ? "Enabled" : "Disabled");
    printf("    Parity Error Response Enable: %s\n", (command & 0x0040) ? "Enabled" : "Disabled");
    printf("    SERR# Enable: %s\n", (command & 0x0100) ? "Enabled" : "Disabled");
    printf("    Fast Back-to-Back Enable: %s\n", (command & 0x0200) ? "Enabled" : "Disabled");
    printf("    Interrupt Disable: %s\n", (command & 0x0400) ? "Disabled" : "Enabled");
}

int main() {
    PciConfigSpace *pci_config = (PciConfigSpace *)pci_data;

    printf("PCI Configuration Space Parsing:\n\n");

    printf("--- Header Section ---\n");
    printf("  [0x00] Vendor ID (2Byte): 0x%04X\n", le_to_native16((const uint8_t *)&pci_config->vendor_id));
    printf("  [0x02] Device ID (2Byte): 0x%04X\n", le_to_native16((const uint8_t *)&pci_config->device_id));

    parse_command_register(le_to_native16((const uint8_t *)&pci_config->command));

    printf("  [0x06] Status (2Byte): 0x%04X\n", le_to_native16((const uint8_t *)&pci_config->status));
    printf("  [0x08] Revision ID (1Byte): 0x%02X\n", pci_config->revision_id);
    printf("  [0x09] Prog IF (1Byte): 0x%02X\n", pci_config->prog_if);
    printf("  [0x0A] Sub Class (1Byte): 0x%02X\n", pci_config->sub_class);
    printf("  [0x0B] Base Class (1Byte): 0x%02X\n", pci_config->base_class);
    printf("  [0x0C] Cache Line Size (1Byte): 0x%02X\n", pci_config->cache_line_size);
    printf("  [0x0D] Latency Timer (1Byte): 0x%02X\n", pci_config->latency_timer);
    printf("  [0x0E] Header Type (1Byte): 0x%02X\n", pci_config->header_type);
    printf("  [0x0F] BIST (1Byte): 0x%02X\n", pci_config->bist);

    printf("\n");

    printf("--- Base Address Registers (BARs) ---\n");
    printf("  [0x10] BAR0 (4Byte): 0x%08X\n", le_to_native32((const uint8_t *)&pci_config->bar0));
    printf("  [0x14] BAR1 (4Byte): 0x%08X\n", le_to_native32((const uint8_t *)&pci_config->bar1));
    printf("  [0x18] BAR2 (4Byte): 0x%08X\n", le_to_native32((const uint8_t *)&pci_config->bar2));
    printf("  [0x1C] BAR3 (4Byte): 0x%08X\n", le_to_native32((const uint8_t *)&pci_config->bar3));
    printf("  [0x20] BAR4 (4Byte): 0x%08X\n", le_to_native32((const uint8_t *)&pci_config->bar4));
    printf("  [0x24] BAR5 (4Byte): 0x%08X\n", le_to_native32((const uint8_t *)&pci_config->bar5));

    printf("\n");

    printf("--- Other Fields ---\n");
    printf("  [0x28] Cardbus CIS Pointer (4Byte): 0x%08X\n", le_to_native32((const uint8_t *)&pci_config->cardbus_cis_pointer));
    printf("  [0x2C] Subsystem Vendor ID (2Byte): 0x%04X\n", le_to_native16((const uint8_t *)&pci_config->subsystem_vendor_id));
    printf("  [0x2E] Subsystem ID (2Byte): 0x%04X\n", le_to_native16((const uint8_t *)&pci_config->subsystem_id));
    printf("  [0x30] Expansion ROM Base Address (4Byte): 0x%08X\n", le_to_native32((const uint8_t *)&pci_config->expansion_rom_base_address));
    printf("  [0x34] Capabilities Pointer (1Byte): 0x%02X\n", pci_config->capabilities_pointer);
    printf("  [0x3C] Interrupt Line (1Byte): 0x%02X\n", pci_config->interrupt_line);
    printf("  [0x3D] Interrupt Pin (1Byte): 0x%02X\n", pci_config->interrupt_pin);
    printf("  [0x3E] Min Grant (1Byte): 0x%02X\n", pci_config->min_grant);
    printf("  [0x3F] Max Latency (1Byte): 0x%02X\n", pci_config->max_latency);

    return 0;
}
