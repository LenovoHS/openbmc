/***************************************************************** 
"Copyright ? 2019-present Lenovo
Licensed under BSD-3, see COPYING.BSD file for details."
*****************************************************************/

#ifndef _FPGA_H
#define _FPGA_H

#include <stdint.h>
#include <stddef.h>


#define FPGA_MAX_DATA_XFER_SIZE       1024
#define FPGA_ACCESS_LOCAL                     0x00
#define FPGA_BLOCK_MAIN 0x00
#define FPGA_VERSION_SIZE  10
#define FPGA_MAX_CMD_SIZE              1036
#define FPGA_MAX_BLOCKS                64
#define FPGA_BLOCK_SIZE                16
#define FPGA_STATE_INIT_IN_PROGRESS    0x01
#define FPGA_INTEGRITY_CRC7            1

#define AST2500                        1
#define PLATFORM_SUPPORT               AST2500          


/* V2 block numbers */
/* Dont use these, use the generic defines below and hal_fpga_map_block() to map them based on fpga interface */
#define FPGA_BLOCK_MAIN_V2                  0
#define FPGA_BLOCK_MAIN2_V2                 1
#define FPGA_BLOCK_FLASH_V2                 2
#define FPGA_BLOCK_FLASH2_V2                3
#define FPGA_BLOCK_POWER_V2                 4
#define FPGA_BLOCK_POWER2_V2                5
#define FPGA_BLOCK_INTERRUPTS_V2            6
#define FPGA_BLOCK_INTERRUPTS2_V2           7
#define FPGA_BLOCK_THERMAL_V2               8
#define FPGA_BLOCK_THERMAL2_V2              9
#define FPGA_BLOCK_SPD_V2                  10
#define FPGA_BLOCK_PLATFORMCTL_V2          12
#define FPGA_BLOCK_PLATFORMCTL2_V2         13
#define FPGA_BLOCK_RAS_V2                  14
#define FPGA_BLOCK_RAS2_V2                 15
#define FPGA_BLOCK_MEMORY_V2               16
#define FPGA_BLOCK_FAMILY_V2               18
#define FPGA_BLOCK_FAMILY2_V2              19
#define FPGA_BLOCK_REMOTE_V2               22
#define FPGA_BLOCK_PLATFORM_V2             24
#define FPGA_BLOCK_PLATFORM2_V2            25
#define FPGA_BLOCK_SCALABILITY_V2          26 /* not supported in driver */
#define FPGA_BLOCK_ONBOARD_DEV_V2          28
#define FPGA_BLOCK_PCIE_HOTPLUG_V2         30
#define FPGA_BLOCK_MAILBOX_V2              34
#define FPGA_BLOCK_I2C_V2                  36
#define FPGA_BLOCK_LCD_V2                  38
#define FPGA_BLOCK_PM_V2                   40
#define FPGA_BLOCK_UEFI_SCRATCH_V2         42
#define FPGA_BLOCK_SERVICE_V2              46
#define FPGA_BLOCK_DEBUG0_V2               60
#define FPGA_BLOCK_DEBUG1_V2               61
#define FPGA_BLOCK_DEBUG2_V2               62
#define FPGA_BLOCK_DEBUG3_V2               63
#define FPGA_ILLEGAL_BLOCK_V2             255
#define FPGA_BLOCK_LAST                    63


#define FPGA_INTEGRITY_CHECKSUM      0
#define FPGA_INTEGRITY_CRC7          1
#define FPGA_READ_MODE               0
#define FPGA_WRITE_MODE              1

#define FPGA_MODE_READ                       1
#define FPGA_MODE_WRITE                      2
#define FPGA_MODE_READ_FLASH                 3

#define FPGA_CLASS_SYSTEM                    0

#define FPGA_SYS_REG_DUMP       0x0

#define FPGA_DRV                "/dev/spidev1.0"

typedef struct {
    uint8_t chip_id;
    uint8_t major;
    uint8_t minor;
    uint8_t test_image_id;
    uint8_t build_month;
    uint8_t build_day;
    uint8_t build_year;
    uint8_t recovery_image; // 0=running good code, 1=running recovery code
} fpga_version_t;

