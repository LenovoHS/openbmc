/*
// Copyright (c) 2017-2019 Intel Corporation
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
#include <arpa/inet.h>
#include <mapper.h>
#include <systemd/sd-bus.h>

#include <boost/algorithm/string.hpp>
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <boost/container/flat_map.hpp>
#include <boost/process.hpp>
#include "../include/commandutils.hpp"
#include <filesystem>
#include <iostream>
#include <ipmid/api.hpp>
#include <ipmid/utils.hpp>
#include <ipmid/types.hpp>
#include "../include/sdrutils.hpp"
#include "../include/selutility.hpp"
#include <ipmid/api-types.hpp>
#include <phosphor-logging/log.hpp>
#include <phosphor-logging/elog-errors.hpp>
#include <sdbusplus/message/types.hpp>
#include <sdbusplus/server.hpp>
#include <sdbusplus/timer.hpp>
#include <stdexcept>
#include "../include/storagecommands.hpp"
#include <string_view>
#include <string>
#include <variant>
#include <xyz/openbmc_project/Common/error.hpp>
#include <nlohmann/json.hpp>

using namespace phosphor::logging;
namespace ipmi::sel::erase_time
{
    static constexpr const char* selEraseTimestamp = "/var/lib/ipmi/sel_erase_time";

    void save()
    {
        // open the file, creating it if necessary
        int fd = open(selEraseTimestamp, O_WRONLY | O_CREAT | O_CLOEXEC, 0644);
        if (fd < 0)
        {
            std::cerr << "Failed to open file\n";
            return;
        }

        // update the file timestamp to the current time
        if (futimens(fd, NULL) < 0)
        {
            std::cerr << "Failed to update timestamp: "
                      << std::string(strerror(errno));
        }
        close(fd);
    }

    int get()
    {
        struct stat st;
        // default to an invalid timestamp
        int timestamp = ::ipmi::sel::invalidTimeStamp;

        int fd = open(selEraseTimestamp, O_RDWR | O_CLOEXEC, 0644);
        if (fd < 0)
        {
            return timestamp;
        }

        if (fstat(fd, &st) >= 0)
        {
            timestamp = st.st_mtime;
        }

        return timestamp;
    }
} // namespace ipmi::sel::erase_time
namespace ipmi
{

namespace storage
{

constexpr static const size_t maxMessageSize = 64;
constexpr static const size_t maxFruSdrNameSize = 16;
constexpr static const size_t maxFruNum = 56;
constexpr static const size_t cpuFruStart = 5;
constexpr static const size_t dimmFruStart = 9;
static constexpr auto InventoryService = "xyz.openbmc_project.Inventory.Manager";
static constexpr auto InventoryPath = "/xyz/openbmc_project/inventory/system/chassis/motherboard/";
static constexpr auto InventoryECCIntf = "xyz.openbmc_project.Memory.MemoryECC";
static constexpr auto CPUSensorService = "xyz.openbmc_project.CPUSensor";
static constexpr auto CPUSensorPath = "/xyz/openbmc_project/inventory/system/chassis/motherboard/";
static constexpr auto CPUSensorIntf ="xyz.openbmc_project.Inventory.Item.Cpu";
static constexpr auto OEMSensorService = "xyz.openbmc_project.OEMSensor";
static constexpr auto OEMSensorPath = "/xyz/openbmc_project/OEMSensor/";
static constexpr auto OEMSensorIntf = "xyz.openbmc_project.Sensor.Discrete.SpecificOffset";

using ManagedObjectType = boost::container::flat_map<
    sdbusplus::message::object_path,
    boost::container::flat_map<
        std::string, boost::container::flat_map<std::string, DbusVariant>>>;
using ManagedEntry = std::pair<
    sdbusplus::message::object_path,
    boost::container::flat_map<
        std::string, boost::container::flat_map<std::string, DbusVariant>>>;

constexpr static const char* fruDeviceServiceName =
    "xyz.openbmc_project.FruDevice";
constexpr static const size_t cacheTimeoutSeconds = 10;

static std::vector<uint8_t> fruCache;
static uint8_t cacheBus = 0xFF;
static uint8_t cacheAddr = 0XFF;

std::unique_ptr<phosphor::Timer> cacheTimer = nullptr;

// we unfortunately have to build a map of hashes in case there is a
// collision to verify our dev-id
boost::container::flat_map<uint8_t, std::pair<uint8_t, uint8_t>> deviceHashes;

void registerStorageFunctions() __attribute__((constructor));

bool writeFru()
{
    std::shared_ptr<sdbusplus::asio::connection> dbus = getSdBus();
    sdbusplus::message::message writeFru = dbus->new_method_call(
        fruDeviceServiceName, "/xyz/openbmc_project/FruDevice",
        "xyz.openbmc_project.FruDeviceManager", "WriteFru");
    writeFru.append(cacheBus, cacheAddr, fruCache);
    try
    {
        sdbusplus::message::message writeFruResp = dbus->call(writeFru);
    }
    catch (sdbusplus::exception_t&)
    {
        // todo: log sel?
        phosphor::logging::log<phosphor::logging::level::ERR>(
            "error writing fru");
        return false;
    }
    return true;
}

void createTimer()
{
    if (cacheTimer == nullptr)
    {
        cacheTimer = std::make_unique<phosphor::Timer>(writeFru);
    }
}

bool searchFruId(uint8_t& fruHash, uint8_t& devId, const std::string& path)
{
    std::size_t found = path.find("HR650");
    if (found != std::string::npos) 
    {
        fruHash = 0;
        return true;
    }

    return false;
}

bool writeCpuDimmFru(uint8_t devId) 
{
    char fruFilename[32] = {0};
    const char* mode = NULL;
    FILE* fp = NULL;

    if ((devId >= cpuFruStart) && (devId < dimmFruStart))
    {
        std::sprintf(fruFilename, CpuFruPath, (devId - cpuFruStart));
    }
    else
    {
        std::sprintf(fruFilename, DimmFruPath, (devId - dimmFruStart));
    }

    if (access(fruFilename, F_OK) == -1)
    {
        mode = "wb+";
    }
    else
    {
        mode = "rb+";
    }

    if ((fp = std::fopen(fruFilename, mode)) != NULL)
    {
        if (std::fwrite(fruCache.data(), fruCache.size(), 1, fp) != 1)
        {
            phosphor::logging::log<phosphor::logging::level::ERR>(
                "Write into fru file failed",
                phosphor::logging::entry("FILE=%s", fruFilename),
                phosphor::logging::entry("ERRNO=%s", std::strerror(errno)));

            std::fclose(fp);
            return false;
        }
        std::fclose(fp);
    }
    else
    {
        phosphor::logging::log<phosphor::logging::level::ERR>(
            "Error trying to open fru file for write action",
            phosphor::logging::entry("FILE=%s", fruFilename));
        return false;
    }

    return true;
}

ipmi_ret_t replaceCpuDimmCacheFru(uint8_t devId)
{
    char fruFilename[32] = {0};
    const char* mode = NULL;
    FILE* fp = NULL;
    int fileLen = 0;
    ipmi_ret_t rc = IPMI_CC_INVALID;

    if ((devId >= cpuFruStart) && (devId < dimmFruStart))
    {
        std::sprintf(fruFilename, CpuFruPath, (devId - cpuFruStart));
    }
    else
    {
        std::sprintf(fruFilename, DimmFruPath, (devId - dimmFruStart));
    }

    if (access(fruFilename, F_OK) == -1)
    {
        mode = "wb+";
    }
    else
    {
        mode = "rb+";
    }

    if ((fp = std::fopen(fruFilename, mode)) != NULL)
    {
        if (std::fseek(fp, 0, SEEK_END))
        {
            phosphor::logging::log<phosphor::logging::level::ERR>(
                "Seek(End) into fru file failed",
                phosphor::logging::entry("FILE=%s", fruFilename),
                phosphor::logging::entry("ERRNO=%s", std::strerror(errno)));

            std::fclose(fp);
            return rc;
        }

        fileLen = ftell(fp);
        if (fileLen < 0) 
        {
           phosphor::logging::log<phosphor::logging::level::ERR>(
                "Check fru file size failed",
                phosphor::logging::entry("FILE=%s", fruFilename),
                phosphor::logging::entry("ERRNO=%s", std::strerror(errno)));

            std::fclose(fp);
            return rc;
        }

        if (std::fseek(fp, 0, SEEK_SET))
        {
            phosphor::logging::log<phosphor::logging::level::ERR>(
                "Seek(Begin) into fru file failed",
                phosphor::logging::entry("FILE=%s", fruFilename),
                phosphor::logging::entry("ERRNO=%s", std::strerror(errno)));

            std::fclose(fp);
            return rc;
        }

        fruCache.clear();
        fruCache.resize(fileLen);

        if (std::fread(fruCache.data(), fileLen, 1, fp) != 1)
        {
            phosphor::logging::log<phosphor::logging::level::ERR>(
                "Read fru file failed",
                phosphor::logging::entry("FILE=%s", fruFilename),
                phosphor::logging::entry("ERRNO=%s", std::strerror(errno)));

            std::fclose(fp);
            return rc;
        }
        std::fclose(fp);
    }
    else
    {
        phosphor::logging::log<phosphor::logging::level::ERR>(
            "Error trying to open fru file for read action",
            phosphor::logging::entry("FILE=%s", fruFilename));
        return rc;
    }

    rc = IPMI_CC_OK;
    return rc;
}

ipmi_ret_t replaceCacheFru(uint8_t devId)
{
    static uint8_t lastDevId = 0xFF;

    if (devId > maxFruNum)
    {
        return IPMI_CC_SENSOR_INVALID;
    }

    if ((devId >= cpuFruStart) && (devId <= maxFruNum))
    {
        ipmi_ret_t status = replaceCpuDimmCacheFru(devId);
        return status;
    }

    bool timerRunning = (cacheTimer != nullptr) && !cacheTimer->isExpired();
    if (lastDevId == devId && timerRunning)
    {
        return IPMI_CC_OK; // cache already up to date
    }
    // if timer is running, stop it and writeFru manually
    else if (timerRunning)
    {
        cacheTimer->stop();
        writeFru();
    }

    std::shared_ptr<sdbusplus::asio::connection> dbus = getSdBus();
    sdbusplus::message::message getObjects = dbus->new_method_call(
        fruDeviceServiceName, "/", "org.freedesktop.DBus.ObjectManager",
        "GetManagedObjects");
    ManagedObjectType frus;
    try
    {
        sdbusplus::message::message resp = dbus->call(getObjects);
        resp.read(frus);
    }
    catch (sdbusplus::exception_t&)
    {
        phosphor::logging::log<phosphor::logging::level::ERR>(
            "replaceCacheFru: error getting managed objects");
        return IPMI_CC_RESPONSE_ERROR;
    }

    deviceHashes.clear();

    // hash the object paths to create unique device id's. increment on
    // collision
    //std::hash<std::string> hasher;
    for (const auto& fru : frus)
    {
        auto fruIface = fru.second.find("xyz.openbmc_project.FruDevice");
        if (fruIface == fru.second.end())
        {
            continue;
        }

        auto busFind = fruIface->second.find("BUS");
        auto addrFind = fruIface->second.find("ADDRESS");
        if (busFind == fruIface->second.end() ||
            addrFind == fruIface->second.end())
        {
            phosphor::logging::log<phosphor::logging::level::INFO>(
                "fru device missing Bus or Address",
                phosphor::logging::entry("FRU=%s", fru.first.str.c_str()));
            continue;
        }

        uint8_t fruBus = std::get<uint32_t>(busFind->second);
        uint8_t fruAddr = std::get<uint32_t>(addrFind->second);

        uint8_t fruHash = 0;
        if (!searchFruId(fruHash, devId, fru.first.str))
        {
            continue;
        }
        /*
        if (fruBus != 0 || fruAddr != 0)
        {
            fruHash = hasher(fru.first.str);
            // can't be 0xFF based on spec, and 0 is reserved for baseboard
            if (fruHash == 0 || fruHash == 0xFF)
            {
                fruHash = 1;
            }
        }*/
        std::pair<uint8_t, uint8_t> newDev(fruBus, fruAddr);
        bool emplacePassed = false;
        auto resp = deviceHashes.emplace(fruHash, newDev);
        emplacePassed = resp.second;
        if (!emplacePassed)
        {
            continue;
        }
        /*
        bool emplacePassed = false;
        while (!emplacePassed)
        {
            auto resp = deviceHashes.emplace(fruHash, newDev);
            emplacePassed = resp.second;
            if (!emplacePassed)
            {
                fruHash++;
                // can't be 0xFF based on spec, and 0 is reserved for
                // baseboard
                if (fruHash == 0XFF)
                {
                    fruHash = 0x1;
                }
            }
        }*/
    }
    auto deviceFind = deviceHashes.find(devId);
    if (deviceFind == deviceHashes.end())
    {
        return IPMI_CC_SENSOR_INVALID;
    }

    fruCache.clear();
    sdbusplus::message::message getRawFru = dbus->new_method_call(
        fruDeviceServiceName, "/xyz/openbmc_project/FruDevice",
        "xyz.openbmc_project.FruDeviceManager", "GetRawFru");
    cacheBus = deviceFind->second.first;
    cacheAddr = deviceFind->second.second;
    getRawFru.append(cacheBus, cacheAddr);
    try
    {
        sdbusplus::message::message getRawResp = dbus->call(getRawFru);
        getRawResp.read(fruCache);
    }
    catch (sdbusplus::exception_t&)
    {
        lastDevId = 0xFF;
        cacheBus = 0xFF;
        cacheAddr = 0xFF;
        return IPMI_CC_RESPONSE_ERROR;
    }

    lastDevId = devId;
    return IPMI_CC_OK;
}

