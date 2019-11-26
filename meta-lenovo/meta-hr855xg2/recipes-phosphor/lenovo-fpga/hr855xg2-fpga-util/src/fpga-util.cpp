/**
 * Copyright (c) 2017 Intel Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "../include/argument.hpp"
#include "../include/fpga-util.hpp"
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <filesystem>
#include <regex>
#ifdef __cplusplus
extern "C" {
#endif
#include <fpga.h>
#ifdef __cplusplus
}
#endif

// Get a handle to system dbus.
auto bus = sdbusplus::bus::new_default();

int add_sel_event(uint8_t snr_num, uint8_t snr_type,
                  uint8_t ed1, uint8_t ed2, uint8_t ed3)
{
    uint8_t netFn = 0x0A; // Storage
    uint8_t lun = 0x00;
    uint8_t cmd = 0x44; // Add SEL Entry command
    std::vector<uint8_t> data = { 0x00, 0x00,             // Record ID
                                  0x02,                   // Record Type
                                  0x00, 0x00, 0x00, 0x00, // Time Stamp
                                  0x20, 0x00,             // Generator ID
                                  0x04,                   // EvM Rev
                                  snr_type,               // Sensor Type
                                  snr_num,                // Sensor #
                                  0x6F,                   // Event Dir | Event Type
                                  ed1,                    // Event Data 1
                                  ed2,                    // Event Data 2
                                  ed3};                   // Event Data 3
    std::map<std::string, sdbusplus::message::variant<int>> options;
    try
    {
        auto add_sel = bus.new_method_call(IPMIService, IPMIPath, IPMIIntf, "execute");
        add_sel.append(netFn, lun, cmd, data, options);
        auto reply = bus.call(add_sel);

        if (reply.is_method_error())
        {
            std::cerr << "Failed to add sel by method error\n";
            return 1;
        }
    }
    catch (const sdbusplus::exception::SdBusError& e)
    {
        std::cerr << "Failed to add sel\n" << e.what();
        return 1;
    }

    return 0;
}

uint8_t fpga_get_data(uint8_t block, uint8_t offset, uint8_t *res_data) {

    // fpga library method
    int fd = -1;
    fd = altera_fpga_init();

    if (fd == -1)
    {
        std::cerr << "Failed to init fpga\n";
        return fd;
    }

    fpga_cmd_t t;
    t.action = 0;
    t.adi = 0;
    t.block = block;
    t.offset = offset;
    t.length = 1;

    int rc = fpga_read(&t);
    if (0 != rc)
    {
        std::cerr << "fpga read error: " << rc << "\n";
        close_dev(fd);
        return -1;
    }

    *res_data = t.data[0];

    close_dev(fd);
    return rc;
}

int CPU_Status_Presence(void)
{
    uint8_t block = 28;
    uint8_t offset = 7;
    bool present = true;

    for (int snr_num=CPU0_Status; snr_num<=CPU3_Status; snr_num++)
    {
        uint8_t res_data = 0xFF;
        int rc = fpga_get_data(block, offset, &res_data);
        if (rc != 0)
        {
            std::cerr << "Could not get fpga data\n";
        }

        offset++;
        present = (~res_data >> 3) & 0x01;
        if (present)
        {
            int retry = 3;
            while (retry)
            {
                uint8_t ret = add_sel_event(snr_num, 0x07, 0x07, 0xFF, 0xFF);
                if (0 != ret)
                {
                    std::cerr << "Sensor " << static_cast<unsigned>(snr_num) << ": Failed to add sel event, retry " 
                              << static_cast<unsigned>(retry) << std::endl;
                    retry--;
                    sleep(5);
                }
                else
                {
                    retry = 0;
                }
            }
        }
    }

    return 0;
}

int DIMM_Status_Presence(void)
{
    // DIMM Present by CPLD
    uint64_t dimm_present = 0;
    for (uint8_t dimm_offset=6; dimm_offset<12; dimm_offset++)
    {
        uint8_t DIMM_FpgaBlock = 10;
        uint8_t res_data = 0x00;
        int rc = fpga_get_data(DIMM_FpgaBlock, dimm_offset, &res_data);
        if (rc != 0)
        {
            std::cerr << "Could not get fpga data\n";
        }

        dimm_present |= ((uint64_t) res_data) << ((dimm_offset-6)*8);
    }

    std::cerr << "dimm_present: " << static_cast<unsigned>(dimm_present) << std::endl;
    for (uint64_t dimm_num=0; dimm_num<48; dimm_num++)
    {
        uint8_t phy_dimm_num = 0;

        if (dimm_num < 24) // DIMM0 to DIMM23
        {
            phy_dimm_num = ((dimm_num/12 + 1)*12 - 1) - (dimm_num%12);
        }
        else
        {
            phy_dimm_num = dimm_num;
        }

        if (0 != ((dimm_present & (1 << dimm_num))))
        {
            // Add SEL
            int retry = 3;
            while (retry)
            {
                uint8_t ret = add_sel_event((0xA0 + phy_dimm_num), 0x0C, 0x06, 0xFF, 0xFF);
                if (0 != ret)
                {
                    std::cerr << "Sensor " << static_cast<unsigned>((0xA0 + phy_dimm_num)) << ": Failed to add sel event, retry "
                              << static_cast<unsigned>(retry) << std::endl;
                    retry--;
                    sleep(5);
                }
                else
                {
                    retry = 0;
                }
            }
        }
    }

    return 0;
}

int event_detect(req_info_t req_info, uint8_t snr_num, uint8_t snr_type, 
                 uint8_t ed1, uint8_t ed2, uint8_t ed3)
{
    uint8_t res_data = 0x00;
    bool val = true;

    int rc = fpga_get_data(req_info.block, req_info.offset, &res_data);
    if (rc != 0)
    {
        std::cerr << "Could not get fpga data\n";
        return rc;
    }

    val = (res_data >> req_info.bit) & 0x01;
    if (req_info.trigstate == val)
    {
        int retry = 3;
        while (retry)
        {
            uint8_t ret = add_sel_event(snr_num, snr_type, ed1, ed2, ed3);
            if (0 != ret)
            {
                std::cerr << "Sensor " << static_cast<unsigned>(snr_num) << ": Failed to add sel event, retry "
                          << static_cast<unsigned>(retry) << std::endl;
                retry--;
                sleep(5);
            }
            else
            {
                retry = 0;
            }
        }
    }

    return 0;
}

static void ExitWithError(const char* err, char** argv)
{
    ArgumentParser::usage(argv);
    std::cerr << std::endl;
    std::cerr << "ERROR: " << err << std::endl;
    exit(-1);
}

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        ArgumentParser::usage(argv);
        return -1;
    }

    auto options = ArgumentParser(argc, argv);

    auto argv_snr = options["snr"];
    if (argv_snr != ArgumentParser::empty_string)
    {
        uint8_t snr = 0;
        if (0 == argv_snr.find("0x", 0))
        {
            snr = (uint8_t) std::stoi(argv_snr.c_str(), 0, 16);
        }
        else
        {
            snr = (uint8_t) std::stoi(argv_snr.c_str());
        }

        int ret = -1;
        if (snr >= CPU0_Status && snr <= CPU3_Status)
        {
            ret = CPU_Status_Presence();
            if (ret != 0)
            {
                std::cerr << "Failed to get CPU presence\n";
                return ret;
            }
        }
        else if (snr >= DIMM0_Status && snr <= DIMM47_Status)
        {
            ret = DIMM_Status_Presence();
            if (ret != 0)
            {
                std::cerr << "Failed to get DIMM presence\n";
                return ret;
            }
        }
        else if (snr >= PE1S1_Status && snr <= PE3S2_Status)
        {
            // PEXSX Bus installed detection by CPLD
            req_info_t req_info[] = {
                { 12, 4, 0, 0 },
                { 12, 4, 1, 0 },
                { 12, 4, 2, 0 },
                { 12, 4, 3, 0 },
                { 12, 4, 4, 0 },
                { 12, 4, 7, 0 },
                { 12, 5, 0, 0 },
                { 12, 5, 1, 0 },
                { 12, 5, 2, 0 },
                { 12, 5, 3, 0 },
                { 12, 4, 5, 0 },
                { 12, 4, 6, 0 },
            };
            uint8_t index = snr - PE1S1_Status;
 
            ret = event_detect(req_info[index], snr, 0x21, 0x02, 0xFF, 0xFF);
            if (ret != 0)
            {
                std::cerr << "Failed to get PEXSX presence\n";
                return ret;
            }
        }
        else if (snr >= Riser_1_Status && snr <= Riser_3_Status)
        {
            // Riser installed detection by CPLD
            req_info_t req_info[] = {
                { 12, 10, 0, 0 },
                { 12, 10, 1, 0 },
                { 12, 10, 2, 0 },
            };
            uint8_t index = snr - Riser_1_Status;

            ret = event_detect(req_info[index], snr, 0x21, 0x02, 0xFF, 0xFF);
            if (ret != 0)
            {
                std::cerr << "Failed to get RiserX presence\n";
                return ret;
            }
        }
        else if (M2_Status == snr)
        {
            // M.2 Presence detection by CPLD
            req_info_t req_info[] = {
                { 12, 10, 3, 0 },
                { 12, 10, 3, 1 },
            };

            for (uint8_t index=0; index<sizeof(req_info)/sizeof(req_info_t); index++)
            {
                ret = event_detect(req_info[index], snr, 0x17, (uint8_t) req_info[index].trigstate, 0xFF, 0xFF);
                if (ret != 0)
                {
                    std::cerr << "Failed to get M.2 presence\n";
                    return ret;
                }
            }
        }
        else if (NCSI_Cable == snr)
        {
            // NCSI Cable detection by CPLD
            req_info_t req_info[] = {
                { 12, 10, 6, 0 },
                { 12, 10, 6, 1 },
            };

            for (uint8_t index=0; index<sizeof(req_info)/sizeof(req_info_t); index++)
            {
                ret = event_detect(req_info[index], snr, 0x17, (uint8_t) req_info[index].trigstate, 0xFF, 0xFF);
                if (ret != 0)
                {
                    std::cerr << "Failed to get NCSI cable state\n";
                    return ret;
                }
            }
        }
        else
        {
            std::cerr << "Incorrect FPGA sensor " << static_cast<unsigned>(snr) << "\n";
            return ret;
        }
    }
    else
    {
        auto argv_block = options["block"];
        if (argv_block == ArgumentParser::empty_string)
        {
            ExitWithError("block not specified", argv);
        }
        uint8_t block = (uint8_t) std::stoi(argv_block.c_str());

        auto argv_offset = options["offset"];
        if (argv_offset == ArgumentParser::empty_string)
        {
            ExitWithError("offset not specified", argv);
        }
        uint8_t offset = (uint8_t) std::stoi(argv_offset.c_str());
    
        uint8_t res_data = 0xFF;
        int rc = fpga_get_data(block, offset, &res_data);

        if (rc != 0)
        {
            std::cerr << "Could not get fpga data\n";
            return -1;
        }

        std::cerr << static_cast<unsigned>(res_data) << std::endl;
    }

    return 0;
}
