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
#include <nlohmann/json.hpp>
#include <iostream>
#include <stdio.h>


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
   
nlohmann::json oemData;
	
void flushOemData()
{
    std::ofstream file(JSON_OEM_DATA_FILE);
    file << oemData;
    return;
}

std::string bytesToStr(uint8_t *byte, int len)
{
    std::stringstream ss;
    uint8_t i;

    ss << std::hex;
    for (i = 0; i < len; i++)
    {
        ss << std::setw(2) << std::setfill('0') << (int)byte[i];
    }

    return ss.str();
}

int strToBytes(std::string &str, uint8_t *data)
{
    std::string sstr;
    uint8_t i;
	uint8_t len = str.length();
	 
    for (i = 0; i < len; i++)
    {
        sstr = str.substr(i * 2, 2);
        data[i] = (uint8_t)std::strtol(sstr.c_str(), NULL, 16);
		
    }
	
    return i;
}
   
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
	auto method = bus.new_method_call(fpgaService, fpgaRoot,
                                      fpgaInterface, "read_fpga");
	
	method.append(req->block,req->offset);
	
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
		
	auto method = bus.new_method_call(fpgaService, fpgaRoot,
                                      fpgaInterface, "write_fpga");
	method.append(req->block,req->offset,req->data);
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
	*data_len = 1;
	return rc;
}

int ExportGpio(int gpionum)
{
	FILE *fd;
	uint8_t len;
    char buf[MAX_STR_LEN];

    memset(buf, 0, sizeof(buf));

    fd = fopen(SYSFS_GPIO_PATH "/export", "w");
    if (fd < 0) {
        printf("Open %s/export failed\n", SYSFS_GPIO_PATH);
        return -1;
    }

    len = snprintf(buf, sizeof(buf), "%d", (GPIOBASE + gpionum));
    fwrite(buf,sizeof(char),len,fd);

    fclose(fd);

    return 0;
}	

int setGPIOValue (int gpionum, int value) {
    FILE *fd;
    char file_name[MAX_STR_LEN];

    memset(file_name, 0, sizeof(file_name));
    snprintf(file_name, sizeof(file_name), SYSFS_GPIO_PATH "/gpio%d/value", (GPIOBASE + gpionum));

    fd = fopen(file_name, "w");
    if (fd < 0) {
        printf("Open %s/gpio%d/value failed\n", SYSFS_GPIO_PATH, (GPIOBASE + gpionum));
        return -1;
    }

	fwrite(&value,sizeof(int),1,fd);

    fclose(fd);

    return 0;
}

int getGPIOValue (int gpionum) {
    FILE *fd;
    char file_name[MAX_STR_LEN];
	uint8_t value = 0;

    memset(file_name, 0, sizeof(file_name));
    snprintf(file_name, sizeof(file_name), SYSFS_GPIO_PATH "/gpio%d/value", (GPIOBASE + gpionum));

    fd = fopen(file_name, "r");
	
    if (fd < 0) {
        printf("Open %s/gpio%d/value failed\n", SYSFS_GPIO_PATH, (GPIOBASE + gpionum));
        return -1;
    }
	fread(&value,sizeof(int),1,fd);

    fclose(fd);

    return value;
}

int setGPIODirection (int gpionum, char const *dir) {
    FILE *fd;
    char file_name[MAX_STR_LEN];
	uint8_t len= 0 ;

    memset(file_name,  0, sizeof(file_name));
    snprintf(file_name, sizeof(file_name), SYSFS_GPIO_PATH  "/gpio%d/direction", (GPIOBASE + gpionum));

    fd = fopen(file_name, "w");
    if (fd < 0) {
        printf("Open %s/gpio%d/direction failed\n", SYSFS_GPIO_PATH, (GPIOBASE + gpionum));
        return -1;
    }
	len	=	strlen(dir) +1 ;
	fwrite(dir,sizeof(int),len,fd);
    
	fclose(fd);

    return 0;
}


int ControlBMCLEDStatus (char *LEDName, char *LEDStatus){
	FILE *fd;
    char file_name[MAX_STR_LEN];
	uint8_t len= 0 ;

    memset(file_name,  0, sizeof(file_name));
    snprintf(file_name, sizeof(file_name), SYSFS_LED_PATH  "/%s/trigger",LEDName );

    fd = fopen(file_name, "w");
    if (fd < 0) {
        printf("Open %s/%s/trigger failed\n",SYSFS_LED_PATH, LEDName );
        return -1;
    }
	
	len	=	strlen(LEDStatus) +1 ;
	fwrite(LEDStatus,sizeof(int),len,fd);
    
	fclose(fd);

    return 0;
	
}

