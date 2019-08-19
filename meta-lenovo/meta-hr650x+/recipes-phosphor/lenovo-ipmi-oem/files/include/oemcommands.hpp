#pragma once

#include <ipmid/api.h>
#include <cstdint>
#include <ipmid/oemrouter.hpp>
#include <iostream>
#include <sdbusplus/bus.hpp>

using std::uint8_t;
static constexpr bool debug = true;

#define I2C_BUS_FPGA "/dev/i2c-1"
#define I2C_ADDR_FPGA 0x44
#define I2C_BUS_BP "/dev/i2c-15"
#define I2C_ADDR_BP 0x60

#define REQ_LEN_READ_FPGA 1
#define REQ_LEN_WRITE_FPGA 2
#define REQ_LEN_READ_BP 1

enum lenovo_oem_cmds
{
    CMD_OEM_ADD_UEFI_SEL = 0x00,
    CMD_OEM_READ_FPGA = 0x01,
    CMD_OEM_WRITE_FPGA = 0x02,
    CMD_OEM_SET_BIOS_LOAD_DEFAULT_STATUS = 0x03,
    CMD_OEM_GET_BIOS_LOAD_DEFAULT_STATUS = 0x04,
    CMD_OEM_READ_BP = 0x0B
};

#define JSON_OEM_DATA_FILE "/etc/oemData.json"
#define BIOS_LOAD_Default_Flag "BIOS_Load_Default_Flag"

#define BIOS_POST_END   0x01
#define BIOS_POST_NOT_FINISH 0x00

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