ipmi_ret_t ipmiStorageReadFRUData(ipmi_netfn_t netfn, ipmi_cmd_t cmd,
                                  ipmi_request_t request,
                                  ipmi_response_t response,
                                  ipmi_data_len_t dataLen,
                                  ipmi_context_t context)
{
    if (*dataLen != 4)
    {
        *dataLen = 0;
        return IPMI_CC_REQ_DATA_LEN_INVALID;
    }
    *dataLen = 0; // default to 0 in case of an error

    auto req = static_cast<GetFRUAreaReq*>(request);

    if (req->countToRead > maxMessageSize - 1)
    {
        return IPMI_CC_INVALID_FIELD_REQUEST;
    }
    ipmi_ret_t status = replaceCacheFru(req->fruDeviceID);

    if (status != IPMI_CC_OK)
    {
        return status;
    }

    size_t fromFRUByteLen = 0;
    if (req->countToRead + req->fruInventoryOffset < fruCache.size())
    {
        fromFRUByteLen = req->countToRead;
    }
    else if (fruCache.size() > req->fruInventoryOffset)
    {
        fromFRUByteLen = fruCache.size() - req->fruInventoryOffset;
    }
    size_t padByteLen = req->countToRead - fromFRUByteLen;
    uint8_t* respPtr = static_cast<uint8_t*>(response);
    *respPtr = req->countToRead;
    std::copy(fruCache.begin() + req->fruInventoryOffset,
              fruCache.begin() + req->fruInventoryOffset + fromFRUByteLen,
              ++respPtr);
    // if longer than the fru is requested, fill with 0xFF
    if (padByteLen)
    {
        respPtr += fromFRUByteLen;
        std::fill(respPtr, respPtr + padByteLen, 0xFF);
    }
    *dataLen = fromFRUByteLen + 1;

    return IPMI_CC_OK;
}