ipmi_ret_t ipmiOemPDBPowerCycle(ipmi_netfn_t netfn, ipmi_cmd_t cmd,
                                   ipmi_request_t request,
                                   ipmi_response_t response,
                                   ipmi_data_len_t data_len,
                                   ipmi_context_t context)
{
	//step1: get host status on/off
	//step2: set host off if it is on/off
	//step3: sleep(10), wait host from S0-S5
	//step4: Run PDB restart 
	
	uint8_t gpio_direction = 0;
	uint8_t len  = *data_len;
	
	ExportGpio(PDB_RESTART_N);
	ExportGpio(FM_PCH_PWRBTN_N);
	ExportGpio(PWRGD_SYS_PWROK_BMC);
	
	if(len != 0)
	{
		return IPMI_CC_REQ_DATA_LEN_INVALID;
	}
	
	phosphor::logging::log<phosphor::logging::level::INFO>(
        "ipmiOemPDBPowerCycle");
	gpio_direction = getGPIOValue(PWRGD_SYS_PWROK_BMC);
	
	
	//step 2
	if(gpio_direction == 0x31)
	{
		 phosphor::logging::log<phosphor::logging::level::INFO>(
        "System power 111 is OK");
		
		setGPIODirection(FM_PCH_PWRBTN_N, "low");
		std::this_thread::sleep_for (6s);
		setGPIOValue(FM_PCH_PWRBTN_N,GPIO_VALUE_H);
		
		std::this_thread::sleep_for (10s);
		phosphor::logging::log<phosphor::logging::level::INFO>(
        "System power 44444 is OK");
	}
		
	//step 3
	setGPIODirection(PDB_RESTART_N, "low");
	
	*data_len = 0;
	
	return IPMI_CC_OK;      
}
	

ipmi_ret_t ipmiOemSetBIOSLoadDefaultStatus(ipmi_netfn_t netfn, ipmi_cmd_t cmd,
                                   ipmi_request_t request,
                                   ipmi_response_t response,
                                   ipmi_data_len_t data_len,
                                   ipmi_context_t context)
{
	uint8_t *req = reinterpret_cast<uint8_t *>(request);
	uint8_t len = *data_len;
	
	std::string BIOSLoadDefaultStr;
	
	if(req[0] != BIOS_POST_END && req[0] != BIOS_POST_NOT_FINISH)
	{
		return IPMI_CC_INVALID_FIELD_REQUEST;
	}
	
	if(len != 1)
	{
		return IPMI_CC_REQ_DATA_LEN_INVALID;
	}
	
	
	BIOSLoadDefaultStr = bytesToStr(req,len);
	oemData[BIOS_LOAD_Default_Flag] = BIOSLoadDefaultStr.c_str();
	flushOemData();
	*data_len = 0;
	
	return IPMI_CC_OK;      
}

ipmi_ret_t ipmiOemGetBIOSLoadDefaultStatus(ipmi_netfn_t netfn, ipmi_cmd_t cmd,
                                   ipmi_request_t request,
                                   ipmi_response_t response,
                                   ipmi_data_len_t data_len,
                                   ipmi_context_t context)
{
	uint8_t *res = reinterpret_cast<uint8_t *>(response);
	std::string BIOSLoadDefaultStr;
	int len = *data_len;
	
	if(len != 0)
	{
		return IPMI_CC_REQ_DATA_LEN_INVALID;
	}
	
	BIOSLoadDefaultStr = oemData[BIOS_LOAD_Default_Flag];	
    *data_len = strToBytes(BIOSLoadDefaultStr, res);
	*data_len = 1;
	
	return IPMI_CC_OK;      
}
ipmi_ret_t ipmiOemSetBIOSCurrentPID(ipmi_netfn_t netfn, ipmi_cmd_t cmd,
                                   ipmi_request_t request,
                                   ipmi_response_t response,
                                   ipmi_data_len_t data_len,
                                   ipmi_context_t context)
{
	uint8_t *req = reinterpret_cast<uint8_t *>(request);
	uint8_t len = *data_len;
	
	std::string BIOSCurrentPIDStr;
	
	if(len != 1)
	{
		return IPMI_CC_REQ_DATA_LEN_INVALID;
	}
	
	
	BIOSCurrentPIDStr = bytesToStr(req,len);
	oemData[BIOS_Profile_Current_Configuration] = BIOSCurrentPIDStr.c_str();
	flushOemData();
	*data_len = 0;
	
	return IPMI_CC_OK;      
}

ipmi_ret_t ipmiOemGetBIOSCurrentPID(ipmi_netfn_t netfn, ipmi_cmd_t cmd,
                                   ipmi_request_t request,
                                   ipmi_response_t response,
                                   ipmi_data_len_t data_len,
                                   ipmi_context_t context)
{
	uint8_t *res = reinterpret_cast<uint8_t *>(response);
	std::string BIOSCurrentPIDStr;
	int len = *data_len;
	
	if(len != 0)
	{
		return IPMI_CC_REQ_DATA_LEN_INVALID;
	}
	
	BIOSCurrentPIDStr = oemData[BIOS_Profile_Current_Configuration];	
    *data_len = strToBytes(BIOSCurrentPIDStr, res);
	*data_len = 1;
	
	return IPMI_CC_OK;      
}

