#pragma once

#include "Utils.hpp"

#include <boost/container/flat_map.hpp>
#include <filesystem>
#include <fstream>
#include <gpiod.hpp>
#include <sdbusplus/asio/object_server.hpp>
#ifdef __cplusplus
extern "C" {
#endif
#include <fpga.h>
#ifdef __cplusplus
}
#endif

#define MAX_GPIO 254

struct FPGAInfo
{
    FPGAInfo(const uint8_t& block, const uint8_t& addr, const uint8_t& offset,
             const uint8_t& evtbit, const uint8_t & trigstate,
             const std::string& service, const std::string& path,
             const std::string& iface, const std::string& property,
             const std::string& ptype, const std::string& setvalue) :
        block(block), addr(addr), offset(offset), evtbit(evtbit), trigstate(trigstate),
        service(service), path(path), iface(iface), property(property), ptype(ptype), setvalue(setvalue)
    {
    }
    uint8_t block;
    uint8_t addr;
    uint8_t offset;
    uint8_t evtbit;
    uint8_t trigstate;
    std::string service;
    std::string path;
    std::string iface;
    std::string property;
    std::string ptype;
    std::string setvalue;
};

struct FPGAConfig
{
    FPGAConfig(const uint8_t& snrnum, const uint8_t& snrtype, const std::string& name, const bool& en_mon,
               std::shared_ptr<sdbusplus::asio::dbus_interface>& snriface,
               const std::string& fpgagpio, const std::vector<FPGAInfo>& fpgainfo) :
        snrnum(snrnum), snrtype(snrtype), name(name), en_mon(en_mon), snriface(snriface), fpgagpio(fpgagpio), fpgainfo(std::move(fpgainfo))
    {
    }
    uint8_t snrnum;
    uint8_t snrtype;
    std::string name;
    std::shared_ptr<sdbusplus::asio::dbus_interface> snriface;
    bool en_mon;
    std::string fpgagpio;
    std::vector<FPGAInfo> fpgainfo;

    bool operator<(const FPGAConfig& rhs) const
    {
        return (name < rhs.name);
    }
};

struct FPGAInterrupt
{
    FPGAInterrupt(const std::string& name, const std::vector<FPGAConfig>& fpgaconfig) :
        name(name), fpgaconfig(std::move(fpgaconfig))
    {
    }
    std::string name;
    std::vector<FPGAConfig> fpgaconfig;

    bool operator<(const FPGAInterrupt& rhs) const
    {
        return (name < rhs.name);
    }
};

class FPGASensor
{
  public:
    FPGASensor(boost::asio::io_service& io,
               std::shared_ptr<sdbusplus::asio::connection>& conn,
               FPGAInterrupt& sensorconfig);
    ~FPGASensor();
	
    static constexpr unsigned int sensorPollMs = 2000;
    int add_sel_event(uint8_t snr_num, uint8_t snr_type,
                          uint8_t ed1, uint8_t ed2, uint8_t ed3);
    int eventUpdate(FPGAConfig fpgaconfigs,
                    std::string service, std::string path,
                    std::string iface, std::string property,
                    std::string ptype, std::string setvalue,
                    bool assert);
  private:
    boost::asio::deadline_timer waitTimer;
    std::shared_ptr<sdbusplus::asio::connection> mDbusConn;
    boost::container::flat_map<std::string, bool> mAssert;
    FPGAInterrupt mSnrInterrupt;

    void setupRead(void);
    void handleResponse(void);
};

extern boost::container::flat_map<std::string, std::unique_ptr<FPGASensor>>
    gFpgaSensors;

constexpr auto fpgaService = "org.openbmc.control.fpga";
constexpr auto fpgaRoot = "/org/openbmc/control/fpga";
constexpr auto fpgaInterface = "org.openbmc.control.fpga";
constexpr auto InventoryService = "xyz.openbmc_project.Inventory.Manager";
constexpr auto InventoryPath = "/xyz/openbmc_project/inventory/system/chassis/motherboard/";
constexpr auto InventoryItemIntf = "xyz.openbmc_project.Inventory.Item";
constexpr auto PROP_INTF = "org.freedesktop.DBus.Properties";
constexpr auto IPMIService = "xyz.openbmc_project.Ipmi.Host";
constexpr auto IPMIPath = "/xyz/openbmc_project/Ipmi";
constexpr auto IPMIIntf = "xyz.openbmc_project.Ipmi.Server";

