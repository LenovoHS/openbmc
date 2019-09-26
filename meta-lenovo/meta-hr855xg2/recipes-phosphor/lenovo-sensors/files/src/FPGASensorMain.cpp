/*
// The source code reference from Intel.
// It will be reviewed by Intel for upstream.
//
// Copyright (c) 2018 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
/      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
*/

#include <fcntl.h>

#include <FPGASensor.hpp>
#include <Utils.hpp>
#include <VariantVisitors.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/container/flat_set.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/process/child.hpp>
#include <filesystem>
#include <fstream>
#include <regex>
#include <boost/asio.hpp>
#include <chrono>
#include <ctime>
#include <iostream>
#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/asio/object_server.hpp>
#include <sdbusplus/asio/sd_event.hpp>
#include <sdbusplus/bus.hpp>
#include <sdbusplus/exception.hpp>
#include <sdbusplus/server.hpp>
#include <sdbusplus/timer.hpp>

static constexpr bool DEBUG = false;

boost::container::flat_map<std::string, std::unique_ptr<FPGASensor>> gFpgaSensors;

namespace fs = std::filesystem;

static constexpr const char* configPrefix =
    "xyz.openbmc_project.Configuration.";
static constexpr std::array<const char*, 1> sensorTypes = {"Fpga"};
static constexpr const char* sensorType =
    "xyz.openbmc_project.Configuration.FPGASensor";

void detecOnlytBooting(const std::shared_ptr<sdbusplus::asio::connection>& systemBus,
                       std::vector<FPGAConfig>& fpgaConfigs);

bool is_number(const std::string& s)
{
    return !s.empty() && std::find_if(s.begin(), 
        s.end(), [](char c) { return !std::isdigit(c); }) == s.end();
}

