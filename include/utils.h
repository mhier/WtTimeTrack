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

inline std::string secondsToString(int seconds, bool includeSecs=false) {
    int secs = seconds % 60;
    int minutes = (seconds / 60) % 60;
    int hours = seconds / 3600;
    std::string shours = std::to_string(static_cast<int>(hours));
    std::string sminutes = std::to_string(static_cast<int>(minutes));
    if(sminutes.length() == 1) sminutes = "0"+sminutes;
    if(shours.length() == 1) shours = "0"+shours;
    if(includeSecs) {
      std::string ssecs = std::to_string(static_cast<int>(secs));
      if(ssecs.length() == 1) ssecs = "0"+ssecs;
      return shours + ":" + sminutes + ":" + ssecs;
    }
    return shours + ":" + sminutes;
}

#endif // INCLUDE_UTILS_H_