ipmi_ret_t ipmiStorageWriteFRUData(ipmi_netfn_t netfn, ipmi_cmd_t cmd,
                                   ipmi_request_t request,
                                   ipmi_response_t response,
                                   ipmi_data_len_t dataLen,
                                   ipmi_context_t context)
{
    if (*dataLen < 4 ||
        *dataLen >=
            0xFF + 3) // count written return is one byte, so limit to one byte
                      // of data after the three request data bytes
    {
        *dataLen = 0;
        return IPMI_CC_REQ_DATA_LEN_INVALID;
    }

    auto req = static_cast<WriteFRUDataReq*>(request);
    size_t writeLen = *dataLen - 3;
    *dataLen = 0; // default to 0 in case of an error

    ipmi_ret_t status = replaceCacheFru(req->fruDeviceID);
    if (status != IPMI_CC_OK)
    {
        return status;
    }
    size_t lastWriteAddr = req->fruInventoryOffset + writeLen;
    if (fruCache.size() < lastWriteAddr)
    {
        fruCache.resize(req->fruInventoryOffset + writeLen);
    }

    std::copy(req->data, req->data + writeLen,
              fruCache.begin() + req->fruInventoryOffset);

    bool atEnd = false;

    if (fruCache.size() >= sizeof(FRUHeader))
    {

        FRUHeader* header = reinterpret_cast<FRUHeader*>(fruCache.data());

        size_t lastRecordStart = std::max(
            header->internalOffset,
            std::max(header->chassisOffset,
                     std::max(header->boardOffset, header->productOffset)));
        // TODO: Handle Multi-Record FRUs?

        lastRecordStart *= 8; // header starts in are multiples of 8 bytes

        // get the length of the area in multiples of 8 bytes
        if (lastWriteAddr > (lastRecordStart + 1))
        {
            // second byte in record area is the length
            int areaLength(fruCache[lastRecordStart + 1]);
            areaLength *= 8; // it is in multiples of 8 bytes

            if (lastWriteAddr >= (areaLength + lastRecordStart))
            {
                atEnd = true;
            }
        }
    }
    uint8_t* respPtr = static_cast<uint8_t*>(response);

    if ((req->fruDeviceID >= cpuFruStart) && (req->fruDeviceID <= maxFruNum))
    {
        if (!writeCpuDimmFru(req->fruDeviceID))
        {
            return IPMI_CC_INVALID_FIELD_REQUEST;
        }
        *respPtr = writeLen;
        *dataLen = 1;

        return IPMI_CC_OK;
    }

    if (atEnd)
    {
        // cancel timer, we're at the end so might as well send it
        cacheTimer->stop();
        if (!writeFru())
        {
            return IPMI_CC_INVALID_FIELD_REQUEST;
        }
        *respPtr = std::min(fruCache.size(), static_cast<size_t>(0xFF));
    }
    else
    {
        // start a timer, if no further data is sent in cacheTimeoutSeconds
        // seconds, check to see if it is valid
        createTimer();
        cacheTimer->start(std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::seconds(cacheTimeoutSeconds)));
        *respPtr = 0;
    }

    *dataLen = 1;

    return IPMI_CC_OK;
}

/** @brief implements the get FRU inventory area info command
 *  @param fruDeviceId  - FRU Device ID
 *
 *  @returns IPMI completion code plus response data
 *   - inventorySize - Number of possible allocation units
 *   - accessType    - Allocation unit size in bytes.
 */