ipmi_ret_t ipmiOemSetBIOSWantedPID(ipmi_netfn_t netfn, ipmi_cmd_t cmd,
                                   ipmi_request_t request,
                                   ipmi_response_t response,
                                   ipmi_data_len_t data_len,
                                   ipmi_context_t context)
{
	uint8_t *req = reinterpret_cast<uint8_t *>(request);
	uint8_t len = *data_len;
	
	std::string BIOSWantedPIDStr;
	
	if(len != 1)
	{
		return IPMI_CC_REQ_DATA_LEN_INVALID;
	}
	
	
	BIOSWantedPIDStr = bytesToStr(req,len);
	oemData[BIOS_Profile_Wanted_Configuration] = BIOSWantedPIDStr.c_str();
	flushOemData();
	*data_len = 0;
	
	return IPMI_CC_OK;      
}

ipmi_ret_t ipmiOemGetBIOSWantedPID(ipmi_netfn_t netfn, ipmi_cmd_t cmd,
                                   ipmi_request_t request,
                                   ipmi_response_t response,
                                   ipmi_data_len_t data_len,
                                   ipmi_context_t context)
{
	uint8_t *res = reinterpret_cast<uint8_t *>(response);
	std::string BIOSWantedPIDStr;
	int len = *data_len;
	
	if(len != 0)
	{
		return IPMI_CC_REQ_DATA_LEN_INVALID;
	}
	
	BIOSWantedPIDStr = oemData[BIOS_Profile_Wanted_Configuration];	
    *data_len = strToBytes(BIOSWantedPIDStr, res);
	*data_len = 1;
	
	return IPMI_CC_OK;      
}
#define LED_OFF  0
#define LED_ON   1
#define LED_BLINKING 2

ipmi_ret_t ipmiOemControlLEDStatus(ipmi_netfn_t netfn, ipmi_cmd_t cmd,
                                   ipmi_request_t request,
                                   ipmi_response_t response,
                                   ipmi_data_len_t data_len,
                                   ipmi_context_t context)
{
	uint8_t *req = reinterpret_cast<uint8_t *>(request);
	uint8_t len = *data_len;
	
	char LEDName[MAX_STR_LEN];
	char LEDStatus[MAX_STR_LEN];
	
	if(req[1] != LED_ON && req[1] != LED_OFF && req[1] != LED_BLINKING)
	{
		return IPMI_CC_INVALID_FIELD_REQUEST;
	}
	
	if(len != 2)
	{
		return IPMI_CC_REQ_DATA_LEN_INVALID;
	}
	 switch(req[0])
	 {
		case 0:
			strcpy(LEDName,"heartbeat");
			break;
		case 1:
			strcpy(LEDName,"fault");
			break;
		default:
			break;
	 };	
	 
	 switch(req[1])
	 {
		case 0:
			strcpy(LEDStatus,"none");
			break;
		case 1:
			strcpy(LEDStatus,"default-on");
			break;			
		case 2:
			strcpy(LEDStatus,"timer");
			break;
		default:
			break;
	 };	 
	ControlBMCLEDStatus(LEDName,LEDStatus);
	*data_len = 0;
	
	return IPMI_CC_OK;      
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
						 
	ipmiPrintAndRegister(NETFUN_OEM, CMD_OEM_SET_BIOS_LOAD_DEFAULT_STATUS, NULL,
                         ipmiOemSetBIOSLoadDefaultStatus,
                         PRIVILEGE_USER); 
						 
	ipmiPrintAndRegister(NETFUN_OEM, CMD_OEM_GET_BIOS_LOAD_DEFAULT_STATUS, NULL,
                         ipmiOemGetBIOSLoadDefaultStatus,
                         PRIVILEGE_USER); 
	
	ipmiPrintAndRegister(NETFUN_OEM, CMD_OEM_SET_BIOS_CURRENT_PID, NULL,
                         ipmiOemSetBIOSCurrentPID,
                         PRIVILEGE_USER); 
						 
	ipmiPrintAndRegister(NETFUN_OEM, CMD_OEM_GET_BIOS_CURRENT_PID, NULL,
                         ipmiOemGetBIOSCurrentPID,
                         PRIVILEGE_USER);
						 
	ipmiPrintAndRegister(NETFUN_OEM, CMD_OEM_SET_BIOS_WANTED_PID, NULL,
                         ipmiOemSetBIOSWantedPID,
                         PRIVILEGE_USER); 
						 
	ipmiPrintAndRegister(NETFUN_OEM, CMD_OEM_GET_BIOS_WANTED_PID, NULL,
                         ipmiOemGetBIOSWantedPID,
                         PRIVILEGE_USER);
						 
	ipmiPrintAndRegister(NETFUN_OEM, CMD_OEM_PDB_POWER_CYCLE, NULL,
                         ipmiOemPDBPowerCycle,
                         PRIVILEGE_USER); 
						 
	ipmiPrintAndRegister(NETFUN_OEM, CMD_OEM_Control_LED_Status, NULL,
                         ipmiOemControlLEDStatus,
                         PRIVILEGE_USER); 
#endif
}


} // namespace ipmi


