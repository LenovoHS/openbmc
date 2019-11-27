/*
// Copyright (c) 2019-present Lenovo
// Licensed under BSD-3, see LICENSE file for details.
*/

#pragma once

#include <cstdint>
namespace ipmi
{
/** The status of the update mechanism or the verification mechanism */
enum class ActionStatus : std::size_t
{
    running = 0,
    success = 1,
    failed = 2,
    unknown = 3,
};

class TriggerableActionInterface
{
    public:
        virtual ~TriggerableActionInterface() = default;

        /**
        * Trigger action.
         *
         * @return true if successfully started, false otherwise.
         */
        virtual bool trigger() = 0;
        /** Abort the action if possible. */
        virtual void abort() = 0;
        /** Check the current state of the action. */
        virtual ActionStatus status() = 0;
};
}