ipmi::RspType<uint16_t, // inventorySize
              uint8_t>  // accessType
    ipmiStorageGetFruInvAreaInfo(uint8_t fruDeviceId)
{
    if (fruDeviceId == 0xFF)
    {
        return ipmi::responseInvalidFieldRequest();
    }

    ipmi::Cc status = replaceCacheFru(fruDeviceId);

    if (status != IPMI_CC_OK)
    {
        return ipmi::response(status);
    }

    constexpr uint8_t accessType =
        static_cast<uint8_t>(GetFRUAreaAccessType::byte);

    return ipmi::responseSuccess(fruCache.size(), accessType);
}

//standard sel log
uint8_t LenovoSetSensorProperty(uint8_t sensorNumber,
                                std::vector<uint8_t> eventData)
{
    sdbusplus::bus::bus bus(ipmid_get_sd_bus_connection());
    std::string event_state;
    std::string PropertyPath;
    if (sensorNumber >= 0xA0 && sensorNumber <= 0xCF)
    {
        char index[] = { 'A', 'B', 'C', 'D', 'E', 'F' };
        const auto name = "CPU" + std::to_string((sensorNumber - 0xA0)/12) +
                          "_DIMM" + index[((sensorNumber - 0xA0)%12)/2] +
                          std::to_string((sensorNumber - 0xA0)%2 + 1);
        try
        {
            switch (eventData[0])
            {
                case 0x00:
                    event_state = "xyz.openbmc_project.Memory.MemoryECC.ECCStatus.CE";
                    PropertyPath = "state";
                    ipmi::setDbusProperty(bus, InventoryService,
                                          InventoryPath + name, InventoryECCIntf,
                                          PropertyPath, event_state);
                    break;
                case 0x01:
                    event_state = "xyz.openbmc_project.Memory.MemoryECC.ECCStatus.UE";
                    PropertyPath = "state";
                    ipmi::setDbusProperty(bus, InventoryService,
                                          InventoryPath + name, InventoryECCIntf,
                                          PropertyPath, event_state);
                    break;
                case 0x05:
                    PropertyPath = "isLoggingLimitReached";
                    ipmi::setDbusProperty(bus, InventoryService,
                                          InventoryPath + name, InventoryECCIntf,
                                          PropertyPath, true);
                    break;
                default:
                    return 1;
                    break;
            }
        }
        catch (std::exception& e)
        {
            log<level::ERR>(e.what());
            return 1;
        }
    }
    else if (sensorNumber >= 0x91 && sensorNumber <= 0x94)
    {
        const auto name = "CPU" + std::to_string(sensorNumber - 0x91) + "_Status";
        try
        {
            switch (eventData[0])
            {
                case 0x05:
                    PropertyPath = "Configuration_Error";
                    ipmi::setDbusProperty(bus, CPUSensorService,
                                          CPUSensorPath + name, CPUSensorIntf,
                                          PropertyPath, true);
                    break;
                default:
                    return 1;
                    break;
            }
        }
        catch (std::exception& e)
        {
            log<level::ERR>(e.what());
            return 1;
        }
    }
    else if (sensorNumber >= 0xD0 && sensorNumber <= 0xDB)
    {
        const auto name = "PE" + std::to_string((sensorNumber - 0xD0)/5 + 1) +
                          "S" + std::to_string((sensorNumber - 0xD0)%5 + 1) +
                          "_Bus_Status";

        try
        {
            switch (eventData[0])
            {
                case 0x04:
                    PropertyPath = "Offset_4";
                    ipmi::setDbusProperty(bus, OEMSensorService,
                                          OEMSensorPath + name, OEMSensorIntf,
                                          PropertyPath, true);
                    break;
                case 0x05:
                    PropertyPath = "Offset_5";
                    ipmi::setDbusProperty(bus, OEMSensorService,
                                          OEMSensorPath + name, OEMSensorIntf,
                                          PropertyPath, true);
                    break;
                case 0x07:
                    PropertyPath = "Offset_7";
                    ipmi::setDbusProperty(bus, OEMSensorService,
                                          OEMSensorPath + name, OEMSensorIntf,
                                          PropertyPath, true);
                    break;
                case 0x08:
                    PropertyPath = "Offset_8";
                    ipmi::setDbusProperty(bus, OEMSensorService,
                                          OEMSensorPath + name, OEMSensorIntf,
                                          PropertyPath, true);
                    break;
                case 0x0A:
                    PropertyPath = "Offset_10";
                    ipmi::setDbusProperty(bus, OEMSensorService,
                                          OEMSensorPath + name, OEMSensorIntf,
                                          PropertyPath, true);
                    break;
                case 0x0B:
                    PropertyPath = "Offset_11";
                    ipmi::setDbusProperty(bus, OEMSensorService,
                                          OEMSensorPath + name, OEMSensorIntf,
                                          PropertyPath, true);
                    break;
                case 0x0F:
                    PropertyPath = "Offset_15";
                    ipmi::setDbusProperty(bus, OEMSensorService,
                                          OEMSensorPath + name, OEMSensorIntf,
                                          PropertyPath, true);
                    break;
                default:
                    return 1;
                    break;
            }
        }
        catch (std::exception& e)
        {
            log<level::ERR>(e.what());
            return 1;
        }
    }
    else if (sensorNumber == 0xF9)
    {
        std::string name = "UPI_Error";
        try
        {
            switch (eventData[0])
            {
                case 0x05:
                    PropertyPath = "Offset_5";
                    ipmi::setDbusProperty(bus, OEMSensorService,
                                          OEMSensorPath + name, OEMSensorIntf,
                                          PropertyPath, true);
                    break;
                default:
                    return 1;
                    break;
            }
        }
        catch (std::exception& e)
        {
            log<level::ERR>(e.what());
            return 1;
        }
    }
    else if (sensorNumber == 0xFC)
    {
        std::string name = "NVRAM_Corrupt";
        try
        {
            switch (eventData[0])
            {
                case 0x00:
                    PropertyPath = "Offset_0";
                    ipmi::setDbusProperty(bus, OEMSensorService,
                                          OEMSensorPath + name, OEMSensorIntf,
                                          PropertyPath, true);
                    break;
                default:
                    return 1;
                    break;
            }
        }
        catch (std::exception& e)
        {
            log<level::ERR>(e.what());
            return 1;
        }
    }
    else if (sensorNumber == 0xFD)
    {
        std::string name = "CPU_Exception";
        try
        {
            switch (eventData[0])
            {
                case 0x01:
                    PropertyPath = "Offset_1";
                    ipmi::setDbusProperty(bus, OEMSensorService,
                                          OEMSensorPath + name, OEMSensorIntf,
                                          PropertyPath, true);
                    break;
                default:
                    return 1;
                    break;
            }
        }
        catch (std::exception& e)
        {
            log<level::ERR>(e.what());
            return 1;
        }
    }

    return 0;
}

