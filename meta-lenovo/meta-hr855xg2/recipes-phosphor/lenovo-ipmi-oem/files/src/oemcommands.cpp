#include "../include/oemcommands.hpp"
#include "xyz/openbmc_project/Common/error.hpp"
#include <array>
#include <cstring>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <ipmid/utils.hpp>
#include <phosphor-logging/log.hpp>
#include <string>
#include <vector>


namespace ipmi
{
    static void registerOEMFunctions() __attribute__((constructor));
    sdbusplus::bus::bus dbus(ipmid_get_sd_bus_connection()); // from ipmid/api.h

ipmi_ret_t ipmiOemAddUefiSel(ipmi_netfn_t netfn, ipmi_cmd_t cmd,
                                   ipmi_request_t request,
                                   ipmi_response_t response,
                                   ipmi_data_len_t data_len,
                                   ipmi_context_t context)
{
    /* Do nothing, return tset data */
      ipmi_ret_t rc = IPMI_CC_OK;
      uint8_t rsp[] = {0xFF, 0x00, 0xAA, 0x55};
      memcpy(response, &rsp, 4);
      *data_len = 4;

      return rc;      
}

static void registerOEMFunctions(void)
{
    phosphor::logging::log<phosphor::logging::level::INFO>(
        "Registering Lenovo OEM commands");
        
    ipmiPrintAndRegister(NETFUN_OEM, CMD_OEM_ADD_UEFI_SEL, NULL,
                         ipmiOemAddUefiSel,
                         PRIVILEGE_USER); 
}


} // namespace ipmi


