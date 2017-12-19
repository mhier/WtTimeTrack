/*
 * WtTimeTrack - Web-based time tracker
 *
 * Copyright (C) 2017 Martin Hierholzer, Hamburg, Germany
 *
 * This program is released under the GNU GENERAL PUBLIC LICENSE v3.0
 * See the LICENSE file for terms of use.
 */

#ifndef INCLUDE_HOLIDAY_H_
#define INCLUDE_HOLIDAY_H_

#include <Wt/WDate.h>

#include <Wt/Dbo/Dbo.h>
#include <Wt/Dbo/WtSqlTraits.h>

class User;

class Holiday {
  public:
    Holiday() {}

    Wt::WDate first;
    Wt::WDate last;

    template<class Action>
    void persist ( Action& a ) {
        Wt::Dbo::field ( a, first, "first" );
        Wt::Dbo::field ( a, last, "last" );
    }

};

#endif /* INCLUDE_HOLIDAY_H_ */
