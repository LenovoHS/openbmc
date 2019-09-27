/*
// Copyright (c) 2018 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
*/

#include <unistd.h>

#include <FPGASensor.hpp>
#include <Utils.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <iostream>
#include <limits>
#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/asio/object_server.hpp>
#include <string>
#include <gpiod.hpp>

static constexpr bool DEBUG = false;

FPGASensor::FPGASensor(boost::asio::io_service& io,
                       std::shared_ptr<sdbusplus::asio::connection>& conn,
                       FPGAInterrupt& sensorconfig) :
    waitTimer(io), mDbusConn(conn), mSnrInterrupt(sensorconfig)
{
    setupRead();
}

FPGASensor::~FPGASensor()
{
    waitTimer.cancel();
}

void FPGASensor::setupRead(void)
{
    if (DEBUG)
    {
        fprintf(stderr, "enter FPGASensor::setupRead\n");
        fprintf(stderr, "Hsensor gpio name: %s\n", mSnrInterrupt.name.c_str());
    }

    for (auto& fpgaconfig : mSnrInterrupt.fpgaconfig)
    {
        if (DEBUG)
        {
            fprintf(stderr, "%s sensor name: %s\n", __func__, fpgaconfig.name);
            fprintf(stderr, "%s sensor num: %x\n", __func__, fpgaconfig.snrnum);
            fprintf(stderr, "%s sensor type: %x\n", __func__, fpgaconfig.snrtype);
        }

        mAssert[fpgaconfig.name] = false;

        uint8_t snrnum = fpgaconfig.snrnum;
        uint8_t snrtype = fpgaconfig.snrtype;
        auto name = fpgaconfig.name;
        auto snriface = fpgaconfig.snriface;
        bool en_mon = fpgaconfig.en_mon;

        for (auto& fpgainfo : fpgaconfig.fpgainfo) 
        {
            uint8_t block = fpgainfo.block;
            uint8_t addr = fpgainfo.addr;
            uint8_t offset = fpgainfo.offset;
            uint8_t evtbit = fpgainfo.evtbit;
            uint8_t trigstate = fpgainfo.trigstate;

            if (DEBUG)
            {
                 fprintf(stderr, "%s block: %x\n", __func__, block);
                 fprintf(stderr, "%s addr: %x\n", __func__, addr);
                 fprintf(stderr, "%s offset: %x\n", __func__, offset);
                 fprintf(stderr, "%s evtbit: %x\n", __func__, evtbit);
                 fprintf(stderr, "%s trigstate: %x\n", __func__, trigstate);
            }

            // Make sure FPGA dbus is ready
            uint8_t res_data = 0xFF;
#if 0
            // dbus methond
            auto method = mDbusConn->new_method_call(fpgaService, fpgaRoot,
                                                     fpgaInterface, "read_fpga");
            method.append(block, addr);
            try
            {
                auto pid = mDbusConn->call(method);
                if (pid.is_method_error())
                {
                    fprintf(stderr, "Error in method call\n");
                }

                pid.read(res_data);
            }
            catch (const sdbusplus::exception::SdBusError& e)
            {
                fprintf(stderr, "fpga dbus error\n");
                waitTimer.cancel();
            }
#else
            // fpga library method
            //altera_fpga_init();
            fpga_cmd_t t;
            t.action = 0;
            t.adi = 0;
            t.block = block; 
            t.offset = addr; 
            t.length = 1;

            int rc = fpga_read(&t);
            if (0 != rc)
            {
                fprintf(stderr, "fpga read error");
            }
            res_data = t.data[0];
#endif
            if (!en_mon)
            {
                uint8_t event = (res_data >> offset) & 0x01;

                auto service = fpgainfo.service;
                auto path = fpgainfo.path;
                auto iface = fpgainfo.iface;
                auto property = fpgainfo.property;
                auto ptype = fpgainfo.ptype;
                auto setvalue = fpgainfo.setvalue;
                int ret = -1;

                if (trigstate == event)
                {
                    // Set Property
                    ret = eventUpdate(fpgaconfig, service, path, iface, property, ptype, setvalue, true);
                    if (0 != ret)
                    {
                        fprintf(stderr, "set property fail\n");
                    }

                    // Add SEL
                    ret = -1;
                    ret = add_sel_event(snrnum, snrtype, evtbit, 0xff, 0xff);
                    if (0 != ret)
                    {
                        fprintf(stderr, "add sel fail\n");
                    }
                }
            } // if (!en_mon)
        }
    }
    handleResponse();
}

