/*
 * WtTimeTrack - Web-based time tracker
 *
 * Copyright (C) 2017 Martin Hierholzer, Hamburg, Germany
 *
 * This program is released under the GNU GENERAL PUBLIC LICENSE v3.0
 * See the LICENSE file for terms of use.
 */

#ifndef PLANNER_CALENDAR_H_
#define PLANNER_CALENDAR_H_

#include "User.h"
#include "Session.h"

#include <Wt/Dbo/Dbo.h>
#include <Wt/WCalendar.h>

using namespace Wt;

class PlannerCalendar : public WCalendar {
  public:
    PlannerCalendar(Session &session);
  protected:
    virtual WWidget* renderCell(WWidget* widget, const WDate& date) override;

  private:
    Session &session_;
};

#endif //PLANNER_CALENDAR_H_
