/*
 * WtTimeTrack - Web-based time tracker
 *
 * Copyright (C) 2017 Martin Hierholzer, Hamburg, Germany
 *
 * This program is released under the GNU GENERAL PUBLIC LICENSE v3.0
 * See the LICENSE file for terms of use.
 */

#ifndef CALENDAR_CELL_H_
#define CALENDAR_CELL_H_

#include "User.h"
#include "Session.h"

#include <Wt/WContainerWidget.h>

using namespace Wt;

class PlannerCalendar;

class CalendarCell : public WContainerWidget {
  public:
    CalendarCell(Session &session);

    void update(const WDate& date);
    void update() {update(date_);};

    WDate date() {return date_; }

    WDate date_;
    std::unique_ptr<WDialog> dialog_;

    Session &session_;
    PlannerCalendar *owner_;

    void showCellDialog();
};

#endif //CALENDAR_CELL_H_
