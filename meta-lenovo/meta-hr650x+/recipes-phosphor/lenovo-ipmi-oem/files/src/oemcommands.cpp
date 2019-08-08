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
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

namespace ipmi
{
    using phosphor::logging::level;
    using phosphor::logging::log;
    using phosphor::logging::entry;

    static void registerOEMFunctions() __attribute__((constructor));
    sdbusplus::bus::bus dbus(ipmid_get_sd_bus_connection()); // from ipmid/api.h

uint8_t i2cMasterWriteRead(int fd, uint16_t addr,  uint8_t *wbuf, uint16_t write_len, uint8_t *rbuf, uint16_t read_len)
{
    struct i2c_rdwr_ioctl_data data;
    struct i2c_msg msg[2];
    int ret = 0;
    uint8_t nmsgs = 0;

    if (write_len)
    {
        msg[nmsgs].len = write_len;
        msg[nmsgs].buf = wbuf;
        msg[nmsgs].addr = addr;
        msg[nmsgs].flags = 0;        // Master write
        nmsgs++;
    }

    if (read_len)
    {
        msg[nmsgs].len = read_len;
        msg[nmsgs].buf = rbuf;
        msg[nmsgs].addr = addr;
        msg[nmsgs].flags = I2C_M_RD;
        nmsgs++;
    }

    data.msgs = msg;
    data.nmsgs = nmsgs;

    ret = ioctl(fd, I2C_RDWR, &data);
    if (ret < 0) {
        phosphor::logging::log<level::ERR>("Failed to do ioctl on FPGA");
        return ret;
    }
    return 0;
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
    uint8_t rsp;
    auto *req = reinterpret_cast<uint8_t *>(request);
    if (*data_len != REQ_LEN_READ_FPGA)
    {
        *data_len = 0;
        return IPMI_CC_REQ_DATA_LEN_INVALID;
    }

    try
    {
        int fd = open(I2C_BUS_FPGA, O_RDWR | O_CLOEXEC);
        if (fd < 0)
        {
            phosphor::logging::log<level::INFO>("Open FPGA i2c bus failed");
            return IPMI_CC_UNSPECIFIED_ERROR;
        }

        int ret = i2cMasterWriteRead(fd, I2C_ADDR_FPGA, req, 1, &rsp, 1);
        if (ret != 0) {
            phosphor::logging::log<level::INFO>("Failed to communicate with FPGA");
            close(fd);
            return IPMI_CC_UNSPECIFIED_ERROR;
        }
        *data_len = 1;
        std::memcpy(response, &rsp, *data_len);

        close(fd);
    }
    catch (const sdbusplus::exception::SdBusError& e)
    {
        phosphor::logging::log<level::ERR>("Failed to get data from FPGA");
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
    auto *req = reinterpret_cast<uint8_t *>(request);
    if (*data_len != REQ_LEN_WRITE_FPGA)
    {
        *data_len = 0;
        return IPMI_CC_REQ_DATA_LEN_INVALID;
    }

    try
    {
        int fd = open(I2C_BUS_FPGA, O_RDWR | O_CLOEXEC);
        if (fd < 0)
        {
            phosphor::logging::log<level::INFO>("Open FPGA i2c bus failed");
            return IPMI_CC_UNSPECIFIED_ERROR;
        }

        int ret = i2cMasterWriteRead(fd, I2C_ADDR_FPGA, req, 2, NULL, 0);
        if (ret != 0) {
            phosphor::logging::log<level::INFO>("Failed to communicate with FPGA");
            close(fd);
            return IPMI_CC_UNSPECIFIED_ERROR;
        }
        close(fd);
    }
    catch (const sdbusplus::exception::SdBusError& e)
    {
        phosphor::logging::log<level::ERR>("Failed to set data to FPGA");
        return IPMI_CC_INVALID_FIELD_REQUEST;
    }
    *data_len = 0;
    return rc;
}

static void registerOEMFunctions(void)
{
    phosphor::logging::log<phosphor::logging::level::INFO>(
        "Registering Lenovo OEM commands");
        
    ipmiPrintAndRegister(NETFUN_OEM, CMD_OEM_ADD_UEFI_SEL, NULL,
                         ipmiOemAddUefiSel,
                         PRIVILEGE_USER);

    ipmiPrintAndRegister(NETFUN_OEM, CMD_OEM_READ_FPGA, NULL,
                         ipmioemReadFPGA,
                         PRIVILEGE_ADMIN);

    ipmiPrintAndRegister(NETFUN_OEM, CMD_OEM_WRITE_FPGA, NULL,
                         ipmioemWriteFPGA,
                         PRIVILEGE_ADMIN);
}


} // namespace ipmi


