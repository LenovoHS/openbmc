/*
// Copyright (c) 2019 Lenovo Corporation
//
*/
#include <systemd/sd-journal.h>
#include <boost/algorithm/string.hpp>
#include <boost/container/flat_map.hpp>
#include <boost/container/flat_set.hpp>
#include <sdbusplus/asio/object_server.hpp>
#include <iomanip>
#include <led-manager.hpp>
#include <iostream>
#include <ipmid/api.hpp>
#include <ipmid/types.hpp>
#include <ipmid/utils.hpp>
#include <phosphor-logging/log.hpp>

using namespace ipmi::sensor;

using namespace phosphor::logging;
using sdbusplus::exception::SdBusError;
extern const ipmi::sensor::IdInfoMap sensors;
static uint8_t f_temp_sensor = 0;


inline static sdbusplus::bus::match::match startThresholdEventMonitor(
    std::shared_ptr<sdbusplus::asio::connection> conn)
{
	auto thresholdEventMatcherCallback = [conn](
						sdbusplus::message::message &msg){
		
		// This static set of std::pair<path, event> tracks asserted events to
        // avoid duplicate logs or deasserts logged without an assert
        static boost::container::flat_set<std::pair<std::string, std::string>>
            assertedLed;

        // Get the event type and assertion details from the message
        std::string thresholdInterface;
        boost::container::flat_map<std::string, std::variant<bool>>
            propertiesChanged;
        msg.read(thresholdInterface, propertiesChanged);
		std::string event = propertiesChanged.begin()->first;
        bool *pval = std::get_if<bool>(&propertiesChanged.begin()->second);
        if (!pval)
        {
            std::cerr << "threshold event direction has invalid type\n";
            return;
        }
        bool assert = *pval;
		// Check the asserted events to determine if we should log this event
        std::pair<std::string, std::string> LedpathAndEvent(
            std::string(msg.get_path()), event);
		
		std::string_view sensorName(msg.get_path());
        sensorName.remove_prefix(
            std::min(sensorName.find_last_of("/") + 1, sensorName.size()));
		 
		std::string_view sensorPath(msg.get_path());
		sensorPath.remove_suffix(sensorName.size() + 1);

		std::string_view sensorType(sensorPath);
		sensorType.remove_prefix(
            std::min(sensorType.find_last_of("/") + 1, sensorType.size()));
			
		if(std::string(sensorType) == "temperature")
		{	
		  if((event == "WarningAlarmHigh") || (event == "CriticalHigh") || (event == "WarningHigh") || (event == "CriticalAlarmHigh"))
		  {
				
			try
			{
				auto b = sdbusplus::bus::new_default();
				auto method = b.new_method_call(faultLedObject,faultLedPath, propertyIface,"Get");
				method.append(faultLedInterface,"Asserted");
				auto result = b.call(method);

				sdbusplus::message::variant<bool> state;
				result.read(state);
				
				bool led_state = sdbusplus::message::variant_ns::get<bool>(state);
				
				if (assert)
				{
					// For asserts, add the event to the set and only log it if it's new
					if (assertedLed.insert(LedpathAndEvent).second == true)
					{
						f_temp_sensor += 1;
					}
					
					if (!led_state)
					{
						led_state = true;
						method = b.new_method_call(faultLedObject,faultLedPath, propertyIface,"Set");

						method.append(faultLedInterface,"Asserted", std::variant<bool>(led_state));
						result = b.call(method);
					}
			
				}	
				else
				{
					if (assertedLed.erase(LedpathAndEvent) != 0)
					{
						f_temp_sensor -= 1;
					}
					
					//trun off fault led
					if (f_temp_sensor == 0)
					{
						if(led_state)
						{
							led_state = false;
							method = b.new_method_call(faultLedObject,faultLedPath, propertyIface,"Set");

							method.append(faultLedInterface,"Asserted", std::variant<bool>(led_state));
							result = b.call(method);
						}
					}
				}
				

				
			}
			catch (SdBusError& e)
			{
				log<level::ERR>("Error dbus fault",
                        entry("ERROR=%s", e.what()));
			}
		  }
		}
		
	};
    sdbusplus::bus::match::match thresholdEventMatcher(
        static_cast<sdbusplus::bus::bus &>(*conn),
        "type='signal',interface='org.freedesktop.DBus.Properties',member='"
        "PropertiesChanged',arg0namespace='xyz.openbmc_project.Sensor."
        "Threshold'",
        std::move(thresholdEventMatcherCallback));
    return thresholdEventMatcher;		
}

int main(int argc, char *argv[])
{
	boost::asio::io_service io;
	auto conn = std::make_shared<sdbusplus::asio::connection>(io);
	sdbusplus::bus::match::match thresholdEventMonitor =
        startThresholdEventMonitor(conn);
	io.run();
	return 0;
    
}