#pragma once

#include <ipmid/api.h>
#include <cstdint>
#include <ipmid/oemrouter.hpp>
#include <iostream>
#include <sdbusplus/bus.hpp>

using std::uint8_t;
static constexpr bool debug = true;


enum lenovo_oem_cmds
{
    CMD_OEM_ADD_UEFI_SEL = 0x01

};

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
