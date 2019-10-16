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

#include <boost/algorithm/string.hpp>
#include <boost/container/flat_map.hpp>
#include <cstring>
#include <phosphor-logging/log.hpp>
#include <sdbusplus/bus/match.hpp>
#include <iostream>
#include <nlohmann/json.hpp>



#pragma once

using namespace phosphor::logging;

// Discrete sensor types.
enum ipmi_sensor_types
{
    IPMI_SENSOR_TEMP = 0x01,
    IPMI_SENSOR_VOLTAGE = 0x02,
    IPMI_SENSOR_CURRENT = 0x03,
    IPMI_SENSOR_FAN = 0x04,
    IPMI_SENSOR_OTHER = 0x0B,
    IPMI_SENSOR_TPM = 0xCC,
};

struct CmpStrVersion
{
    bool operator()(std::string a, std::string b) const
    {
        return strverscmp(a.c_str(), b.c_str()) < 0;
    }
};

// GetSubTree returns a map where the key is a D-Bus path and the value is
// another map where the key is the D-Bus service name and the value is an array
// of interfaces implemented on the D-Bus path
using SensorSubTree = boost::container::flat_map<
    std::string,
    boost::container::flat_map<std::string, std::vector<std::string>>,
    CmpStrVersion>;

inline static bool getSensorSubtree(SensorSubTree& subtree)
{
    std::cout<<"this is func getSensorSubtree"<<std::endl;
    sdbusplus::bus::bus dbus = sdbusplus::bus::new_default_system();

    auto mapperCall =
        dbus.new_method_call("xyz.openbmc_project.ObjectMapper",
                             "/xyz/openbmc_project/object_mapper",
                             "xyz.openbmc_project.ObjectMapper", "GetSubTree");
    static constexpr const auto depth = 2;
    static constexpr std::array<const char*, 3> interfaces = {
        "xyz.openbmc_project.Sensor.Value",
        "xyz.openbmc_project.Sensor.Threshold.Warning",
        "xyz.openbmc_project.Sensor.Threshold.Critical"};
    mapperCall.append("/xyz/openbmc_project/sensors", depth, interfaces);

    try
    {
        auto mapperReply = dbus.call(mapperCall);
        std::cout<<"this is after func auto mapperReply = dbus.call(mapperCall)"<<std::endl;
        subtree.clear();
        mapperReply.read(subtree);
    }
    catch (sdbusplus::exception_t& e)
    {
        log<level::ERR>(e.what());
        return false;
    }
    return true;
}

struct CmpStr
{
    bool operator()(const char* a, const char* b) const
    {
        return std::strcmp(a, b) < 0;
    }
};

const static boost::container::flat_map<const char*, ipmi_sensor_types, CmpStr>
    sensorTypes{{{"temperature", IPMI_SENSOR_TEMP},
                 {"voltage", IPMI_SENSOR_VOLTAGE},
                 {"current", IPMI_SENSOR_CURRENT},
                 {"fan_tach", IPMI_SENSOR_FAN},
                 {"fan_pwm", IPMI_SENSOR_FAN},
                 {"power", IPMI_SENSOR_OTHER}}};



