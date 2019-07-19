/*
// Copyright (c) 2019 Lenovo Corporation
//

*/

#include <fstream>
#include <iostream>
#include <ipmid/api-types.hpp>
#include <ipmid/utils.hpp>
#include <nlohmann/json.hpp>
#include <phosphor-logging/elog-errors.hpp>
#include <phosphor-logging/log.hpp>
#include <regex>
#include <sdbusplus/timer.hpp>
#include <sdbusplus/bus.hpp>
#include <stdexcept>
#include <string_view>
#include <ipmid/api.h>
#include <cstdint>
//#include <ipmid/oemrouter.hpp>
#include <sdbusplus/bus.hpp>
#include <sdbusplus/timer.hpp>
#include "../include/chassiscommands.hpp"

using namespace phosphor::logging;

namespace ipmi::chassis
{
std::unique_ptr<phosphor::Timer> identifyTimer
    __attribute__((init_priority(101)));
	
constexpr size_t defaultIdentifyTimeOut = 15;


static void registerChassisFunctions() __attribute__((constructor));
sdbusplus::bus::bus dbus(ipmid_get_sd_bus_connection()); // from ipmid/api.h

void IdentifyLedoff(void)
{
	static constexpr auto uidservice = "uidoff.service";
	auto uider = ipmi::chassis::UIDIdentify::CreateIdentify(
        sdbusplus::bus::new_default(), uidservice);
	std::unique_ptr<TriggerableActionInterface> uid = std::move(uider);
	bool result = uid->trigger();
	if (result){
		log<level::INFO>("force identify trigger");
	}
}

void IdentifyLedon(void)
{
	static constexpr auto uidservice = "uidon.service";
	auto uider = ipmi::chassis::UIDIdentify::CreateIdentify(
        sdbusplus::bus::new_default(), uidservice);
	std::unique_ptr<TriggerableActionInterface> uid = std::move(uider);
	bool result = uid->trigger();
	if (result){
		log<level::INFO>("force identify trigger");
	}
}

/** @brief Create timer to turn on and off the enclosure LED
 */
void createIdentifyTimer()
{
    if (!identifyTimer)
    {
        identifyTimer =
            std::make_unique<phosphor::Timer>(IdentifyLedoff);
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
			
	if( request != NULL){ 
		auto* req = reinterpret_cast<ipmi::chassis::Identify*>(request);
		if (*data_len != sizeof(Identify)){
			req->forceIdentify = 0;	
		}
		if (req->identifyInterval || req->forceIdentify){
       	log<level::INFO>("force identify");
		// stop the timer if already started;
        // for force identify we should not turn off LED
        identifyTimer->stop();
		IdentifyLedon();
        if (req->forceIdentify){
			//turn on always
			log<level::INFO>("force identify start");
			return rc;
			}
		}else if (req->identifyInterval == 0){
			log<level::INFO>("no identify");
			identifyTimer->stop();
			IdentifyLedoff(); //turn off
			return rc;
		}
		// start the timer
         time = std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::seconds(req->identifyInterval));
        
	}else{
		identifyTimer->stop();
		IdentifyLedon();	
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
    UIDIdentify::CreateIdentify(sdbusplus::bus::bus&& bus,
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
	log<level::INFO>("UIDIdentify::trigger");
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
