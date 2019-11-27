/*
// Copyright (c) 2019-present Lenovo
// Licensed under BSD-3, see LICENSE file for details.
*/

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
#include <ipmid/oemopenbmc.hpp>

namespace lenovo
{
namespace ipmi
{

	using phosphor::logging::level;
	using phosphor::logging::log;
	using phosphor::logging::entry;
	
	static constexpr auto fpgaService = "org.openbmc.control.fpga";
    static constexpr auto fpgaRoot = "/org/openbmc/control/fpga";
    static constexpr auto fpgaInterface = "org.openbmc.control.fpga";
	//auto bus = sdbusplus::bus::new_default();
	static constexpr auto pwmHwmonPath = "/sys/class/hwmon/hwmon0/pwm";
	

    sdbusplus::bus::bus bus(ipmid_get_sd_bus_connection()); // from ipmid/api.h
   
nlohmann::json oemData;
	
void flushOemData()
{
    std::ofstream file(JSON_OEM_DATA_FILE);
    file << oemData;
    return;
}

int retrieveOemData ()
{
    std::ifstream file(JSON_OEM_DATA_FILE);
    if (file) {
        file >> oemData;
    } else {
        return -1;
    }
	return 0;
}

std::string bytesToStr(const uint8_t *byte, int len)
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

int strToBytes(const std::string &str, uint8_t *data)
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
   

ipmi_ret_t ipmioemReadFPGA(ipmi_cmd_t cmd,
                          const uint8_t* request,
                           uint8_t* response,
                           size_t* data_len
                           )
{
	ipmi_ret_t rc = IPMI_CC_OK;
	uint8_t res_data;
	size_t respLen = 0;
	fpgadata resp = {0};
	struct readFPGA req; 
	
	
	if(*data_len != sizeof(readFPGA)){
		*data_len = 0;
		return IPMI_CC_REQ_DATA_LEN_INVALID;
		
	}
	std::memcpy(&req, &request[0], sizeof(req));
	
	auto method = bus.new_method_call(fpgaService, fpgaRoot,
                                      fpgaInterface, "read_fpga");
	
	method.append(req.block,req.offset);
	
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

ipmi_ret_t ipmioemWriteFPGA( ipmi_cmd_t cmd,
                              const uint8_t* request,
                              uint8_t* response,
                              size_t* data_len
                             )
{
	
	ipmi_ret_t rc = IPMI_CC_OK;
	struct writeFPGA req; 
	
	if(*data_len != sizeof(writeFPGA)){
		*data_len = 0;
		return IPMI_CC_REQ_DATA_LEN_INVALID;
		
	}
	
	std::memcpy(&req, &request[0], sizeof(req));	
	
	auto method = bus.new_method_call(fpgaService, fpgaRoot,
                                      fpgaInterface, "write_fpga");
	method.append(req.block,req.offset,req.data);
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


ipmi_ret_t ipmiOemSetBIOSLoadDefaultStatus( ipmi_cmd_t cmd,
											const uint8_t* req,
											uint8_t* res,
											size_t* data_len
											)
{
	
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

ipmi_ret_t ipmiOemGetBIOSLoadDefaultStatus( ipmi_cmd_t cmd,
											const uint8_t* req,
											uint8_t* res,
											size_t* data_len
											)
{
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
ipmi_ret_t ipmiOemSetBIOSCurrentPID( ipmi_cmd_t cmd,
									const uint8_t* req,
									 uint8_t* res,
									 size_t* data_len
									)
{
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

ipmi_ret_t ipmiOemGetBIOSCurrentPID( ipmi_cmd_t cmd,
									const uint8_t* req,
									 uint8_t* res,
									 size_t* data_len
									)
{
   
    std::string BIOSCurrentPIDStr;
    int len = *data_len;
	
    if(len != 0) {
        return IPMI_CC_REQ_DATA_LEN_INVALID;
    }
	
    if (retrieveOemData() != 0) {
        return IPMI_CC_INVALID_FIELD_REQUEST;
    }

    BIOSCurrentPIDStr = oemData[BIOS_Profile_Current_Configuration];	
    *data_len = strToBytes(BIOSCurrentPIDStr, res);
    *data_len = 1;
	
    return IPMI_CC_OK;      
}

ipmi_ret_t ipmiOemSetBIOSWantedPID( ipmi_cmd_t cmd,
                            const  uint8_t* req,
                              uint8_t* res,
                              size_t* data_len
                             )
{
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

ipmi_ret_t ipmiOemGetBIOSWantedPID( ipmi_cmd_t cmd,
                           const   uint8_t* req,
                              uint8_t* res,
                              size_t* data_len
                             )
{
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

ipmi_ret_t ipmiOemControlLEDStatus( ipmi_cmd_t cmd,
                            const  uint8_t* req,
                              uint8_t* res,
                              size_t* data_len
                             )
{
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

uint8_t count_pwm(void)
{
    FILE *f = NULL; // for use with popen
    std::string command;
    std::string pwm_path("/sys/class/hwmon/**/pwm*");
    uint8_t num = 0;

    command = "ls " + pwm_path + " | wc -l ";
    f = popen(command.c_str(), "r");
    fscanf(f, "%hhx", &num);
    pclose(f);

    return num;
}

ipmi_ret_t ipmiOemSetManualPWMVal( ipmi_cmd_t cmd,
                             const  uint8_t* request,
                              uint8_t* response,
                              size_t* data_len
                             )
{
    ipmi_ret_t rc = IPMI_CC_OK;
    uint8_t reqPWMName, reqPWMVal, total_pwm = 0;

    /* Check if data_len is valid or not,
     * if not, return INVALID_FIELD_REQUEST.
     */
    if (*data_len < sizeof(struct FanCtrlSetPWM))
    {
        return IPMI_CC_INVALID_FIELD_REQUEST;
    }

    const auto req =
        reinterpret_cast<const struct FanCtrlSetPWM*>(request);

    /* Check if any inputs are valid,
     * if not, return IPMI_CC_INVALID
     */
    reqPWMName = req->pwm_name;
    reqPWMVal = req->val;

    /* Fetch current PWM numbers on the system
     * and save it to total_pwm.
     */

    total_pwm = count_pwm();

    if ( reqPWMVal > 0xFF || reqPWMName > total_pwm )
        return IPMI_CC_INVALID;

    /* Write PWM value to the designated PWM path.
     * Output any errors.
     */
    std::string pwmdev = pwmHwmonPath + std::to_string(reqPWMName);
    std::ofstream ofile;
    ofile.open(pwmdev);
    if (ofile)
    {
        ofile << static_cast<int64_t>(reqPWMVal);
        ofile.close();
        return rc;
    }
    else
    {
        std::cerr <<" Fail to write to PWM, path: "<< pwmdev.c_str() << std::endl;
        return IPMI_CC_INVALID;
    }
}

void setupLenovoOEMCommands() __attribute__((constructor));

void setupLenovoOEMCommands()
{
    oem::Router* oemRouter = oem::mutableRouter();
	std::fprintf(stderr,
                 "Registering OEM:[%#08X] lenovo OEM Commands\n",
                 lenovoOemNumber);
				 
	
	oemRouter->registerHandler(lenovoOemNumber, CMD_OEM_READ_FPGA,ipmioemReadFPGA);
	
	
	oemRouter->registerHandler(lenovoOemNumber, CMD_OEM_WRITE_FPGA,ipmioemWriteFPGA);
						 
	oemRouter->registerHandler(lenovoOemNumber, CMD_OEM_SET_BIOS_LOAD_DEFAULT_STATUS,ipmiOemSetBIOSLoadDefaultStatus); 
						 
	oemRouter->registerHandler(lenovoOemNumber, CMD_OEM_GET_BIOS_LOAD_DEFAULT_STATUS,ipmiOemGetBIOSLoadDefaultStatus); 
	
	oemRouter->registerHandler(lenovoOemNumber, CMD_OEM_SET_BIOS_CURRENT_PID,ipmiOemSetBIOSCurrentPID); 
						 
	oemRouter->registerHandler(lenovoOemNumber, CMD_OEM_GET_BIOS_CURRENT_PID,ipmiOemGetBIOSCurrentPID);
						 
	oemRouter->registerHandler(lenovoOemNumber, CMD_OEM_SET_BIOS_WANTED_PID,ipmiOemSetBIOSWantedPID); 
						 
	oemRouter->registerHandler(lenovoOemNumber, CMD_OEM_GET_BIOS_WANTED_PID,ipmiOemGetBIOSWantedPID);
						  
						 
	oemRouter->registerHandler(lenovoOemNumber, CMD_OEM_Control_LED_Status,ipmiOemControlLEDStatus);

	oemRouter->registerHandler(lenovoOemNumber, CMD_OEM_SET_FAN_PWM, ipmiOemSetManualPWMVal);
	
	
}

} // namespace ipmi

} // namespace lenovo