/*
inline static std::string getSensorTypeStringFromPath(const std::string& path)
{
    // get sensor type string from path, path is defined as
    // /xyz/openbmc_project//<type>/label
    size_t typeEnd = path.rfind("/");
    if (typeEnd == std::string::npos)
    {
        return path;
    }
    size_t typeStart = path.rfind("/", typeEnd - 1);
    if (typeStart == std::string::npos)
    {
        return path;
    }
    // Start at the character after the '/'
    typeStart++;
    return path.substr(typeStart, typeEnd - typeStart);
}
inline static uint8_t getSensorTypeFromPath(const std::string& path)
{
    uint8_t sensorType = 0;
    int oct;
    std::cout<<"this is in func getSensorTypeFromPath"<<std::endl;
    std::ifstream jsonFile("/var/lib/1.json");
    auto data = nlohmann::json::parse(jsonFile, nullptr, false);
    for (nlohmann::json::iterator it = data.begin(); it != data.end(); ++it)
    {
        auto tmp = it.value();
        std::string tmppath = tmp.at("path");
        if( tmppath == path )
        {
            oct = it.key();
            break;
        }
    }
    std::string type = getSensorTypeStringFromPath(path);
    std::cout<<"this is in func getSensorTypeFromPath type="<<type<<std::endl;
    auto findSensor = sensorTypes.find(type.c_str());
    if (findSensor != sensorTypes.end())
    {
        sensorType = findSensor->second;
    } // else default 0x0 RESERVED

    return sensorType;
}

inline static uint8_t getSensorNumberFromPath(const std::string& path)
{
    uint8_t sensorNum = 0xFF;
    std::string octstr;
    std::cout<<"this is in func getSensorNumberFromPath"<<std::endl;
    std::ifstream jsonFile("/var/lib/1.json");
    auto data = nlohmann::json::parse(jsonFile, nullptr, false);
    for (nlohmann::json::iterator it = data.begin(); it != data.end(); ++it)
    {
        auto tmp = it.value();
        std::string tmppath = tmp.at("path");
        if( tmppath == path )
        {
            octstr = it.key();
            break;
        }
    }
    sensorNum = fromOctStrtohex(octstr);
    std::cout<<"hex sensornumber :"<<std::hex<<(unsigned int)(sensorNum)<<std::endl;

    SensorSubTree sensorTree;
    if (!getSensorSubtree(sensorTree))
        return 0xFF;
    std::cout<<"this is in func getSensorNumberFromPath"<<std::endl;
    //uint8_t sensorNum = 0xFF;
    std::cout<<std::hex<<(unsigned int)(sensorNum)<<std::endl;
    std::cout<<"this is in func getSensorNumberFromPath,path="<<path<<std::endl;

    sensorNum = std::distance(std::begin(sensorTree), sensorTree.find(path));
    std::cout<<"this is after std::distance sensorNum="<<std::hex<<(unsigned int)(sensorNum)<<std::endl;

    return sensorNum;
}

inline static uint8_t getSensorEventTypeFromPath(const std::string& path)
{
    // TODO: Add support for additional reading types as needed
    std::cout<<"this is func getSensorEventTypeFromPath"<<std::endl;
    int oct;
    std::ifstream jsonFile("/var/lib/1.json");
    auto data = nlohmann::json::parse(jsonFile, nullptr, false);
    for (nlohmann::json::iterator it = data.begin(); it != data.end(); ++it)
    {
        auto tmp = it.value();
        std::string tmppath = tmp.at("path");
        if( tmppath == path )
        {
            std::cout<<"this is in if( tmppath == path ) tmppath="<<tmppath<<std::endl;
            oct = tmp.at("sensorReadingType");
            break;
        }
    }

    uint8_t EventType=fromOctohex(oct);
    std::cout<<"this is in func getSensorEventTypeFromPath EventType="<<std::hex<<(unsigned int)(EventType)<<std::endl;
    return EventType; // reading type = threshold
}
*/
inline static std::string getPathFromSensorNumber(uint8_t sensorNum)
{
    std::cout<<"this is func getPathFromSensorNumber"<<std::endl;

    std::string path;
    std::string sensorname;
    std::stringstream decstr;
    try {
        decstr << std::dec << (unsigned int) (sensorNum);

        std::ifstream jsonFile("/var/lib/sensor.json");
        auto data = nlohmann::json::parse(jsonFile, nullptr, false);
        for (nlohmann::json::iterator it = data.begin(); it != data.end(); ++it) {
            std::string tmp = it.key();
            if (decstr.str() == tmp) {
                path = it.value().at("path");
                break;
            }
        }
    }
    catch (const std::exception& e) {
        /* TODO: Once phosphor-logging supports unit-test injection, fix
         * this to log.
         */
        std::fprintf(stderr,
                     "Excepted building HandlerConfig from json: %s\n",
                     e.what());
    }
    std::cout<<"path="<<path<<std::endl;

    return path;
}
