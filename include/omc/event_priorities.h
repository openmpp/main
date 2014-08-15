/**
 * @file    evnet_priorities.h
 * Declares fixed constants for event priorities
 *         
 */
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

namespace openm {
    const int event_priority_self_scheduling = 255;
    const int event_priority_time_keeping = 254;
    const int event_priority_maximum_developer = 250;
};