ipmi_ret_t getSELInfo(ipmi_netfn_t netfn, ipmi_cmd_t cmd,
                      ipmi_request_t request, ipmi_response_t response,
                      ipmi_data_len_t data_len, ipmi_context_t context)
{
    if (*data_len != 0)
    {
        *data_len = 0;
        return IPMI_CC_REQ_DATA_LEN_INVALID;
    }
    ipmi::sel::GetSELInfoResponse* responseData =
            static_cast<ipmi::sel::GetSELInfoResponse*>(response);

    responseData->selVersion = ipmi::sel::selVersion;
    responseData->addTimeStamp = ipmi::sel::invalidTimeStamp;
    responseData->operationSupport = ipmi::sel::selOperationSupport;
    responseData->entries = 0;

    // Fill in the last erase time
    responseData->eraseTimeStamp = ipmi::sel::erase_time::get();

    // Open the journal
    sd_journal* journalTmp = nullptr;
    if (int ret = sd_journal_open(&journalTmp, SD_JOURNAL_LOCAL_ONLY); ret < 0)
    {
        log<level::ERR>("Failed to open journal: ",
                        entry("ERRNO=%s", strerror(-ret)));
        return IPMI_CC_RESPONSE_ERROR;
    }
    std::unique_ptr<sd_journal, decltype(&sd_journal_close)> journal(
            journalTmp, sd_journal_close);
    journalTmp = nullptr;

    // Filter the journal based on the SEL MESSAGE_ID
    std::string match = "MESSAGE_ID=" + std::string(ipmi::sel::selMessageId);
    sd_journal_add_match(journal.get(), match.c_str(), 0);

    // Count the number of SEL Entries in the journal and get the timestamp of
    // the newest entry
    bool timestampRecorded = false;
    SD_JOURNAL_FOREACH_BACKWARDS(journal.get())
    {
        if (!timestampRecorded)
        {
            uint64_t timestamp;
            if (int ret =
                        sd_journal_get_realtime_usec(journal.get(), &timestamp);
                    ret < 0)
            {
                log<level::ERR>("Failed to read timestamp: ",
                                entry("ERRNO=%s", strerror(-ret)));
                return IPMI_CC_RESPONSE_ERROR;
            }
            timestamp /= (1000 * 1000); // convert from us to s
            responseData->addTimeStamp = static_cast<uint32_t>(timestamp);
            timestampRecorded = true;
        }
        responseData->entries++;
    }

    *data_len = sizeof(ipmi::sel::GetSELInfoResponse);
    return IPMI_CC_OK;
}

static int fromHexStr(const std::string hexStr, std::vector<uint8_t>& data)
{
    for (unsigned int i = 0; i < hexStr.size(); i += 2)
    {
        try
        {
            data.push_back(static_cast<uint8_t>(
                                   std::stoul(hexStr.substr(i, 2), nullptr, 16)));
        }
        catch (std::invalid_argument& e)
        {
            log<level::ERR>(e.what());
            return -1;
        }
        catch (std::out_of_range& e)
        {
            log<level::ERR>(e.what());
            return -1;
        }
    }
    return 0;
}
static uint8_t fromDecStrtohex(const std::string decStr)
{
    unsigned  int dec =std::stoul(decStr, nullptr, 10);
    std::stringstream hexstr;
    hexstr<<std::hex<<dec;
    uint8_t hex=static_cast<uint8_t>(std::stoul(hexstr.str(), nullptr, 16));
    return hex;
}

static uint8_t fromDectohex(const int dec)
{
    std::stringstream hexstr;
    hexstr<<std::hex<<dec;
    uint8_t hex=static_cast<uint8_t>(std::stoul(hexstr.str(), nullptr, 16));
    return hex;
}
static int getJournalMetadata(sd_journal* journal,
                              const std::string_view& field,
                              std::string& contents)
{
    const char* data = nullptr;
    size_t length = 0;

    // Get the metadata from the requested field of the journal entry
    if (int ret = sd_journal_get_data(journal, field.data(),
                                      (const void**)&data, &length);
            ret < 0)
    {
        return ret;
    }
    std::string_view metadata(data, length);
    // Only use the content after the "=" character.
    metadata.remove_prefix(std::min(metadata.find("=") + 1, metadata.size()));
    contents = std::string(metadata);

    return 0;
}

static int getJournalMetadata(sd_journal* journal,
                              const std::string_view& field, const int& base,
                              int& contents)
{
    std::string metadata;
    // Get the metadata from the requested field of the journal entry
    if (int ret = getJournalMetadata(journal, field, metadata); ret < 0)
    {
        return ret;
    }
    try
    {
        contents = static_cast<int>(std::stoul(metadata, nullptr, base));
    }
    catch (std::invalid_argument& e)
    {
        log<level::ERR>(e.what());
        return -1;
    }
    catch (std::out_of_range& e)
    {
        log<level::ERR>(e.what());
        return -1;
    }
    return 0;
}

static int getJournalSelData(sd_journal* journal, std::vector<uint8_t>& evtData)
{
    std::string evtDataStr;
    // Get the OEM data from the IPMI_SEL_DATA field
    if (int ret = getJournalMetadata(journal, "IPMI_SEL_DATA", evtDataStr);
            ret < 0)
    {
        return ret;
    }
    return fromHexStr(evtDataStr, evtData);
}