void FPGASensor::handleResponse()
{
    size_t pollTime = FPGASensor::sensorPollMs;
    waitTimer.expires_from_now(boost::posix_time::milliseconds(pollTime));
    waitTimer.async_wait([&](const boost::system::error_code& ec) {
        // case of timer expired
        if (!ec)
        {
            if (mSnrInterrupt.name != "NA")
            {
                // Interrupt event wait
                 

                for (auto& fpgaconfig : mSnrInterrupt.fpgaconfig)
                {
                    uint8_t snrnum = fpgaconfig.snrnum;
                    uint8_t snrtype = fpgaconfig.snrtype;
                    auto name = fpgaconfig.name;
                    auto snriface = fpgaconfig.snriface;
                    bool en_mon = fpgaconfig.en_mon;

                    for (auto& fpgainfo : fpgaconfig.fpgainfo)
                    {
                        uint8_t block = fpgainfo.block;
                        uint8_t addr = fpgainfo.addr;
                        uint8_t offset = fpgainfo.offset;
                        uint8_t evtbit = fpgainfo.evtbit;
                        uint8_t trigstate = fpgainfo.trigstate;

                        uint8_t res_data = 0xFF;
#if 0
                        // dbus methond
                        auto method = mDbusConn->new_method_call(fpgaService, fpgaRoot,
                                                                 fpgaInterface, "read_fpga");
                        method.append(block, addr);
                        try
                        {
                            auto pid = mDbusConn->call(method);
                            if (pid.is_method_error())
                            {
                                std::cerr << "Error in method call\n";
                            }

                            pid.read(res_data);
                        }
                        catch (const sdbusplus::exception::SdBusError& e)
                        {
                            std::cerr << "fpga dbus error" << "\n";
                        }
#else
                        // fpga library method
                        //altera_fpga_init();
                        fpga_cmd_t t;
                        t.action = 0;
                        t.adi = 0;
                        t.block = block;
                        t.offset = addr;
                        t.length = 1;

                        int rc = fpga_read(&t);
                        if (0 != rc)
                        {
                            fprintf(stderr, "fpga read error");
                        }
                        res_data = t.data[0];
#endif
                        uint8_t event = 0xFF;
                        event = (res_data >> fpgainfo.offset) & 0x01;

                        auto service = fpgainfo.service;
                        auto path = fpgainfo.path;
                        auto iface = fpgainfo.iface;
                        auto property = fpgainfo.property;
                        auto ptype = fpgainfo.ptype;
                        auto setvalue = fpgainfo.setvalue;
                        int ret = -1;

                        // Assert
                        if (!mAssert[name] && (trigstate == event))
                        {
                            mAssert[name] = true;

                            // Set Property
                            ret = eventUpdate(fpgaconfig, service, path, iface, property, ptype, setvalue, mAssert[name]);
                            if (0 != ret)
                            {
                                fprintf(stderr, "set property fail\n");
                            }

                            // Add SEL
                            ret = -1;
                            ret = add_sel_event(snrnum, snrtype, evtbit, 0xff, 0xff);
                            if (0 != ret)
                            {
                                fprintf(stderr, "add sel fail\n");
                            }
                        }

                        // Deassert
                        if (mAssert[name] && (trigstate != event))
                        {
                            mAssert[name] = false;
                            ret = eventUpdate(fpgaconfig, service, path, iface, property, ptype, setvalue, mAssert[name]);
                            if (0 != ret)
                            {
                                fprintf(stderr, "set property fail\n");
                            }
                        }
                    }
                } // if (mSnrInterrupt.name != "NA")
            }
            // trigger next polling
            handleResponse();
        }
        // case of being canceled
        else if (ec == boost::asio::error::operation_aborted)
        {
            std::cerr << "Timer of fpga sensor is cancelled. Return \n";
            return;
        }
    });
}

int FPGASensor::eventUpdate(FPGAConfig fpgaconfig,
                            std::string service, std::string path,
                            std::string iface, std::string property,
                            std::string ptype, std::string setvalue,
                            bool assert)
{
    uint8_t snrnum = fpgaconfig.snrnum;
    uint8_t snrtype = fpgaconfig.snrtype;
    auto name = fpgaconfig.name;
    auto snriface = fpgaconfig.snriface;
    bool en_mon = fpgaconfig.en_mon;

    // Set Discrete sesnsor
    if (0 != service.compare("xyz.openbmc_project.FPGASensor"))
    {
        if (assert)
        {
            try
            {
                auto method = mDbusConn->new_method_call(service.c_str(), path.c_str(),
                                                         PROP_INTF, "Set");
                if (ptype == "bool")
                {
                    std::variant<bool> value = false;

                    if (setvalue == "1")
                    {
                        value = true;
                    }

                    method.append(iface, property, value);

                    if (!mDbusConn->call(method))
                    {
                        std::cerr << "Failed to set property" << "\n";
                        return -1;
                    }
                }
            }
            catch (const sdbusplus::exception::SdBusError& e)
            {
                std::cerr << e.what() << "\n";
                return -1;
            }
        }
    }
    else
    {
        bool value = assert;
        snriface->set_property(property, value);
    }

    return 0;
}

int FPGASensor::add_sel_event(uint8_t snr_num, uint8_t snr_type,
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
        auto add_sel = mDbusConn->new_method_call(IPMIService, IPMIPath, IPMIIntf, "execute");
        add_sel.append(netFn, lun, cmd, data, options);
        auto reply = mDbusConn->call(add_sel);

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
