/*
 * WtTimeTrack - Web-based time tracker
 *
 * Copyright (C) 2017 Martin Hierholzer, Hamburg, Germany
 *
 * This program is released under the GNU GENERAL PUBLIC LICENSE v3.0
 * See the LICENSE file for terms of use.
 */

#ifndef INCLUDE_UTILS_H_
#define INCLUDE_UTILS_H_

#include <cmath>
#include <string>

std::string secondsToString(double seconds) {
    double minutes = std::floor(seconds / 60.);
    double hours = std::floor(minutes / 60.);
    minutes -= hours*60.;
    std::string shours = std::to_string(static_cast<int>(hours));
    std::string sminutes = std::to_string(static_cast<int>(minutes));
    if(sminutes.length() == 1) sminutes = "0"+sminutes;
    if(shours.length() == 1) shours = "0"+shours;
    return shours + ":" + sminutes;
}

#endif // INCLUDE_UTILS_H_
