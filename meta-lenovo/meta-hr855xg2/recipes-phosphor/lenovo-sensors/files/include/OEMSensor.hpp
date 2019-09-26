#pragma once

#include "Utils.hpp"

#include <boost/container/flat_map.hpp>
#include <filesystem>
#include <fstream>
#include <gpiod.hpp>
#include <sdbusplus/asio/object_server.hpp>

struct OEMInfo
{
    OEMInfo(const std::string& iface, const std::string& property,
            const std::string& ptype, const std::string& dfvalue) :
        iface(iface), property(property), ptype(ptype), dfvalue(dfvalue)
    {
    }
    std::string iface;
    std::string property;
    std::string ptype;
    std::string dfvalue;

    bool operator<(const OEMInfo& rhs) const
    {
        return (iface < rhs.iface);
    }
};

struct OEMConfig
{
    OEMConfig(const uint8_t& snrnum, const uint8_t& snrtype,
              const std::string& name, const std::vector<OEMInfo>& oeminfo) :
        snrnum(snrnum), snrtype(snrtype), name(name), oeminfo(std::move(oeminfo))
    {
    }
    uint8_t snrnum;
    uint8_t snrtype;
    std::string name;
    std::vector<OEMInfo> oeminfo;

    bool operator<(const OEMConfig& rhs) const
    {
        return (name < rhs.name);
    }
};

constexpr auto oemService = "org.openbmc.control.oem";
constexpr auto oemRoot = "/org/openbmc/control/oem";
constexpr auto oemInterface = "org.openbmc.control.oem";
constexpr auto InventoryService = "xyz.openbmc_project.Inventory.Manager";
constexpr auto InventoryPath = "/xyz/openbmc_project/inventory/system/chassis/motherboard/";
constexpr auto InventoryItemIntf = "xyz.openbmc_project.Inventory.Item";
constexpr auto PROP_INTF = "org.freedesktop.DBus.Properties";
constexpr auto IPMIService = "xyz.openbmc_project.Ipmi.Host";
constexpr auto IPMIPath = "/xyz/openbmc_project/Ipmi";
constexpr auto IPMIIntf = "xyz.openbmc_project.Ipmi.Server";