typedef struct
{
    uint8_t port;
    uint8_t pin;
} gpio_t;

typedef struct
{
    gpio_t    wdtovf;
    gpio_t    prog_b;
    gpio_t    done;
    gpio_t    irq;
    uint8_t   spi_bus_no;
    uint8_t   spi_cs;
    uint8_t   bmc_addr;
} fpga_cfg_t;

/* context attribute has different meaning depending on the action */
/* action = FPGA_ACCESS_LOCAL  -> context is not used         */
/* action = FPGA_ACCESS_REMOTE -> context = remote fpga id    */
/* action = FPGA_ACCESS_SCALE  -> context = scale fpga id     */
typedef struct {
    uint8_t  action;
    uint8_t  adi;
	uint8_t  block;
	uint16_t length;
	uint16_t offset;
    uint8_t  context;
    uint8_t  bit;
    uint16_t page;
	uint8_t  data[ FPGA_MAX_DATA_XFER_SIZE ];
}fpga_cmd_t;

typedef struct {
    uint8_t  rmt;
    uint8_t  node_id;
    uint8_t  fpga_integrity_value;
    uint8_t  mode;           // 0=read, 1=write
    uint8_t  data[ FPGA_MAX_DATA_XFER_SIZE ];
    uint8_t  status;
    uint8_t  integrity_value;
    uint16_t data_count;
    uint16_t addr;
    uint8_t  adi;
    uint8_t  page_size;
    uint16_t page;
    uint8_t  rmt_status;
}fpga_msg_t;            

typedef struct {
    fpga_cmd_t   cmd;
    uint8_t           i2c_err;
    uint8_t           hw_recover_mode;
    uint8_t           lib_debug_level;
}fpga_trans_t;

typedef struct _fpga_caps {
    uint8_t block;
    uint8_t always : 1;
    uint8_t offset : 4;
    uint8_t bit    : 3;
    const char *name;
}fpga_caps_t;

typedef struct {
    uint8_t  block_num;
    uint8_t  data[ FPGA_BLOCK_SIZE ];
}block_t;

typedef struct {
    uint8_t            present;
    block_t       block0;
    fpga_caps_t  *caps;
    uint16_t          *block_mapper;
    uint16_t           block_addresses[ FPGA_MAX_BLOCKS ];
    uint8_t            id;
    uint8_t            enforce_integrity;
    uint8_t            integrity_type;
    uint8_t            state;
}common_fpga_t;

typedef struct {
    // all FPGAs have this
    common_fpga_t common;

    // large structure for buffers, etc..
    fpga_msg_t       msg;
    fpga_cmd_t  cmd;

    // only local FPGAs have the following
    uint8_t           spi_bus;
    uint8_t           spi_cs;
    uint8_t           frequency;
    uint8_t           protocol_version;

    // for flashing
    uint16_t          page_size;
    uint16_t          pages_per_sector;
    uint32_t          sector_size;
}local_fpga_t;

typedef struct
{
    uint8_t          *in_buf;
    uint8_t          *out_buf;
    local_fpga_t     local_fpga;
}fpga_glob;

typedef struct
{
	uint32_t u32SPIAddr;         //SPI Address (NEW)
	uint32_t tx_len; // Msg transmit data length
	uint32_t rx_len;  // Msg receive data length 
	uint8_t *tx_buff;   // Msg transmit buffer
	uint8_t *rx_buff;    // Msg receive buffer
    uint8_t spi_cmd;            // SPI Command (opcode) (NEW)
	uint8_t spi_mode;           // SPI bus mode
	uint8_t cs;        // SPI chip select
	uint8_t freq;      // SPI clock speed
	uint8_t num;            // SPI bus number
	uint8_t add_mode;          // SPI Address Mode (0=3Byte, 1=4Byte) (NEW)
    uint8_t spi_mem;      // SPI Memory Access (1= Read/Write/Erase 0=No Access) (NEW)
    uint8_t action;  // SPI Chip Select Action (0 - perform SPI operation and deassert. 1 - leave asserted. (NEW)
    uint8_t dummy;        // Number of Dummy bytes for the command (NEW)
}spiInfo;

