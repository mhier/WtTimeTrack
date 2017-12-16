/*
 * WtTimeTrack - Web-based time tracker
 *
 * Copyright (C) 2017 Martin Hierholzer, Hamburg, Germany
 *
 * This program is released under the GNU GENERAL PUBLIC LICENSE v3.0
 * See the LICENSE file for terms of use.
 */

#ifndef INCLUDE_CREDITTIME_H_
#define INCLUDE_CREDITTIME_H_

#include <tuple>
#include <cmath>

#include <Wt/WDateTime.h>

#include <Wt/Dbo/Dbo.h>
#include <Wt/Dbo/WtSqlTraits.h>

using namespace Wt;

class User;

class CreditTime {
  public:
    CreditTime() {}

    Dbo::ptr<User> user;

    WDateTime start;
    WDateTime stop;
    bool hasClockedOut{false};

    unsigned int getSecs() const {
      if(hasClockedOut) {
        return start.secsTo(stop);
      }
      else {
        return start.secsTo(WDateTime::currentDateTime());
      }
    }

    template<class Action>
    void persist ( Action& a ) {
        Dbo::belongsTo ( a, user, "creditTimes" );

        Dbo::field ( a, start, "start" );
        Dbo::field ( a, stop, "stop" );
        Dbo::field ( a, hasClockedOut, "hasClockedOut" );
    }

};

class CreditTimePeriod {

  public:

    CreditTimePeriod() {}

    CreditTimePeriod(Wt::Dbo::ptr<CreditTime> creditTime) {
      seconds = creditTime->getSecs();
    };

    CreditTimePeriod(Wt::Dbo::collection< Wt::Dbo::ptr<CreditTime> > creditTimes) {
      for(auto &entry : creditTimes) {
        seconds += entry->getSecs();
      }
    };

    /** return hours and minutes of the time period */
    std::tuple<unsigned int, unsigned int> getHoursMinutes() {
      double minutes = std::round(seconds / 60.);
      double hours = std::floor(minutes / 60.);
      minutes -= hours*60.;
      return {hours, minutes};
    }

    std::string getAsString() {
      auto hm = getHoursMinutes();
      std::string shours = std::to_string(std::get<0>(hm));
      std::string sminutes = std::to_string(std::get<1>(hm));
      if(sminutes.length() == 1) sminutes = "0"+sminutes;
      if(shours.length() == 1) shours = "0"+shours;
      return shours + ":" + sminutes;
    }

  protected:

    double seconds{0};

};

#endif /* INCLUDE_CREDITTIME_H_ */
