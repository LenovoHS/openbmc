#pragma once

#include <ipmid/api.h>
#include <cstdint>
#include <ipmid/oemrouter.hpp>
#include <iostream>
#include <sdbusplus/bus.hpp>

using std::uint8_t;
static constexpr bool debug = true;
//static constexpr uint8_t block = 0x02;
#define FAST_SYNC_MAX_REGISTER 15
#define FAST_SYNC_RONLY_MIN 4
#define FAST_SYNC_RONLY_MAX 7

enum lenovo_oem_cmds
{
    CMD_OEM_ADD_UEFI_SEL = 0x00,
	CMD_OEM_READ_FPGA = 0x01,
	CMD_OEM_WRITE_FPGA = 0x02,
	CMD_OEM_SET_BIOS_LOAD_DEFAULT_STATUS = 0x03,
	CMD_OEM_GET_BIOS_LOAD_DEFAULT_STATUS = 0x04,
	CMD_OEM_PDB_POWER_CYCLE   = 0x05,
	CMD_OEM_Control_LED_Status	= 0x06,
	CMD_OEM_SET_BIOS_CURRENT_PID = 0x07,
	CMD_OEM_GET_BIOS_CURRENT_PID = 0x08,
	CMD_OEM_SET_BIOS_WANTED_PID = 0x09,
	CMD_OEM_GET_BIOS_WANTED_PID = 0x0A,
};
#define JSON_OEM_DATA_FILE "/etc/oemData.json"
#define BIOS_LOAD_Default_Flag "BIOS_Load_Default_Flag"
#define BIOS_Profile_Current_Configuration "BIOS_Current_PID"
#define BIOS_Profile_Wanted_Configuration   "BIOS_Wanted_PID"

#define BIOS_POST_END	0x01
#define BIOS_POST_NOT_FINISH 0x00

#define PDB_RESTART_N     			202 	// GPIO_Z2
#define PWRGD_SYS_PWROK_BMC 		63 		//GPIO_H7
#define FM_PCH_PWRBTN_N 			65		//GPIO_I1

#define GPIO_VALUE_H 				1
#define GPIO_VALUE_L 				0
#define MAX_STR_LEN 256
#define GPIOBASE 280
#define SYSFS_GPIO_PATH "/sys/class/gpio"
#define S0 1
#define S5 0

#define SYSFS_LED_PATH	"/sys/class/leds" 

struct fpgadata
{
	uint8_t data;
} __attribute__((packed));

struct readFPGA
{
	uint8_t block;
	uint8_t offset;
} __attribute__((packed));

struct writeFPGA
{
	uint8_t block;
	uint8_t offset;
	uint8_t data;
} __attribute__((packed));

inline static void printRegistration(unsigned int netfn, unsigned int cmd)
{
    if constexpr (debug)
    {
        std::fprintf(stderr, "Registering Lenovo NetFn:[%#04X], Cmd:[%#04X] \n",
             netfn,cmd);                      
    }
}

inline static void ipmiPrintAndRegister(ipmi_netfn_t netfn, ipmi_cmd_t cmd,
                                        ipmi_context_t context,
                                        ipmid_callback_t handler,
                                        ipmi_cmd_privilege_t priv)
{
    printRegistration(netfn, cmd);
    ipmi_register_callback(netfn, cmd, context, handler, priv);
}