const fpga_caps_t fpga_caps_v2[] = {
    // block, always, offset, bit
    {  0, 1, 0x00, 0,         "Main" }, // always supported
    {  1, 1, 0x00, 0,        "Main2" }, // always supported
    {  2, 1, 0x00, 0,        "Flash" }, // always supported
    {  3, 0, 0x0E, 3,       "Flash2" },
    {  4, PLATFORM_SUPPORT, 0x0E, 4,        "Power" },
    {  5, 0, 0x0E, 5,       "Power2" },
    {  6, PLATFORM_SUPPORT, 0x0E, 6,   "Interrupts" },
    {  7, 0, 0x0E, 7,  "Interrupts2" },

    {  8, PLATFORM_SUPPORT, 0x0D, 0,      "Thermal" },
    {  9, 0, 0x0D, 1,     "Thermal2" },
    { 10, PLATFORM_SUPPORT, 0x0D, 2,          "SPD" },
    { 11, 0, 0x0D, 3,  "Reserved-11" }, // Reserved
    { 12, PLATFORM_SUPPORT, 0x0D, 4,  "PlatformCtl" },
    { 13, PLATFORM_SUPPORT, 0x0D, 5, "PlatformCtl2" },
    { 14, PLATFORM_SUPPORT, 0x0D, 6,          "RAS" },
    { 15, 0, 0x0D, 7,         "RAS2" },

    { 16, PLATFORM_SUPPORT, 0x0C, 0,       "Memory" },
    { 17, 0, 0x0C, 1,  "Reserved-17" }, // Reserved
    { 18, 0, 0x0C, 2,       "Family" },
    { 19, 0, 0x0C, 3,      "Family2" },
    { 20, 0, 0x0C, 4,  "Reserved-20" }, // Reserved
    { 21, 0, 0x0C, 5,  "Reserved-21" }, // Reserved
    { 22, 0, 0x0C, 6,       "Remote" },
    { 23, 0, 0x0C, 7,  "Reserved-23" }, // Reserved

    { 24, 0, 0x0B, 0,     "Platform" },
    { 25, 0, 0x0B, 1,    "Platform2" },
    { 26, 0, 0x0B, 2,  "Scalability" },
    { 27, 0, 0x0B, 3,  "Reserved-27" }, // Reserved
    { 28, PLATFORM_SUPPORT, 0x0B, 4,  "Onboard-dev" },
    { 29, 0, 0x0B, 5,  "Reserved-29" }, // Reserved
    { 30, 0, 0x0B, 6, "PCIe Hotplug" },
    { 31, 0, 0x0B, 7,  "Reserved-31" }, // Reserved

    { 32, 0, 0x0A, 0,  "Reserved-32" }, // Reserved
    { 33, 0, 0x0A, 1,  "Reserved-33" }, // Reserved
    { 34, 0, 0x0A, 2,      "Mailbox" },
    { 35, 0, 0x0A, 3,  "Reserved-35" }, // Reserved
    { 36, 0, 0x0A, 4,          "I2C" },
    { 37, 0, 0x0A, 5,  "Reserved-37" }, // Reserved
    { 38, 0, 0x0A, 6,          "LCD" },
    { 39, 0, 0x0A, 7,  "Reserved-39" }, // Reserved

    { 40, PLATFORM_SUPPORT, 0x09, 0,           "PM" },
    { 41, 0, 0x09, 1,  "Reserved-41" }, // Reserved
    { 42, 0, 0x09, 2, "UEFI Scratch" },
    { 43, 0, 0x09, 3,  "Reserved-43" }, // Reserved
    { 44, 0, 0x09, 4,      "Service" },
    { 45, 0, 0x09, 5,  "Reserved-45" }, // Reserved
    { 46, PLATFORM_SUPPORT, 0x09, 6,  "Reserved-46" }, // Reserved
    { 47, 0, 0x09, 7,  "Reserved-47" }, // Reserved

    { 48, 0, 0x08, 0,  "Reserved-48" }, // Reserved
    { 49, 0, 0x08, 1,  "Reserved-49" }, // Reserved
    { 50, 0, 0x08, 2,  "Reserved-50" }, // Reserved
    { 51, 0, 0x08, 3,  "Reserved-51" }, // Reserved
    { 52, 0, 0x08, 4,  "Reserved-52" }, // Reserved
    { 53, 0, 0x08, 5,  "Reserved-53" }, // Reserved
    { 54, 0, 0x08, 6,  "Reserved-54" }, // Reserved
    { 55, 0, 0x08, 7,  "Reserved-55" }, // Reserved

    { 56, 0, 0x07, 0,  "Reserved-56" }, // Reserved
    { 57, 0, 0x07, 1,  "Reserved-57" }, // Reserved
    { 58, 0, 0x07, 2,  "Reserved-58" }, // Reserved
    { 59, 0, 0x07, 3,  "Reserved-59" }, // Reserved

    { 60, 1, 0x00, 0,       "DEBUG0" }, // always supported
    { 61, 1, 0x00, 0,       "DEBUG1" }, // always supported
    { 62, 1, 0x00, 0,       "DEBUG2" }, // always supported
    { 63, 1, 0x00, 0,       "DEBUG3" }, // always supported

    /* last entry */
    { 0xFF, 0, 0, 0, NULL }
};

