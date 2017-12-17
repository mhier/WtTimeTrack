/*
 * WtTimeTrack - Web-based time tracker
 *
 * Copyright (C) 2017 Martin Hierholzer, Hamburg, Germany
 *
 * This program is released under the GNU GENERAL PUBLIC LICENSE v3.0
 * See the LICENSE file for terms of use.
 */

#ifndef INCLUDE_DEBITTIME_H_
#define INCLUDE_DEBITTIME_H_

#include <tuple>
#include <cmath>

#include <Wt/WDate.h>

#include <Wt/Dbo/Dbo.h>
#include <Wt/Dbo/WtSqlTraits.h>

using namespace Wt;

class User;

class DebitTime {
  public:
    DebitTime() {}

    Dbo::ptr<User> user;

    WDate validFrom;

    std::array<double, 7> workHoursPerWeekday{0,0,0,0,0,0,0};

    template<class Action>
    void persist ( Action& a ) {
        Dbo::belongsTo ( a, user, "debitTimes" );

        Dbo::field ( a, validFrom, "validFrom" );
        for(size_t i=0; i<7; ++i) {
          Dbo::field ( a, workHoursPerWeekday[i], "workHoursPerWeekday_"+std::to_string(i) );
        }
    }

};

#endif /* INCLUDE_DEBITTIME_H_ */
