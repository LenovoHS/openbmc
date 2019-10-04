#include <arpa/inet.h>
#include <mapper.h>
#include <systemd/sd-bus.h>

#include <algorithm>
#include <boost/process.hpp>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <ipmid/api.h>
#include "../include/sdrutils.hpp"
#include "../include/selutility.hpp"

#include <ipmid/utils.hpp>
#include <ipmid/types.hpp>
#include <phosphor-logging/elog-errors.hpp>
#include <phosphor-logging/log.hpp>
#include <sdbusplus/server.hpp>
#include <string>
#include <string_view>
#include <variant>
#include <xyz/openbmc_project/Common/error.hpp>
#include <nlohmann/json.hpp>

static constexpr auto InventoryService = "xyz.openbmc_project.Inventory.Manager";
static constexpr auto InventoryPath = "/xyz/openbmc_project/inventory/system/chassis/motherboard/";
static constexpr auto InventoryECCIntf = "xyz.openbmc_project.Memory.MemoryECC";
static constexpr auto CPUSensorService = "xyz.openbmc_project.CPUSensor";
static constexpr auto CPUSensorPath = "/xyz/openbmc_project/inventory/system/chassis/motherboard/";
static constexpr auto CPUSensorIntf ="xyz.openbmc_project.Inventory.Item.Cpu";
static constexpr auto OEMSensorService = "xyz.openbmc_project.OEMSensor";
static constexpr auto OEMSensorPath = "/xyz/openbmc_project/OEMSensor/";
static constexpr auto OEMSensorIntf = "xyz.openbmc_project.Sensor.Discrete.SpecificOffset";

extern const ipmi::sensor::IdInfoMap sensors;


void register_netfn_storage_functions() __attribute__((constructor));


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

// Lenovo Set Sensor Property
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
    std::cout<<"this is func JOURNAL_SEL getSELInfo"<<std::endl;
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
    std::cout<<"this is after contents = std::string(metadata);contents="<<contents<<std::endl;
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
    std::cout<<"this is func JournalSel:getSELEntry"<<std::endl;
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
    std::cout<<"this is after  std::string match =v MESSAGE_ID=match="<<match<<std::endl;
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
    std::cout<<"this is after func match = IPMI_SEL_RECORD_ID=match="<<match<<std::endl;
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
                std::cout<<"this is after func getJournalMetadata(journal.get(),IPMI_SEL_RECORD_TYPE"<<std::endl;

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
                std::cout<<"this is after record->recordType = recordType"<<recordType<<std::endl;

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
                std::cout<<"this is after IPMI_SEL_SENSOR_PATH path="<<path<<std::endl;
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
                std::cout<<"record->sensorNum ="<<std::hex<<(unsigned int)(record->sensorNum)<<std::endl;
                std::cout<<"record->sensorType ="<<std::hex<<(unsigned int)(record->sensorType)<<std::endl;
                std::cout<<"record->eventType ="<<std::hex<<(unsigned int)(record->eventType)<<std::endl;
                std::cout<<"this is after getSensorEventTypeFromPath"<<record->eventType<<std::endl;

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
                std::cout<<"this is after  else if (recordType >= ipmi::sel::oemTsEventFirst &&"<<std::endl;
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
        //auto iter = sensors.find(req->sensorNum);
        //std::string sensorPath = (iter->second).sensorPath;
        std::cout<<"this is in func ipmi_storage_add_sel sensorPath"<<sensorPath<<std::endl;
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



void register_netfn_storage_functions()
{

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
    return;
}