/* index is generic block number, value is corresponding v2 block number */
const uint16_t fpga_block_mapper_v2[] = {
    FPGA_BLOCK_MAIN_V2,          // FPGA_BLOCK_MAIN
    FPGA_BLOCK_MAIN2_V2,         // FPGA_BLOCK_MAIN2
    FPGA_BLOCK_FLASH_V2,         // FPGA_BLOCK_FLASH
    FPGA_BLOCK_FLASH2_V2,        // FPGA_BLOCK_FLASH2
    FPGA_BLOCK_POWER_V2,         // FPGA_BLOCK_POWER
    FPGA_BLOCK_POWER2_V2,        // FPGA_BLOCK_POWER2
    FPGA_BLOCK_INTERRUPTS_V2,    // FPGA_BLOCK_INTERRUPTS
    FPGA_BLOCK_INTERRUPTS2_V2,   // FPGA_BLOCK_INTERRUPTS2
    FPGA_BLOCK_THERMAL_V2,       // FPGA_BLOCK_THERMAL
    FPGA_BLOCK_THERMAL2_V2,      // FPGA_BLOCK_TSOD
    FPGA_BLOCK_SPD_V2,           // FPGA_BLOCK_SPD
    FPGA_BLOCK_PLATFORM_V2,      // FPGA_BLOCK_PLATFORM
    FPGA_BLOCK_PLATFORMCTL_V2,   // FPGA_BLOCK_PLATFORMCTL
    FPGA_BLOCK_PLATFORMCTL2_V2,  // FPGA_BLOCK_PLATFORMCTL2
    FPGA_BLOCK_RAS_V2,           // FPGA_BLOCK_RAS
    FPGA_BLOCK_RAS2_V2,          // FPGA_BLOCK_RAS2
    FPGA_BLOCK_MEMORY_V2,        // FPGA_BLOCK_MEMORY
    FPGA_BLOCK_FAMILY_V2,        // FPGA_BLOCK_FAMILY
    FPGA_BLOCK_FAMILY2_V2,       // FPGA_BLOCK_FAMILY2
    FPGA_BLOCK_REMOTE_V2,        // FPGA_BLOCK_REMOTE
    FPGA_BLOCK_PLATFORM2_V2,     // FPGA_BLOCK_PLATFORM2
    FPGA_BLOCK_SCALABILITY_V2,   // FPGA_BLOCK_SCALABILITY
    FPGA_BLOCK_ONBOARD_DEV_V2,   // FPGA_BLOCK_ONBOARD_DEV
    FPGA_BLOCK_PCIE_HOTPLUG_V2,  // FPGA_BLOCK_PCIE_HOTPLUG
    FPGA_BLOCK_MAILBOX_V2,       // FPGA_BLOCK_MAILBOX
    FPGA_ILLEGAL_BLOCK_V2,       // FPGA_BLOCK_DEBUG
    FPGA_BLOCK_I2C_V2,           // FPGA_BLOCK_I2C
    FPGA_BLOCK_LCD_V2,           // FPGA_BLOCK_LCD
    FPGA_BLOCK_PM_V2,            // FPGA_BLOCK_PM
    FPGA_BLOCK_UEFI_SCRATCH_V2,  // FPGA_BLOCK_UEFI_SCRATCH
    FPGA_BLOCK_SERVICE_V2,       // FPGA_BLOCK_SERVICE
    FPGA_BLOCK_DEBUG0_V2,        // FPGA_BLOCK_DEBUG0
    FPGA_BLOCK_DEBUG1_V2,        // FPGA_BLOCK_DEBUG1
    FPGA_BLOCK_DEBUG2_V2,        // FPGA_BLOCK_DEBUG2
    FPGA_BLOCK_DEBUG3_V2,        // FPGA_BLOCK_DEBUG3
};