bool getFpgaConfig(
    const std::shared_ptr<sdbusplus::asio::connection>& systemBus,
    boost::container::flat_set<FPGAInterrupt>& fpgaIntr,
    ManagedObjectType& sensorConfigs,
    sdbusplus::asio::object_server& objectServer,
    boost::container::flat_map<
        std::string, std::shared_ptr<sdbusplus::asio::dbus_interface>>&
        sensorIfaces)
{
    boost::container::flat_map<std::string, std::vector<FPGAConfig>> IntrList;
    bool useCache = false;
    sensorConfigs.clear();

    // use new data the first time, then refresh
    for (const char* type : sensorTypes)
    {
        if (!getSensorConfiguration(configPrefix + std::string(type), systemBus,
                                    sensorConfigs, useCache))
        {
            return false;
        }

        useCache = true;
    }

    // check PECI client addresses and names from FPGA configuration
    // before starting ping operation
    for (const char* type : sensorTypes)
    {
        for (const std::pair<sdbusplus::message::object_path, SensorData>&
                 sensor : sensorConfigs)
        {
            for (const SensorBaseConfiguration& config : sensor.second)
            {
                if ((configPrefix + std::string(type)) != config.first)
                {
                    continue;
                }

                auto findName = config.second.find("Name");
                if (findName == config.second.end())
                {
                    continue;
                }
                std::string nameRaw =
                    std::visit(VariantToStringVisitor(), findName->second);
                std::string name =
                    std::regex_replace(nameRaw, illegalDbusRegex, "_");

                auto findSnrNum = config.second.find("SnrNum");
                if (findSnrNum == config.second.end())
                {
                    std::cerr << "Can't find 'SnrNum' setting in " << name << "\n";
                    continue;
                }
                uint64_t snrnum = std::visit(VariantToUnsignedIntVisitor(), 
				                             findSnrNum->second);

                auto findSnrType = config.second.find("SnrType");
                if (findSnrType == config.second.end())
                {
                    std::cerr << "Can't find 'SnrType' setting in " << name << "\n";
                    continue;
                }
                uint64_t snrtype = std::visit(VariantToUnsignedIntVisitor(),
                                              findSnrType->second);

                auto findFpgaGpio = config.second.find("FpgaGpio");
                if (findFpgaGpio == config.second.end())
                {
                    std::cerr << "Can't find 'FpgaGpio' setting in " << name << "\n";
                    continue;
                }
                std::string fpgagpio =
                    std::visit(VariantToStringVisitor(), findFpgaGpio->second);

                bool en_mon = false;
                if (fpgagpio != "NA")
                {
                    en_mon = true;
                }

                std::vector<FPGAInfo> fpgainfoVector;
                uint64_t FpgaBlock = 0;
                uint64_t FpgaAddr = 0;
                uint64_t FpgaBit = 0;
                uint64_t EvtBit = 0;
                uint64_t FpgaTrigState = 0;
                std::string EvtService = "";
                std::string EvtPath = "";
                std::string EvtIface = "";
                std::string EvtProperty = "";
                std::string EvtPtype = "";
                std::string EvtSetvalue = "";
                for (const SensorBaseConfiguration& suppConfig : sensor.second)
                {
                    if (suppConfig.first.find("Offset") !=
                        std::string::npos)
                    {
                        auto fpgaSetting = suppConfig.second;

                        auto findFpgaBlock = fpgaSetting.find("Block");
                        auto findFpgaAddr = fpgaSetting.find("Addr");
                        auto findFpgaBit = fpgaSetting.find("Bit");
                        auto findEvtBit = fpgaSetting.find("EvtBit");

                        if (findFpgaBlock == fpgaSetting.end() ||
                            findFpgaAddr == fpgaSetting.end() ||
                            findFpgaBit == fpgaSetting.end() ||
                            findEvtBit == fpgaSetting.end())
                        {
                            std::cerr << "Incorrect FPGA configuration setting\n";
                            break;
                        }

                        FpgaBlock = std::visit(VariantToUnsignedIntVisitor(),
                                               findFpgaBlock->second);
                        FpgaBit = std::visit(VariantToUnsignedIntVisitor(),
                                             findFpgaBit->second);
                        FpgaAddr = std::visit(VariantToUnsignedIntVisitor(),
                                              findFpgaAddr->second);
                        EvtBit = std::visit(VariantToUnsignedIntVisitor(),
                                            findEvtBit->second);

                        auto findFpgaTrigState = fpgaSetting.find("TrigState");
                        if (findFpgaTrigState == fpgaSetting.end())
                        {
                            FpgaTrigState = 1;
                        }
                        else
                        {
                            FpgaTrigState = std::visit(VariantToUnsignedIntVisitor(),
                                                       findFpgaTrigState->second);
                        }

                        auto findEvtService = fpgaSetting.find("Service");
                        auto findEvtPath = fpgaSetting.find("Path");
                        auto findEvtIface = fpgaSetting.find("Iface");
                        auto findEvtProperty = fpgaSetting.find("Property");
                        auto findEvtPtype = fpgaSetting.find("Ptype");
                        auto findEvtSetvalue = fpgaSetting.find("Setvalue");

                        if (findEvtService == fpgaSetting.end() ||
                            findEvtIface == fpgaSetting.end() ||
                            findEvtProperty == fpgaSetting.end() ||
                            findEvtPtype == fpgaSetting.end() ||
                            findEvtSetvalue == fpgaSetting.end())
                        {
                            EvtService = "xyz.openbmc_project.FPGASensor";
                            EvtPath = "/xyz/openbmc_project/FPGASensor" + std::string("/") + name;
                            EvtIface = "xyz.openbmc_project.Sensor.Discrete.SpecificOffset";
                            EvtProperty = "Offset_" + std::to_string(EvtBit);
                            EvtPtype = "bool";
                            EvtSetvalue = "1";
                        }
                        else
                        {
                            EvtService = std::visit(VariantToStringVisitor(), findEvtService->second);
                            EvtPath = std::visit(VariantToStringVisitor(), findEvtPath->second);
                            EvtIface = std::visit(VariantToStringVisitor(), findEvtIface->second);
                            EvtPtype = std::visit(VariantToStringVisitor(), findEvtPtype->second);
                            EvtProperty = std::visit(VariantToStringVisitor(), findEvtProperty->second);
                            EvtSetvalue = std::visit(VariantToStringVisitor(), findEvtSetvalue->second);
                        }

                        if (DEBUG)
                        {
                            std::cerr << "snrnum: " << snrnum << "\n";
                            std::cerr << "snrtype: " << snrtype << "\n";
                            std::cerr << "name: " << name << "\n";
                            std::cerr << "block: " << FpgaBlock << "\n";
                            std::cerr << "addr: " << FpgaAddr << "\n";
                            std::cerr << "offset: " << FpgaBit << "\n";
                            std::cerr << "evtbit: " << EvtBit << "\n";
                            std::cerr << "trigstate: " << FpgaTrigState << "\n";
                            std::cerr << "service: " << EvtService << "\n";
                            std::cerr << "path: " << EvtPath << "\n";
                            std::cerr << "iface: " << EvtIface << "\n";
                            std::cerr << "property: " << EvtProperty << "\n";
                            std::cerr << "ptype: " << EvtPtype << "\n";
                            std::cerr << "setvalue: " << EvtSetvalue << "\n";
                            std::cerr << "type: " << type << "\n";
                        }

                        fpgainfoVector.emplace_back((uint8_t) FpgaBlock, (uint8_t) FpgaAddr, (uint8_t) FpgaBit, (uint8_t) EvtBit, (uint8_t) FpgaTrigState,
                                                    EvtService, EvtPath, EvtIface, EvtProperty, EvtPtype, EvtSetvalue);
                    }
                }

                if (sensorIfaces.find(name) == sensorIfaces.end())
                {
                    auto iface = objectServer.add_interface(
                            "/xyz/openbmc_project/FPGASensor" + std::string("/") + name,
                            "xyz.openbmc_project.Sensor.Discrete.SensorInfo");
                    iface->register_property("SensorNum", snrnum);
                    iface->register_property("SensorType", snrtype);
                    iface->initialize();

                    iface = objectServer.add_interface(
                            "/xyz/openbmc_project/FPGASensor" + std::string("/") + name,
                            "xyz.openbmc_project.Sensor.Discrete.SpecificOffset");

                    for (auto& fpgaEvt : fpgainfoVector)
                    {
                        const auto offset_property = "Offset_" + std::to_string(fpgaEvt.evtbit);
                        iface->register_property(offset_property, false,
                                                 sdbusplus::asio::PropertyPermission::readWrite);
                    }
                    iface->initialize();
                    IntrList[fpgagpio].emplace_back(snrnum, snrtype, name, en_mon, iface, fpgagpio, fpgainfoVector);
                    sensorIfaces[name] = std::move(iface);
                }
            }
        }
    }

    if (IntrList.size())
    {
        for (auto& fpgasensor : IntrList)
        {
            fpgaIntr.emplace(fpgasensor.first, fpgasensor.second);
        }

        std::cerr << "FPGA Intr" << (fpgaIntr.size() == 1 ? " is" : "s are")
                  << " parsed\n";
        std::cerr << "FPGA intr size is " << fpgaIntr.size() << "\n";

        return true;
    }

    std::cerr << "fpgaIntr is NULL\n";
    return false;
}

