#pragma once

#include <ipmid/api.h>
#include <cstdint>
#include <ipmid/oemrouter.hpp>
#include <iostream>
#include <sdbusplus/bus.hpp>

using std::uint8_t;
static constexpr bool debug = true;
static constexpr uint8_t block = 0x02;
#define FAST_SYNC_MAX_REGISTER 15
#define FAST_SYNC_RONLY_MIN 4
#define FAST_SYNC_RONLY_MAX 7


enum lenovo_oem_cmds
{
    CMD_OEM_ADD_UEFI_SEL = 0x01,
	CMD_OEM_READ_FPGA = 0x02,
	CMD_OEM_WRITE_FPGA = 0x03

};

struct fpgadata
{
	uint8_t data;
} __attribute__((packed));

struct readFPGA
{
	uint8_t offset;
} __attribute__((packed));

struct writeFPGA
{
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