/* index is generic block number, value is corresponding v2 block number */
const uint8_t _FPGA_block_lut_v2[FPGA_BLOCK_LAST+1] = {
    FPGA_BLOCK_MAIN_V2,          // FPGA_BLOCK_MAIN
    FPGA_BLOCK_MAIN2_V2,         // FPGA_BLOCK_MAIN2
    FPGA_BLOCK_FLASH_V2,         // FPGA_BLOCK_FLASH
    FPGA_BLOCK_FLASH2_V2,        // FPGA_BLOCK_FLASH2
    FPGA_BLOCK_POWER_V2,         // FPGA_BLOCK_POWER
    FPGA_BLOCK_POWER2_V2,        // FPGA_BLOCK_POWER2
    FPGA_BLOCK_INTERRUPTS_V2,    // FPGA_BLOCK_INTERRUPTS
    FPGA_BLOCK_INTERRUPTS2_V2,   // FPGA_BLOCK_INTERRUPTS2
    FPGA_BLOCK_THERMAL_V2,       // FPGA_BLOCK_THERMAL
    FPGA_BLOCK_THERMAL2_V2,      // FPGA_BLOCK_TSOD
    FPGA_BLOCK_SPD_V2,           // FPGA_BLOCK_SPD
    FPGA_BLOCK_PLATFORM_V2,      // FPGA_BLOCK_PLATFORM
    FPGA_BLOCK_PLATFORMCTL_V2,   // FPGA_BLOCK_PLATFORMCTL
    FPGA_BLOCK_PLATFORMCTL2_V2,  // FPGA_BLOCK_PLATFORMCTL2
    FPGA_BLOCK_RAS_V2,           // FPGA_BLOCK_RAS
    FPGA_BLOCK_RAS2_V2,          // FPGA_BLOCK_RAS2
    FPGA_BLOCK_MEMORY_V2,        // FPGA_BLOCK_MEMORY
    FPGA_BLOCK_FAMILY_V2,        // FPGA_BLOCK_FAMILY
    FPGA_BLOCK_FAMILY2_V2,       // FPGA_BLOCK_FAMILY2
    FPGA_BLOCK_REMOTE_V2,        // FPGA_BLOCK_REMOTE
    FPGA_BLOCK_PLATFORM2_V2,     // FPGA_BLOCK_PLATFORM2
    FPGA_BLOCK_SCALABILITY_V2,   // FPGA_BLOCK_SCALABILITY
    FPGA_BLOCK_ONBOARD_DEV_V2,   // FPGA_BLOCK_ONBOARD_DEV
    FPGA_BLOCK_PCIE_HOTPLUG_V2,  // FPGA_BLOCK_PCIE_HOTPLUG
    FPGA_BLOCK_MAILBOX_V2,       // FPGA_BLOCK_MAILBOX
    FPGA_ILLEGAL_BLOCK_V2,       // FPGA_BLOCK_DEBUG
    FPGA_BLOCK_I2C_V2,           // FPGA_BLOCK_I2C
    FPGA_BLOCK_LCD_V2,           // FPGA_BLOCK_LCD
    FPGA_BLOCK_PM_V2,            // FPGA_BLOCK_PM
    FPGA_BLOCK_UEFI_SCRATCH_V2,  // FPGA_BLOCK_UEFI_SCRATCH
    FPGA_BLOCK_SERVICE_V2,       // FPGA_BLOCK_SERVICE
    FPGA_BLOCK_DEBUG0_V2,
    FPGA_BLOCK_DEBUG1_V2,
    FPGA_BLOCK_DEBUG2_V2,
    FPGA_BLOCK_DEBUG3_V2
};


