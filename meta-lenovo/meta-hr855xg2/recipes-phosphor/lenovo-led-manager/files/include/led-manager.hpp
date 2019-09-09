/*
// Copyright (c) 2019 Lenovo Corporation
//
*/

#pragma once
#include <filesystem>

static constexpr size_t selEvtDataMaxSize = 3;

static constexpr char const *ipmiSelObject = "xyz.openbmc_project.Logging.IPMI";
static constexpr char const *ipmiSelPath = "/xyz/openbmc_project/Logging/IPMI";
static constexpr char const *ipmiSelAddInterface =
    "xyz.openbmc_project.Logging.IPMI";
	
constexpr auto propertyIface = "org.freedesktop.DBus.Properties";
static constexpr char const *faultLedObject = "xyz.openbmc_project.LED.GroupManager";
constexpr auto faultLedPath = "/xyz/openbmc_project/led/groups/enclosure_fault";
static constexpr char const *faultLedInterface = "xyz.openbmc_project.Led.Group";

static constexpr char const *powerObject = "org.openbmc.control.Power";
constexpr auto powerPath = "/org/openbmc/control/power0";
static constexpr char const *powerInterface = "org.openbmc.control.Power";


#define FAULT_LED_GROUP "enclosure_fault"