ipmi_ret_t getSELEntry(ipmi_netfn_t netfn, ipmi_cmd_t cmd,
                       ipmi_request_t request, ipmi_response_t response,
                       ipmi_data_len_t data_len, ipmi_context_t context)
{

    if (*data_len != sizeof(ipmi::sel::GetSELEntryRequest))
    {
        *data_len = 0;
        return IPMI_CC_REQ_DATA_LEN_INVALID;
    }
    *data_len = 0; // Default to 0 in case of errors
    auto requestData =
            static_cast<const ipmi::sel::GetSELEntryRequest*>(request);

    if (requestData->reservationID != 0 || requestData->offset != 0)
    {
        if (!checkSELReservation(requestData->reservationID))
        {
            return IPMI_CC_INVALID_RESERVATION_ID;
        }
    }

    // Check for the requested SEL Entry.
    sd_journal* journalTmp;
    if (int ret = sd_journal_open(&journalTmp, SD_JOURNAL_LOCAL_ONLY); ret < 0)
    {
        log<level::ERR>("Failed to open journal: ",
                        entry("ERRNO=%s", strerror(-ret)));
        return IPMI_CC_UNSPECIFIED_ERROR;
    }
    std::unique_ptr<sd_journal, decltype(&sd_journal_close)> journal(
            journalTmp, sd_journal_close);
    journalTmp = nullptr;

    std::string match = "MESSAGE_ID=" + std::string(ipmi::sel::selMessageId);

    sd_journal_add_match(journal.get(), match.c_str(), 0);

    // Get the requested target SEL record ID if first or last is requested.
    int targetID = requestData->selRecordID;
    if (targetID == ipmi::sel::firstEntry)
    {
        SD_JOURNAL_FOREACH(journal.get())
        {
            // Get the record ID from the IPMI_SEL_RECORD_ID field of the first
            // entry
            if (getJournalMetadata(journal.get(), "IPMI_SEL_RECORD_ID", 10,
                                   targetID) < 0)
            {
                return IPMI_CC_UNSPECIFIED_ERROR;
            }
            break;
        }
    }
    else if (targetID == ipmi::sel::lastEntry)
    {
        SD_JOURNAL_FOREACH_BACKWARDS(journal.get())
        {
            // Get the record ID from the IPMI_SEL_RECORD_ID field of the first
            // entry
            if (getJournalMetadata(journal.get(), "IPMI_SEL_RECORD_ID", 10,
                                   targetID) < 0)
            {
                return IPMI_CC_UNSPECIFIED_ERROR;
            }
            break;
        }
    }
    // Find the requested ID
    match = "IPMI_SEL_RECORD_ID=" + std::to_string(targetID);

    sd_journal_add_match(journal.get(), match.c_str(), 0);
    // And find the next ID (wrapping to Record ID 1 when necessary)
    int nextID = targetID + 1;
    if (nextID == ipmi::sel::lastEntry)
    {
        nextID = 1;
    }
    match = "IPMI_SEL_RECORD_ID=" + std::to_string(nextID);
    sd_journal_add_match(journal.get(), match.c_str(), 0);
    SD_JOURNAL_FOREACH(journal.get())
    {
        // Get the record ID from the IPMI_SEL_RECORD_ID field
        int id = 0;
        if (getJournalMetadata(journal.get(), "IPMI_SEL_RECORD_ID", 10, id) < 0)
        {
            return IPMI_CC_UNSPECIFIED_ERROR;
        }
        if (id == targetID)
        {
            // Found the desired record, so fill in the data
            int recordType = 0;
            // Get the record type from the IPMI_SEL_RECORD_TYPE field
            if (getJournalMetadata(journal.get(), "IPMI_SEL_RECORD_TYPE", 16,
                                   recordType) < 0)
            {

                return IPMI_CC_UNSPECIFIED_ERROR;
            }
            // The record content depends on the record type
            if (recordType == ipmi::sel::systemEvent)
            {
                ipmi::sel::GetSELEntryResponse* record =
                        static_cast<ipmi::sel::GetSELEntryResponse*>(response);

                // Set the record ID
                record->recordID = id;

                // Set the record type
                record->recordType = recordType;

                // Get the timestamp
                uint64_t ts = 0;
                if (sd_journal_get_realtime_usec(journal.get(), &ts) < 0)
                {
                    return IPMI_CC_UNSPECIFIED_ERROR;
                }
                record->timeStamp = static_cast<uint32_t>(
                        ts / 1000 / 1000); // Convert from us to s

                int generatorID = 0;
                // Get the generator ID from the IPMI_SEL_GENERATOR_ID field
                if (getJournalMetadata(journal.get(), "IPMI_SEL_GENERATOR_ID",
                                       16, generatorID) < 0)
                {
                    return IPMI_CC_UNSPECIFIED_ERROR;
                }
                record->generatorID = generatorID;

                // Set the event message revision
                record->eventMsgRevision = ipmi::sel::eventMsgRev;

                std::string path;
                // Get the IPMI_SEL_SENSOR_PATH field
                if (getJournalMetadata(journal.get(), "IPMI_SEL_SENSOR_PATH",
                                       path) < 0)
                {
                    return IPMI_CC_UNSPECIFIED_ERROR;
                }

                try {
                    std::string sensornumdecstr;
                    int sensorstypedec;
                    int enventtypedec;
                    std::ifstream jsonFile("/var/lib/sensor.json");
                    auto data = nlohmann::json::parse(jsonFile, nullptr, false);
                    for (nlohmann::json::iterator it = data.begin(); it != data.end(); ++it) {
                        auto tmp = it.value();
                        std::string tmppath = tmp.at("path");
                        if (tmppath == path) {
                            sensornumdecstr = it.key();
                            enventtypedec = tmp.at("sensorReadingType");
                            sensorstypedec = tmp.at("sensorType");

                            break;
                        }
                    }
                    record->sensorType = fromDectohex(sensorstypedec);
                    record->sensorNum = fromDecStrtohex(sensornumdecstr);
                    record->eventType = fromDectohex(enventtypedec);
                }
                catch (const std::exception& e) {
                    /* TODO: Once phosphor-logging supports unit-test injection, fix
                     * this to log.
                     */
                    std::fprintf(stderr,
                                 "Excepted building HandlerConfig from json: %s\n",
                                 e.what());
                }

                int eventDir = 0;
                // Get the event direction from the IPMI_SEL_EVENT_DIR field
                if (getJournalMetadata(journal.get(), "IPMI_SEL_EVENT_DIR", 16,
                                       eventDir) < 0)
                {
                    return IPMI_CC_UNSPECIFIED_ERROR;
                }
                // Set the event direction
                if (eventDir == 0)
                {
                    record->eventType |= ipmi::sel::deassertionEvent;
                }

                std::vector<uint8_t> evtData;
                // Get the event data from the IPMI_SEL_DATA field
                if (getJournalSelData(journal.get(), evtData) < 0)
                {
                    return IPMI_CC_UNSPECIFIED_ERROR;
                }
                record->eventData1 = evtData.size() > 0
                                     ? evtData[0]
                                     : ipmi::sel::unspecifiedEventData;
                record->eventData2 = evtData.size() > 1
                                     ? evtData[1]
                                     : ipmi::sel::unspecifiedEventData;
                record->eventData3 = evtData.size() > 2
                                     ? evtData[2]
                                     : ipmi::sel::unspecifiedEventData;
            }
            else if (recordType >= ipmi::sel::oemTsEventFirst &&
                     recordType <= ipmi::sel::oemTsEventLast)
            {

                ipmi::sel::GetSELEntryResponseOEMTimestamped* oemTsRecord =
                        static_cast<ipmi::sel::GetSELEntryResponseOEMTimestamped*>(
                                response);

                // Set the record ID
                oemTsRecord->recordID = id;

                // Set the record type
                oemTsRecord->recordType = recordType;

                // Get the timestamp
                uint64_t timestamp = 0;
                if (sd_journal_get_realtime_usec(journal.get(), &timestamp) < 0)
                {
                    return IPMI_CC_UNSPECIFIED_ERROR;
                }
                oemTsRecord->timestamp = static_cast<uint32_t>(
                        timestamp / 1000 / 1000); // Convert from us to s

                std::vector<uint8_t> evtData;
                // Get the OEM data from the IPMI_SEL_DATA field
                if (getJournalSelData(journal.get(), evtData) < 0)
                {
                    return IPMI_CC_UNSPECIFIED_ERROR;
                }
                // Only keep the bytes that fit in the record
                std::copy_n(evtData.begin(),
                            std::min(evtData.size(), ipmi::sel::oemTsEventSize),
                            oemTsRecord->eventData);
            }
            else if (recordType >= ipmi::sel::oemEventFirst &&
                     recordType <= ipmi::sel::oemEventLast)
            {
                ipmi::sel::GetSELEntryResponseOEM* oemRecord =
                        static_cast<ipmi::sel::GetSELEntryResponseOEM*>(response);

                // Set the record ID
                oemRecord->recordID = id;

                // Set the record type
                oemRecord->recordType = recordType;

                std::vector<uint8_t> evtData;
                // Get the OEM data from the IPMI_SEL_DATA field
                if (getJournalSelData(journal.get(), evtData) < 0)
                {
                    return IPMI_CC_UNSPECIFIED_ERROR;
                }
                // Only keep the bytes that fit in the record
                std::copy_n(evtData.begin(),
                            std::min(evtData.size(), ipmi::sel::oemEventSize),
                            oemRecord->eventData);
            }
        }
        else if (id == nextID)
        {
            ipmi::sel::GetSELEntryResponse* record =
                    static_cast<ipmi::sel::GetSELEntryResponse*>(response);
            record->nextRecordID = id;
        }
    }

    ipmi::sel::GetSELEntryResponse* record =
            static_cast<ipmi::sel::GetSELEntryResponse*>(response);

    // If we didn't find the requested record, return an error
    if (record->recordID == 0)
    {
        return IPMI_CC_SENSOR_INVALID;
    }

    // If we didn't find the next record ID, then mark it as the last entry
    if (record->nextRecordID == 0)
    {
        record->nextRecordID = ipmi::sel::lastEntry;
    }

    *data_len = sizeof(ipmi::sel::GetSELEntryResponse);
    if (requestData->readLength != ipmi::sel::entireRecord)
    {
        if (requestData->offset + requestData->readLength >
            ipmi::sel::selRecordSize)
        {
            return IPMI_CC_PARM_OUT_OF_RANGE;
        }

        auto diff = ipmi::sel::selRecordSize - requestData->offset;
        auto readLength =
                std::min(diff, static_cast<int>(requestData->readLength));

        uint8_t* partialRecord = static_cast<uint8_t*>(response);
        std::copy_n(partialRecord + sizeof(record->nextRecordID) +
                    requestData->offset,
                    readLength, partialRecord + sizeof(record->nextRecordID));
        *data_len = sizeof(record->nextRecordID) + readLength;
    }

    return IPMI_CC_OK;
}