typedef struct
{
    //
    //  -- General commands:
    //

    uint8_t  type;                   // -c
    uint32_t platform_id;            // -s
    uint32_t platform_rev;           // NONE
    uint8_t  version;                // -v
    uint8_t  rom_version;            // -g
    uint8_t  version_format;         // -V <N>  the N, else 0
    uint8_t  mode;                   // -r
    uint8_t  block;                  // -b
    uint32_t offset;                 // -o
    uint32_t length;                 // -l
    uint8_t  adi;                    // -a
    uint16_t page;                   // -p
    uint8_t  use_IPMI;               // -I
    uint8_t  busID;                  // -I
    uint8_t  re_init_driver;         // -X
    uint8_t  remote_access;          // -R, -S
    uint8_t  remote_id;              // -R, -S
    uint8_t  i2c;                    // -C
    uint8_t  i2c_bus_id;             // -C
    uint8_t  retry_count;            // -q <N>

    //
    //  -- Watchdog commands
    //
    uint8_t  watchdog;               // -W M:N
    uint8_t  watchdog_query;         // -W M:N
    uint8_t  watchdog_mode;          // -W M:N
    uint8_t  watchdog_timeout;       // -W M:N

    //
    //  -- Firmware update commands:
    //

    char     *flash_file;            // -f
    char     *flash_mafs_file;       // -u
    uint8_t  flash_optimization;     // -O #
    uint8_t  parallel_flash;         // -P
    uint8_t  flash_irq;              // -G
    uint8_t  force_mafs_flash;       // -u
    uint32_t mafs_sector_size;       // -Z
    uint8_t  mafs_block_mode;        // -Y
    uint8_t  force_flash;            // -f
    uint8_t  test_flash_policy;      // -U
    uint8_t  mafs_debug_mode;        // -B
    uint8_t  defeat_phase2_latch;    // -L
    uint8_t  reboot_fpga;            // -x

    //
    //  -- Development commands:
    //

    uint8_t  test_mode;               // -t
    uint32_t test_loops;              // -Q #
    uint8_t  irq_listen;              // -i

    //
    //  -- Debugging commands:
    //

    uint32_t fpga_debug_level;       // -d
    uint8_t  dump;                   // -m
    uint8_t  dump_type;              //
    uint8_t  display_trace_data;     // -T
    char     *interface_trace_file;  // -T
    char     *debug_file;            // -D
    uint32_t hal_debug_level;        // -H
    char     *register_dump_file;    // -M
    uint8_t  wr_data[ 64 ];

}cmd_args_t;


struct fpga_global_data {
    //FILE*           fpga_debug_fp;
    //FILE*           fpga_interface_trace_fp;
    fpga_cfg_t      cfg;
    cmd_args_t      args;
    fpga_trans_t    trans;
};
//#ifdef __cplusplus
//extern "C" {
//#endif
	int open_dev(void);
	int fpga_read(fpga_cmd_t *cmd);

	int altera_fpga_init(void);

	uint16_t fpga_block_to_addr( common_fpga_t *info, uint8_t block_id );
	int fpga_transfer(spiInfo *info);
	uint8_t fpga_calc_integrity_value( uint8_t *buf, uint16_t len, uint8_t type );
	int fpga_message_read(fpga_msg_t *msg);
	int fpga_message_write(fpga_msg_t *msg);
	int fpga_write(fpga_cmd_t *cmd);
	int fpga_read_block(fpga_cmd_t *cmd );
	
//#ifdef __cplusplus
//}
//#endif

#endif // _FPGA_H