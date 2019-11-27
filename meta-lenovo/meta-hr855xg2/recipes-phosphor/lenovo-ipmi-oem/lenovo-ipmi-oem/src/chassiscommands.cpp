/*
// Copyright (c) 2019-present Lenovo
// Licensed under BSD-3, see LICENSE file for details.
*/


#include "../include/chassiscommands.hpp"
#include <ipmid/api.h>
#include <fstream>
#include <ipmid/api-types.hpp>
#include <ipmid/utils.hpp>
#include <iostream>
#include <nlohmann/json.hpp>
#include <phosphor-logging/elog-errors.hpp>
#include <phosphor-logging/log.hpp>
#include <regex>
#include <stdexcept>
#include <string_view>
using namespace phosphor::logging;

namespace ipmi::chassis
{
std::unique_ptr<phosphor::Timer> identifyTimer
    __attribute__((init_priority(101)));

constexpr size_t defaultIdentifyTimeOut = 15;


static void registerChassisFunctions() __attribute__((constructor));
sdbusplus::bus::bus dbus(ipmid_get_sd_bus_connection()); // from ipmid/api.h

void identifyLedOff(void)
{
    static constexpr auto uidService = "uidoff.service";
    auto uider = ipmi::chassis::UIDIdentify::createIdentify(
        sdbusplus::bus::new_default(), uidService);
    std::unique_ptr<TriggerableActionInterface> uid = std::move(uider);
    uid->trigger();
}

void identifyLedOn(void)
{
    static constexpr auto uidService = "uidon.service";
    auto uider = ipmi::chassis::UIDIdentify::createIdentify(
        sdbusplus::bus::new_default(), uidService);
    std::unique_ptr<TriggerableActionInterface> uid = std::move(uider);
    uid->trigger();

}

/** @brief Create timer to turn on and off the enclosure LED
 */
void createIdentifyTimer()
{
    if (!identifyTimer)
    {
        identifyTimer =
            std::make_unique<phosphor::Timer>(identifyLedOff);
    }
}

ipmi_ret_t ipmiChassisIdentify(ipmi_netfn_t netfn, ipmi_cmd_t cmd,
                               ipmi_request_t request,
                               ipmi_response_t response,
                               ipmi_data_len_t data_len,
                               ipmi_context_t context)
{
    
    ipmi_ret_t rc = IPMI_CC_OK;
    
    auto time = std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::seconds(defaultIdentifyTimeOut));

    if( request != NULL)
    { 
        auto* req = reinterpret_cast<ipmi::chassis::Identify*>(request);
        if (*data_len != sizeof(Identify))
        {
            req->forceIdentify = 0;
        }
        if (req->identifyInterval || req->forceIdentify)
        {
            // stop the timer if already started;
            // for force identify we should not turn off LED
            identifyTimer->stop();
            identifyLedOn();
            if (req->forceIdentify)
            {
                return rc;
            }
        }
        else if (req->identifyInterval == 0)
        {
            identifyTimer->stop();
            identifyLedOff(); //turn off
            return rc;
        }
        // start the timer
         time = std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::seconds(req->identifyInterval));
        
    }
    else
    {
        identifyTimer->stop();
        identifyLedOn();
    }
    identifyTimer->start(time);
    return rc;
}


static void registerChassisFunctions(void)
{
    log<level::INFO>("Registering Chassis commands");
    createIdentifyTimer();
    // <Chassis Identify>
    ipmi_register_callback(ipmi::netFnChassis,ipmi::chassis::cmdChassisIdentify,NULL,ipmiChassisIdentify,PRIVILEGE_USER);

}

std::unique_ptr<TriggerableActionInterface>
    UIDIdentify::createIdentify(sdbusplus::bus::bus&& bus,
                                const std::string& service)
{
    return std::make_unique<UIDIdentify>(std::move(bus), service);
}

bool UIDIdentify::trigger()
{
    static constexpr auto systemdService = "org.freedesktop.systemd1";
    static constexpr auto systemdRoot = "/org/freedesktop/systemd1";
    static constexpr auto systemdInterface = "org.freedesktop.systemd1.Manager";

    auto method = bus.new_method_call(systemdService, systemdRoot,
                                      systemdInterface, "StartUnit");
    method.append(triggerService);
    method.append("replace");
    try
    {
        bus.call_noreply(method);
    }
    catch (const sdbusplus::exception::SdBusError& ex)
    {
        /* TODO: Once logging supports unit-tests, add a log message to test
         * this failure.
         */
        state = ActionStatus::failed;
        return false;
    }

    state = ActionStatus::success;
    return true;
}

void UIDIdentify::abort()
{
    /* TODO: Implement this. */
}

ActionStatus UIDIdentify::status()
{
    return state;
}


} // namespace ipmi::chassis
