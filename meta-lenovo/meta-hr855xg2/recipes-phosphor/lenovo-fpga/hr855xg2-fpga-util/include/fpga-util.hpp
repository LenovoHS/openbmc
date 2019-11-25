#include <boost/algorithm/string/predicate.hpp>
#include <boost/container/flat_map.hpp>
#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/asio/object_server.hpp>
#include <sdbusplus/message/types.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/container/flat_set.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/process/child.hpp>
#include <boost/asio.hpp>
#include <sdbusplus/asio/sd_event.hpp>
#include <sdbusplus/bus.hpp>
#include <sdbusplus/exception.hpp>
#include <sdbusplus/server.hpp>
#include <sdbusplus/timer.hpp>

#define CPU0_Status    0x91
#define CPU3_Status    0x94
#define DIMM0_Status   0xA0
#define DIMM47_Status  0xCF
#define PE1S1_Status   0xE0
#define PE3S2_Status   0xEB
#define Riser_1_Status 0xF4
#define Riser_3_Status 0xF6
#define M2_Status      0xF3
#define NCSI_Cable     0xF7

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

typedef struct {
    uint8_t block;
    uint8_t offset;
    uint8_t bit;
    bool trigstate;
} req_info_t;
