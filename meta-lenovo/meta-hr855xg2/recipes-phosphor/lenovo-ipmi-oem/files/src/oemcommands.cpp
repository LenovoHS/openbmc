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
	using phosphor::logging::level;
	using phosphor::logging::log;
	using phosphor::logging::entry;
	
	static constexpr auto fpgaService = "org.openbmc.control.fpga";
    static constexpr auto fpgaRoot = "/org/openbmc/control/fpga";
    static constexpr auto fpgaInterface = "org.openbmc.control.fpga";
	//auto bus = sdbusplus::bus::new_default();
	
	static void registerOEMFunctions() __attribute__((constructor));
    sdbusplus::bus::bus bus(ipmid_get_sd_bus_connection()); // from ipmid/api.h
   
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
ipmi_ret_t ipmioemReadFPGA(ipmi_netfn_t netfn, ipmi_cmd_t cmd,
                                   ipmi_request_t request,
                                   ipmi_response_t response,
                                   ipmi_data_len_t data_len,
                                   ipmi_context_t context)
{
	ipmi_ret_t rc = IPMI_CC_OK;
	uint8_t res_data;
	size_t respLen = 0;
	fpgadata resp = {0};
	auto* req = reinterpret_cast<readFPGA*>(request);
	if(*data_len != sizeof(readFPGA)){
		*data_len = 0;
		return IPMI_CC_REQ_DATA_LEN_INVALID;
		
	}
	if(req->offset >FAST_SYNC_MAX_REGISTER){
		return IPMI_CC_INVALID_FIELD_REQUEST;
	}
	auto method = bus.new_method_call(fpgaService, fpgaRoot,
                                      fpgaInterface, "read_fpga");
	
	method.append(block,req->offset);
	
    try
	{
        auto pid = bus.call(method);
		if (pid.is_method_error())
        {
            phosphor::logging::log<level::ERR>("Error in method call");
			return IPMI_CC_UNSPECIFIED_ERROR;
        }
		pid.read(res_data);
		resp.data = res_data;
		respLen = sizeof(resp);
	    std::memcpy(response, &resp, sizeof(resp));
		*data_len = respLen;
		return rc;
	}
	catch (const sdbusplus::exception::SdBusError& e)
    {
        phosphor::logging::log<level::ERR>("fpga dbus error");
		return IPMI_CC_INVALID_FIELD_REQUEST;
    }
	return rc;
}

ipmi_ret_t ipmioemWriteFPGA(ipmi_netfn_t netfn, ipmi_cmd_t cmd,
                                   ipmi_request_t request,
                                   ipmi_response_t response,
                                   ipmi_data_len_t data_len,
                                   ipmi_context_t context)
{
	
	ipmi_ret_t rc = IPMI_CC_OK;
	auto* req = reinterpret_cast<writeFPGA*>(request);
	if(*data_len != sizeof(writeFPGA)){
		*data_len = 0;
		return IPMI_CC_REQ_DATA_LEN_INVALID;
		
	}
	if(req->offset >FAST_SYNC_MAX_REGISTER){
		return IPMI_CC_INVALID_FIELD_REQUEST;
	}else if((req->offset >=FAST_SYNC_RONLY_MIN) && (req->offset <=FAST_SYNC_RONLY_MAX)){
		return IPMI_CC_INVALID_FIELD_REQUEST;
	}
		
	auto method = bus.new_method_call(fpgaService, fpgaRoot,
                                      fpgaInterface, "write_fpga");
	method.append(block,req->offset,req->data);
	try
	{
		auto fast_sync = bus.call(method);
        if (fast_sync.is_method_error())
        {
            phosphor::logging::log<level::INFO>("write fpga command error");
			return IPMI_CC_UNSPECIFIED_ERROR;
		}
        
	}
	catch (const sdbusplus::exception::SdBusError& e)
    {
        phosphor::logging::log<level::INFO>("fpga dbus error");
		return IPMI_CC_INVALID_FIELD_REQUEST;
    }
	
	return rc;
}

static void registerOEMFunctions(void)
{
    phosphor::logging::log<phosphor::logging::level::INFO>(
        "Registering Lenovo OEM commands");
        
    ipmiPrintAndRegister(NETFUN_OEM, CMD_OEM_ADD_UEFI_SEL, NULL,
                         ipmiOemAddUefiSel,
                         PRIVILEGE_USER); 
#if 1
	ipmiPrintAndRegister(NETFUN_OEM, CMD_OEM_READ_FPGA, NULL,
                         ipmioemReadFPGA,
                         PRIVILEGE_ADMIN);
	ipmiPrintAndRegister(NETFUN_OEM, CMD_OEM_WRITE_FPGA, NULL,
                         ipmioemWriteFPGA,
                         PRIVILEGE_ADMIN);
#endif
}


} // namespace ipmi