void detectFPGASensor(boost::asio::io_service& io,
                      std::shared_ptr<sdbusplus::asio::connection>& systemBus,
                      boost::container::flat_set<FPGAInterrupt>& fpgaIntr)
{
    for (auto& fpgasensor : fpgaIntr)
    {
        gFpgaSensors[fpgasensor.name] = std::make_unique<FPGASensor>(io, systemBus, fpgasensor);
    }
}

int main()
{
    boost::asio::io_service io;
    auto systemBus = std::make_shared<sdbusplus::asio::connection>(io);
    boost::container::flat_set<FPGAInterrupt> fpgaIntr;
    systemBus->request_name("xyz.openbmc_project.FPGASensor");
    sdbusplus::asio::object_server objectServer(systemBus);
    std::vector<std::unique_ptr<sdbusplus::bus::match::match>> matches;
    boost::asio::deadline_timer pingTimer(io);
    boost::asio::deadline_timer creationTimer(io);
    boost::asio::deadline_timer filterTimer(io);
    ManagedObjectType sensorConfigs;
    boost::container::flat_map<std::string,
                               std::shared_ptr<sdbusplus::asio::dbus_interface>>
        sensorIfaces;

    altera_fpga_init();

    if (getFpgaConfig(systemBus, fpgaIntr, sensorConfigs, objectServer, sensorIfaces))
    {
        detectFPGASensor(io, systemBus, fpgaIntr);
    }
    // callback to handle configuration change
    std::function<void(sdbusplus::message::message&)> eventHandler =
        [&](sdbusplus::message::message& message) {
            if (message.is_method_error())
            {
                std::cerr << "callback method error\n";
                return;
            }

            std::cerr << "rescan due to configuration change \n";
            if (getFpgaConfig(systemBus, fpgaIntr, sensorConfigs, objectServer, sensorIfaces))
            {
                detectFPGASensor(io, systemBus, fpgaIntr);
            }
        };

    auto match = std::make_unique<sdbusplus::bus::match::match>(
        static_cast<sdbusplus::bus::bus&>(*systemBus),
        "type='signal',member='PropertiesChanged',path_namespace='" +
        std::string(inventoryPath) + "',arg0namespace='" + configPrefix + 
        sensorType + "'", eventHandler);

    io.run();
    return 0;
}