ipmi_ret_t clearSEL(ipmi_netfn_t netfn, ipmi_cmd_t cmd, ipmi_request_t request,
                    ipmi_response_t response, ipmi_data_len_t data_len,
                    ipmi_context_t context)
{
    if (*data_len != sizeof(ipmi::sel::ClearSELRequest))
    {
        *data_len = 0;
        return IPMI_CC_REQ_DATA_LEN_INVALID;
    }
    auto requestData = static_cast<const ipmi::sel::ClearSELRequest*>(request);

    if (!checkSELReservation(requestData->reservationID))
    {
        *data_len = 0;
        return IPMI_CC_INVALID_RESERVATION_ID;
    }

    if (requestData->charC != 'C' || requestData->charL != 'L' ||
        requestData->charR != 'R')
    {
        *data_len = 0;
        return IPMI_CC_INVALID_FIELD_REQUEST;
    }

    uint8_t eraseProgress = ipmi::sel::eraseComplete;

    /*
     * Erasure status cannot be fetched from DBUS, so always return erasure
     * status as `erase completed`.
     */
    if (requestData->eraseOperation == ipmi::sel::getEraseStatus)
    {
        *static_cast<uint8_t*>(response) = eraseProgress;
        *data_len = sizeof(eraseProgress);
        return IPMI_CC_OK;
    }

    // Per the IPMI spec, need to cancel any reservation when the SEL is cleared
    cancelSELReservation();

    // Save the erase time
    ipmi::sel::erase_time::save();

    // Clear the SEL by by rotating the journal to start a new file then
    // vacuuming to keep only the new file
    if (boost::process::system("/bin/journalctl", "--rotate") != 0)
    {
        return IPMI_CC_UNSPECIFIED_ERROR;
    }
    if (boost::process::system("/bin/journalctl", "--vacuum-files=1") != 0)
    {
        return IPMI_CC_UNSPECIFIED_ERROR;
    }

    *static_cast<uint8_t*>(response) = eraseProgress;
    *data_len = sizeof(eraseProgress);
    return IPMI_CC_OK;
}

