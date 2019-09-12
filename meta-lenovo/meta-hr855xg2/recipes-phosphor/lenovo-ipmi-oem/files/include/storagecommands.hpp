/*
// Copyright (c) 2017 2018 Intel Corporation
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

#pragma once
#include <cstdint>

#pragma pack(push, 1)
struct GetFRUAreaReq
{
    uint8_t fruDeviceID;
    uint16_t fruInventoryOffset;
    uint8_t countToRead;
};

struct WriteFRUDataReq
{
    uint8_t fruDeviceID;
    uint16_t fruInventoryOffset;
    uint8_t data[];
};
#pragma pack(pop)

enum class GetFRUAreaAccessType : uint8_t
{
    byte = 0x0,
    words = 0x1
};

enum class IPMINetfnStorageCmds : ipmi_cmd_t
{
    ipmiCmdReadFRUData = 0x11,
    ipmiCmdWriteFRUData = 0x12,
};

#pragma pack(push, 1)
struct FRUHeader
{
    uint8_t commonHeaderFormat;
    uint8_t internalOffset;
    uint8_t chassisOffset;
    uint8_t boardOffset;
    uint8_t productOffset;
    uint8_t multiRecordOffset;
    uint8_t pad;
    uint8_t checksum;
};
#pragma pack(pop)

/*namespace ipmi
{
namespace storage
{
ipmi_ret_t getFruSdrs(size_t index, get_sdr::SensorDataFruRecord& resp);

ipmi_ret_t getFruSdrCount(size_t& count);
} // namespace storage
}*/ // namespace ipmi
