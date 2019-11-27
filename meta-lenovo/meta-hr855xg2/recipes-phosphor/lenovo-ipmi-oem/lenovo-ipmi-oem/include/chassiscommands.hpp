/*
// Copyright (c) 2019-present Lenovo
// Licensed under BSD-3, see LICENSE file for details.
*/

#include "status.hpp"
#include <ipmid/api.h>
#include <mapper.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <systemd/sd-bus.h>
#include <unistd.h>
#include <memory>
#include <sdbusplus/bus.hpp>
#include <sdbusplus/bus.hpp>
#include <sdbusplus/timer.hpp>

namespace ipmi::chassis
{
struct Identify
{
    uint8_t identifyInterval;
    uint8_t forceIdentify;
} __attribute__((packed));

class UIDIdentify : public TriggerableActionInterface
{
    public:
        /**
        * Create a default Verification object that uses systemd to trigger the
        * process.
        *
        * @param[in] bus - an sdbusplus handler for a bus to use.
        * @param[in] path - the path to check for verification status.
        * @param[in[ service - the systemd service to start to trigger
        * verification.
        */
        static std::unique_ptr<TriggerableActionInterface>
        createIdentify(sdbusplus::bus::bus&& bus,
                       const std::string& service);

        UIDIdentify(sdbusplus::bus::bus&& bus,
                    const std::string& service):
        bus(std::move(bus)),triggerService(service)
       {
       }

        ~UIDIdentify() = default;
        UIDIdentify(const UIDIdentify&) = delete;
        UIDIdentify& operator=(const UIDIdentify&) = delete;
        UIDIdentify(UIDIdentify&&) = default;
        UIDIdentify& operator=(UIDIdentify&&) = default;

        bool trigger() override;
        void abort() override;
        ActionStatus status() override;

    private:
        sdbusplus::bus::bus bus;
        const std::string triggerService;
        ActionStatus state = ActionStatus::unknown;
};
}