ipmi_ret_t ipmi_storage_add_sel(ipmi_netfn_t netfn, ipmi_cmd_t cmd,
                                ipmi_request_t request,
                                ipmi_response_t response,
                                ipmi_data_len_t data_len,
                                ipmi_context_t context)
{
    static constexpr char const* ipmiSELService =
            "xyz.openbmc_project.Logging.IPMI";
    static constexpr char const* ipmiSELPath =
            "/xyz/openbmc_project/Logging/IPMI";
    static constexpr char const* ipmiSELAddInterface =
            "xyz.openbmc_project.Logging.IPMI";
    static const std::string ipmiSELAddMessage =
            "IPMI SEL entry logged using IPMI Add SEL Entry command.";
    uint16_t recordID = 0;
    sdbusplus::bus::bus bus{ipmid_get_sd_bus_connection()};

    if (*data_len != sizeof(ipmi::sel::AddSELEntryRequest))
    {
        *data_len = 0;
        return IPMI_CC_REQ_DATA_LEN_INVALID;
    }
    ipmi::sel::AddSELEntryRequest* req =
            static_cast<ipmi::sel::AddSELEntryRequest*>(request);

    // Per the IPMI spec, need to cancel any reservation when a SEL entry is
    // added
    cancelSELReservation();

    if (req->recordType == ipmi::sel::systemEvent)
    {
        std::string sensorPath = getPathFromSensorNumber(req->sensorNum);
        std::vector<uint8_t> eventData(
                req->eventData, req->eventData + ipmi::sel::systemEventSize);
        bool assert = !(req->eventType & ipmi::sel::deassertionEvent);
        uint16_t genId = req->generatorID;
        sdbusplus::message::message writeSEL = bus.new_method_call(
                ipmiSELService, ipmiSELPath, ipmiSELAddInterface, "IpmiSelAdd");
        writeSEL.append(ipmiSELAddMessage, sensorPath, eventData, assert,
                        genId);
        try
        {
            sdbusplus::message::message writeSELResp = bus.call(writeSEL);
            writeSELResp.read(recordID);
        }
        catch (sdbusplus::exception_t& e)
        {
            log<level::ERR>(e.what());
            *data_len = 0;
            return IPMI_CC_UNSPECIFIED_ERROR;
        }

        uint8_t sensorNumber = req->sensorNum;
        uint8_t ret = LenovoSetSensorProperty(sensorNumber, eventData);
        if (0 != ret)
        {
            std::cerr << "Set Property Failed" << "\n";
        }
    }
    else if (req->recordType >= ipmi::sel::oemTsEventFirst &&
             req->recordType <= ipmi::sel::oemEventLast)
    {
        std::vector<uint8_t> eventData;
        if (req->recordType <= ipmi::sel::oemTsEventLast)
        {
            ipmi::sel::AddSELEntryRequestOEMTimestamped* oemTsRequest =
                    static_cast<ipmi::sel::AddSELEntryRequestOEMTimestamped*>(
                            request);
            eventData = std::vector<uint8_t>(oemTsRequest->eventData,
                                             oemTsRequest->eventData +
                                             ipmi::sel::oemTsEventSize);
        }
        else
        {
            ipmi::sel::AddSELEntryRequestOEM* oemRequest =
                    static_cast<ipmi::sel::AddSELEntryRequestOEM*>(request);
            eventData = std::vector<uint8_t>(oemRequest->eventData,
                                             oemRequest->eventData +
                                             ipmi::sel::oemEventSize);
        }
        sdbusplus::message::message writeSEL = bus.new_method_call(
                ipmiSELService, ipmiSELPath, ipmiSELAddInterface, "IpmiSelAddOem");
        writeSEL.append(ipmiSELAddMessage, eventData, req->recordType);
        try
        {
            sdbusplus::message::message writeSELResp = bus.call(writeSEL);
            writeSELResp.read(recordID);
        }
        catch (sdbusplus::exception_t& e)
        {
            log<level::ERR>(e.what());
            *data_len = 0;
            return IPMI_CC_UNSPECIFIED_ERROR;
        }

        uint8_t sensorNumber = req->sensorNum;
        uint8_t ret = LenovoSetSensorProperty(sensorNumber, eventData);
        if (0 != ret)
        {
            std::cerr << "Set Property Failed" << "\n";
        }
    }
    else
    {
        *data_len = 0;
        return IPMI_CC_PARM_OUT_OF_RANGE;
    }

    *static_cast<uint16_t*>(response) = recordID;
    *data_len = sizeof(recordID);
    return IPMI_CC_OK;
}
void registerStorageFunctions()
{
    // <Get FRU Inventory Area Info>
    ipmi::registerHandler(ipmi::prioOemBase, ipmi::netFnStorage,
                          ipmi::storage::cmdGetFruInventoryAreaInfo,
                          ipmi::Privilege::User, ipmiStorageGetFruInvAreaInfo);
    // <READ FRU Data>
    ipmiPrintAndRegister(
        NETFUN_STORAGE,
        static_cast<ipmi_cmd_t>(IPMINetfnStorageCmds::ipmiCmdReadFRUData), NULL,
        ipmiStorageReadFRUData, PRIVILEGE_USER);

    // <WRITE FRU Data>
    ipmiPrintAndRegister(
        NETFUN_STORAGE,
        static_cast<ipmi_cmd_t>(IPMINetfnStorageCmds::ipmiCmdWriteFRUData),
        NULL, ipmiStorageWriteFRUData, PRIVILEGE_OPERATOR);

    // <Add SEL Entry>
    ipmi_register_callback(NETFUN_STORAGE, IPMI_CMD_ADD_SEL, NULL,
                           ipmi_storage_add_sel, PRIVILEGE_OPERATOR);


    // <Clear SEL>
    ipmi_register_callback(NETFUN_STORAGE, IPMI_CMD_CLEAR_SEL, NULL, clearSEL,
                           PRIVILEGE_OPERATOR);

    // <Get SEL Info>
    ipmi_register_callback(NETFUN_STORAGE, IPMI_CMD_GET_SEL_INFO, NULL,
                           getSELInfo, PRIVILEGE_USER);

    // <Get SEL Entry>
    ipmi_register_callback(NETFUN_STORAGE, IPMI_CMD_GET_SEL_ENTRY, NULL,
                           getSELEntry, PRIVILEGE_USER);
}
} // namespace storage
} // namespace ipmi
